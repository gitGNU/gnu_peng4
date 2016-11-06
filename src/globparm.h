/*
    PENG - A Permutation Engine
    Copyright (C) 1998-2016 by Klaus-J. Wolf
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


/* These values are CANONICAL.
 * 
 * Don't change any of these defines - what you would produce then is a
 * non-working experimental version.
 * 
 * One of these changed, gives you BETA=0, ALPHA=1
 */

#if defined(DORKINESS) || defined(SKIP_XOR) ||  defined(SKIP_PERMUT) || defined(USE_MODE_XPX) || defined(USE_MODE_CBC) || defined(USE_UNENCRYPTED_HEADER)
#undef  ALPHA
#undef  BETA
#define ALPHA                       1
#define BETA                        0
#else
#define DORKINESS                   0
#define SKIP_XOR                    0
#define SKIP_PERMUT                 0
#define USE_MODE_XPX                1
#define USE_MODE_CBC                1
#define USE_UNENCRYPTED_HEADER      1
#endif
