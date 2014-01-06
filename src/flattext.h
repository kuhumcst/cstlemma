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
#ifndef FLATTEXT_H
#define FLATTEXT_H

#include "text.h"
#if defined PROGLEMMATISE

class flattext : public text
    {
    private:
/*
        unsigned long int * Lines;
        unsigned long int lineno;
*/
    public:
        flattext(
#if STREAM
            istream * fpi
#else
            FILE * fpi
#endif
            ,bool InputHasTags
            ,char * Iformat
            ,int keepPunctuation
            ,bool nice
            ,unsigned long int size
            ,bool treatSlashAsAlternativeSeparator
            /*
            ,bool XML
            ,const char * ancestor // if not null, restrict lemmatisation to elements that are offspring of ancestor
            ,const char * element // if null, analyse all PCDATA that is text
            ,const char * wordAttribute // if null, word is PCDATA
            ,const char * POSAttribute // if null, POS is PCDATA
            ,const char * lemmaAttribute // if null, Lemma is PCDATA
            ,const char * lemmaClassAttribute // if null, lemma class is PCDATA
            */
            );
        ~flattext(){}
        
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