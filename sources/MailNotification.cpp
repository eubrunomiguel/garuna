//
//  MailNotification.cpp
//  server
//
//  Created by Bruno Macedo Miguel on 10/22/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#include "NetworkServer.hpp"
#include <cassert>

uint8_t MessageNotificationCenter::numinstantiated = 0;

MessageNotificationCenter::MessageNotificationCenter(MailBox& mailBox) : _mailBox(mailBox), _messageNumber(0)
{
    assert(numinstantiated < MAXWORLDS);
    numinstantiated++;
    
    // The first one is available
    _firstAvailable = &_parcels[0];
    
    // Each parcel points to the next
    for (int i = 0; i < MESSAGEPOOLSIZE - 1; i++)
    {
        _parcels[i].setNext(&_parcels[i+1]);
    }
    
    // Last one terminates the list
    _parcels[MESSAGEPOOLSIZE-1].setNext(nullptr);
}


void MessageNotificationCenter::update(NetworkServer& network)
{
    //std::cerr << "messages on flight " << _messagesOnFly.size() << std::endl;
    for (int i = 0; i < MESSAGEPOOLSIZE; i++)
    {
        switch(_parcels[i].getStatus())
        {
            case MessageStatus::READY:
                    network.send(_parcels[i]);
                    _parcels[i].updateStatus(MessageStatus::FLIGHT);
                    _parcels[i].updateTimeSent();
                    _messagesOnFly.insert(std::make_pair<uint32_t, FlightMessage*>(_parcels[i].getMessageNumber(), &_parcels[i]));
                break;
            case MessageStatus::FLIGHT:
                    if (_parcels[i].flightTimeExpired())
                    {
                        //std::cerr << "resending " << _parcels[i].getMessageNumber() <<  std::endl;
                        if ((_parcels[i].getOptions() & MessageOptions::IMPORTANT) && !_parcels[i].hasSendingTryoutExpired())
                        {
                            if (_parcels[i].getOptions() & MessageOptions::CURRENT)
                            {
                                // get current info;
                            }
                            _parcels[i].send();
                            _parcels[i].updateSendingTryout();
                        }
                        else
                        {
                            _parcels[i].updateStatus(MessageStatus::COMPLETED);
                        }
                    }
                break;
            case MessageStatus::COMPLETED:
                    notify(_parcels[i].getMessageNumber());
                    recycle(&_parcels[i]);
                break;
            default:
                break;
        }
    }
}

void MessageNotificationCenter::notify(uint32_t messageNumber)
{
    auto it = _messagesOnFly.find(messageNumber);
    if (it != _messagesOnFly.end())
    {
        it->second->updateStatus(MessageStatus::COMPLETED);
        _messagesOnFly.erase(it);
    }
}

void MessageNotificationCenter::recycle(FlightMessage* parcel)
{
    if (parcel->getMessage() != nullptr)
    {
        _mailBox.recycle(*parcel->getMessage());
        parcel->setMessage(nullptr);
    }
    
    parcel->setMessageNumber(0);
    parcel->updateStatus(MessageStatus::AVAILABLE);
    parcel->setNext(_firstAvailable);
    _firstAvailable = parcel;
}

FlightMessage* MessageNotificationCenter::create(Computer* user, MessageParcel* message)
{
    // if it happens, better work around the pool size and recycle management
    assert(_firstAvailable != nullptr);
    
    FlightMessage* freshParcel = _firstAvailable;
    
    _firstAvailable = freshParcel->getNext();
    
    // Clean its state for the upcoming message
    freshParcel->reset();
    
    freshParcel->updateStatus(MessageStatus::CREATING);
    freshParcel->setMessage(message);
    freshParcel->setUser(user);
    freshParcel->setMessageNumber(message->getID());
    
    return freshParcel;
}
