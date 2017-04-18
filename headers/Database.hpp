//
//  Database.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 3/23/17.
//  Copyright © 2017 d2server. All rights reserved.
//

#ifndef Database_hpp
#define Database_hpp

#include <mysql.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "Account.hpp"
#include "Player.hpp"
#include "Body.hpp"

class DatabaseIO
{
public:
    DatabaseIO()
    {
		
        assert(numinstantiated < 1);
        numinstantiated++;
        
        mysql_init(&mysql);
        
        connection = mysql_real_connect(&mysql,"localhost","root","260894","garuna",0,0,0);
        
        if (connection == NULL)
        {
            std::cout << mysql_error(&mysql) << std::endl;
        }
		
    }
    
    ~DatabaseIO()
    {
        mysql_close(connection);
    }
    
    bool loadPlayer(const uint32_t& databaseid, Player& player);
    bool loadAccount(const uint32_t& databaseid, const uint32_t password, Account& account);
    bool save(const Account& account);
    bool save(const Player& player);

    
private:
    MYSQL_RES *result;
    MYSQL_ROW row;
    MYSQL     *connection, mysql;
    
    std::map<uint32_t, Account> accounts_cache;
    std::map<uint32_t, Player>  players_cache;
    
    static uint8_t  numinstantiated;
};

#endif /* Database_hpp */
