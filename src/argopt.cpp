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
/* argopt.cpp */

#include "argopt.h"
#include <string.h>

char *myoptarg;
int myoptind = 0;

int getopt(int argc,char *argv[],char *opts)
    {
    static char emptystr[] = "";
    char *index;
    int optc;
    
    if (!myoptind)    /* argv[0] points to the command verb */
        ++myoptind;
    if (myoptind >= argc)
        {
        myoptarg = NULL;
        return -1;
        }
    
    if ((index = argv[myoptind]) != NULL)
        {
        char * optpos;
        if (*index != '-' && *index != '/')
            {
            /* no option, perhaps something else ? */
            myoptarg = NULL;
            return -1;
            }
        if (*(++index) == '-')
            {
            ++myoptind;            /* double --,  end of options */
            myoptarg = NULL;
            return -1;
            }
        if (!*index)
            {
                                /* single -, probably not an option */
            myoptarg = NULL;
            return -1;
            }
        optc = *index;       /* option letter */
        optpos = strchr(opts,optc);
        if(optpos)
            {
            if(optpos[1] == ':')
                {
                /* this option has always data */
                if(!*++index)
                    {
                    /* try next argument */
                    for (;++myoptind < argc && !*argv[myoptind];);
                    if(  myoptind == argc
                      || argv[myoptind] == NULL
                      || (  *argv[myoptind] == '-'
                         && *(argv[myoptind]+1) != '\0'
                         )
                      )
                        {
                        myoptarg = emptystr;
                        return optc;  /* no data after all */
                        }
                    else
                        {
                        myoptarg = argv[myoptind++]; 
                        if(myoptarg[strlen(myoptarg) - 1] == '\r') // Last argument has trailing '\r' under Linux !
                            myoptarg[strlen(myoptarg) - 1] = '\0';
                        return optc;
                        }
                    }
                else
                    {
                    myoptind++;
                    myoptarg = index; 
                    if(myoptarg[strlen(myoptarg) - 1] == '\r') // Last argument has trailing '\r' under Linux !
                        myoptarg[strlen(myoptarg) - 1] = '\0';
                    return optc;
                    }
                }
            else
                {
                myoptind++;
                myoptarg = NULL; 
                return optc;
                }
            }
        else
            {
            myoptind++;
            myoptarg = NULL; 
            return -1;
            }
        }
    else
        {
        myoptind++;
        myoptarg = NULL;
        return -1;
        }
    }



