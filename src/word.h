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

#ifndef WORD_H
#define WORD_H

#include "defines.h"
#if defined PROGLEMMATISE
#include "outputclass.h"
#include "basefrmpntr.h"
#include <string.h>
#if STREAM
# include <iostream>
# ifndef __BORLANDC__
using namespace std;
# endif
#else
# include <stdio.h>
#endif

class taggedWord;
class Word;
typedef void (Word::*trav0)();
typedef void (taggedWord::*trav0T)();
typedef void (Word::*trav0C)()const;
typedef void (Word::*trav)(void *);

typedef int (Word::*cmp_f)(const Word *) const;
typedef int (taggedWord::*cmp_ft)(const taggedWord *) const;
class text;


class basefrm;
struct lext;

class Word : public OutputClass
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    public:
        static Word * Root;
        static int LineNumber; // The number of the line where the previous word was found. For line-wise output. 0 is initial value
        static bool DictUnique;
        static int NewLinesAfterWord;
//  protected:
    public:
        static int reducedtotal;
    protected:
        bool hasAddedItselfToBaseForm:1;
        bool FoundInDict:1;
        bool owns:1;
    protected:
#if STREAM
        static ostream * fp;
#else
        static FILE * fp;
#endif
    public:
        char * m_word;
        char * m_tag;
    protected:
        baseformpointer * pbfD;  // dictionary's base forms
        baseformpointer * pbfL; // constructed base forms
        // If there is a constructed base form (lemma), then do not do 
        // statistics on the dictionary's lemmas.
        int cnt;
        void i() const
            {
#if STREAM
            if(pbfL)
                {
                if(pbfD)
                    *fp << '+';
                else
                    *fp << '-';
                }
            else
                *fp << ' ';
#else
            if(pbfL)
                {
                if(pbfD)
                    fputc('+',fp);
                else
                    fputc('-',fp);
                }
            else
                fputc(' ',fp);
#endif
            }
        void f() const
            {
#if STREAM
            *fp << cnt;
#else
            fprintf(fp,"%d",cnt);
#endif
            }
        void w() const
            {
            ::print(fp,m_word);
#if STREAM
            //*fp << m_word;
#else
            //fprintf(fp,"%s",m_word);
#endif
            }
        void b() const
            {
            if(pbfD)
                pbfD->printfbf(fp,bfuncs,sep);
            }
        void B() const
            {
            if(pbfL)
                pbfL->printfbf(fp,Bfuncs,sep);
            }
        void s() const
            {
#if STREAM
            if(NewLinesAfterWord)
                {
                LineNumber++;
                if(LineNumber > 1)
                    *fp << '\n';
                }
            else
                *fp << ' ';
#else
            if(NewLinesAfterWord)
                {
                if(LineNumber > 0)
                    {
                    for(int n = NewLinesAfterWord;n > 0;--n)
                        fputc('\n',fp);
                    }
                LineNumber++;
                }
            else
                fputc(' ',fp);
#endif
            }
        static bool hasb;
        static bool hasB;
        unsigned int maxFrequency(lext * Plext,int nmbr,const char * a_type,int & n);// The dictionary's available
                               // lexical information for this word.
        char * commonStem(lext * Plext,int nmbr,const char * type,unsigned int freq,unsigned int & offset);
         // Find the common type of the most frequent readings
        char * commonType(lext * Plext,int nmbr,unsigned int freq);
    public:
        void inc(){++cnt;}
        static functionTree * funcs;
        static bool setFormat(const char * cformat,const char * bformat,const char * Bformat,bool InputHasTags);
        virtual bool skip()const
            {
            return false; 
            }
        static functionTree * bfuncs;
        static functionTree * Bfuncs;
        static void deleteStaticMembers();
        static function * getUnTaggedWordFunction(int character,bool & SortInput,int & testType);
        static function * getUnTaggedWordFunctionNoBb(int character,bool & SortInput,int & testType);
#if STREAM
        static void setFile(ostream * a_fp);
#else
        static void setFile(FILE * a_fp);
#endif
        static const char * sep;
        int itsCnt()const{return cnt;}
        const char * itsWord()const{return m_word;}
        int cmpword(const Word * other)const{return strcmp(m_word,other->m_word);}
        static cmp_f cmp;
        int comp_fw(const Word * w) const
            {
            int c = w->cnt - cnt;
            if(!c) c = strcmp(m_word,w->m_word);
            return c;
            }

        int comp_wf(const Word * w) const
            {
            int c = strcmp(m_word,w->m_word);
            if(!c) c = w->cnt - cnt;
            return c;
            }
        void assignTo(basefrm **& D,basefrm **& L)
            {
            if(pbfL && !FoundInDict)
                pbfL->assignTo(L);
            else if(pbfD) // we do not do gather statistics from both.
                pbfD->assignTo(D);
            }
        int countBaseForms() const
            {
            if(pbfD)
                return pbfD->count();
            else
                return 0;
            }
        int countBaseFormsL()const
            {
            if(pbfL)
                return pbfL->count();
            else
                return 0;
            }
        virtual void print()const;
        virtual void printLemmaClass()const;
        virtual void printnew()const
            {
            if(pbfL && !pbfD)
                {
                print();
                }
            }            
        virtual void printConflict()const
            {
            if(pbfL && pbfD)
                {
                print();
                }
            }
        Word(const char * word)
            : 
                hasAddedItselfToBaseForm(false),FoundInDict(false),owns(true)
                ,pbfD(NULL),pbfL(NULL),cnt(1)
            {
            this->m_word = new char[strlen(word) + 1];
            strcpy(this->m_word,word);
            ++reducedtotal;
#ifdef COUNTOBJECTS
            ++COUNT;
#endif
            }
        Word(const Word & w)
            : 
                hasAddedItselfToBaseForm(w.hasAddedItselfToBaseForm),
                FoundInDict(w.FoundInDict),
                owns(false),
                m_word(w.m_word),
                pbfD(w.pbfD),
                pbfL(w.pbfL),
                cnt(w.cnt)
            {
#ifdef COUNTOBJECTS
            ++COUNT;
#endif
            }
        void deleteSecondaryStuff()
            {
            delete pbfD;
            delete pbfL;
            pbfD = NULL;
            pbfL = NULL;
            }
        virtual ~Word()
            {
            if(owns)
                {
                delete m_word;
                deleteSecondaryStuff();
                }
#ifdef COUNTOBJECTS
            --COUNT;
#endif
            }
