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
#include "lemmtags.h"
#if defined PROGLEMMATISE

#include "caseconv.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>


static char * lemmaX = NULL;
char ** fullTags = NULL; //List of tags that may occur in the text  (e.g. V_MED_PAST)
char ** lemmaTags = NULL; //List of tags of the corresponding lemmata (e.g. V_MED)
// tags that occur in the text as well as in the dictionary are not listed.
int fulltagcnt = 0;

const char * LemmaTag(const char * tag)
    {
    int f;
    for(f = 0;f < fulltagcnt;++f)
        {
        if(!strcmp(tag,fullTags[f]))
            break;
        }
    if(f < fulltagcnt)
        return lemmaTags[f];
    else
        return tag;
    }

static bool taglinecheck(const char * xx)
    {
#if 1 /* 20120122 */
    // Lines starting with # or ; or // are considered comment lines.
    // 20140224 /**/ comments are no longer supported. They should have been multi-line
    // Comments can be preceded by whitespace.
    size_t whitePrefix = strspn(xx," \t\v");
    switch(xx[whitePrefix])
        {
        case '#':
        case ';':
        case '\0':
        case '\n':
        case '\r':
            return false;
        case '/':
            switch(xx[whitePrefix+1])
                {
                case '/':
                case '*':
                    return false;
                default:
                    return true;
                }
        default:
            return true;
        }
#else
    size_t d; 
    size_t len = strlen(xx);
    return strspn(xx,"ABCDEFGHIJKLMNOPQRSTUVWXYZÆØÅ_\t ") == strlen(xx) // line consists only of capital letters and white space
        && (d = strcspn(xx,"\t ")) > 0 // line contains identifier
        && d < len // line contains space
        && strcspn(xx+d,"ABCDEFGHIJKLMNOPQRSTUVWXYZÆØÅ_") < (size_t)(len - d); // line contains identifier after space
#endif
    }

bool readLemmaTags(FILE * fpx,bool nice)
    {
    fseek(fpx,0,SEEK_END);

    long cnt = ftell(fpx);
    if(nice)
        printf("cnt = %ld\n",cnt);
    rewind(fpx);
    lemmaX = new char[cnt+1];// 20140224 +1
    size_t readbytes = fread(lemmaX,1,cnt,fpx);
    if(readbytes != (size_t)cnt)
        {
        fprintf(stderr
               ,"Problem reading lemma tag file. (Read %lu of %ld bytes)\n"
               ,(unsigned long)readbytes,cnt
               );
        return false;
        }
    lemmaX[cnt] = 0;// 20140224 new
    fulltagcnt = 0;
    char * nxt;
    char * xx;
    for(xx = lemmaX;(nxt = strchr(xx,'\n')) != NULL;xx = nxt + 1)
        {
        *nxt = '\0';
        if(taglinecheck(xx))
            ++fulltagcnt;
        *nxt = '\n';
        }
    lemmaTags = new char*[fulltagcnt];
    fullTags = new char*[fulltagcnt];
    fulltagcnt = 0;
    int line = 0;
    for(xx = lemmaX;(nxt = strchr(xx,'\n')) != NULL;xx = nxt + 1)
        {
        ++line;
        *nxt = '\0';
        if(taglinecheck(xx))
            {
            while(isSpace(*xx))
                ++xx;

            fullTags[fulltagcnt] = xx;

            while(*xx && !isSpace(*xx))
                ++xx;

            if(*xx)
                {
                *xx++ = '\0';
                while(isSpace(*xx))
                    ++xx;
                lemmaTags[fulltagcnt] = xx;
                while(*xx && !isSpace(*xx))
                    ++xx;
                *xx = '\0';
                }
            else
                {
                printf("error in tag translation table in line %d: [%s].\n",line,xx);
                }
            ++fulltagcnt;
            }
        }
    if(nice)
        {
        printf("\n");
        for(int i = 0;i < fulltagcnt;++i)
            printf("%s %s\n",fullTags[i],lemmaTags[i]);
        }
    return true;
    }
#endif