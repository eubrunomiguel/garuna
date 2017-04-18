//
//  Body.hpp
//  server
//
//  Created by Bruno Macedo Miguel on 12/11/16.
//  Copyright © 2016 d2server. All rights reserved.
//

#ifndef Body_hpp
#define Body_hpp

#include <cstdlib>
#include <string>
#include "Math.hpp"
#include "Consts.hpp"
#include "MemoryStream.hpp"
#include "Object.hpp"
#include "Tools.hpp"
#include "Item.hpp"

class GameMap;

class Body : public Object
{
    friend class BodyPool;
    friend class InputMemoryBitStream;

public:
    Body() : Object(ObjectType::ORGANISM), mBackpack(Backpack())
    {
        resetBody();
    }
    
    void resetBody()
    {
        mName = "";
        mMaxHealth = mMaxStamina =  mHealth = mStamina = mAttack = mArmor = mDefense = mMovementSpeed = mExperience = 0;
        mLevel = 1;
        mBodyType = BodyType::NONE;
        mBodyColor = BodyColor::DARK_GREEN;
        mBackpack = Backpack();
        
        // Object
        setMapNext  (nullptr);
        setMapPrev  (nullptr);
        setPosition (Vector3());
        setGUID     (0);

		// Server
		s_t_attack.reset();
		s_t_movement.reset();
		s_attackSpeed = 1.5;
		s_minimumdistancefromtarget = BODYVIEWRANGE * .7;
		s_attackChanceSuccess = 30;
    }

    int static getRequiredLevelExperience(const uint16_t& level)
		{return 85/3*(static_cast<int>(pow(level,3))-6*static_cast<int>(pow(level,2)) + 17 * level - 12);}
    
	int getRemainingExperienceToLevel()	       const;
    
    const std::string&  getName ()             const  {return mName;}
    const uint16_t&     getHealth ()           const  {return mHealth;}
    const uint16_t&     getStamina ()          const  {return mStamina;}
    const uint16_t&     getMaxHealth ()        const  {return mMaxHealth;}
    const uint16_t&     getMaxStamina ()       const  {return mMaxStamina;}
    const uint16_t&     getAttack ()           const  {return mAttack;}
    const uint16_t&     getArmor ()            const  {return mArmor;}
    const uint16_t&     getDefense ()          const  {return mDefense;}
    const uint16_t&     getExperience ()       const  {return mExperience;}
    const uint16_t&     getLevel      ()       const  {return mLevel;}
	const uint16_t&     getMovementSpeed()     const  {return mMovementSpeed; }
    const BodyColor&    getBodyColor  ()       const  {return mBodyColor;}
    const BodyType&     getBodyType()          const  {return mBodyType;}
    
    void setName        (std::string name)				{mName = std::move(name);}
    void setHealth      (uint16_t value)                {mHealth = std::move(value);}
    void setStamina     (uint16_t value)                {mStamina = std::move(value);}
    void setMaxHealth   (uint16_t value)                {mMaxHealth = std::move(value);}
    void setMaxStamina  (uint16_t value)                {mMaxStamina = std::move(value);}
    void setAttack      (uint16_t value)                {mAttack = std::move(value);}
    void setArmor       (uint16_t value)                {mArmor = std::move(value);}
    void setDefense     (uint16_t value)                {mDefense = std::move(value);}
    void setSpeed       (uint16_t value)                {mMovementSpeed = std::move(value);}
	void setBodyColor	(BodyColor color)				{mBodyColor = std::move(color); }
	void setBodyType	(BodyType type)					{mBodyType = std::move(type); }

    void setExperience  (uint16_t value)                {mExperience = std::move(value);}
    bool addExperience  (uint16_t value)                {mExperience += std::move(value); return getRemainingExperienceToLevel() <= 0;}
    void setLevel       (uint16_t value)                {mLevel = std::move(value);}
    void levelUp        ()                              {mLevel++;}
	void updateStats	();

	void setAttackSuccess		(uint8_t chance) { s_attackChanceSuccess = std::move(chance); }
	void setMinAttackDistance	(double dist)		{ s_minimumdistancefromtarget = std::move(dist); }
	void setAttackSpeed			(double speed)		{ s_attackSpeed = std::move(speed); }

	void move(MovementDirection diretion);
	void attack()					{ s_t_attack.reset(); }
	void setTarget(const Body* t) { s_target = t; }

	bool   canAttack() { return s_t_attack.getElapsedSeconds() > s_attackSpeed; }
	bool   canMove() { return s_t_movement.getElapsedSeconds() > 1.0 / mMovementSpeed * 1.1; }
	double getmove	() { return s_t_movement.getElapsedSeconds(); }

	const uint8_t&				getAttackChance()		const { return s_attackChanceSuccess; }
	const double&				getMinDistFromTarget()  const { return s_minimumdistancefromtarget; }
	const MovementDirection&	getLastDirection()		const { return s_lastDirection; }
	const Body*					getTarget()				const { return s_target; }

	Backpack    mBackpack;

private:
	// Serializable
    std::string       mName;
    uint16_t          mLevel;
    uint16_t          mExperience;
    uint16_t          mHealth;
    uint16_t          mMaxHealth;
    uint16_t          mStamina;
    uint16_t          mMaxStamina;
    uint16_t          mAttack;
    uint16_t          mArmor;
    uint16_t          mDefense;
	uint16_t          mMovementSpeed;
	BodyColor         mBodyColor;
    BodyType          mBodyType;

	// Server
			double		      s_attackSpeed;
			double			  s_minimumdistancefromtarget;
			uint8_t			  s_attackChanceSuccess;
			MovementDirection s_lastDirection;
			Timer			  s_t_attack;
			Timer		      s_t_movement;
	const	Body*			  s_target;
};

#endif /* Body_hpp */
