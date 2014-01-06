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
#ifndef FREQFILE_H
#define FREQFILE_H

#include "defines.h"
#if defined PROGMAKEDICT

class FreqFile
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        char * name;
        char * format;
        FreqFile * next;
    public:
        FreqFile()
            {
            name = 0;
            format = 0;
            next = 0;
#ifdef COUNTOBJECTS
            ++COUNT;
#endif
            }
        ~FreqFile()
            {
            delete [] name;
            delete [] format;
            delete next;
#ifdef COUNTOBJECTS
            --COUNT;
#endif
            }
        const char * itsName() const
            {
            return name;
            }
        const char * itsFormat() const
            {
            return format;
            }
        const FreqFile * Next() const
            {
            return next;
            }
        void addName(char * name);
        void addFormat(char * format);
    };
  
#endif
#endif