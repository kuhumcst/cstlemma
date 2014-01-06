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
#ifndef TAGS_H
#define TAGS_H

#include "defines.h"
#if defined PROGLEMMATISE
#include <stdio.h>

class tagpairs
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        char ** textTags; //List of tags that may occur in the text, but are not in the dictionary
        char ** dictTags; //List of corresponding tags as found in the dictionary
        // tags that occur in the text as well as in the dictionary are not listed.
        int tagcnt;
        char * X;
    public:
        tagpairs(FILE * fpx,bool nice);
        ~tagpairs();
        const char * translate(const char * Tp);
        int Closeness(const char * tag,const char * t);
    };


extern const char NOT_KNOWN[];
#endif
#endif