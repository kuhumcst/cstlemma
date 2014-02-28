/*
CSTLEMMA - trainable lemmatiser

Copyright (C) 2002, 2007  Center for Sprogteknologi, University of Copenhagen

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
#define CSTLEMMAVERSION "5.05"
#define CSTLEMMADATE "2014.02.28"
#define CSTLEMMACOPYRIGHT "2002-2014 Center for Sprogteknologi"

#include "lemmatiser.h"
#include "option.h"
#if defined PROGLEMMATISE
#include "word.h"
#include "caseconv.h"
#endif
#if STREAM
# if defined __BORLANDC__
#  include <strstrea.h>
# else
#  ifdef __GNUG__
#   if __GNUG__ > 2
#    include <sstream>
#   else
#    include <strstream.h>
#   endif
#  else
#   include <sstream>
#  endif
# endif
# ifndef __BORLANDC__
using namespace std;
# endif
#endif

#include <stdio.h>
#include <stddef.h>

int main(int argc, char * argv[])
    {
    if(argc == 1)
        {
        LOG1LINE("");
        LOG1LINE("CSTLEMMA version " CSTLEMMAVERSION " (" CSTLEMMADATE ")");
        LOG1LINE("Copyright (C) " CSTLEMMACOPYRIGHT);
        if(sizeof(ptrdiff_t) == 8)
            LOG1LINE("64-bit");
        else
            LOG1LINE("32-bit");
// GNU >> 
        LOG1LINE("CSTLEMMA comes with ABSOLUTELY NO WARRANTY; for details use option -w.");
        LOG1LINE("This is free software, and you are welcome to redistribute it under");
        LOG1LINE("certain conditions; use option -r for details.");
        LOG1LINE("");
        LOG1LINE("");
// << GNU
        LOG1LINE("Use option -h for usage.");
        return 0;
        }

    optionStruct Option;
    int ret;

    OptReturnTp optResult = Option.readArgs(argc,argv);
    if(optResult == Error)
        return 1;

    if(optResult == Leave)
        { // option -r, -w, -? or -h
        return 0;
        }

    Lemmatiser theLemmatiser(Option);
    if((ret = theLemmatiser.getStatus()) == 0)
        {
        switch(Option.whattodo)
            {
            case MAKEDICT:
                {
                break;
                }
            case MAKEFLEXPATTERNS:
                {
                break;
                }
            default:
                {
#if defined PROGLEMMATISE
                ret = theLemmatiser.LemmatiseFile();
#endif
                }
            }
        }
#if defined PROGLEMMATISE
    Word::deleteStaticMembers();
#endif
    return ret;
    }
