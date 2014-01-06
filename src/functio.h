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
#ifndef FUNCTION_H
#define FUNCTION_H

#include "defines.h"
#if defined PROGLEMMATISE

#if STREAM
# include <iostream>
# ifndef __BORLANDC__	 // Borland 5.02
using namespace std;
# endif
#else
# include <stdio.h>
#endif

class OutputClass;
class basefrm;
class Word;
class taggedWord;

typedef void (basefrm::*fptrbf)() const;
typedef void (Word::*fptr)() const;
typedef void (taggedWord::*fptrt)() const;
typedef int (Word::*fptcount)() const;
typedef int (basefrm::*fptcountbf)() const;

class function
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
    public:
        virtual void doIt(const OutputClass * outputObj)const = 0;
        virtual int count(const OutputClass * outputObj)const{REFER(outputObj) return -1;}
#ifdef COUNTOBJECTS
        function()
            {
            ++COUNT;
            }
#endif
        virtual ~function()
            {
#ifdef COUNTOBJECTS
            --COUNT;
#endif
            }
        virtual bool skip(const basefrm * bf)const = 0;
    };


class functionNoArgB : public function
    {
    private:
        fptrbf m_fn;
    public:
        functionNoArgB(fptrbf fn):m_fn(fn){}
        void doIt(const OutputClass * outputObj)const
            {
            (((const basefrm*)outputObj)->*m_fn)();
            }
        virtual bool skip(const basefrm * bf)const
            {
            REFER(bf)
            return false;
            }
    };

class functionString : public function
    {
    private:
        char * arg;
    public:
#if STREAM
        static ostream * fp;
#else
        static FILE * fp;
#endif
        functionString(char * arg);
        ~functionString()
            {
            delete [] arg;
            }
        void doIt(const OutputClass * outputObj)const
            {
            REFER(outputObj)
#if STREAM
            *fp << arg;
#else
            for(char * s = arg;*s;++s)
                fputc(*s,fp);
#endif
            }
        virtual bool skip(const basefrm * bf)const
            {
            REFER(bf)
            return false;
            }
    };

class functionNoArg : public function
    {
    private:
        fptr m_fn;
        fptcount m_fncount;
    public:
        functionNoArg(fptr fn,fptcount fncount):m_fn(fn),m_fncount(fncount){}
        void doIt(const OutputClass  * u)const
            {
            const Word * tmp = (const Word*)u;
            (tmp->*m_fn)();
            }
        virtual bool skip(const basefrm * bf)const
            {
            REFER(bf)
            return false;
            }
        int count(const OutputClass  * u)const
            {
            if(m_fncount)
                {
                const Word * tmp = (const Word*)u;
                return (tmp->*m_fncount)();
                }
            else
                return function::count(u);
            }
    };

class functionNoArgW : public function
    {
    private:
        fptrbf m_fn;
        fptcountbf m_fncount;
    public:
        functionNoArgW(fptrbf fn,fptcountbf fncount):m_fn(fn),m_fncount(fncount){}
        void doIt(const OutputClass  * u)const
            {
            const basefrm * tmp = (const basefrm*)u;
            (tmp->*m_fn)();
            }
        virtual bool skip(const basefrm * bf)const
            {
            REFER(bf)
            return false;
            }
        int count(const OutputClass * u)const
            {
            if(m_fncount)
                {
                const basefrm * tmp = (const basefrm*)u;
                return (tmp->*m_fncount)();
                }
            else
                return function::count(u);
            }
    };


class functionNoArgT : public function
    {
    private:
        fptrt m_fn;
    public:
        functionNoArgT(fptrt fn):m_fn(fn){}
        void doIt(const OutputClass  * u)const
            {
            ((const taggedWord*)u->*m_fn)();
            }
        virtual bool skip(const basefrm * bf)const
            {
            REFER(bf)
            return false;
            }
    };


#endif
#endif