//
//  GameLevel.cpp
//  server
//
//  Created by Bruno Macedo Miguel on 2/5/17.
//  Copyright © 2017 d2server. All rights reserved.
//
#include "GameWorld.hpp"
#include "GameLevel.hpp"
#include "NetworkData.hpp"
#include "Factory.hpp"
#include <stack>
#include <random>

uint32_t GameLevel::UNIQUEIDIDENTIFIER = 0;

void GameLevel::update()
{
    if (fpstimer.getElapsedSeconds() < 0.016)
        return;
    
    fpstimer.reset();
    
	sincronizePlayers();

    updateRespaw();

    updateItems();
    
    updateCreatures();
}

void GameLevel::sincronizePlayers()
{
	if (sincronizationTimer.getElapsedSeconds() < 5 || status != GameStatus::PLAYING)
		return;

	sincronizationTimer.reset();

	for (int i = 0; i < MAXPLAYERSPERGAME; i++)
		if (players[i])
			gameworld->broadcastBodyCoordinate(players[i]->getGUID(), players[i], players[i]->getPosition(), true);
}

void GameLevel::updateRespaw()
{
    if (respawtimer.getElapsedSeconds() < 1 || status != GameStatus::PLAYING)
        return;
    
    performance_active.time += 1;
    
    respawtimer.reset();
    
    if (!creaturesToRespaw.empty())
    {
        createCreature(creaturesToRespaw.front().first, creaturesToRespaw.front().second);
        creaturesToRespaw.pop();
    }
}

void GameLevel::updateItems()
{
    for (int i = 0; i < MAXITEMSPERLEVEL; i++)
    {
        if (items[i].getGUID() != 0 && (!items[i].isExpired() || !items[i].isTemporary()))
        {
            if (items[i].isTemporary())
                items[i].fps();
            
            if (items[i].isAmmo())
            {
                itemUpdatePosition(&items[i]);
				if (sphereCollisionHandler(&items[i], .08f))
                    items[i].doExpire();
            }
        }
        else
            if (items[i].getGUID() != 0)
                unsummonItem(&items[i], true);
    }
}

bool  GameLevel::sphereCollisionHandler(Item* item, float radius)
{
	const auto& center	= item->getPosition();

	Vector3 right		= center + Vector3(radius, 0, 0);
	Vector3 left		= center + Vector3(-radius, 0, 0);
	Vector3 top			= center + Vector3(0, 0, radius);
	Vector3 bottom		= center + Vector3(0, 0, -radius);

	const Vector3* positions[5] = { &center, &right, &left, &top, &bottom };

	for (auto pos : positions)
	{
		if (mapInBoundaries((*pos)))
		{
			Object* object = mapCell((*pos));

			if (object) {
			next:
				if (object->getType() == ObjectType::ITEM) {
					auto i = static_cast<Item*>(object);
					if (i->isBlockable())
						return true;
				}
				else
				{
					auto body = static_cast<Body*>(object);
					Body* owner = getBody(item->getOwner(), BodyType::NONE);

					if (owner != body) {
						creaturechangelife(body, owner, item->getLifeChange(), true, true);
						return true;
					}
				}

				object = object->getMapNext();
				if (object) goto next;
			}
		}
	}

	return false;
}

bool  GameLevel::itemCollisionHandler(Item* item)
{
    const auto& itemposition = item->getPosition();
    
    if (mapInBoundaries(itemposition))
    {
        Object* object = mapCell(itemposition);
        
        if (object){
            next:
            if (object->getType() == ObjectType::ITEM){
                auto i = static_cast<Item*>(object);
				if (i->isBlockable())
					return true;
            }
            else
            {
                auto body  = static_cast<Body*>(object);
                Body* owner = getBody(item->getOwner(), BodyType::NONE);
                
                if (!owner || owner != body){
					std::cerr << "Hit: " << body->getName() << std::endl;
                    creaturechangelife(body, owner, (owner ? owner->getAttack() : 0), true, true);
                    return true;
                }
            }
            
            object = object->getMapNext();
            if (object) goto next;
        }
        
    }
    return false;
}

void GameLevel::itemUpdatePosition(Item* item)
{
    Vector3 newposition = item->getPosition();
    newposition.mX += sin(item->getAngle())/4;
    newposition.mZ += cos(item->getAngle())/4;
    item->setPosition(newposition);
}

void    GameLevel::updateCreatures()
{
    for (int i = 0; i < MAXCREATURESPERLEVEL; i++)
        if (creatures[i].getGUID() != 0)
            updateCreatureState(&creatures[i]);
}

void    GameLevel::updateCreatureState(Body* creature)
{
     updateCreatureAttack(creature);
     updateCreatureMovement(creature);
}

