#pragma once
#include "Car.h"
#include "Creature.h"

class Car;		//used to help compile since Car is required within the cell
class Creature; //used to help compile since Creature is required within the cell

class Cell
{
	static long idGen;
	long cellID;
	bool occupied;
	bool occupied_creature;
	Car * car;
	Creature * creature;
	bool backwards;		//direction of the cell. If false, left-to-right. If true right-to-left
public:
	Cell(void);
	static void resetID(void);
	long getCellID(void);
	bool isOccupied(void);
	bool hasCreature(void);
	void setDirection(bool);
	bool getDirection(void);

	void addCreature(Creature * creature);
	void addCar(Car * car);
	Car * getCar(void);
	Car * removeCar(void);
	Creature * getCreature(void);
	Creature * removeCreature(void);
	~Cell(void);
};