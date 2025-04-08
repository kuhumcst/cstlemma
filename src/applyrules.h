/*
CSTLEMMA - trainable lemmatiser

Copyright (C) 2002, 2008  Center for Sprogteknologi, University of Copenhagen

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
#ifndef APPLYRULES_H
#define APPLYRULES_H

#if (defined PROGLEMMATISE)
#include "defines.h"
#include <stdio.h>

#if LEMMATIZEV0
int newStyleRules();
#endif
bool readRules(FILE * flexrulefile,const char * flexFileName);
bool readRules(const char * flexFileName);
const char * applyRules(const char * word,bool SegmentInitial, bool RulesUnique);
const char * applyRules(const char * word,const char * tag,bool SegmentInitial, bool RulesUnique);
void deleteRules();
//extern bool oneAnswer;
#if LEMMATIZEV0
bool setNewStyleRules(int val);
#endif

#endif
#endif
