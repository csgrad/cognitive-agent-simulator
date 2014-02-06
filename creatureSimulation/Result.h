#pragma once

#include "Cell.h"
#include "Car.h"

#define CLEAR 0

#define DEAD -1
#define ALIVE 1

class Result
{
	int crossPoint;
	int lane;
	int proximity;
	int speed;
	int crossed;
	int crossed_total;
	int crossed_squared_total;
	int hit;
	int hit_total;
	int hit_squared_total;
	int total;
	int time_success;
	int time_killed;
	bool waiting;
public:
	Result(int crossPoint, int lane, int proximity, int speed, int crossed, int hit);
	int getCrossPoint(void);
	int getLane(void);
	int getHit(void);
	int getCrossed(void);
	void success(int wait_time);
	void failure(int wait_time);
	int getProximity(void);
	int getSpeed(void);
	void setHit(int hit);
	int getTotal(void);
	int getHitTotal(void);
	int getHitSquaredTotal(void);
	int getCrossedTotal(void);
	int getCrossedSquaredTotal(void);
	int getTimeSuccess(void);
	int getTimeKilled(void);
	void setWaiting(bool state);
	bool getWaiting(void);
	void setCrossed(int crossed);
	~Result(void);
};

