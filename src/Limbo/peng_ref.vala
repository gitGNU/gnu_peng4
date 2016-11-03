/*
    PENG - A Permutation Engine
    Copyright (C) 1998-2016 by Klaus-J. Wolf    kj !at! seismic !dot! de

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

class PengSet : GLib.Object
{
    public uint32            blksize;
    public uint16[]?         perm1;
    public uint16[]?         perm2;
    public uint8[]?          mask1;
 /* if USE_MODE_XPX */
    public uint8[]?          mask2;
 /* endif */
 
    public PengSet(uint32 blksize, MersenneTwister mt);
    ~PengSet()
    {
    }
    public void execpengset(uint8[] buf1, uint8[] tmpbuf, uint8[] buf2, bool encrypt)
    {
    }
}

class PengPipe : GLib.Object
{
    public uint32            blksize;
    public uint32            rounds;       /* y dimension */
    public uint32            variations;   /* x dimension */
    public PengSet[,]?       mtx;          /* 2d matrix */
 /* if USE_MODE_CBC */
    public uint8[]?          iv;
 /* endif */
 
    public PengPipe(uint32 blksize, uint32 rounds, uint32 variations, MersenneTwister mt)
    {
    }
    ~PengPipe()
    {
    }
    public void exec(uint8[] buf1, uint8[] tmpbuf, uint8[] buf2, bool encrypt, bool threads_flag)
    {
    }
    public uint32 getbufsize()
    {
    }
}