void GameLevel::updateCreatureAttack(Body* creature)
{
    if (!creature->canAttack() || !RobotMath::TryChance(creature->getAttackChance()))
        return;
    
    const Body* currentTarget   = hasTarget(creature);
    const Body* newtarget       = nullptr;
    
    // If target has left the map or is to far: cancel
    if(currentTarget)
        if (!inGame(currentTarget) || Vector3::Distance(currentTarget->getPosition(), creature->getPosition()) > BODYVIEWRANGE)
            newtarget = nullptr;
    
    if (!newtarget)
        newtarget = mapGetClosestBody(creature, BodyType::PLAYER);
    
    
    if (currentTarget != newtarget){
        creature->setTarget(newtarget);
        for(int i = 0; i < MAXPLAYERSPERGAME; i++)
            if (players[i])
                gameworld->broadcastBodyAttacker(players[i]->getGUID(), creature, newtarget);
    }
    
	auto isShootable = [currentTarget, creature, this]() -> bool
	{
		if (Vector3::Distance(currentTarget->getPosition(), creature->getPosition()) > creature->getMinDistFromTarget())
			return false;

		Vector3 fromPosition = creature->getPosition();
		float angle = RobotMath::getRadians(currentTarget->getPosition(), fromPosition);

		while (true)
		{
			fromPosition.mX += sin(angle) / 4;
			fromPosition.mZ += cos(angle) / 4;
			if (Vector3::Distance(fromPosition, currentTarget->getPosition()) <= 2) return true;
			if (!mapIsSquareAvailable(fromPosition)) return false;
		}

		return true;
	};

     if (currentTarget && isShootable())
        creatureshoot(creature, currentTarget->getPosition() + Vector3(0.5f, 0, 0.4f));
}

void GameLevel::updateCreatureMovement(Body* creature)
{

    if (creature->canMove() && creature->getTarget())
    {
        const auto& target = creature->getTarget();
        
        const float min_distance = BODYVIEWRANGE * .5;
		const float min_distance_approachable = min_distance / 2;

		float distanceBetweenCreatures = Vector3::Distance(target->getPosition(), creature->getPosition());
        
        auto checkObstacle = [target, creature, this]() -> bool
        {
            Vector3 fromPosition = creature->getPosition();
            float angle = RobotMath::getRadians(target->getPosition(), fromPosition);
            
            while (true)
            {
                fromPosition.mX += sin(angle)/2;
                fromPosition.mZ += cos(angle)/2;
                if (Vector3::Distance(fromPosition, target->getPosition()) < 3) return true;
                if (!mapIsSquareAvailable(fromPosition)) return false;
            }
            
            return true;
        };
        
        if (distanceBetweenCreatures > min_distance || !checkObstacle())
        {
            auto comp = [](Node* lhs, Node* rhs) {return lhs->fvalue() > rhs->fvalue();};
            
            std::set<int> uniquenodes;
            std::priority_queue<Node*, std::vector<Node*>, decltype(comp)> opennodelist(comp);
            Node* chosenParent = nullptr;
            
            // Start
            std::vector<MovementDirection> walkOptions;
            mapGetEmptySquaresAround(walkOptions, creature->getPosition());

            bool searching = true;
            while(searching)
            {
                Node* currentnode = (opennodelist.empty() ? nullptr : opennodelist.top());
                
                if (currentnode)
                    opennodelist.pop();
                
                Vector3 fromPosition = (currentnode ? currentnode->position : creature->getPosition());
                Vector3 toPosition   = target->getPosition();
                
                walkOptions.clear();
                
                mapGetEmptySquaresAround(walkOptions, fromPosition);
                
                for(auto& pos : walkOptions)
                {
                    Node* newnode = new Node(Vector3::Move(fromPosition, pos), toPosition, Compass::get().isDiagonal(pos), currentnode);
                    
                    int uniquecell_id = getCellNumber(newnode->position.mX, newnode->position.mZ);
                    
                    if (uniquenodes.count(uniquecell_id) < 1 && fromPosition != toPosition)
                    {
                        uniquenodes.insert(uniquecell_id);
                        opennodelist.push(newnode);
                        if (newnode->hvalue <= 2) {
                            chosenParent = newnode;
                            searching = false;
                            break;
                        }
                    }
                    else
                        delete newnode;
                }
                
                if (opennodelist.empty())
                    break;
            }
        
            std::stack<Node*> path;
            
            while(chosenParent != nullptr)
            {
                path.push(chosenParent);
                chosenParent = chosenParent->parent;
            }
            
            if (!path.empty()){
                Node* at = path.top();
                path.pop();
                creaturemove(creature, RobotMath::getDirection(at->position, creature->getPosition()));
            }
            
            while(!opennodelist.empty())
            {
                delete opennodelist.top();
                opennodelist.pop();
            }
		}
		else if (distanceBetweenCreatures <= min_distance_approachable)
		{
			Vector3 toPosition = creature->getPosition();
			if (mapGetFirstEmptySquaresAround(toPosition, RobotMath::getDirection(target->getPosition(), creature->getPosition()), true)) {
				creaturemove(creature, RobotMath::getDirection(toPosition, creature->getPosition()));
			}
		}
     }
}

Item* GameLevel::createItem(const std::string& name)
{
    for (int i = 0; i < MAXITEMSPERLEVEL; i++)
    {
        if (items[i].getGUID() == 0)
        {
            if (gameworld->getFactory()->loadItem(name, items[i])){
                items[i].setGUID(GameWorld::getUniqueID());
                items[i].setPosition(Vector3());
                return &items[i];
            }
        }
    }
    return nullptr;
}

