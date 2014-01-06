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


#include "flex.h"
#if defined PROGLEMMATISE
#include "utf8func.h" // Inc
#include "caseconv.h"
#include "readlemm.h"
#include "applyrules.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#if STREAM
#include <iostream>
#include <iomanip> 
using namespace std;
#endif

bool flex::baseformsAreLowercase = true;

void base::print(int n)
    {
#if STREAM
    cout << setw(n) << "[" << m_baseform << "] " << refcnt << endl;
#else
    printf("%*c%s] %d\n",n,'[',m_baseform,refcnt);
#endif
    if(m_next)
        m_next->print(n);
    }

//------------------
void node::removeAmbiguous(node *& prev)
    {
    if(basef)
        {
        if(basef->Next())
            {
            delete basef;
            basef = 0;
            }
        }
    
    if(m_sub)
        m_sub->removeAmbiguous(m_sub);

    if(!m_sub && !basef)
        {
        prev = remove();
        if(prev)
            prev->removeAmbiguous(prev);
        }
    else if(m_next)
        m_next->removeAmbiguous(m_next);
    }
//------------------
void node::print(int n)
    {
#if STREAM
    cout << setw(n) << "{" << m_tail << "}" << endl;
#else
    printf("%*c%s}\n",n,'{',m_tail);
#endif
    if(m_sub)
        m_sub->print(n+2);
    if(basef)
        basef->print(n);
    if(m_next)
        m_next->print(n);
    }
//-----------------

char * type::Baseform(char * invertedWord,base *& bf,size_t & ln)
    {
    base * BF = 0;    
    size_t LN = 0;
    char * TP = 0;
    if(m_next)
        TP = m_next->Baseform(invertedWord,BF,LN);
    if(end->Baseform(invertedWord,bf,ln))
        {
        if(TP && BF && LN > ln) // choose the longest 
                        // (what if there are more than one?)
                // "longest" rule (rule that looks at most characters of 
                // word's ending to decide on the proper baseform) works 
                // better than both refcount and "shortest" rule. A priory, 
                // the latter should be better, because shorter rules should
                // be more general than longer rules. But then again, the 
                // chance for mistakes is greater if the rule looks at fewer
                // characters before deciding.
            {
            bf = BF;
            ln = LN;
            return TP;
            }
        else
            return this->m_tp;
        }
    else if(TP)
        {
        bf = BF;
        ln = LN;
        return TP;
        }
    else
        return 0;
    }

void type::print()
    {
#if STREAM
    cout << m_tp << endl;
#else
    printf("%s\n",m_tp);
#endif
    if(end)
        end->print(2);
    if(m_next)
        m_next->print();
    }

void type::removeAmbiguous(type *& prev)
    {
    if(end)
        {
        end->removeAmbiguous(end);
        if(!end)
            {
            prev = remove();
            if(prev)
                prev->removeAmbiguous(prev);
            return;
            }
        }
    if(m_next)
        m_next->removeAmbiguous(m_next);
    }

//------------------
void flex::removeAmbiguous()
    {
    if(newStyleRules())
        {
        oneAnswer = true;
        }
    else if(types)
        types->removeAmbiguous(types);
    }

void flex::print()
    {
    if(types)
        types->print();
    }

bool flex::Baseform(const char * word,const char * tag,const char *& bf,size_t & borrow)
    {
    if(newStyleRules())
        {
        bf = applyRules(word,tag);
        return true;
        }

    if(types)
        {
        size_t offset = 0;
        size_t wlen = strlen(word);
        if(wlen > 256)
            {
            if(baseformsAreLowercase)
                {
                size_t length = 0;
                word = changeCase(word,true,length);

                }
            bf = word;
            borrow = wlen;
            return true;
            }
        static char aWord[256];
        if(baseformsAreLowercase)
            {
            size_t length = 0;
            strncpy(aWord,changeCase(word,true,length),sizeof(aWord)-1);
            aWord[sizeof(aWord)-1] = '\0';
            }
        else
            strcpy(aWord,word);
        base * Base;
        Strrev(aWord);
        if(types->Baseform(aWord,tag,Base,offset))
            {
            borrow = wlen - offset;
            static char buf[256];
            char * b = buf;
            for(;;)
                {
                strncpy(b,word,borrow);
                strcpy(b+borrow,Base->bf());

                if(baseformsAreLowercase)
                    {
                    size_t length = 0;
                    strcpy(b,changeCase(b,true,length));
                    }
                else if(IsAllUpper(word)) // made UTF8-capable
                    {
                    size_t length = 0;
                    strcpy(b,changeCase(b,false,length));
                    }
                else if(borrow == 0 && is_Upper(word))
                    {
                    size_t length = 0;
                    strcpy(b,changeCase(b,false,length));
                    length = 0;
                    char * nb = b + skipUTF8char(b);
                    strcpy(nb,changeCase(nb,true,length));
                    }

                b += strlen(b);
                Base = Base->Next();
                if(Base)
                    {
                    *b++ = ' ';
                    }
                else
                    break;
                }
            bf = buf;
            return true;
            }
        else
            {
            return false;
            }
        }
    else
        return false;
    }

char * flex::Baseform(const char * word,const char *& bf,size_t & borrow)
    {
    if(newStyleRules())
        {
        bf = applyRules(word);
        static char hyphen[] = "-";
        return hyphen; // 20120710 Returning "-" directly generates warning in newer gcc
        }


    if(types)
        {
        size_t offset = 0;
        size_t wlen = strlen(word);
        if(wlen > 256)
            {
            if(baseformsAreLowercase)
                {
                size_t length = 0;
                word = changeCase(word,true,length);
                }
            bf = word;
            borrow = wlen;
            return 0;
            }
        static char aWord[256];
        if(baseformsAreLowercase)
            {
            size_t length = 0;
            strncpy(aWord,changeCase(word,true,length),sizeof(aWord)-1);
            aWord[sizeof(aWord)-1] = '\0';
            }
        else
            strcpy(aWord,word);
        base * Base;
        Strrev(aWord);
        char * tag = types->Baseform(aWord,Base,offset);
        if(tag)
            {
            borrow = wlen - offset;
            static char buf[256];
            char * b = buf;
            
            for(;;)
                {
                strncpy(b,word,borrow);
                strcpy(b+borrow,Base->bf());
                if(baseformsAreLowercase)
                    {
                    size_t length = 0;
                    strcpy(b,changeCase(b,true,length));
                    // We have no lexical type information to 
                    }
                // decide whether capitals should be used, so we assume all 
                // lower case.

                b += strlen(b);
                Base = Base->Next();
                if(Base)
                    {
                    *b++ = ' ';
                    }
                else
                    break;
                }
            bf = buf;
            return tag;
            }
        else
            {
            return 0;
            }
        }
    else
        return 0;
    }

    bool flex::readFromFile(FILE * fpflex,const char * flexFileName)
        {
        if(!fpflex)
            {
            return false;
            }
        int start;
        if(fread(&start,sizeof(int),1,fpflex) != 1)
            return false;
        rewind(fpflex);
        if(start == 0)
            { // new style flexrules 20080218
            if(!readRules(fpflex,flexFileName))
                return false;
            }
        else
            {
            setNewStyleRules(false);
            return readFromFile(fpflex);
            }
        return true;
        }
#endif
