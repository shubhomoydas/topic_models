/*
 * Timer.cpp
 *
 *  Created on: Jan 19, 2014
 *      Author: Moy
 */
#include "Timer.h"

namespace osu {

	osu::Timer* startTimer(osu::TimerCollection* timers, std::string name) {
		if (!timers) return 0;
		osu::Timer* timer = \
				timers->getTimer(name);
		timer->start();
		return timer;
	}

}
