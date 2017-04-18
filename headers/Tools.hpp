//
//  Tools.h
//  server
//
//  Created by Bruno Macedo Miguel on 10/8/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef Tools_h
#define Tools_h

#include <cstdlib>
#include <iostream>
#include <string>
#include <chrono>

inline int64_t OTSYS_TIME()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

class Tools
{
public:
    static void FastCerr(std::string text)
    {
        std::cerr << text << std::endl;
    }
};

class Timer
{
public:
    Timer()
    {
        start = std::chrono::system_clock::now();
        end = std::chrono::system_clock::now();
    }
    inline void   reset()
    {
        start = std::chrono::system_clock::now();
        end = std::chrono::system_clock::now();
    }
    inline double getElapsedSeconds()
    {
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        return elapsed_seconds.count();
    }
private:
    std::chrono::time_point<std::chrono::system_clock> start, end;
};

#endif /* Tools_h */
