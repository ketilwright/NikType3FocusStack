/*
 * Main.cpp
 *
 * Created: 4/30/2014 8:09:31 AM
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

// Usb & ptp includes
#include <Usb.h>
#include <usbhub.h>
#include <ptp.h>
#include <ptpdebug.h>
// LCD i2c includes
#include "LcdImpl.h"
// app includes
#include "NikType003.h"
#include "NikType003State.h"
#include "MessagePump.h"
#include "MessageHandler.h"
#include "MainMenu.h"
#include "SetupHandler.h"
#include "RunFocusStackHandler.h"
#include "Button.h"

//  LCD is currently using I2c. It should be possible and easy
//  to replace this with other liquid crystal libraries,
//  eg MaxLCD.
#define I2C_ADDR 0x27 // docs incorrectly indicate 0x3f
#define BACKLIGHT_PIN 3
#define En_pin 2
#define Rw_pin 1
#define Rs_pin 0
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7
LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);
// Declare a global pointer to the LCD for IMessageHandlers
// to print to. g_print is used throughout this program, so
// it should be possible to change the LCD implementation
// without requiring other changes.
// See also LcdImpl.h
LCD				*g_print = &lcd;

// The USB Host Shield uses some pins we can't stomp on:
// Digital I/O pins 0-7, like already mentioned analog pins are not used by the shield and provided only for convenience.
// Digital I/O pins 8-13. In this group, the shield in its default configuration uses pins 9 and 10 for INT and SS interface signals.
// However, standard-sized Arduino boards, such as Duemilanove and UNO have SPI signals routed to pins 11-13 in addition to ICSP connector,
// therefore shields using pins 11-13 combined with standard-sized Arduinos will interfere with SPI. INT and SS signals can be re-assigned
// to other pins (see below); SPI signals can not.
volatile int intVect0Bump = 1; // active low
void enableButtonInterrupts();
/*
	Interrupt
	Board			int.0	int.1	int.2	int.3	int.4	int.5
	Uno, Ethernet	2	3
	Mega2560		2	3	21	20	19	18
	Leonardo		3	2	0	1	7
	Due	has further capabilities. See http://arduino.cc/en/Reference/attachInterrupt

	From http://www.me.ucsb.edu/~me170c/Code/How_to_Enable_Interrupts_on_ANY_pin.pdf
	Since arduino uses its own chip assignments, there is a map we will use to find the arrays and pin
	assignments that the ATMEGA uses to control these pins. The best source for this information is the
	datasheet for the ATMEGA 328 (Linked on webpage). Upon examination, we find:

	Arduino Pin Pin PCINT # | PCIE # | PCMSK# | Main PCINT #
	D0-D7			16-23	| 2		 | 2	  |	2
	D8-D13			0-5		| 0		 | 0	  |	0
	A0-A5 (D14-D19) 8-13	| 1		 | 1	  |	1*/

/*****************************************
 * Suggested button HW configuration:
 *  1) eUp/eDown move cursor in menus
 *  2) eLeft/eRight change values of parameter at cursor
 *  3) eSelect returns from this menu, or executes a command
 *     associated with the cursor position.
 *  4) The code currently assumes the buttons have active LOW logic
 *
 *                eUp
 *
 *    eLeft     eSelect     eRight
 *
 *               eDown
 *
*/
Button buttonLeft(2, eLeft, LOW);
Button buttonRight(3, eRight, LOW);
Button buttonUp(4, eUp, LOW);
Button buttonDown(5, eDown, LOW);
Button buttonSelect(6, eSelect, LOW);
Button* buttons[5] = {&buttonLeft, &buttonRight, &buttonUp, &buttonDown, &buttonSelect};

// Called once to setup the LCD. Note that the rest of the code
// is currently dependent on a 20x4 character LCD.
void configureLCD()
{
    lcd.begin (20,4,LCD_5x8DOTS);
	lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
	lcd.setBacklight(HIGH);
}

