//
//  Account.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 12/13/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef Account_hpp
#define Account_hpp

#include <cstdlib>
#include <vector>
#include "Player.hpp"
#include "NetworkData.hpp"

class Account
{
    friend class Factory;
public:
    Account() : mAccount(0), mPassword(0), mPremiumDays(0), mStatus(OnlineStatus::OFFLINE){}
    
    const uint32_t&           getAccountID()      const{return mAccount;}
    
    const uint32_t&           getPassword()       const{return mPassword;}
    
    const uint32_t&           getPremiumDays()    const{return mPremiumDays;}
    
    bool isConnected() const {return mStatus == OnlineStatus::ONLINE;}
    
    void connect() {mStatus = OnlineStatus::ONLINE;}
    void disconnect() {mStatus = OnlineStatus::OFFLINE;}
    
    Player& getPlayer() {return mLoadedPlayer;}
    
    void load(const uint32_t& id, const uint32_t& password, const uint32_t& premiumdays = 30,
              const OnlineStatus& status = OnlineStatus::OFFLINE)
    {
        mStatus = status;
        mAccount = id;
        mPassword = password;
        mPremiumDays = premiumdays;
    }
    
private:
    
    OnlineStatus mStatus;
    uint32_t     mAccount;
    uint32_t     mPassword;
    uint32_t     mPremiumDays;
    
    // Static here, but body is loaded dynamically
    Player       mLoadedPlayer;
};

#endif /* Account_hpp */
