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

#ifndef TALLY_H_
#define TALLY_H_

// menu functions
extern void sx_tally(u8 line);
extern void mx_tally(u8 line);
extern void display_tally(u8 line, u8 update);

extern void reset_tally(void);
extern void tally_tick(void);
extern void init_tally(void);

struct tallydata
{
    u16 count;
    u8 countchars[6];
};

extern struct tallydata stallydata;

#endif
