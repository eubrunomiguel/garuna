//
//  MessageAction.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 10/22/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef MessageAction_hpp
#define MessageAction_hpp

#include <stdio.h>
#include <iostream>
#include "Consts.hpp"
#include "Math.hpp"
#include "NetworkData.hpp"
#include "Body.hpp"

class OutputMemoryBitStream;
class InputMemoryBitStream;

class Computer;
class Factory;

class Action
{
public:
    Action(MessageHeader header, Factory* factory) : _header(header), _factory(factory){}
    virtual void write(OutputMemoryBitStream&, void* inData)  = 0;
    virtual bool read(InputMemoryBitStream&)                  = 0;
    virtual void execute(Computer&)                           = 0;
protected:
    MessageHeader   _header;
	Factory*		_factory;
};


class ActionAccountLogin : public Action
{
public:

	ActionAccountLogin(Factory* factory) : Action(MessageHeader::LOGIN_ACCOUNT, factory) {}

	virtual void write(OutputMemoryBitStream& stream, void* inData) final;
	virtual bool read(InputMemoryBitStream& stream) final;
	virtual void execute(Computer& computer) final;

private:
	AccountData     _accountdata;
};

class ActionChat : public Action
{
public:

	ActionChat(Factory* factory) : Action(MessageHeader::CHAT, factory) {}

	virtual void write(OutputMemoryBitStream& stream, void* inData) final;
	virtual bool read(InputMemoryBitStream& stream) final;
	virtual void execute(Computer& computer) final {}

private:
	std::string message;
};

        
class ActionMovementCardinal: public Action
{
public:
    ActionMovementCardinal(Factory* factory) : Action(MessageHeader::MOVEMENT_CARDINAL, factory) {}
    
    void write(OutputMemoryBitStream& stream, void* inData) final;
    bool read(InputMemoryBitStream& stream) final;
    void execute(Computer& computer) final;
    
private:
    MovementCardinalData    _movementInfo;
};

class ActionSetTarget: public Action
{
public:
    ActionSetTarget(Factory* factory) : Action(MessageHeader::CREATURE_SETTARGET, factory) {}
    
    void write(OutputMemoryBitStream& stream, void* inData) final;
    bool read(InputMemoryBitStream& stream) final;
    void execute(Computer& computer) final;
    
private:
    TargetData    _targetData;
};


class ActionMovementCoordinate: public Action
{
public:
    
    ActionMovementCoordinate(Factory* factory) : Action(MessageHeader::MOVEMENT_COORDINATE, factory) {}
    
    void write(OutputMemoryBitStream& stream, void* inData) final;
    bool read(InputMemoryBitStream& stream) final;
    void execute(Computer& computer) final;
    
private:
    MovementCoordinateData    _movementInfo;
};


class ActionMovementRotation: public Action
{
public:
    ActionMovementRotation(Factory* factory) : Action(MessageHeader::MOVEMENT_ROTATION, factory) {}
    
    void write(OutputMemoryBitStream& stream, void* inData) final;
    bool read(InputMemoryBitStream& stream) final;
    void execute(Computer& computer) final;
    
private:
    MovementRotationData    _rotationInfo;
};


class ActionShoot: public Action
{
public:
    
    ActionShoot(Factory* factory) : Action(MessageHeader::SHOOT, factory) {}
    
    virtual void write(OutputMemoryBitStream& stream, void* inData) final;
    virtual bool read(InputMemoryBitStream& stream) final;
    virtual void execute(Computer& computer) final;
    
private:
    ShootTargetData         _targetData;
};

class ActionLifeChange: public Action
{
public:
    
    ActionLifeChange(Factory* factory) : Action(MessageHeader::LIFECHANGE, factory) {}
    
    virtual void write(OutputMemoryBitStream& stream, void* inData) final;
    virtual bool read(InputMemoryBitStream& stream) final;
    virtual void execute(Computer& computer) final;
    
private:
    LifeChangeData         _data;
};
        
class ActionMessageNotification: public Action
{
public:
    
    ActionMessageNotification(Factory* factory) : Action(MessageHeader::RECEIVE_NOTIFY_ID, factory) {}
    
    virtual void write(OutputMemoryBitStream& stream, void* inData) final;
    virtual bool read(InputMemoryBitStream& stream) final;
    virtual void execute(Computer& computer) final;
    
private:
    uint32_t         _number;
};
        
class ActionRegisterUser: public Action
{
public:
    
    ActionRegisterUser(Factory* factory) : Action(MessageHeader::REGISTER_USERID, factory) {}
    
    virtual void write(OutputMemoryBitStream& stream, void* inData) final;
    virtual bool read(InputMemoryBitStream& stream) final;
	virtual void execute(Computer& computer) final {}
    
private:
    uint16_t         _userID;
};

class ActionOutputMessage: public Action
{
public:
    
    ActionOutputMessage(Factory* factory) : Action(MessageHeader::MESSAGE_OUTPUT, factory) {}
    
