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
#ifndef FUNCTIONTREE_H
#define FUNCTIONTREE_H

#include "defines.h"
#if defined PROGLEMMATISE

#include "comparison.h"

class function;
class OutputClass;
//typedef enum comparison;

class functionTree
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        function * m_fnc;
        functionTree * next;
        functionTree * child;
        comparison m_comp;
        int m_nmbr;      // number of OutputClass elements (success/failure criterion)
        bool Hidden;
    public:
        /*
        void print()
            {
            if(fnc)
                printf("F");
            if(child)
                {
                printf("[");
                child->print();
                printf("]");
                switch(comp)
                    {
                    case less:printf("<%d",nmbr);break;
                    case equal:printf("%d",nmbr);break;
                    case notequal:printf("~%d",nmbr);break;
                    case more:printf(">%d",nmbr);break;
                    case any:printf("*");break;
                    case test:printf("");break;
                    }
                }
            if(next)
                next->print();
            }
            */
        functionTree();
        ~functionTree();
        void hide(){Hidden=true;}
        bool passTest(const OutputClass * outputObj)const;
        bool OK(const OutputClass * outputObj)const;
        void printIt(const OutputClass * outputObj)const;
        int count(const OutputClass * outputObj)const;
        int count2(const OutputClass * outputObj)const;
        void setFunction(function * fnc)
            {
            this->m_fnc = fnc;
            }
        functionTree & addChild()
            {
            child = new functionTree();
            return *child;
            }
        functionTree & addNext()
            {
            next = new functionTree();
            return *next;
            }
        void setComp(comparison comp)
            {
            this->m_comp = comp;
            }
        void setNmbr(int nmbr)
            {
            this->m_nmbr = nmbr;
            }
        bool skip(const OutputClass * outputObj)const;
    };

#endif
#endif