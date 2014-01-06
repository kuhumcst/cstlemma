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
#include "functiontree.h"
#if defined PROGLEMMATISE
#include "functio.h"
#include "comparison.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#if STREAM
#include <iostream>
#include <iomanip> 
using namespace std;
#endif


#ifdef COUNTOBJECTS
int functionTree::COUNT = 0;
#endif


functionTree::functionTree():m_fnc(NULL),next(NULL),child(NULL),m_comp(eany),m_nmbr(-1),Hidden(false)
    {
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }

functionTree::~functionTree()
    {
    delete m_fnc;
    delete next;
    delete child;
#ifdef COUNTOBJECTS
    --COUNT;
#endif
    }

void functionTree::printIt(const OutputClass * outputObj)const
    {
    if(!Hidden)
        {
        if(!skip(outputObj))
            {
            if(m_fnc)
                m_fnc->doIt(outputObj);
            if(child)
                {
                child->printIt(outputObj);
                }
            }
        }
    if(next)
        next->printIt(outputObj);
    }

int functionTree::count(const OutputClass * outputObj)const
    {
    int ret = -1;
    if(m_fnc)
        ret = m_fnc->count(outputObj);
    if(ret == -1 && child)
        ret = child->count(outputObj);
    if(ret == -1 && next)
        ret = next->count(outputObj);
    return ret;
    }

int functionTree::count2(const OutputClass * outputObj)const
    {
    int ret = -1;
    if(m_comp != eless && m_comp != eequal && m_comp != emore && m_comp != enotequal)
        {
        if(m_fnc)
            ret = m_fnc->count(outputObj);
        if(ret == -1 && child)
            {
            ret = child->count2(outputObj);
            }
        }
    if(ret == -1 && next)
        ret = next->count2(outputObj);
    return ret;
    }

bool functionTree::OK(const OutputClass * outputObj)const
    {
    int cnt = -1;
    switch(m_comp)
        {
        case eless:
        case eequal:
        case enotequal:
        case emore:
            if(child)
                {
                assert(child);
                cnt = child->count(outputObj);
                }
            else // $b2
                cnt = count(outputObj);
            if(cnt < 0)
                {
#if STREAM
                cout << "Something wrong in field specification.\n"
"The number-of-values specification " << m_nmbr << " is only valid if there is a field\n"
"with a variable number of values." << endl;
#else
                printf("Something wrong in field specification.\n"
"The number-of-values specification %d is only valid if there is a field\n"
"with a variable number of values.\n",m_nmbr);
#endif
                exit(0);
                }
        default:
            ;
        }
    switch(m_comp)
        {
        case eless:
            return cnt < m_nmbr;
        case eequal:
            return cnt == m_nmbr;
        case enotequal:
            return cnt != m_nmbr;
        case emore:
            return cnt > m_nmbr;
        default:
            return true;
        }
    }

bool functionTree::passTest(const OutputClass * outputObj)const
    {
    if(OK(outputObj))
        {
        if(m_comp != etest && child && !child->passTest(outputObj))
            return false;
        if(next)
            return next->passTest(outputObj);
        return true;
        }
    else
        return false;
    }

bool functionTree::skip(const OutputClass * outputObj)const
    {
    if(m_comp == etest) // bare [...] (no comparisons)
        // [...] tests for two situations:
        // 1) whether nested [...] have conditions that are not met and 
        // 2) whether anything countable that is not between [...]0 or [...]<1 returns zero
        // If either of these situations occur, the actions between [ and ] are skipped.
        // Notice that a [...] always succeeds, as does [...]* and ... (an expression without [ and ])
        {
        assert(!m_fnc);
        assert(child);
        return !child->passTest(outputObj) || child->count2(outputObj) == 0;
        }
    else 
        return !OK(outputObj);
    }
#endif