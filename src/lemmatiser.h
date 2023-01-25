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
#include "defines.h"
#if (defined PROGLEMMATISE) || (defined PROGPRINTDICT)
#include "dictionary.h"
#endif


#if STREAM
# include <iostream>
# ifndef __BORLANDC__
using namespace std;
# endif
#endif

#if defined PROGLEMMATISE
class tagpairs;
#endif

struct optionStruct;
struct tallyStruct;

class Lemmatiser
    {
    private:
        Lemmatiser& operator= (const Lemmatiser& f)
            {
            REFER(f) // unused
            return *this;
            }
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        static int instance;
#if (defined PROGLEMMATISE) || (defined PROGPRINTDICT)
        dictionary dict;
#if (defined PROGLEMMATISE)
        static tagpairs * TextToDictTags;
        int listLemmas;
        bool SortInput; // derived from other options
        bool changed;
#endif
#endif
        optionStruct & Option;
        int status;
    public:
        int getStatus()
            {
            return status;
            }
        Lemmatiser(optionStruct & Option);
        ~Lemmatiser();
#if defined PROGLEMMATISE || defined PROGPRINTDICT
        int openFiles();
        int LemmatiseInit();
        int LemmatiseFile();
#if defined PROGLEMMATISE
        void LemmatiseEnd();
        static const char * translate(const char * tag);
        int setFormats();
        void showSwitches();
#if STREAM
        void LemmatiseText(istream * fpin,ostream * fpout,tallyStruct * tally);
#else
        void LemmatiseText(FILE * fpin,FILE * fpout,tallyStruct * tally);
#endif
#endif
#endif
#if defined PROGMAKEDICT
        int MakeDict();
#endif
#if defined PROGMAKESUFFIXFLEX
        int MakeFlexPatterns();
#endif
    };

