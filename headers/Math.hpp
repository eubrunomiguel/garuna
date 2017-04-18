//
//  Math.h
//  serialization
//
//  Created by Bruno Macedo Miguel on 9/8/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef Math_h
#define Math_h

#include <math.h>
#include <vector>
#include "Consts.hpp"
#include <string>

struct CompassDirection
{
public:
    CompassDirection(MovementDirection d, float t) : direction(d), to(t) {}
    MovementDirection direction;
    float             to;
    
    bool              doesPoint(float angle)
    {
        if (direction == MovementDirection::NORTH &&  angle > 360 - mStdDev) return true;
        
        return angle >= (to - mStdDev) && angle <= (to + mStdDev);
    }
    
    const float mStdDev = 22.5;
};

class Compass
{
public:
    static Compass& get(){
        static Compass instance;
        return instance;
    }
    
    MovementDirection getDirection(float angle)
    {
        for (auto& dir : mDirections)
            if (dir->doesPoint(angle))
                return dir->direction;
        
        return MovementDirection::NONE;
    }
    
    bool isDiagonal(const MovementDirection& dir)
    {
        return (dir == MovementDirection::NW || dir == MovementDirection::NE || dir == MovementDirection::SE || dir == MovementDirection::SW);
    }
    
    void getOppositeDirections(std::vector<MovementDirection>& directions, const MovementDirection& from)
    {
        switch(from)
        {
            case MovementDirection::NORTH:
                directions.push_back(MovementDirection::SOUTH);
                directions.push_back(MovementDirection::SE);
                directions.push_back(MovementDirection::SW);
                break;
            case MovementDirection::NE:
                directions.push_back(MovementDirection::WEST);
                directions.push_back(MovementDirection::SOUTH);
                directions.push_back(MovementDirection::SW);
                break;
            case MovementDirection::NW:
                directions.push_back(MovementDirection::EAST);
                directions.push_back(MovementDirection::SOUTH);
                directions.push_back(MovementDirection::SE);
                break;
            case MovementDirection::SOUTH:
                directions.push_back(MovementDirection::NORTH);
                directions.push_back(MovementDirection::NW);
                directions.push_back(MovementDirection::NE);
                break;
            case MovementDirection::SE:
                directions.push_back(MovementDirection::NORTH);
                directions.push_back(MovementDirection::WEST);
                directions.push_back(MovementDirection::NW);
                break;
            case MovementDirection::SW:
                directions.push_back(MovementDirection::EAST);
                directions.push_back(MovementDirection::NORTH);
                directions.push_back(MovementDirection::NW);
                break;
            case MovementDirection::EAST:
                directions.push_back(MovementDirection::WEST);
                directions.push_back(MovementDirection::SW);
                directions.push_back(MovementDirection::NW);
                break;
            case MovementDirection::WEST:
                directions.push_back(MovementDirection::NE);
                directions.push_back(MovementDirection::SE);
                directions.push_back(MovementDirection::EAST);
                break;
            default:
                break;
        }
    }
    
    // delete copy and move constructors and assign operators
    Compass(Compass const&) = delete;             // Copy construct
    Compass(Compass&&) = delete;                  // Move construct
    Compass& operator=(Compass const&) = delete;  // Copy assign
    Compass& operator=(Compass &&) = delete;      // Move assign
    
protected:
    Compass()
    {
        mDirections.push_back(new CompassDirection(MovementDirection::NORTH, 0));
        mDirections.push_back(new CompassDirection(MovementDirection::SOUTH, 180));
        mDirections.push_back(new CompassDirection(MovementDirection::WEST,  270));
        mDirections.push_back(new CompassDirection(MovementDirection::EAST,  90));
        mDirections.push_back(new CompassDirection(MovementDirection::NW,    315));
        mDirections.push_back(new CompassDirection(MovementDirection::NE,    45));
        mDirections.push_back(new CompassDirection(MovementDirection::SW,    225));
        mDirections.push_back(new CompassDirection(MovementDirection::SE,    135));
    }
    ~Compass()
    {
        for (auto dir : mDirections)
        {
            delete dir;
        }
    }
private:
    std::vector <CompassDirection*> mDirections;
};




class Vector3
{
public:
    
    float		mX, mY, mZ;

