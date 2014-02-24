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

#if defined PROGMAKEDICT
class FreqFile;
#endif

typedef enum {MAKEDICT,MAKEFLEXPATTERNS,LEMMATISE} whattodoTp;
typedef enum {GoOn = 0,Leave = 1,Error = 2} OptReturnTp;

#if defined _WIN32
#define commandlineQuote "\""
#else
#define commandlineQuote "\'"
#endif

extern char * dupl(const char * s);

struct optionStruct
    {
#ifdef COUNTOBJECTS
    public:
    static int COUNT;
#endif
#if defined PROGLEMMATISE
    static const char DefaultSep[]; // -s
    static const char DefaultCFormat[]; // -c
    static const char DefaultCFormat_NoTags[]; // -c
    static const char DefaultCFormat_NoDict[];
    static const char DefaultCFormat_NoTags_NoDict[];
    static const char DefaultCFormatXML[]; // -c in combination with -Xl 
    static const char DefaultCFormatXML_NoDict[];
    static const char Default_b_format[]; // -b
    static const char * Default_B_format; // -B
#endif
    // program task
    whattodoTp whattodo; // -D, -F, -L

    // -D: Make dictionary
#if defined PROGMAKEDICT
    bool CollapseHomographs; // -k makedict
    FreqFile * freq; // -n, -N makedict
#endif
    // -L
    // linguistic resources
#if defined PROGLEMMATISE
    const char * dictfile;  // -d
#endif
#if (defined PROGLEMMATISE) || (defined PROGMAKESUFFIXFLEX)
    const char * flx;       // -f
#endif
#if defined PROGLEMMATISE
    const char * v;         // -v
    const char * x;         // -x
    const char * z;         // -z
    // pre-run rule treatment
    bool RulesUnique; // -U removeAmbiguous

    // input text info
    bool InputHasTags;                      // -t text::text
    char * Iformat;                         // -I text::text
    int keepPunctuation;                    // -p text::text
#endif
    bool nice;                              // -y makedict, text::text, text::Lemmatise
#if defined PROGLEMMATISE
    unsigned long int size;                 // -m text::text
    bool treatSlashAsAlternativesSeparator; // -A text::text
    bool XML;                               // -X

    char * ancestor;            // -Xaxyz if not null, restrict lemmatisation to elements that are offspring of ancestor
    char * element;             // -Xexyz if null, analyse all PCDATA that is text
    char * wordAttribute;       // -Xwxyz if null, word is PCDATA
    char * POSAttribute;        // -Xpxyz if null, POS is PCDATA
    char * lemmaAttribute;      // -Xlxyz if null, Lemma is PCDATA
    char * lemmaClassAttribute; // -Xcxyz if null, lemma class is PCDATA
#endif
    // input and output
    const char * argi; // -i
    const char * argo; // -o
    const char * arge; // -e

    // output format
    char * cformat; // -c (also option for -D and -F tasks)
#if defined PROGLEMMATISE
    char * Wformat; // -W
    char * bformat; // -b
    char * Bformat; // -B

    // output switches
    char * Sep;                             // -s  text::Lemmatise
    unsigned int SortOutput;                // -q  text::Lemmatise
//    unsigned int SortFreq;                // -q#wp text::Lemmatise
    int UseLemmaFreqForDisambiguation;      // -H text::Lemmatise
    bool DictUnique;                        // -u text::Lemmatise
#endif
    bool baseformsAreLowercase;             // -l text::Lemmatise


#if defined PROGLEMMATISE
    bool defaultbformat;
    bool defaultBformat;
#endif
    bool defaultCformat;

#if defined PROGMAKESUFFIXFLEX
    long CutoffRefcount;
    bool showRefcount;
#endif
#if defined PROGLEMMATISE
    bool Defaultbformat()
        {
        return defaultbformat;
        }
    bool DefaultBformat()
        {
        return defaultBformat;
        }
    bool DefaultCformat()
        {
        return defaultCformat;
        }
#endif
    optionStruct();
    ~optionStruct();
    OptReturnTp doSwitch(int c,char * locoptarg,char * progname);
    OptReturnTp readOptsFromFile(char * locoptarg,char * progname);
    OptReturnTp readArgs(int argc, char * argv[]);
#if defined PROGLEMMATISE
    void setcformat(const char * format);            // -c
#endif
#if defined PROGLEMMATISE
    void setIformat(const char * format);            // -I
    void setBformat(const char * format);            // -B
    void setbformat(const char * format);            // -b
    void setWformat(const char * format);            // -W
    void setSep(const char * format);                // -s
    void setUseLemmaFreqForDisambiguation(int n);    // -H 0, 1 or 2
    void setkeepPunctuation(bool b);
    void setsize(unsigned long int n);
    void settreatSlashAsAlternativesSeparator(bool b);
    void setUseLemmaFreqForDisambiguation(bool b);
    void setDictUnique(bool b);
#endif
    void setbaseformsAreLowercase(bool b);
    };
