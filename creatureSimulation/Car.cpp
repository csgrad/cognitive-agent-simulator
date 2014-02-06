#include "StdAfx.h"
#include "Car.h"
#include "UniformPseudoRandomMarsaglia.h"

Car::Car(long carID, unsigned int maxSpeed, unsigned long int time, unsigned int curSpeed, float propright)
{
	this->carID = carID;
	this->maxSpeed = maxSpeed;
	this->curSpeed = curSpeed;
	this->create = time;
	this->start = time;
	this->route.clear();
	coin = UniformPseudoRandomMarsaglia(carID);

	//flip the coin to determine whether to follow the
	float result = coin.DrawFloat();

	if(result >= 0.5)
		this->propright = propright;
	else
		this->propright = 0.0;
}

void Car::addRoute(Cell * cell)
{
	route.push_back(cell);
}

long Car::getID(void)
{
	return carID;
}

// returns the current speed of the car
unsigned int Car::getSpeed(void)
{
	return curSpeed;
}

void Car::accelerate(void)
{
	if(curSpeed + 1 <= maxSpeed)
		curSpeed++;
}

void Car::setStart(unsigned long int start)
{
	this->start = start;
}

void Car::setSpeed(unsigned int speed)
{
	curSpeed = speed;
}

// output all the relevant information about a particular car into the output file
// carID, creation time, start time, current time, maximum speed, propright, exit location (lane, cell), route
// where route is (lane, cell) over and over until destination
void Car::saveToFile(ofstream * out, unsigned long int time)
{
	//start time = createtim for now, need to fix
	// also lane = 0, exit cell is always last cell
	*out << "car: " << carID << " " << create << " " << start << " " << time << " " << maxSpeed << " " << " " << propright << " " << 0 << " " << route.back()->getCellID() << endl;
	for(unsigned long i = 0; i < route.size(); i++)
	{
		*out << " " << route[i]->getCellID();
		//*out << 0 << " " << route.at(i)->getCellID();
	}
	*out << endl;
}

// Accessor method to return the cars propensity to return to the rightmost lane
float Car::getPropRight(void)
{
	return this->propright;
}

Car::~Car(void)
{
}
