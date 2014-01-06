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
baseformpointer::baseformpointer(const char * s,const char * t,size_t len,/*int cnt,*/unsigned int frequency):owning(true),next(NULL),hidden(false)
    {
    bf = new basefrm(s,t,*this,len,/*cnt,*/frequency);
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }
#else
#if PFRQ
baseformpointer::baseformpointer(const char * s,const char * t,size_t len,/*int cnt,*/unsigned int frequency)
        :owning(true),next(NULL),hidden(false),pfrq(frequency)
    {
    bf = new basefrm(s,t,*this,len);
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }
#else
baseformpointer::baseformpointer(const char * s,const char * t,size_t len):owning(true),next(NULL),hidden(false)
    {
    bf = new basefrm(s,t,*this,len);
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

#if STREAM
void baseformpointer::printFn(ostream *fp,bfn Fn,const char * sep)
#else
void baseformpointer::printFn(FILE *fp,bfn Fn,const char * sep)
#endif
    {
        bool doSep = false;
        baseformpointer * bfp = this;
        while(bfp)
            {
            while(bfp && bfp->hidden)
                {
                if(UseLemmaFreqForDisambiguation == 1)
                    {
                    if(doSep)
                        print(fp,sep);
                    else
                        doSep = true;
                    print(fp,"<<");
                    (bfp->bf->*Fn)();
                    print(fp,">>");
                    }
                bfp = bfp->next;
                }
            while(bfp && !bfp->hidden)
                {
                if(doSep)
                    print(fp,sep);
                else
                    doSep = true;
                (bfp->bf->*Fn)();
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
        while(bfp)
            {
            while(bfp && bfp->hidden)
                {
                if(UseLemmaFreqForDisambiguation == 1)
                    {
                    if(doSep)
                        print(fp,sep);
                    else
                        doSep = true;
                    print(fp,"<<");
                    fns->printIt(bfp->bf);
                    print(fp,">>");
                    }
                bfp = bfp->next;
                }
            while(bfp && !bfp->hidden)
                {
                if(doSep)
                    print(fp,sep);
                else
                    doSep = true;
                fns->printIt(bfp->bf);
                bfp = bfp->next;
                }
            }
        }
    }


/*
    while(hidden && next)
        next->
    if(fns)
        {
        if(hidden)
            {
            if(UseLemmaFreqForDisambiguation == 1)
                {
                fprintf(fp,"%s","<<");
                fns->printIt(bf);
                fprintf(fp,"%s",">>");
                }
            }
        else 
            {
            fns->printIt(bf);
            }
        }
    baseformpointer * nxt = next;
    while(nxt && nxt->hidden)
        {
        nxt = nxt->next;
        }
    if(nxt)
        {
        fprintf(fp,"%s",sep);
        next->printfbf(fp,fns,sep);
        }
    }
*/

void baseformpointer::reassign(basefrm * arg_bf)
    {
    owning = false;
//    bf->inc(this->bf);
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

void baseformpointer::DissambiguateByLemmaFrequency()
    {
    int maxfreq = 0;
    baseformpointer * p = this;
//    baseformpointer * q = NULL;
    for(;p;p = p->next)
        {
        int f = p->bf->lemmaFreq();
        if(f > maxfreq)
            {
            maxfreq = f;
//            q = p;
            }
        }
    for(p = this;p;p = p->next)
        {
        int f = p->bf->lemmaFreq();
        if(f < maxfreq)
            {
//            maxfreq = f;
            p->hidden = true;
//            printf("DissambiguateByLemmaFrequency-hide %d<%d:",f,maxfreq);p->bf->testPrint();q->bf->testPrint();
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

void baseformpointer::DissambiguateByTagFriends(const char * tag)
    {
    if(next)
        {
        int closeness = -1;
        baseformpointer * p = this;
//        baseformpointer * q;
        for(;p;p = p->next)
            {
            int f = p->bf->Closeness(tag);
            if(f >= 0 && (closeness < 0 || f < closeness))
                {
                closeness = f;
//                q = p;
                }
            }
        for(p = this;p;p = p->next)
            {
            int f = p->bf->Closeness(tag);
            if(f > closeness) // The lower, the better!
                {
                p->hidden = true;
//                printf("DissambiguateByTagFriends-hide:%s",tag);p->bf->testPrint();q->bf->testPrint();
                }
            }
        }
    }

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
//        printf("Equal!");
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
//        printf("Equal!");
    }
#endif
#endif