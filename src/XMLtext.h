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
#ifndef XMLTEXT_H
#define XMLTEXT_H

#include "defines.h"
#if defined PROGLEMMATISE

#include <stddef.h>
#include "text.h"

class substring
    {
    private:
        char * start;
        char * end;
    public:
        substring():start(NULL),end(NULL)
            {
            }
        void set(char * Start,char * End)
            {
            this->start = Start;
            this->end = End;
            }
        char * getStart() const
            {
            return start;
            }
        char * getEnd() const
            {
            return end;
            }
    };


class token
    {
    public:
        substring tokenWord;
        substring tokenPOS;
        substring tokenLemma;
        substring tokenLemmaClass;
        substring tokenToken;
    };

class crumb;
struct optionStruct;

class XMLtext : public text
    {
    public:
        char * ch;
        const char * wordAttribute; // if null, word is PCDATA
    private:
        token * Token;
        char * startElement;
        char * endElement;
        char * startAttributeName;
        char * endAttributeName;
        char * startValue;
        char * endValue;
        const char * ancestor; // if not null, restrict lemmatisation to elements that are offspring of ancestor
        const char * element; // if not null, analyse only element's attributes and/or PCDATA
        const char * segment; // if not null, begin new segment after this tag
        const char * POSAttribute; // if null, POS is PCDATA
        const char * lemmaAttribute; // if null, Lemma is PCDATA
        const char * lemmaClassAttribute; // if null, lemma class is PCDATA
        crumb * Crumbs;
        char * alltext;
        size_t wordAttributeLen;/*20120709 int -> ptrdiff_t*/
        size_t POSAttributeLen;/*20120709 int -> ptrdiff_t*/
        size_t lemmaAttributeLen;/*20120709 int -> ptrdiff_t*/
        size_t lemmaClassAttributeLen;/*20120709 int -> ptrdiff_t*/
        bool ClosingTag;
        bool WordPosComing;
        bool POSPosComing;
        bool LemmaPosComing;
        bool LemmaClassPosComing;
    private:
        virtual const char * convert(const char * s, char * buf, const char * lastBufByte);
    public:
        void wordDone();
        bool analyseThis();
        bool segmentBreak();
        token * getCurrentToken();  // pristine
        token * getJustMadeToken(); // the previous one, that is. May be partly filled in.
        void reachedTokenEnd(char * ch);
        void CallBackStartElementName();
        void CallBackEndElementName();
        void CallBackStartAttributeName();
        void CallBackEndAttributeNameInserting();
        void CallBackEndAttributeNameCounting();
        void CallBackStartValue();
        void CallBackEndValue();
        void CallBackEndTag();
        void CallBackEmptyTag();
        void CallBackNoMoreAttributes();
        XMLtext(
#if STREAM
            istream * fpi
#else
            FILE * fpi
#endif
            ,optionStruct & Option
/*
            ancestor // if not null, restrict lemmatisation to elements that are offspring of ancestor
            element // if null, analyse all PCDATA that is text
            wordAttribute // if null, word is PCDATA
            POSAttribute // if null, POS is PCDATA
            lemmaAttribute // if null, Lemma is PCDATA
            lemmaClassAttribute // if null, lemma class is PCDATA
*/
            );
        ~XMLtext(){}
        virtual void DoYourWork(
#if STREAM
            istream* fpi
#else
            FILE* fpi
#endif
            , optionStruct& Option
        );
        virtual void printUnsorted(
#if STREAM
            ostream * fpo
#else
            FILE * fpo            
#endif
            );
    };

#endif
#endif
