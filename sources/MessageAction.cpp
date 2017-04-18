//
//  MessageAction.cpp
//  server
//
//  Created by Bruno Macedo Miguel on 10/22/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#include "MessageAction.hpp"
#include "MemoryStream.hpp"
#include "Computer.hpp"
#include "Factory.hpp"
#include "MailNotification.hpp"
#include <iostream>
#include <vector>


void ActionMessageNotification::write(OutputMemoryBitStream& stream, void* inData)
{
    _number = *static_cast<uint32_t*>(inData);
    stream.Write(_header);
    stream.Write(_number);
}


bool ActionMessageNotification::read(InputMemoryBitStream& stream)
{
    stream.Read(_number);
    return stream.messageSuccessfullyRead();
}

void ActionMessageNotification::execute(Computer& computer)
{
    computer.getNotificationCenter().notify(_number);
}

void ActionRegisterUser::write(OutputMemoryBitStream& stream, void* inData)
{
    _userID = *static_cast<uint16_t*>(inData);
    stream.Write(_header);
    stream.Write(_userID);
}


bool ActionRegisterUser::read(InputMemoryBitStream& stream)
{
    stream.Read(_userID);
    return stream.messageSuccessfullyRead();
}

void ActionShoot::write(OutputMemoryBitStream& stream, void* inData)
{
    _targetData = *static_cast<ShootTargetData*>(inData);
    stream.Write(_header);
    stream.Write(_targetData);
}

bool ActionShoot::read(InputMemoryBitStream& stream)
{
    stream.Read(_targetData);
    return stream.messageSuccessfullyRead();
}

void ActionShoot::execute(Computer& computer)
{
    DataBus* tempData     = _factory->createAction();
    tempData->messageFrom = _targetData.mGUID;
    tempData->messageTo   = BROADCASTALL;
    tempData->messageType = MessageHeader::SHOOT;
    
    auto shootData        = tempData->create<ShootTargetData>();
    *shootData            = _targetData; // check
    
    _factory->uploadActionToGameWorld(tempData);
}

void ActionLifeChange::write(OutputMemoryBitStream& stream, void* inData)
{
    _data = *static_cast<LifeChangeData*>(inData);
    stream.Write(_header);
    stream.Write(_data);
}

bool ActionLifeChange::read(InputMemoryBitStream& stream)
{
    stream.Read(_data);
    return stream.messageSuccessfullyRead();
}

void ActionLifeChange::execute(Computer& computer)
{
    DataBus* tempData           = _factory->createAction();
    tempData->messageFrom       = _data.target;
    tempData->messageTo         = BROADCASTALL;
    tempData->messageType       = MessageHeader::LIFECHANGE;
    
    auto lifeData               = tempData->create<LifeChangeData>();
    *lifeData                   = _data;
    
    _factory->uploadActionToGameWorld(tempData);
}

void ActionChat::write(OutputMemoryBitStream& stream, void* inData)
{
	auto message = static_cast<char*>(inData);
	char * token = strchr(message, '\n');
	uint8_t size = token - message;
	stream.Write(_header);
	stream.Write(size);
	stream.WriteBytes(message, size);
}

bool ActionChat::read(InputMemoryBitStream& stream)
{
	uint8_t size;
	stream.Read(size);

	if (size > MAXCHATSIZE)
	{
		std::cerr << "Chat message limite " << MAXDATABUSSIZE << " bytes." << std::endl;
		return false;
	}
	DataBus* tempData	= _factory->createAction();
	tempData->messageTo = BROADCASTALL;
	tempData->messageType = MessageHeader::CHAT;

	auto message = static_cast<char*>(tempData->get());
	stream.ReadBytes(message, size);

	message[size] = '\n';

	_factory->addActionToBeBroadcasted(tempData);
	
	return stream.messageSuccessfullyRead();
}

void ActionMovementCardinal::write(OutputMemoryBitStream& stream, void* inData)
{
    _movementInfo = *static_cast<MovementCardinalData*>(inData);
    stream.Write(_header);
    stream.Write(_movementInfo);
}

