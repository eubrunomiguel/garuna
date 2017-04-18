//
//  Library.cpp
//  server
//
//  Created by Bruno Macedo Miguel on 12/13/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#include "Factory.hpp"
#include "Account.hpp"
#include "Player.hpp"
#include "GameWorld.hpp"
#include "Database.hpp"
#include <sstream>
#include <iostream>
#include <cassert>

uint8_t Factory::numinstantiated = 0;

Factory::Factory(GameWorld& gameworld, DatabaseIO& database) : mGameWorld(gameworld), mDatabase(database)
{
    assert(numinstantiated < MAXWORLDS);
    numinstantiated++;
    
    // Create our creature list database
    Body* body = new Body;
    body->setName("Soldier");
    body->setHealth(20);
    body->setMaxHealth(20);
    body->setStamina(50);
    body->setMaxStamina(50);
    body->setAttack(40);
    body->setArmor(10);
    body->setDefense(5);
    body->setSpeed(5);
    body->setLevel(1);
    mCreatureDatabase.insert(body);
    
    // Item database
    Item* flag = new Item;
    flag->setOwner(0);
    flag->setName("Flag");
    flag->setSlot(ItemSlots::NONE);
    flag->setAngle(0);
    flag->setAction(ItemAction::NONE);
    flag->setOptions(ItemFlags::PICKABLE);
    flag->setDropChance(100);
    mItemDatabase.insert(flag);
    
    
    Item* health = new Item;
    health->setOwner(0);
    health->setName("Potion");
    health->setSlot(ItemSlots::CONTAINER);
    health->setAngle(0);
    health->setAction(ItemAction::NONE);
    health->setOptions(ItemFlags::CONSUMABLE);
    health->setDropChance(0);
    mItemDatabase.insert(health);
}

bool Factory::accountLogin(Account& account, const AccountData& accountdata, MessageOutput& output)
{
    if (mDatabase.loadAccount(accountdata.account, accountdata.password, account))
    {
        if (mDatabase.loadPlayer(account.getAccountID(), account.getPlayer()))
        {
            output = MessageOutput::ACCOUNT_CONNECTED;
            mGameWorld.registerPlayer(account.getPlayer());
            return true;
        }
        else{
            output = MessageOutput::ACCOUNT_PLAYERERROR;
        }
    }
    else
    {
        output = MessageOutput::ACCOUNT_INCORRECT;
    }
    
    return false;
}

void Factory::accountLogout(Account& account)
{
    unloadPlayer(account.getPlayer());
    account.disconnect();
    mDatabase.save(account);
    account = Account();
}


void Factory::unloadPlayer(Player& player)
{
    player.disconnect();
	savePlayer(player);
    mGameWorld.leaveGame(&player.getPlayerBody());
    mGameWorld.unregisterPlayer(player);
    player = Player();
}

bool Factory::savePlayer(const Player& player) const
{
	return mDatabase.save(player);
}

void Factory::uploadActionToGameWorld(DataBus* action)
{
    mGameWorld.addAction(action);
}

bool Factory::loadItem(const std::string& name, Item& item) const
{
    auto search = [&name](const Item* i) { return i->getName() == name; };
    auto it = std::find_if(mItemDatabase.begin(), mItemDatabase.end(), search);
    
    if(it != mItemDatabase.end())
    {
        uint32_t guid = item.getGUID();
        
        item = *(*it);
        
        item.setGUID(guid);
        return true;
    }
    else
        std::cerr << "Item " << item.getName() << " could not be found." << std::endl;
    
    return false;
}

bool Factory::loadCreature(const std::string& name, Body& newCreature) const
{
    auto search = [&name](const Body* body) { return body->getName() == name; };
    auto it = std::find_if(mCreatureDatabase.begin(), mCreatureDatabase.end(), search);
    if(it != mCreatureDatabase.end())
    {
        newCreature.setLevel((*it)->getLevel());
        newCreature.setExperience((*it)->getExperience());
        newCreature.setName((*it)->getName());
        newCreature.setHealth((*it)->getHealth());
        newCreature.setMaxHealth((*it)->getMaxHealth());
        newCreature.setStamina((*it)->getStamina());
        newCreature.setMaxStamina((*it)->getMaxStamina());
        newCreature.setAttack((*it)->getAttack());
        newCreature.setArmor((*it)->getArmor());
        newCreature.setDefense((*it)->getDefense());
        newCreature.setSpeed((*it)->getMovementSpeed());
        return true;
    }
    else
        std::cerr << "Could not load creature " << name << std::endl;
    return false;
}

void Factory::sendGameList(const uint32_t& toplayer)
{
    for (auto game : mGameWorld.getGames())
    {
		if (!game.second->isLobbying())
			continue;

        DataBus* tempData         = createAction();
        tempData->messageType     = MessageHeader::GAME;
        tempData->messageFrom     = 0;
        tempData->messageTo       = toplayer;
        auto tempLevel            = tempData->create<GameData>();
        tempLevel->guid           = 0;
        tempLevel->message        = GameMessage::NEWGAME;
        tempLevel->currentlevel   = static_cast<uint8_t>(game.second->getCurrentLevel());
        tempLevel->players        = static_cast<uint8_t>(game.second->getPlayersCount());
        tempLevel->uniqueid       = game.second->getUniqueGameID();  
        addActionToBeBroadcasted(tempData);
    }
}
