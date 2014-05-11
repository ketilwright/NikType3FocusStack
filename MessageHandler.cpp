/*
 * MessageHandler.cpp
 *
 * Created: 5/2/2014 11:40:41 AM
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

#include "MessagePump.h"
#include "MessageHandler.h"
#include "LcdImpl.h"

// Base class ctor just initializes OK on bottom line of menu[].
IMessageHandler::IMessageHandler(MessagePump* _pump)
    :
    m_pump(_pump),
    m_caretRow(1)
{
    for(unsigned int line = 0; line < sizeof(menu) / sizeof(menu[0]); line++) menu[line] = NULL;
    menu[3] = "OK"; // other 3 slots used by subclasses.
}

// Draws the menu text, indenting all lines but
// the 1st by one column
void IMessageHandler::show()
{
    g_print->clear();
    int column = 0;
    for(unsigned int line = 0; line < sizeof(menu) / sizeof(menu[0]); line++)
    {
        if(menu[line])
        {
            g_print->setCursor(column, line);
            // subsequent columns indented
            column = 1;
            g_print->print(menu[line]);
        }
    }
    showCaret(true);
}

// Draws or hides the caret.
void IMessageHandler::showCaret(bool show)
{
    g_print->setCursor(0, m_caretRow);
    if(show)
    {
        g_print->print(F(">"));
    }
    else
    {
        g_print->print(F(" "));
    }
}

// Turns off the caret, moves it and redraws it
// up or down one location. Currently only supports
// the caret in the 1st column.
void IMessageHandler::moveCaret(eCaretMoveDir dir)
{
    switch(dir)
    {
        case eMoveDown:
        {
            showCaret(false);
            m_caretRow++;
            if(m_caretRow > 3) m_caretRow = 1;
            showCaret(true);
            break;
        }
        case eMoveUp:
        {
            showCaret(false);
            m_caretRow--;
            if(m_caretRow < 1) m_caretRow = 3;
            showCaret(true);
            break;
        }
        case eMoveLeft:
        case eMoveRight:
        default:
        {
            return; // not implemented
        }
    }
}
