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
    u8 str[5];
    str[0] = ' ';


    str[4] = 0;

    display_chars(LCD_SEG_L1_3_0, str, SEG_ON);
}

#endif /* CONFIG_DISCORDIAN */

