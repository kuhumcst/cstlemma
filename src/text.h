/*
CSTLEMMA - trainable lemmatiser

Copyright (C) 2002, 2005  Center for Sprogteknologi, University of Copenhagen

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
#ifndef TEXT_H
#define TEXT_H

#include "defines.h"
#if STREAM
# include <iostream>
# ifndef __BORLANDC__
using namespace std;
# endif
#else
# include <stdio.h>
#endif

class Word;
class taggedWord;
class basefrm;

struct tallyStruct
    {
    unsigned long int totcnt;
    unsigned long int totcntTypes;
    unsigned long int newcnt;
    unsigned long int newcntTypes;
    unsigned long int newhom;
    unsigned long int newhomTypes;
    tallyStruct()
        {
        totcnt = 0;
        totcntTypes = 0;
        newcnt = 0;
        newcntTypes = 0;
        newhom = 0;
        newhomTypes = 0;
        }
    };

class field;


class text      
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        /*
        token * Token;
        */
        virtual const char * convert(const char * s)
            {
            return s;
            }
    protected:
        size_t N;
        Word ** Root;
        const Word ** tunsorted;
        unsigned long int * Lines;
        unsigned long int lineno;
        //bool sorted;
        unsigned long int total;
        unsigned long int reducedtotal;
        field * fields;
        void AddField(field * fld);
        field * translateFormat(char * Iformat,field *& wordfield,field *& tagfield);
    private:
        basefrm ** basefrmarrD;
        basefrm ** basefrmarrL;
    protected:
        bool InputHasTags;
    private:
/*
        field * fields;
        void AddField(field * fld);
        field * translateFormat(char * Iformat,field *& wordfield,field *& tagfield);
*/
    protected:
        void insert(const char * w);
        void insert(const char * w, const char * tag);
    private:
/*
    public:
        char * ch;
    private:
        char * startElement;
        char * endElement;
        char * startAttributeName;
        char * endAttributeName;
        char * startValue;
        char * endValue;
        const char * ancestor; // if not null, restrict lemmatisation to elements that are offspring of ancestor
        const char * element; // if not null, analyse only element's attributes and/or PCDATA
    public:
        const char * wordAttribute; // if null, word is PCDATA
    private:
        const char * POSAttribute; // if null, POS is PCDATA
        const char * lemmaAttribute; // if null, Lemma is PCDATA
        const char * lemmaClassAttribute; // if null, lemma class is PCDATA
        int wordAttributeLen;
        int POSAttributeLen;
        int lemmaAttributeLen;
        int lemmaClassAttributeLen;
        crumb * Crumbs;
        bool ClosingTag;
        bool WordPosComing;
        bool POSPosComing;
        bool LemmaPosComing;
        bool LemmaClassPosComing;
        char * alltext;
*/
    public:
        /*
        token * getCurrentToken();
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
        */
        basefrm ** ppD;
        basefrm ** ppL;
        int cntD;
        int cntL;
        int newcnt;
        int newcntTypes;
        int aConflict;
        int aConflictTypes;
        void incTotal()
            {
            ++total;
            }
        void incTotal(unsigned long int inc)
            {
            total += inc;
            }
        static bool setFormat(const char * format,const char * bformat,const char * Bformat,bool InputHasTags);
#if STREAM
        void Lemmatise
            (ostream * fpo
            /*,FILE * fpnew
            ,FILE * fpconflict*/
            ,const char * Sep
            ,tallyStruct * tally
            ,unsigned int SortOutput
            ,int UseLemmaFreqForDisambiguation
            ,bool nice
            ,bool DictUnique
            ,bool baseformsAreLowercase
            ,int listLemmas
            ,bool mergeLemmas // Bart 20101102
            );
        text(/*istream * fpi,*/bool InputHasTags/*,char * Iformat*/,/*int keepPunctuation,*/bool nice,unsigned long int size/*,bool treatSlashAsAlternativeSeparator*/
            );
#else
        void Lemmatise
            (FILE * fpo
            /*,FILE * fpnew
            ,FILE * fpconflict*/
            ,const char * Sep
            ,tallyStruct * tally
            ,unsigned int SortOutput
            ,int UseLemmaFreqForDisambiguation
            ,bool nice
            ,bool DictUnique
            ,bool baseformsAreLowercase
            ,int listLemmas
            ,bool mergeLemmas // Bart 20101102
            );
        text(/*FILE * fpi,*/bool InputHasTags/*,char * Iformat*/,/*int keepPunctuation,*/bool nice/*,unsigned long int size,bool treatSlashAsAlternativeSeparator*/
            );
#endif
        virtual ~text();
        void createUnTaggedAlternatives(
#ifndef CONSTSTRCHR
            const 
#endif
            char * w);
        void createUnTagged(const char * w);
        void createTaggedAlternatives(
#ifndef CONSTSTRCHR
            const 
#endif

            char * w, const char * tag);
        void createTagged(const char * w, const char * tag);
        virtual void printUnsorted(
#if STREAM
            ostream * fpo
#else
            FILE * fpo            
#endif
            ) = 0;
        void makeList();
    };

extern char * globIformat;
extern int findSlashes(const char * buf);

#endif
