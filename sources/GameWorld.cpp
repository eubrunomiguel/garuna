//
//  GameWorld.cpp
//  server
//
//  Created by Bruno Macedo Miguel on 12/11/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#include "GameWorld.hpp"
#include "Factory.hpp"
#include "Player.hpp"

uint8_t GameWorld::numinstantiated = 0;
uint32_t GameWorld::entityid = 0;

void GameWorld::update()
{
    processInput();
    updateGame();
}

GameLevel* GameWorld::isInGame(const Body* body)
{
    for(auto& game : _games)
        if (game.second->inGame(body))
            return game.second;
    
    return nullptr;
}

void  GameWorld::registerPlayer(Player& player)
{
	auto& body = player.getPlayerBody();
    body.setGUID(getUniqueID());
    body.setBodyType(BodyType::PLAYER);
    _players[body.getGUID()] = &player;
}

void  GameWorld::unregisterPlayer(Player& player)
{
	auto& body = player.getPlayerBody();
	_players[body.getGUID()] = nullptr;
	_players.erase(body.getGUID());
}

void  GameWorld::forceSave(uint32_t playerguid)
{
	if (_players.count(playerguid) == 1) 
		_factory->savePlayer(*_players[playerguid]);
}


void GameWorld::processInput()
{
    if (!_incomingactions.empty())
    {
        DataBus* action     = _incomingactions.front();
        
        auto it = _players.find(action->messageFrom);
        
		if (it != _players.end())
		{
			if (action->messageType == MessageHeader::GAME)
			{
				gameManager(&it->second->getPlayerBody(), action->get());
			}
			else
			{
				auto game = isInGame(&it->second->getPlayerBody());
				if (game)
				{
					game->processWorldAction(action->messageType, &it->second->getPlayerBody(), action->get());
				}
			}
		}
        
        _factory->recycleAction(*action);
        
        
        _incomingactions.pop();
    }
}

void GameWorld::updateGame()
{
    for (auto iter = _games.begin(); iter != _games.end();) {
        if (iter->second->isPlaying()){
            iter->second->update();
            iter++;
        }
        else if(iter->second->isOver())
        {
            (*iter).second->endGame();
            iter = _games.erase(iter);
            _gamePool.destroy(*(*iter).second);
        }
        else
            iter++;
    }
}


void GameWorld::newGame(Body* player)
{
    assert(_factory);
    
    auto game = isInGame(player);
    // Player is not on any game
    if (!game)
    {
        
        // Create Game
        GameLevel* newGame = _gamePool.create();
        
        // Install Game
        newGame->newGame(this);
        _games.insert(std::make_pair(newGame->getUniqueGameID(), newGame));
        
        // Broadcast new game information
        broadcastGameStatus(BROADCASTALL, GameMessage::NEWGAME, *newGame, player->getGUID());
        
        // Add to lobby
        joinGame(newGame->getUniqueGameID(), player);
    }
    else
    {
        broadcastWarningMessage(player->getGUID(), MessageOutput::PLAYER_ALREADYINLEVEL);
        
        std::cerr << player->getName() << " is already in a game." << std::endl;
    }
}

void GameWorld::startGame(Body* player)
{
    auto game = isInGame(player);
    
    // Player is not on any game
    if (game)
    {
        if (game->isLobbyManager(player))
            game->startGame();
        else
           broadcastWarningMessage(player->getGUID(), MessageOutput::LEVEL_NOTMASTER);
    }
    else
    {
        broadcastWarningMessage(player->getGUID(), MessageOutput::LEVEL_NOTFOUND);
    }
}

void GameWorld::leaveGame(Body * player)
{
    auto game = isInGame(player);
    if (game)
    {
        game->unsummonBody(player);
		forceSave(player->getGUID());
    }
}

