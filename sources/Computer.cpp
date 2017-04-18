//
//  Computer.cpp
//  server
//
//  Created by Bruno Macedo Miguel on 10/12/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#include <cassert>
#include "Computer.hpp"
#include "MailNotification.hpp"
#include "MessageAction.hpp"
#include "Factory.hpp"

bool Computer::update(DataBus* action)
{
    if (!inUse()) return true;
    if (!isActive()) return false;
    
    assert(hasDependencies());
    
    if (action)
        if (_playerAccount.getPlayer().isConnected())
            if (action->messageTo == _playerAccount.getPlayer().getPlayerBody().getGUID() || action->messageTo == BROADCASTALL)
                writeMessage(action->messageType, action->get());
    
    readMessage();
    sendMessage();
    
    return true;
}

void Computer::installDependencies(MailBox* mailbox, MessageNotificationCenter* messageCenter, Factory* factory){
    _mailBox = mailbox;
    _notificationCenter = messageCenter;
    _factory = factory;
}

void Computer::addMessage(MessageParcel& incomingMessage)
{
    if (filterMessage(incomingMessage))
    {
        _receiveMessages.push(&incomingMessage);
        return;
    }
    _mailBox->recycle(incomingMessage);
}

void Computer::cleanState() {
    _state.active.lastSent          = 0;
    _state.active.lastReceived      = 0;
    _state.active.lastActivity      = 0;
    _state.active.forceRestart      = false;
    
    while (!_receiveMessages.empty())
    {
        _mailBox->recycle(*_receiveMessages.front());
        _receiveMessages.pop();
    }
    
    if(_outputMessage != nullptr)
    {
        _mailBox->recycle(*_outputMessage);
        _outputMessage = nullptr;
    }
    
	if (_playerAccount.isConnected() || _playerAccount.getPlayer().isConnected()) {
		_factory->accountLogout(_playerAccount);
	}
}


bool Computer::filterMessage(MessageParcel& message)
{
    InputMemoryBitStream tempReader(&message);

    ParcelIdentification head;
    tempReader.Read(head);
    
    uint32_t receiveNumber = head.user_send_id;
    uint32_t expectedNumber = _state.active.lastReceived + 1;
    
    uint32_t messageToAcknoledge = head.notification_id;
    
    if (receiveNumber < expectedNumber)
    {
        return false;
    }
    
    _state.active.lastReceived = receiveNumber;
    
    writeMessage(MessageHeader::RECEIVE_NOTIFY_ID, static_cast<void*>(&messageToAcknoledge));
    
    return true;
}

void Computer::writeMessage(MessageHeader header, void* data)
{
    // make sure this cannot bug! cannot happen between  sending
    if (_outputMessage == nullptr)
    {
        _outputMessage = _mailBox->create();
        _writer.load(_outputMessage);
        
        ParcelIdentification head;
        head.notification_id = _outputMessage->getID();
        head.user_send_id = ++_state.active.lastSent;
        
        _writer.Write(MessageHeader::UNDEFINED);
        _writer.Write(head);
    }
    
    _action.reset(ActionHandler::getAction(header, _factory));
    if (_action)
    {
        _action->write(_writer, data);
    }
    
}

void Computer::sendMessage(bool force)
{
    
    if (_outputMessage == nullptr)
        return;
    
    if (_outputMessage->getByteSize() < MINPACKETBYTESIZE  && !force)
    {
        _outputMessage->resetCreationTime();
        return;
    }
    
    if (_outputMessage->lifeTimeExpired() || _outputMessage->getByteSize() > MAXPACKETBYTESIZE || force)
    {
        FlightMessage* newFlightMessage = _notificationCenter->create(this, _outputMessage);
        newFlightMessage->setOptions(MessageOptions::CURRENT);
        newFlightMessage->setOptions(MessageOptions::IMPORTANT);
        newFlightMessage->send();
        _outputMessage = nullptr;
    }
}


void Computer::readMessage()
{
    if (_receiveMessages.empty())
        return;
    
    MessageParcel* message = _receiveMessages.front();
    
    _reader.load(message);
    
    _reader.loadHeader();
    
    _action.reset(ActionHandler::getAction(message->getCurrentHeader(), _factory));

    if (_action && _action->read(_reader))
    {
        _action->execute(*this);
        if (_reader.getRemainingBitCount() > sizeof(MessageHeader))
        {
            readMessage();
            return;
        }
    }
    _receiveMessages.pop();
    _mailBox->recycle(*message);
    updateActivity();
}
