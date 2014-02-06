#pragma once
#include "Result.h"
#include "Cell.h"
#include "Creature.h"
#include "UniformPseudoRandomMarsaglia.h"

#define DEBUG		0				// enables output for debugging
#define SLOW_PROB	0.5				// probability a given car will slow down randomly

#define NAIIVE			0
#define NAIIVE_FEAR		1
#define FUZZY			2
#define NEW_FUZZY		3
#define NEW_FUZZY_FEAR	4

class Simulation
{
	//simulation parameters
	unsigned long int maxTime;
	unsigned long int maxCars;
	unsigned long int maxCreatures;
	unsigned long int cellNum;				//max cells per lane
	unsigned short int laneNum;				//max lanes
	unsigned int maxSpeed;
	unsigned int horizontal_max;			//limits on creatures' horiztonal movements
	unsigned int horizontal_hop;
	vector<unsigned int> distances;			//distances and velocity values for naiive algorithms (from low to high)
	vector<unsigned int> velocities;
	vector<unsigned int> backwards;			//list of the lane ids that go backwards
	float entryCarProb;						//probability a car appears at a crosspoint during a timestep (between 0 & 1)
	float entryCreatureProb;				//probabiliy a creature appears at a crosspoint during a timestep (between 0 & 1)
	float fixedFear, fixedDesire;			//allows fixed fear and desire, if either is -1, each creature will have randomized value (between 0 & 1)
	float propright;						//expected value propensity for cars to return to rightmost lane
	int fuzzyMembers;						//controls how many membership functions exist for estimating prox & speed (should be at least 3)
	bool kbOutput;							//if set to true, the trace output will contain the knowledgebase for each timestep
	bool traceOutput;						//if false, no trace files produced

	vector<unsigned int> crossPoints;
	UniformPseudoRandomMarsaglia UPR;

	//output file streams
	ofstream * simulationOutput;
	ofstream * knowledgeOutput;
	ofstream * configDependentOutput;
	ofstream * configIndependentOutput;

	unsigned long totalWaitTime;

	//experiment id (used to help seed the random generator)
	unsigned long experiment_id;

	//simulation variables
	unsigned long int currentTime;
	unsigned long int carsCreated;
	unsigned long int creaturesCreated;
	unsigned long int creaturesHit;
	unsigned long int creaturesCrossed;
	bool randomDecel;
	int intelligence;

	//Cell * lane;
	vector<Cell *> lanes;
	vector<queue<Car *>> carQueues;					//an array of queues of cars (one for each lane)
	vector<vector<Creature *>> creatureQueues;		//an array of queues of creatures (one for each crosspoint)

	vector<Result *> results;						//an array of sim states that a creature either died or survived in

	bool naiiveCreatureShouldMove(Creature c, int lane);
	bool naiiveCreatureShouldMove2(Creature c, int lane);
	int naiiveGetSpeed(Creature c, int lane);
	int naiiveGetProx(Creature c, int lane);
	int carNearby(unsigned int lane, unsigned int distance, unsigned int cell);
	int closestCar(int lane, int cell);
	void generateCars(void);
	void generateCreatures(void);
	unsigned int collisionSpeed(unsigned int lane, unsigned int position, unsigned int maxspeed);
	void updateCarSpeeds(void);
	void moveCars(void);
	void moveCreaturesOntoHighway(void);
	void checkCreatureHit(int lane, int cell, int speed);
	void doRandomDecel(int lane);
	void outputKnowledge(void);
	void outputTimeDependentData(void);
	void updateProxSpeed(Creature * creature, int lane, int cell);
	void updateWaiting(int prox, int speed);
public:
	Simulation(void);
	Simulation(unsigned long int maxTime, unsigned long int cellNum, unsigned short int laneNum, unsigned int maxSpeed, float entryCarProb, float entryCreatureProb, vector<unsigned int> crossPoints, unsigned long experiment_id, vector<unsigned int> backwards);
	int findResult(Result r);
	void displayCfg(void);
	void run(void);
	void setMaxCars(unsigned long maxCars);
	void setMaxCreatures(unsigned long maxCreatures);
	void setDistances(vector<unsigned int> distances);
	vector<unsigned int> getDistances(void);
	void setVelocities(vector<unsigned int> velocities);
	vector<unsigned int> getVelocities(void);
	void setSimulationTrace(ofstream * out);
	void setKnowledgeOutput(ofstream * out);
	void setConfigDependentOutput(ofstream * out);
	void setConfigIndependentOutput(ofstream * out);
	void setRandomDecel(bool randomDecel);
	void setBackwards(vector<unsigned int> backwards);
	vector<unsigned int> getBackwards(void);
	bool getRandomDecel(void);
	void setIntelligence(int intelligence);
	int getIntelligence(void);
	void setFixedFear(float fear);
	float getFixedFear(void);
	void setFixedDesire(float desire);
	float getFixedDesire(void);
	void setFuzzyMembers(int fuzzyMembers);
	int getFuzzyMembers(void);
	void setKBOutput(bool kbOutput);
	bool getKBOutput(void);
	void addResult(Result * r);
	unsigned long int getMaxTime(void);
	unsigned long int getCellNum(void);
	unsigned short int getLaneNum(void);
	unsigned int getMaxSpeed(void);
	float getCarEntryProb(void);
	float getCreatureEntryProb(void);
	unsigned long int getCreaturesCreated(void);
	unsigned long int getCreaturesHit(void);
	unsigned long int getCreaturesCrossed(void);
	vector<unsigned int> getCrossPoints(void);
	unsigned long int getMaxCars(void);
	unsigned long int getMaxCreatures(void);
	void setTraceOutput(bool tr);
	bool getTraceOutput(void);
	void setHorizontalMax(unsigned int horizontal_max);
	unsigned int getHorizontalMax(void);
	void setHorizontalHop(unsigned int horizontal_hop);
	unsigned int getHoriztonalHop(void);
	void setPropRight(float propright);
	float getPropRight(void);
	~Simulation(void);
};
