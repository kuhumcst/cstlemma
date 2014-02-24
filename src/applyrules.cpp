/*
CSTLEMMA - trainable lemmatiser

Copyright (C) 2002, 2008  Center for Sprogteknologi, University of Copenhagen

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

#include "applyrules.h"
#if defined PROGLEMMATISE

#include "hashmap.h"
#include "flex.h"
#include "utf8func.h"
#include "caseconv.h"
#include "option.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#if STREAM
#include <iostream>
using namespace std;
#endif


bool oneAnswer = false;

struct var
    {
    const char * s;
    const char * e;
    };

static const char * flexFileName;
static char bufbuf[] = "\0\0\0\0\t\t\t\n"; //20090811: corrected wrong value // "lemma == word" default rule set
static char * buf = bufbuf; // Setting buf directly to a constant string generates a warning in newer gcc
static long end = 8;
static char * result = 0;
#define TESTING 0
#if TESTING
static char * replacement = 0; // FOR TEST PURPOSE
#endif
static bool NewStyle = true;

void setNewStyleRules(bool val)
    {
    NewStyle = val;
    }

static char * readRules(FILE * flexrulefile,long & end)
    {
    if(flexrulefile)
        {
        int start;
        if(fread(&start,sizeof(int),1,flexrulefile) != 1)
            return 0;
        if(start != 0)
            {
            return 0; // not the new format
            }
        fseek(flexrulefile,0,SEEK_END);
        end = ftell(flexrulefile);
        char * buf = new char[end+1]; // 20140224 +1
        if(buf && end > 0)
            {
            rewind(flexrulefile);
            if(fread(buf,1,end,flexrulefile) != (size_t)end)
                return 0; // 20120710
            NewStyle = true;
            buf[end] = '\0';// 20140224 new
            }
        return buf;
        }
    return 0;
    }

class rules
    {
    private:
        char * TagName;
        char * Buf;
        long End;
    public:
        rules(const char * TagName)
            {
            this->TagName = new char[strlen(TagName)+1];
            strcpy(this->TagName,TagName);
            char * filename = new char[strlen(TagName)+strlen(flexFileName)+2];
            sprintf(filename,"%s.%s",flexFileName,TagName);
            FILE * f = fopen(filename,"rb");
            if(f)
                {
                Buf = readRules(f, End);
                fclose(f);
                }
            else
                {
                Buf = 0;
                End = 0;
                }
            }
        const char * tagName() const {return TagName;}
        const char * buf(){return Buf;}
        long end(){return End;}
        void print(){}
    };

static hash<rules> * Hash = NULL;


bool newStyleRules()
    {
    return NewStyle;
    }

static const char * samestart(const char ** fields,const char * s,const char * we)
    {
    const char * f = fields[0];
    const char * e = fields[1]-1;
    while((f < e) && (s < we) && (*f == *s))
        {
        ++f;
        ++s;
        }
    // On success: return pointer to first unparsed character
    // On failure: return 0
    return f == e ? s : 0;
    }

static const char * sameend(const char ** fields,const char * s,const char * wordend)
    {
    const char * f = fields[2];
    const char * e = fields[3]-1;
    const char * S = wordend - (e-f);
    if(S >= s)
        {
        s = S;
        while(f < e && *f == *S)
            {
            ++f;
            ++S;
            }
        }
    // On success: return pointer to successor of last unparsed character
    // On failure: return 0
    return f == e ? s : 0;
    }

static bool substr(const char ** fields,int k,const char * w,const char * wend,var * vars,int vindex)
    {
    if(w == wend)
        return false;
    const char * f = fields[k];
    const char * e = fields[k+1]-1;
    const char * p = w;
    assert(f != e);
    const char * ff;
    const char * pp;
    do
        {
        while((p < wend) && (*p != *f))
            {
            ++p;
            }
        if(p == wend)
            return false;
        pp = ++p;
        ff = f+1;
        while(ff < e)
            {
            if(pp == wend)
                return false;
            if(*pp != *ff)
                break;
            ++pp;
            ++ff;
            }
        }
    while(ff != e);
    vars[vindex].e = p-1;
    vars[vindex+1].s = pp;
    return true;
    }

/** /
static void printpat(const char ** fields,int findex)
    {
    printf("findex %d {%.*s]",findex,fields[1] - fields[0] - 1,fields[0]);//,vars[0].e - vars[0].s,vars[0].s);
    for(int m = 1;2*m+3 < findex;++m)
        {
        int M = 2*m+3;
        printf("[%.*s]",fields[M] - fields[M-1] - 1,fields[M-1]);//,vars[m].e - vars[m].s,vars[m].s);
        }
    if(findex > 2)
        printf("[%.*s}\n",fields[3] - fields[2] - 1,fields[2]);
    else
        printf("\n");
    }
/ **/
#if TESTING
static void printpat2(const char ** fields,int findex,char * start,char * middle,char * end)
    {
    int L = fields[1] - fields[0] - 1;
    strncpy(start,fields[0],L);
    start[L] = '\0';
    *middle = '\0';
    for(int m = 1;2*m+3 < findex;++m)
        {
        int M = 2*m+3;
        L = fields[M] - fields[M-1] - 1;
        *middle++ = '*';
        strncpy(middle,fields[M-1],L);
        middle[L] = '\0';
        middle += L;
        }
    if(findex > 2)
        {
        L = fields[3] - fields[2] - 1;
        strncpy(end,fields[2],L);
        end[L] = '\0';
        }
    else
        *end = '\0';
    }
