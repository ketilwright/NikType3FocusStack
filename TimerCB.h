/*
 * TimerCB.h
 *
 * Created: 4/30/2014 7:38:04 AM
 * Author: Ketil Wright
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

class TimerCB
{
    // Call signature of user callback function.
    typedef void (*timerCallback)(void*);
    // User supplied context to callback function.
    void *m_context;
    // User supplied callback function
    timerCallback m_callBack;
    // Time interval, in milliseconds between callbacks.
    unsigned long m_delay;
    // The number of times to invoke m_callback after m_delay milliseconds
    // when -1, m_callbacks is invoked infinitum.
    long m_count;
    unsigned long m_startTime;
public:
    // Construct with user context to be sent to timerCallback.
    // context can be NULL, timerCallback must be non NULL.
	TimerCB(void *context, timerCallback cb);
	~TimerCB();
    // Starts a timer with delay milliseconds interval, called count times.
    // Can also be used to change a timer interval or call count.
    void start(unsigned long delay, unsigned long count);
    // Stops further calls to m_callback
    void cancel();
    // Users should be called once for each button in the main loop.
    void update(unsigned long curMillis);
};