    virtual void write(OutputMemoryBitStream& stream, void* inData) final;
    
    // We only send it.
    virtual bool read(InputMemoryBitStream& stream) final;
    virtual void execute(Computer& computer) final;
    
private:
    MessageOutput    _message;
};

class ActionBodyDespawn: public Action
{
public:
    ActionBodyDespawn(Factory* factory) : Action(MessageHeader::BODY_DESPAWN, factory) {}
    
    virtual void write(OutputMemoryBitStream& stream, void* inData) final;
    virtual bool read(InputMemoryBitStream& stream) final;
	virtual void execute(Computer& computer) final {}
private:
    BodyDespawnData   _bodyData;
};


class ActionCreatureSpawn: public Action
{
public:
    ActionCreatureSpawn(Factory* factory) : Action(MessageHeader::BODY_SPAWN, factory) {}
    
    virtual void write(OutputMemoryBitStream& stream, void* inData) final;
    virtual bool read(InputMemoryBitStream& stream) final;
	virtual void execute(Computer& computer) final {}
private:
    Body         _creatureData;
};

class ActionExperience: public Action
{
public:
    ActionExperience(Factory* factory) : Action(MessageHeader::EXPERIENCE, factory) {}
    
    virtual void write(OutputMemoryBitStream& stream, void* inData) final;
	virtual bool read(InputMemoryBitStream& stream) final { return false; }
	virtual void execute(Computer& computer) final {}
private:
    ExperienceData _experiencedata;
};

class ActionGame: public Action
{
public:
    ActionGame(Factory* factory) : Action(MessageHeader::GAME, factory) {}
    
    virtual void write(OutputMemoryBitStream& stream, void* inData) final;
    virtual bool read(InputMemoryBitStream& stream) final;
    virtual void execute(Computer& computer) final;
private:
    GameData  _gamedata;
};

class ActionItem: public Action
{
public:
    ActionItem(Factory* factory) : Action(MessageHeader::ITEM, factory) {}
    
    virtual void write(OutputMemoryBitStream& stream, void* inData) final;
    virtual bool read(InputMemoryBitStream& stream) final;
	virtual void execute(Computer& computer) final {}
private:
    Item  _item;
};

class ActionCreatureDead: public Action
{
public:
    ActionCreatureDead(Factory* factory) : Action(MessageHeader::CREATURE_DEAD, factory) {}
    
    virtual void write(OutputMemoryBitStream& stream, void* inData) final;
    virtual bool read(InputMemoryBitStream& stream) final;
	virtual void execute(Computer& computer) final {}
private:
    CreatureDeadData  _data;
};

        
class ActionHandler
{
public:
    static Action* getAction(MessageHeader action, Factory* factory)
    {
        Action* newaction = nullptr;
        switch(action)
        {
            case MessageHeader::LOGIN_ACCOUNT:
                newaction = new ActionAccountLogin(factory);
                break;
            case MessageHeader::MESSAGE_OUTPUT:
                newaction = new ActionOutputMessage(factory);
                break;
            case MessageHeader::REGISTER_USERID:
                newaction = new ActionRegisterUser(factory);
                break;
            case MessageHeader::MOVEMENT_CARDINAL:
                newaction = new ActionMovementCardinal(factory);
                break;
            case MessageHeader::MOVEMENT_COORDINATE:
                newaction = new ActionMovementCoordinate(factory);
                break;
            case MessageHeader::MOVEMENT_ROTATION:
                newaction = new ActionMovementRotation(factory);
                break;
            case MessageHeader::SHOOT:
                newaction = new ActionShoot(factory);
                break;
            case MessageHeader::RECEIVE_NOTIFY_ID:
                newaction = new ActionMessageNotification(factory);
                break;
            case MessageHeader::LIFECHANGE:
                newaction = new ActionLifeChange(factory);
                break;
            case MessageHeader::BODY_DESPAWN:
                newaction = new ActionBodyDespawn(factory);
                break;
            case MessageHeader::BODY_SPAWN:
                newaction = new ActionCreatureSpawn(factory);
                break;
            case MessageHeader::GAME:
                newaction = new ActionGame(factory);
                break;
            case MessageHeader::CREATURE_SETTARGET:
                newaction = new ActionSetTarget(factory);
                break;
            case MessageHeader::CREATURE_DEAD:
                newaction = new ActionCreatureDead(factory);
                break;
            case MessageHeader::ITEM:
                newaction = new ActionItem(factory);
                break;
			case MessageHeader::EXPERIENCE:
				newaction = new ActionExperience(factory);
				break;
			case MessageHeader::CHAT:
				newaction = new ActionChat(factory);
				break;
            default:
                std::cerr << "Unknow Header " << (uint16_t)action << ", MessageAction.hpp::239." << std::endl;
        }
        return newaction;
    }
};
        


#endif /* MessageAction_hpp */