Item* GameLevel::createItem()
{
    for (int i = 0; i < MAXITEMSPERLEVEL; i++)
    {
        if (items[i].getGUID() == 0)
        {
            items[i].setGUID(GameWorld::getUniqueID());
            items[i].setPosition(Vector3());
            return &items[i];
        }
    }
    return nullptr;
}

void GameLevel::summonItem(Item* item)
{
    if (!item->isAmmo()){
    
        item->setAction(ItemAction::SUMMON);
        item->setOwner(0);
    
        for (int i = 0; i < MAXPLAYERSPERGAME; i++)
            if (players[i])
                gameworld->broadcastNewItem(players[i]->getGUID(), item);
        
        mapEnterTile(item);
    }
}

void GameLevel::unsummonItem(Item * item, bool reset)
{
    item->setAction(ItemAction::UNSUMMON);

    if (!item->isAmmo())
    {
        mapExitTile(item);
        for (int i = 0; i < MAXPLAYERSPERGAME; i++)
            if (players[i])
                gameworld->broadcastNewItem(players[i]->getGUID(), item);
    }
    
    if (reset)
        *item = Item();
}

const bool GameLevel::isWaveEmpty() const
{
    for (int i = 0; i < MAXCREATURESPERLEVEL; i++)
        if (creatures[i].getGUID() != 0)
            return false;
    return true;
}

bool GameLevel::nextWave(){
    
    currentLevel++;
    
    if (currentLevel <= MAXWAVES){
        
        if (currentLevel > 1){
            performance_active.points += POINTS_NEXTWAVE;
        
            for (int i = 0; i < MAXPLAYERSPERGAME; i++)
                if (players[i])
                    creatureAddExperience(players[i], POINTS_NEXTWAVE, true);
        }
        
        std::cerr << "Game: " << getUniqueGameID() << ".Next wave (" << currentLevel << "), total creatures: " << getMaxCreatures() << std::endl;
        
        for(auto& creature : mapData.respaw)
            creaturesToRespaw.push(std::make_pair("Soldier", Vector3(creature.first, 0, creature.second)));

		for (int i = 0; i < MAXPLAYERSPERGAME; i++)
			if (players[i])
				gameworld->broadcastGameStatus(players[i]->getGUID(), GameMessage::NEXTWAVE, *this, players[i]->getGUID());
    }
    
    return currentLevel <= MAXWAVES;
}

void GameLevel::cleanWave()
{
    for (int i = 0; i < MAXCREATURESPERLEVEL; i++)
        if (creatures[i].getGUID() != 0)
            unsummonBody(&creatures[i]);
}

void GameLevel::cleanAllObjects()
{
    std::queue<std::pair<std::string, Vector3>> empty;
    std::swap(creaturesToRespaw, empty);
    
    for(int i = 0; i < MAXCREATURESPERLEVEL; i++)
        if (creatures[i].getGUID() != 0)
            unsummonBody(&creatures[i]);
    
    for(int i = 0; i < MAXITEMSPERLEVEL; i++)
        if (items[i].getGUID() != 0)
            unsummonItem(&items[i], true);

    for(int i = 0; i < MAXPLAYERSPERGAME; i++)
        if (players[i]){
            unsummonBody(players[i]);
        }
    
}

Body* GameLevel::createCreature(const std::string& name, Vector3 position)
{
    for(int i = 0; i < MAXCREATURESPERLEVEL; i++)
    {
        if (creatures[i].getGUID() == 0)
        {
            if (gameworld->getFactory()->loadCreature(name, creatures[i])){
                
                creatures[i].setPosition(position);
                
                if (getValidPosition(&creatures[i], false))
                {
                    creatures[i].setGUID(GameWorld::getUniqueID());
                    creatures[i].setBodyType(BodyType::CREATURE);
                    creatures[i].setBodyColor(static_cast<BodyColor>(rand() % static_cast<int>(BodyColor::LAST)));
					creatures[i].setAttackSpeed(2 - ((getMediumLevel()*1.0)/10));
					creatures[i].setAttackSuccess(30 + ((getMediumLevel() - 1) * 3));
					creatures[i].setMinAttackDistance(BODYVIEWRANGE * .6 + (0.07 * (getMediumLevel()-1)));
                    
                    // Update Status
                    while (creatures[i].getLevel() < getMediumLevel())
                        creatureAddExperience(&creatures[i], creatures[i].getRemainingExperienceToLevel(), false);
                        
                    // Add Loot
                    Item* potion = createItem("Potion");
                    if (potion)
                    {
                        potion->setDropChance(80);
                        potion->setOptions(ItemFlags::TEMPORARY);
                        potion->setLifeTime(10000);
                        potion->setLifeChange((rand() % 100) + ((getMediumLevel() - 1) * 10));
                        creatures[i].mBackpack.addItem(potion);
                    }
                        
                    // Broadcast
                    summonBody(&creatures[i], true);
                        
                    return &creatures[i];
                }
            }
        }
    }
    
    std::cerr << "Failed to load creature: " << name << std::endl;
    
    return nullptr;
}

