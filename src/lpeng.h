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


extern int verbosity;
extern int debugmask;


struct peng_cmd_environment
{
    struct pengpipe         *pp;
    struct mersennetwister   mt;
    uint8_t                 *buf1, *buf2, *buf3;
    uint32_t                 blksize, bufsize;
    int                      eflag;
};


void peng_unit_prep(void); /* if you call peng_cmd_prep(), you don't need to call this one */

void peng_cmd_prep(struct peng_cmd_environment *pce, uint32_t blksize, uint32_t rounds, uint32_t variations, char *passphrase, int eflag);

int peng_cmd_process(struct peng_cmd_environment *pce, const char *infn, int inh, uint64_t total, const char *outfn, int outh, char multithreading, char min_locrr_seq_len);

void peng_cmd_unprep(struct peng_cmd_environment *pce);
