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
#ifndef OUTPUTCLASS_H
#define OUTPUTCLASS_H

#include "defines.h"
#if defined PROGLEMMATISE

#define NUMBERTEST 1
#define STRINGTEST 2

#if defined PROGLEMMATISE
class function;
class functionTree;

typedef function * (*getFunction)(int kar,bool & SortInput,int & testType);
#endif

class OutputClass
    {
#if defined PROGLEMMATISE
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
        OutputClass()
            {
            ++COUNT;
            }
#endif
    public:
        virtual ~OutputClass()
            {
#ifdef COUNTOBJECTS
            --COUNT;
#endif
            } // it is important to have at least one virtual 
        //function. Otherwise OutputClass isn't polymorphic and pointers are not 
        //correctly downcasted.
        static const char * Format(const char * format,getFunction gfnc,functionTree & tree,const char * allFormat,bool & SortInput,int & testType);
        virtual bool skip() const{return false;}
#endif
    };

#endif
#endif