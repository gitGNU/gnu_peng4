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


struct pengset
{
    unsigned            blksize;
    unsigned short     *perm1;
    unsigned short     *perm2;
    unsigned char      *mask1;
#if USE_MODE_XPX
    unsigned char      *mask2;
#endif
};


struct pengpipe
{
    unsigned            blksize;
    unsigned            rounds;       /* y dimension */
    unsigned            variations;   /* x dimension */
    struct pengset   ***mtx;          /* 2d matrix */
#if USE_MODE_CBC
    unsigned char      *iv;
#endif
};



struct pengset *genpengset(unsigned blksize, struct mersennetwister *mt);
struct pengpipe *genpengpipe(unsigned blksize, unsigned rounds, unsigned variations, struct mersennetwister *mt);
void destroypengset(struct pengset *p);
void destroypengpipe(struct pengpipe *p);
void execpengset(struct pengset *p, const unsigned char *buf1, unsigned char *tmpbuf, unsigned char *buf2, char encrypt);
void execpengpipe(struct pengpipe *p, unsigned char *buf1, unsigned char *tmpbuf, unsigned char *buf2, char encrypt, char threads_flag);
unsigned long getbufsize(struct pengpipe *p);
