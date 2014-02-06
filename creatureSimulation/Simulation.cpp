#include "StdAfx.h"
#include "Simulation.h"
#include "Cell.h"
#include "UniformPseudoRandomMarsaglia.h"
#include "Result.h"

Simulation::Simulation(void)
{
	maxTime = 0;
	cellNum = 0;
	laneNum = 0;
	maxSpeed = 0;
	kbOutput = 0;
	entryCarProb = 0.0;
	creaturesHit = 0;
	creaturesCrossed = 0;
	randomDecel = false;
	fixedFear = -1;
	fixedDesire = -1;
	fuzzyMembers = 3;
	traceOutput = true;
	totalWaitTime = 0;
}

Simulation::Simulation(unsigned long int maxTime, unsigned long int cellNum, unsigned short int laneNum, unsigned int maxSpeed, float entryCarProb, float entryCreatureProb, vector<unsigned int> crossPoints, unsigned long experiment_id, vector<unsigned int> backwards)
{
	this->maxTime = maxTime;
	this->cellNum = cellNum;
	this->laneNum = laneNum;
	this->maxSpeed = maxSpeed;
	this->entryCarProb = entryCarProb;
	this->entryCreatureProb = entryCreatureProb;
	this->crossPoints = crossPoints;
	this->experiment_id = experiment_id;
	this->backwards = backwards;
	UPR = UniformPseudoRandomMarsaglia(experiment_id);

	//intitialize the array of creatures for each cell
	//(used to be for each crosspoint, but now creatures can move sideways along the highway)
	for(unsigned int c = 0; c < cellNum; c++)
	{
		vector<Creature *> creatures;
		creatureQueues.push_back(creatures);
	}

	//initialize car queues
	for(unsigned int c = 0; c < laneNum; c++)
	{
		queue<Car *> cars;
		carQueues.push_back(cars);
	}

	//initialize each lane
	Cell::resetID();
	for(int c = 0; c < laneNum; c++)
	{
		//cout << "LANE: " << c << "CELLs: " << cellNum << endl;
		Cell * lane = new Cell[cellNum];
		
		//check if this lane is backwards
		for(unsigned int l = 0; l < this->backwards.size(); l++)
		{
			if(this->backwards[l] == c)
			{
				for(unsigned int i = 0; i < cellNum; i++)
				{
					lane[i].setDirection(true);
				}
			}
		}
		
		lanes.push_back(lane);
	}

	//initialize other variables
	currentTime = 0;
	carsCreated = 0;
	creaturesCreated = 0;
	creaturesHit = 0;
	creaturesCrossed = 0;
	randomDecel = false;
	fixedFear = -1;
	fixedDesire = -1;
	fuzzyMembers = 3;
	kbOutput = 0;
	traceOutput = true;
	totalWaitTime = 0;
}

// This function starts the simulation running after it has been initialized
void Simulation::run(void)
{
	*configIndependentOutput << setw(10) << "Time" << setw(10) << "Ns" << setw(10) << "Nh" << setw(10) << "Nq" << setw(10) << "Tw" << endl;

	//main simulation loop
	while(currentTime < maxTime)
	{
		if(DEBUG)
			cout << "TIME: " << currentTime << endl;

		generateCars();
		generateCreatures();
		updateCarSpeeds();

		//reset the waiting status of all configurations (used for timing how long a creature is waiting at a config)
		for(unsigned int c = 0; c < results.size(); c++)
			results.at(c)->setWaiting(false);

		moveCreaturesOntoHighway();

		//repeat the same process for each lane of the highway
		for(int l=0; l < laneNum; l++)
		{
			//move creatures on highway to next lane, or off highway
			for(unsigned int i = 0; i < cellNum; i++)
			{
				if(lanes.at(l)[i].hasCreature() && (!lanes.at(l)[i].getCreature()->hasMoved()))
				{
					//only move if the algo thinks we should
					Creature * creature = lanes.at(l)[i].getCreature();

					//change proximity and speed info to next lane for considering
					//(need to change back if we dont move for correct hit stats)
					if(l+1 < laneNum)
						updateProxSpeed(creature, l+1, i);

					bool shouldMove = false;
					if(DEBUG)
						cout << "  creature considering move off highway...\n";
					switch(intelligence)
					{
						case NAIIVE:
							shouldMove = naiiveCreatureShouldMove(*creature, l+1);
						break;
						case NAIIVE_FEAR:
							shouldMove = naiiveCreatureShouldMove2(*creature, l+1);
						break;
						default:
							shouldMove = naiiveCreatureShouldMove(*creature, l+1);
						break;
					}

					if(shouldMove)
					{
						//successfully crossed
						if(l+1 >= laneNum)
						{
							if(DEBUG)
								cout << "    moved off highway\n";
							Creature * creature = lanes.at(l)[i].removeCreature();

							vector<Result*> route = creature->getRoutes();
							unsigned int i;
							for(i = 0; i < route.size(); i++)
							{
								Result * result = route.at(i);
								int r;
								if((r = findResult(*result)) == -1)
								{
									result->setCrossed(1);
									result->setHit(0);
									results.push_back(result);
								}
								else
									results.at(r)->success(currentTime - creature->getCreateTime());
							}
							creature->saveToFile(simulationOutput, currentTime, laneNum);
							creaturesCrossed++;
							if(DEBUG)
								cout << "  Creature Finished Successfully" << endl;
						}
						//move into next lane
						else if(!lanes.at(l+1)[i].hasCreature())
						{
							if(DEBUG)
								cout << "  creature moving into next lane\n";
							Creature * creature = lanes.at(l)[i].removeCreature();
							Result * r = new Result(i, l+1, creature->getProximity(), creature->getSpeed(), 0, 0);
							creature->addRoute2(r);
							lanes.at(l+1)[i].addCreature(creature);
						}
						else
						{ 
							if(DEBUG)
								cout << "  creature cant move, next lane occupied\n";
							// do nothing if the next lane already has a creature
							updateProxSpeed(creature, l, i);
						}
					}
					//return back to old prox and speed in case we get hit here
					else
					{
						if(DEBUG)
							cout << "  creature decided not to move\n";
						updateProxSpeed(creature, l, i);
					}
				}
			}//end of creature motion
			moveCars();
		}//end of lane interation		
		currentTime++;

		//reset the hasmoved value on all previously moved creatures
		for(int l=0; l < laneNum; l++)
		{
			for(unsigned int i = 0; i < cellNum; i++)
			{
				if(lanes.at(l)[i].hasCreature())
					lanes.at(l)[i].getCreature()->resetMove();
			}
		}

		outputTimeDependentData();
	}
	outputKnowledge();
}

