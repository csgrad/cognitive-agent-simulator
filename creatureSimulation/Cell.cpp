#include "StdAfx.h"
#include "Cell.h"
#include "Car.h"
#include "Creature.h"
#include "Simulation.h" //allows us to know whether DEBUG is true

long Cell::idGen = 0;	//helps with auto increment id

Cell::Cell(void)
{
	cellID = idGen++;
	//cout << "CELL: " << cellID << " ";
	occupied = false;
	occupied_creature = false;
	backwards = false;
}

void Cell::resetID(void)
{
	idGen = 0;
}

long Cell::getCellID(void)
{
	return cellID;
}

// Used to check whether a cell already has a car in it
bool Cell::isOccupied(void)
{
	return occupied;
}

// Usedd to check whether a cell has a creature in it
bool Cell::hasCreature(void)
{
	return occupied_creature;
}

// Adds a creature to the cell, if the cell does not already have a creature
// For now no return value, should likely return a failure if already occupied
void Cell::addCreature(Creature * creature)
{
	if(!occupied_creature)
	{
		if(DEBUG)
			cout << "Cell: " << cellID << " Added creature: " << creature->getID() << " SPEED: " << creature->getSpeed() << " PROX: " << creature->getProximity() << " REALSPEED: " << creature->getRealSpeed() << " REALPROX: " << creature->getRealProximity() << endl;
		this->creature = creature;
		occupied_creature = true;
		creature->addRoute(this);
	}
	else
	{
		cout << "Cell: " << cellID << " ERROR: trying to add a creature to a cell that is already occupied by another creature" << endl;
		exit(1);
	}
}

// Adds a new car to the cell, if the cell is not occupied
// for now there is no return value, but this shoudl likely change to be more robust in the future
void Cell::addCar(Car * car)
{
	if(!occupied)
	{
		if(DEBUG)
			cout << "Cell: " << cellID << " Added car: " << car->getID() << endl;
		if(cellID < 0)
		{
			cout << "ERROR NEG CELLID" << endl;
			exit(0);
		}
		this->car = car;
		occupied = true;
		car->addRoute(this);
	}
	else
	{
		cout << "Cell: " << cellID << " ERROR: trying to add a car to a cell that is already occupied. Current car: " << this->car->getID() << " new car: " << car->getID() << endl;
		exit(1);
	}
}

// Returns the car from the cell, but does not remove it
Car * Cell::getCar(void)
{
	return car;
}

//Removes the current car from the cell and returns it
//(only if occupied)
Car * Cell::removeCar(void)
{
	if(occupied)
	{
		Car * oldcar = car;
		car = NULL;
		occupied = false;
		return oldcar;
	}
	return NULL;
}


Creature * Cell::getCreature(void)
{
	return creature;
}

Creature * Cell::removeCreature(void)
{
	if(occupied_creature)
	{
		Creature * oldcreature = creature;
		creature = NULL;
		occupied_creature = false;
		return oldcreature;
	}
	return NULL;
}


bool Cell::getDirection(void)
{
	return this->backwards;
}

void Cell::setDirection(bool direction)
{
	this->backwards = direction;
}

Cell::~Cell(void)
{
}