bool ActionMovementCardinal::read(InputMemoryBitStream& stream)
{
    stream.Read(_movementInfo);
    return stream.messageSuccessfullyRead();
}

void ActionMovementCardinal::execute(Computer& computer)
{
    DataBus* tempData       = _factory->createAction();
    tempData->messageFrom   = _movementInfo.mGUID;
    tempData->messageTo     = BROADCASTALL;
    tempData->messageType   = MessageHeader::MOVEMENT_CARDINAL;
    
    auto tempMovement       = tempData->create<MovementCardinalData>();
    *tempMovement           = _movementInfo;

    _factory->uploadActionToGameWorld(tempData);
}

void ActionMovementCoordinate::write(OutputMemoryBitStream& stream, void* inData)
{
    _movementInfo = *static_cast<MovementCoordinateData*>(inData);
    stream.Write(_header);
    stream.Write(_movementInfo);
}

bool ActionMovementCoordinate::read(InputMemoryBitStream& stream)
{
    stream.Read(_movementInfo);

    return stream.messageSuccessfullyRead();
}

void ActionMovementCoordinate::execute(Computer& computer)
{
    DataBus* tempData         = _factory->createAction();
    tempData->messageFrom     = _movementInfo.mGUID;
    tempData->messageTo       = BROADCASTALL;
    tempData->messageType     = MessageHeader::MOVEMENT_COORDINATE;
    auto tempMovement         = tempData->create<MovementCoordinateData>();
    *tempMovement             = _movementInfo;
    _factory->uploadActionToGameWorld(tempData);
}

void ActionMovementRotation::write(OutputMemoryBitStream& stream, void* inData)
{
    _rotationInfo = *static_cast<MovementRotationData*>(inData);
    stream.Write(_header);
    stream.Write(_rotationInfo);
}

bool ActionMovementRotation::read(InputMemoryBitStream& stream)
{
    stream.Read(_rotationInfo);
    return stream.messageSuccessfullyRead();
}

void ActionMovementRotation::execute(Computer& computer)
{
    DataBus* tempData           = _factory->createAction();
    tempData->messageFrom       = _rotationInfo.mGUID;
    tempData->messageTo         = BROADCASTALL;
    tempData->messageType       = MessageHeader::MOVEMENT_ROTATION;
    
    auto rotationData           = tempData->create<MovementRotationData>();
    *rotationData               = _rotationInfo;
    _factory->addActionToBeBroadcasted(tempData);
}

void ActionAccountLogin::write(OutputMemoryBitStream& stream, void* inData)
{
    stream.Write(_header);
    stream.Write(*static_cast<Body*>(inData));
}


bool ActionAccountLogin::read(InputMemoryBitStream& stream)
{
    stream.Read(_accountdata);
    return stream.messageSuccessfullyRead();
}

void ActionAccountLogin::execute(Computer& computer)
{
    MessageOutput toSendMessage = MessageOutput::NONE;
    if (!computer.getAccount().isConnected())
    {
        if (_factory->accountLogin(computer.getAccount(), _accountdata, toSendMessage))
        {
            computer.writeMessage(MessageHeader::LOGIN_ACCOUNT, static_cast<void*>(&computer.getAccount().getPlayer().getPlayerBody()));
            _factory->sendGameList(computer.getAccount().getPlayer().getPlayerBody().getGUID());
        }
    }
    else
        toSendMessage = MessageOutput::ACCOUNT_ERROR;
    
    computer.writeMessage(MessageHeader::MESSAGE_OUTPUT, static_cast<void*>(&toSendMessage));
}


void ActionOutputMessage::write(OutputMemoryBitStream& stream, void* inData)
{
    _message = *static_cast<MessageOutput*>(inData);
    stream.Write(_header);
    stream.Write(_message);
}

