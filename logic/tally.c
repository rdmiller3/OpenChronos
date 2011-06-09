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

#include "rfsimpliciti.h"

#ifdef FEATURE_PROVIDE_ACCEL
#include "acceleration.h"
#endif

struct tallydata stallydata;

void update_tallystring(void);

u8 isRadio;

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
    // Toggle the isRadio boolean.
    isRadio = (isRadio ? 0 : 1);
}

void mx_tally(u8 line)
{
    reset_tally();
}

void display_tally(u8 line, u8 update)
{
    if (isRadio)
    {
        // TBD
        // start_tally_sync();
        display_chars(LCD_SEG_L2_4_0, "RADIO", SEG_ON);
    }
    else
    {
        display_chars(LCD_SEG_L2_4_0, stallydata.countchars, SEG_ON);
    }
}

void reset_tally(void)
{
    isRadio = 0;

    // Reset display to all zeros.  (Does NOT clear the log.)
    stallydata.count = 0;
    update_tallystring();

	// Force full display update
	display.flag.full_update = 1;
}

void init_tally(void)
{
    // Initialize the ringlog.
    // TBD: load from tally ring in infomem?
    stallydata.ringpos = 0;
    stallydata.ringrolled = 0;
    display_symbol(LCD_ICON_RECORD, SEG_OFF_BLINK_OFF);

    reset_tally();
}

void start_tally_sync(void)
{
    /*
        UNTESTED CODE - copied from logic/rfsimpliciti.c:start_simpliciti_sync
     */

  	// Clear LINE1
	//clear_line(LINE1);
	//fptr_lcd_function_line1(LINE1, DISPLAY_LINE_CLEAR);
	
	#ifdef FEATURE_PROVIDE_ACCEL
	// Stop acceleration sensor
	as_stop();
	#endif

	// Get updated altitude
#ifdef CONFIG_ALTITUTDE
	start_altitude_measurement();
	stop_altitude_measurement();	
#endif
		
	// Get updated temperature	
	temperature_measurement(FILTER_OFF);

	// Turn on beeper icon to show activity
	display_symbol(LCD_ICON_BEEPER1, SEG_ON_BLINK_ON);
	display_symbol(LCD_ICON_BEEPER2, SEG_ON_BLINK_ON);
	display_symbol(LCD_ICON_BEEPER3, SEG_ON_BLINK_ON);

	// Debounce button event
	Timer0_A4_Delay(CONV_MS_TO_TICKS(BUTTONS_DEBOUNCE_TIME_OUT));

	// Prepare radio for RF communication
	open_radio();

	// Set SimpliciTI mode
	sRFsmpl.mode = SIMPLICITI_TALLY;
	
	// Set SimpliciTI timeout to save battery power
	sRFsmpl.timeout = SIMPLICITI_TIMEOUT; 
		
	// Start SimpliciTI stack. Try to link to access point.
	// Exit with timeout or by a button DOWN press.
	if (simpliciti_link())
	{
		// Enter sync routine. This will send ready-to-receive packets
        // at regular intervals to the access point.
		// The access point replies with a command
        // (NOP if no other command is set)
		simpliciti_main_sync();
	}

	// Set SimpliciTI state to OFF
	sRFsmpl.mode = SIMPLICITI_OFF;

	// Powerdown radio
	close_radio();
	
	// Clear last button events
	Timer0_A4_Delay(CONV_MS_TO_TICKS(BUTTONS_DEBOUNCE_TIME_OUT));
	BUTTONS_IFG = 0x00;  
	button.all_flags = 0;
	
	// Clear icons
	display_symbol(LCD_ICON_BEEPER1, SEG_OFF_BLINK_OFF);
	display_symbol(LCD_ICON_BEEPER2, SEG_OFF_BLINK_OFF);
	display_symbol(LCD_ICON_BEEPER3, SEG_OFF_BLINK_OFF);
	
	// Force full display update
	display.flag.full_update = 1;	
}

#endif /* CONFIG_TALLY */