void GameLevel::summonBody(Body* body, bool broadcast)
{
    mapEnterTile(body);

	if (!broadcast)
		return;
    
    // Broadcast
    for(int i = 0; i < MAXPLAYERSPERGAME; i++)
        if (players[i] && players[i]->getGUID() != body->getGUID())
            gameworld->broadcastSpawn(players[i]->getGUID(), body);
}

void GameLevel::unsummonBody(Body* body)
{
    if (flagdata.holder() == body)
        creaturedropitem(body, flagdata.item);
    
    mapExitTile(body);
    
    for(int i = 0; i < MAXPLAYERSPERGAME; i++)
        if (players[i])
            gameworld->broadcastDespawn(players[i]->getGUID(), body);
    
    if (body->getBodyType() == BodyType::PLAYER)
        leaveGame(body);
    else
        *body = Body();
    
    if ((isLobbying() || isPlaying()) && (getPlayersCount() <= 0  || (isWaveEmpty() && !nextWave())))
        prepareEndGame();
}

const Body* GameLevel::hasTarget(Body *who) const
{
    return who->getTarget();
}

void GameLevel::creatureAddExperience  (Body* creature, uint16_t experience, bool broadcast)
{
    uint16_t oldLevel = creature->getLevel();
    
    if (creature->addExperience(experience)){
        creature->levelUp();
        creature->updateStats();
    }
    
    if (!broadcast)
        return;
    
    ExperienceData data;
    data.level = creature->getLevel();
    data.experience = creature->getExperience();
    gameworld->broadcastExperience(creature->getGUID(), data);
    
    if (oldLevel != creature->getLevel())
        creatureUpdateState(creature);
}

void GameLevel::requestShoot (Body* creature, void* data)
{
    auto shootdata = static_cast<ShootTargetData*>(data);
    creatureshoot(creature, shootdata->mLocal);
}


void GameLevel::creatureshoot(Body* gameobject, const Vector3& target)
{
    Item* bullet = createItem();
    
    if (bullet)
    {
        gameobject->attack();
        
        bullet->setName("Bullet");
        bullet->setOptions(ItemFlags::AMMUNITION);
        bullet->setOptions(ItemFlags::TEMPORARY);
        
        // Offset bullet instantiation
        Vector3 bulletposition = gameobject->getPosition();
        bulletposition.mX += 0.5f;
        bulletposition.mZ += 0.4f;
        
        bullet->setPosition(bulletposition);
        
        bullet->setOwner(gameobject->getGUID());
        bullet->setLifeTime(BULLETFRAMESALIVE);
        bullet->setAngle(RobotMath::getRadians(target, bullet->getPosition()));
		bullet->setLifeChange(gameobject->getAttack());
        
        summonItem(bullet);
        
        for (int i = 0; i < MAXPLAYERSPERGAME; i++)
            if (players[i])
                gameworld->broadcastShoot(players[i]->getGUID(), gameobject, target);
    }
    else
        std::cerr << "Could not create bullet." << std::endl;
}

void GameLevel::requestCardinalMove (Body* creature, void* data)
{
    auto movedata = static_cast<MovementCardinalData*>(data);

    creaturemove(creature, movedata->mToDirection);
}

void GameLevel::creaturemove(Body* gameobject, const MovementDirection& direction)
{
    Vector3 newPosition = Vector3::Move(gameobject->getPosition(), direction);
    
    if (mapIsSquareAvailable(newPosition)){
        
        gameobject->move(direction);
        
        mapExitTile(gameobject);
        
        gameobject->setPosition(newPosition);
        
        mapEnterTile(static_cast<Object*>(gameobject));
        
        for (int i = 0; i < MAXPLAYERSPERGAME; i++)
            if (players[i])
                    gameworld->broadcastBodyCardinal(players[i]->getGUID(), gameobject, direction);
        
        if (Vector3::Distance(getSafeZone(), gameobject->getPosition()) < 4 && flagdata.holder() == gameobject)
        {
            Vector3 flagposition = getFlagPosition();
            creaturedropitem(gameobject, flagdata.flagitem(), &flagposition);
			won = true;
            prepareEndGame();
        }
    }
    else{
        gameworld->broadcastBodyCoordinate(gameobject->getGUID(), gameobject, gameobject->getPosition(), true);
    }
}

void GameLevel::creaturemove(Body* gameobject, const Vector3& newposition)
{
    mapExitTile(gameobject);
    
    gameobject->setPosition(newposition);
    
    mapEnterTile(gameobject);
    
    for (int i = 0; i < MAXPLAYERSPERGAME; i++)
        if (players[i])
            gameworld->broadcastBodyCoordinate(players[i]->getGUID(), gameobject, newposition, true);
}

void GameLevel::requestCoordinateMove (Body* creature, void* data)
{
    auto movedata = static_cast<MovementCoordinateData*>(data);
    creaturemove(creature, movedata->mToPosition);
}

