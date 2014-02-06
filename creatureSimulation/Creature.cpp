#include "StdAfx.h"
#include "Creature.h"
#include "UniformPseudoRandomMarsaglia.h"

Creature::Creature(long creatureID, float desireToCross, float fear, unsigned int crossPoint, unsigned long int time)
{
	this->creatureID = creatureID;
	this->crossPoint = crossPoint;
	this->create = time;
	this->start = 0;
	this->justMoved = false;
	this->proximity = 0;
	this->speed = 0;
	coin1 = UniformPseudoRandomMarsaglia(creatureID);		//each creature is seeded with its own id...this may need to be changed later
	coin2 = UniformPseudoRandomMarsaglia(creatureID+1);

	//flip the coins to decide whether we take fear, desire, none or some comination of both
	float result1 = coin1.DrawFloat();
	float result2 = coin2.DrawFloat();

	//TF
	if((result1 >= 0.5) && (result2 < 0.5))
	{
		this->fear = fear;
		this->desireToCross = 0;
	}
	//FT
	else if((result1 < 0.5) && (result2 >= 0.5))
	{
		this->fear = 0;
		this->desireToCross = desireToCross; 
	}
	//TT
	else if((result1 >= 0.5) && (result2 >= 0.5))
	{
		this->fear = fear;
		this->desireToCross = desireToCross;
	}
	//FF
	else
	{
		this->fear = 0;
		this->desireToCross = 0;
	}
}

unsigned long Creature::getCreateTime(void)
{
	return this->create;
}

// output all the relevant information about a particular creature into the output file
// (you can determine if the creature has been killed by the lane in the output)
// creatureID, creation time, start time, current time, fear, desire to cross, cross point, lane
void Creature::saveToFile(ofstream * out, unsigned long int time, unsigned long int lane)
{
	*out << "creature: " << creatureID << " " << create << " " << start << " " << time << " " << fear << " " << desireToCross << " " << crossPoint << " " << lane << endl;
	*out << " ";
	int i;
	for(i = 0; i < route.size(); i++)
		*out << route.at(i)->getCellID() << " ";
	*out << endl;
}

void Creature::addRoute(Cell * cell)
{
	justMoved = true;
	route.push_back(cell);
}

void Creature::addRoute2(Result * result)
{
	justMoved = true;
	results.push_back(result);
}

vector<Result*> Creature::getRoutes(void)
{
	return results;
}

long Creature::getID(void)
{
	return creatureID;
}

float Creature::getFear(void)
{
	return fear;
}

float Creature::getDesire(void)
{
	return desireToCross;
}

void Creature::setStart(unsigned long int time)
{
	start = time;
}

void Creature::resetMove(void)
{
	justMoved = false;
}

bool Creature::hasMoved(void)
{
	return justMoved;
}

void Creature::setRealProximity(int real_proximity)
{
	this->real_proximity = real_proximity;
}

void Creature::setRealSpeed(int real_speed)
{
	this->real_speed = real_speed;
}

void Creature::setProximity(int proximity)
{
	this->proximity = proximity;
}

void Creature::setSpeed(int speed)
{
	this->speed = speed;
}

unsigned int Creature::getRealProximity(void)
{
	return real_proximity;
}

unsigned int Creature::getRealSpeed(void)
{
	return real_speed;
}

int Creature::getProximity(void)
{
	return proximity;
}

int Creature::getSpeed(void)
{
	return speed;
}

int Creature::getCrossPoint(void)
{
	return crossPoint;
}

Creature::~Creature(void)
{

}
