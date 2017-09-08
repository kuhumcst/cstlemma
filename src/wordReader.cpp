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
/*
static int (*namechar)(int c);

static bool anychar = false;

struct lowup
    {
    int  l;
    int u;
    int s; / * NameStartChar * /
    };

struct lowup lu[] =
    {     {'-','-',false}
        , {'.','.',false}
        , {'0','9',false}
        , {':',':',true}
        , {'A','Z',true}
        , {'_','_',true}
        , {'a','z',true} 
        , {0xB7,0xB7,false}
        , {0xC0,0xD6,true} 
        , {0xD8,0xF6,true}
        , {0xF8,0x2FF,true}
        , {0x300,0x36F,false}
        , {0x370,0x37D,true}
        , {0x37F,0x1FFF,true}
        , {0x200C,0x200D,true}
        , {0x203F,0x2040,false}
        , {0x2070,0x218F,true}
        , {0x2C00,0x2FEF,true}
        , {0x3001,0xD7FF,true}
        , {0xF900,0xFDCF,true}
        , {0xFDF0,0xFFFD,true}
        , {0x10000,0xEFFFF,true}
        , {0x7FFFFFFF,0x7FFFFFFF,true}
    };

static int NameChar(int c)
    {
    int i;
    for( i = 0
       ; c > lu[i].u
       ; ++i
       )
       ;
    return c >= lu[i].l && (anychar || lu[i].s);        
    }

static int decimal(int c)
    {
    if('0' <= c && c <= '9')
        return true;
    return false;
    }

static int hex(int c)
    {
    if(  ('0' <= c && c <= '9')
      || ('A' <= c && c <= 'F')
      || ('a' <= c && c <= 'f')
      )
        return true;
    return false;
    }

static int number(int c)
    {
    if(c == 'x' || c == 'X')
        {
        namechar = hex;
        return true;
        }
    if(decimal(c))
        {
        namechar = decimal;
        return true;
        }
    return false;
    }

static int entity(int c)
    {
    if(c == '#')
        {
        namechar = number;
        return true;
        }
    else
        {
        anychar = false;
        if(NameChar(c))
            {
            anychar = true;
            namechar = NameChar;
            return true;
            }
        }
    return false;
    }
*/

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
        nextfield->read(kars,nextfield);
        }
    token * Tok = Text->getCurrentToken();
    if(Tok)
        {
        if(kars[0])
            {
            Tok->tokenWord.set(Text->ch-1,Text->ch-1);
            Tok->tokenToken.set(Text->ch-1,Text->ch-1);
            }
        else
            {
            Tok->tokenWord.set(Text->ch,Text->ch);
            Tok->tokenToken.set(Text->ch,Text->ch);
            }
        }
    }

wordReader::wordReader(field * format,field * wordfield,field * tagfield,bool treatSlashAsAlternativesSeparator,XMLtext * Text)
    :format(format)
    ,nextfield(0/*format*/)
    ,wordfield(wordfield)
    ,tagfield(tagfield)
    ,newlines(0)
    ,lineno(0)
    ,tag(NULL)
    ,lastkar(0)
    ,treatSlashAsAlternativesSeparator(treatSlashAsAlternativesSeparator)
    ,Text(Text)
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
        nextfield->read(kars,nextfield);
        }
    xput = &wordReader::rawput;
    p = buf = new char[BUFSIZE];
    }


wordReader::~wordReader()
    {
    delete [] buf;
    }

CHAR * wordReader::readChar(int kar)
    {
    if(!nextfield)
        {
        initWord();
        }
    if(kar == '\n')
        ++newlines;
    kars[0] = kar == EOF ? '\0' : (char)kar;
    char * plastkar = nextfield->read(kars,nextfield);
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
    CHAR * w = readChar(kar);
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

bool wordReader::readToken(int kar)
    {
    CHAR * w = readChar(kar);
//    if(!Text->segmentBreak())
        {
        if(Text->analyseThis())
            {
            if(!Text->wordAttribute && w && *w)
                {
                token * Tok = Text->getCurrentToken();
                char * start = Tok->tokenWord.getStart();
                Tok->tokenWord.set(start,start+strlen(w));
                char * POS = Tok->tokenPOS.getStart();
                if(POS != NULL)
                    {
                    char * EP = Tok->tokenPOS.getEnd();
                    char savP = *EP;
                    *EP = '\0';
                    if(treatSlashAsAlternativesSeparator && findSlashes(w))
                        Text->createTaggedAlternatives(w,POS);
                    else
                        Text->createTagged(w,POS);
                    *EP = savP;
                    }
                else if(tag)
                    {
                    if(treatSlashAsAlternativesSeparator && findSlashes(w))
                        Text->createTaggedAlternatives(w,tag);
                    else
                        Text->createTagged(w,tag);
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

int wordReader::rawput(bool (wordReader::*fnc)(int kar),int kar)
    {
    return (this->*fnc)(kar);
    }

int wordReader::nrawput(bool (wordReader::*fnc)(int kar),char * c)
    {
    while(*c)
        rawput(fnc,*c++);
    return true;
    }

/*
int wordReader::charref(bool (wordReader::*fnc)(int kar),int kar)
    {
    if(kar == ';')
        {
        *p = '\0';
        char * pItem = findEntity(buf);
        if (pItem!=NULL)
            {
            p = buf;
            xput = &wordReader::Put;
            return nrawput(fnc, pItem);
            }
        rawput(fnc,'&');
        nrawput(fnc,buf);
        rawput(fnc,';');
        p = buf;
        xput = &wordReader::Put;
        return false;
        p = buf;
        xput = &wordReader::Put;
        }
    else if(!namechar(kar))
        {
        rawput(fnc,'&');
        *p = '\0';
        nrawput(fnc,buf);
        if(kar > 0)
            rawput(fnc,kar);
        p = buf;
        xput = &wordReader::Put;
        return false;
        }
    else if(p < buf+BUFSIZE-1)
        {
        *p++ = (char)kar;
        }
    return true;
    }

int wordReader::Put(bool (wordReader::*fnc)(int kar),int kar)
    {
    if(kar == '&')
        {
        xput = &wordReader::charref;
        namechar = entity;
        return true;
        }
    return rawput(fnc,kar);
    }
*/

const char * wordReader::convert(const char * s,char * buf,const char * lastBufByte)
    {
    if(buf+strlen(s) < lastBufByte)
        {
        char * q = buf;
        char entity[100]; 
        char * p = NULL;
        for(const char * t = s;*t;++t)
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
					char * pItem = findEntity(entity);
					if (pItem!=NULL)
						{
						p = pItem;
						}
					else
						{
						*q++ = '&';
						p = entity;
						}
					for(;*p;++p && (q < lastBufByte),++q)
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