/*
static void printpat(char ** fields,int findex,char * start,char * end)
    {
    sprintf(start+strlen(start),"%.*s",fields[1] - fields[0] - 1,fields[0]);
    fprintf(fm,"%s",start);
    for(int m = 1;2*m+3 < findex;++m)
        {
        int M = 2*m+3;
        fprintf(fm,"*%.*s",fields[M] - fields[M-1] - 1,fields[M-1]);//,vars[m].e - vars[m].s,vars[m].s);
        }
    if(findex > 2)
        {
        Strrev(end);
        int L = strlen(end);
        sprintf(end+L,"%.*s",fields[3] - fields[2] - 1,fields[2]);
        Strrev(end+L);
        Strrev(end);
        fprintf(fm,"*%s\t-->\t",end);
        / *Strrev(end);
        end[L] = 0;
        Strrev(end);* /
        }
    else
        fprintf(fm,"%s\t-->\t",end);

    fprintf(fm,"%.*s",fields[2] - fields[1] - 1,fields[1]);
    for(int m = 1;2*m+3 < findex;++m)
        {
        int M = 2*m+3;
        fprintf(fm,"*%.*s",fields[M+1] - fields[M] - 1,fields[M]);//,vars[m].e - vars[m].s,vars[m].s);
        }
    if(findex > 2)
        {
        fprintf(fm,"*%.*s\n",fields[4] - fields[3] - 1,fields[3]);
        }
    else
        fprintf(fm,"\n");
    }
    */
