//
//  Consts.h
//  server
//
//  Created by Bruno Macedo Miguel on 10/8/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef Consts_h
#define Consts_h

#include <cstdlib>
#include <iostream>

struct ParcelIdentification
{
    ParcelIdentification() : notification_id(0), user_send_id(0) {}
    uint32_t notification_id;
    uint32_t user_send_id;
};

// What should expect
enum class MessageHeader : uint16_t
{
    // Incoming information to manipulate new/current users
    UNDEFINED                       = 0,
    LOGIN_REQUEST                   = 1,
    REGISTER_USERID                 = 2,
    // Message to acknowledge
    RECEIVE_NOTIFY_ID               = 3,
    // Message to the Client
    MESSAGE_OUTPUT                  = 4,
    // Login Action
    LOGIN_ACCOUNT                   = 5,
    // Server updates that will be used by all clients
    // When we create a new creature, we want to let all computers know
    BODY_DESPAWN                    = 6,
    BODY_SPAWN                      = 7,
    // We use Cardinal to movement, and coordinate to update and offsetting position
    MOVEMENT_CARDINAL               = 8,
    MOVEMENT_COORDINATE             = 9,
    MOVEMENT_ROTATION				= 10,
    SHOOT                           = 11,
    LIFECHANGE                      = 12,
    // Level
    GAME                            = 13,
    CREATURE_SETTARGET              = 14,
    CREATURE_DEAD                   = 15,
    ITEM                            = 16,
    EXPERIENCE                      = 17,
	CHAT							= 18
};

enum class MovementDirection   : uint8_t
{
    NONE    = 0,
    NORTH   = 1,
    SOUTH   = 2,
    WEST    = 3,
    EAST    = 4,
    SW      = 5,
    SE      = 6,
    NW      = 7,
    NE      = 8,
    LAST    = 9
};

enum class BodyColor : uint8_t
{
    DARK_GREEN    = 0,
    GREEN         = 1,
    GRAY          = 2,
    DARK_GRAY     = 3,
    ORANGE        = 4,
    BRONZE        = 5,
    BLUE          = 6,
    BLACK         = 7,
    LAST          = 8
};

enum class MessageOptions : uint8_t
{
    // characteristics of the message
    UNDEFINED           = 0x00,
    IMPORTANT           = 0x01,
    CURRENT             = 0x02,
    TRASH               = 0x80
};

inline MessageOptions operator|(MessageOptions a,MessageOptions b)
{
    
    return (MessageOptions)(static_cast<uint8_t>(a)|static_cast<uint8_t>(b));
}

inline MessageOptions operator|=(MessageOptions& a,MessageOptions b)
{
    a = (MessageOptions)(static_cast<uint8_t>(a)|static_cast<uint8_t>(b));
    return a;
}

inline bool operator&(MessageOptions a,MessageOptions b)
{
    return (static_cast<uint8_t>(a) & static_cast<uint8_t>(b)) > 0;
}

enum class MessageStatus : uint8_t
{
    AVAILABLE           = 0,
    CREATING            = 1,
    READY               = 2,
    FLIGHT              = 3,
    COMPLETED           = 4
};

// To be translated in the client
enum class MessageOutput : uint8_t
{
    NONE                    = 0,
    LOGIN_DISCONNECTED  	= 1,
    LOGIN_ERROR  			= 2,
    LOGIN_CONNECTING    	= 3,
    LOGIN_CONNECTED    		= 4,
    //ACCOUNT
    ACCOUNT_ERROR  			= 5,
    ACCOUNT_DISCONNECTED    = 6,
    ACCOUNT_PLAYERERROR     = 7,
    ACCOUNT_INCORRECT       = 8,
    ACCOUNT_ALREADYCON      = 9,
    ACCOUNT_CONNECTING    	= 10,
    ACCOUNT_CONNECTED   	= 11,
    //LEVEL
    LEVEL_NOTFOUND          = 12,
    LEVEL_FULL              = 13,
    LEVEL_NOTMASTER         = 14,
    LEVEL_EXIT              = 15,
    PLAYER_ALREADYINLEVEL   = 16,
    PLAYER_REVIVE           = 17,
};

