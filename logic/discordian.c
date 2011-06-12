/*
    Discordian date for OpenChronos on the TI ez430 chronos watch.
    Copyright 2011 Rick Miller <rdmiller3@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "project.h"

#ifdef CONFIG_DISCORDIAN

// driver
#include "display.h"

// logic
#include "user.h"
#include "clock.h"
#include "date.h"
#include "discordian.h"

#include "menu.h"

void sx_discordian(u8 line)
{
}

void mx_discordian(u8 line)
{
}

void display_discordian(u8 line, u8 update)
{
    u16 tmp = 0;
    u8 str[6];

    if ((sDate.day == 29) && (sDate.month == 2))
    {
        // St. Tib's Day
        str[0] = 'S';
        str[0] = 'T';
        str[0] = 'T';
        str[0] = 'I';
        str[0] = 'B';
    }
    else
    {
        // Calculate days from start of year.
        switch (sDate.month) // using lots of drop-through!
        {
            case 12:
                tmp += 30; // for nov
            case 11:
                tmp += 31; // for oct
            case 10:
                tmp += 30; // for sep
            case 9:
                tmp += 31; // for aug
            case 8:
                tmp += 31; // for jul
            case 7:
                tmp += 30; // for jun
            case 6:
                tmp += 31; // for may
            case 5:
                tmp += 30; // for apr
            case 4:
                tmp += 31; // for mar
            case 3:
                tmp += 28; // for feb
            case 2:
                tmp += 31; // for jan
            case 1:
            default:
                // do nothing
                break;
        }
        tmp += sDate.day;
        tmp--; // because day-of-month is 1-based (01-01 is the ZERO-th day).

        u8 season = (u8)((tmp / 73) + 1);
        u8 day = (u8)((tmp % 73) + 1);

        str[0] = ' ';
        str[1] = (u8)('0' + season);
        str[2] = ' ';
        str[4] = (u8)('0' + (day % 10));
        day /= 10;
        str[3] = (u8)('0' + (day % 10));
    }

    str[5] = 0;

    display_chars(LCD_SEG_L2_4_0, str, SEG_ON);

}

#endif /* CONFIG_DISCORDIAN */

