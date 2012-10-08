/*
CSTLEMMA - trainable lemmatiser

Copyright (C) 2002, 2005  Center for Sprogteknologi, University of Copenhagen

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
#if defined PROGLEMMATISE
#include "dictionary.h"
#endif


#if STREAM
# include <iostream>
# ifndef __BORLANDC__
using namespace std;
# endif
#endif



struct optionStruct;
#if defined PROGLEMMATISE
class tagpairs;
#endif
struct tallyStruct;
//class istream;
//class std::ostream;

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
        int listLemmas;
        bool SortInput; // derived from other options

        static int instance;
#if (defined PROGLEMMATISE)// || defined PROGMAKEDICT)
        dictionary dict;
#endif
        optionStruct & Option;
        int status;
#if defined PROGLEMMATISE
        static tagpairs * TextToDictTags;
#endif
        bool changed;
    public:
        // functions to change Option on the fly
        /*
        void setIformat(const char * format);            // -I
        void setBformat(const char * format);            // -B
        void setbformat(const char * format);            // -b
        void setcformat(const char * format);            // -c
        void setWformat(const char * format);            // -W
        void setSep(const char * a);
        void setUseLemmaFreqForDisambiguation(int n);    // -H 0, 1 or 2
        void setkeepPunctuation(bool b);
        void setsize(unsigned long int n);
        void settreatSlashAsAlternativesSeparator(bool b);
        void setUseLemmaFreqForDisambiguation(bool b);
        void setDictUnique(bool b);
        void setbaseformsAreLowercase(bool b);
        */
#if defined PROGLEMMATISE
        static const char * translate(const char * tag);
#endif
        int getStatus()
            {
            return status;
            }
        Lemmatiser(optionStruct & Option);
        ~Lemmatiser();
#if defined PROGLEMMATISE
        int setFormats();
        int openFiles();
        void showSwitches();
#endif
        int MakeDict();
#if defined PROGMAKESUFFIXFLEX
        int MakeFlexPatterns();
#endif
#if defined PROGLEMMATISE
#if STREAM
        void LemmatiseText(istream * fpin,ostream * fpout,tallyStruct * tally);
#else
        void LemmatiseText(FILE * fpin,FILE * fpout,tallyStruct * tally);
#endif
        int LemmatiseFile();
        int LemmatiseInit();
        void LemmatiseEnd();
#endif
    };

