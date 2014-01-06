/*
CSTLEMMA - trainable lemmatiser

Copyright (C) 2002, 2014, 2009  Center for Sprogteknologi, University of Copenhagen

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

#include "dictionary.h"
#if defined PROGLEMMATISE

#include "utf8func.h"
#include "caseconv.h"
#include <string.h>
#include <stdlib.h>

#ifdef COUNTOBJECTS
int dictionary::COUNT = 0;
#endif

static bool staticUTF8 = true; // Initial assumption. If the data are not UTF-8, reading continues byte-wise.

typedef int tchildrencount; // type for variables that are optimal for counting
                            // small numbers, but the value of which eventually
                            // will be typecasted to tchildren.


struct Nodes
    {
    tcount nnodes; // number of nodes = number of elements in initialchars, 
                   // strings and children
    tchildren ntoplevel; // number of nodes at top level (for other levels, 
                         // this number is given by numberOfChildren)
    int * initialchars;  // (optimization) string with all first characters. 
                         //     (Not part of the dictionary file!)
                         // First ntoplevel characters for each of the 
                         // ntoplevel nodes. May contain zero bytes!
                         // If first character of candidate string isn't in
                         // stretch of initialchars, then the candidate string
                         // cannot be matched.
                         // If first character of candidate string is in 
                         // stretch of initialchars, then the candidate string
                         // must be compared (strcmp) with precisely one 
                         // string.
                         // Position of string to compare with can be computed
                         // from position of character in initialchars 
    char ** strings; // array of strings. First ntoplevel strings are for 
                     // each of the ntoplevel nodes.
                     // Full forms are encoded by stringing together the 
                     // appropriate *strings needed to reach the final 
                     // 'lext' structure.
    tchildren * numberOfChildren; // If node is a leaf, numberOfChildren 
                                  // denotes number of consecutive elements in
                                  // LEXT, otherwise its meaning is analogous
                                  // to ntoplevel
    tindex * pos; // zero or positive: index into lext array (LEXT)
                  // negative: inverse of index into initialchars, strings,
                  // numberOfChildren and pos
    };

static char * STRINGS;
static char * STRINGS1; // STRINGS1 = STRINGS + 1
lext * LEXT;
static Nodes NODES;
static char EMPTY[] = "";

bool dictionary::initdict(FILE * fpin)
    {
    if(fpin)
        {
        return readStrings(fpin) && readLeaves(fpin) && readNodes(fpin);
        }
    return false;
    }

dictionary::dictionary()
    {
    NODES.ntoplevel = 0;
#ifdef COUNTOBJECTS
    ++COUNT = 0;
#endif
    }

dictionary::~dictionary()
    {
    cleanup();
#ifdef COUNTOBJECTS
    --COUNT = 0;
#endif
    }

void dictionary::printall(FILE * fp)
    {
    for(tchildrencount i = 0;i < NODES.ntoplevel;++i)
        {
        printnode(0,i,fp);
        }
    }

void dictionary::printall2(FILE * fp)
    {
    for(tchildrencount i = 0;i < NODES.ntoplevel;++i)
        {
        printnode2(EMPTY,i,fp);
        }
    }

bool dictionary::findword(const char * word,tcount & Pos,int & Nmbr)
    {
    if(findwordSub(word,Pos,Nmbr))
        {
        return true;
        }
    else if(is_Upper(word))
        return findwordSub(allToLower(word),Pos,Nmbr);
    else
        return false;
    }

bool dictionary::findwordSub(const char * word,tcount & Pos,int & Nmbr)
    {
    int kar = UTF8char(word,staticUTF8);
    const char * w = word;
    int nmbr = NODES.ntoplevel;
    tcount pos = 0;
    while(nmbr > 0)
        {
        int kar2 = NODES.initialchars[pos];
        if(kar2 < kar)
            {
            ++pos;
            --nmbr;
            }
        else if(kar2 == kar)
            {
            if(kar)
                {
                ptrdiff_t p,q;
                char * s = NODES.strings[pos];
                strcmpN(s,w,p,q);
                if(s[p])
                    return false;
                w += q;
                }
            nmbr = NODES.numberOfChildren[pos];
            pos = NODES.pos[pos];
            if(pos < 0) // not a leaf, descend further
                {
                pos = -pos; // Make it a valid index.
                kar = UTF8char(w,staticUTF8);
                }
            else if(*w && *++w)
                {
                return false;
                }
            else // This is a leaf. Do the baseform and type stuff.
                {
                Pos = pos;
                Nmbr = nmbr;
                return true;
                }
            }
        else // Initial character alphabetically greater than any of the
             // available candidates.
            {
            return false;
            }
        }
    return false;
    }


/*
Structure of dictionary file.
# indicates that a number is read from the file (binary,non portable!)
Read operations are indicated by {}.
Repeated read operations are idicated by {}*(n), where n indicates the number of repetitions

FILE     = {STRINGS1}{LEXT}{NODES}
STRINGS1 = {#STRINGS1}{STRINGS1}

*/

bool dictionary::readStrings(FILE * fp)
    {
    tlength stringBufLen;
    if(fread(&stringBufLen,sizeof(stringBufLen),1,fp) == 1)
        {
        STRINGS = new char[stringBufLen+1];
        STRINGS[0] = '\0';
        STRINGS1 = STRINGS + 1;
        return fread(STRINGS1,stringBufLen,1,fp) == 1;
        }
    return false;
    }

