//
//  LanHouse.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 10/8/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef LanHouse_hpp
#define LanHouse_hpp

#include <cstdlib>
#include <cassert>
#include "Consts.hpp"
#include "Computer.hpp"
#include "MailNotification.hpp"
#include <boost/asio.hpp>
#include <unordered_map>

class Factory;

using boost::asio::ip::udp;

class LanHouse
{
    friend class NetworkServer;
public:
    
    LanHouse(MailBox& mailbox, MessageNotificationCenter& messageCenter, Factory& factory);
    
    const uint16_t getActiveComputers() {return static_cast<uint16_t>(_computersMap.size());}
    
    void update();
    
protected:
    // Initialize computers
    void            turnOnComputers();
    
    // Get Computer
    Computer*       getComputer(MessageHeader clientNumber, udp::endpoint address, bool createIfNotExist = false);
    Computer*       getComputer(uint16_t clientNumber, udp::endpoint address, bool createIfNotExist = false);
    Computer*       assignComputerToUser(udp::endpoint address);
    void            restartComputer(Computer& computer);
    
    // Process first network message
    void            fowardMessage(udp::endpoint endpoint, MessageParcel& message);
    
private:
    
    // Dependencies
    Factory&                        _factory;
    MailBox&                        _mailBox;
    MessageNotificationCenter&      _messageCenter;
    
    // Computer Map
    std::unordered_map<uint16_t, Computer*>         _computersMap;
    
    // Object Pool
    Computer*                       _firstAvailable;
    Computer                        _computers[MAXUSERS];
    
    // Memory Stream
    InputMemoryBitStream            _reader;
    OutputMemoryBitStream           _writer;
    
    static uint16_t uniqueClientIdentification;
    
    // Unique Instance
    static uint8_t numinstantiated;
};


#endif /* LanHouse_hpp */
