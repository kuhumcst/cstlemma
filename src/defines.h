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
#ifndef DEFINES_H
#define DEFINES_H

#define STREAM 1 // 0: use stdio 1: use iostream

#if STREAM
//#define fprintf xxx
//void xxx(){}
#define LOG1LINE(x) cout << x << endl
#define LOGANDFLUSH(x) cout << x << flush
#else
#define LOG1LINE(x) printf("%s\n",x)
#define LOGANDFLUSH(x) printf("%s", x);fflush(stdout)
#endif

#define FREQ24 0 /* $n field in baseform. This is a meaningless field
                     (not used for disambiguation) */
#define PFRQ 0 /* frequency field in baseformpointer. This keeps the
        frequency as stored in the dictionary. Dictionary frequency is a
        relation n(full form, base form). The baseformpointer is the right
        place to store this value, but it is difficult to get at it:
        If you list lemmas per full form, then it should be printed together
        with the lemmas. If, on the other hand, you list full forms per
        lemma, then it should be printed with the full forms. In the
        current architecture, the baseformpointer connecting full form and
        base form is not part of the known context in the printIt function.
        */ 



//#define COUNTOBJECTS  // for finding memory leak. 
                       // Printout of counts in ~flex()

#define SORTWORD 1
#define SORTFREQ 2
#define SORTPOS  3
/*#define SORTFREQ1 (SORTFREQ << 2)
#define SORTWORD1 (SORTWORD << 2)
#define SORTPOS1  (SORTPOS  << 2)*/

#define SortFreq(i) (i & SORTFREQ)
/*#define SortWord(i) (i & SORTWORD)
#define SortPos(i)  (i & SORTPOS )
#define SortFreq1(i) (i & SORTFREQ1)
#define SortWord1(i) (i & SORTWORD1)
#define SortPos1(i)  (i & SORTPOS1 )*/

#if defined __BORLANDC__
#define CONSTSTRCHR
/* Borland C++ 5.02 defines
const char *strchr(const char *s, int c);
char *strchr( char *s, int c);

Standard C++ defines 
char *strchr( const char *string, int c );

The Borland prototype is more picky and forces other functions to return
non-const character strings.
*/
#define REFER(v) v;
#elif defined _MSC_VER
#define REFER(v) v;
#if _MSC_VER >= 1400
#define CONSTSTRCHR
#endif
#else
#if defined __GNUC__  && ((__GNUC__ == 4 && __GNUC_MINOR__ > 3) || __GNUC__ > 4)
#define CONSTSTRCHR
#endif
#define REFER(v)
#endif


#include <limits.h>

#if defined _WIN64
/*Microsoft*/
#define LONG long long
#define INT32 __int32
#define STRTOUL _strtoui64
#define STRTOL _strtoi64
#define FSEEK _fseeki64
#define FTELL _ftelli64
#else
#define LONG long
#if defined _WIN32
/*Microsoft*/
#define INT32 __int32
#else
#include <stdint.h>
#define INT32 int32_t
#endif
#define STRTOUL strtoul
#define STRTOL strtol
#define FSEEK fseek
#define FTELL ftell
#endif



#if (defined PROGMAKEDICT)
#elif (defined PROGMAKESUFFIXFLEX)
#elif (defined PROGLEMMATISE)
#else
#define PROGMAKEDICT
#define PROGMAKESUFFIXFLEX
#define PROGLEMMATISE
#endif
#if !defined PROGMAKEDICT
#if !defined PROGMAKESUFFIXFLEX
#if !defined PROGLEMMATISE
#error At least one of PROGMAKEDICT  PROGMAKESUFFIXFLEX and PROGLEMMATISE must be defined
#endif
#endif
#endif

#ifdef CONSTSTRCHR
#define CHAR char
#else
#define CHAR const char
#endif
#endif
