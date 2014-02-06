#pragma once
#include "Cell.h"
#include "UniformPseudoRandomMarsaglia.h"

class Cell;			//used to help compile since Cell is used in the car

class Car
{
private:
	long carID;
	unsigned int maxSpeed;
	unsigned int curSpeed;
	unsigned long int create;		//the time the car has been created
	unsigned long int start;		//the time the car started moving
	float propright;
	UniformPseudoRandomMarsaglia coin;
	vector<Cell*> route;
public:
	Car(long carID, unsigned int maxSpeed, unsigned long int time, unsigned int curSpeed, float propright);
	void saveToFile(ofstream * out, unsigned long int time);
	void addRoute(Cell * cell);
	long getID(void);
	unsigned int getSpeed(void);
	void accelerate(void);
	void setStart(unsigned long int start);
	void setSpeed(unsigned int speed);
	float getPropRight(void);
	~Car(void);
};