void Simulation::updateProxSpeed(Creature * creature, int lane, int cell)
{
	int speed, prox;
	//naiive uses a different algorithm for determining the creatures' perception of distance and speed
	if(intelligence == NAIIVE || intelligence == NAIIVE_FEAR)
	{
		speed = naiiveGetSpeed(*creature,lane);
		prox = naiiveGetProx(*creature,lane);
	}
	else
	{
		cout << "INTELLIGENCE UNDEFINED! CRIT ERROR" << endl;
		exit(1);
	}

	creature->setSpeed(speed);
	creature->setProximity(prox);

	int closest_car;
	if((closest_car = closestCar(0,cell)) != -1)
	{
		int real_distance = cell - closest_car;
		creature->setRealProximity(real_distance);
		creature->setRealSpeed(lanes.at(0)[closest_car].getCar()->getSpeed());
	}
	else
	{
		creature->setRealProximity(distances.at(distances.size()-1) + 1);	//set distance to higher than max
		creature->setRealSpeed(0);											//set speed to 0
	}
}

//Function which determines the creature's intelligence, can be replaced by various agorithms
//returns true if the creature should move, false if not
bool Simulation::naiiveCreatureShouldMove2(Creature c, int lane)
{
	int r;
	Result * result = new Result(c.getCrossPoint(), lane, c.getProximity(), c.getSpeed(), 0, 0);
	if((r = findResult(*result)) != -1)
	{
		float successratio;
		//int outcome = results.at(r)->getOutcome();
		int outcome = results.at(r)->getCrossed() - results.at(r)->getHit();
		if(creaturesCrossed == 0)
			if(creaturesHit < 5)
				successratio = 1;		//allow creatures to take risk at first
			else
				successratio = 0;
		else
			successratio = (float)outcome / (float)creaturesCrossed;

		//cout << "DESIRE: " << c.getDesire() << " FEAR: " << c.getFear() << " SUCCESS: " << successratio << " TOT: " << successratio + c.getDesire() - c.getFear() << endl;
		if((successratio + c.getDesire() - c.getFear()) < 0)
			return false;
		else
			return true;
	}
	return true;
}

//same as above but does not use the fear or desire
bool Simulation::naiiveCreatureShouldMove(Creature c, int lane)
{
	int r;
	Result * result = new Result(c.getCrossPoint(), lane, c.getProximity(), c.getSpeed(), 0, 0);
	if((r = findResult(*result)) != -1)
	{
		//if(results.at(r)->getOutcome() < 0)
		if((results.at(r)->getCrossed() - results.at(r)->getHit()) < 0)
			return false;
		else
			return true;
	}
	return true;
}

// Returns the estimation of speed for the creature based on mid, near and far passed in the sim parameters
int Simulation::naiiveGetSpeed(Creature c, int lane)
{
	int cell = c.getCrossPoint();
	int carCell;
	
	//first we need to figure out what distance category the closest car is in
	for(unsigned int d = 0; d < distances.size(); d++)
	{
		if((carCell = carNearby(lane,distances.at(d),cell)) != -1)
			break;
	}

	//only need to compare if there is a car close enough, otherwise the creature sees "all clear"
	if(carCell != -1)
	{
		// see if the closest car is travelling within one of the velocity categories, if so return the index of the category in the vector
		for(unsigned int v = 0; v < velocities.size(); v++)
		{
			if(lanes.at(lane)[carCell].getCar()->getSpeed() <= velocities.at(v))
				return v+1;	//always do + 1 because clear = 0
		}
		return CLEAR;
	}
	return CLEAR;
}

// Returns the index of the distance category the creature perceives the nearest car in
int Simulation::naiiveGetProx(Creature c, int lane)
{
	int cell = c.getCrossPoint();
	int carCell = -1;

	for(unsigned int d = 0; d < distances.size(); d++)
	{
		if((carCell = carNearby(lane, distances.at(d), cell)) != -1)
			return d + 1; //always do + 1 because clear = 0
	}
	return CLEAR;
}

//generates cars with probability specified in cfg file
void Simulation::generateCars(void)
{
	//ensure we don't generate too many cars (mostly used for testing)
	if(maxCars != 0)
		if(carsCreated >= maxCars)
			return;

	//repeat the same process for each lane of the highway
	for(int l=0; l < laneNum; l++)
	{
		//newCarChance should generate between 0 and 1, if it is less than the prob
		//of creating a new car, then create one!
		float newCarChance = UPR.DrawFloat();
		if(newCarChance < entryCarProb)
		{
			if(DEBUG)
				cout << "Created Car " << endl;
			
			unsigned int initspeed = UPR.DrawInteger(maxSpeed+1) - 1;	//draw int goes from 1 to n, we want from 0 to n
			Car * car = new Car(carsCreated++,maxSpeed,currentTime, initspeed, propright);
			//Car * car = new Car(carsCreated++,maxSpeed,currentTime, 0);			//uncomment for fixed zero initial speed
			carQueues.at(l).push(car);
		}

		//move a car from the queue into the first cell if possible
		
		//backwards direction first
		if(lanes.at(l)[0].getDirection())
		{
			if(!lanes.at(l)[cellNum-1].isOccupied() && !carQueues.at(l).empty())
			{
				Car * car = carQueues.at(l).front();
				car->setStart(currentTime);
				carQueues.at(l).pop();
				lanes.at(l)[cellNum - 1].addCar(car);
			}
		}
		else
		{
			if(!lanes.at(l)[0].isOccupied() && !carQueues.at(l).empty())
			{
				Car * car = carQueues.at(l).front();
				car->setStart(currentTime);
				carQueues.at(l).pop();
				lanes.at(l)[0].addCar(car);
			}
		}
	}
}

