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
#include "simpliciti.h"
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

    // Calculate seconds since 2000-01-01 and save to ringlog.
    tmp = ((u32)sDate.year % 200) * 365;
    tmp += ((sDate.year % 200) / 4); // leap days
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
            if ((sDate.year % 4) == 0)
            {
                tmp++;
            }
        case 2:
            tmp += 31; // for jan
        case 1:
        default:
            // do nothing
            break;
    }
    tmp += sDate.day;
    tmp--; // because day-of-month is 1-based (2000-01-01 is the ZERO day).
    tmp *= 24;
    tmp += sTime.hour;
    tmp *= 60;
    tmp += sTime.minute;
    tmp *= 60;
    stallydata.ringlog[stallydata.ringpos] = tmp + sTime.second;
    stallydata.ringpos++;
    if (stallydata.ringpos >= TALLY_RINGLOG_MAX_ENTRIES)
    {
        stallydata.ringpos = 0;
        stallydata.ringrolled++;
    }
    if (stallydata.ringrolled)
    {
        display_symbol(LCD_ICON_RECORD, SEG_ON_BLINK_OFF);
        display_symbol(LCD_ICON_RECORD, SEG_ON);
    } else if (stallydata.ringpos > TALLY_RINGLOG_WARN_COUNT)
    {
        display_symbol(LCD_ICON_RECORD, SEG_ON_BLINK_ON);
    }
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
    reset_tally_count();
}

void display_tally(u8 line, u8 update)
{
    display_chars(LCD_SEG_L2_4_0, stallydata.countchars, SEG_ON);
}

void reset_tally_count(void)
{
    // Reset display to all zeros.  (Does NOT clear the log.)
    stallydata.count = 0;
    update_tallystring();

	// Force full display update
	display.flag.full_update = 1;
}

void init_tally(void)
{
    // Initialize the ringlog.
    clear_tally_log();
    reset_tally_count();
}

void clear_tally_log(void)
{
    int i;
    for (i = 0; i < TALLY_RINGLOG_MAX_ENTRIES; i++)
    {
        stallydata.ringlog[i] = 0;
    }
    stallydata.ringpos = 0;
    stallydata.ringrolled = 0;
    display_symbol(LCD_ICON_RECORD, SEG_OFF_BLINK_OFF);
}

void assemble_tally_data_payload(void)
{
    /*
     * put data into simpliciti_data[]
     *
     * simpliciti_data[0] is already set to SYNC_ED_TYPE_MEMORY.
     * simpliciti_data[1] is already set to the hi byte of burst addr.
     * simpliciti_data[2] is already set to the lo byte of burst addr.
     * The array is BM_SYNC_DATA_LENGTH (19) bytes long,
     *      so there are 16 data bytes (4 u32's) per burst.
     *
     * burst address zero is special:
     *  u32 max_entries = TALLY_RINGLOG_MAX_ENTRIES;
     *  u32 ringpos;
     *  u32 ringrolled;
     *  u32 unused;
     *
     * All the rest of the bursts hold tally timestamp data.
     *
     * All data is transferred in big-endian format.
     */

    u16 burst = ((u16)(simpliciti_data[1]) << 8) | simpliciti_data[2];

    if (burst == 0)
    {
        simpliciti_data[3] = 0;
        simpliciti_data[4] = 0;
        simpliciti_data[5] = (u8)((TALLY_RINGLOG_MAX_ENTRIES>>8)&0x00ff);
        simpliciti_data[6] = (u8)(TALLY_RINGLOG_MAX_ENTRIES&0x00ff);

        simpliciti_data[7] = 0;
        simpliciti_data[8] = 0;
        simpliciti_data[9] = (u8)((stallydata.ringpos>>8)&0x00ff);
        simpliciti_data[10] = (u8)(stallydata.ringpos&0x00ff);

        simpliciti_data[11] = 0;
        simpliciti_data[12] = 0;
        simpliciti_data[13] = 0;
        simpliciti_data[14] = stallydata.ringrolled;

        simpliciti_data[15] = 0;
        simpliciti_data[16] = 0;
        simpliciti_data[17] = 0;
        simpliciti_data[18] = 0;
    }
    else
    {
        int i;
        int first = (burst - 1) * 4; // assuming 4 u32's per burst
        // just in case...
        // assumes (TALLY_RINGLOG_MAX_ENTRIES % 4) == 0
        first = first % TALLY_RINGLOG_MAX_ENTRIES;
        for (i = 0; i < 4; i++)
        {
            simpliciti_data[(i*4)+3] = (u8)((stallydata.ringlog[first + i]>>24)&0x000000ff);
            simpliciti_data[(i*4)+4] = (u8)((stallydata.ringlog[first + i]>>16)&0x000000ff);
            simpliciti_data[(i*4)+5] = (u8)((stallydata.ringlog[first + i]>>8)&0x000000ff);
            simpliciti_data[(i*4)+6] = (u8)((stallydata.ringlog[first + i]>>0)&0x000000ff);
        }
    }
}

#endif /* CONFIG_TALLY */

