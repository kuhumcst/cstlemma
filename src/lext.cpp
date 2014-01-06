/*
CSTLEMMA - trainable lemmatiser

Copyright (C) 2002, 2014  Center for Sprogteknologi, University of Copenhagen

This file is part of CSTLEMMA.

CSTLEMMA is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

CSTLEMMA is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with CSTLEMMA; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "lext.h"
#if defined PROGLEMMATISE
#include "caseconv.h"
#include "utf8func.h"
#include <string.h>

#ifdef COUNTOBJECTS
int lext::COUNT = 0;
#endif


const char * lext::constructBaseform(const char * fullform) const
    {
    static char buf[256];
    size_t off = S.Offset;
    const char * w;
    char * pbuf = buf;
    if(off > 0)
        {
        strcpy(buf,changeCase(fullform,true,off));
        pbuf = buf + strlen(buf);
        }
    for(w = BaseFormSuffix;*w;)
        {
        *pbuf++ = *w++;
        }
    *pbuf = '\0';
    return buf;
    }

#endif