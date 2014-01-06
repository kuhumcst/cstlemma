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
#include "field.h"
#if defined PROGLEMMATISE

#include <string.h>
#include <ctype.h>
#include <assert.h>


#ifdef COUNTOBJECTS
int field::COUNT = 0;
#endif

field::field():next(NULL)
    {
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }
#ifndef CONSTSTRCHR
const 
#endif
char * field::getString(){return NULL;}

char * readValue::read(char * kar,field *& nextfield)
    {
    char * nxt = kar;
    if(next)
        nxt = next->read(kar,nextfield);
    if(nxt && *nxt)
        {
        size_t ln = strlen(nxt);
        if(pos + ln >= len)
            {
            len = pos + ln + 1;
            char * nw = new char[len];
            strcpy(nw,word);
            delete word;
            word = nw;
            }
        strcpy(word + pos,nxt);
        pos += ln;
        assert(strlen(word) < len);
        nextfield = this;
        }
    return NULL; // Value takes all!
    }

char * readWhiteSpace::read(char * kar,field *& nextfield)
    {
    int k = (unsigned char)*kar;
    if(!k)
        {
        found = true;
        if(next)
            {
            next->read(kar,nextfield);
            return NULL;
            }
        else
            {
            nextfield = NULL;
            return kar; 
            }
        }
    else if(isspace(k))
        {
        found = true;
        nextfield = this;
        return NULL;
        }
    else if(found)
        {
        if(next)
            {
            next->read(kar,nextfield);
            return NULL;
            }
        else
            {
            nextfield = NULL;
            return kar; // add to first
            }
        //assert(nextfield != this);
        }
    else
        return kar; // add to previous
    }

char * readAllButWhiteSpace::read(char * kar,field *& nextfield)
    {
    int k = *kar;
    if(isspace(k))
        {
        if(found)
            {
            if(next)
                {
                next->read(kar,nextfield);
                return NULL;
                }
            else
                {
                nextfield = NULL;
                return kar; // add to first
                }
            }
        else
            return kar;
        }
    else
        {
        found = true;
        nextfield = this;
        return NULL;
        }
    }

char * readTab::read(char * kar,field *& nextfield)
    {
    if(*kar == '\t')
        {
        nextfield = next;
        return NULL;
        }
    else
        return kar;
    }

char * readNewLine::read(char * kar,field *& nextfield)
    {
    if(*kar == '\n')
        {
        nextfield = next;
        return NULL;
        }
    else
        return kar;
    }

readLitteral::readLitteral(char first)
    {
    giveback = NULL;
    givebacklen = 0;
    litteral = new char[2];
    litteral[0] = first;
    litteral[1] = '\0';
    matched = new char[2];
    len = 2;
    pos = 0;
    }

void readLitteral::add(char kar)
    {
    ++len;
    char * nw = new char[len];
    delete matched;
    matched = new char[len];
    strcpy(nw,litteral);
    nw[len-2] = kar;
    nw[len-1] = '\0';
    delete litteral;
    litteral = nw;
    }

char * readLitteral::read(char * kar,field *& nextfield)
    {
    if(pos == len - 1) // all matched
        {
        if(next)
            next->read(kar,nextfield);
        return NULL;
        }
    else
        {
        matched[pos] = *kar;
        if(*kar == litteral[pos])
            {
            pos++;
            return NULL;
            }
        else
            {
            int j;
            int l = pos;
            for(j = 1;l;++j,--l)
                if(!strncmp(litteral,matched+j,l))
                    {
                    if(givebacklen <= j)
                        {
                        delete giveback;
                        giveback = new char[j+1];
                        }
                    int k;
                    for(k = 0;k < j;++k)
                        giveback[k] = matched[k];
                    giveback[k] = '\0';
                    for(;k <= l;++k)
                        matched[k-j] = matched[k];
                    matched[k-j] = '\0';
                    pos = pos - j + 1;
                    return giveback;
                    }
            matched[pos+1] = '\0';
            pos = 0;
            return matched;
            }
        }
    }
#endif