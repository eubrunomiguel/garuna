//
//  Mail.cpp
//  server
//
//  Created by Bruno Macedo Miguel on 10/8/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#include "Mail.hpp"

uint8_t MailBox::numinstantiated = 0;
uint16_t MailBox::mailIdentification = 0;

MailBox::MailBox()
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

MessageParcel* MailBox::create()
{
    // if it happens, better work around the pool size and recycle management
    assert(_firstAvailable != nullptr);
    
    MessageParcel* freshParcel = _firstAvailable;
    
    _firstAvailable = freshParcel->getNext();
    
    // Clean its state for the upcoming message
    freshParcel->reset();
    
    freshParcel->setID(MailBox::getUniqueID());
    
    return freshParcel;
}


void MailBox::recycle(MessageParcel& parcel)
{
    parcel.setID(0);
    parcel.setNext(_firstAvailable);
    _firstAvailable = &parcel;
}
