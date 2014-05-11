/*
 * TimerCB.cpp
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

#include "TimerCB.h"
#include "Sketch.h"

TimerCB::TimerCB(void *context, timerCallback cb)
    :
    m_context(context),
    m_callBack(cb)
{
}


TimerCB::~TimerCB()
{
}

void TimerCB::start(unsigned long delay, unsigned long count)
{
    m_startTime = millis();
    m_delay = delay;
    m_count = count;
}
void TimerCB::cancel()
{
    m_startTime = 0;
    m_delay = 0;
    m_count = 0;
}
void TimerCB::update(unsigned long curMillis)
{
    if((curMillis - m_startTime) > m_delay)
    {
        switch(m_count)
        {
            case 0:
            {
                // completely elapsed. Do Nothing
                break;
            }
            case -1:
            {
                // infinitely recurring
                m_callBack(m_context);
                break;
            }
            default:
            {
                // m_callback may want to restart the timer, so
                // decrement count 1st.
                m_count--;
                m_callBack(m_context);
                break;
            }
        }
        m_startTime = curMillis;
    }
}