    Vector3( float x, float y, float z ) :
    mX( x ),
    mY( y ),
    mZ( z )
    {}
    
    Vector3() :
    mX( 0.0f ),
    mY( 0.0f ),
    mZ( 0.0f )
    {}
    
    void Set( float x, float y, float z )
    {
        mX = x;
        mY = y;
        mZ = z;
    }
    
    static Vector3 Zero()
    {
        return Vector3(0,0,0);
    }
    
    static float Distance (const Vector3& l, const Vector3& r)
    {
        return sqrt(pow((l.mX - r.mX),2) + pow((l.mZ - r.mZ),2));
    }
    
    static Vector3 Move(const Vector3& pos, MovementDirection direction)
    {
        Vector3 position = pos;
        
        switch(direction)
        {
            case MovementDirection::NORTH:
                position += Vector3(0,0,1);
                break;
            case MovementDirection::SOUTH:
                position += Vector3(0,0,-1);
                break;
            case MovementDirection::EAST:
                position += Vector3(1,0,0);
                break;
            case MovementDirection::WEST:
                position += Vector3(-1,0,0);
                break;
            case MovementDirection::NE:
                position += Vector3(1,0,1);
                break;
            case MovementDirection::NW:
                position += Vector3(-1,0,1);
                break;
            case MovementDirection::SE:
                position += Vector3(1,0,-1);
                break;
            case MovementDirection::SW:
                position += Vector3(-1,0,-1);
                break;
            default:
                break;
        }
        return position;
    }
    
    static bool ShareCell( const Vector3& inLeft, const Vector3& inRight )
    {
        return  (floor(inLeft.mX) == floor(inRight.mX) && floor(inLeft.mZ) == floor(inRight.mZ));
    }
    
    friend bool operator==( const Vector3& inLeft, const Vector3& inRight )
    {
        return (inLeft.mX == inRight.mX && inLeft.mZ == inRight.mZ && inLeft.mY == inRight.mY);
    }
    
    friend bool operator!=( const Vector3& inLeft, const Vector3& inRight )
    {
        return (inLeft.mX != inRight.mX || inLeft.mZ != inRight.mZ);
    }
    
    friend Vector3 operator+( const Vector3& inLeft, const Vector3& inRight )
    {
        return Vector3( inLeft.mX + inRight.mX, inLeft.mY + inRight.mY, inLeft.mZ + inRight.mZ );
    }
    
    friend Vector3 operator-( const Vector3& inLeft, const Vector3& inRight )
    {
        return Vector3( inLeft.mX - inRight.mX, inLeft.mY - inRight.mY, inLeft.mZ - inRight.mZ );
    }
    
    // Component-wise multiplication
    friend Vector3 operator*( const Vector3& inLeft, const Vector3& inRight )
    {
        return Vector3( inLeft.mX * inRight.mX, inLeft.mY * inRight.mY, inLeft.mZ * inRight.mZ );
    }
    
    // Scalar multiply
    friend Vector3 operator*( float inScalar, const Vector3& inVec )
    {
        return Vector3( inVec.mX * inScalar, inVec.mY * inScalar, inVec.mZ * inScalar );
    }
    
    friend Vector3 operator*( const Vector3& inVec, float inScalar )
    {
        return Vector3( inVec.mX * inScalar, inVec.mY * inScalar, inVec.mZ * inScalar );
    }
    
    Vector3& operator*=( float inScalar )
    {
        mX *= inScalar;
        mY *= inScalar;
        mZ *= inScalar;
        return *this;
    }
    
    Vector3& operator+=( const Vector3& inRight )
    {
        mX += inRight.mX;
        mY += inRight.mY;
        mZ += inRight.mZ;
        return *this;
    }
    
    Vector3& operator-=( const Vector3& inRight )
    {
        mX -= inRight.mX;
        mY -= inRight.mY;
        mZ -= inRight.mZ;
        return *this;
    }
    
    float Length()
    {
        return sqrtf( mX * mX + mY * mY + mZ * mZ );
    }
    
    float LengthSq()
    {
        return mX * mX + mY * mY + mZ * mZ;
    }
    
    float Length2D()
    {
        return sqrtf( mX * mX + mY * mY );
    }
    
    float LengthSq2D()
    {
        return mX * mX + mY * mY;
    }
    
