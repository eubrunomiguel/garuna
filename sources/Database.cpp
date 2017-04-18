//
//  Database.cpp
//  server
//
//  Created by Bruno Macedo Miguel on 3/23/17.
//  Copyright © 2017 d2server. All rights reserved.
//

#include "Database.hpp"

uint8_t DatabaseIO::numinstantiated = 0;

bool DatabaseIO::loadPlayer(const uint32_t& databaseid, Player& player)
{
    if (players_cache.count(databaseid) == 1)
    {
        Player* cache_player = &players_cache[databaseid];
        if (!cache_player->isConnected()){
            cache_player->connect();
            player = *cache_player;
            std::cerr << "Loaded player " << player.getPlayerBody().getName() << " from cache." << std::endl;
            return true;
        }
        else
        {
            std::cerr << "Player " << player.getPlayerBody().getName() << " is already connected." << std::endl;
            return false;
        }
    }
    
    std::string query = "SELECT * FROM players WHERE account_id = " + std::to_string(databaseid);
    
    int state = mysql_query(connection, query.c_str());
    
    if (state !=0)
    {
        std::cout << mysql_error(connection) << std::endl;
        return false;
    }
    
    result = mysql_store_result(connection);
    
    if (mysql_num_rows(result) == 0)
    {
        std::cerr << "Player from account " << databaseid << " not found." << std::endl;
        return false;
    }
    if (mysql_num_rows(result) > 1)
    {
        std::cerr << "More than one player occurence for database " << databaseid << "." << std::endl;
        return false;
    }
    
    while ( ( row=mysql_fetch_row(result)) != NULL )
    {
        Player newplayer;
        newplayer.load(databaseid, OnlineStatus::ONLINE);
        
        Body& newplayerBody = newplayer.getPlayerBody();
        
        newplayerBody.setName(row[1]);
        newplayerBody.setLevel(std::atoi(row[2]));
        newplayerBody.setExperience(std::atoi(row[3]));
        newplayerBody.setHealth(std::atoi(row[4]));
        newplayerBody.setMaxHealth(std::atoi(row[5]));
        newplayerBody.setStamina(std::atoi(row[6]));
        newplayerBody.setMaxStamina(std::atoi(row[7]));
        newplayerBody.setAttack(std::atoi(row[8]));
        newplayerBody.setArmor(std::atoi(row[9]));
        newplayerBody.setDefense(std::atoi(row[10]));
        newplayerBody.setSpeed(std::atoi(row[11]));
        newplayerBody.setBodyColor(static_cast<BodyColor>(std::atoi(row[12])));
        
        players_cache.insert(std::make_pair(databaseid, newplayer));
        
        player = newplayer;
        
        std::cerr << "Loaded player " << player.getPlayerBody().getName() << " from database." << std::endl;
        return true;
    }
    
    mysql_free_result(result);
    
    return false;
}


bool DatabaseIO::loadAccount(const uint32_t& databaseid, const uint32_t password, Account& account)
{
    if (accounts_cache.count(databaseid) == 1)
    {
        Account* cache_account = &accounts_cache[databaseid];
        
        if (cache_account->getPassword() == password && !cache_account->isConnected()){
            cache_account->connect();
            account = *cache_account;;
            std::cerr << "Loaded account " << account.getAccountID() << " from cache." << std::endl;
            return true;
        }
        else
        {
            if (cache_account->getPassword() != password)
                std::cerr << "Password incorrect" << std::endl;
            if (cache_account->isConnected())
                std::cerr << "Account already connected" << std::endl;
            return false;
        }
    }
    
    std::string query = "SELECT * FROM accounts WHERE id = " + std::to_string(databaseid) + " and password = " + std::to_string(password);
    
    int state = mysql_query(connection, query.c_str());
    
    if (state !=0)
    {
        std::cout << mysql_error(connection) << std::endl;
        return false;
    }
    
    result = mysql_store_result(connection);
    
    if (mysql_num_rows(result) == 0)
    {
        std::cerr << "Account " << databaseid << " not found, or password incorrect " << password << "." << std::endl;
        return false;
    }
    if (mysql_num_rows(result) > 1)
    {
        std::cerr << "More than one account occurence [" << databaseid << "]." << std::endl;
        return false;
    }
    
    while ( ( row=mysql_fetch_row(result)) != NULL )
    {
        Account newaccount;
        
        newaccount.load(databaseid, password, std::atoi(row[2]), OnlineStatus::ONLINE);
        
        accounts_cache.insert(std::make_pair(databaseid, newaccount));
        
        account = newaccount;
        
        std::cerr << "Loaded account " << account.getAccountID() << " from database." << std::endl;
        return true;
    }
    
    mysql_free_result(result);
    
    return false;
}

bool DatabaseIO::save(const Account& account)
{
    if (accounts_cache.count(account.getAccountID()) == 1)
		accounts_cache[account.getAccountID()] = account;
    
    std::string query = "UPDATE `accounts` SET `id`= " + std::to_string(account.getAccountID()) + ",`password`=  " + std::to_string(account.getPassword()) + ",`premium_days`= " + std::to_string(account.getPremiumDays()) + " WHERE id = " + std::to_string(account.getAccountID());
    
    int state = mysql_query(connection, query.c_str());
    
    if (state !=0)
    {
        std::cout << mysql_error(connection) << std::endl;
        return false;
    }
    
    std::cerr << "Account " << account.getAccountID() << " saved!" << std::endl;
    return true;
}

bool DatabaseIO::save(const Player& player)
{
    if (players_cache.count(player.getAccountID()) == 1)
        players_cache[player.getAccountID()] = player;
    
    const Body& body = player.getPlayerBody();
    
    std::string query = "UPDATE `players` SET ";
    query += "`name`='" + body.getName() + "',";
    query += "`level`='" + std::to_string(body.getLevel()) + "',";
    query += "`experience`='" + std::to_string(body.getExperience()) + "',";
    query += "`health`='" + std::to_string(body.getHealth()) + "',";
    query += "`max_health`='" + std::to_string(body.getMaxHealth()) + "',";
    query += "`stamina`='" + std::to_string(body.getStamina()) + "',";
    query += "`max_stamina`='" + std::to_string(body.getMaxStamina()) + "',";
    query += "`attack`='" + std::to_string(body.getAttack()) + "',";
    query += "`armor`='" + std::to_string(body.getArmor()) + "',";
    query += "`defense`='" + std::to_string(body.getDefense()) + "',";
    query += "`movement`='" + std::to_string(body.getMovementSpeed()) + "',";
    query += "`body_color`='" + std::to_string(static_cast<int>(body.getBodyColor())) + "'";
    query += " WHERE account_id = " + std::to_string(player.getAccountID());
    
    int state = mysql_query(connection, query.c_str());
    
    if (state !=0)
    {
        std::cout << mysql_error(connection) << std::endl;
        return false;
    }
    
    std::cerr << "Player " << body.getName() << " saved!" << std::endl;
    return true;
}
