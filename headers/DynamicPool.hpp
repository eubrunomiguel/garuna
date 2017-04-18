//
//  DynamicPool.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 12/18/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef DynamicPool_hpp
#define DynamicPool_hpp

#include <cstdlib>
#include <set>
#include <cassert>

template <class Type>
class DynamicPool {
public:
    DynamicPool()
    {
        size = 256;
        objects = new Type[size];
        for (int i = 0; i < size; i++)
            _available.insert(&objects[i]);
    }
    ~DynamicPool()
    {
        delete[] objects;
    }
    Type* create()
    {
        assert(!_available.empty());
        
        auto it = _available.begin();
        
        Type* newObject = *it;
        
        _used.insert(newObject);
        
        _available.erase(it);
        
        return newObject;
    }
    
    void destroy(Type& object)
    {
        auto it = _used.find(&object);
        if (it != _used.end())
        {
            _available.insert(&object);
            _used.erase(it);
        }
    }
public:
    Type* objects;
    std::set<Type*> _available;
    std::set<Type*> _used;
    uint16_t size;
};

#endif /* DynamicPool_hpp */
