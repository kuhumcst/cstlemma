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
//-L -eU -p+ -q- -t- -U- -H2 -fD:\projects\tvarsok\ru\rules_0utf8.lem -dD:\projects\cstlemmares.ru\dictrus -B$w -l- -c$w/$B$s -i D:\dokumenter\russisk\oetsjastiee.txt -o D:\dokumenter\russisk\oetsjastiee.lemmatised.txt -m0
//-L -eU -p+ -q- -t- -U- -H2 -fD:\projects\tvarsok\ru\rules_0utf8.lem -B$w -l- -c$w/$B$s -i D:\dokumenter\russisk\oetsjastiee.txt -o D:\dokumenter\russisk\oetsjastiee.lemmatised.txt -m0
//-L -e0 -p+ -q- -t- -U- -H2 -fD:\projects\tvarsok\ru\rules_0utf8.lem -B$w -l- -b$w -dD:\projects\cstlemmares.ru\dictrus -u- -c$i$w/$b1[[$b~1]?$B]$s -i D:\dokumenter\russisk\oetsjastiee.txt -o D:\dokumenter\russisk\oetsjastiee.lemmatised.txt -m0
#define CSTLEMMAVERSION "4.7"
#define CSTLEMMADATE "2012.09.07"
#define CSTLEMMACOPYRIGHT "2002-2012 Center for Sprogteknologi"

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
        printf("\n");
        printf("CSTLEMMA version " CSTLEMMAVERSION " (" CSTLEMMADATE ")\n");
        printf("Copyright (C) " CSTLEMMACOPYRIGHT "\n");
        if(sizeof(ptrdiff_t) == 8)
            printf("64-bit\n");
        else
            printf("32-bit\n");
// GNU >> 
        printf("CSTLEMMA comes with ABSOLUTELY NO WARRANTY; for details use option -w.\n");
        printf("This is free software, and you are welcome to redistribute it under\n");
        printf("certain conditions; use option -r for details.\n");
        printf("\n\n");
// << GNU
        printf("Use option -h for usage.\n");
        return 0;
        }

    optionStruct Option;
    int ret;

    //setEncoding(1);


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
#define BATCH 1
#if BATCH || !STREAM
                ret = theLemmatiser.LemmatiseFile();
#else
                while(true)
                    {
                    char line[256];
                    cin.getline(line,sizeof(line),'\n');
                    if(!line[0])
                        break;
#if defined __BORLANDC__ || defined __GNUG__ && __GNUG__ < 3
                    strstream * str = new strstream;
                    *str << line;
                    strstream str2;
#else
                    stringstream * str = new stringstream;
                    *str << line;
                    stringstream str2;
#endif
                    theLemmatiser.LemmatiseText(str,&str2,NULL);
/*
                    theLemmatiser.setSep(" AND ");
                    theLemmatiser.setDictUnique(false);
#if defined __BORLANDC__ || defined __GNUG__ && __GNUG__ < 3
                    str = new strstream;
#else
                    str = new stringstream;
#endif
                    *str << line;
                    str2 << endl << endl;
                    theLemmatiser.LemmatiseText(str,&str2,NULL);
*/
                    str2 << '\0';
#if defined __BORLANDC__ || defined __GNUG__ && __GNUG__ < 3
                    if(str2.str())
#endif
                        cout << str2.str();
                    }
#endif
#endif
                }
            }
        }
#if defined PROGLEMMATISE
    Word::deleteStaticMembers(); // Bart 20050916: memory leak. 
#endif
    return ret;
    }
