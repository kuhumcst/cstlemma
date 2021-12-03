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
#include "basefrmpntr.h"
#if defined PROGLEMMATISE
#include "basefrm.h"
#include "functiontree.h"
#include <stdio.h>

#ifdef COUNTOBJECTS
int baseformpointer::COUNT = 0;
#endif


int baseformpointer::UseLemmaFreqForDisambiguation = 0;

void baseformpointer::testPrint()
    {
    if(bf)
        bf->testPrint();
    if(next)
        {
#if STREAM
        cout << "||";
#else
        printf("||");
#endif
        next->testPrint();
        }
    }

#if FREQ24
baseformpointer::baseformpointer(const char * s, const char * t, size_t len,/*int cnt,*/unsigned int frequency) :owning(true), next(NULL), hidden(false)
    {
    bf = new basefrm(s, t, *this, len,/*cnt,*/frequency);
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }
#else
#if PFRQ
baseformpointer::baseformpointer(const char * s, const char * t, size_t len,/*int cnt,*/unsigned int frequency)
    :owning(true), next(NULL), hidden(false), pfrq(frequency)
    {
    bf = new basefrm(s, t, *this, len);
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }
#else
baseformpointer::baseformpointer(const char * s, const char * t, size_t len) :owning(true), next(NULL), hidden(false)
    {
    bf = new basefrm(s, t, *this, len);
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }
#endif
#endif

baseformpointer::~baseformpointer()
    {
    if(owning)
        {
        delete bf;
        bf = NULL;
        }
    delete next;
#ifdef COUNTOBJECTS
    --COUNT;
#endif
    }

bool baseformpointer::hasDuplicateLemma(baseformpointer * startOfList, baseformpointer * current)
    {
    baseformpointer * bfp2 = startOfList;
    while (bfp2 != current && bfp2->bf->cmps(current->bf))
        {
        bfp2 = bfp2->next;
        }
    if (bfp2 == current)
        {
        if (current->hidden)
            {
            bfp2 = bfp2->next;
            while (bfp2 && (bfp2->hidden || bfp2->bf->cmps(current->bf)))
                {
                bfp2 = bfp2->next;
                }
            if (!bfp2) /*Lemma was not shown earlier.*/
                return false;
            }
        else
            return false;
        }
    return true;
    }

#if STREAM
void baseformpointer::printFn(ostream *fp,bfn Fn,const char * sep)
#else
void baseformpointer::printFn(FILE *fp,bfn Fn,const char * sep)
#endif
    {
    bool doSep = false;
    baseformpointer * bfp = this;
    while (bfp)
        {
        if(!bfp->hidden)
            {
            if (!hasDuplicateLemma(this, bfp))
                {
                if (doSep)
                    print(fp, sep);
                else
                    doSep = true;
                (bfp->bf->*Fn)();
                }
            }
        bfp = bfp->next;
        }
    if (UseLemmaFreqForDisambiguation == 1)
        {
        bfp = this;
        while (bfp)
            {
            if(bfp->hidden)
                {
                if (!hasDuplicateLemma(this, bfp))
                    {
                    if (doSep)
                        print(fp, sep);
                    else
                        doSep = true;
                    (bfp->bf->*Fn)();
                    }
                }
            bfp = bfp->next;
            }
        }
    }

#if STREAM
void baseformpointer::printfbf(ostream *fp,functionTree * fns,const char * sep)
#else
void baseformpointer::printfbf(FILE *fp,functionTree * fns,const char * sep)
#endif
    {
    if(fns)
        {
        bool doSep = false;
        baseformpointer * bfp = this;
        while (bfp)
            {
            if(!bfp->hidden)
                {
                if (!hasDuplicateLemma(this, bfp))
                    {
                    if (doSep)
                        print(fp, sep);
                    else
                        doSep = true;
                    fns->printIt(bfp->bf);
                    }
                }
            bfp = bfp->next;
            }
        if (UseLemmaFreqForDisambiguation == 1)
            {
            bfp = this;
            while (bfp)
                {
                if (bfp->hidden)
                    {
                    if (!hasDuplicateLemma(this, bfp))
                        {
                        if (doSep)
                            print(fp, sep);
                        else
                            doSep = true;
                        fns->printIt(bfp->bf);
                        }
                    }
                bfp = bfp->next;
                }
            }
        }
    }