//generates creatures with probability specified in cfg file
void Simulation::generateCreatures(void)
{
	if(maxCreatures != 0)
		if(creaturesCreated >= maxCreatures)
			return;

	//create new creatures simular to new cars
	float newCreatureChance = UPR.DrawFloat();
	if(newCreatureChance < entryCreatureProb)
	{
		float desireToCross, fear;

		if(DEBUG)
			cout << "Created Creature ";
		if(fixedDesire == -1)
			desireToCross = 0;//desireToCross = UPR.DrawFloat();					//random between 0 and 1
		else
			//desireToCross = UPR.DrawFloat() * fixedDesire;
			desireToCross = fixedDesire;

		if(fixedFear == -1)
			fixedFear = 0;//fear = UPR.DrawFloat();								//random between 0 and 1
		else
			//fear = UPR.DrawFloat() * fixedFear;
			fear = fixedFear;

		int crossPointID = UPR.DrawInteger(crossPoints.size()) - 1;	//random between cross point #0 and crosspoint #size of crosspoints
		int crossPoint = crossPoints.at(crossPointID);
		if(DEBUG)
			cout << "at crossPoint: " << crossPoint << endl;
		Creature * creature = new Creature(creaturesCreated++, desireToCross, fear, crossPoint, currentTime);
		creatureQueues.at(crossPoint).push_back(creature);
	}
}

// performs nagel-shrek accel and decel
void Simulation::updateCarSpeeds(void)
{
	//repeat the same process for each lane of the highway
	for(unsigned int l=0; l < lanes.size(); l++)
	{
		//accelerate all vehicles
		for(unsigned int i = 0; i < cellNum; i++)
		{
			if(lanes.at(l)[i].isOccupied())
			{
				Car * car = lanes.at(l)[i].getCar();
				car->accelerate();
			}
		}
		
		if(randomDecel)
			doRandomDecel(l);
	}
}

// given the potential lane, current position and desired speed, this function will
// return the maximum possible speed given the cars within the potential lane
// assumes that all potential collisions have already been moved into the cell already
unsigned int Simulation::collisionSpeed(unsigned int lane, unsigned int position, unsigned int maxspeed)
{
	//forward
	if(!lanes.at(lane)[0].getDirection())
	{
		//ramp up to maxspeed of the creature
		for(unsigned int s = 1; s <= maxspeed; s++)
		{
			//make sure we don't go out of bounds
			if((position + s) >= (cellNum))
				return (cellNum - 1) - position;

			//if we reach an occupied position, return speed-1
			if(lanes.at(lane)[position + s].isOccupied())
				return s - 1;
		}
	}
	//backward
	else
	{
		for(unsigned int s = 1; s <= maxspeed; s++)
		{
			//make sure we dont go out of bounds
			if((position - s) < 0)
				return position;

			if(lanes.at(lane)[position - s].isOccupied())
				return s - 1;
		}
	}

	return maxspeed;
}

