//
//  Object.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 12/12/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef Object_hpp
#define Object_hpp

class Body;

class IObjectPool
{
public:
    virtual Body* create() = 0;
    virtual void recycle(Body*) = 0;
};

#endif /* Object_hpp */
