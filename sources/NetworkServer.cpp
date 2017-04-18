//
//  NetworkServer.cpp
//  gameserver
//
//  Created by Bruno Macedo Miguel on 10/7/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#include "NetworkServer.hpp"
#include "Mail.hpp"

uint8_t NetworkServer::numinstantiated = 0;

void NetworkServer::start_receive()
{
    MessageParcel* emptyParcel = _mailbox.create();
    
    socket_.async_receive_from(boost::asio::buffer(emptyParcel->getMessageBuffer(),MAXPACKETBYTESIZE), remote_endpoint_,
                               boost::bind(&NetworkServer::handle_receive, this,
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred,
                                           emptyParcel));
    
}

void NetworkServer::handle_receive(const boost::system::error_code& error, std::size_t bytecount, MessageParcel* messageParcel)
{
    if (!error || error == boost::asio::error::message_size)
    {
        if (bytecount >= MINPACKETBYTESIZE)
        {
            messageParcel->setBitSize(bytecount*8);
            _lanhouse.fowardMessage(remote_endpoint_, *messageParcel);
            
        }else
        {
            _mailbox.recycle(*messageParcel);
        }
        
        start_receive();
    }
}

void NetworkServer::send(FlightMessage& message)
{
    socket_.async_send_to(boost::asio::buffer(message.getMessage()->getMessageBuffer(),message.getMessage()->getByteSize()), message.getUser()->getAddress(),
                          boost::bind(&NetworkServer::handle_send, this,
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));
}

void NetworkServer::handle_send(const boost::system::error_code& error/*error*/,
                 std::size_t bytecount)
{

}

void NetworkServer::broadcast()
{
    /*if(_connections.size() == 2)
     for (const auto& player : _connections)
     send(player.first);*/
    
    timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(2));
    timer_.async_wait(boost::bind(&NetworkServer::broadcast, this));
}
