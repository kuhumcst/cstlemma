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
#include "defines.h"
#if defined PROGLEMMATISE

class field;
class XMLtext;
class wordReader
    {
    private:
        char * p;
        char * buf;
        field * format;
        field * nextfield;
        field * wordfield;
        field * tagfield;
        unsigned long newlines;
        unsigned long lineno;
        const char * tag;
        int kar;
        int lastkar;
        bool treatSlashAsAlternativesSeparator;
        XMLtext * Text;
        char kars[2];
    public:
        int Put(bool (wordReader::*)(int kar),int kar);
        int (wordReader::*xput)(bool (wordReader::*)(int kar),int kar);
        int rawput(bool (wordReader::*)(int kar),int kar);
        int nrawput(bool (wordReader::*)(int kar),char * c);
        int charref(bool (wordReader::*)(int kar),int kar);
        unsigned long getNewlines()
            {
            return newlines;
            }
        unsigned long getLineno()
            {
            return lineno;
            }
        void initWord();
        wordReader(field * format,field * wordfield,field * tagfield,bool treatSlashAsAlternativesSeparator,XMLtext * Text);
        ~wordReader();
        CHAR * readChar(int kar);
        bool countToken(int kar);
        bool readToken(int kar);
    };
#endif