//assumptions right now:
//left-to-right pass in lane l+1
//right-to-left pass in lane l-1 (if available)
//no rule to force the cars to a "slow lane" yet
void Simulation::moveCars(void)
{
	bool stayInLane = false;

	for(unsigned int l=0; l < lanes.size(); l++)
	{
		//move cars forward (left-to-right) (start at end)
		if(!lanes.at(l)[0].getDirection())
		{
			for(int i = cellNum-1; i >= 0; i--)
			{
				//check if there is a car to be moved
				if(lanes.at(l)[i].isOccupied())
				{
					stayInLane = false;
					Car * car = lanes.at(l)[i].getCar();
					unsigned int speed = car->getSpeed();
					unsigned int current_lane_speed = collisionSpeed(l,i,speed);

					//first priority is checking to move back into the rightmost lane
					float rightmove = UPR.DrawFloat();
					if((rightmove <= car->getPropRight()) && (l > 0))
					{
						if(!lanes.at(l-1)[0].getDirection())
						{
							unsigned int prev_lane_speed = collisionSpeed(l-1,i,speed);

							if(prev_lane_speed != 0)
							{
								Car * car = lanes.at(l)[i].removeCar();
								car->setSpeed(prev_lane_speed);

								checkCreatureHit(l,i,car->getSpeed());

								if(DEBUG)
								{
									cout << "RIGHT Changing Lanes. Moving car: " << car->getID() << " from cell " << i << " to cell " << i+prev_lane_speed << " in lane " << l-1 << endl;
									cout << "WANTED speed " << car->getSpeed() << " ACTUAL speed in current lane " << current_lane_speed << " ACTUAL SPEED in prev lane " << prev_lane_speed << endl;
								}

								if(i + prev_lane_speed >= cellNum)
								{
									lanes.at(l-1)[cellNum-1].addCar(car);
									Car * car = lanes.at(l-1)[cellNum-1].removeCar();
									car->saveToFile(simulationOutput, currentTime);
									if(DEBUG)
										cout << "  Car finished" << endl;
								}
								else
									lanes.at(l-1)[i + prev_lane_speed].addCar(car);
							}
							else
								stayInLane = true;
						}
						//just stay in our own lane
						else
							stayInLane = true;
					}
					//check if next lane can be passed into
					else if(l+1 < laneNum)
					{
						//check that the next lane direction matches this lane
						if(!lanes.at(l+1)[0].getDirection())
						{
							unsigned int next_lane_speed = collisionSpeed(l+1,i,speed);
							if(next_lane_speed > current_lane_speed)
							{
								Car * car = lanes.at(l)[i].removeCar();
								car->setSpeed(next_lane_speed);

								checkCreatureHit(l,i,car->getSpeed());

								if(DEBUG)
								{
									cout << "RIGHT Changing Lanes. Moving car: " << car->getID() << " from cell " << i << " to cell " << i+next_lane_speed << " in lane " << l+1 << endl;
									cout << "WANTED speed " << car->getSpeed() << " ACTUAL speed in current lane " << current_lane_speed << " ACTUAL SPEED in next lane " << next_lane_speed << endl;
								}

								//car finished (add to last cell and remove first to make sure it gets added to the route
								if((i + next_lane_speed) >= (cellNum - 1))
								{
									lanes.at(l+1)[cellNum-1].addCar(car);
									Car * car = lanes.at(l+1)[cellNum-1].removeCar();
									car->saveToFile(simulationOutput, currentTime);
									if(DEBUG)
										cout << "  Car finished" << endl;
								}
								else
									lanes.at(l+1)[i + next_lane_speed].addCar(car);
							}
							else
							{
								if(DEBUG)
									cout << "Next lane is slower ( " << next_lane_speed << ") than current lane ( " << current_lane_speed << " ). Moving car: " << car->getID() << " from cell " << i << " to cell " << i+current_lane_speed << " in lane " << l << endl;
								stayInLane = true;
							}
						}
						//if directions don't match, stay in lane
						else
						{
							if(DEBUG)
								cout << "Next lane direction does not match. Moving car: " << car->getID() << " from cell " << i << " to cell " << i+current_lane_speed << " in lane " << l << endl;
							stayInLane = true;
						}
					}
					//if we can't pass into next lane, just move as far as we can in this lane
					else
					{
						if(DEBUG)
							cout << "No more lanes to pass into. Moving car: " << car->getID() << " from cell " << i << " to cell " << i+current_lane_speed << " in lane " << l << endl;
						stayInLane = true;
					}


					if(stayInLane)
					{
						if(DEBUG)
							cout << "WANTED speed " << car->getSpeed() << " ACTUAL speed " << current_lane_speed << endl;

						Car * car = lanes.at(l)[i].removeCar();
						car->setSpeed(current_lane_speed);

						checkCreatureHit(l,i,car->getSpeed());

						//car finished
						if((i + current_lane_speed) >= (cellNum+1))
						{
							car->saveToFile(simulationOutput, currentTime);
							if(DEBUG)
								cout << "  Car finished" << endl;
						}
						//advance car
						else
							lanes.at(l)[i + current_lane_speed].addCar(car);
					}
				}
			}
		}
		//move cars backward (right-to-left)
		else
		{
			for(unsigned int i = 0; i <= cellNum - 1; i++)
			{
				//check if there is a car to be moved
				if(lanes.at(l)[i].isOccupied())
				{
					stayInLane = false;
					Car * car = lanes.at(l)[i].getCar();
					unsigned int speed = car->getSpeed();
					unsigned int current_lane_speed = collisionSpeed(l,i,speed);

					//first priority is checking to move back into the rightmost lane (l+1 in this direction)
					float rightmove = UPR.DrawFloat();
					if((rightmove <= car->getPropRight()) && (l+1 < laneNum))
					{
						if(lanes.at(l+1)[0].getDirection())
						{
							unsigned int prev_lane_speed = collisionSpeed(l+1,i,speed);
							if(prev_lane_speed += 0)
							{
								Car * car = lanes.at(l)[i].removeCar();
								car->setSpeed(prev_lane_speed);

								checkCreatureHit(l,i,car->getSpeed());

								if(DEBUG)
								{
									cout << "Changing Lanes. Moving car: " << car->getID() << " from cell " << i << " to cell " << i+prev_lane_speed << " in lane " << l+1 << endl;
									cout << "WANTED speed " << car->getSpeed() << " ACTUAL speed in current lane " << current_lane_speed << " ACTUAL SPEED in prev lane " << prev_lane_speed << endl;
								}

								if(i - prev_lane_speed <= 0)
								{
									lanes.at(l+1)[0].addCar(car);
									Car * car = lanes.at(l+1)[0].removeCar();
									car->saveToFile(simulationOutput, currentTime);
									if(DEBUG)
										cout << "  Car finished" << endl;
								}
								else
									lanes.at(l+1)[i - prev_lane_speed].addCar(car);
							}
							else
								stayInLane = true;
						}
						//just stay in our own lane
						else
							stayInLane = true;
					}
					//check if next lane can be passed into
					else if(l-1 > 0)
					{
						//check that the next lane direction matches this lane
						if(lanes.at(l-1)[0].getDirection())
						{
							unsigned int next_lane_speed = collisionSpeed(l-1,i,speed);
							if(next_lane_speed > current_lane_speed)
							{
								Car * car = lanes.at(l)[i].removeCar();
								car->setSpeed(next_lane_speed);

								checkCreatureHit(l,i,car->getSpeed());

								if(DEBUG)
								{
									cout << "Changing Lanes. Moving car: " << car->getID() << " from cell " << i << " to cell " << i+next_lane_speed << " in lane " << l+1 << endl;
									cout << "WANTED speed " << car->getSpeed() << " ACTUAL speed in current lane " << current_lane_speed << " ACTUAL SPEED in next lane " << next_lane_speed << endl;
								}

								//car finished (add to last cell and remove first to make sure it gets added to the route
								if(i - next_lane_speed <= 0)
								{
									lanes.at(l-1)[0].addCar(car);
									Car * car = lanes.at(l-1)[0].removeCar();
									car->saveToFile(simulationOutput, currentTime);
									if(DEBUG)
										cout << "  Car finished" << endl;
								}
								else
									lanes.at(l-1)[i - next_lane_speed].addCar(car);
							}
							else
							{
								if(DEBUG)
									cout << "Next lane is slower than current lane. Moving car: " << car->getID() << " from cell " << i << " to cell " << i+current_lane_speed << " in lane " << l << endl;
								stayInLane = true;
							}
						}
						//if directions don't match, stay in lane
						else
						{
							if(DEBUG)
								cout << "Next lane direction does not match. Moving car: " << car->getID() << " from cell " << i << " to cell " << i+current_lane_speed << " in lane " << l << endl;
							stayInLane = true;
						}
					}
					//if we can't pass into next lane, just move as far as we can in this lane
					else
					{
						if(DEBUG)
							cout << "No more lanes to pass into. Moving car: " << car->getID() << " from cell " << i << " to cell " << i+current_lane_speed << " in lane " << l << endl;
						stayInLane = true;
					}


					if(stayInLane)
					{
						if(DEBUG)
							cout << "WANTED speed " << car->getSpeed() << " ACTUAL speed " << current_lane_speed << endl;

						Car * car = lanes.at(l)[i].removeCar();
						car->setSpeed(current_lane_speed);

						checkCreatureHit(l,i,car->getSpeed());

						//car finished
						if(i - current_lane_speed <= 0)
						{
							car->saveToFile(simulationOutput, currentTime);
							if(DEBUG)
								cout << "  Car finished" << endl;
						}
						//advance car
						else
							lanes.at(l)[i - current_lane_speed].addCar(car);
					}
				}
			}
		}
	}
}

