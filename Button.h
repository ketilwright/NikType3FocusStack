/*
 * Button.h
 *
 * Created: 4/30/2014 9:10:42 AM
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

#pragma once

#include <stdint.h>
#include "TimerCB.h"
#include "MessageHandler.h"
// Class Button
//  remarks:
//  1) Implements press, release and hold.
//  2) Currently only tested with interrupt driven state changes.
//  3) Depends on TimerCB and MessagePump classes.
class Button
{
    // HIGH or LOW, depending on HW implementation.
    const bool m_activeState;
    // the MCU pin this button is attached to
    const int m_mcuPin;
    // The code this button uses in Msgs
    const MsgCode m_code;
    // Inverted logic. LOW = pressed
    uint8_t m_logicState;
    // the millis() value at the time the button
    // was most recently pressed.
    unsigned long m_pressedTime;
    // Contains information about which button
    // was pressed, released or held.
    Msg m_msg;
    // helper for button hold implementation
    TimerCB m_holdTimer;
    // Static member function implementing the button hold
    // message behavior
    static void HoldTimerCallback(void *context);
public:
    // construct with a digital input pin, a uniquely
    // identifying button code, and logic active state.
    Button(int pin, MsgCode code, bool activeState);
    // Returns true if the button is pressed.
    bool isPressed() const;
    // Returns the milliseconds since a button
    // was most recently pressed.
    unsigned long getPressedTime() const;
    // Restarts the timer. Used to control
    // frequency of hold message generation
    void resetPressTime();
    // Returns information about the current
    // state of the button.
    Msg & getMsg();
    // This method should be called once each invocation of loop(),
    // if a button implements hold/repeat behavior.
    void updateHoldTimer(unsigned long curMillis);
    // Returns the digital input line the button is
    // connected to.
    int getMcuPin() const { return m_mcuPin; }
};