#endif
// return values:
// 0: failure
// 1: success, caller must add lemma to the end of the result string
// 2: success, caller must add lemma to the start of the result string
// 3: success, caller must not add lemma to the result string
static int lemmatiseer(const char * word,const char * wordend,const char * buf,const char * maxpos
#if TESTING
                        ,char * start = NULL ,char * middle = NULL ,char * end = NULL
#endif
                        )
    {
    int pos = 0;
    do // loop until matching pattern found
        {
        assert(pos >= 0);
        assert(maxpos == buf+pos || *(int*)(buf+pos) >= 0);
        buf += pos;
        if(buf == maxpos)
            {
            pos = 0;
            return 0;
            }
        else
            pos = *(int*)buf; // pos now points to the next record, which has to be tried if the current record fails.
        const char * p = buf + sizeof(int);
#if TESTING
        int slen = strlen(start);
        int mlen = strlen(middle);
        int elen = strlen(end);
#endif
        switch(*p)
            {
            case 1: // on success, subtree makes lhs of alternating pair
            case 2: // on success, subtree makes rhs of alternating pair
                {
                if(oneAnswer)
                    {
                    if(*p == 1)
                        {
                        p += sizeof(int);
                        int hs = lemmatiseer(word,wordend,p,maxpos
#if TESTING
                            ,start,middle,end
#endif
                            );
#if TESTING
                        start[slen] = 0;
                        middle[mlen] = 0;
                        end[elen] = 0;
#endif
                        return hs ? 3 : 0;
                        }
                    else
                        return 0;
                    }
                p += sizeof(int);
                int hs = lemmatiseer(word,wordend,p,maxpos
#if TESTING
                                    ,start,middle,end
#endif
                                    );
#if TESTING
                start[slen] = 0;
                middle[mlen] = 0;
                end[elen] = 0;
#endif
                return hs ? *p : 0;
                }
            case 3: // on success, subtree makes both lhs and rhs of alternating pair
                {
                p += sizeof(int);
                int altpos = *(int *)p;
                int lhs = lemmatiseer(word,wordend,p + sizeof(int),p + altpos
#if TESTING
                                      ,start,middle,end
#endif
                                      );
                if(oneAnswer)
                    return lhs;
                else
                    {
#if TESTING
                    start[slen] = 0;
                    middle[mlen] = 0;
                    Strrev(end);
                    end[elen] = 0;
                    Strrev(end);
#endif
                    int rhs = lemmatiseer(word,wordend,p + altpos,maxpos
#if TESTING
                                     ,start,middle,end
#endif
                                     );
                    // Not sure whether following test is right.
                    // Is it possible that lhs and rhs both are > 0 but different?
                    //assert(lhs == rhs);
                    // it is possible that eg. lhs fails and rhs succeeds
                    if(lhs)
                        {
                        if(rhs)
                            return 3;
                        else
                            return 1;
                        }
                    else if(rhs)
                        return 2;
                    else
                        return 0;
                    }
                }
            default:
                {
                var vars[20];
                const char * fields[44]; // 44 = (2*20 + 3) + 1
                // The even numbered fields contain patterns
                // The odd numbered fields contain replacements
                // The first two fields (0,1) refer to the prefix
                // The third and fourth (2,3) fields refer to the affix
                // The remaining fields (4,5,..,..) refer to infixes, from left to right
                // output=fields[1]+vars[0]+fields[5]+vars[1]+fields[7]+vars[2]+...+fields[2*n+3]+vars[n]+...+fields[3]
                const char * wend = wordend;
                fields[0] = p;
                int findex = 1;
                while(*p != '\n')
                    {
                //    putchar(*p);
                    if(*p == '\t')
                        fields[findex++] = ++p;
                    else
                        ++p;
                    }
                //putchar('\n');
                fields[findex] = ++p; 
                // fields[findex] points to character after \n. 
                // When 1 is subtracted, it points to the character following the last replacement.
                // p is now within 3 bytes from the first Record of the subtree
                //        printpat(fields,findex);
                // check Lpat
                vars[0].s = samestart(fields,word,wend);
                if(vars[0].s)
                    {
                    // Lpat succeeded
                    vars[0].e = wend;
                    char * destination = NULL;
                    int printed = 0;
                    int subres = 0;
                    if(findex > 2) // there is more than just a prefix
                        {
                        const char * newend = sameend(fields,vars[0].s,wend);
                        if(newend)
                            wend = newend;
                        else
                            continue; //suffix didn't match

                        int k;
                        const char * w = vars[0].s;
                        int vindex = 0;
                        for(k = 4;k < findex;k += 2)
                            {
                            if(!substr(fields,k,w,wend,vars,vindex))
                                break;
                            ++vindex;
                            w = vars[vindex].s;
                            }
                        if(k < findex)
                            continue;

                        vars[vindex].e = newend;
                        // Find first record of subtree, if there is any
                        ptrdiff_t nxt = p - buf; /*20120709 long -> ptrdiff_t*/
                        nxt += sizeof(int)-1;
                        nxt /= sizeof(int);
                        nxt *= sizeof(int);
#if TESTING
                        char _end[30];
                        if(start)
                            {
                            printpat2(fields,findex,start+strlen(start),middle,_end);
                            strcpy(_end+strlen(_end),end);
                            strcpy(end,_end);
                            }
#endif
                        const char * mmaxpos = pos ? buf + pos : maxpos;
                        const char * mbuf = buf + nxt;
                        assert(buf <= mmaxpos);
                        if(  mbuf >= mmaxpos                              // There is no subtree
                          || (subres = lemmatiseer(vars[0].s,wend,mbuf,mmaxpos // There is a subtree, but it has no matching records
#if TESTING
                                                  ,start,middle,end
#endif
                                                  )
                             ) < 3 // if subres IS 3, this rule must not fire.
                          )
                            {//                     length of prefix       length of first unmatched         length of suffix
                            ptrdiff_t resultlength = (fields[2] - fields[1] - 1) + (vars[0].e - vars[0].s) + (fields[4] - fields[3] - 1);/*20120709 int -> ptrdiff_t*/
#if TESTING
                            int replacementlength = (fields[2] - fields[1] - 1) + 1 + (fields[4] - fields[3] - 1);
#endif
                            int m;
                            for(m = 1;2*m+3 < findex;++m)
                                {
                                int M = 2*m+3;
                                //                    length of infix       length of unmatched after infix
                                resultlength += (fields[M+1] - fields[M] - 1) + (vars[m].e - vars[m].s);
#if TESTING
                                replacementlength += (fields[M+1] - fields[M] - 1) + 1;
#endif
                                }
                            destination = new char[resultlength+1];
                            printed = sprintf(destination,"%.*s%.*s",(int)(fields[2] - fields[1] - 1),fields[1],(int)(vars[0].e - vars[0].s),vars[0].s);
#if TESTING
                            replacement = new char[replacementlength+1];
                            int printed2 = sprintf(replacement,"%.*s%.*s",fields[2] - fields[1] - 1,fields[1],1,"*");
#endif
                            for(m = 1;2*m+3 < findex;++m)
                                {
                                int M = 2*m+3;
                                printed += sprintf(destination+printed,"%.*s%.*s",(int)(fields[M+1] - fields[M] - 1),fields[M],(int)(vars[m].e - vars[m].s),vars[m].s);
#if TESTING
                                printed2 += sprintf(replacement+printed2,"%.*s%.*s",fields[M+1] - fields[M] - 1,fields[M],1,"*");
#endif
                                }
                            printed += sprintf(destination+printed,"%.*s",(int)(fields[4] - fields[3] - 1),fields[3]);
#if TESTING
                            sprintf(replacement+printed2,"%.*s",fields[4] - fields[3] - 1,fields[3]);
#endif
                            }
                        }
                    else if(vars[0].e == vars[0].s) // whole-word match: everything matched by "prefix"
                        {
                        subres = 0;
                        destination = new char[(fields[2] - fields[1] - 1)+1];
                        printed = sprintf(destination,"%.*s",(int)(fields[2] - fields[1] - 1),fields[1]);
#if TESTING
                        replacement = new char[(fields[2] - fields[1] - 1)+1];
                        sprintf(replacement,"%.*s",fields[2] - fields[1] - 1,fields[1]);
                        char _end[30];
                        if(start)
                            {
                            printpat2(fields,findex,start+strlen(start),middle,_end);
                            strcpy(_end+strlen(_end),end);
                            strcpy(end,_end);

                            }
#endif
                        }
                    else
                        continue; // something unmatched

                    if(destination)
                        {
#if TESTING
                        char temp[1000];
                        if(findex == 2)
                            sprintf(temp,"%s:%s%s%s->%s %d",destination,start,middle,end,replacement,findex);
                        else
                            sprintf(temp,"%s:%s%s*%s->%s %d",destination,start,middle,end,replacement,findex);
                        printf("%s\n",temp);
#endif
                        assert(subres < 3);
                        if(!result)
                            {
                            assert(subres == 0);
                            result = destination;
                            }
                        else // Check whether an alternative lemma was found
                            {
                            //assert(subres == 1 || subres == 2);
                            char * sub = strstr(result,destination);
                            if(  !sub 
                              || (  sub != result 
                                 && sub[-1] != ' '
                                 ) 
                              || (  sub[printed] != '\0'
                                 && sub[printed] != ' '
                                 )
                              )
                                { // Yes, lemma was not found already
                                char * newresult = new char[strlen(result)+printed+2];
                                if(subres == 1)
                                    sprintf(newresult,"%s %s",result,destination);
                                else if(subres == 2)
                                    sprintf(newresult,"%s %s",destination,result);
                                else
                                    sprintf(newresult,"%s %s",result,destination);
                                delete [] result;
                                result = newresult;
                                }
                            delete [] destination;
                            }
                        }
                    return 3;
                    }
                else
                    {
                    // Lpat failed
                    continue; // prefix failed
                    }
                }
            }
        }
    while(pos);
    return 0;
    }


