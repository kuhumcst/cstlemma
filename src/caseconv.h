/*
CSTLEMMA - trainable lemmatiser

Copyright (C) 2002, 2014, 2009  Center for Sprogteknologi, University of Copenhagen

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
#ifndef CASECONV_H
#define CASECONV_H

#include <stddef.h>


#define DEFAULTENCODING 0
#define ISO8859_1 1 //Western European
#define ISO8859_2 2 //Eastern European
#define ISO8859_7 7 //Greek
#define ISO8859_9 9 //Turkish
#define ENUNICODE 'U' //Unicode


                // If you don't want case conversion and
                // assumptions about what are characters in the 
                // range >127, then set to DEFAULTENCODING.
void setEncoding(int encoding);
/*
const char * allToLowerISO(const char * s);
void AllToLowerISO(char * s);
void AllToUpperISO(char * s);
bool isAlphaISO(int s); 
void toUpper(char * s);
bool isAllUpper(const char * s);
extern const bool * alpha;
*/
//void toLower(char * s);
extern const bool * space;
#define isSpace(s) space[(int)(s) & 0xFF]

#if 0
#if ENCODING == DEFAULTENCODING

#define isUpper(s) 1
#define Upper(k) k
#define Lower(k) k

#else

extern const unsigned char * upperEquivalent;
extern const unsigned char * lowerEquivalent;

#define isUpper(s) (upperEquivalent[(int)(*s & 0xFF)] == (int)(*s & 0xFF))
#define Upper(k) upperEquivalent[(int)(k & 0xFF)]
#define Lower(k) lowerEquivalent[(int)(k & 0xFF)]

#endif
#endif

extern bool (*is_Upper)(const char * s);
//extern unsigned int (*Upper)(int k);
//extern unsigned int (*Lower)(int k);
//extern void (*NToLower)(char * s,const char * stop); // 20100303, partly replaces Lower
extern int (*strcasecmpN)(const char *s, const char *p,ptrdiff_t & is,ptrdiff_t & ip); // 20100303, partly replaces Lower
extern int (*strcmpN)(const char *s, const char *p,ptrdiff_t & is,ptrdiff_t & ip); // 20100303, increments to UTF-8 character boundaries
extern bool (*is_Alpha)(int s);
extern const char * (*allToLower)(const char * s);
//extern void (*AllToLower)(char * s);
extern bool (*IsAllUpper)(const char * s);
//extern void (*allToUpper)(char * s);
#endif
