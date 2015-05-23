/*
    PENG - A Permutation Engine
    Copyright (C) 1998-2015 by Klaus-J. Wolf
                               yanestra !at! lab6 !dot! seismic !dot! de

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


#define MALLOC(x) chkmalloc(x)
#define MALLOCA(x) alloca(x)
#define FREE(x) free(x)
#define FREEA(x)



void *chkmalloc(unsigned x);

int mymemcmp(const void *abuf0, const void *bbuf0, unsigned sz0);

void memxor(void *dst0, const void *src0, unsigned sz0);

unsigned do_padding(void *buf0, unsigned sz0, const unsigned long *marker, unsigned nmarker, unsigned marker_byteoffset);

int locrr(void *buf, unsigned sz, const unsigned long *marker, unsigned nmarker, int minmatch);

unsigned countconsecutivezeros(void *buf0, unsigned sz);

unsigned long byte_reorder(const char *from_order, const char *to_order, unsigned long from, int bytes);

void rectify(const char *from_order, const char *to_order, void *ptr, int numbytes);