    void Normalize()
    {
        float length = Length();
        mX /= length;
        mY /= length;
        mZ /= length;
    }
    
    void Normalize2D()
    {
        float length = Length2D();
        mX /= length;
        mY /= length;
    }
    
    static float Dot( const Vector3& inLeft, const Vector3& inRight )
    {
        return ( inLeft.mX * inRight.mX + inLeft.mY * inRight.mY + inLeft.mZ * inRight.mZ );
    }
    
    static float Dot2D( const Vector3& inLeft, const Vector3& inRight )
    {
        return ( inLeft.mX * inRight.mX + inLeft.mY * inRight.mY );
    }
    
    static Vector3 Cross( const Vector3& inLeft, const Vector3& inRight )
    {
        Vector3 temp;
        temp.mX = inLeft.mY * inRight.mZ - inLeft.mZ * inRight.mY;
        temp.mY = inLeft.mZ * inRight.mX - inLeft.mX * inRight.mZ;
        temp.mZ = inLeft.mX * inRight.mY - inLeft.mY * inRight.mX;
        return temp;
    }
    
    // point between the line drawed between 2 vectors
    static Vector3 Lerp( const Vector3& inA, const Vector3& inB, float t )
    {
        return Vector3( inA + t * ( inB - inA ) );
    }
    
    static void Print(const std::string& legend, const Vector3& vec, bool round)
    {
        std::cout << legend;
        if (round)
           std::cerr << " {" << floor(vec.mX) << " : " << floor(vec.mZ) << "}" << std::endl;
        else
           std::cerr << " {" << vec.mX << " : " << vec.mZ << "}" << std::endl;
    }
    
    static Vector3 offSet()
    {
        return Vector3(0.5f,0,0.5f);
    }
    
    static const Vector3 UnitX;
    static const Vector3 UnitY;
    static const Vector3 UnitZ;
};


class Quaternion
{
public:
    
    float		mX, mY, mZ, mW;
    
};


template< int tValue, int tBits >
struct GetRequiredBitsHelper
{
    enum { Value = GetRequiredBitsHelper< ( tValue >> 1 ), tBits + 1 >::Value };
};

template< int tBits >
struct GetRequiredBitsHelper< 0, tBits >
{
    enum { Value = tBits };
};

template< int tValue >
struct GetRequiredBits
{
    enum { Value = GetRequiredBitsHelper< tValue, 0 >::Value };
};


namespace Colors
{
    static const Vector3 Black( 0.0f, 0.0f, 0.0f );
    static const Vector3 White( 1.0f, 1.0f, 1.0f );
    static const Vector3 Red( 1.0f, 0.0f, 0.0f );
    static const Vector3 Green( 0.0f, 1.0f, 0.0f );
    static const Vector3 Blue( 0.0f, 0.0f, 1.0f );
    static const Vector3 LightYellow( 1.0f, 1.0f, 0.88f );
    static const Vector3 LightBlue( 0.68f, 0.85f, 0.9f );
    static const Vector3 LightPink( 1.0f, 0.71f, 0.76f );
    static const Vector3 LightGreen( 0.56f, 0.93f, 0.56f );
}

namespace RobotMath
{
    const float PI = 3.1415926535f;
    
    inline float ToDegrees( const float& inRadians )
    {
        float degree = inRadians * 180.0f / PI;
        
        if (degree < 0)
            degree += 360;
        
        return degree;
    }
    
    inline bool TryChance(const int& percentage)
    {
        int pc = rand() % 101;
        return pc <= percentage;
    }
    
    inline MovementDirection getDirection(const Vector3& lhs, const Vector3& rhs)
    {
        MovementDirection direction = MovementDirection::NONE;
        Vector3           newVector = lhs - rhs;
        newVector.Normalize();
        
        float degrees = RobotMath::ToDegrees(atan2(newVector.mX, newVector.mZ));
        
        direction = Compass::get().getDirection(degrees);
        
        return direction;
    }
    
    inline float getRadians(const Vector3& lhs, const Vector3& rhs)
    {
        Vector3 toplace  = lhs - rhs;
        toplace.Normalize();
        float degrees = ToDegrees(atan2(toplace.mX, toplace.mZ));
        return degrees * PI / 180.0f;
    }
}

#endif /* Math_h */