#if PRINTRULE
#if STREAM
void baseformpointer::printfrule(ostream *fp, functionTree * fns, const char * sep)
#else
void baseformpointer::printfrule(FILE *fp, functionTree * fns, const char * sep)
#endif
{
if (fns)
    {
    bool doSep = false;
    baseformpointer * bfp = this;
    while (bfp)
        {
        if (!bfp->hidden)
            {
            if (!hasDuplicateLemma(this, bfp))
                {
                if (doSep)
                    print(fp, sep);
                else
                    doSep = true;
                fns->printIt(bfp->bf);
                }
            }
        bfp = bfp->next;
        }
    if (UseLemmaFreqForDisambiguation == 1)
        {
        bfp = this;
        while (bfp)
            {
            if (bfp->hidden)
                {
                if (!hasDuplicateLemma(this, bfp))
                    {
                    if (doSep)
                        print(fp, sep);
                    else
                        doSep = true;
                    fns->printIt(bfp->bf);
                    }
                }
            bfp = bfp->next;
            }
        }
    }
}

void baseformpointer::P()
    { 
    this->bf->P(); 
    }
void baseformpointer::R() 
    { 
    this->bf->R(); 
    }

#endif

void baseformpointer::reassign(basefrm * arg_bf)
    {
    owning = false;
    if(basefrm::hasW)
        arg_bf->addFullForms(this->bf);
    delete this->bf;
    this->bf = arg_bf;
    }

void baseformpointer::addFullForm(Word * word)
    {
    bf->addFullForm(word);
    if(next)
        next->addFullForm(word);
    }

void baseformpointer::DisambiguateByLemmaFrequency()
    {
    bool maxSeen = false;
    int maxfreq = 0;
    baseformpointer * p = this;
    for(;p;p = p->next)
        {
        int f = p->bf->lemmaFreq();
        if(f > maxfreq)
            {
            maxfreq = f;
            }
        }
    for(p = this;p;p = p->next)
        {
        int f = p->bf->lemmaFreq();
        if(f == maxfreq)
            {
            if (maxSeen)
                p->hidden = true;
            else
                maxSeen = true;
            }
        else
            {
            p->hidden = true;
            }
        }
    }

void baseformpointer::decFreq(Word * w)
    {
    for(baseformpointer * p = this;p;p = p->next)
        {
        if(p->hidden)
            {
            p->bf->removeFullForm(w); // remove element from full form list (may already be removed!)
            }
        }
    }

void baseformpointer::DisambiguateByTagFriends(const char * tag)
    {
    if(next)
        {
        int closeness = -1;
        baseformpointer * p = this;
        for(;p;p = p->next)
            {
            int f = p->bf->Closeness(tag);
            if(f >= 0 && (closeness < 0 || f < closeness))
                {
                closeness = f;
                }
            }
        for(p = this;p;p = p->next)
            {
            int f = p->bf->Closeness(tag);
            if(f > closeness) // The lower, the better!
                {
                p->hidden = true;
                }
            }
        }
    }

#if PRINTRULE
#if PFRQ || FREQ24
int baseformpointer::addBaseForm(const char * s, const char * t, const char * p,size_t len,/*int cnt,*/unsigned int frequency)
    {
    if (!bf->equal(s, t))
        {
        if (next)
            {
            return next->addBaseForm(s, t, p,len,/*cnt,*/frequency);
            }
        else
            {
            next = new baseformpointer(s, t, p,len,/*cnt,*/frequency);
            }
        return 1;
        }
    else
        return 0;
    }
#else
int baseformpointer::addBaseForm(const char * s, const char * t, size_t len)
    {
    if (!bf->equal(s, t))
        {
        if (next)
            {
            return next->addBaseForm(s, t,len);
            }
        else
            {
            next = new baseformpointer(s, t, len);
            }
        return 1;
        }
    else
        return 0;
    }
#endif
#else
#if PFRQ || FREQ24
int baseformpointer::addBaseForm(const char * s,const char * t,size_t len,/*int cnt,*/unsigned int frequency)
    {
    if(!bf->equal(s,t))
        {
        if(next)
            {
            return next->addBaseForm(s,t,len,/*cnt,*/frequency);
            }
        else
            {
            next = new baseformpointer(s,t,len,/*cnt,*/frequency);
            }
        return 1;
        }
    else
        return 0;
    }
#else
int baseformpointer::addBaseForm(const char * s,const char * t,size_t len) 
    {
    if(!bf->equal(s,t))
        {
        if(next)
            {
            return next->addBaseForm(s,t,len);
            }
        else
            {
            next = new baseformpointer(s,t,len);
            }
        return 1;
        }
    else
        return 0;
    }
#endif
#endif
#endif