#if PFRQ || FREQ24
        int addBaseFormD(const char * s,const char * t,unsigned int frequency)
            {
            //this->cnt++;
            if(pbfD)
                return pbfD->addBaseForm(s,t,strlen(s),frequency);
            else
                pbfD = new baseformpointer(s,t,strlen(s),frequency);
            return 1;
            }
#else
        int addBaseFormD(const char * s,const char * t)
            {
            //this->cnt++;
            if(pbfD)
                return pbfD->addBaseForm(s,t,strlen(s));
            else
                pbfD = new baseformpointer(s,t,strlen(s));
            return 1;
            }
#endif
        int addBaseFormL(const char * s,const char * t);
        virtual int addBaseFormsL();
		virtual int addBaseFormsDL(lext * Plext,int nmbr,// The dictionary's available
                               // lexical information for this word.
           bool & conflict,int & cntD,int & cntL);//
        void addFullForm()
            {
            if(!hasAddedItselfToBaseForm)
                {
                hasAddedItselfToBaseForm = true;
                if(pbfD)
                    pbfD->addFullForm(this);
                if(pbfL)
                    pbfL->addFullForm(this);
                }
            }
        void DissambiguateByLemmaFrequency()
            {
            if(pbfD)
                {
                pbfD->DissambiguateByLemmaFrequency();
                }
            if(pbfL)
                {
                pbfL->DissambiguateByLemmaFrequency();
                }
            }
        void decFreq()
            {
            if(pbfD)
                {
                pbfD->decFreq(this);
                }
            if(pbfL)
                {
                pbfL->decFreq(this);
                }
            }
        void lookup(text * txt);
    };

class taggedWord : public Word
    {
    private:
        void t() const
            {
            ::print(fp,m_tag);
#if STREAM
            //*fp << m_tag;
#else
            //fprintf(fp,"%s",m_tag);
#endif
            }
    protected:
    public:
        static function * getTaggedWordFunction(int character,bool & SortInput,int & testType);
        static function * getTaggedWordFunctionNoBb(int character,bool & SortInput,int & testType);
        static cmp_ft comp;
        int cmptaggedword(const taggedWord * other)const
            {
            int c = strcmp(m_tag,other->m_tag);
            if(!c)
                c = strcmp(m_word,other->m_word);
            return c;
            }
        int cmp_ftw(const taggedWord * w) const
            {
            int c = ((taggedWord*)w)->cnt - cnt;
            if(!c) c = strcmp(m_tag,((taggedWord*)w)->m_tag);
            if(!c) c = strcmp(m_word,w->m_word);
            return c;
            }
        int cmp_fwt(const taggedWord* w) const
            {
            int c = ((taggedWord*)w)->cnt - cnt;
            if(!c) c = strcmp(m_word,w->m_word);
            if(!c) c = strcmp(m_tag,((taggedWord*)w)->m_tag);
            return c;
            }
        int cmp_wft(const taggedWord* w) const
            {
            int c = strcmp(m_word,w->m_word);
            if(!c) c = ((taggedWord*)w)->cnt - cnt;
            if(!c) c = strcmp(m_tag,((taggedWord*)w)->m_tag);
            return c;
            }
        int cmp_wtf(const taggedWord* w) const
            {
            int c = strcmp(m_word,w->m_word);
            if(!c) c = strcmp(m_tag,((taggedWord*)w)->m_tag);
            if(!c) c = ((taggedWord*)w)->cnt - cnt;
            return c;
            }
        int cmp_tfw(const taggedWord* w) const
            {
            int c = strcmp(m_tag,((taggedWord*)w)->m_tag);
            if(!c) c = ((taggedWord*)w)->cnt - cnt;
            if(!c) c = strcmp(m_word,w->m_word);
            return c;
            }
        int cmp_twf(const taggedWord* w) const
            {
            int c = strcmp(m_tag,((taggedWord*)w)->m_tag);
            if(!c) c = strcmp(m_word,w->m_word);
            if(!c) c = ((taggedWord*)w)->cnt - cnt;
            return c;
            }
        virtual bool skip()const
            {
            return false; 
            }
        taggedWord(const char * word,const char * tag):Word(word)
            {
            this->m_tag = new char[strlen(tag) + 1];
            strcpy(this->m_tag,tag);
            }
        taggedWord(taggedWord & w):Word(w) // (Word & w) ==> (taggedWord & w)
            {
            m_tag = w.m_tag;
            }
        virtual ~taggedWord()
            {
            if(owns)
                delete m_tag;
            }
        virtual int addBaseFormsL();
		virtual int addBaseFormsDL(lext * Plext,int nmbr,// The dictionary's available
                               // lexical information for this word.
           bool & conflict,int & cntD,int & cntL);//
        void DissambiguateByTagFriends()
            {
            if(pbfD)
                {
                pbfD->DissambiguateByTagFriends(m_tag);
                }
            if(pbfL)
                {
                pbfL->DissambiguateByTagFriends(m_tag);
                }
            }
    };

#endif
#endif