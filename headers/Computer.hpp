//
//  Computer.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 10/12/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef Computer_hpp
#define Computer_hpp

#include <cstdlib>
#include <queue>
#include <boost/asio.hpp>
#include <iostream>
#include "Consts.hpp"
#include "Account.hpp"
#include "MemoryStream.hpp"
#include "MessageAction.hpp"

using boost::asio::ip::udp;

class Factory;
class MessageNotificationCenter;

class Computer
{
    friend class LanHouse;
public:
    // getters
    const bool          inUse(){return _userID != 0;}
    const uint16_t&     getUserID(){return _userID;}
    const udp::endpoint getAddress(){return _address;}
    
    // process and output message
    bool update(DataBus*);
    
    // Action to use objects
    MessageNotificationCenter& getNotificationCenter(){return *_notificationCenter;}
    
    Account& getAccount()          {return _playerAccount;}
    Factory& getFactory()          {return *_factory;}
    
    // write message
    void writeMessage(MessageHeader header, void* data);
    
    void forceDisconnection(){_state.active.forceRestart = true;}
    
protected:
    
    // Dependencies manager
    void installDependencies(MailBox* mailbox, MessageNotificationCenter* messageCenter, Factory* factory);
    bool hasDependencies() {return (_mailBox && _notificationCenter && _factory);}
    
    // Reset variables and recycle message (if exist)
    void cleanState();
    
    // Message processing
    void addMessage(MessageParcel& incomingMessage); // receive message from LanHouse
    void readMessage();
    void sendMessage(bool force = false);
    bool filterMessage(MessageParcel& message);
    
    // State Manager
    void setUserID (uint16_t uid) { _userID = uid; }
    void setAddress (udp::endpoint address) {_address = address;}
    
    // Activity Manager
    bool isActive() {return _state.active.lastActivity + REQUIREDACTIVITYTIME > static_cast<uint64_t>(OTSYS_TIME()) && !_state.active.forceRestart;}
    void updateActivity() {_state.active.lastActivity = static_cast<uint64_t>(OTSYS_TIME());}
    
private:
    
    Computer() :
    _userID(0),
    _mailBox(nullptr),
    _notificationCenter(nullptr),
    _factory(nullptr),
    _outputMessage(nullptr)
    {}
    
    ~Computer(){}
    
    // State
    uint16_t _userID;
    udp::endpoint _address;
    
    // Dependencies
    MailBox*                    _mailBox;
    MessageNotificationCenter*  _notificationCenter;
    Factory*                    _factory;
    
    // Player
    std::unique_ptr<Action>             _action;
    Account _playerAccount;
    
    // Memory Stream
    OutputMemoryBitStream               _writer;
    InputMemoryBitStream                _reader;
    
    // Object Pool
    Computer* getNext() {return _state.next;}
    void setNext(Computer* next) {_state.next = next;}
    
    // Messages
    MessageParcel*                      _outputMessage;
    std::queue<MessageParcel*>         _receiveMessages;
    
    
    union
    {
        struct
        {
            uint32_t                    lastSent;
            uint32_t                    lastReceived;
            uint64_t                    lastActivity;
            bool                        forceRestart;
        } active;
        Computer* next;
    } _state;
};

#endif /* Computer_hpp */