void GameLevel::creaturechangelife(Body* creature, Body* responsible, const uint16_t& value, bool aggressive, bool filter)
{
    uint16_t changevalue = value;
    
    if (filter && value > 0)
    {
        try {
            changevalue = static_cast<uint16_t>(round(value * ((rand() % 100 <= 50 ? 1 : -1) * (static_cast<float>(rand() % value)/100) + 1)));
        } catch (std::exception& error ) {
            changevalue = value;
            std::cerr << "Error occurred in calculating life change: " << error.what() << std::endl;
        }
        
        if (aggressive && !RobotMath::TryChance(creature->getDefense())) // Apply Armor
            changevalue -= creature->getArmor()/2 + rand() % creature->getArmor()/2;
    }
    
    auto tolife = creature->getHealth() + (aggressive ? -changevalue : changevalue);
    tolife = (tolife < 0 ? 0 : tolife);
    tolife = (tolife > creature->getMaxHealth() ? creature->getMaxHealth() : tolife);
    creature->setHealth(tolife);
    
    for (int i = 0; i < MAXPLAYERSPERGAME; i++)
        if (players[i])
            gameworld->broadcastBodyLife(players[i]->getGUID(), creature, responsible, changevalue, aggressive);
    
    if (creature->getHealth() <= 0)
        creaturedead(creature, responsible);
}
        
        
void GameLevel::creaturedead(Body* creature, Body* responsible)
{
    if (creature->getBodyType() == BodyType::CREATURE){
		performance_active.monsterskilled++;
		if (responsible) {
			uint16_t experiencegained = creature->getLevel() * currentLevel + 8;

			for (int i = 0; i < MAXPLAYERSPERGAME; i++)
				if (players[i])
					creatureAddExperience(players[i], ((players[i] == responsible) ? experiencegained : (experiencegained / 4)), true);

			performance_active.points += experiencegained;
		}
    }
    else
        performance_active.points += POINTS_EXPNOKILLER;
    
    for (int i = 0; i < MAXPLAYERSPERGAME; i++)
        if (players[i])
            gameworld->broadcastCreatureDead(players[i]->getGUID(), creature->getGUID(), (responsible ? responsible->getGUID() : 0));
    
    auto items = creature->mBackpack.getItems();
    for (uint8_t i = 0; i < creature->mBackpack.getMaxItems(); i++)
    {
        Item* item = items[i];
        if (item && RobotMath::TryChance(item->getDropChance()))
            creaturedropitem(creature, item);
    }
    
    unsummonBody(creature);
}


void GameLevel::newGame(GameWorld* world)
{

    gameworld = world;
    currentLevel = 0;
    currentUniqueID = ++UNIQUEIDIDENTIFIER;
    status = GameStatus::LOBBY;
    fpstimer.reset();
    respawtimer.reset();
	sincronizationTimer.reset();
    
    loadmap();
    
    std::cerr << "[Game ID " << getUniqueGameID() << "] created." << std::endl;
}

void GameLevel::prepareEndGame()
{
    std::cerr << "Level [" << getUniqueGameID() << "] ended." << std::endl;
    
    for(int i = 0; i < MAXPLAYERSPERGAME; i++)
        if (players[i]){
            performance_active.points += POINTS_PERPLAYER;
            creatureAddExperience(players[i], POINTS_PERPLAYER, true);
        }
    
    status = GameStatus::END;
    
    cleanAllObjects();
    
    gameworld->broadcastGameStatus(BROADCASTALL, GameMessage::GAMEOVER, *this, BROADCASTALL);
}

void GameLevel::endGame()
{
    currentLevel = currentUniqueID = playerscount = 0;
    status = GameStatus::UNDEFINED;
    performance_active = Performance();
}

void GameLevel::startGame()
{
    if (isLobbying())
    {
        for(int i = 0; i < MAXPLAYERSPERGAME; i++)
        {
            if (players[i])
            {
                // Add Player to the game
                players[i]->setPosition(getSafeZone());
                
                if (getValidPosition(players[i], false)){
                    players[i]->setHealth(players[i]->getMaxHealth());
                    
                    summonBody(players[i], false);

					// Tell player about his body
					gameworld->broadcastSpawn(players[i]->getGUID(), players[i]);
                }
                else
                {
                    leaveGame(players[i]);
                    unsummonBody(players[i]);
                    std::cerr << "Could not summon player " << players[i]->getName() << std::endl;
                }
            }
        }

		// Replicate world
		for (int i = 0; i < MAXPLAYERSPERGAME; i++)
			if (players[i])
				replicateWorld(players[i]);

		gameworld->broadcastGameStatus(BROADCASTALL, GameMessage::STARTGAME, *this, BROADCASTALL);
        
        status = GameStatus::PLAYING;
        createFlag();
        nextWave();   
    }
}

void    GameLevel::replicateWorld  (const Body* to)
{
    for (int i = 0; i < MAXCREATURESPERLEVEL; i++)
        if (creatures[i].getGUID() != 0)
            gameworld->broadcastSpawn(to->getGUID(), &creatures[i]);
    
    for (int i = 0; i < MAXPLAYERSPERGAME; i++)
        if (players[i] && players[i] != to)
            gameworld->broadcastSpawn(to->getGUID(), players[i]);
    
    for (int i = 0; i < MAXITEMSPERLEVEL; i++)
        if (items[i].getGUID() != 0)
            gameworld->broadcastNewItem(to->getGUID(), &items[i]);
}


