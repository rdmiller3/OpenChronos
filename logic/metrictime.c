/*
    MetricTime clock for OpenChronos on the TI ez430 chronos watch.
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

#ifdef CONFIG_METRICTIME

// driver
#include "altitude.h"
#include "display.h"
#include "vti_ps.h"
#include "ports.h"
#include "timer.h"

#include "stopwatch.h"

// logic
#include "user.h"
#include "clock.h"
#include "date.h"
#include "metrictime.h"

#include "menu.h"

void sx_metrictime(u8 line)
{
}

void mx_metrictime(u8 line)
{
}

u8 hexchar(u8 x)
{
    u8 retval = '-'; // default for out-of-domain input values

    if (x < 10)
    {
        retval = (u8)('0' + x);
    }
    else if (x < 16)
    {
        retval = (u8)('A' + (x - 10));
    }

    return retval;
}

void display_metrictime(u8 line, u8 update)
{
    u8 str[5];
    u8 shift = sTime.hour / 3; // workaround for 32-bit math ceiling
    u32 tmp =
        ((u32)(sTime.hour % 3) * 3600u) +
        ((u32)(sTime.minute) * 60u) +
        sTime.second;
    tmp = (tmp << 9) / 675u;
    tmp += (0x2000 * shift);
    str[0] = hexchar((u8)((tmp >> 12) & 0x000f));
    str[1] = hexchar((u8)((tmp >> 8) & 0x000f));
    str[2] = hexchar((u8)((tmp >> 4) & 0x000f));
    str[3] = hexchar((u8)(tmp & 0x000f));
    str[4] = 0;
    display_chars(LCD_SEG_L1_3_0, str, SEG_ON);
}

#endif /* CONFIG_METRICTIME */

