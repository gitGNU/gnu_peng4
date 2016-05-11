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
    uint32_t            blksize;
    uint16_t           *perm1;
    uint16_t           *perm2;
    uint8_t            *mask1;
#if USE_MODE_XPX
    uint8_t            *mask2;
#endif
};


struct pengpipe
{
    uint32_t            blksize;
    uint32_t            rounds;       /* y dimension */
    uint32_t            variations;   /* x dimension */
    struct pengset   ***mtx;          /* 2d matrix */
#if USE_MODE_CBC
    uint8_t            *iv;
#endif
};



struct pengset *genpengset(uint32_t blksize, struct mersennetwister *mt);
struct pengpipe *genpengpipe(uint32_t blksize, uint32_t rounds, uint32_t variations, struct mersennetwister *mt);
void destroypengset(struct pengset *p);
void destroypengpipe(struct pengpipe *p);
void execpengset(struct pengset *p, const uint8_t *buf1, uint8_t *tmpbuf, uint8_t *buf2, char encrypt);
void execpengpipe(struct pengpipe *p, uint8_t *buf1, uint8_t *tmpbuf, uint8_t *buf2, char encrypt, char threads_flag);
uint32_t getbufsize(struct pengpipe *p);
