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
#include "tags.h"
#if defined PROGLEMMATISE

#include "caseconv.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>


const char NOT_KNOWN[] = "NOT_KNOWN";

/*
e.g.
PRON PRON_DEMO PRON_UBST PRON_PERS PRON_POSS PRON_INTER_REL
V V_INF V_PRES V_PAST V_PARTC_PAST V_PARTC_PRES
"PRON" and "V" are tags as they occur in the dictionary, the other
tags (but also "PRON" and "V") can occur in the text. Before dictionary
look-up, e.g. "V_PAST" is converted to "V".
*/

tagpairs::tagpairs(FILE * fpx,bool nice):textTags(NULL),dictTags(NULL),tagcnt(0),X(NULL)
    {
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    fseek(fpx,0,SEEK_END);

    long cnt = ftell(fpx);
    if(nice)
        printf("cnt = %ld\n",cnt);
    rewind(fpx);
    X = new char[cnt+1];
    if(fread(X,cnt,1,fpx) == 1)
        {
        X[cnt] = '\0';// 20140224 new
        tagcnt = 0;
        char * nxt;
        char * xx;
        for(xx = X;(nxt = strchr(xx,'\n')) != NULL;xx = nxt + 1)
            {
            for(char * p = xx;p < nxt;)
                {
                if(isSpace(*p))
                    {
                    while(++p < nxt && isSpace(*p))
                        ;
                    if(p < nxt)
                        ++tagcnt;
                    }
                else
                    ++p;
                }
            }
        dictTags = new char*[tagcnt];
        textTags = new char*[tagcnt];
        tagcnt = 0;
        for(xx = X;(nxt = strchr(xx,'\n')) != NULL;xx = nxt + 1)
            {
            *nxt = '\0';
            for(char * p = xx;p < nxt;)
                {
                if(isSpace(*p))
                    {
                    *p = '\0';
                    while(++p < nxt && isSpace(*p))
                        ;
                    if(p < nxt)
                        {
                        textTags[tagcnt] = p;
                        dictTags[tagcnt] = xx;
                        ++tagcnt;
                        }
                    }
                else
                    ++p;
                }
            }
        if(nice)
            {
            printf("\n");
            for(int i = 0;i < tagcnt;++i)
                printf("%s %s\n",dictTags[i],textTags[i]);
            }
        }
    }


tagpairs::~tagpairs()
    {
    delete [] dictTags;
    delete [] textTags;
    delete [] X;
#ifdef COUNTOBJECTS
    --COUNT;
#endif
    }

const char * tagpairs::translate(const char * Tp)
    {
    static int i = 0;
    if(tagcnt && !strcmp(Tp,textTags[i]))
        return dictTags[i]; // optimisation. (Often the same tag must be translated many times in sequence).
    for(i = 0;i < tagcnt;++i)
        {
        if(!strcmp(Tp,textTags[i]))
            {
            return dictTags[i]; // translate tag to dictionary-type
            }
        }
    i = 0;
    return Tp;
    }

int tagpairs::Closeness(const char * tag,const char * t)
    {
    for(int i = 0;i < tagcnt;++i)
        {
        if(!strcmp(tag,dictTags[i]))
            {
            int dist = 0;
            do 
                {
                if(!strcmp(textTags[i],t))
                    {
                    return dist;
                    }
                ++dist;
                }
            while(++i < tagcnt && !strcmp(tag,dictTags[i]));
            return -1;
            }
        }
    return -1;
    }
#endif