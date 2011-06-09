/*
    Tally counter/clicker for OpenChronos on the TI ez430 chronos watch.
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

#ifdef CONFIG_TALLY

// driver
#include "altitude.h"
#include "display.h"
#include "vti_ps.h"
#include "ports.h"
#include "timer.h"
#include "rfsimpliciti.h"

#include "stopwatch.h"

// logic
#include "user.h"
#include "clock.h"
#include "date.h"
#include "tally.h"

#include "menu.h"

struct tallydata stallydata;

void update_tallystring(void);

void tally_tick(void)
{
    u32 tmp = 0;

    // Update the on-screen count.
    stallydata.count++;
    update_tallystring();

	// Force full display update
	display.flag.full_update = 1;
}

void update_tallystring(void)
{
    u8 i;
    u16 c = stallydata.count;
    for (i = 0; i < 5; i++)
    {
        stallydata.countchars[4 - i] = (u8)('0' + (c % 10));
        c = c / 10;
    }
    stallydata.countchars[5] = 0;
}

void sx_tally(u8 line)
{
}

void mx_tally(u8 line)
{
    reset_tally();
}

void display_tally(u8 line, u8 update)
{
    display_chars(LCD_SEG_L2_4_0, stallydata.countchars, SEG_ON);
}

void reset_tally(void)
{
    // Reset display to all zeros.
    stallydata.count = 0;
    update_tallystring();

	// Force full display update
	display.flag.full_update = 1;
}

void init_tally(void)
{
    reset_tally();
}

#endif /* CONFIG_TALLY */

