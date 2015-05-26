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


#include <stdio.h>

#include "sysparm.h"


uint32_t bytebits(int c)
{
    register uint32_t n=0;
    
    if(c&1) n++;
    if(c&2) n++;
    if(c&4) n++;
    if(c&8) n++;
    if(c&0x10) n++;
    if(c&0x20) n++;
    if(c&0x40) n++;
    if(c&0x80) n++;
    return n;
}


int main(int argc, const char *argv[])
{
    uint64_t num;
    int i, c;
    FILE *f;

    for(i=1; i<argc; i++)
    {
        num = 0;
        f=fopen(argv[i], "r");
        if(f==NULL)
        {
            perror(argv[i]);
            continue;
        }
        for(;;)
        {
            c = fgetc(f);
            if(c==EOF)
                break;
            num += bytebits(c);
        }
        fclose(f);
        printf("%s: %llu\n", argv[i], num);
    }
    return 0;
}