// g_pump dispatches MSG from Button objects to the current IMessageHandler
MessagePump		g_pump(NULL);
// Default handler displayed on startup, and return from the other
// handlers.
MainMenuHandler	g_main(&g_pump);
// Handler to setup focus stack parameters
SetupHandler	g_setup(&g_pump, 250, 10);
// Handler for focus stack execution
RunFocusStackHandler g_runStack(&g_pump);
// Global pointers for other modules
MainMenuHandler         *g_pMain     = &g_main;
SetupHandler            *g_pSetup	 = &g_setup;
RunFocusStackHandler    *g_pRunStack = &g_runStack;
MessagePump             *g_pPump     = &g_pump;

// USB/Ptp objects
USB                 Usb;
//USBHub              Hub1(&Usb);
NikType003StateHandler nk3State;
NikType003  nk3(&Usb, &nk3State);

// Turns on interrupt capability for each button we use
void enableButtonInterrupts()
{
    PCICR |= ( 1 << PCIE2);					// enable interrupt vector2
    PCMSK2 |= (1 << PCINT18);				// unmask interrupt dig pin 2
    pinMode(buttonRight.getMcuPin(), INPUT);
    PCMSK2 |= (1 << PCINT19);				// unmask interrupt dig pin 3
    pinMode(buttonUp.getMcuPin(), INPUT);
    PCMSK2 |= (1 << PCINT20);				// unmask interrupt dig pin 4
    pinMode(buttonDown.getMcuPin(), INPUT);
    PCMSK2 |= (1 << PCINT21);				// unmask interrupt dig pin 5
    pinMode(buttonSelect.getMcuPin(), INPUT);
    PCMSK2 |= (1 << PCINT22);				// unmask interrupt dig pin 6
    MCUCR = ((1 << ISC01) | (1 << ISC00));	// int executes on change
}

// eeprom storage
#include <avr/eeprom.h>

uint16_t g_savedFocusAmount = 0;
uint8_t g_savedNumFrames = 2;

void setup()
{
    intVect0Bump = 1; // active low
	Serial.begin( 115200 );
    configureLCD();
    delay( 200 );
	if (Usb.Init() == -1) { Serial.println("OSC did not start."); }
    enableButtonInterrupts();
    uint16_t focAmt = eeprom_read_word(&g_savedFocusAmount);
    uint8_t nFrames = eeprom_read_byte(&g_savedNumFrames);
    if(0 != focAmt)
    {
        g_setup.setDriveAmount(focAmt);
    }
    if(0 != nFrames)
    {
        g_setup.setNumFrames(nFrames);
    }
    Serial.print("foc amt: "); Serial.print(focAmt);
    Serial.print("nFrames "); Serial.print(nFrames); Serial.println();



}

void loop()
{
    // Usb does its work.
	Usb.Task();
    if(0 == intVect0Bump)
    {
        // A button was pressed or released.
        // clear interrupt flag
        intVect0Bump = 1;
        // check the state of all buttons, and dispatch
        // press & release.
        for(size_t b = 0; b < sizeof(buttons) / sizeof(buttons[0]); b++)
        {
            Msg& msg = buttons[b]->getMsg();
            switch(msg.m_type)
            {
                case eButtonActionPress:
                case eButtonActionRelease:
                {
                    g_pump.dispatch(msg);
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    } // button interrupt handling
    else
    {
        // No button interrupt, but let's give the button hold
        // timers a chance to expire. This may implicitly generate
        // a hold message on any some buttons if their hold timer expires.
        unsigned long curMillis = millis();
        for(size_t b = 0; b < sizeof(buttons) / sizeof(buttons[0]); b++)
        {
            buttons[b]->updateHoldTimer(curMillis);
        }
    } // button processing.

}

// interrupt handler for the buttons. This just sets
// a flag that the loop function uses to go read the
// button activity.
ISR(PCINT2_vect)
{
	// tell the main loop that a button went low.
    intVect0Bump = 0;
}