bool readRules(FILE * flexrulefile,const char * flexFileName)
    {
    if(flexFileName)
        {
        ::flexFileName = flexFileName;
        }
    if(flexrulefile)
        {
        fseek(flexrulefile,0,SEEK_END);
        end = ftell(flexrulefile);
        buf = new char[end+1];// 20140224 +1
        if(buf && end > 0)
            {
            rewind(flexrulefile);
            if(fread(buf,1,end,flexrulefile) != (size_t)end)
                return 0;// 20120710
            NewStyle = true;
            buf[end] = '\0';// 20140224 new
            }
        return buf != 0;
        }
    return flexFileName != 0;
    }


bool readRules(const char * flexFileName)
    {
    if(flexFileName)
        {
        ::flexFileName = flexFileName;
        }
    if(Hash == NULL)
        Hash = new hash<rules>(&rules::tagName,10);
    return flexFileName != 0;
    }

void deleteRules()
    {
    delete [] buf;
    buf = 0;
    delete [] result;
    result = 0;
#if TESTING
    delete [] replacement;
    replacement = 0;
#endif
    NewStyle = false;
    }


const char * applyRules(const char * word)
    {
    if(buf)
        {
        size_t len = strlen(word);
        if(flex::baseformsAreLowercase)
            {
            size_t length = 0;
            word = changeCase(word,true,length);
            // This only works if changeCase is not called in the remainder of applyRules
            /*
            loword = new char[len+1];
            strcpy(loword,word);
            AllToLower(loword);
            word = loword;
            */
            }

        delete [] result;
        result = 0;
#if TESTING
        delete [] replacement;
        replacement = 0;
        char Start[30] = {'\0'};
        char Middle[30] = {'\0'};
        char End[30] = {'\0'};
        printf("WOORD [%.*s]\n",len,word);
#endif
        lemmatiseer(word,word+len
            ,buf
            ,buf+end
#if TESTING
            ,Start,Middle,End
#endif
            );
        //printf("%.*s -> %s ",len,word,result);

#if TESTING
        char temp[1000];
        sprintf(temp,"%s\t%s%s*%s->%s",result,Start,Middle,End,replacement);
        int newresult = strlen(temp)+1;
        delete [] result;
        result = new char[newresult];
        strcpy(result,temp);
#endif
//        delete [] loword;
        return result;
        }
    return 0;
    }


