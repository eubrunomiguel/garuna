//
//  MailNotification.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 10/22/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef MailNotification_hpp
#define MailNotification_hpp

#include "Tools.hpp"
#include "Consts.hpp"
#include <unordered_map>
#include <cstdlib>
#include "MemoryStream.hpp"

class NetworkServer;
class MessageParcel;
class MailBox;
class Computer;

class FlightMessage
{
    friend class MessageNotificationCenter;
public:
    void setMessage(MessageParcel* message) {_carryingMessage = message;}
    MessageParcel* getMessage()                       {return _carryingMessage;}
    
    void updateTimeSent() {_state.active.timeSent = OTSYS_TIME();}
    bool flightTimeExpired(){return (_state.active.timeSent + RESENDINTERVAL) < static_cast<uint64_t>(OTSYS_TIME());}
    
    void setOptions(MessageOptions opt) {_state.active.messageOptions |= opt;}
    MessageOptions getOptions() {return _state.active.messageOptions;}
    
    void updateStatus(MessageStatus status) {_status = status;}
    MessageStatus getStatus() {return _status;}
    
    bool inUse() {return _status != MessageStatus::AVAILABLE;}
    void setUser(Computer* computer) {_user = computer;}
    Computer* getUser()             {return _user;}
    
    bool hasSendingTryoutExpired() {return _state.active.sentTryouts > MAXSENDTRYOUTS;}
    void updateSendingTryout() {_state.active.sentTryouts++;}
    
    void setMessageNumber(uint32_t num) {_state.active.messageNumber = num;}
    uint32_t getMessageNumber() {return _state.active.messageNumber;}
    
    void send(){_status = MessageStatus::READY;}
    
private:
    FlightMessage() : _status(MessageStatus::AVAILABLE), _carryingMessage(nullptr), _user(nullptr){}
    FlightMessage*  getNext () const                        { return _state.next; }
    void            setNext (FlightMessage* next)           { _state.next = next; }
    
    void            reset()
    {
        _state.active.timeSent = 0;
        _state.active.messageOptions = MessageOptions::UNDEFINED;
        _state.active.sentTryouts = 0;
        _state.active.messageNumber = 0;
    }
    
    union
    {
        struct
        {
            MessageOptions  messageOptions;
            uint64_t        timeSent;
            uint32_t        messageNumber;
            uint16_t        sentTryouts;
        } active;
        
        FlightMessage* next;
    } _state;
    
    MessageParcel*  _carryingMessage;
    Computer*       _user;
    
    MessageStatus _status;
};

class MessageNotificationCenter
{
    friend class MailBox;
public:
    MessageNotificationCenter(MailBox& mailBox);
    
    FlightMessage* create(Computer* user, MessageParcel* message);
    
    void notify(uint32_t messageNumber);
    
    void recycle(FlightMessage* parcel);
    
    void update(NetworkServer&);
    
private:
    MailBox&                                        _mailBox;
    FlightMessage*                                  _firstAvailable;
    FlightMessage                                   _parcels[MESSAGEPOOLSIZE];
    
    uint32_t                                        _messageNumber;
    
    OutputMemoryBitStream                           _writer;
    
    std::unordered_map<uint32_t, FlightMessage*>    _messagesOnFly;
    
    static uint8_t numinstantiated;
};


#endif /* MailNotification_hpp */
