/*
 * SetupHandler.cpp
 *
 * Created: 4/29/2014 11:35:56 AM
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

#include "SetupHandler.h"
#include "LcdImpl.h"
#include <avr/eeprom.h>

extern uint16_t g_savedFocusAmount;
extern uint8_t g_savedNumFrames;

extern IMessageHandler *g_pMain;
SetupHandler::SetupHandler(MessagePump *_pump, uint32_t driveAmount, uint32_t frames)
    :
    IMessageHandler(_pump),
    m_driveAmount(driveAmount),
    m_numFrames(frames),
    m_changeFast(false)
{
    menu[0] = "Setup Focus Stack:";
    //           234567890123456789
    menu[1] =   "Focus amount";
    menu[2] =   "Frames";
};
SetupHandler::~SetupHandler()
{}

MsgResp SetupHandler::processMessage(Msg& msg)
{
    MsgResp rsp = eFail;
    // which button?
    switch(msg.m_code)
    {
        case eLeft:
        {
            switch(getCaretRow())
            {
                case 1: // drive amount
                {
                    int change = 0;
                    switch(msg.m_type)
                    {
                        case eButtonActionPress: change = -1; break;
                        case eButtonActionHoldShort:
                        {
                            if(m_changeFast) change = -100;
                            else change = -5;
                            break;
                        }
                        default: break;
                    }
                    if(change != 0) updateDriveAmountUI(change);
                    break;
                }
                case 2: // frame count
                {
                    int change = 0;
                    switch(msg.m_type)
                    {
                        case eButtonActionPress: change = -1; break;
                        case eButtonActionHoldShort:
                        {
                            if(m_changeFast) change = -10;
                            else change = -5;
                            break;
                        }
                        default: break;
                    }
                    if(change != 0) updateFramesUI(change);
                    break;
                }
            }
            break;
        }
        case eRight:
        {
            switch(getCaretRow())
            {
                case 1: // drive amount
                {
                    int change = 0;
                    switch(msg.m_type)
                    {
                        case eButtonActionPress: change = 1; break;
                        case eButtonActionHoldShort:
                        {
                            if(m_changeFast) change = 100;
                            else change = 5;
                            break;
                        }
                        default: break;
                    }
                    if(change != 0) updateDriveAmountUI(change);
                    break;
                }
                case 2: // frame count
                {
                    int change = 0;
                    switch(msg.m_type)
                    {
                        case eButtonActionPress: change = 1; break;
                        case eButtonActionHoldShort:
                        {
                            if(m_changeFast) change = 10;
                            else change = 5;
                            break;
                        }
                        default: break;
                    }
                    if(change != 0) updateFramesUI(change);
                    break;
                }
            }
            break;
        }
        case eUp:
        {
            if(eButtonActionPress == msg.m_type) moveCaret(eMoveUp);
            break;
        }
        case eDown:
        {
            if(eButtonActionPress == msg.m_type) moveCaret(eMoveDown);
            break;
        }
        case eSelect:
        {
            switch(m_caretRow)
            {
                case 3:
                {
                    if(eButtonActionPress == msg.m_type)
                    {
                        // write the current setting to the eprom
                        eeprom_write_word(&g_savedFocusAmount, m_driveAmount);
                        eeprom_write_byte(&g_savedNumFrames, m_numFrames);
                        msg.m_nextHandler = g_pMain;
                        rsp = eSuccess;
                    }
                    break;
                }
                default:
                {
                    m_changeFast = (eButtonActionRelease != msg.m_type);
                    rsp = eSuccess;
                    break;
                } // default case on caret row for eSelect button handling.
            }
            break;
        } // eSelect handler
        default: break;

    }
    return rsp;
}

void SetupHandler::show()
{
    IMessageHandler::show();
    updateDriveAmountUI(0); // 0: don't change, just show the current value
    updateFramesUI(0);      // 0: don't change, just show the current value
    setCaretRow(1);
    showCaret(true);
}
void SetupHandler::updateDriveAmountUI(int change)
{
    m_driveAmount += change;
    if(m_driveAmount < 1 ) m_driveAmount = 1;
    if(m_driveAmount > 9999) m_driveAmount = 9999;
    const unsigned char caretRow = getCaretRow();
    // clear to EOL.
    g_print->setCursor(16, 1);
    g_print->print(F("     "));
    g_print->setCursor(16, 1);
    g_print->print(m_driveAmount);
    setCaretRow(caretRow);

}
void SetupHandler::updateFramesUI(int change)
{
    m_numFrames += change;
    if(m_numFrames < 1 ) m_numFrames = 1;
    if(m_numFrames > 100) m_numFrames = 100;
    unsigned char caretRow = getCaretRow();
    // clear to EOL.
    g_print->setCursor( 8, 2);
    g_print->print(F("     "));
    g_print->setCursor( 8, 2);
    g_print->print(m_numFrames);
    setCaretRow(caretRow);
}