void GameWorld::joinGame(const uint32_t& id, Body* player)
{
    auto game = isInGame(player);
    
    // Player is not on any game
    if (!game)
    {
        auto it = _games.find(id);
        if (it != _games.end())
        {
            if (!it->second->inGame(player))
            {
                // Add to lobby
                if (it->second->isLobbying()){
					if (!it->second->enterGame(player)) {
						broadcastWarningMessage(player->getGUID(), MessageOutput::LEVEL_FULL);
						std::cerr << player->getName() << " cannot join the game. It is full." << std::endl;
					}
				}
				else
				{
					broadcastWarningMessage(player->getGUID(), MessageOutput::LEVEL_NOTFOUND);
					std::cerr << player->getName() << " cannot join the game. It is already playing." << std::endl;
				}
            }
            else
            {
                broadcastWarningMessage(player->getGUID(), MessageOutput::PLAYER_ALREADYINLEVEL);
            }
        }
        else
        {
            broadcastWarningMessage(player->getGUID(), MessageOutput::LEVEL_NOTFOUND);
            std::cerr << "Level not found." << std::endl;
        }
    }
    else
    {
        broadcastWarningMessage(player->getGUID(), MessageOutput::PLAYER_ALREADYINLEVEL);
        
        std::cerr << player->getName() << " is already in a level." << std::endl;
    }
}

void GameWorld::endGame(const uint32_t& levelid)
{
    auto it = _games.find(levelid);
    if (it != _games.end())
    {
        // Reset Level
        it->second->prepareEndGame();
    }
}

void GameWorld::gameManager(Body* gameobject, const void* object)
{
    const auto* data = static_cast<const GameData*>(object);
    switch (data->message) {
        case GameMessage::NEWGAME:
            newGame(gameobject);
            break;
        case GameMessage::JOINGAME:
            joinGame(data->uniqueid, gameobject);
            break;
        case GameMessage::GAMEOVER:
            std::cerr << "gameover" << std::endl;
            break;
        case GameMessage::STARTGAME:
            startGame(gameobject);
            break;
        case GameMessage::LEAVEGAME:
            leaveGame(gameobject);
            break;
        default:
            break;
    }
}

void GameWorld::broadcastGameStatus(uint32_t to, GameMessage message, const GameLevel& game, uint32_t who)
{
	broadcastGameStatus(std::move(to), std::move(message), game, std::move(who), "");
}

void GameWorld::broadcastGameStatus(uint32_t to, GameMessage message, const GameLevel& game, uint32_t who, std::string who_name)
{
	DataBus* tempData = _factory->createAction();
	tempData->messageType = MessageHeader::GAME;
	tempData->messageTo = std::move(to);
	auto tempLevel = tempData->create<GameData>();
	tempLevel->guid = std::move(who);
	tempLevel->name = std::move(who_name);
	tempLevel->message = std::move(message);
	tempLevel->uniqueid = game.getUniqueGameID();
	tempLevel->currentlevel = static_cast<uint8_t>(game.getCurrentLevel());
	tempLevel->players = static_cast<uint8_t>(game.getPlayersCount());
	tempLevel->points = game.getPerformance().points;
	tempLevel->time = game.getPerformance().time;
	tempLevel->monsterskilled = game.getPerformance().monsterskilled;
	tempLevel->won = game.hasWin();

	_factory->addActionToBeBroadcasted(tempData);
}

void GameWorld::broadcastSpawn   (const uint32_t& to, Body* body)
{
    DataBus* newAction              = _factory->createAction();
    newAction->messageType          = MessageHeader::BODY_SPAWN;
    newAction->messageTo            = to;
    newAction->copy(static_cast<void*>(body));
    _factory->addActionToBeBroadcasted(newAction);
}

void GameWorld::broadcastDespawn (const uint32_t& to, const Body* body)
{
    DataBus* newAction              = _factory->createAction();
    newAction->messageType          = MessageHeader::BODY_DESPAWN;
    newAction->messageTo            = to;
    auto     despawndata            = newAction->create<BodyDespawnData>();
    despawndata->guid               = body->getGUID();
    _factory->addActionToBeBroadcasted(newAction);
}

void  GameWorld::broadcastBodyAttacker   (const uint32_t& to, const Body* attacker, const Body* target)
{
    DataBus* tempData           = _factory->createAction();
    tempData->messageTo         = to;
    tempData->messageFrom       = attacker->getGUID();
    tempData->messageType       = MessageHeader::CREATURE_SETTARGET;
    
    auto tempMovement           = tempData->create<TargetData>();
    tempMovement->attacker      = attacker->getGUID();
    tempMovement->targetguid    = (target ? target->getGUID() : 0);
    
    _factory->addActionToBeBroadcasted(tempData);
}