//checks if a creature has been hit. If so, removes the creature from the highway
void Simulation::checkCreatureHit(int lane, int cell, int speed)
{
	//left to right
	if(!lanes.at(lane)[0].getDirection())
	{
		//check if a creature has been hit
		for(long int c=cell; c < (cell + speed); c++)
		{
			if(c < (long)cellNum)
			{
				if(lanes.at(lane)[c].hasCreature())
				{
					Creature * creature = lanes.at(lane)[c].removeCreature();
					vector<Result*> route = creature->getRoutes();
					unsigned int k;
					for(k = 0; k < route.size(); k++)
					{
						Result * result = route.at(k);
						int r;
						if((r = findResult(*result)) == -1)
						{
							result->setHit(1);
							result->setCrossed(0);
							results.push_back(result);
						}
						else
							results.at(r)->failure(currentTime - creature->getCreateTime());
					}
					creature->saveToFile(simulationOutput, currentTime, lane);
					creaturesHit++;
					if(DEBUG)
						cout << "  Creature Hit!" << endl;
				}
			}
			else
				break;
		}
	}
	else
	{
		//check if a creature has been hit
		for(long int c=cell; c > (cell-speed); c--)
		{
			if(c >= 0)
			{
				if(lanes.at(lane)[c].hasCreature())
				{
					Creature * creature = lanes.at(lane)[c].removeCreature();
					vector<Result*> route = creature->getRoutes();
					unsigned int k;
					for(k = 0; k < route.size(); k++)
					{
						Result * result = route.at(k);
						int r;
						if((r = findResult(*result)) == -1)
						{
							result->setHit(1);
							result->setCrossed(0);
							results.push_back(result);
						}
						else
							results.at(r)->failure(currentTime - creature->getCreateTime());
					}
					creature->saveToFile(simulationOutput, currentTime, lane);
					creaturesHit++;
					if(DEBUG)
						cout << "  Creature Hit!" << endl;
				}
			}
			else
				break;
		}
	}
}

