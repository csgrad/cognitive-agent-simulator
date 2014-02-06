/*
 * Experiment is similar to the original creatureSimulation program
 * except it is designed for running multiple experiments from a configuration
 * file. The main difference is the configuration file should now 
 * contain ranges for values which should flucuate, and the number of repetitions
 * of the experiment. For each repetition, the random number seed will increment
 * allowing reproducable results (ie seed starts at zero), while allowing for
 * exploration of varying conditions.
 *
 * The aggregate stats which show how many creatures crossed, were hit and the ratio
 * are printed in an output file with the name generated based in the input parameters
 * and stored one result per line. The number of lines in the file depends on the number
 * of repetitions of the experiment.
 *
 * The knowledgeout and trace file names will also be auto generated based on the parameters for the
 * current simulation run. The knowledgein file is optional - it is only needed to import previous knowledge
 * from an existing set of experiments.
 */
#include "stdafx.h"
#include "Simulation.h"

#define NUM_ARGS	1				/* Number of required command-line arguments */
#define OPT_ARGS	1				/* Number of optional arguments */
#define SIM_ERROR	-1				/* return values for success and failure */
#define SIM_SUCCESS	0

int readConfiguration(string fileName);
int readPriorKnowledge(string fileName);
int runExperiment(float creature_prob, float car_prob);
int repeats;

//maintain two simulation instances (one is never actually run, just used to we dont need to re-parse the cfg file each run
Simulation * cfg;
Simulation * sim;
string prevkbfile;		//keeps track of the previous kb file so we can read it into the next experiment for learning
bool kbpreserve;

float car_prob_max_value = 0;	//used for varying a variable in the experiment
float cre_prob_max_value = 0;
float inc_value = 0;

int main(int argc, char * argv[])
{
	//check for correct number of args
	if((argc < NUM_ARGS+1) || argc > NUM_ARGS + OPT_ARGS + 1)
	{
		cout << "Usage: creatureSimulation-yy-xx <simcfg> <knowledgein>" << endl;
		return SIM_ERROR;
	}

	//In an experiment, all we need for successful input is the configuration file. All the rest are created by this program and
	//re-used for later simulation runs
	cout << "Reading configuration input file: '" << argv[1] << "'...\n";
	if(readConfiguration(argv[1]) != SIM_SUCCESS)
	{
		cout << "Error reading configuration file" << endl;
		return SIM_ERROR;
	}
	cout << "Success" << endl;
	cout << "Running the experiment for " << repeats << " repetition(s)" << endl;

	if(argc == NUM_ARGS + OPT_ARGS + 1)
		prevkbfile = argv[2];

	if(car_prob_max_value != 0) //varying car prob
	{
		float prob;
		for(prob = cfg->getCarEntryProb(); prob <= car_prob_max_value; prob = prob + inc_value)
		{
			if(!kbpreserve)
				prevkbfile = "";
			cout << "  Car Prob: " << prob << endl;
			runExperiment(cfg->getCreatureEntryProb(), prob);
		}
	}
	else if(cre_prob_max_value != 0) //varying creature prob
	{
		float prob;
		for(prob = cfg->getCreatureEntryProb(); prob <= cre_prob_max_value; prob = prob + inc_value)
		{
			if(!kbpreserve)
				prevkbfile = "";
			cout << "  Creature Prob: " << prob << endl;
			runExperiment(prob, cfg->getCarEntryProb());
		}
	}
	else //single run
	{
		runExperiment(cfg->getCreatureEntryProb(), cfg->getCarEntryProb());
	}

	return SIM_SUCCESS;
}