enum class OnlineStatus : uint8_t
{
    OFFLINE = 0,
    ONLINE  = 1
};

enum class BodyType : uint8_t{
    NONE,
    PLAYER,
    CREATURE,
    ITEM
};

enum class ObjectType : uint8_t{
    NONE,
    ORGANISM,
    ITEM
};

enum class ItemFlags : uint8_t
{
    NONE                = 0,
    PICKABLE            = 1,
    CONSUMABLE          = 2,
    DESTROYABLE         = 4,
    AMMUNITION          = 8,
    TEMPORARY           = 16
};

enum class ItemSlots  : uint8_t
{
    NONE         = 0,
    ARMOR        = 1,
    HAND         = 2,
    CONTAINER    = 3
};

enum class ItemAction : uint8_t
{
    NONE         = 0,
    PICK         = 1,
    DROP         = 2,
    CONSUME      = 3,
    SUMMON       = 4,
    UNSUMMON     = 5
};

inline ItemFlags operator|(ItemFlags a,ItemFlags b)
{
    
    return (ItemFlags)(static_cast<uint8_t>(a)|static_cast<uint8_t>(b));
}

inline ItemFlags operator|=(ItemFlags& a,ItemFlags b)
{
    a = (ItemFlags)(static_cast<uint8_t>(a)|static_cast<uint8_t>(b));
    return a;
}

inline bool operator&(ItemFlags a,ItemFlags b)
{
    return (static_cast<uint8_t>(a) & static_cast<uint8_t>(b)) > 0;
}

// NETWORK
const static uint16_t MAXPACKETBYTESIZE  = 1300; // 1300 bytes
const static uint16_t PACKETHEADSIZE     = sizeof(uint32_t) + sizeof(uint32_t);
const static uint16_t MINPACKETBYTESIZE  = sizeof(MessageHeader) + PACKETHEADSIZE; // first byte and id
const static uint16_t MESSAGEPOOLSIZE    = 200;
const static uint16_t SENDINTERVAL       = 15; // 100 ms
const static uint16_t RESENDINTERVAL     = 300; // ms
const static uint16_t MAXSENDTRYOUTS     = 3;   // max tryouts in case parcel does not send

// WORLD
const static uint16_t MAXWORLDS          = 1;
const static uint16_t MAXBODIES          = 100;

// PLAYINGWORLD
const static uint16_t MAPSIZEX              = 109;
const static uint16_t MAPSIZEZ              = 260;

const static uint16_t MAXDATABUSSIZE		= 100;
const static uint16_t MAXCHATSIZE			= MAXDATABUSSIZE - 1;
const static uint16_t MAXPLAYERSPERGAME     = 4;
const static uint16_t MAXCREATURESPERLEVEL  = 50;
const static uint16_t MAXITEMSPERLEVEL      = 250;

const static uint32_t  BROADCASTALL          = 0;
const static uint8_t   BODYVIEWRANGE         = 20;
const static uint8_t   BACKPACKMAXISZE       = 20;
const static uint16_t  BULLETFRAMESALIVE     = 360;

const static uint8_t  MAXWAVES              = 100;

// POINTS
const static uint8_t  POINTS_PICKFLAG       = 97;
const static uint8_t  POINTS_EXPNOKILLER    = 7;
const static uint8_t  POINTS_NEXTWAVE       = 33;
const static uint8_t  POINTS_PERPLAYER      = 12;

// CONNECTION
const static uint16_t PORT                 = 7171;
const static uint16_t MAXUSERS             = 100;
const static uint32_t REQUIREDACTIVITYTIME = 5000; // time before computer gets disconnected

#endif /* Consts_h */
