//
//  GameLevel.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 2/4/17.
//  Copyright © 2017 d2server. All rights reserved.
//

#ifndef GameLevel_hpp
#define GameLevel_hpp

#include "Consts.hpp"
#include "Tools.hpp"
#include "Math.hpp"
#include "NetworkData.hpp"
#include "Map.hpp"
#include "Item.hpp"
#include "Body.hpp"
#include <map>
#include <vector>

class GameWorld;
class Object;
class Performance;

struct EnemyFlag
{
    friend class GameLevel;
private:
    EnemyFlag () : carrier(nullptr), item(nullptr) {}
    
    bool  loaded()              const {return item != nullptr;}
    bool  exist()               const {return item && item->getGUID() != 0;}
    bool  isGrounded()          const {return carrier == nullptr;}
    
    Body* holder    ()          const {return  carrier;}
    Item* flagitem  ()          const {return  item;}
    
    void  pick      (Body* b)   {carrier = b;}
    
    void  load      (Item* i)   {item = i;}
    
    Item* item;
    Body* carrier;
};

class GameLevel
{
private:

	int                      getCellNumber(const float& x, const float& z) {return getCellNumber(static_cast<int>(floor(x)), static_cast<int>(floor(z)));}
    int                      getCellNumber(const int& x, const int& z){ return x + z*MAPSIZEX;}
    
    MapData                  mapData;
    
    Object*                  cells        [MAPSIZEX][MAPSIZEZ] = { nullptr };
    
    // To avoid huge amount of data at once, we periodically respaw
    std::queue<std::pair<std::string, Vector3>> creaturesToRespaw;
    
    Body                     creatures    [MAXCREATURESPERLEVEL];
	Body*                    players	  [MAXPLAYERSPERGAME] = {nullptr};
    Item                     items        [MAXITEMSPERLEVEL];

	bool					 won;
    
    EnemyFlag                flagdata;
    
    Timer                    respawtimer;
	Timer                    fpstimer;
	Timer                    sincronizationTimer;
    GameWorld*               gameworld = nullptr;
    
    struct Performance{
        Performance() : monsterskilled(0), time(0), points(0){}
        uint16_t monsterskilled = 0;
        uint16_t time           = 0;
        uint16_t points         = 0;
    } performance_active;
    
    uint16_t                 currentLevel;
	uint16_t                 playerscount;
    uint32_t				 currentUniqueID;
    GameStatus               status;
    
    static uint32_t UNIQUEIDIDENTIFIER;
    
public:
    
    GameLevel() : status(GameStatus::UNDEFINED), playerscount(0), currentUniqueID(0), currentLevel(0), won(false)
    {
        wActions[MessageHeader::MOVEMENT_CARDINAL]   = &GameLevel::requestCardinalMove;
        wActions[MessageHeader::MOVEMENT_COORDINATE] = &GameLevel::requestCoordinateMove;
        wActions[MessageHeader::SHOOT]               = &GameLevel::requestShoot;
    }
    
    void processWorldAction(MessageHeader& header, Body* gameobject, void* object)
    {
        if(wActions[header])
            (this->*wActions[header])(gameobject, object);
    }
    
    void       newGame(GameWorld* world);
    void       prepareEndGame();
    void       endGame();
    void       startGame();
    
    bool       enterGame(Body*);
    void       leaveGame(Body*);
    bool       isLobbyManager(Body*) const;
    
    Body*      getBody(uint32_t, BodyType);
    
    void       update();
    bool       nextWave();
    void       cleanWave();
    void       cleanAllObjects();
    
    Item*      createItem();
    Item*      createItem(const std::string&);
    void       summonItem(Item*);
    void       unsummonItem(Item*, bool reset);
    
    void       summonBody(Body* body, bool broadcast);
    void       unsummonBody(Body* body);
    
    bool       inGame(const Body*) const;
    
    const Performance&  getPerformance()        const {return performance_active;}
    