int runExperiment(float creature_prob, float car_prob)
{
	for(int r=0; r < repeats; r++)
	{
		cout << "    Run " << r+1 << " of " << repeats;

		//init sim params for this run
		sim = new Simulation(cfg->getMaxTime(), cfg->getCellNum(), cfg->getLaneNum(), cfg->getMaxSpeed(), car_prob, creature_prob, cfg->getCrossPoints(), r+1, cfg->getBackwards());
		sim->setDistances(cfg->getDistances());
		sim->setVelocities(cfg->getVelocities());
		sim->setRandomDecel(cfg->getRandomDecel());
		sim->setIntelligence(cfg->getIntelligence());
		sim->setFuzzyMembers(cfg->getFuzzyMembers());
		sim->setKBOutput(cfg->getKBOutput());
		sim->setFixedDesire(cfg->getFixedDesire());
		sim->setFixedFear(cfg->getFixedFear());
		sim->setTraceOutput(cfg->getTraceOutput());
		sim->setMaxCars(cfg->getMaxCars());
		sim->setMaxCreatures(cfg->getMaxCreatures());
		sim->setHorizontalHop(cfg->getHoriztonalHop());
		sim->setHorizontalMax(cfg->getHorizontalMax());
		sim->setPropRight(cfg->getPropRight());

		//read in previous knowledge (may not be necessary in the first run if no kb file was specified)
		if(prevkbfile != "")
		{
			if(readPriorKnowledge(prevkbfile)!= SIM_SUCCESS)
			{
				cout << "Error Reading KB file '" << prevkbfile << "'" << endl;
				return SIM_ERROR;
			}
		}

		//generate file names
		ostringstream filebase;
		filebase << "i" << cfg->getIntelligence() << "c" << cfg->getCellNum() << "l" << cfg->getLaneNum() << "t" << cfg->getMaxTime() << "car" << car_prob << "cre" << creature_prob << "fear" << cfg->getFixedFear() << "desire" << cfg->getFixedDesire() << "prop" << cfg->getPropRight();

		//open kb file
		ostringstream kbstr;
		kbstr << filebase.str() << "rep" << r+1 << "-kb.txt";
		ofstream kbFile(kbstr.str(), ofstream::out);
		if(kbFile.is_open())
			sim->setKnowledgeOutput(&kbFile);
		else
		{
			cout << "  Error opening the trace file '" << kbstr.str() << "' for the simulation run" << endl;
			return SIM_ERROR;
		}
		prevkbfile = kbstr.str();	//set the prev kb file name to this file

		//open trace file
		ostringstream tracestr;
		tracestr << filebase.str() << "rep" << r+1 << "-trace.txt";
		ofstream traceFile(tracestr.str(), ofstream::out);
		if(traceFile.is_open())
			sim->setSimulationTrace(&traceFile);
		else
		{
			cout << "  Error opening the trace file '" << tracestr.str() << "' for the simulation run" << endl;
			return SIM_ERROR;
		}

		//open the config depenedent stat file
		ostringstream configdependentstr;
		configdependentstr << filebase.str() << "rep" << r+1 << "-configdependent.txt";
		ofstream configdependentFile(configdependentstr.str(), ofstream::out);
		if(configdependentFile.is_open())
			sim->setConfigDependentOutput(&configdependentFile);
		else
		{
			cout << "  Error opening the config dependent stat file '" << configdependentstr.str() << "' for the simulation run" << endl;
			return SIM_ERROR;
		}

		//open the config indepenedent stat file
		ostringstream configindependentstr;
		configindependentstr << filebase.str() << "rep" << r+1 << "-configindependent.txt";
		ofstream configindependentFile(configindependentstr.str(), ofstream::out);
		if(configindependentFile.is_open())
			sim->setConfigIndependentOutput(&configindependentFile);
		else
		{
			cout << "  Error opening the config independent stat file '" << configindependentstr.str() << "' for the simulation run" << endl;
			return SIM_ERROR;
		}

		//open aggregate stat file
		ofstream statFile;
		if(r == 0)
			statFile.open(filebase.str() += "-stats.txt", ofstream::out);
		else
			statFile.open(filebase.str() += "-stats.txt", ofstream::app);
		if(!statFile.is_open())
		{
			cout << "  Error opening the aggregate stat file '" << filebase.str() << "-stats.txt" << "' for the simulation run" << endl;
			return SIM_ERROR;
		}

		//fill out the rest of the KB table with zeros
		vector<unsigned int> crossPoints = cfg->getCrossPoints();

		//all possible crosspoints
		for(unsigned int c=0; c < crossPoints.size(); c++)
		{
			//all possible lanes
			for(int l=0; l < sim->getLaneNum(); l++)
			{
				//all possible proximities
				for(unsigned int p=0; p <= sim->getDistances().size(); p++)

					//all possible speeds
					for(unsigned int s=0; s <= sim->getVelocities().size(); s++)
					{
						Result * r = new Result(crossPoints[c], l, p, s, 0, 0);
						if((sim->findResult(*r)) == -1)
							sim->addResult(r);
					}
			}
		}

		//run simulation here
		sim->displayCfg();
		sim->run();

		cout << "Simulation run completed" << endl;

		//add results of simulation to statfile
		float successRatio = (float)sim->getCreaturesCrossed() / (float)sim->getCreaturesCreated();
		unsigned long int queuedCreatures = sim->getCreaturesCreated() - (sim->getCreaturesCrossed() + sim->getCreaturesHit());
		if(r == 0)
			statFile << setw(10) << "Intelligence" << setw(15) << "CarProb" << setw(15) << "CreatureProb" << setw(20) << "Creatures Created" << setw(10) << "Success" << setw(10) << "Hit" << setw(20) << "Queued Creatures" << setw(18) << "Success Ratio" << endl; //if first run add a header to the output file
		
		statFile << setw(10) << sim->getIntelligence() << setw(15) << sim->getCarEntryProb() << setw(15) << sim->getCreatureEntryProb() << setw(20) << sim->getCreaturesCreated() << setw(10) << sim->getCreaturesCrossed() << setw(10) << sim->getCreaturesHit() << setw(20) << queuedCreatures << setw(18) << successRatio << endl;

		//close output files
		kbFile.close();
		traceFile.close();
		statFile.close();
		configdependentFile.close();
		configindependentFile.close();
		cout << endl;
	}

	return SIM_SUCCESS;
}