bool GameLevel::inGame(const Body* body) const
{
    for (int i = 0; i < MAXPLAYERSPERGAME; i++)
        if (players[i] && players[i] == body)
            return true;
    
    for (int i = 0; i < MAXCREATURESPERLEVEL; i++)
        if (creatures[i].getGUID() != 0 && creatures[i].getGUID() == body->getGUID())
            return true;
    
    return false;
}

void GameLevel::loadmap()
{
    if (mapData.loaded)
        return;
    
    MapLoader::get().load(mapData);
    
    Item* blockade = createItem();
    blockade->setName("Block");
    
    for(auto& block : mapData.blockade)
        cells[block.first][block.second] = blockade;
}

void GameLevel::creaturepickitem (Body* creature, Item* item)
{
    if (creature->mBackpack.addItem(item))
    {
        if (item == flagdata.flagitem())
            pickFlag(creature);
        
        item->setOwner(creature->getGUID());
        item->setAction(ItemAction::PICK);
        
        for(int i = 0; i < MAXPLAYERSPERGAME; i++)
            if (players[i])
                gameworld->broadcastNewItem(players[i]->getGUID(), item);
        
        unsummonItem(item, false);
    }
}

void GameLevel::creatureconsumeitem (Body* creature, Item* item)
{
    if (item->getLifeChange() > 0)
        creaturechangelife(creature, nullptr, item->getLifeChange(), false, false);
    unsummonItem(item, true);
}

void GameLevel::creaturedropitem (Body* creature, Item* item, const Vector3* customlocation)
{
    creature->mBackpack.removeItem(item);
    
    if (item == flagdata.flagitem())
        dropFlag(creature);
    
    item->setAction(ItemAction::DROP);
    item->setOwner (creature->getGUID());
    
    if (customlocation)
        item->setPosition(*customlocation);
    else
        item->setPosition(creature->getPosition());
    
    for (int i = 0; i < MAXPLAYERSPERGAME; i++)
        if (players[i])
            gameworld->broadcastNewItem(players[i]->getGUID(), item);
    
    item->setOwner(0);
    
    summonItem(item);
}

void GameLevel::mapEnterTile(Object* object)
{
    // Determine which grid cell it's in.
    int cellX = static_cast<int>(floor(object->getPosition().mX));
    int cellZ = static_cast<int>(floor(object->getPosition().mZ));
    
    // Add to the front of list for the cell it's in.
    object->setMapPrev(nullptr);
    object->setMapNext(cells[cellX][cellZ]);
    cells[cellX][cellZ] = object;
    
    if (object->getMapNext())
        object->getMapNext()->setMapPrev(object);
    
    Item* item = mapHasInteractableItem(cellX, cellZ);
    if (item){
        if (object->getType() == ObjectType::ORGANISM)
        {
            Body* body = static_cast<Body*>(object);
            if (item->isPickable() && body->getBodyType() == BodyType::PLAYER)
                creaturepickitem(body, item);
            else if (item->isConsumable())
                creatureconsumeitem(body, item);
        }
    }
    
    //std::cerr << cellX << "|" << cellZ << " = " << object->getGUID() << std::endl;
}

void GameLevel::mapExitTile(Object* object)
{
    // Determine which grid cell it's in.
    int cellX = static_cast<int>(floor(object->getPosition().mX));
    int cellZ = static_cast<int>(floor(object->getPosition().mZ));
    
    // if it is first
    if (cells[cellX][cellZ] == object)
    {
        // assume that there is no other behind us
        if (object->getMapNext())
            object->getMapNext()->setMapPrev(nullptr);
        
        cells[cellX][cellZ] = object->getMapNext();
    }
    else
    {
        if (object->getMapPrev())
            object->getMapPrev()->setMapNext(object->getMapNext());
        if (object->getMapNext())
            object->getMapNext()->setMapPrev(object->getMapPrev());
    }
    
    object->setMapNext(nullptr);
    object->setMapPrev(nullptr);
}


Object* GameLevel::mapCell(const Vector3& pos)
{
    if (mapInBoundaries(pos))
        return cells[static_cast<int>(floor(pos.mX))][static_cast<int>(floor(pos.mZ))];
    return nullptr;
}

bool GameLevel::getValidPosition(Body * body, bool random)
{
    if (mapIsSquareAvailable(body->getPosition()))
        return true;
    
    if (random){
        std::vector<MovementDirection> positions;
        mapGetEmptySquaresAround(positions, body->getPosition());
        
        if (positions.size() > 0){
            body->setPosition(Vector3::Move(body->getPosition(),  positions[rand() % positions.size()]));
            return true;
        }
    }
    else
    {
        Vector3 newPosition = body->getPosition();
        if (mapGetFirstEmptySquaresAround(newPosition))
        {
            body->setPosition(newPosition);
            return true;
        }
    }
    
    body->setPosition(Vector3());
    return false;
}

