//
//  GameWorld.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 12/11/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef GameWorld_hpp
#define GameWorld_hpp

#include <cstdlib>
#include <cassert>
#include <map>
#include <queue>
#include <iostream>
#include "DynamicPool.hpp"

#include "GameLevel.hpp"
#include "Consts.hpp"
#include "Item.hpp"

class Factory;
class Player;

class GameWorld{
public:
    
    GameWorld()
    {
        assert(numinstantiated < MAXWORLDS);
        numinstantiated++;
    }
    
    void  loadFactory(Factory* fac) {_factory = fac;}
    
    void  update();
    
    void  addAction(DataBus* action) { _incomingactions.push(action);}
    
    GameLevel* isInGame(const Body* body);

    const std::map <uint32_t, GameLevel*>& getGames(){return _games;}
    
	void  broadcastGameStatus	   (uint32_t to, GameMessage message, const GameLevel& game, uint32_t who, std::string who_name);
    void  broadcastGameStatus      (uint32_t to, GameMessage message, const GameLevel& game, uint32_t who);
    void  broadcastBodyCoordinate  (const uint32_t& to, const Body*, const Vector3&, const bool&);
    void  broadcastBodyCardinal    (const uint32_t& to, const Body*, const MovementDirection&);
    void  broadcastBodyAttacker    (const uint32_t& to, const Body* attacker, const Body* target);
    void  broadcastBodyLife        (const uint32_t& to, const Body*, const Body* responsible, const uint16_t&, const bool&);
    void  broadcastShoot           (const uint32_t& to, const Body* shooter, const Vector3& target);
    void  broadcastCreatureDead    (uint32_t to, uint32_t who, uint32_t by);
    void  broadcastNewItem         (const uint32_t& to, const Item* item);
    void  broadcastDespawn         (const uint32_t& to, const Body* body);
    void  broadcastWarningMessage  (const uint32_t& tp, const MessageOutput&);
    void  broadcastExperience      (const uint32_t& to, const ExperienceData&);
    void  broadcastSpawn           (const uint32_t& to, Body* body); // Non-Const since we send a copy pointer to ActionData
    
    static uint32_t getUniqueID() {return ++entityid;}
    
    const Factory* getFactory() const {return _factory;}
    
    void  registerPlayer(Player&);
    void  unregisterPlayer(Player&);
	void  forceSave(uint32_t playerguid);
    
    void  leaveGame(Body*);
    
private:
    
    void  gameManager(Body*, const void*);
    void  startGame(Body*);
    void  joinGame(const uint32_t&, Body*);
    void  newGame(Body*);
    void  endGame(const uint32_t&);
    
    void processInput();
    void updateGame();
    
    
    Factory* _factory;
    
    std::vector<GameLevel*>             _gamestodestroy;
    std::map <uint32_t, GameLevel*>     _games;
    DynamicPool<GameLevel>              _gamePool;
    
    // to process actions
    std::queue<DataBus*> _incomingactions;
    
    // Keep track of our bodies in the world
    std::map <uint32_t, Player*> _players;
    
    // Unique Entity ID
    static uint32_t entityid;
    
    // Control number instanciated
    static uint8_t  numinstantiated;
};

#endif /* GameWorld_hpp */