bool ActionOutputMessage::read(InputMemoryBitStream& stream)
{
    stream.Read(_message);
    return stream.messageSuccessfullyRead();
}

void ActionOutputMessage::execute(Computer &computer)
{
    switch(_message)
    {
        case MessageOutput::ACCOUNT_DISCONNECTED:
                computer.forceDisconnection();
				std::cerr << "Force disconnection requested from computer " << (int)computer.getUserID() << std::endl;
            break;
        default:
            std::cerr << "MessageOutput from client not processed " << (int)_message << std::endl;
            break;
    }
}


void ActionBodyDespawn::write(OutputMemoryBitStream& stream, void* inData)
{
    _bodyData = *static_cast<BodyDespawnData*>(inData);
    stream.Write(_header);
    stream.Write(_bodyData.guid);
}

bool ActionBodyDespawn::read(InputMemoryBitStream& stream)
{
    stream.Read(_bodyData.guid);
    return stream.messageSuccessfullyRead();
}

void ActionCreatureSpawn::write(OutputMemoryBitStream& stream, void* inData)
{
    _creatureData = *static_cast<Body*>(inData);
    stream.Write(_header);
    stream.Write(_creatureData);
}

bool ActionCreatureSpawn::read(InputMemoryBitStream& stream)
{
    stream.Read(_creatureData);
    return stream.messageSuccessfullyRead();
}

void ActionExperience::write(OutputMemoryBitStream& stream, void* inData)
{
    _experiencedata = *static_cast<ExperienceData*>(inData);
    stream.Write(_header);
    stream.Write(_experiencedata);
}


void ActionItem::write(OutputMemoryBitStream& stream, void* inData)
{
    _item = *static_cast<Item*>(inData);
    stream.Write(_header);
    stream.Write(_item);
}

bool ActionItem::read(InputMemoryBitStream& stream)
{
    stream.Read(_item);
    return stream.messageSuccessfullyRead();
}

void ActionGame::write(OutputMemoryBitStream& stream, void* inData)
{
    _gamedata = *static_cast<GameData*>(inData);
    stream.Write(_header);
    stream.Write(_gamedata);
}

bool ActionGame::read(InputMemoryBitStream& stream)
{
    stream.Read(_gamedata);
    return stream.messageSuccessfullyRead();
}

void ActionGame::execute(Computer& computer)
{
    DataBus* tempData         = _factory->createAction();
    tempData->messageType     = MessageHeader::GAME;
    tempData->messageFrom     = _gamedata.guid;
    tempData->messageTo       = BROADCASTALL;
    auto tempLevel            = tempData->create<GameData>();
    *tempLevel                = _gamedata;
    _factory->uploadActionToGameWorld(tempData);
}

void ActionSetTarget::write(OutputMemoryBitStream& stream, void* inData)
{
    _targetData = *static_cast<TargetData*>(inData);
    stream.Write(_header);
    stream.Write(_targetData.attacker);
    stream.Write(_targetData.targetguid);
}

bool ActionSetTarget::read(InputMemoryBitStream& stream)
{
    stream.Read(_targetData.attacker);
    stream.Read(_targetData.targetguid);
    
    return stream.messageSuccessfullyRead();
}

void ActionSetTarget::execute(Computer& computer)
{
    DataBus* tempData         = _factory->createAction();
    tempData->messageFrom     = _targetData.targetguid;
    tempData->messageTo       = BROADCASTALL;
    tempData->messageType     = MessageHeader::CREATURE_SETTARGET;
    auto targetData           = tempData->create<TargetData>();
    *targetData               = _targetData;
    _factory->uploadActionToGameWorld(tempData);
}

void ActionCreatureDead::write(OutputMemoryBitStream& stream, void* inData)
{
    _data = *static_cast<CreatureDeadData*>(inData);
    stream.Write(_header);
    stream.Write(_data);
}

bool ActionCreatureDead::read(InputMemoryBitStream& stream)
{
    stream.Read(_data);
    return stream.messageSuccessfullyRead();
}