Vector3 GameLevel::mapGetEmptySquare() const
{
    int x,z = 0;
    
    do
    {
        x = rand() % MAPSIZEX;
        z = rand() % MAPSIZEZ;
    } while(!mapIsSquareAvailable(x, z));
    
    return Vector3(static_cast<float>(x),0, static_cast<float>(z));
}

bool GameLevel::mapGetFirstEmptySquaresAround  (Vector3& from, const MovementDirection& exception, bool opposite) const
{
    Vector3 newPosition = from;
    
    if (opposite)
    {
        std::vector<MovementDirection> directions;
        
        // Get opposite directions
        Compass::get().getOppositeDirections(directions, exception);
        
        // Delete the ones not available
        std::vector<MovementDirection>::iterator iter;
        for (iter = directions.begin(); iter != directions.end(); ) {
            newPosition = Vector3::Move(from, static_cast<MovementDirection>((*iter)));
            if (mapIsSquareAvailable(newPosition)){
                from = newPosition;
                return true;
            }
            else
                iter = directions.erase(iter);
        }
    }
    else
    {
        for (int i = 0; i < static_cast<int>(MovementDirection::LAST); i++)
        {
            // We do not want to include the exception
            if (exception == static_cast<MovementDirection>(i)) continue;
            
            newPosition = Vector3::Move(from, static_cast<MovementDirection>(i));
            if (mapIsSquareAvailable(newPosition))
            {
                from = newPosition;
                return true;
            }
        }
    }
    return false;
}

void GameLevel::mapGetEmptySquaresAround  (std::vector<MovementDirection>& directions, const Vector3& from, const MovementDirection& exception, bool opposite) const
{
    Vector3 newPosition = from;
    
    if (opposite)
    {
        // Get opposite directions
        Compass::get().getOppositeDirections(directions, exception);
        
        // Delete the ones not available
        std::vector<MovementDirection>::iterator iter;
        for (iter = directions.begin(); iter != directions.end(); ) {
            newPosition = Vector3::Move(from, static_cast<MovementDirection>((*iter)));
            if (!mapIsSquareAvailable(newPosition))
                iter = directions.erase(iter);
            else
                ++iter;
        }
    }
    else
    {
        for (int i = 0; i < static_cast<int>(MovementDirection::LAST); i++)
        {
            // We do not want to include the exception
            if (exception == static_cast<MovementDirection>(i)) continue;
            
            newPosition = Vector3::Move(from, static_cast<MovementDirection>(i));
            if (mapIsSquareAvailable(newPosition))
                directions.push_back(static_cast<MovementDirection>(i));
        }
    }
}

void GameLevel::mapGetEmptySquaresAround  (std::vector<MovementDirection>& directions, const Body* body, const MovementDirection& exception, bool opposite) const
{
    mapGetEmptySquaresAround(directions, body->getPosition(), exception, opposite);
}
    
bool    GameLevel::mapHasBloackableObject(const int& x, const int& z) const
{
    Object* object = cells[x][z];
    if (object)
    {
        switch(object->getType())
        {
            case ObjectType::ORGANISM: return true;
            case ObjectType::ITEM:     return static_cast<Item*>(object)->isBlockable();
            default: return false;
        }
    }
    return false;
}

Item*   GameLevel::mapHasInteractableItem(const int& x, const int& z) const
{
    Object* object = cells[x][z];
    if (object)
    {
        next:
        
        if (object->getType() == ObjectType::ITEM){
            Item* item = static_cast<Item*>(object);
            if ((item->isConsumable() || item->isPickable()) && !item->isAmmo())
                return item;
        }
        
        object = object->getMapNext();
        if (object) goto next;
    }
    return nullptr;
}


bool    GameLevel::mapIsSquareAvailable(const int& x, const int& z) const
{
    return mapInBoundaries(x,z) && !mapHasBloackableObject(x,z);
}
    
const Body*   GameLevel::mapGetClosestBody(const Body* from,  const BodyType& onlybodytype) const
{
    return mapGetClosestBody(from->getPosition(), onlybodytype, from);
}
    