void  GameWorld::broadcastCreatureDead    (uint32_t to, uint32_t who, uint32_t by)
{
    DataBus* tempData           = _factory->createAction();
    tempData->messageTo         = std::move(to);
    tempData->messageType       = MessageHeader::CREATURE_DEAD;
    
    auto temp           = tempData->create<CreatureDeadData>();
    temp->creatureguid  = std::move(who);
    temp->killerguid    = std::move(by);

    _factory->addActionToBeBroadcasted(tempData);
}

void GameWorld::broadcastNewItem(const uint32_t& to, const Item* item){
    DataBus* tempData           = _factory->createAction();
    tempData->messageTo         = to;
    tempData->messageType       = MessageHeader::ITEM;
    
    auto itemData               = tempData->create<Item>();
    *itemData                   = *item;
    _factory->addActionToBeBroadcasted(tempData);
}

void  GameWorld::broadcastBodyLife(const uint32_t& to, const Body* body, const Body* responsible, const uint16_t& value, const bool& aggresive)
{
    DataBus* tempData           = _factory->createAction();
    tempData->messageTo         = to;
    tempData->messageFrom       = body->getGUID();
    tempData->messageType       = MessageHeader::LIFECHANGE;
    
    auto lifeData               = tempData->create<LifeChangeData>();
    lifeData->aggresive         = aggresive;
    lifeData->responsible       = (responsible ? responsible->getGUID() : 0);
    lifeData->target            = body->getGUID();
    lifeData->value             = value;
    _factory->addActionToBeBroadcasted(tempData);
}

void  GameWorld::broadcastBodyCoordinate  (const uint32_t& to, const Body* body, const Vector3& topos, const bool& instant)
{
    DataBus* tempData           = _factory->createAction();
    tempData->messageTo         = to;
    tempData->messageFrom       = body->getGUID();
    tempData->messageType       = MessageHeader::MOVEMENT_COORDINATE;
    
    auto tempMovement           = tempData->create<MovementCoordinateData>();
    tempMovement->mGUID     = body->getGUID();
    tempMovement->mToPosition   = topos;
    tempMovement->mInstant      = instant;
    _factory->addActionToBeBroadcasted(tempData);
}

void  GameWorld::broadcastBodyCardinal  (const uint32_t& to, const Body* body, const MovementDirection& direction)
{
    DataBus* tempData           = _factory->createAction();
    tempData->messageTo         = to;
    tempData->messageFrom       = body->getGUID();
    tempData->messageType       = MessageHeader::MOVEMENT_CARDINAL;
    
    auto tempMovement           = tempData->create<MovementCardinalData>();
    tempMovement->mGUID         = body->getGUID();
    tempMovement->mToDirection  = direction;
    _factory->addActionToBeBroadcasted(tempData);
}

void  GameWorld::broadcastShoot(const uint32_t& to, const Body* shooter, const Vector3& target)
{
    DataBus* tempData           = _factory->createAction();
    tempData->messageTo         = to;
    tempData->messageFrom       = shooter->getGUID();
    tempData->messageType       = MessageHeader::SHOOT;
    
    auto tempMovement           = tempData->create<ShootTargetData>();
    tempMovement->mGUID         = shooter->getGUID();
    tempMovement->mLocal        = target;
    _factory->addActionToBeBroadcasted(tempData);
}

void GameWorld::broadcastWarningMessage(const uint32_t& guid, const MessageOutput& message)
{
    // Update Data
    DataBus* tempData           = _factory->createAction();
    tempData->messageTo         = guid;
    tempData->messageType       = MessageHeader::MESSAGE_OUTPUT;
    
    auto lifeData               = tempData->create<MessageOutput>();
    *lifeData                   = message;
    _factory->addActionToBeBroadcasted(tempData);
}

void GameWorld::broadcastExperience(const uint32_t& to, const ExperienceData& data)
{
    DataBus* tempData           = _factory->createAction();
    tempData->messageTo         = to;
    tempData->messageType       = MessageHeader::EXPERIENCE;
    
    auto experience             = tempData->create<ExperienceData>();
    *experience                 = data;
    _factory->addActionToBeBroadcasted(tempData);
}