// Reads the configuration file input1 which contains all of the simulation parameters
// returns SIM_ERROR on error or SIM_SUCCESS on success
int readConfiguration(string fileName)
{
	unsigned long int maxTime = 0;
	unsigned long int maxCreatures = 0;
	unsigned long int maxCars = 0;
	unsigned long int cellNum = 0;
	unsigned short int laneNum = 0;
	unsigned int horizontal_max = 0;
	unsigned int horizontal_hop = 0;
	vector <unsigned int> distances;
	vector <unsigned int> velocities;
	vector <unsigned int> backwards;
	unsigned int maxSpeed = 0;
	bool randomDecel = false;
	int intelligence = 0;
	int fuzzyMembers = 3;
	bool kbOutput = false;
	bool traceOutput = true;
	float entryCarProb = 0.0;
	float entryCreatureProb = 0.0;
	float fixedFear = 0.0;
	float fixedDesire = 0.0;
	float propright = 0.0;
	vector<unsigned int> crossPoints;

	ifstream infile(fileName, ifstream::in);
	
	//if we want to support more than 10 velocities and distances we need to increase parameters here
	string line, cmd, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10;

	if(infile.is_open())
	{
		getline(infile, line);

		//parse each line until end of file
		while(!infile.eof())
		{
			istringstream istr(string(line), ios_base::out);
			istr >> cmd >> para1;

			//only set the variables if they haven't already been set (as per spec), except for crossPoints
			if((cmd == "MAX_TIME") && (maxTime == 0))
			{
				maxTime = atoi(para1.c_str());
				if((maxTime == 0) || (maxTime == INT_MAX) || (maxTime == INT_MIN))
					cout << "Error converting maxTime, check the format" << endl;
			}
			else if((cmd == "MAX_CREATURES") && (maxCreatures == 0))
			{
				maxCreatures = atoi(para1.c_str());
				if((maxCreatures == INT_MAX) || (maxCreatures == INT_MIN))
					cout << "Error converting maxCreatures, check the format" << endl;
			}
			else if((cmd == "MAX_CARS") && (maxCars == 0))
			{
				maxCars = atoi(para1.c_str());
				if((maxCars == INT_MAX) || (maxCars == INT_MIN))
					cout << "Error converting maxCars, check the format" << endl;
			}
			else if((cmd == "CELL_NUM") && (cellNum == 0))
			{
				cellNum = atoi(para1.c_str());
				if((cellNum == 0) || (cellNum == INT_MAX) || (cellNum == INT_MIN))
					cout << "Error converting cellNum, check the format" << endl;
			}
			else if((cmd == "LANE_NUM") && (laneNum == 0))
			{
				laneNum = atoi(para1.c_str());
				if((laneNum == 0) || (laneNum == INT_MAX) || (laneNum == INT_MIN))
					cout << "Error converting laneNum, check the format" << endl;
			}
			else if((cmd == "MAX_SPEED") && (maxSpeed == 0))
			{
				maxSpeed = atoi(para1.c_str());
				if((maxSpeed == 0) || (maxSpeed == INT_MAX) || (maxSpeed == INT_MIN))
					cout << "Error converting maxSpeed, check the format" << endl;
			}
			else if((cmd == "ENTRY_CAR_PROB") && (entryCarProb == 0))
			{
				entryCarProb = (float)atof(para1.c_str());
				if((entryCarProb == 0) || (entryCarProb == HUGE_VAL))
					cout << "Error converting entryCarProb, check the format" << endl;
			}
			else if(cmd == "ENTRY_CAR_PROB_MAX")
			{
				car_prob_max_value = (float)atof(para1.c_str());
				if(car_prob_max_value == HUGE_VAL)
					cout << "Error converting ENTRY_CAR_PROB_MAX, check the format" << endl;
			}
			else if(cmd == "ENTRY_CREATURE_PROB_MAX")
			{
				cre_prob_max_value = (float)atof(para1.c_str());
				if(cre_prob_max_value == HUGE_VAL)
					cout << "Error converting ENTRY_CREATURE_PROB_MAX, check the format" << endl;
			}
			else if(cmd == "ENTRY_CAR_PROB_INC" || cmd == "ENTRY_CREATURE_PROB_INC")
			{
				inc_value = (float)atof(para1.c_str());
				if((inc_value == 0) || (inc_value == HUGE_VAL))
					cout << "Error converting ENTRY_CREATURE_PROB_MAX, check the format" << endl;
			}
			else if((cmd == "ENTRY_CREATURE_PROB") && (entryCreatureProb == 0))
			{
				entryCreatureProb = (float)atof(para1.c_str());
				if((entryCreatureProb < 0) || (entryCreatureProb == HUGE_VAL))
					cout << "Error converting entryCreatureProb, check the format" << endl;
			}
			else if(cmd == "CROSS_POINT")
			{
				unsigned int crossPoint = atoi(para1.c_str());
				if((crossPoint == 0) || (crossPoint == INT_MAX) || (crossPoint == INT_MIN))
					cout << "Error converting one of the crossPoints, check the format" << endl;
				crossPoints.push_back(crossPoint);
			}
			else if(cmd == "DISTANCE")
			{
				//if we want to support more than 10 velocities and distances we need to increase parameters here
				istr >> para2 >> para3 >> para4 >> para5 >> para6 >> para7 >> para8 >> para9 >> para10;
				int distance = atoi(para1.c_str());
				if(distance != 0)
					distances.push_back(distance);
				distance = atoi(para2.c_str());
				if(distance != 0)
					distances.push_back(distance);
				distance = atoi(para3.c_str());
				if(distance != 0)
					distances.push_back(distance);
				distance = atoi(para4.c_str());
				if(distance != 0)
					distances.push_back(distance);
				distance = atoi(para5.c_str());
				if(distance != 0)
					distances.push_back(distance);
				distance = atoi(para6.c_str());
				if(distance != 0)
					distances.push_back(distance);
				distance = atoi(para7.c_str());
				if(distance != 0)
					distances.push_back(distance);
				distance = atoi(para8.c_str());
				if(distance != 0)
					distances.push_back(distance);
				distance = atoi(para9.c_str());
				if(distance != 0)
					distances.push_back(distance);
				distance = atoi(para10.c_str());
				if(distance != 0)
					distances.push_back(distance);
			}
			else if(cmd == "VELOCITY")
			{
				//if we want to support more than 10 velocities and distances we need to increase parameters here
				istr >> para2 >> para3 >> para4 >> para5 >> para6 >> para7 >> para8 >> para9 >> para10;
				int velocity = atoi(para1.c_str());
				if(velocity != 0)
					velocities.push_back(velocity);
				velocity = atoi(para2.c_str());
				if(velocity != 0)
					velocities.push_back(velocity);
				velocity = atoi(para3.c_str());
				if(velocity != 0)
					velocities.push_back(velocity);
				velocity = atoi(para4.c_str());
				if(velocity != 0)
					velocities.push_back(velocity);
				velocity = atoi(para5.c_str());
				if(velocity != 0)
					velocities.push_back(velocity);
				velocity = atoi(para6.c_str());
				if(velocity != 0)
					velocities.push_back(velocity);
				velocity = atoi(para7.c_str());
				if(velocity != 0)
					velocities.push_back(velocity);
				velocity = atoi(para8.c_str());
				if(velocity != 0)
					velocities.push_back(velocity);
				velocity = atoi(para9.c_str());
				if(velocity != 0)
					velocities.push_back(velocity);
				velocity = atoi(para10.c_str());
				if(velocity != 0)
					velocities.push_back(velocity);
			}
			else if(cmd == "INTELLIGENCE")
			{
				//see simulation.h for intelligence definitions
				intelligence = atoi(para1.c_str());					
			}
			else if(cmd == "RANDOM_DECEL")
			{
				unsigned int c;
				for(c = 0 ; c < strlen(para1.c_str()); c++)
					toupper(para1[c]);
				if(para1 == "TRUE")
					randomDecel = true;
				else
					randomDecel = false;
			}
			else if(cmd == "FIXED_FEAR")
			{
				fixedFear = (float)atof(para1.c_str());
			}
			else if(cmd == "FIXED_DESIRE")
			{
				fixedDesire = (float)atof(para1.c_str());
			}
			else if(cmd == "FUZZY_MEMBERS")
			{
				fuzzyMembers = atoi(para1.c_str());
			}
			else if(cmd == "KB_OUTPUT")
			{
				unsigned int c;
				for(c = 0 ; c < strlen(para1.c_str()); c++)
					toupper(para1[c]);
				if(para1 == "TRUE")
					kbOutput = true;
				else
					kbOutput = false;
			}
			else if(cmd == "TRACE_OUTPUT")
			{
				unsigned int c;
				for(c = 0 ; c < strlen(para1.c_str()); c++)
					toupper(para1[c]);
				if(para1 == "TRUE")
					traceOutput = true;
				else
					traceOutput = false;
			}
			else if(cmd == "REPEATS")
			{
				repeats = atoi(para1.c_str());
			}
			else if(cmd == "KB_EXPERIMENT")
			{
				unsigned int c;
				for(c = 0 ; c < strlen(para1.c_str()); c++)
					toupper(para1[c]);
				if(para1 == "TRUE")
					kbpreserve = true;
				else
					kbpreserve = false;
			}
			else if(cmd == "LANE_BACKWARDS")
			{
				unsigned int l = atoi(para1.c_str());
				backwards.push_back(l);
			}
			else if(cmd == "HORIZONTAL_MAX")
			{
				horizontal_max = atoi(para1.c_str());
			}
			else if(cmd == "HORIZONTAL_HOP")
			{
				horizontal_hop = atoi(para1.c_str());
			}
			else if(cmd == "PROPENSITY_RIGHT")
			{
				propright = (float)atof(para1.c_str());
				if(propright == HUGE_VAL)
					cout << "Error converting PROPENSITY_RIGHT, check the format" << endl;
			}
			else if((cmd == "") || (cmd[0] == '#') || (cmd[0] == '/'))
			{
				//do nothing (handle blank lines or comments)
			}
			else
				cout << "Cannot read this command (or duplicate): " << cmd << " from this line: \n  " << line << endl;

			getline(infile, line);
		}

		infile.close();
	}
	else
	{
		cout << "Unable to open file: '" << fileName << "'" << endl;
		return SIM_ERROR;
	}

	// check to make sure we have read all of the correct information from the input file
	if( (maxTime == 0) || (cellNum == 0) || (laneNum == 0) || (maxSpeed == 0) || (crossPoints.size() == 0))
	{
		cout << "Reading configuration file: '" << fileName << "' failed, please check the format!" << endl;
		return SIM_ERROR;
	}

	// check that all crosspoints are within the number of cells in the simulation
	for(unsigned int c = 0; c < crossPoints.size(); c++)
	{
		if(crossPoints.at(c) > cellNum)
		{
			cout << "Error, crosspoint: " << crossPoints.at(c) << " is greater than total number of cells: " << cellNum << endl;
			return SIM_ERROR;
		}
	}

	//initialize the simulation using the parameters read from the file
	cfg = new Simulation(maxTime, cellNum, laneNum, maxSpeed, entryCarProb, entryCreatureProb, crossPoints, 0, backwards);
	cfg->setDistances(distances);
	cfg->setVelocities(velocities);
	cfg->setRandomDecel(randomDecel);
	cfg->setIntelligence(intelligence);
	cfg->setFuzzyMembers(fuzzyMembers);
	cfg->setKBOutput(kbOutput);
	cfg->setFixedDesire(fixedDesire);
	cfg->setFixedFear(fixedFear);
	cfg->setTraceOutput(traceOutput);
	cfg->setMaxCars(maxCars);
	cfg->setMaxCreatures(maxCreatures);
	cfg->setHorizontalHop(horizontal_hop);
	cfg->setHorizontalMax(horizontal_max);
	cfg->setPropRight(propright);

	return SIM_SUCCESS;
}

