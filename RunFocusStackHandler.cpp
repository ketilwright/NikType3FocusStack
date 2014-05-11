/*
 * RunFocusStackHandler.cpp
 *
 * Created: 4/30/2014 10:57:24 AM
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

#include "RunFocusStackHandler.h"
#include "NikType003.h"
#include "LcdImpl.h"
extern IMessageHandler *g_pMain;
extern NikType003  nk3;

RunFocusStackHandler::RunFocusStackHandler(MessagePump *_pump)
    :
    IMessageHandler(_pump)
{
    menu[0] = "Run stack";
}

RunFocusStackHandler::~RunFocusStackHandler()
{
}

MsgResp RunFocusStackHandler::processMessage(Msg& msg)
{
    MsgResp rsp = eFail;
    if(eButtonActionPress == msg.m_type)
    {
        switch(msg.m_code)
        {
            case eDown:
            {
                nk3.cancelFocusStack();
                msg.m_nextHandler = g_pMain;
                rsp = eSuccess;
                break;
            }
            default:
            {
                break;
            }
        }
    }
    return rsp;
}
void RunFocusStackHandler::show()
{
    IMessageHandler::show();
    g_print->setCursor(0, 1);
    g_print->print(F("Running focus stack"));
    g_print->setCursor(0, 2);
    g_print->print(F("Press down to cancel"));
    nk3.startFocusStack();
}