//moves creatures from the queueing positions at the side of the road onto the highway
void Simulation::moveCreaturesOntoHighway(void)
{
	//creature moves before the car onto the highway so that it has the chance to get hit
	//move creatures onto the highway
	for(unsigned int c = 0; c < creatureQueues.size(); c++)
	{
		//if this is nonzero, a creature moved ahead into this cell 
		//we must prevent the creature from moving again within the same timestep
		int moved_pos = -1; 

		//check if the crosspoint has a creature waiting to cross
		if(creatureQueues.at(c).size() != 0)
		{
			int cell = c;//= crossPoints.at(c);
			//int cell = c;
			Creature * creature = creatureQueues.at(c).front();
			
			//estimate the speed & prox of the cars in the cell the creature may move into
			updateProxSpeed(creature, 0, cell);

			bool shouldMove = false;
			if(DEBUG)
				cout << "  Creature considering move onto highway at cell: " << c << "...\n";
			
			switch(intelligence)
			{
				case NAIIVE:
					shouldMove = naiiveCreatureShouldMove(*creature, 0);
				break;
				case NAIIVE_FEAR:
					shouldMove = naiiveCreatureShouldMove2(*creature, 0);
				break;
				default:
					shouldMove = naiiveCreatureShouldMove(*creature, 0);
				break;
			}

			if((shouldMove == true) && (cell != moved_pos))
			{
				//check if the lane, cell already has a creature
				if((!lanes.at(0)[cell].hasCreature()))
				{
					if(DEBUG)
						cout << "  moving onto highway.\n";
					creatureQueues.at(c).erase(creatureQueues.at(c).begin());
					creature->setStart(currentTime);
					
					updateProxSpeed(creature, 0, cell);
					
					Result * r = new Result(cell, 0, creature->getProximity(), creature->getSpeed(), 0, 0);
					creature->addRoute2(r);
					lanes.at(0)[cell].addCreature(creature);
				}
				else
				{
					if(DEBUG)
						cout << "  not moving onto highway because the cell is already occupied in the next lane\n";
					totalWaitTime++;
						//update the configuration so that a creature is waiting
					int r;
					Result * result = new Result(cell, 0, creature->getProximity(), creature->getSpeed(), 0, 0);
					r = findResult(*result);

					//only set the waiting state to true if the result actually exists already
					if(r != -1)
						results.at(r)->setWaiting(true);

					//see if the creature should move to the left, right or stay put (33% each)
					float decision = UPR.DrawFloat();
					//int distance = UPR.DrawInteger(this->horizontal_hop);
					int distance = 1;

					//left
					if((decision < ((float)1/(float)3)) && ((c-distance)>=0) && ((creature->getCrossPoint() - (c + distance)) <= this->horizontal_max))
					{
						if(DEBUG)
							cout << "Creature moving left from cell " << c << " to cell " << c-1 << endl;
						creatureQueues.at(c).erase(creatureQueues.at(c).begin());
						creatureQueues.at(c-distance).push_back(creature);
					}
					//right
					else if((decision < ((float)2/(float)3)) && ((c+distance)<cellNum-1) && ((c + distance) - creature->getCrossPoint() <= this->horizontal_max))
					{
						if(DEBUG)
							cout << "Creature moving right from cell " << c << " to cell " << c+1 << endl;
						creatureQueues.at(c).erase(creatureQueues.at(c).begin());
						creatureQueues.at(c+distance).push_back(creature);

						//advance past the next creature queue if the creature we just put into it is the only one
						//(prevents creature from moving right and then onto the highway in one timestep)
						//if(creatureQueues.at(c+distance).size() == 1)
						//	moved_pos = c+distance;
					}
					//do nothign
					else
					{ 
						if(DEBUG)
							cout << "Creature not moving left or right" << endl; 
						/* do nothing */ 
					}
				}
			}
			//creature decides not to move
			else
			{
				//reset the moved_pos because we've avoided moving the same creature twice now
				if(moved_pos == c)
					moved_pos = -1;
				else
				{
					totalWaitTime++;
					//update the configuration so that a creature is waiting
					int r;
					Result * result = new Result(cell, 0, creature->getProximity(), creature->getSpeed(), 0, 0);
					r = findResult(*result);

					//only set the waiting state to true if the result actually exists already
					if(r != -1)
						results.at(r)->setWaiting(true);
				
					//see if the creature should move to the left, right or stay put (33% each)
					float decision = UPR.DrawFloat();
					//int distance = UPR.DrawInteger(this->horizontal_hop);
					int distance = 1;

					//left
					if((decision < ((float)1/(float)3)) && ((c-distance)>=0) && (abs((int)(c + distance) - creature->getCrossPoint()) <= (int)this->horizontal_max))
					{
						if(DEBUG)
							cout << "Creature moving left from cell " << c << " to cell " << c-1 << endl;
						creatureQueues.at(c).erase(creatureQueues.at(c).begin());
						creatureQueues.at(c-distance).push_back(creature);
					}
					//right
					else if((decision < ((float)2/(float)3)) && ((c+distance)<cellNum-1) && (abs((int)(c + distance) - creature->getCrossPoint()) <= (int)this->horizontal_max))
					{
						if(DEBUG)
							cout << "Creature moving right from cell " << c << " to cell " << c+1 << endl;
						creatureQueues.at(c).erase(creatureQueues.at(c).begin());
						creatureQueues.at(c+distance).push_back(creature);

						//advance past the next creature queue if the creature we just put into it is the only one
						//(prevents creature from moving right and then onto the highway in one timestep)
						//if(creatureQueues.at(c+distance).size() == 1)
						//	moved_pos = c+distance;
					}
					//do nothing
					else
					{ 
						if (DEBUG)
							cout << "Creature not moving left or right" << endl; /* do nothing */ 
					}
				}
			}
		}
	}
}

// displays the simulation's configuration information
// used for debugging
void Simulation::displayCfg(void)
{
	cout << "\nSimulation Configuration: " << endl;
	cout << "===========================================" << endl;
	cout << "maxTime: " << maxTime << endl;
	cout << "maxCreatures: " << maxCreatures << " (0=disabled)" << endl;
	cout << "maxCars: " << maxCars << " (0=disabled)" << endl;
	cout << "cellNum: " << cellNum << endl;
	cout << "laneNum: " << laneNum << endl;
	cout << "maxSpeed: " << maxSpeed << endl;
	cout << "entryCarProb: " << entryCarProb << endl;
	cout << "entryCreatureProb: " << entryCreatureProb << endl;
	cout << "crossPoints: ";
	for(unsigned int i = 0; i < crossPoints.size(); i++)
		cout << crossPoints.at(i) << " ";
	cout << endl;
	cout << "Distances: ";
	for(unsigned int i = 0; i < distances.size(); i++)
		cout << distances[i] << " ";
	cout << endl;
	cout << "Velocities: ";
	for(unsigned int i = 0; i < velocities.size(); i++)
		cout << velocities[i] << " ";
	cout << endl;
	cout << "Backward Lanes: ";
	for(unsigned int i = 0; i < backwards.size(); i++)
		cout << backwards[i] << " ";
	cout << endl;
	cout << "Random Decel: " << randomDecel << endl;
	cout << "Intelligence: " << intelligence << endl;
	cout << "KB output: " << kbOutput << endl;
	cout << "Fear: " << fixedFear << " Desire: " << fixedDesire << endl;
	cout << "Propensity: " << propright << endl;
	cout << "Horizontal Hops: " << horizontal_hop << endl;
	cout << "Horizontal Max: " << horizontal_max << endl;
}

// returns the cellid of the nearest car so that we can get the speed and distance of it
// given the max distance away and the lane & cell to check from
// todo either direction (right now only looks left)
// returns -1 if there are no cars within the distance
int Simulation::carNearby(unsigned int lane, unsigned int distance, unsigned int cell)
{
	if(lanes.at(lane)[0].getDirection())
	{
		unsigned int end = cell + distance;
		if(end >= cellNum)
			end = cellNum - 1;
		
		while(cell < end)
		{
			if(lanes.at(lane)[cell].isOccupied())
				return cell;
			cell++;
		}
	}
	else
	{
		unsigned int end = cell - distance;
		if(end < 0)
			end = 0;

		while(cell >= end)
		{
			if(lanes.at(lane)[cell].isOccupied())
				return cell;
			cell--;
		}
	}
	return -1;
}

// returns the cell of the closest car in a particular lane
// or -1 if there is none
int Simulation::closestCar(int lane, int cell)
{
	int end = 0;
	int current = cell;
	while(current >= end)
	{
		if(lanes.at(lane)[current].isOccupied())
		{
			if(DEBUG)
				cout << "CAR FOUND AT " << current << endl;
			return current;
		}
		current--;
	}
	return -1;
}

