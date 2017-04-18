//
//  core.cpp
//  gameserver
//
//  Created by Bruno Macedo Miguel on 8/25/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#include "networkserver.hpp"
#include "GameWorld.hpp"
#include "Factory.hpp"
#include "Database.hpp"

int main()
{
    std::cout << "Setting up Server..." << std::endl;
    
    try
    {
        
        std::cout << "Creating GameWorld...";
        GameWorld gameworld;
        std::cout << "[DONE]" << std::endl;
        
        std::cout << "Connecting to Database...";
        DatabaseIO database;
        std::cout << "[DONE]" << std::endl;
        
        std::cout << "Creating Factory...";
        Factory factory(gameworld, database);
        std::cout << "[DONE]" << std::endl;
        
        std::cout << "Loading Factory...";
        gameworld.loadFactory(&factory);
        std::cout << "[DONE]" << std::endl;
        
        std::cout << "Loading MailBox...";
        MailBox mail;
        std::cout << "[DONE]" << std::endl;
        
        std::cout << "Loading NotificationCenter...";
        MessageNotificationCenter notificationCenter(mail);
        std::cout << "[DONE]" << std::endl;
        
        std::cout << "Loading LanHouse...";
        LanHouse lan(mail, notificationCenter, factory);
        std::cout << "[DONE]" << std::endl;
        
        std::cout << "Opening Server at port " << PORT << "...";
        boost::asio::io_service io_service;
        NetworkServer server(io_service, PORT, mail, lan);
        std::cout << "[DONE]" << std::endl;
        
        
        std::cout << "Running gameloop..." << std::endl;
        
        
        while(true)
        {
            // update input
            io_service.poll();
            // update computer
            lan.update();
            // update output
            notificationCenter.update(server);
            // update world
            gameworld.update();
        }
        /*
        for (int i =0; i < 10000000; i++)
        {
            io_service.poll();
            lan.update();
        }
         */
        
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    
    
    std::cout << "Server is shutdown!" << std::endl;
    
    return 0;
}
