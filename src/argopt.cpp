/*
CSTLEMMA - trainable lemmatiser

Copyright (C) 2002, 2005  Center for Sprogteknologi, University of Copenhagen

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

char *optarg;
int optind = 0;

int getopt(int argc,char *argv[],char *opts)
    {
    static char emptystr[] = "";
    char *index/* = NULL*/;
    int optc/* = -1*/;
    
    if (!optind)    /* argv[0] points to the command verb */
        ++optind;
    if (optind >= argc)
        {
        optarg = NULL;
        return -1;
        }
    
    if ((index = argv[optind]) != NULL)
        {
        char * optpos;
        if (*index != '-' && *index != '/')
            {
            /* no option, perhaps something else ? */
            optarg = NULL;
            return -1;
            }
        if (*(++index) == '-')
            {
            ++optind;            /* double --,  end of options */
            optarg = NULL;
            return -1;
            }
        if (!*index)
            {
                                /* single -, probably not an option */
            optarg = NULL;
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
                    for (;++optind < argc && !*argv[optind];);
                    if(  optind == argc
                      || argv[optind] == NULL
                      || (  *argv[optind] == '-'
                         && *(argv[optind]+1) != '\0'
                         )
                      )
                        {
                        optarg = emptystr;
                        return optc;  /* no data after all */
                        }
                    else
                        {
                        optarg = argv[optind++]; 
                        if(optarg[strlen(optarg) - 1] == '\r') // Bart 20030806 Last argument has trailing '\r' under Linux !
                            optarg[strlen(optarg) - 1] = '\0';
                        return optc;
                        }
                    }
                else
                    {
                    optind++;
                    optarg = index; 
                    if(optarg[strlen(optarg) - 1] == '\r') // Bart 20030806 Last argument has trailing '\r' under Linux !
                        optarg[strlen(optarg) - 1] = '\0';
                    return optc;
                    }
                }
            else
                {
                optind++;
                optarg = NULL; 
                return optc;
                }
            }
        else
            {
            optind++;
            optarg = NULL; 
            return -1;
            }
        }
    else
        {
        optind++;
        optarg = NULL;
        return -1;
        }
    }



