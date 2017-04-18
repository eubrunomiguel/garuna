//
//  Map.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 3/11/17.
//  Copyright © 2017 d2server. All rights reserved.
//

#ifndef Map_hpp
#define Map_hpp

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>

class Object;

class Node
{
public:
    Node(const Vector3& currentposition, const Vector3& toposition, bool diagonal, Node* p) : parent(p), position(currentposition)
    {
        gvalue = (diagonal ? 14 : 10);
        hvalue =  abs(static_cast<int>(currentposition.mX - toposition.mX)) + abs(static_cast<int>(currentposition.mZ - toposition.mZ));
    }
    
    int fvalue() const {return hvalue + gvalue;}
    
    int hvalue;
    int gvalue; // Movementcost: 10 for horin/vert 14 for diagonal
    
    Vector3 position;
    
    Node* parent;
};

struct MapData
{
public:
    std::pair<uint16_t,uint16_t> dimension;
    std::pair<uint16_t,uint16_t> flag;
    std::pair<uint16_t,uint16_t> base;
    std::vector<std::pair<uint16_t,uint16_t>> respaw;
    std::vector<std::pair<uint16_t,uint16_t>> blockade;
    bool loaded = false;
    
};

class MapLoader
{
public:
    static MapLoader& get(){
        static MapLoader instance;
        return instance;
    }
    
    bool load(MapData& map)
    {
        if (cache.loaded){
              std::cerr << "Loading map... [CACHE]" << std::endl;
              map = cache;
              return true;
        }
        
        std::cerr << "Loading map..." << std::endl;
        
        bool error = false;
        
        mapdata = &map;
        
        std::ifstream infile("C:\\Users\\epvaverka\\Documents\\Visual Studio 2015\\Projects\\GarunaServer\\Release\\mapinformation.txt");
        
        if (infile.is_open())
        {
            std::string line;
            while (std::getline(infile, line))
            {
                if (mapdictionary.count(line))
                {
                    std::string key = line;
                    
                    std::getline(infile, line);
                    while (!isEnd(line))
                    {
                        (this->*mapdictionary[key])(line);
                        std::getline(infile, line);
                    }
                }
            }

        }
        else{
            std::cerr << "Could not open file!" << std::endl;
            error = true;
        }
        
        std::string status = (error ? "[ERROR]" : "[SUCCESS]");
        std::cerr << "Loading map... " << status << std::endl;
        
        mapdata->loaded = true;
        
        cache = *mapdata;
        mapdata = nullptr;
        
        return error;
    }
    
protected:
    
    void loadDimension(std::string& line)
    {
        std::istringstream line_reader(line);
        
        line_reader >> mapdata->dimension.first >> mapdata->dimension.second;
        
        //std::cerr << "Loading dimension" << mapdata->dimension.first << "|" << mapdata->dimension.second << std::endl;
    }
    
    void loadFlag(std::string& line)
    {
        std::istringstream line_reader(line);
    
        line_reader >> mapdata->flag.first >> mapdata->flag.second;
        
        //std::cerr << "Loading flag" << mapdata->flag.first << "|" << mapdata->flag.second << std::endl;
    }
    
    void loadSafezone(std::string& line)
    {
        std::istringstream line_reader(line);
        
        line_reader >> mapdata->base.first >> mapdata->base.second;
        
        //std::cerr << "Loading safezone" << mapdata->base.first << "|" << mapdata->base.second << std::endl;
    }
    
    void loadCreature(std::string& line)
    {
        std::istringstream line_reader(line);
        
        uint16_t x, z;
        
        line_reader >> x >> z;
        
        mapdata->respaw.push_back(std::pair<uint16_t, uint16_t>(x,z));
        
        //std::cerr << "Loading creature" << x << "|" << z << std::endl;
    }
    
    void loadBlockades(std::string& line)
    {
        std::istringstream line_reader(line);
        
        uint16_t x, z;
        
        line_reader >> x >> z;
        
        mapdata->blockade.push_back(std::pair<uint16_t, uint16_t>(x,z));
        
        //std::cerr << "Loading blockade" << x << "|" << z << std::endl;
    }
    
    bool isEnd(std::string& line) const
    {
        return line == "End";
    }
    
private:
    
    MapData  cache;
    
    MapData* mapdata;

    std::map<std::string, void(MapLoader::*)(std::string&)> mapdictionary;
    
    MapLoader() : mapdata(nullptr)
    {
        mapdictionary["Map"] = &MapLoader::loadDimension;
        mapdictionary["Flag"] = &MapLoader::loadFlag;
        mapdictionary["Safezone"] = &MapLoader::loadSafezone;
        mapdictionary["Blockades"] = &MapLoader::loadBlockades;
        mapdictionary["Creature"] = &MapLoader::loadCreature;
    }
};

#endif /* Map_hpp */
