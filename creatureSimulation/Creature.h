#pragma once
#include "Cell.h"
#include "Result.h"
#include "UniformPseudoRandomMarsaglia.h"

class Cell;			//used to help compile since Cell is used in the car
class Result;

class Creature
{
	long creatureID;
	float desireToCross;
	float fear;

	//speed and proximity "categories"
	int proximity;				//proximity and speed change according to the cars near the creature
	int speed;

	//actual speed and proximity
	unsigned int real_proximity;
	unsigned int real_speed;

	unsigned int crossPoint;
	unsigned long int create;
	unsigned long int start;
	vector<Cell*> route;
	vector<Result*> results;
	bool justMoved;				//used so that a creature cannot move onto the highway and across a lane in one jump
	UniformPseudoRandomMarsaglia coin1;
	UniformPseudoRandomMarsaglia coin2;
public:
	Creature(long creatureID, float desireToCross, float fear, unsigned int crossPoint, unsigned long int time);
	void saveToFile(ofstream * out, unsigned long int time, unsigned long int lane);
	void addRoute(Cell * cell);
	void addRoute2(Result * result);
	vector<Result*> getRoutes(void);
	long getID(void);
	unsigned long getCreateTime(void);
	float getFear(void);
	float getDesire(void);
	void setStart(unsigned long int time);
	void resetMove(void);
	void setRealProximity(int real_proximity);
	void setRealSpeed(int real_speed);
	void setProximity(int proximity);
	void setSpeed(int speed);
	int getCrossPoint(void);
	int getProximity(void);
	int getSpeed(void);
	unsigned int getRealProximity(void);
	unsigned int getRealSpeed(void);
	bool hasMoved(void);
	~Creature(void);
};