// Implements the random decel feature from the traffic model by Nagel & Schrekenburg
void Simulation::doRandomDecel(int lane)
{
	//random decel
	for(unsigned int i = 0; i < cellNum; i++)
	{
		if(lanes.at(lane)[i].isOccupied())
		{
			float p = UPR.DrawFloat();
			if(p < SLOW_PROB)
			{
				Car * car = lanes.at(lane)[i].getCar();
				unsigned int speed = car->getSpeed();

				//only let the speed go down to 1 so that the cars don't totally stop
				if(speed > 0)
				{
					car->setSpeed(speed - 1);
					if(DEBUG)
						cout << "Decel car: " << car->getID() << " in lane " << lane << " at cell " << i << endl;
				}
			}
		}
	}
}

//returns the location in the results vector of the matching result, or -1
int Simulation::findResult(Result r)
{
	//if(DEBUG)
	//	cout << "Displaying results: " << endl;
	for(unsigned int c=0; c<results.size(); c++)
	{
		//if(DEBUG)
		//	cout << "  " << results.at(c)->getProximity() << " " << results.at(c)->getSpeed() << " " << results.at(c)->getCrossPoint() << endl;
		if( (r.getProximity() == results.at(c)->getProximity()) && (r.getSpeed() == results.at(c)->getSpeed()) && (r.getCrossPoint() == results.at(c)->getCrossPoint()) )
			return c;
	}
	return -1;
}

void Simulation::outputTimeDependentData(void)
{
	//KB output within the trace at each timestep
	if(kbOutput)
	{
		*simulationOutput << "Timestep: " << currentTime << " (cross, hit)" << endl;
		*simulationOutput << setw(20) << "Speed (note: 0 = clear, 1 = low)" << endl;

		*simulationOutput << " ";
		for(unsigned int s=0; s <= velocities.size(); s++)
		{
			*simulationOutput << setw(12) << s;
		}
		*simulationOutput << endl;

		char * label = "Proximity";

		*configDependentOutput << "timestep: " << currentTime-1 << endl;
		*configDependentOutput << "=======================================================================" << endl;
		*configDependentOutput << setw(10) << "Cross" << setw(10) << "Speed" << setw(10) << "Prox" << setw(10) << "Ns" << setw(10) << "Nk" << setw(10) << "Nw" << setw(10) << "Ts" << setw(10) << "Tk" << endl;
		*configDependentOutput << "=======================================================================" << endl;

		for(unsigned int p=0; p <= distances.size(); p++)
		{
			if(p < strlen(label))
				*simulationOutput << label[p] << " ";
			*simulationOutput << p;
			for(unsigned int s=0; s <= velocities.size(); s++)
			{
				for(unsigned int c=0; c<results.size(); c++)
				{
					//only print the results at the row,col we are looking for
					if((results.at(c)->getProximity() == p) && (results.at(c)->getSpeed() == s))
					{
						*simulationOutput << setw(10) << results.at(c)->getCrossed() << "," << results.at(c)->getHit();

						//todo - figure out if a creature has been waiting or not
						*configDependentOutput << setw(10) << results.at(c)->getCrossPoint() << setw(10) << s << setw(10) << p << setw(10) << results.at(c)->getCrossed() << setw(10) << results.at(c)->getHit() << setw(10) << results.at(c)->getWaiting() << setw(10) << results.at(c)->getTimeSuccess() << setw(10) << results.at(c)->getTimeKilled() << endl;
					}
				}
			}
			*simulationOutput << endl;
		}
		*simulationOutput << endl;

		//output the config independent stats
		int totalQueueSize = 0;
		for(unsigned int c = 0; c < creatureQueues.size(); c++)
		{
			//check if the crosspoint has a creature waiting to cross
			totalQueueSize+=creatureQueues.at(c).size();
		}

		*configIndependentOutput << setw(10) << currentTime << setw(10) << creaturesCrossed << setw(10) << creaturesHit << setw(10) << totalQueueSize << setw(10) << totalWaitTime << endl;
	}
}

void Simulation::outputKnowledge(void)
{
	//output the results
	if(DEBUG)
	{
		cout << "Knowledgebase: " << endl;
		cout << "0 = too fast, too far away, 1 = slow, near" << endl;
		cout << "Cross Lane Proximity Speed Crossed Hit" << endl;
		for(unsigned int c=0; c<results.size(); c++)
		{
			cout << "  " << results.at(c)->getCrossPoint() << "    " << results.at(c)->getLane() << "       " << results.at(c)->getProximity() << "       " << results.at(c)->getSpeed() << "      " << results.at(c)->getCrossed() << "    " << results.at(c)->getHit() << endl; //results.at(c)->getOutcome() << endl;
		}
		cout << "Hit: " << creaturesHit << endl;
		cout << "Crossed: " << creaturesCrossed << endl;
		cout << "Created: " << creaturesCreated << endl;
	}

	*knowledgeOutput << setw(10) << "CrossPoint" << setw(10) << "Lane" << setw(15) << "Prox (1=near)" << setw(15) << "Speed (1=slow)" << setw(10) << "Crossed" << setw(10) << "Hit" << setw(16) << "Crossed Mean" << setw(13) << "Crossed Var" << setw(13) << "Hit Mean" << setw(13) << "Hit Var" << endl;
	for(unsigned int c=0; c<results.size(); c++)
	{
		int total = results.at(c)->getTotal();
		float cross_mean; float cross_variance, hit_mean, hit_variance;
		if(total == 0)
		{
			cross_mean = 0;
			cross_variance = 0;
			hit_mean = 0;
			hit_variance = 0;
		}
		else
		{
			cross_mean = (float)results.at(c)->getCrossedTotal() / (float)total;
			cross_variance = (float)(results.at(c)->getCrossedSquaredTotal() / (float)total) - (float)(cross_mean * cross_mean);
			hit_mean = (float)results.at(c)->getHitTotal() / (float)total;
			hit_variance = (float)(results.at(c)->getHitSquaredTotal() / total) - (float)(hit_mean * hit_mean);
		}
		*knowledgeOutput << setw(10) << results.at(c)->getCrossPoint() << setw(10) << results.at(c)->getLane() << setw(12) << results.at(c)->getProximity() << setw(12) << results.at(c)->getSpeed() << setw(12) << results.at(c)->getCrossed() << setw(12) << results.at(c)->getHit() //results.at(c)->getOutcome() << endl;
			<< setw(13) << cross_mean << setw(13) << cross_variance << setw(13) << hit_mean << setw(13) << hit_variance << endl;
	}
}