/*
LEXT                   = {#LEXT}{LEXT[0]LEXT[1]...LEXT[#LEXT-1]}

where

LEXT[i]                = {#TypeIndex}{#BaseFormSuffixIndex}{#Offset}

such that

LEXT[i].Type           = STRINGS1[TypeIndex-1]
LEXT[i].BaseFormSuffix = STRINGS1[BaseFormSuffixIndex-1]
*/
bool dictionary::readLeaves(FILE * fp)
    {
    tcount leafBufLen;
    int readcount = 0;
    if(fread(&leafBufLen,sizeof(leafBufLen),1,fp) == 1)
        {
        LEXT = new lext[leafBufLen];
        for(tcount i = 0;i < leafBufLen;++i)
            {
            tindex tmp;
            if(fread(&tmp,sizeof(tmp),1,fp) == 1)
                {
                LEXT[i].Type = tmp + STRINGS;
                if(fread(&tmp,sizeof(tmp),1,fp) == 1)
                    {
                    LEXT[i].BaseFormSuffix = tmp + STRINGS;
                    if(fread(&LEXT[i].S,sizeof(LEXT[i].S),1,fp) == 1)
                        {
                        ++readcount;
                        }
                    else
                        {
                        fprintf(stderr,"Function dictionary::readLeaves returns false.\n");
                        return false;
                        }
                    }
                }
            }
        return readcount == leafBufLen;
        }
    return false;
    }

/*
NODES = {#nodeBufLen}{#ntoplevel}NODE*(ntoplevel)
NODE  = {#stringsIndex}{#numberOfChildren}{#pos}NODE*(numberOfChildren)

where N is ntoplevel for the first NODE and numberOfChildren for child NODES.
The tree is built in depth-first fashion.
*/
tcount dictionary::readStretch(tchildren length,tcount pos,FILE * fp)
    {
    tchildrencount i;
    for(i = 0;i < length;++i)
        {
        tindex tmp;
        if(  fread(&tmp,sizeof(tmp),1,fp) != 1
          || fread(&NODES.numberOfChildren[pos + i],sizeof(NODES.numberOfChildren[pos + i]),1,fp) != 1
          || fread(&NODES.pos[pos + i],sizeof(NODES.pos[pos + i]),1,fp) != 1
          )
            return 0; // error!
        NODES.strings[pos + i] = STRINGS + tmp;
        }
    tcount curr = pos + length;
    for(i = 0;i < length;++i)
        {
        if(NODES.pos[pos + i] < 0)
            {
            NODES.pos[pos + i] = -curr;
            curr = readStretch(NODES.numberOfChildren[pos + i],curr,fp);
            }
        }
    return curr;
    }

bool dictionary::readNodes(FILE * fp)
    {
    tcount nodeBufLen;
    if(fread(&nodeBufLen,sizeof(nodeBufLen),1,fp) == 1)
        {
        NODES.nnodes = nodeBufLen;
        NODES.initialchars = new int[nodeBufLen];
        NODES.strings = new char * [nodeBufLen];
        NODES.numberOfChildren = new tchildren[nodeBufLen];
        NODES.pos = new tindex[nodeBufLen];
        tchildren length;
        if(fread(&length,sizeof(length),1,fp) == 1)
            {
            NODES.ntoplevel = length;
            readStretch(NODES.ntoplevel,0,fp);
            for(tcount i = 0;i < nodeBufLen;++i)
                {
                NODES.initialchars[i] = UTF8char(NODES.strings[i],staticUTF8);
                }
            }
        return true;
        }
    return false;
    }

void dictionary::cleanup()
    {
    delete [] STRINGS;
    delete [] LEXT;
    delete [] NODES.initialchars;
    delete [] NODES.strings;
    delete [] NODES.numberOfChildren;
    delete [] NODES.pos;
    }

void dictionary::printlex(tindex pos, FILE * fp)
    {
    fprintf(fp,"%s %s %d %d",LEXT[pos].BaseFormSuffix,LEXT[pos].Type,LEXT[pos].S.Offset,LEXT[pos].S.frequency);
    }

void dictionary::printlex2(char * head,tindex pos, FILE * fp)
    {
    fprintf(fp,"%.*s%s/%s %d",(int)LEXT[pos].S.Offset,head,LEXT[pos].BaseFormSuffix,LEXT[pos].Type,LEXT[pos].S.frequency);
    }

void dictionary::printnode(size_t indent, tindex pos, FILE * fp)
    {
    tchildren n = NODES.numberOfChildren[pos];
    tchildrencount i;
    for(size_t j = indent;j;--j)
        fputc(' ',fp);
    fprintf(fp,"%s",NODES.strings[pos]);
    if(NODES.pos[pos] < 0)
        {
        fprintf(fp,"\n");
        for(i = 0;i < n;++i)
            {
            printnode(indent + 2,i - NODES.pos[pos],fp);
            }
        }
    else
        {
        fprintf(fp,"(");
        for(i = 0;i < n;++i)
            {
            printlex(NODES.pos[pos] + i,fp);
            if(i < NODES.numberOfChildren[pos] - 1)
                fprintf(fp,",");
            }
        fprintf(fp,")\n");
        }
    }

void dictionary::printnode2(char * head, tindex pos, FILE * fp)
    {
    size_t len = strlen(head);
    strcpy(head+len,NODES.strings[pos]);
    tchildren n = NODES.numberOfChildren[pos];
    tchildrencount i;
    if(NODES.pos[pos] < 0)
        {
        for(i = 0;i < n;++i)
            {
            printnode2(head,i - NODES.pos[pos],fp);
            }
        }
    else
        {
        fprintf(fp,"%s\t(",head);
        for(i = 0;i < n;++i)
            {
            printlex2(head,NODES.pos[pos] + i,fp);
            if(i < NODES.numberOfChildren[pos] - 1)
                fprintf(fp,",");
            }
        fprintf(fp,")\n");
        }
    head[len] = '\0';
    }
#endif