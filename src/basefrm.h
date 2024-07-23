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
#ifndef BASEFRM_H
#define BASEFRM_H

#include "defines.h"
#if defined PROGLEMMATISE
#include "outputclass.h"
#include <string.h>
#if STREAM
# include <iostream>
# ifndef __BORLANDC__
using namespace std;
# endif
#else
# include <stdio.h>
#endif

class baseformpointer;
class Word;
class basefrm;

enum Wstatus { nofW, fseen, Wseen, fWseen };
/*  0: no f or W seen in baseform format
    1: f seen
    2: W seen
    3: both f and W seen

    if 1: create dummy -W option with no parameter.
    This is useful if you ask for lemma frequencies without also asking for full form frequencies.
    Although you don't want to see them, they are needed to compute the lemma frequencies.

    Example

    -q fwt
    -B $f\t$w\n    <-- $f seen, but no $W
    -b $f\t$w\n
                   <-- No -W (why should I need one?)

    In this casse cstlemma creates a dummy -W option.
*/

class basefrm : public OutputClass
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        basefrm& operator= (const basefrm& f)
            {
            REFER(f)
                return *this;
            }
    private:
        Word** fullForm; // list of full forms
        unsigned int nfullForm : 8;
#if FREQ24
        unsigned int freq24 : 24; // 0 for new words; >= 0 for words from the
        // dictionary: the number of times the word is found in a 
        // corpus (-N<frequency file).
#endif
        char* m_s;
        char* m_t;
#if PRINTRULE
        char* m_p;
        const char* m_r() const
            {
            return m_p ? m_p + strlen(m_p) + 1 : 0L;
            }
#endif
        static int index;
    public:
#if STREAM
        static ostream* m_fp;
#else
        static FILE* m_fp;
#endif
    public:
        void T() const;
    private:
        void F() const
            {
#if STREAM
            * m_fp << lemmaFreq();
#else
            fprintf(m_fp, "%d", lemmaFreq()); // shows the frequency of the lemma in the current text.
#endif
            }
#if FREQ24
        void N() const
            {
#if STREAM
            * m_fp << freq24;
#else
            fprintf(m_fp, "%u", freq24);
#endif
            }
#endif
        void W() const;
        void L() const;
    public:
#if PRINTRULE
        void P() const;
        void R() const;
#endif
        static functionTree* bfuncs;// used if -W option set
        static functionTree* Bfuncs;// used if -W option set
        static functionTree* wfuncs;// used if -W option set
        static functionTree* Format(const char* format);
        static bool hasW;
        static Wstatus needW;
        static const char* sep;
        static bool setFormat(const char* Wformat, const char* bformat, const char* Bformat, bool InputHasTags); // used with -W option
        static formattingFunction* getBasefrmFunction(int character, bool& DummySortInput, int& testType);
        static formattingFunction* getBasefrmFunctionNoW(int character, bool& DummySortInput, int& testType);
#if STREAM
        static void setFile(ostream* a_fp);
#else
        static void setFile(FILE* a_fp);
#endif
        int cmpf(const basefrm* b) const { return b->lemmaFreq() - lemmaFreq(); }
        int cmpt(const basefrm* b) const { return strcmp(m_t, b->m_t); }
        int cmps(const basefrm* b) const { return strcmp(m_s, b->m_s); }
#if PRINTRULE
        int cmpp(const basefrm* b) const { return strcmp(m_p, b->m_p); }
#endif
        baseformpointer& m_owner;
#if PRINTRULE
#if FREQ24
        basefrm(const char* s, const char* t, const char* p, baseformpointer& owner, size_t len,/*int cnt,*/unsigned int frequency) :fullForm(NULL), nfullForm(0), freq24(frequency), m_owner(owner)
#else
        basefrm(const char* s, const char* t, baseformpointer& owner, size_t len/*int cnt,*/) : fullForm(NULL), nfullForm(0), m_owner(owner)
#endif
#else
#if FREQ24
        basefrm(const char* s, const char* t, baseformpointer& owner, size_t len,/*int cnt,*/unsigned int frequency) :fullForm(NULL), nfullForm(0), freq24(frequency), m_owner(owner)
#else
        basefrm(const char* s, const char* t, baseformpointer& owner, size_t len/*int cnt,*/) : fullForm(NULL), nfullForm(0), m_owner(owner)
#endif
#endif
            {
            this->m_s = new char[len + 1];
            strncpy(this->m_s, s, len);
            this->m_s[len] = '\0';
            this->m_t = new char[strlen(t) + 1];
            strcpy(this->m_t, t);
#if PRINTRULE
            this->m_p = strchr(this->m_s, '\v');
            if(this->m_p)
                {
                *this->m_p++ = 0;
                char* r = strchr(this->m_p, '\v');
                *r = 0;
                }
#endif
#ifdef COUNTOBJECTS
            ++COUNT;
#endif
            }
        ~basefrm();
        void getAbsorbedBy(basefrm* other);
        void addFullForm(Word* word);
        void addFullForms(basefrm* other);
        virtual void printb()const;
        virtual void printB()const;
        int countFullForms() const;
        int lemmaFreq() const;
        void testPrint()const;
        bool equal(const char* s, const char* t)
            {
#if PRINTRULE
            /* s contains lemma\vrule */
            if(strcmp(t, this->m_t))
                return false;
            const char* S = this->m_s;
            while(*s == *S)
                {
                ++s;
                ++S;
                }
            return !*S && (!*s || *s == '\v');
#else
            return !strcmp(s, this->m_s) && !strcmp(t, this->m_t);
#endif
            }
        int Closeness(const char* tag);
        void removeFullForm(Word* w);
    };

class tagpairs;
extern tagpairs* TagFriends;

extern void (*print)(
#if STREAM
    ostream* fpo
#else
    FILE* fpo
#endif
    , const char* s);

#endif
#endif