	const bool			hasWin()			    const {return won;}
    const uint32_t      getUniqueGameID()       const {return currentUniqueID;}
    const Vector3       getSafeZone    ()       const {return Vector3(mapData.base.first, 0, mapData.base.second);}
    const Vector3       getFlagPosition()       const {return Vector3(mapData.flag.first, 0, mapData.flag.second);}
    const uint16_t      getCurrentLevel()       const {return currentLevel;}
	const uint16_t      getPlayersCount()       const {return playerscount;}
    const uint16_t      getMaxCreatures()       const {return static_cast<uint16_t>(mapData.respaw.size());}
    const bool          isWaveEmpty    ()       const;
    const bool          isPlaying      ()       const {return status == GameStatus::PLAYING;}
    const bool          isOver         ()       const {return status == GameStatus::END;}
    const bool          isLobbying     ()       const {return status == GameStatus::LOBBY;}
    const Body*         hasTarget(Body* who)    const;
    const uint16_t      getMediumLevel()        const;

protected:
    void    createFlag();
    void    pickFlag(Body*);
    void    dropFlag(Body*);
    
    void    updateCreatureState      (Body*);
    void    updateCreatureMovement   (Body*);
    void    updateCreatureAttack     (Body*);
    
    void    loadmap       ();
    
    Body*   createCreature  (const std::string&, Vector3 position);
    
    void    replicateWorld  (const Body* to);
    
    // World Actions
    void requestCardinalMove    (Body*, void* data);
    void requestCoordinateMove  (Body*, void* data);
    void requestShoot           (Body*, void* data);
    std::map <MessageHeader, void(GameLevel::*)(Body*, void*)> wActions;
    // World Actions
    
    void	creatureUpdateState    (Body*);
    void	creatureAddExperience  (Body*, uint16_t, bool);
    void	creaturedead           (Body*, Body* by);
    void	creaturechangelife     (Body*, Body*, const uint16_t& value, bool aggressive, bool filter);
    void	creaturemove           (Body*, const MovementDirection& local);
    void	creaturemove           (Body*, const Vector3& position);
    void	creatureshoot          (Body*, const Vector3& tolocal);
    void	creaturepickitem       (Body*, Item*);
    void	creatureconsumeitem    (Body*, Item*);
    void	creaturedropitem       (Body*, Item*, const Vector3* customlocation = nullptr);
    
    void    updateItems();
    void    updateCreatures();
    void    updateRespaw();
	void	sincronizePlayers();
    
    Object* mapCell(const Vector3& pos);
    
    void    mapEnterTile                (Object* gameobject);
    void    mapExitTile                 (Object* gameobject);
    
    bool    mapInBoundaries       (const Vector3& pos)          const {return mapInBoundaries(static_cast<int>(floor(pos.mX)), static_cast<int>(floor(pos.mZ)));}
    bool    mapInBoundaries       (const int& x, const int& z)  const {return ((z >= 0) & (x >= 0) & (x < MAPSIZEX) & (z < MAPSIZEZ));};
    
    bool    mapIsSquareAvailable  (const Vector3& pos)          const {return mapIsSquareAvailable(static_cast<int>(floor(pos.mX)),static_cast<int>(floor(pos.mZ)));}
    bool    mapIsSquareAvailable  (const int& x, const int& y)  const;
    
    bool    mapHasBloackableObject (const int& x, const int& z) const;
    Item*   mapHasInteractableItem (const int& x, const int& z) const;
    
    Vector3 mapGetEmptySquare     ()                            const;
    
    bool    mapGetFirstEmptySquaresAround (Vector3& from, const MovementDirection& exception = MovementDirection::NONE, bool opposite = false) const;
    void    mapGetEmptySquaresAround      (std::vector<MovementDirection>& directions, const Vector3& from, const MovementDirection& exception = MovementDirection::NONE, bool opposite = false) const;
    void    mapGetEmptySquaresAround      (std::vector<MovementDirection>& directions, const Body* from, const MovementDirection& exception = MovementDirection::NONE, bool opposite = false) const;
    bool    getValidPosition(Body*, bool random);
    
    const Body*   mapGetClosestBody(const Body* from,  const BodyType& onlybodytype)                             const;
    const Body*   mapGetClosestBody(const Vector3&   frompos,  const BodyType& onlybodytype,  const Body* from)  const;
    
    void    itemUpdatePosition(Item*);
	bool    itemCollisionHandler(Item*);
	bool    sphereCollisionHandler(Item*, float diameter);
};

#endif /* GameLevel_hpp */
