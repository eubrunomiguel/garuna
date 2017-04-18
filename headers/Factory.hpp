//
//  Factory
//  server
//
//  Created by Bruno Macedo Miguel on 12/13/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef Library_hpp
#define Library_hpp

#include <set>
#include <cstdlib>
#include <cassert>
#include <queue>
#include <iostream>
#include <string>
#include "Consts.hpp"
#include "DynamicPool.hpp"
#include "NetworkData.hpp"

class Player;
class Account;
class GameWorld;
class Body;
class Item;
class DatabaseIO;


// class to indirect connect network to gameworld

class Factory
{
public:
    Factory(GameWorld& gameworld, DatabaseIO& database);
    
    bool    accountLogin(Account&, const AccountData&, MessageOutput&);
    void    accountLogout(Account&);
    
	bool    savePlayer	(const Player& player) const;
    bool    loadCreature(const std::string&, Body&) const;
    bool    loadItem    (const std::string& name, Item& item) const;
    
    DataBus* createAction() { return mMessageTranslatorPool.create(); }
    
    void        recycleAction(DataBus& action)
    {
        action.reset();
        mMessageTranslatorPool.destroy(action);
    }
    
    void uploadActionToGameWorld(DataBus* action);
    
    void addActionToBeBroadcasted(DataBus* data)
    {
        mToBroadcastMessages.push(data);
    }
    
    DataBus* getActionToBeBroadcasted()
    {
        DataBus* temp = nullptr;
        if (!mToBroadcastMessages.empty())
        {
            temp = mToBroadcastMessages.front();
            mToBroadcastMessages.pop();
        }
        return temp;
    }
    
    void sendGameList(const uint32_t& toplayer);
    
private:
    GameWorld&         mGameWorld;
    DatabaseIO&        mDatabase;
    
    void    unloadPlayer (Player&);
    
    // We will store an object from now to make copy later, remember that we will be returning from the database
    std::set<Body*>      mCreatureDatabase;
    std::set<Item*>      mItemDatabase;
    
    std::queue<DataBus*> mToBroadcastMessages;
    
    DynamicPool<DataBus> mMessageTranslatorPool;
    
    static uint8_t       numinstantiated;
};

#endif /* Factory */
