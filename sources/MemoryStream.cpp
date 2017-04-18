#include "MemoryStream.hpp"
#include "Item.hpp"
#include "Body.hpp"

void OutputMemoryBitStream::WriteBits( uint8_t inData, uint32_t inBitCount )
{
    uint32_t nextBitHead = mParcel->getBitHead() + static_cast< uint32_t >( inBitCount );
    
    //calculate the byteOffset into our buffer
    //by dividing the head by 8
    //and the bitOffset by taking the last 3 bits
    uint32_t byteOffset = mParcel->getBitHead() >> 3;
    uint32_t bitOffset = mParcel->getBitHead() & 0x7;
    
    uint8_t currentMask = ~( 0xff << bitOffset );
    mParcel->getMessageBuffer()[ byteOffset ] = ( mParcel->getMessageBuffer()[ byteOffset ] & currentMask ) | ( inData << bitOffset );
    
    //calculate how many bits were not yet used in
    //our target byte in the buffer
    uint32_t bitsFreeThisByte = 8 - bitOffset;
    
    //if we needed more than that, carry to the next byte
    if( bitsFreeThisByte < inBitCount )
    {
        //we need another byte
        mParcel->getMessageBuffer()[ byteOffset + 1 ] = inData >> bitsFreeThisByte;
    }
    
    mParcel->getBitHead() = nextBitHead;
    
    uint16_t totalBitsWritten = mParcel->getBitSize() + inBitCount;
    mParcel->setBitSize(totalBitsWritten);
}

void InputMemoryBitStream::ReadBits( uint8_t& outData, uint32_t inBitCount )
{
    uint32_t byteOffset = mParcel->getBitHead() >> 3;
    uint32_t bitOffset = mParcel->getBitHead() & 0x7;
    
    outData = static_cast< uint8_t >( mParcel->getMessageBuffer()[ byteOffset ] ) >> bitOffset;
    
    
    uint32_t bitsFreeThisByte = 8 - bitOffset;
    if( bitsFreeThisByte < inBitCount )
    {
        //we need another byte
        outData |= static_cast< uint8_t >( mParcel->getMessageBuffer()[ byteOffset + 1 ] ) << bitsFreeThisByte;
    }
    
    //don't forget a mask so that we only read the bit we wanted...
    outData &= ( ~( 0x00ff << inBitCount ) );
    
    mParcel->getBitHead() += inBitCount;
}

void OutputMemoryBitStream::WriteBits( const void* inData, uint32_t inBitCount )
{
    // Make sure we have enough data to be read
    if (getRemainingBitCount() >= inBitCount)
    {
        const char* srcByte = static_cast< const char* >( inData );
        //write all the bytes
        while( inBitCount > 8 )
        {
            WriteBits( *srcByte, 8 );
            ++srcByte;
            inBitCount -= 8;
        }
        //write anything left
        if( inBitCount > 0 )
        {
            WriteBits( *srcByte, inBitCount );
        }
    }
}

void InputMemoryBitStream::ReadBits( void* outData, uint32_t inBitCount )
{
    // Make sure we have enough data to be read
    if (getRemainingBitCount() >= inBitCount)
    {
        uint8_t* destByte = reinterpret_cast< uint8_t* >( outData );
        //write all the bytes
        while( inBitCount > 8 )
        {
            ReadBits( *destByte, 8 );
            ++destByte;
            inBitCount -= 8;
        }
        //write anything left
        if( inBitCount > 0 )
        {
            ReadBits( *destByte, inBitCount );
        }
    }
    else
    {
        // Data is corrupt
        std::cerr << "Message has been corrupted, parcel id: " << mParcel->getID() << std::endl;
        mIntegrity = false;
    }
}

