#include "StdAfx.h"
#include "Result.h"

//todo allow for comparing two results with ==, < etc.

Result::Result(int crossPoint, int lane, int proximity, int speed, int crossed, int hit)
{
	this->crossPoint = crossPoint;
	this->lane = lane;
	this->proximity = proximity;
	this->speed = speed;
	this->crossed = crossed;
	this->crossed_total = 0;			//S (used for var & mean)
	this->crossed_squared_total = 0;	//SS (used for var & mean)
	this->hit = hit;
	this->hit_total = 0;				//S
	this->hit_squared_total = 0;		//SS
	this->total = 0;					//N		(total # of creatures who have used this configuration)
	this->time_killed = 0;
	this->time_success = 0;
	this->waiting = false;
}

void Result::setWaiting(bool state)
{
	waiting = state;
}

bool Result::getWaiting(void)
{
	return waiting;
}

int Result::getHit(void)
{
	return hit;
}

int Result::getCrossed(void)
{
	return crossed;
}

int Result::getCrossPoint(void)
{
	return crossPoint;
}

int Result::getLane(void)
{
	return lane;
}

void Result::success(int wait_time)
{
	//cout << "    successssssssscreatures" << endl;
	crossed++;
	crossed_total+=crossed;
	crossed_squared_total+=(crossed*crossed);
	total++;
	time_success += wait_time;
}

void Result::failure(int wait_time)
{
	hit++;
	hit_total+=hit;
	hit_squared_total+=(hit*hit);
	total++;
	time_killed += wait_time;
}

int Result::getProximity(void)
{
	return proximity;
}

int Result::getSpeed(void)
{
	return speed;
}

void Result::setHit(int hit)
{
	this->hit = hit;
}

void Result::setCrossed(int crossed)
{
	this->crossed = crossed;
}

Result::~Result(void)
{
}

int Result::getTotal(void)
{
	return total;
}


int Result::getHitTotal(void)
{
	return hit_total;
}

int Result::getHitSquaredTotal(void)
{
	return hit_squared_total;
}

int Result::getCrossedTotal(void)
{
	return crossed_total;
}

int Result::getCrossedSquaredTotal(void)
{
	return crossed_squared_total;
}

int Result::getTimeKilled(void)
{
	return time_killed;
}

int Result::getTimeSuccess(void)
{
	return time_success;
}