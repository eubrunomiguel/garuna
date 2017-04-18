//
//  Object.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 2/19/17.
//  Copyright © 2017 d2server. All rights reserved.
//

#ifndef Object_hpp
#define Object_hpp

#include "Consts.hpp"
#include "Math.hpp"
#include "MemoryStream.hpp"

class Object
{
    friend class InputMemoryBitStream;
public:
    Object() = delete;
    Object(const ObjectType& t) : guid(0), type(t), position(Vector3()), mapnext(nullptr), mapprev(nullptr) {}
    
    Object*     getMapNext () const                 { return mapnext; }
    void        setMapNext (Object* next)           { mapnext = next; }
    Object*     getMapPrev () const                 { return mapprev; }
    void        setMapPrev (Object* prev)           { mapprev = prev; }
    
    void        setPosition   (const Vector3& pos) {position = pos;}
    void        setGUID       (const uint32_t& g)      {guid = g;}
    
    const ObjectType& getType()     const {return type;}
    const Vector3&    getPosition() const {return position;}
    const uint32_t&       getGUID ()    const  {return guid;}
    
private:
    uint32_t        guid;
    ObjectType  type;
    Vector3     position;
    Object*     mapnext;
    Object*     mapprev;
};

#endif /* Object_hpp */
