//
//  MemoryStream.hpp
//  serialization
//
//  Created by Bruno Macedo Miguel on 9/8/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef MemoryStream_hpp
#define MemoryStream_hpp

#include <iostream>
#include <cstdint>
#include <string>
#include "Math.hpp"
#include "Mail.hpp"
#include "NetworkData.hpp"

class Item;
class Body;

inline uint32_t ConvertToFixed( float inNumber, float inMin, float inPrecision )
{
    return static_cast< int > ( ( inNumber - inMin ) / inPrecision );
}

inline float ConvertFromFixed( uint32_t inNumber, float inMin, float inPrecision )
{
    return static_cast< float >( inNumber ) * inPrecision + inMin;
}

class OutputMemoryBitStream
{
public:
    
    OutputMemoryBitStream() : mParcel(nullptr){}
    
    OutputMemoryBitStream(MessageParcel* message) : mParcel(message) {}
    
    ~OutputMemoryBitStream(){mParcel = nullptr;}
    
    const uint32_t getRemainingBitCount() {return (MAXPACKETBYTESIZE * 8) - mParcel->getBitHead();}

    void        load( MessageParcel* message)
    {
        mParcel = message;
    }
    
    void		WriteBits( uint8_t inData, uint32_t inBitCount );
    void		WriteBits( const void* inData, uint32_t inBitCount );
    
    uint32_t	GetBitLength()		const	{ return mParcel->getBitHead(); }
    uint32_t	GetByteLength()		const	{ return ( mParcel->getBitHead() + 7 ) >> 3; }
    
    void WriteBytes( const void* inData, uint32_t inByteCount ) { WriteBits( inData, inByteCount << 3 ); }
    
    void 		Write( bool inData ) { WriteBits( &inData, 8 ); }
    
    void		Write( const Vector3& inVector );

    void		Write( const PlayerData& player)
    {
        Write(player.uint32_t);
        Write(player.name);
    }
    
    void		Write( const AccountData& account)
    {
        Write(account.account);
        Write(account.password);
    }
    
    
    void		Write( const Quaternion& inQuat );
    
    template< typename T >
    void Write( T inData, uint32_t inBitCount = sizeof( T ) * 8 )
    {
        static_assert( std::is_arithmetic< T >::value ||
                      std::is_enum< T >::value,
                      "Generic Write only supports primitive data types" );
        WriteBits( &inData, inBitCount );
    }
    
    void Write( const std::string& inString )
    {
        uint8_t elementCount = static_cast< uint8_t >( inString.size() );
        Write( elementCount );
        for( const auto& element : inString )
        {
            Write( element );
        }
    }
    
    void Write( const ParcelIdentification& data )
    {
        Write(data.notification_id);
        Write(data.user_send_id);
    }
    
    void Write(CreatureDeadData& data )
    {
        Write(data.creatureguid);
        Write(data.killerguid);
    }
    
    void Write( const MovementCardinalData& data )
    {
        Write(data.mGUID);
        Write(data.mToDirection);
    }
    
    void Write( const MovementCoordinateData& data )
    {
        Write(data.mGUID);
        Write(data.mToPosition);
        Write(data.mInstant);
    }
    
    void Write( const MovementRotationData& data )
    {
        Write(data.mGUID);
        Write(data.mRotation);
    }
    
    void Write( const GameData& data )
    {
		Write(data.guid);
		Write(data.name);
        Write(data.uniqueid);
        Write(data.currentlevel);
        Write(data.message);
        Write(data.players);
        Write(data.points);
        Write(data.time);
		Write(data.monsterskilled);
		Write(data.won);
    }
    
    void Write( const BodyDespawnData& data )
    {
        Write(data.guid);
    }
    
    void Write( const ShootTargetData& data )
    {
        Write(data.mGUID);
        Write(data.mLocal);
    }
    
    void Write( const ExperienceData& data )
    {
        Write(data.level);
        Write(data.experience);
    }
    
    void Write( const LifeChangeData& data )
    {
        Write(data.responsible);
        Write(data.target);
        Write(data.value);
        Write(data.aggresive);
    }
    
    void Write( const Item& data );
    
    void Write( const Body& data );
    
private:
    MessageParcel* mParcel;
};

