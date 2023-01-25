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
#include "wordReader.h"
#if defined PROGLEMMATISE

#include "field.h"
#include "XMLtext.h"
#include "utf8func.h"
#include "entities.h"
#include <string.h>
#include <assert.h>

#define BUFSIZE 35000

void wordReader::initWord()
    {
    nextfield = format;
    newlines = 0;
    format->reset();
    kars[1] = '\0';
    if(lastkar)
        {
        if(lastkar == '\n')
            ++newlines;
        kars[0] = (char)lastkar;
        lastkar = 0;
        nextfield->read(kars, nextfield);
        }
    token* Tok = Text->getCurrentToken();
    if(Tok)
        {
        if(kars[0])
            {
            Tok->tokenWord.set(Text->ch - 1, Text->ch - 1);
            Tok->tokenToken.set(Text->ch - 1, Text->ch - 1);
            }
        else
            {
            Tok->tokenWord.set(Text->ch, Text->ch);
            Tok->tokenToken.set(Text->ch, Text->ch);
            }
        }
    }

wordReader::wordReader(field* format, field* wordfield, field* tagfield, bool treatSlashAsAlternativesSeparator, XMLtext* Text)
    :format(format)
    , nextfield(0/*format*/)
    , wordfield(wordfield)
    , tagfield(tagfield)
    , newlines(0)
    , lineno(0)
    , tag(NULL)
    , lastkar(0)
    , treatSlashAsAlternativesSeparator(treatSlashAsAlternativesSeparator)
    , Text(Text)
    {
    initWord();
    assert(wordfield);
    if(lastkar)
        {
        assert(lastkar != EOF);
        if(lastkar == '\n')
            ++newlines;
        kars[0] = (char)lastkar;
        lastkar = 0;
        nextfield->read(kars, nextfield);
        }
    xput = &wordReader::rawput;
    p = buf = new char[BUFSIZE];
    }


wordReader::~wordReader()
    {
    delete[] buf;
    }

CHAR* wordReader::readChar(int kar)
    {
    if(!nextfield)
        {
        initWord();
        }
    if(kar == '\n')
        ++newlines;
    kars[0] = kar == EOF ? '\0' : (char)kar;
    char* plastkar = nextfield->read(kars, nextfield);
    if(kar == '\0')
        lastkar = '\0';
    else if(plastkar)
        lastkar = *plastkar;

    if(nextfield)
        return NULL;
    else
        {
        lineno += newlines;
        if(tagfield)
            tag = tagfield->getString();
        return wordfield->getString();
        }
    }


bool wordReader::countToken(int kar)
    {
    CHAR* w = readChar(kar);
    //    if(!Text->segmentBreak())
    {
    if(Text->analyseThis())
        {
        if(!Text->wordAttribute && w && *w)
            {
            Text->incTotal();
            if(treatSlashAsAlternativesSeparator)
                {
                Text->incTotal(findSlashes(w));
                }
            return true;
            }
        }
    }
    return false;
    }
/*
Suppose that if the input is tagged XML and contains an element like

<span xml:id="t31767" from="#i58910" to="#i58912" pos="X" lemma="samfund">samfund ...</span>

then the "..." is treated as untagged!

Therefore one cannot assume that all generated words are of the class taggedWord. Some can be of the class word.

*/
bool wordReader::readToken(int kar)
    {
    CHAR* w = readChar(kar);
    //    if(!Text->segmentBreak())
    {
    if(Text->analyseThis())
        {
        if(!Text->wordAttribute && w && *w)
            {
            token* Tok = Text->getCurrentToken();
            char* start = Tok->tokenWord.getStart();
            Tok->tokenWord.set(start, start + strlen(w));
            char* POS = Tok->tokenPOS.getStart();
            if(POS != NULL)
                {
                char* EP = Tok->tokenPOS.getEnd();
                char savP = *EP;
                *EP = '\0';
                if(treatSlashAsAlternativesSeparator && findSlashes(w))
                    Text->createTaggedAlternatives(w, POS);
                else
                    Text->createTagged(w, POS);
                *EP = savP;
                }
            else if(tag)
                {
                if(treatSlashAsAlternativesSeparator && findSlashes(w))
                    Text->createTaggedAlternatives(w, tag);
                else
                    Text->createTagged(w, tag);
                }
            else
                {
                if(treatSlashAsAlternativesSeparator && findSlashes(w))
                    Text->createUnTaggedAlternatives(w);
                else
                    Text->createUnTagged(w);
                }
            Text->wordDone();
            return true;
            }
        }
    }
    return false;
    }

int wordReader::rawput(bool (wordReader::* fnc)(int kar), int kar)
    {
    return (this->*fnc)(kar);
    }

int wordReader::nrawput(bool (wordReader::* fnc)(int kar), char* c)
    {
    while(*c)
        rawput(fnc, *c++);
    return true;
    }

const char* wordReader::convert(const char* s, char* buf, const char* lastBufByte)
    {
    if(buf + strlen(s) < lastBufByte)
        {
        char* q = buf;
        char entity[100];
        char* p = NULL;
        for(const char* t = s; *t; ++t)
            {
            if(*t == '&')
                {
                p = entity;
                }
            else if(p)
                {
                if(*t == ';')
                    {
                    *p = '\0';
                    char* pItem = findEntity(entity);
                    if(pItem != NULL)
                        {
                        p = pItem;
                        }
                    else
                        {
                        *q++ = '&';
                        p = entity;
                        }
                    for(; *p; ++p && (q < lastBufByte), ++q)
                        *q = *p;
                    p = NULL;
                    }
                else if(q < lastBufByte)
                    {
                    *p++ = *t;
                    }
                else
                    break;
                }
            else if(q < lastBufByte)
                {
                *q++ = *t;
                }
            else
                break;
            }
        *q = '\0';
        return buf;
        }
    return s;
    }

#endif
