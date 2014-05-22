/*
 * MainMenu.cpp
 *
 * Created: 4/30/2014 9:50:19 AM
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

#include "MessageHandler.h"
#include "MainMenu.h"
#include "SetupHandler.h"
#include "RunFocusStackHandler.h"

#include "NikType003.h"
#include "LcdImpl.h"
#include "Sketch.h"
extern RunFocusStackHandler *g_pRunStack;
extern SetupHandler *g_pSetup;
extern NikType003 nk3;
// Main menu ctor. Sets up the text strings
// to display.
MainMenuHandler::MainMenuHandler(MessagePump *_pump)
    :
    IMessageHandler(_pump)
{
    menu[0] = "Nikon";
    menu[1] = " Setup stack";
    menu[2] = " Run stack";
    menu[3] = " Off";
};
MainMenuHandler::~MainMenuHandler()
{}

// Listens for button press Msgs dispatched from loop().
MsgResp MainMenuHandler::processMessage(Msg& msg)
{
    MsgResp rsp = eFail;
    if(eButtonActionPress == msg.m_type)
    {
        // up and down buttons move the caret.
        // select button invokes Setup or runStack
        // handling.
        switch(msg.m_code)
        {
            case eLeft:     { break; }
            case eRight:    { break; }
            case eDown:     { moveCaret(eMoveDown); break; }
            case eUp:       { moveCaret(eMoveUp); break; }
            case eSelect:
            {
                switch(m_caretRow)
                {
                    case 1:
                    {
                        msg.m_nextHandler = g_pSetup;
                        rsp = eSuccess;
                        break;
                    }
                    case 2:
                    {
                        if(nk3.isConnected())
                        {
                            msg.m_nextHandler = g_pRunStack;
                            rsp = eSuccess;
                        }
                        break;
                    }
                    case 3:
                    {
                        // on/off?
                        // TODO: turn display off/on
                        // how far to sleep can we go
                        // and still wake?
                        break;
                    }
                    default: { break; }
                }
                break;
            }
            default : { break; }
        }
    }
    return rsp;
}
// Show the main menu, the camera type & connection state.
extern bool g_usbOK;
void MainMenuHandler::show()
{
	if(!g_usbOK) return;
    IMessageHandler::show();
    g_print->setCursor(0, 0);
    switch(nk3.getProductID())
    {
        case 0x0428:
        {
             g_print->print(F("D7k "));
             break;
        }
        case 0x0429:
        {
             g_print->print(F("D5100 "));
             break;
        }
        case 0x0421:
        {
            g_print->print(F("D90 "));
            break;
        }
        default:
        {
            g_print->print(F("Nikon "));
            break;
        }
    }
    g_print->print(nk3.isConnected() ? F("connected") : F("disconnected"));
}