class InputMemoryBitStream
{
public:
    
    InputMemoryBitStream() : mParcel(nullptr), mIntegrity(true){}
    InputMemoryBitStream(MessageParcel* message) : mParcel(message) {}
    ~InputMemoryBitStream(){mParcel = nullptr;}
    
    void        load( MessageParcel* message)
    {
        mIntegrity = true;
        mParcel = message;
    }
    
    const uint32_t getRemainingBitCount() {return mParcel->getBitSize() - mParcel->getBitHead();}
    
    const bool  messageSuccessfullyRead() const {return mIntegrity;}
    
    void		ReadBits( uint8_t& outData, uint32_t inBitCount );
    void		ReadBits( void* outData, uint32_t inBitCount );
    
    void		ReadBytes( void* outData, uint32_t inByteCount )		{ ReadBits( outData, inByteCount << 3 ); }
    
    void		Read( uint32_t& outData, uint32_t inBitCount = 32 )		{ ReadBits( &outData, inBitCount ); }
    void		Read( int& outData, uint32_t inBitCount = 32 )			{ ReadBits( &outData, inBitCount ); }
    void		Read( float& outData )									{ ReadBits( &outData, 32 ); }
    
    void		Read( uint16_t& outData, uint32_t inBitCount = 16 )		{ ReadBits( &outData, inBitCount ); }
    void		Read( int16_t& outData, uint32_t inBitCount = 16 )		{ ReadBits( &outData, inBitCount ); }
    
    void		Read( uint8_t& outData, uint32_t inBitCount = 8 )		{ ReadBits( &outData, inBitCount ); }
    void		Read( bool& outData )									{ ReadBits( &outData, 8 ); }
    
    void		Read( Quaternion& outQuat );
    
    void Read( Vector3& inVector );
    
    template< typename T >
    void Read( T& inData, uint32_t inBitCount = sizeof( T ) * 8 )
    {
        static_assert( std::is_arithmetic< T >::value ||
                      std::is_enum< T >::value,
                      "Generic Read only supports primitive data types" );
        ReadBits( &inData, inBitCount );
    }
    
    void        loadHeader()
    {
        Read(mParcel->getCurrentHeader());
    }
    
    void Read(std::string& inString )
    {
        uint8_t elementCount;
        Read( elementCount );
        inString.resize( elementCount );
        for( auto& element : inString )
        {
            Read( element );
        }
    }
    
    void Read(ParcelIdentification& data )
    {
        Read(data.notification_id);
        Read(data.user_send_id);
    }
    
    void Read(CreatureDeadData& data )
    {
        Read(data.creatureguid);
        Read(data.killerguid);
    }
    
    void Read(ExperienceData& data )
    {
        Read(data.level);
        Read(data.experience);
    }
    
    void Read(BodyDespawnData& data )
    {
        Read(data.guid);
    }
    
    void Read( MovementCardinalData& data )
    {
        Read(data.mGUID);
        Read(data.mToDirection);
    }
    
    void Read( MovementCoordinateData& data )
    {
        Read(data.mGUID);
        Read(data.mToPosition);
        Read(data.mInstant);
    }
    
    void Read( MovementRotationData& data )
    {
        Read(data.mGUID);
        Read(data.mRotation);
    }
    
    void Read( ShootTargetData& data )
    {
        Read(data.mGUID);
        Read(data.mLocal);
    }
    
    void Read( PlayerData& player)
    {
        Read(player.uint32_t);
        Read(player.name);
    }
    
    void Read(AccountData& account)
    {
        Read(account.account);
        Read(account.password);
    }
    
    void Read(LifeChangeData& data)
    {
        Read(data.responsible);
        Read(data.target);
        Read(data.value);
        Read(data.aggresive);
    }
    
    void Read(GameData& data )
    {
		Read(data.guid);
		Read(data.name);
        Read(data.uniqueid);
        Read(data.currentlevel);
        Read(data.message);
        Read(data.players);
        Read(data.points);
        Read(data.time);
		Read(data.monsterskilled);
		Read(data.won);
    }
    
    void Read( Item& data );
    
    void Read(Body& data);

    
private:
    MessageParcel*  mParcel;
    bool            mIntegrity;
};

#endif /* MemoryStream_hpp */
