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
#ifndef LEXT_H
#define LEXT_H

#include "defines.h"
#if defined PROGLEMMATISE

#include "lem.h"
/*
Dictionary entry. Or rather, the type of the full form's lemma and the data
needed to construct the lemma from the full form, which are an offset to the
'tail' of the full form and a suffix that replaces this 'tail'.

The full form itself is not in this structure, but encoded in the Nodes that
are traversed to reach the lext-object, starting at the top Nodes-object.
*/
struct lext
    {
#ifdef COUNTOBJECTS
    public:
    static int COUNT;
    lext()
        {
        ++COUNT;
        }
    ~lext()
        {
        --COUNT;
        }
#endif
    char * Type;
    char * BaseFormSuffix;
    tsundry S;
    const char * constructBaseform(const char * fullform) const;
    /*
    Construct lemma by taking the first Offset characters from fullform and 
    appending BaseFormSuffix.
    */
    };

#endif
#endif