//
//  NetworkServer.hpp
//  gameserver
//
//  Created by Bruno Macedo Miguel on 10/7/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef NetworkServer_hpp
#define NetworkServer_hpp

#include <ctime>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include "LanHouse.hpp"

class MailBox;

using boost::asio::ip::udp;


class NetworkServer
{
    friend class MessageNotificationCenter;
public:
    NetworkServer(boost::asio::io_service& io_service, uint16_t port , MailBox& mailbox, LanHouse& lanhouse):
    socket_(io_service, udp::endpoint(udp::v4(), port)),
    timer_(io_service, boost::posix_time::seconds(1)),
    _mailbox(mailbox),
    _lanhouse(lanhouse)
    {
        assert(numinstantiated < MAXWORLDS);
        numinstantiated++;
        
        start_receive();
    }
    
    ~NetworkServer()
    {
        
    }
    
    
private:
    
    void start_receive();
    
    void handle_receive(const boost::system::error_code& error, std::size_t bytecount, MessageParcel* messageParcel);
    
    void send(FlightMessage& message);
    
    void handle_send(const boost::system::error_code& error/*error*/,
                     std::size_t /*bytes_transferred*/);
    
    void broadcast();
    
    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    
    boost::asio::deadline_timer timer_;
    
    static uint8_t numinstantiated;
    
    LanHouse&                   _lanhouse;
    MailBox&                    _mailbox;
};


#endif /* NetworkServer_hpp */
