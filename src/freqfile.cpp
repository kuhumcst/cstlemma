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
#include "freqfile.h"
#if defined PROGMAKEDICT

#include <string.h>

#ifdef COUNTOBJECTS
int FreqFile::COUNT = 0;
#endif


void FreqFile::addName(char * a_name)
    {
    if(this->name)
        {
        if(next)
            next->addName(a_name);
        else
            {
            next = new FreqFile();
            next->name = new char[strlen(a_name)+1];
            strcpy(next->name,a_name);
            }
        }
    else
        {
        this->name = new char[strlen(a_name)+1];
        strcpy(this->name,a_name);
        }
    }

void FreqFile::addFormat(char * a_format)
    {
    if(this->format)
        {
        if(next)
            next->addFormat(a_format);
        else
            {
            next = new FreqFile();
            next->format = new char[strlen(a_format)+1];
            strcpy(next->format,a_format);
            }
        }
    else
        {
        this->format = new char[strlen(a_format)+1];
        strcpy(this->format,a_format);
        }
    }
#endif