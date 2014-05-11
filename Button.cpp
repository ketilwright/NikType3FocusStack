/*
 * Button.cpp
 *
 * Created: 4/30/2014 9:11:02 AM
 *  Author: Ketil Wright
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

#include "Button.h"
#include "MessagePump.h"
#include "MessageHandler.h"
extern MessagePump g_pump;

Button::Button(int pin, MsgCode code, bool activeState)
    :
        m_activeState(activeState),         // HIGH or LOW, depending on HW impl
        m_mcuPin(pin),                      // arduino pin we're attached to
        m_code(code),                       // code which gets packed into m_msg
        m_logicState(!activeState),         // eg, assume the button is unpressed.
        m_pressedTime(0),                   // milliseconds the button has been held
        m_msg(eButtonActionNone, code),     // dispatched to the message pump
        m_holdTimer(this, HoldTimerCallback)// helper class for hold event generation
{
    pinMode(m_mcuPin, INPUT);
}

// Returns true if the button is pressed.
bool Button::isPressed() const
{
    return m_logicState == m_activeState;
}
// Returns the time in milliseconds since the button
// was most recently pressed.
unsigned long Button::getPressedTime() const
{
    return isPressed() ? (millis() - m_pressedTime) : 0;
}

// Restarts the timer. Used to control
// frequency of hold message generation
void Button::resetPressTime()
{
    if(isPressed())
    {
        m_pressedTime = millis();
    }
}

// Returns information about the current
// state of the button.
Msg& Button::getMsg()
{
    unsigned long curState = digitalRead(m_mcuPin);
    if(curState != m_logicState)
    {
        if(curState == m_activeState)
        {
            m_msg.m_type = eButtonActionPress;
            m_pressedTime = millis();
            // start a timer call back one time one thousand milliseconds from now.
            m_holdTimer.start(1000, 1);
        }
        else
        {
            m_msg.m_type = eButtonActionRelease;
            m_pressedTime = 0;
            m_holdTimer.cancel();
        }
    }
    m_logicState = curState;
    return m_msg;
}

// This method should be called once each invocation of loop(),
// if a button implements hold/repeat behavior.
void Button::updateHoldTimer(unsigned long curMillis)
{
    m_holdTimer.update(curMillis);
}

// Static member function implementing the button hold
// message behavior. Currently this just advances the
// message from press to hold short, and reschedules
// the timer to 200 milliseconds.
void Button::HoldTimerCallback(void *context)
{
    Button *_this = static_cast<Button*>(context);
    unsigned long newDelayTime = 0;
    switch(_this->m_msg.m_type)
    {
        case eButtonActionPress:
        case eButtonActionHoldShort:
        {
            _this->m_msg.m_type = eButtonActionHoldShort;
            newDelayTime = 200;
            break;
        }
        default: break;
    }

    g_pump.dispatch(_this->m_msg);
    // change timer to call back once after the new delay time
    if(0 != newDelayTime)
    {
        _this->m_holdTimer.start(newDelayTime, 1);
    }
}
