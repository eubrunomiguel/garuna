//
//  Player.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 1/26/17.
//  Copyright © 2017 d2server. All rights reserved.
//

#ifndef Player_hpp
#define Player_hpp

#include <cstdlib>
#include "Consts.hpp"
#include "Body.hpp"

class Player
{
    friend class Factory;
public:
    Player() : mAccountID(0), mStatus(OnlineStatus::OFFLINE), mPlayerBody(Body()){}
    
    const uint32_t   getAccountID()  const {return mAccountID;}
    const Body&      getPlayerBody() const {return mPlayerBody;}
    bool isConnected() const {return (mStatus == OnlineStatus::ONLINE && mPlayerBody.getGUID() != 0);}
    void connect()    {mStatus = OnlineStatus::ONLINE;}
    void disconnect() {mStatus = OnlineStatus::OFFLINE;}
    
    Body& getPlayerBody()           {return mPlayerBody;}
    
    void load(const uint32_t& account, const OnlineStatus& status = OnlineStatus::OFFLINE)
    {
        mAccountID  = account;
        mStatus     = status;
    }
private:
    
    OnlineStatus mStatus;
    uint32_t   mAccountID;
    Body         mPlayerBody;
};

#endif /* Player_hpp */
