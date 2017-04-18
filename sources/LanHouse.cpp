//
//  LanHouse.cpp
//  server
//
//  Created by Bruno Macedo Miguel on 10/8/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#include <cassert>
#include "LanHouse.hpp"
#include "Factory.hpp"

uint8_t LanHouse::numinstantiated = 0;
uint16_t LanHouse::uniqueClientIdentification = 1000;

LanHouse::LanHouse(MailBox& mailbox, MessageNotificationCenter& messageCenter, Factory& factory) :
_mailBox(mailbox),
_messageCenter(messageCenter),
_factory(factory)
{
    assert(numinstantiated < MAXWORLDS);
    numinstantiated++;
    
    turnOnComputers();
}

void LanHouse::turnOnComputers()
{
    _firstAvailable = &_computers[0];
    
    for (int i = 0; i < MAXUSERS - 1; i++)
    {
        _computers[i].setNext(&_computers[i+1]);
        _computers[i].installDependencies(&_mailBox, &_messageCenter, &_factory);
    }
    
    _computers[MAXUSERS-1].setNext(nullptr);
    _computers[MAXUSERS-1].installDependencies(&_mailBox, &_messageCenter, &_factory);
}


Computer* LanHouse::getComputer(MessageHeader clientNumber, udp::endpoint address, bool createIfNotExist)
{
    return getComputer(static_cast<uint16_t>(clientNumber), address, createIfNotExist);
}

Computer* LanHouse::getComputer(uint16_t clientNumber, udp::endpoint address, bool createIfNotExist)
{
    auto it = _computersMap.find(clientNumber);
    if (it != _computersMap.end())
    {
        return it->second;
    }
    else
    {
        if (createIfNotExist)
        { 
            return assignComputerToUser(address);
        }
    }
    
    return nullptr;
}


void LanHouse::update()
{
    DataBus* action = _factory.getActionToBeBroadcasted();
    for (int i = 0; i < MAXUSERS; i++)
    {
        if (!_computers[i].update(action)) // Inactive for a while
        {
            // Warn that connection is closed
            MessageOutput toSendMessage = MessageOutput::LOGIN_DISCONNECTED;
            _computers[i].writeMessage(MessageHeader::MESSAGE_OUTPUT, static_cast<void*>(&toSendMessage));
            _computers[i].sendMessage(true);
        
            std::cerr << "Computer " << _computers[i].getUserID() << " disconnected." << std::endl;
            
            restartComputer(_computers[i]);
        }
    }
    
    if(action)
        _factory.recycleAction(*action);
}

void LanHouse::fowardMessage(udp::endpoint endpoint, MessageParcel& message)
{
    _reader.load(&message);
    
    _reader.loadHeader();

    Computer* newComputer = getComputer(message.getCurrentHeader(), endpoint, message.getCurrentHeader() ==  MessageHeader::LOGIN_REQUEST);
    
    if (newComputer){
        newComputer->addMessage(message);
    }
    else
    {
        std::cerr << "Computer not found, and it is not intend to login. Message header: " << (uint16_t)message.getCurrentHeader() << std::endl;
        _mailBox.recycle(message);
    }
}


Computer* LanHouse::assignComputerToUser(udp::endpoint address)
{
    // if it happens, better work around the pool size and recycle management
    assert(_firstAvailable != nullptr);
    
    Computer* newComputer = _firstAvailable;
    
    _firstAvailable = newComputer->getNext();
    
    uint16_t newUserID = ++uniqueClientIdentification;
    
    // Clean its state for the upcoming message
    newComputer->cleanState();
    newComputer->setUserID(newUserID);
    newComputer->setAddress(address);
    newComputer->updateActivity();
    
    // add to map
    _computersMap[newUserID] = newComputer;
    
    // notify new client id
    newComputer->writeMessage(MessageHeader::REGISTER_USERID, (void*)&newUserID);
    
    std::cerr << "New computer connected, id " << newComputer->getUserID() << std::endl;
    
    return newComputer;
}

void LanHouse::restartComputer(Computer& computer)
{
    auto it = _computersMap.find(computer.getUserID());
    if (it != _computersMap.end())
    {
        _computersMap.erase(it);
    }
    
    computer.cleanState();
    computer.setUserID(0);
    computer.setNext(_firstAvailable);
    
    _firstAvailable = &computer;
}
