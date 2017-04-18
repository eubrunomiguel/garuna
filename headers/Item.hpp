//
//  Item.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 2/18/17.
//  Copyright © 2017 d2server. All rights reserved.
//

#ifndef Item_hpp
#define Item_hpp

#include "Consts.hpp"
#include "Math.hpp"
#include "Object.hpp"
#include "MemoryStream.hpp"
#include <string>

class Item;
class Body;

class Backpack
{
public:
    Backpack()
    {
        for (uint8_t i = 0; i < getMaxItems(); i++)
            items[i] = nullptr;
    }
    
    Item** getItems() {return items;}
    
    const uint8_t getMaxItems() const {return BACKPACKMAXISZE;}
    
    bool addItem(Item* item) {
        for (uint8_t i = 0; i < getMaxItems(); i++)
        {
            if (items[i] == nullptr){
                items[i] = item;
                return true;
            }
        }
        return false;
    }
    
    void removeItem(Item* item)
    {
        for (uint8_t i = 0; i < getMaxItems(); i++)
            if (items[i] == item)
                items[i] = nullptr;
    }
    
private:
    Item* items[BACKPACKMAXISZE];
};

class Item : public Object
{
    friend class InputMemoryBitStream;
public:
    void          setOptions(const ItemFlags& opt)  {options |= opt;}
    void          setAction (const ItemAction& act) {action = act;}
    void          setSlot   (const ItemSlots& s) {slot = s;}
    
    const bool         hasOption (const ItemFlags& opt) const  {return options & opt;}
    
    const uint8_t&     getCount()      const {return count;}
    const ItemFlags&   getOptions()    const {return options;}
    const ItemSlots&   getSlot()       const {return slot;}
    const ItemAction&  getAction()     const {return action;}
    const std::string& getName()       const {return name;}
    
    // Game
    const bool         isAmmo()        const {return options  & ItemFlags::AMMUNITION;}
    const bool         isTemporary()   const {return (options & ItemFlags::TEMPORARY);}
    const bool         isPickable ()   const {return (options & ItemFlags::PICKABLE);}
    const bool         isConsumable () const {return (options & ItemFlags::CONSUMABLE);}
    const bool         isBlockable()   const {return !isConsumable() && !isPickable();}
    
    const bool         isExpired()     const {return lifetime_fps <= 0;}
    
    const uint16_t     getLifeTime()   const {return lifetime_fps;}
    const uint32_t     getOwner()      const {return owner;}
    const float&       getAngle()      const {return angle;}
    const uint32_t&    getLifeChange() const {return lifechange;}
    const uint8_t      getDropChance() const {return dropchance;}
    
    void               doExpire()                   {lifetime_fps = 0;}
    void               fps()                        {lifetime_fps--;}
    void               setDropChance(uint8_t c)     {dropchance = c;}
    void               setLifeTime(uint16_t fps)    {lifetime_fps = fps;}
    void               setOwner(const uint32_t& body) {owner = body;}
    void               setAngle(float a)            {angle = a;}
    void               setLifeChange(uint32_t a)    {lifechange = a;}
    void               setName(const std::string& n){name = n;}
    
    
    Item() :  Object(ObjectType::ITEM), name(""), count(0), options(ItemFlags::NONE), action(ItemAction::NONE), slot(ItemSlots::NONE), dropchance(0), owner(0), lifetime_fps(0), lifechange(0), angle(0) {}
    
private:
    // Serializable
    uint32_t        owner;
    std::string name;
    uint8_t     count;
    ItemFlags   options;
    ItemSlots   slot;
    ItemAction  action;
    
    // Game
    uint8_t     dropchance;
    uint16_t    lifetime_fps;

	float	    angle;
	uint32_t    lifechange;
};

#endif /* Item_hpp */