const char * applyRules(const char * word,const char * tag)
    {
    if(buf)
        {
        size_t len = strlen(word);
        //char * loword = 0;
        if(flex::baseformsAreLowercase)
            {
            size_t length = 0;
            if(strchr(word,'/') > 0)
#if STREAM
                cout << "Strange word [" << word << "] tag [" << tag << "]" << endl;
#else
                printf("Strange word [%s] tag [%s]\n",word,tag);
#endif
            word = changeCase(word,true,length);
            /*
            loword = new char[len+1];
            strcpy(loword,word);
            AllToLower(loword);
            word = loword;
            */
            }
        delete [] result;
        result = 0;
#if TESTING
        delete [] replacement;
        replacement = 0;
#endif
        if(tag && *tag)
            {
            void * v;
            rules * Rules;
            if(!Hash)
                Hash = new hash<rules>(&rules::tagName,10);
            Rules = Hash->find(tag,v);
            if(!Rules)
                {
                Rules = new rules(tag);
                Hash->insert(Rules,v);
                /*
                Hash->remove(Rules,v);
                Hash->deleteThings();
                Hash->forall(&rules::print);
                int N;
                rules ** Rls = Hash->convertToList(N);
                */
                }
            if(Rules->buf())
                lemmatiseer(word,word+len,Rules->buf(),Rules->buf()+Rules->end());
            else
                lemmatiseer(word,word+len,buf,buf+end);
            }
        else
            lemmatiseer(word,word+len,buf,buf+end);
        //delete [] loword;
        return result;
        }
    return 0;
    }

#endif