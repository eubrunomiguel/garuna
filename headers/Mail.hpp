//
//  Mail.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 10/8/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef Mail_hpp
#define Mail_hpp

#undef max
#undef min

#include <cstdlib>
#include <cassert>
#include "Consts.hpp"
#include "Tools.hpp"
#include <limits>

class MessageParcel
{
    friend class MailBox;
public:

    const bool      inUse()                                 {return _uniqueID != 0;}
    
    const uint16_t  getID()                                 {return _uniqueID;}
    
    void            setBitSize(uint16_t size)               {_state.active.bitSize = size;}
    void            setBitSize(size_t size)                 {_state.active.bitSize = static_cast<uint16_t>(size);}
    
    uint16_t        getByteSize()                           {return _state.active.bitHead/8;}
    uint16_t        getBitSize()                            {return _state.active.bitSize;}
    uint16_t&       getBitHead()                            {return _state.active.bitHead;}
    void            setBitHead(uint32_t begin)              {_state.active.bitHead = begin;}
    
    char*           getMessageBuffer ()                     {return _message;}
    
    void            resetCreationTime()                     {_state.active.creationtime = OTSYS_TIME();}
    uint64_t        getCreationTime()                       {return _state.active.creationtime;}
    bool            lifeTimeExpired()                       {return (_state.active.creationtime + SENDINTERVAL) < static_cast<uint64_t>(OTSYS_TIME());}
    
    void            setCurrentHeader(MessageHeader header)  {_state.active.messageCurrentHeader = header;}
    void            setCurrentHeader(uint16_t header)       {_state.active.messageCurrentHeader = static_cast<MessageHeader>(header);}
    MessageHeader&  getCurrentHeader()                      {return _state.active.messageCurrentHeader;}
    
    
    
private:
    
    MessageParcel(): _uniqueID(0){}
    
    void            setID(uint16_t uid)                     {_uniqueID = uid;}

    MessageParcel*  getNext () const                        { return _state.next; }
    void            setNext (MessageParcel* next)           { _state.next = next; }
    
    
    void reset()
    {
        _state.active.messageCurrentHeader      = MessageHeader::UNDEFINED;
        _state.active.creationtime              = OTSYS_TIME();
        _state.active.bitSize                   = 0;
        _state.active.bitHead                   = 0;
    }
    
    union
    {
        struct
        {
            MessageHeader  messageCurrentHeader;
            uint64_t       creationtime;
            uint16_t       bitSize;
            uint16_t       bitHead;
        } active;

        
        MessageParcel* next;
    } _state;
    

    uint32_t        _uniqueID;
    char            _message[MAXPACKETBYTESIZE];
};


class MailBox
{
    friend class NetworkServer;
public:
    
    MailBox();
    
    MessageParcel* create();
    
    void recycle(MessageParcel& parcel);
    
private:
    
    MessageParcel* _firstAvailable;
    MessageParcel _parcels[MESSAGEPOOLSIZE];
    static uint8_t numinstantiated;
    
    static uint16_t mailIdentification;
    
    static uint16_t getUniqueID()
    {
		auto max = std::numeric_limits<uint16_t>::max() - 1;
        if (mailIdentification > max)
            mailIdentification = 1000;
        
        return ++mailIdentification;
    }
};

#endif /* Mail_hpp */