void OutputMemoryBitStream::Write( const Vector3& inVector )
{
    // FLOORED
    //uint16_t x = static_cast<uint16_t>(floor(inVector.mX));
    //uint16_t z = static_cast<uint16_t>(floor(inVector.mZ));
    //Write(x);
    //Write(z);
    
    // NORMAL
    //Write( inVector.mX );
    //Write( inVector.mY );
    //Write( inVector.mZ );
    
    // COMPRESSED
    Write(ConvertToFixed(inVector.mX, 0, 0.1f), 16);
    Write(ConvertToFixed(inVector.mZ, 0, 0.1f), 16);
}

void InputMemoryBitStream::Read( Vector3& outVector )
{
    // FLOORED
    //uint16_t x,z;
    //Read(x);
    //Read(z);
    //outVector.mX = x;
    //outVector.mZ = z;
    
    // NORMAL
    //Read( outVector.mX );
    //Read( outVector.mY );
    //Read( outVector.mZ );
    
    // COMPRESSED
    uint32_t val = 0;
    
    Read( val, 16 );
    outVector.mX = ConvertFromFixed( val, 0, 0.1f );
    Read( val, 16 );
    outVector.mZ = ConvertFromFixed( val, 0, 0.1f );
}

void OutputMemoryBitStream::Write( const Quaternion& inQuat )
{
    float precision = ( 2.f / 65535.f );
    Write( ConvertToFixed( inQuat.mX, -1.f, precision ), 16 );
    Write( ConvertToFixed( inQuat.mY, -1.f, precision ), 16 );
    Write( ConvertToFixed( inQuat.mZ, -1.f, precision ), 16 );
    Write( inQuat.mW < 0 );
}


void InputMemoryBitStream::Read( Quaternion& outQuat )
{
    float precision = ( 2.f / 65535.f );
    
    uint32_t f = 0;
    
    Read( f, 16 );
    outQuat.mX = ConvertFromFixed( f, -1.f, precision );
    Read( f, 16 );
    outQuat.mY = ConvertFromFixed( f, -1.f, precision );
    Read( f, 16 );
    outQuat.mZ = ConvertFromFixed( f, -1.f, precision );
    
    outQuat.mW = sqrtf( 1.f -
                       outQuat.mX * outQuat.mX +
                       outQuat.mY * outQuat.mY +
                       outQuat.mZ * outQuat.mZ );
    bool isNegative;
    Read( isNegative );
    
    if( isNegative )
    {
        outQuat.mW *= -1;
    }
}

void OutputMemoryBitStream::Write( const Body& data )
{
    Write(data.getGUID());
    Write(data.getBodyType());
    Write(data.getName());
    
    Write(data.getLevel());
    Write(data.getExperience());
    
    Write(data.getHealth());
    Write(data.getMaxHealth());
    
    Write(data.getStamina());
    Write(data.getMaxStamina());
    
    Write(data.getArmor());
    Write(data.getAttack());
    Write(data.getDefense());
    
    Write(data.getMovementSpeed());
    Write(data.getPosition());
    
    Write(data.getBodyColor());
}

void InputMemoryBitStream::Read(Body& data)
{
    Read(data.guid);
    Read(data.mBodyType);
    Read(data.mName);
    Read(data.mHealth);
    Read(data.mMaxHealth);
    Read(data.mStamina);
    Read(data.mMaxStamina);
    Read(data.mArmor);
    Read(data.mAttack);
    Read(data.mDefense);
    Read(data.mMovementSpeed);
    Read(data.position);
    Read(data.mBodyColor);
}

void InputMemoryBitStream::Read( Item& data )
{
    Read(data.guid);
    Read(data.name);
    Read(data.count);
    Read(data.options);
    Read(data.action);
    Read(data.slot);
    Read(data.position);
}

void OutputMemoryBitStream::Write( const Item& data )
{
    Write(data.getGUID());
    Write(data.getOwner());
    Write(data.getName());
    Write(data.getCount());
    Write(data.getOptions());
    Write(data.getAction());
    Write(data.getSlot());
    Write(data.getPosition());
}