// Reads the prior create knowledge from the input file
// returns SIM_ERROR on error or SIM_SUCCESS on success
int readPriorKnowledge(string fileName)
{
	ifstream infile(fileName, ifstream::in);
	string line, p1, p2, p3, p4, p5, p6;

	if(infile.is_open())
	{
		getline(infile, line);

		//parse each line until end of file
		while(!infile.eof())
		{
			istringstream istr(string(line), ios_base::out);
			istr >> p1 >> p2 >> p3 >> p4 >> p5 >> p6;

			int crossPoint = atoi(p1.c_str());
			int lane = atoi(p2.c_str());
			int proximity = atoi(p3.c_str());
			int speed = atoi(p4.c_str());
			int crossed = atoi(p5.c_str());
			int hit = atoi(p6.c_str());

			if(DEBUG)
				cout << "  " << crossPoint << "    " << lane << "     " << proximity << "      " << speed << "      " << crossed << "      " << hit << endl;
			Result * r = new Result(crossPoint, lane, proximity, speed, crossed, hit);
			sim->addResult(r);

			getline(infile, line);
		}
		infile.close();
	}
	else
	{
		if(DEBUG)
			cout << "Unable to open file: '" << fileName << "'" << endl;
		return SIM_ERROR;
	}
	return SIM_SUCCESS;
}