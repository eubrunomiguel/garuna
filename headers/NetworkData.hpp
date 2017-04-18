//
//  NetworkData.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 1/26/17.
//  Copyright © 2017 d2server. All rights reserved.
//

#ifndef NetworkData_hpp
#define NetworkData_hpp

#include "Math.hpp"
#include <string>
#include <cassert>

class DataBus
{
public:
    DataBus()
    {
        reset();
    }
    
    template<class T>
    T* create()
    {
        assert(sizeof(T) < sizeof(dataspace));
        
        return new(datapointer) T();
    }
    
    void copy(void* data)
    {
        datapointer = data;
    }
    
    void* get()
    {
        return datapointer;
    }
    
    void reset()
    {
		memset(dataspace, '\0', MAXDATABUSSIZE);
        datapointer = dataspace;
        messageTo = messageFrom = 0;
        messageType = MessageHeader::UNDEFINED;
    }
    
    void* datapointer;
    char  dataspace[MAXDATABUSSIZE];
    
    uint32_t      messageTo;
    uint32_t      messageFrom;
    MessageHeader messageType;
};

class MovementCardinalData
{
public:
    void reset()
    {
        mToDirection = MovementDirection::NONE;
        mGUID = 0;
    }
    uint32_t             mGUID;
    MovementDirection    mToDirection;
};

class MovementCoordinateData
{
public:
    void reset()
    {
        mToPosition = Vector3::Zero();
        mGUID   = 0;
        mInstant    = false;
    }
    uint32_t   mGUID;
    Vector3    mToPosition;
    bool       mInstant;
};

class MovementRotationData
{
public:
    void reset()
    {
        mRotation = Vector3::Zero();
        mGUID = 0;
    }
    uint32_t   mGUID;
    Vector3    mRotation;
};

class ShootTargetData
{
public:
    void reset()
    {
        mLocal = Vector3::Zero();
        mGUID = 0;
    }
    uint32_t   mGUID;
    Vector3    mLocal;
};

class LifeChangeData
{
public:
    void reset()
    {
        responsible = target = value = aggresive = 0;
    }
    
    uint32_t responsible;
    uint32_t target;
    uint16_t value;
    bool aggresive;
};

struct PlayerData
{
public:
    PlayerData() : uint32_t(0), name(""){}
    PlayerData(uint32_t i, std::string n) : uint32_t(i), name(n){}
    uint32_t uint32_t;
    std::string name;
};

struct BodyDespawnData
{
public:
    BodyDespawnData() : guid(0){}
    BodyDespawnData(uint32_t body) : guid(body){}
    uint32_t guid;
};

struct AccountData
{
public:
    AccountData() : account(0), password(0){}
    AccountData(uint32_t a, uint32_t p) : account(a), password(p){}
    uint32_t account;
    uint32_t password;
};

struct TargetData
{
public:
    TargetData() : attacker(0), targetguid(0){}
    TargetData(uint32_t stalk, uint32_t target) : targetguid(stalk), attacker(target){}
    uint32_t attacker;
    uint32_t targetguid;
};

struct CreatureDeadData
{
    uint32_t creatureguid;
    uint32_t killerguid;
};

struct ExperienceData
{
	uint16_t level;
	uint16_t experience;
};

enum class GameMessage : uint8_t
{
    /*
     newgame should be a request from player to create a new game, and a message to server saying that there is a new game
     joingame is a request to join; or a message to the player to join
     leave game is a request to leave; or a message to the player to leave
     */
    
    UNDEFINED    = 0,
    // Only Player
    NEWGAME      = 1,
    JOINGAME     = 2,
    LEAVEGAME    = 3,
    // Only Server
    STARTGAME    = 4,
    GAMEOVER     = 5,
    NEXTWAVE     = 6
};

enum class GameStatus: uint16_t
{
    UNDEFINED = 0,
    LOBBY     = 1,
    PLAYING   = 2,
    END       = 3
};

struct GameData
{
    GameData()
    {
        guid = players = currentlevel = 0;
        message = GameMessage::UNDEFINED;
    }
    uint32_t        guid;
	std::string     name;
    uint32_t        uniqueid;
    
    uint16_t        points;
    uint16_t        time;
    uint16_t        monsterskilled;

	GameMessage     message;

	uint8_t         currentlevel;
	uint8_t         players;

	bool			won;
};

#endif /* NetworkData_hpp */