void Simulation::setMaxCreatures(unsigned long maxCreatures)
{
	this->maxCreatures = maxCreatures;
}

// Sets the "fuzzy" distances for naiive algorithm, should be in low->high order
void Simulation::setDistances(vector<unsigned int> distances)
{
	this->distances = distances;
}

// Sets the "fuzzy" velocities for naiive algorithm, should be in low->high order
void Simulation::setVelocities(vector<unsigned int> velocities)
{
	this->velocities = velocities;
}

// Controls whether the simulation implements the random decel feature from
// the traffic modelling paper by Nagel and Schreckenburg
void Simulation::setRandomDecel(bool randomDecel)
{
	this->randomDecel = randomDecel;
}

// Sets the intelligence algorithm used for the simulation creature
void Simulation::setIntelligence(int intelligence)
{
	this->intelligence = intelligence;
}

//Accessor method to return the intelligence of the current simulation instance
int Simulation::getIntelligence(void)
{
	return this->intelligence;
}

// Sets the fear to be fixed for all creatures created
// note: if set to -1, it will remain random
void Simulation::setFixedFear(float fear)
{
	this->fixedFear = fear;
}

// Sets the desire to be fixed for all creatures created
// note: if set to -1, it will remain random
void Simulation::setFixedDesire(float desire)
{
	this->fixedDesire = desire;
}

void Simulation::setFuzzyMembers(int fuzzyMembers)
{
	this->fuzzyMembers = fuzzyMembers;
}

void Simulation::setKBOutput(bool kbOutput)
{
	this->kbOutput = kbOutput;
}

//used for importing prior knowledge into the knowledge base
void Simulation::addResult(Result * r)
{
	results.push_back(r);
}

unsigned long int Simulation::getMaxTime(void)
{
	return this->maxTime;
}

unsigned long int Simulation::getCellNum(void)
{
	return this->cellNum;
}

unsigned short int Simulation::getLaneNum(void)
{
	return this->laneNum;
}

unsigned int Simulation::getMaxSpeed(void)
{
	return this->maxSpeed;
}

float Simulation::getCarEntryProb(void)
{
	return this->entryCarProb;
}

float Simulation::getCreatureEntryProb(void)
{
	return this->entryCreatureProb;
}

vector<unsigned int> Simulation::getCrossPoints(void)
{
	return this->crossPoints;
}

vector<unsigned int> Simulation::getDistances(void)
{
	return distances;
}

vector<unsigned int> Simulation::getVelocities(void)
{
	return velocities;
}

bool Simulation::getRandomDecel(void)
{
	return this->randomDecel;
}

int Simulation::getFuzzyMembers(void)
{
	return this->fuzzyMembers;
}

bool Simulation::getKBOutput(void)
{
	return this->kbOutput;
}

unsigned long int Simulation::getCreaturesCreated(void)
{
	return this->creaturesCreated;
}

unsigned long int Simulation::getCreaturesHit(void)
{
	return this->creaturesHit;
}

unsigned long int Simulation::getCreaturesCrossed(void)
{
	return this->creaturesCrossed;
}

float Simulation::getFixedDesire(void)
{
	return fixedDesire;
}

float Simulation::getFixedFear(void)
{
	return fixedFear;
}

void Simulation::setTraceOutput(bool tr)
{
	traceOutput = tr;
}

bool Simulation::getTraceOutput(void)
{
	return traceOutput;
}

unsigned long int Simulation::getMaxCars(void)
{
	return maxCars;
}

unsigned long int Simulation::getMaxCreatures(void)
{
	return maxCreatures;
}

void Simulation::setBackwards(vector<unsigned int> backwards)
{
	this->backwards = backwards;
}

vector<unsigned int> Simulation::getBackwards(void)
{
	return this->backwards;
}

void Simulation::setHorizontalMax(unsigned int horizontal_max)
{
	this->horizontal_max = horizontal_max;
}

unsigned int Simulation::getHorizontalMax(void)
{
	return horizontal_max;
}

void Simulation::setHorizontalHop(unsigned int horizontal_hop)
{
	this->horizontal_hop = horizontal_hop;
}

unsigned int Simulation::getHoriztonalHop(void)
{
	return horizontal_hop;
}

void Simulation::setSimulationTrace(ofstream * out)
{
	simulationOutput = out;
}

void Simulation::setKnowledgeOutput(ofstream * out)
{
	knowledgeOutput = out;
}

void Simulation::setConfigDependentOutput(ofstream * out)
{
	configDependentOutput = out;
}

void Simulation::setConfigIndependentOutput(ofstream * out)
{
	configIndependentOutput = out;
}

void Simulation::setMaxCars(unsigned long maxCars)
{
	this->maxCars = maxCars;
}

void Simulation::setPropRight(float propright)
{
	this->propright = propright;
}

float Simulation::getPropRight(void)
{
	return this->propright;
}

Simulation::~Simulation(void)
{

}

//return the mid value between two values
int getmid(int a1, int a2)
{
	return (a2 + a1) / 2;
}

//return the mid value of two floats
float getfmid(float a1, float a2)
{
	return (a2 + a1) / 2;
}

void midValue(unsigned int * array, int low, int high)
{
	int mid = getmid(low, high);
	if((mid <= low) || (mid >= high))
		return;
	else
		array[mid] = getmid(array[low], array[high]);

	midValue(array, low, mid);
	midValue(array, mid, high);
}