const Body*   GameLevel::mapGetClosestBody(const Vector3&   frompos,  const BodyType& onlybodytype,  const Body* from) const
{
    const Body* target     = nullptr;
    double  targetdistance = 0;
    
    if (onlybodytype == BodyType::PLAYER){
        for (int i = 0; i < MAXPLAYERSPERGAME; i++)
        {
            if (players[i])
            {
                double distancebetweenbodies = Vector3::Distance(players[i]->getPosition(), frompos);
                if (distancebetweenbodies > BODYVIEWRANGE) continue;
                
                if (!target || distancebetweenbodies < targetdistance){
                    target         = players[i];
                    targetdistance = distancebetweenbodies;
                }
            }
        }
    }else if (onlybodytype == BodyType::CREATURE)
    {
        for (int i = 0; i < MAXCREATURESPERLEVEL; i++)
        {
            if (creatures[i].getGUID() != 0)
            {
                double distancebetweenbodies = Vector3::Distance(creatures[i].getPosition(), frompos);
                if (distancebetweenbodies > BODYVIEWRANGE) continue;
                
                if (!target || distancebetweenbodies < targetdistance){
                    target         = &creatures[i];
                    targetdistance = distancebetweenbodies;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < MAXCREATURESPERLEVEL; i++)
        {
            if (creatures[i].getGUID() != 0)
            {
                double distancebetweenbodies = Vector3::Distance(creatures[i].getPosition(), frompos);
                if (distancebetweenbodies > BODYVIEWRANGE) continue;
                
                if (!target || distancebetweenbodies < targetdistance){
                    target         = &creatures[i];
                    targetdistance = distancebetweenbodies;
                }
            }
        }
        
        for (int i = 0; i < MAXPLAYERSPERGAME; i++)
        {
            if (players[i])
            {
                double distancebetweenbodies = Vector3::Distance(players[i]->getPosition(), frompos);
                if (distancebetweenbodies > BODYVIEWRANGE) continue;
                
                if (!target || distancebetweenbodies < targetdistance){
                    target         = players[i];
                    targetdistance = distancebetweenbodies;
                }
            }
        }
    }
    
    return target;
}

void  GameLevel::createFlag()
{
    Item* flag = createItem("Flag");
    
    if (flag)
    {
        flag->setPosition(getFlagPosition());
        flagdata.load(flag);
        flagdata.pick(nullptr);
        summonItem(flag);
    }
    
    assert(flag);
}

void  GameLevel::pickFlag(Body* player)
{
    if (flagdata.item->getPosition() == getFlagPosition())
    {
        uint16_t gainedpoints = POINTS_PICKFLAG * currentLevel;
        
        creatureAddExperience(player, gainedpoints, true);
        performance_active.points += gainedpoints;
        
        nextWave();
    }
    
    flagdata.pick(player);
}

void  GameLevel::dropFlag(Body* player)
{
    flagdata.pick(nullptr);
}

bool GameLevel::enterGame(Body* player)
{
    for (uint8_t i = 0; i < MAXPLAYERSPERGAME; i++){
        if (players[i] == nullptr){
            
            players[i] = player;
            
            std::cerr << player->getName() << " joined game " << currentUniqueID << std::endl;
            
            playerscount++;
            
			// This iteration occures only once, thus it is safe
			gameworld->broadcastGameStatus(BROADCASTALL, GameMessage::JOINGAME, *this, player->getGUID(), player->getName());

			// send current list of players to recently joined player
			for (int i = 0; i < MAXPLAYERSPERGAME; i++)
				if (players[i] && players[i] != player)
					gameworld->broadcastGameStatus(player->getGUID(), GameMessage::JOINGAME, *this, players[i]->getGUID(), players[i]->getName());
            
            return true;
        }
    }
    
    return false;
}

void GameLevel::leaveGame(Body* player)
{
	gameworld->broadcastGameStatus(BROADCASTALL, GameMessage::LEAVEGAME, *this, player->getGUID());
	gameworld->broadcastGameStatus(player->getGUID(), GameMessage::GAMEOVER, *this, player->getGUID());
    
    player->setHealth(player->getMaxHealth());
    
    if (status == GameStatus::LOBBY)
    {   
        for (uint8_t i = 0; i < MAXPLAYERSPERGAME; i++){
            if (players[i] && players[i] == player){
                players[i] = nullptr;
                playerscount--;
            }
        }
        
		if (!players[0])
		{
			for (uint8_t i = 1; i < MAXPLAYERSPERGAME - 1; i++) {
				if (players[i])
				{
					players[0] = players[i];
					players[i] = nullptr;
					return;
				}
			}
		}
    }
    else
    {
        for (uint8_t i = 0; i < MAXPLAYERSPERGAME; i++){
            if (players[i] == player){
                players[i] = nullptr;
                playerscount--;
                return;
            }
        }
    }

	gameworld->forceSave(player->getGUID());
}

bool GameLevel::isLobbyManager(Body* player) const
{
    return players[0] == player;
}

Body* GameLevel::getBody(uint32_t guid, BodyType type)
{
    // Little 'cache' if we want only the player
    if (type == BodyType::PLAYER){
        for (int i = 0; i < MAXPLAYERSPERGAME; i++)
            if (players[i] && players[i]->getGUID() == guid)
                return players[i];
    }
    
    for (int i = 0; i < MAXPLAYERSPERGAME; i++)
        if (players[i] && players[i]->getGUID() == guid)
            return players[i];

    for (int i = 0; i < MAXCREATURESPERLEVEL; i++)
        if (creatures[i].getGUID() == guid)
            return &creatures[i];
    
    return nullptr;
}

const uint16_t GameLevel::getMediumLevel()        const
{
    uint16_t mediumLevel = 0;
    uint16_t totalPlayers = 0;
    for (int i = 0; i < MAXPLAYERSPERGAME; i++)
        if (players[i]){
            mediumLevel += players[i]->getLevel();
            totalPlayers++;
        }
    return (totalPlayers > 0 ? static_cast<uint16_t>(round(mediumLevel/totalPlayers)) : 1);
    
}

void GameLevel::creatureUpdateState(Body * player)
{
    gameworld->broadcastSpawn(player->getGUID(), player); // Update player status after the game
}
