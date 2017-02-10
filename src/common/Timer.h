/*
 * Timer.h
 *
 *  Created on: Oct 26, 2013
 *      Author: Moy
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <float.h>
#include <math.h>
#include <iostream>
#include<time.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include "utils.h"

namespace osu {

class Timer {
public:
	std::string name;
	double max, min, sq_time;
	double total_time;
	int count;
	time_t start_time;

	Timer(std::string _name): name(_name), max(0), min(DBL_MAX), sq_time(0), total_time(0), count(0) {}
	Timer(std::string _name, double _max, double _min, double _sq_time, double _total_time, int _count): \
			name(_name), max(_max), min(_min), sq_time(_sq_time), total_time(_total_time), count(_count) {}
	virtual ~Timer() {}

	void start() {
		time(&start_time);
	}
	void end() {
		time_t end_time;
		time(&end_time);
		double curr_time;
		curr_time = difftime(end_time, start_time);
		total_time += curr_time;
		sq_time += curr_time*curr_time;
		if (curr_time < min) min = curr_time;
		if (curr_time > max) max = curr_time;
		count++;
	}
	void mergeTimer(osu::Timer* timer) {
		max = std::max(max, timer->max);
		min = std::max(min, timer->min);
		sq_time += timer->sq_time;
		total_time += timer->total_time;
		count += timer->count;
	}
	double avg() {
		return (count == 0) ? 0 : total_time / count;
	}
	double var() {
		if (count < 2) return 0;
		double a = avg();
		return (sq_time / count - a*a)*count/(count-1);
	}
	void save(std::ofstream& file);
	void load(std::ifstream& file);
	friend std::ostream& operator<<(std::ostream& out, osu::Timer& timer) {
		if (timer.count > 1)
			return out << timer.name << ": " << timer.avg() << "sec.(" << sqrt(timer.var()) << \
					") [" << timer.min << "," << timer.max << "], calls: " << timer.count;
		else
			return out << timer.name << ": " << timer.avg() << "sec., calls: " << timer.count;
	}
};

class TimerCollection {
public:
	std::map<std::string, osu::Timer*>* timers;
	TimerCollection(): timers(0) {}
	osu::Timer* getTimer(std::string name) {
		return getTimer(name, true);
	}
	osu::Timer* getTimer(std::string& name, bool addnew) {
		if (addnew) init();
		if (!timers) return 0;
		std::map<std::string, osu::Timer*>::iterator it = timers->find(name);
		osu::Timer* timer = 0;
		if (it == timers->end()) {
			if (addnew) {
				timer = new osu::Timer(name);
				timers->insert(std::pair<std::string, osu::Timer*>(name, timer));
			} else return 0;
		} else {
			timer = it->second;
		}
		return timer;
	}
	void mergeTimerCollection(osu::TimerCollection* coll) {
		if (!coll) return;
		init();
		std::map<std::string, osu::Timer*>::iterator it = coll->timers->begin();
		for (; it != coll->timers->end(); it++) {
			osu::Timer* curr = it->second;
			mergeTimer(curr);
		}
	}
	osu::Timer* mergeTimer(osu::Timer* timer) {
		if (!timer) return 0;
		init();
		osu::Timer* curr = 0;
		std::map<std::string, osu::Timer*>::iterator it = timers->find(timer->name);
		if (it == timers->end()) {
			curr = new osu::Timer(timer->name, timer->max, timer->min, \
					timer->sq_time, timer->total_time, timer->count);
			timers->insert(std::pair<std::string, osu::Timer*>(curr->name, curr));
		} else {
			curr = it->second;
			curr->mergeTimer(timer);
		}
		return curr;
	}
	friend std::ostream& operator<<(std::ostream& out, osu::TimerCollection& timers) {
		if (!timers.timers) return out << "no timers in collection";
		std::map<std::string, osu::Timer*>::iterator it = timers.timers->begin();
		for (; it != timers.timers->end(); it++) {
			if (it->second) {
				out << *it->second << std::endl;
			} else {
				out << it->first << ": no timer" << std::endl;
			}
		}
		return out;
	}
	void save(std::ofstream& file);
	void load(std::ifstream& file);
	virtual ~TimerCollection() {
		if(timers)
			osu::releaseMapElements< std::string, osu::Timer* >(timers);
		timers = 0;
	}
protected:
	void init() {
		if (!timers) timers = new std::map<std::string, osu::Timer*>();
	}
};

osu::Timer* startTimer(osu::TimerCollection* timers, std::string name);

inline void endTimer(osu::Timer* timer) {
	if (timer) timer->end();
}

}

#endif /* TIMER_H_ */
