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

typedef unsigned char typetype;

bool oneAnswer = false;

struct startEnd
    {
    const char * s;
    const char * e;
    };

struct lemmaCandidate
    {
    const char * L;
    bool ruleHasPrefix;
    };

//static int news = 0;

static const char * flexFileName;
static char bufbuf[] = "\0\0\0\0\t\t\t\n"; //20090811: corrected wrong value // "lemma == word" default rule set
//static char * buf = bufbuf; // Setting buf directly to a constant string generates a warning in newer gcc
//static long buflen = 8;
static char * result = 0;
#define TESTING 0
#if TESTING
static char * replacement = 0; // FOR TEST PURPOSE
#endif
//static int NewStyle = 2;
static bool donotAddLemmaUnlessRuleHasPrefix = false;

class rules;



static rules * taglessrules = 0;

class rules
    {
    private:
        char * TagName;
        char * buf;
        long buflen;
        long End;
        int NewStyle;
    public:
        rules() : TagName(0), buf(bufbuf), buflen(8), NewStyle(2)
            {
            }
        rules(const char * TagName) : buflen(8), NewStyle(2)
            {
            this->TagName = new char[strlen(TagName) + 1];
            strcpy(this->TagName, TagName);
            char * filename = new char[strlen(TagName) + strlen(flexFileName) + 2];
            sprintf(filename, "%s.%s", flexFileName, TagName);
            FILE * f = fopen(filename, "rb");
            if (f)
                {
                buf = readRules(f, End);
                fclose(f);
                }
            else
                {
                buf = 0;
                End = 0;
                fprintf(stderr, "CSTlemma-applyrules.cpp: Cannot open rules [%s]\n",filename);
                }
			delete [] filename;
            }
        ~rules()
            {
			delete [] TagName;
            if (buf != bufbuf)
                delete [] buf;
            }
        const char * tagName() const { return TagName; }
        const char * Buf(){ return buf; }
        long end(){ return End; }
        void print(){}
        int newStyleRules(){ return NewStyle; }
        char * readRules(FILE * flexrulefile, long & end);
        bool readRules(FILE * flexrulefile, const char * flexFileName);
        const char * applyRules(const char * word, bool SegmentInitial);
        const char * applyRules(const char * word, const char * tag, bool SegmentInitial);
        bool setNewStyleRules(int val);
//        int newStyleRules();
    };

//rules * taglessrules = 0;
bool rules::setNewStyleRules(int val)
    {
    assert(val == 2 || val == 3);
    switch (val)
        {
            case 0:// first version, suffix only
            case 2:// second version (about 2009), affix. File starts with four 0 bytes.
            case 3:// third version (2015), affix, handles triple (and more) ambiguity. File starts with "\rV3\r".
                NewStyle = val;
                return true;
            default:
                return false;
        }
    }

bool setNewStyleRules(int val)
    {
    assert(val == 0);
    assert(taglessrules == 0);
    return true;
    }

bool readRules(FILE * flexrulefile, const char * flexFileName)
    {
    if (taglessrules == 0)
        taglessrules = new rules();
    return taglessrules->readRules(flexrulefile, flexFileName);
    }

const char * applyRules(const char * word, bool SegmentInitial)
    {
    assert(taglessrules);
    return taglessrules->applyRules(word, SegmentInitial);
    }

const char * applyRules(const char * word, const char * tag, bool SegmentInitial)
    {
    assert(taglessrules);
    return taglessrules->applyRules(word, tag, SegmentInitial);
    }

bool rules::readRules(FILE * flexrulefile, const char * flexFileName)
    {
    if (flexFileName)
        {
        ::flexFileName = flexFileName;
        }
    long end;
    if (flexrulefile)
        return 0 != readRules(flexrulefile, end);
    return flexFileName != 0;
    }

/*static*/ char * rules::readRules(FILE * flexrulefile, long & end)
    {
    if (flexrulefile)
        {
        int start;
        if (fread(&start, sizeof(int), 1, flexrulefile) != 1)
            return bufbuf;
        if (start == 0)
            {
            if (!setNewStyleRules(2))
                return bufbuf;
            }
        else if (start == *(int*)"\rV3\r")
            {
            if (!setNewStyleRules(3))
                return bufbuf;
            }
        else
            {
            return bufbuf; // not the new format
            }
        fseek(flexrulefile, 0, SEEK_END);
        end = ftell(flexrulefile);
        if (newStyleRules() == 3)
            {
            fseek(flexrulefile, sizeof(int), SEEK_SET);
            end -= sizeof(int);
            }
        else
            rewind(flexrulefile);
        buf = new char[end + 1];
        buflen = end;
        if (buf && end > 0)
            {
            if (fread(buf, 1, end, flexrulefile) != (size_t)end)
                return 0;
            buf[end] = '\0';
            }
        return buf;
        }
    return bufbuf;
    }


static hashmap::hash<rules> * Hash = NULL;

bool readRules(const char * flexFileName) // Does not read at all.
    { // Rules are read on a as-needed basis.
    if (flexFileName)
        {
        ::flexFileName = flexFileName;
        }
    if (Hash == NULL)
        Hash = new hashmap::hash<rules>(&rules::tagName, 10); // Memoizes the rule files that have been read.
    return flexFileName != 0;
    }

int newStyleRules()
    {
    if (taglessrules)
        return taglessrules->newStyleRules();
    return 0;
//    return NewStyle;
    }

static const char * samestart(const char ** fields, const char * s, const char * we)
    {
    const char * f = fields[0];
    const char * e = fields[1] - 1;
    while ((f < e) && (s < we) && (*f == *s))
        {
        ++f;
        ++s;
        }
    // On success: return pointer to first unparsed character
    // On failure: return 0
    return f == e ? s : 0;
    }

static const char * sameend(const char ** fields, const char * s, const char * wordend)
    {
    const char * f = fields[2];
    const char * e = fields[3] - 1;
    const char * S = wordend - (e - f);
    if (S >= s)
        {
        s = S;
        while (f < e && *f == *S)
            {
            ++f;
            ++S;
            }
        }
    // On success: return pointer to successor of last unparsed character
    // On failure: return 0
    return f == e ? s : 0;
    }

static bool substr(const char ** fields, int k, const char * w, const char * wend, startEnd * vars, int vindex)
    {
    if (w == wend)
        return false;
    const char * f = fields[k];
    const char * e = fields[k + 1] - 1;
    const char * p = w;
    assert(f != e);
    const char * ff;
    const char * pp;
    do
        {
        while ((p < wend) && (*p != *f))
            {
            ++p;
            }
        if (p == wend)
            return false;
        pp = ++p;
        ff = f + 1;
        while (ff < e)
            {
            if (pp == wend)
                return false;
            if (*pp != *ff)
                break;
            ++pp;
            ++ff;
            }
        } while (ff != e);
        vars[vindex].e = p - 1;
        vars[vindex + 1].s = pp;
        return true;
    }

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
#endif

// return values:
// 0: failure
// 1: success, caller must add lemma to the end of the result string
// 2: success, caller must add lemma to the start of the result string
// 3: success, caller must not add lemma to the result string
static int lemmatiseer(const char * word, const char * wordend, const char * buf, const char * maxpos
#if TESTING
                       ,char * start = NULL ,char * middle = NULL ,char * end = NULL
#endif
                       )
    {
    int pos = 0;
    do // loop until matching pattern found
        {
        assert(pos >= 0);
        assert(maxpos == buf + pos || *(int*)(buf + pos) >= 0);
        buf += pos;
        if (buf == maxpos)
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
        switch (*p)
            {
            case 1: // on success, subtree makes lhs of alternating pair
            case 2: // on success, subtree makes rhs of alternating pair
                {
                if (oneAnswer)
                    {
                    if (*p == 1)
                        {
                        p += sizeof(int);
                        int hs = lemmatiseer(word, wordend, p, maxpos
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
                int hs = lemmatiseer(word, wordend, p, maxpos
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
                int lhs = lemmatiseer(word, wordend, p + sizeof(int), p + altpos
#if TESTING
                                      ,start,middle,end
#endif
                                      );
                if (oneAnswer)
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
                    int rhs = lemmatiseer(word, wordend, p + altpos, maxpos
#if TESTING
                                          ,start,middle,end
#endif
                                          );
                    // Not sure whether following test is right.
                    // Is it possible that lhs and rhs both are > 0 but different?
                    //assert(lhs == rhs);
                    // it is possible that eg. lhs fails and rhs succeeds
                    if (lhs)
                        {
                        if (rhs)
                            return 3;
                        else
                            return 1;
                        }
                    else if (rhs)
                        return 2;
                    else
                        return 0;
                    }
                }
            default:
                {
                startEnd vars[20];
                const char * fields[44]; // 44 = (2*20 + 3) + 1
                // The even numbered fields contain patterns
                // The odd numbered fields contain replacements
                // The first two fields (0,1) refer to the prefix
                // The third and fourth (2,3) fields refer to the suffix
                // The remaining fields (4,5,..,..) refer to infixes, from left to right
                // output=fields[1]+vars[0]+fields[5]+vars[1]+fields[7]+vars[2]+...+fields[2*n+3]+vars[n]+...+fields[3]
                const char * wend = wordend;
                fields[0] = p;
                int findex = 1;
                while (*p != '\n')
                    {
                    //    putchar(*p);
                    if (*p == '\t')
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
                vars[0].s = samestart(fields, word, wend);
                if (vars[0].s)
                    {
                    // Lpat succeeded
                    vars[0].e = wend;
                    char * destination = NULL;
                    int printed = 0;
                    int subres = 0;
                    if (findex > 2) // there is more than just a prefix
                        {
                        const char * newend = sameend(fields, vars[0].s, wend);
                        if (newend)
                            wend = newend;
                        else
                            continue; //suffix didn't match

                        int k;
                        const char * w = vars[0].s;
                        int vindex = 0;
                        for (k = 4; k < findex; k += 2)
                            {
                            if (!substr(fields, k, w, wend, vars, vindex))
                                break;
                            ++vindex;
                            w = vars[vindex].s;
                            }
                        if (k < findex)
                            continue;

                        vars[vindex].e = newend;
                        // Find first record of subtree, if there is any
                        ptrdiff_t nxt = p - buf; /*20120709 long -> ptrdiff_t*/
                        nxt += sizeof(int) - 1;
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
                        if (mbuf >= mmaxpos                              // There is no subtree
                            || (subres = lemmatiseer(vars[0].s, wend, mbuf, mmaxpos // There is a subtree, but it has no matching records
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
                            for (m = 1; 2 * m + 3 < findex; ++m)
                                {
                                int M = 2 * m + 3;
                                //                    length of infix       length of unmatched after infix
                                resultlength += (fields[M + 1] - fields[M] - 1) + (vars[m].e - vars[m].s);
#if TESTING
                                replacementlength += (fields[M+1] - fields[M] - 1) + 1;
#endif
                                }
                            //++news;
                            destination = new char[resultlength + 1];
                            printed = sprintf(destination, "%.*s%.*s", (int)(fields[2] - fields[1] - 1), fields[1], (int)(vars[0].e - vars[0].s), vars[0].s);
#if TESTING
                            replacement = new char[replacementlength+1];
                            int printed2 = sprintf(replacement,"%.*s%.*s",fields[2] - fields[1] - 1,fields[1],1,"*");
#endif
                            for (m = 1; 2 * m + 3 < findex; ++m)
                                {
                                int M = 2 * m + 3;
                                printed += sprintf(destination + printed, "%.*s%.*s", (int)(fields[M + 1] - fields[M] - 1), fields[M], (int)(vars[m].e - vars[m].s), vars[m].s);
#if TESTING
                                printed2 += sprintf(replacement+printed2,"%.*s%.*s",fields[M+1] - fields[M] - 1,fields[M],1,"*");
#endif
                                }
                            printed += sprintf(destination + printed, "%.*s", (int)(fields[4] - fields[3] - 1), fields[3]);
#if TESTING
                            sprintf(replacement+printed2,"%.*s",fields[4] - fields[3] - 1,fields[3]);
#endif
                            }
                        }
                    else if (vars[0].e == vars[0].s) // whole-word match: everything matched by "prefix"
                        {
                        subres = 0;
                        //++news;
                        destination = new char[(fields[2] - fields[1] - 1) + 1];
                        printed = sprintf(destination, "%.*s", (int)(fields[2] - fields[1] - 1), fields[1]);
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

                    if (destination)
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
                        if (!result)
                            {
                            assert(subres == 0);
                            result = destination;
                            }
                        else // Check whether an alternative lemma was found
                            {
                            //assert(subres == 1 || subres == 2);
                            char * sub = strstr(result, destination);
                            if (!sub
                                || (sub != result
                                && sub[-1] != ' '
                                )
                                || (sub[printed] != '\0'
                                && sub[printed] != ' '
                                )
                                )
                                { // Yes, lemma was not found already
                                //++news;
                                char * newresult = new char[strlen(result) + printed + 2];
                                if (subres == 1)
                                    sprintf(newresult, "%s %s", result, destination);
                                else if (subres == 2)
                                    sprintf(newresult, "%s %s", destination, result);
                                else
                                    sprintf(newresult, "%s %s", result, destination);
                                //--news;
                                delete[] result;
                                result = newresult;
                                }
                            //--news;
                            delete[] destination;
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
        } while (pos);
        return 0;
    }

/*
static void Strrev(char * s)
    {
    char * e = s + strlen(s);
    while (s < --e)
        {
        char t = *s;
        *s++ = *e;
        *e = t;
        }
    }

static void printpat(const char ** fields, int findex)
    {
    char start[100] = { 0 };
    char end[100] = { 0 };
    sprintf(start + strlen(start), "%.*s", (int)(fields[1] - fields[0] - 1), fields[0]);
    printf("%s", start);
    for (int M = 5; M < findex; M += 2)
        {
        printf("*%.*s", (int)(fields[M] - fields[M - 1] - 1), fields[M - 1]);
        }
    if (findex > 2)
        {
        Strrev(end);
        size_t L = strlen(end);
        sprintf(end + L, "%.*s", (int)(fields[3] - fields[2] - 1), fields[2]);
        Strrev(end + L);
        Strrev(end);
        printf("*%s\t-->\t", end);
        }
    else
        {
        printf("%s\t-->\t", end);
        }

    printf("%.*s", (int)(fields[2] - fields[1] - 1), fields[1]);
    for (int M = 5; M < findex; M += 2)
        {
        printf("*%.*s", (int)(fields[M + 1] - fields[M] - 1), fields[M]);
        }
    if (findex > 2)
        {
        printf("*%.*s\n", (int)(fields[4] - fields[3] - 1), fields[3]);
        }
    else
        {
        printf("\n");
        }
    }
*/

static char * rewrite(const char *& word, const char *& wordend, const char * p)
    {
    startEnd vars[20];
    const char * fields[44]; // 44 = (2*20 + 3) + 1
    // The even numbered fields contain patterns
    // The odd numbered fields contain replacements
    // The first two fields (0,1) refer to the prefix
    // The third and fourth (2,3) fields refer to the suffix
    // The remaining fields (4,5,..,..) refer to infixes, from left to right
    // output=fields[1]+vars[0]+fields[5]+vars[1]+fields[7]+vars[2]+...+fields[2*n+3]+vars[n]+...+fields[3]
    const char * wend = wordend;
    fields[0] = p;
    int findex = 1;
    while (*p != '\n')
        {
        if (*p == '\t')
            fields[findex++] = ++p;
        else
            ++p;
        }
    fields[findex] = ++p;
    //printpat(fields, findex);
    // fields[findex] points to character after \n. 
    // When 1 is subtracted, it points to the character following the last replacement.
    // p is now within 3 bytes from the first Record of the subtree
    //        printpat(fields,findex);
    // check Lpat
    vars[0].s = samestart(fields, word, wend);
    if (vars[0].s)
        {
        // Lpat succeeded
        vars[0].e = wend;
        char * destination = NULL;
        int printed = 0;
        if (findex > 2) // there is more than just a prefix
            {
            const char * newend = sameend(fields, vars[0].s, wend);
            if (newend)
                wend = newend;
            else
                return 0; //suffix didn't match

            int k;
            const char * w = vars[0].s;
            int vindex = 0;
            for (k = 4; k < findex; k += 2)
                {
                if (!substr(fields, k, w, wend, vars, vindex))
                    break;
                ++vindex;
                w = vars[vindex].s;
                }
            if (k < findex)
                return 0;

            vars[vindex].e = newend;
            //                     length of prefix       length of first unmatched         length of suffix
            ptrdiff_t resultlength = (fields[2] - fields[1] - 1) + (vars[0].e - vars[0].s) + (fields[4] - fields[3] - 1);/*20120709 int -> ptrdiff_t*/
            int m;
            for (m = 1; 2 * m + 3 < findex; ++m)
                {
                int M = 2 * m + 3;
                //                    length of infix       length of unmatched after infix
                resultlength += (fields[M + 1] - fields[M] - 1) + (vars[m].e - vars[m].s);
                }
            //++news;
            destination = new char[resultlength + 1];
            printed = sprintf(destination, "%.*s%.*s", (int)(fields[2] - fields[1] - 1), fields[1], (int)(vars[0].e - vars[0].s), vars[0].s);
            for (m = 1; 2 * m + 3 < findex; ++m)
                {
                int M = 2 * m + 3;
                printed += sprintf(destination + printed, "%.*s%.*s", (int)(fields[M + 1] - fields[M] - 1), fields[M], (int)(vars[m].e - vars[m].s), vars[m].s);
                }
            printed += sprintf(destination + printed, "%.*s", (int)(fields[4] - fields[3] - 1), fields[3]);
            word = vars[0].s;
            wordend = newend;
            }
        else if (vars[0].e == vars[0].s) // whole-word match: everything matched by "prefix"
            {
            //++news;
            destination = new char[(fields[2] - fields[1] - 1) + 1];
            printed = sprintf(destination, "%.*s", (int)(fields[2] - fields[1] - 1), fields[1]);
            }
        else
            return 0; // something unmatched

        return destination;
        }
    else
        {
        // Lpat failed
        return 0; // prefix failed
        }
    }

static char ** addLemma(char ** lemmas, const lemmaCandidate * lemma)
    {
    if(donotAddLemmaUnlessRuleHasPrefix)
        {
        if(!lemma->ruleHasPrefix)
            {
            return lemmas;
            }
        }

    if (lemma->L)
        {
        if (lemmas)
            {
            int i;
            for (i = 0; lemmas[i]; ++i)
                {
                if (!strcmp(lemmas[i], lemma->L))
                    {
                    return lemmas;
                    }
                }
            //++news;
            char ** nlemmas = new char *[i + 2];
            for (i = 0; lemmas[i]; ++i)
                {
                nlemmas[i] = lemmas[i];
                }
            //--news;
            delete[] lemmas;
            lemmas = nlemmas;
            //++news;
            lemmas[i] = new char[strlen(lemma->L) + 1];
            strcpy(lemmas[i], lemma->L);
            lemmas[++i] = 0;
            }
        else
            {
            //++news;
            lemmas = new char *[2];
            lemmas[1] = 0;
            //++news;
            lemmas[0] = new char[strlen(lemma->L) + 1];
            strcpy(lemmas[0], lemma->L);
            }
        }
    return lemmas;
    }

static char * concat(char ** L)
    {
    if (L)
        {
        size_t lngth = 0;
        int i;
        for (i = 0; L[i]; ++i)
            lngth += strlen(L[i]) + 1;
        ++lngth;
        //++news;
        char * ret = new char[lngth];
        ret[0] = 0;
        for (i = 0; L[i]; ++i)
            {
            strcat(ret, L[i]);
            //--news;
            delete[] L[i];
            strcat(ret, " ");
            }
        //--news;
        delete[] L;
        ret[lngth-2] = 0;
        return ret;
        }
    else
        return 0;
    }

static char ** pruneEquals(char ** L)
    {
    if (L)
        {
        for (int i = 0; L[i]; ++i)
            {
            for (int j = i + 1; L[j]; ++j)
                {
                if (!strcmp(L[i], L[j]))
                    {
                    delete[] L[j];
                    for (int k = j; L[k]; ++k)
                        {
                        L[k] = L[k + 1];
                        }
                    }
                }
            }
        }
    return L;
    }

static char ** lemmatiseerV3
    ( const char * word
    , const char * wordend
    , const char * buf
    , const char * maxpos
    , const lemmaCandidate * parentcandidate
    , char ** lemmas
    );

static char ** chainV3
    ( const char * word
    , const char * wordend
    , const char * buf
    , const char * maxpos
    , const lemmaCandidate * parentcandidate
    , char ** lemmas
    )
    {
    for (int next = *(int*)buf
         ;
         ; buf += next, next = *(int*)buf
         )
        {
        assert((next & 3) == 0);
        assert(next == -4 || next == 0 || next == 4 || next >= 12);
        if (next == -4 || next == 4)
            {
            // add parent candidate to lemmas.
            lemmas = addLemma(lemmas, parentcandidate);
            }
        else
            {
            char ** temp = lemmatiseerV3(word, wordend, buf + sizeof(int), next > 0 ? buf + next : maxpos, parentcandidate, lemmas);
            if (temp)
                {
                lemmas = temp;
                }
            else
                {
                lemmas = addLemma(lemmas, parentcandidate);
                }
            }
        if (next <= 0)
            break;
        }
    return lemmas;
    }

static char ** lemmatiseerV3
    ( const char * word
    , const char * wordend
    , const char * buf
    , const char * maxpos
    , const lemmaCandidate * parentcandidate
    , char ** lemmas
    )
    {
    if (maxpos <= buf)
        return 0;
    const char * cword = word;
    const char * cwordend = wordend;
    int pos = 0;
    pos = *(int*)buf;
    const char * until;
    char ** result;
    assert((pos & 3) == 0);
    assert(pos >= 0);
    if (pos == 0)
        until = maxpos;
    else
        until = buf + pos;
    typetype type;
    const char * p = buf + sizeof(int);
    type = *(typetype*)p;
    /*
    buf+4
    first bit  0: Fail branch is unambiguous, buf points to tree. (A)
    first bit  1: Fail branch is ambiguous, buf points to chain. (B)
    second bit 0: Success branch is unambiguous, buf+8 points to tree (C)
    second bit 1: Success branch is ambiguous, buf+8 points to chain (D)
    */
    if (type < 4)
        {
        ++p;
        }
    else
        {
        type = 0; // no ambiguity
        }
    lemmaCandidate candidate;
    if(*p && *p != '\t')
        {
        candidate.ruleHasPrefix = true;
        }
    else
        candidate.ruleHasPrefix = parentcandidate ? parentcandidate->ruleHasPrefix : false;
    candidate.L = rewrite(cword, cwordend, p);
    p = strchr(p, '\n');
    ptrdiff_t off = p - buf;
    off += sizeof(int);
    off /= sizeof(int);
    off *= sizeof(int);
    p = buf + off;
    if (candidate.L)
        {
        const lemmaCandidate * defaultCandidate = candidate.L[0] ? &candidate : parentcandidate;
        /* 20150806 A match resulting in a zero-length candidate is valid for
        descending, but if all descendants fail, the candidate is overruled by
        an ancestor that is not zero-length. (The top rule just copies the
        input, so there is a always a non-zero length ancestor.) */
        switch (type)
            {
            case 0:
            case 1:
                {
                /* Unambiguous children. If no child succeeds, take the
                candidate, otherwise take the succeeding child's result. */
                char ** childcandidates = lemmatiseerV3(cword, cwordend, p, until, defaultCandidate, lemmas);
                result = childcandidates ? childcandidates : addLemma(lemmas, defaultCandidate);
                delete[] candidate.L;
                break;
                }
            case 2:
            case 3:
                {
                /* Ambiguous children. If no child succeeds, take the
                candidate, otherwise take the succeeding children's result
                Some child may in fact refer to its parent, which is our
                current candidate. We pass the candidate so it can be put
                in the right position in the sequence of answers. */
                char ** childcandidates = chainV3(cword, cwordend, p, until, defaultCandidate, lemmas);
                result = childcandidates ? childcandidates : addLemma(lemmas, defaultCandidate);
                delete[] candidate.L;
                break;
                }
            default:
                result = lemmas;
            }
        }
    else
        {
        switch (type)
            {
            case 0:
            case 2:
                {
                /* Unambiguous siblings. If no sibling succeeds, take the
                parent's candidate. */
                char ** childcandidates = lemmatiseerV3(word, wordend, until, maxpos, parentcandidate, lemmas);
                result = childcandidates ? childcandidates : addLemma(lemmas, parentcandidate);
                break;
                }
            case 1:
            case 3:
                {
                /* Ambiguous siblings. If a sibling fails, the parent's
                candidate is taken. */
                char ** childcandidates = chainV3(word, wordend, until, maxpos, parentcandidate, lemmas);
                result = childcandidates ? childcandidates : addLemma(lemmas, parentcandidate);
                break;
                }
            default:
                result = lemmas;
            }
        }
    return result;
    }


void deleteRules()
    {
//    delete[] buf;
//    buf = 0;
    //--news;
    delete[] result;
    result = 0;
#if TESTING
    delete [] replacement;
    replacement = 0;
#endif
//    setNewStyleRules(0);
    }


const char * rules::applyRules(const char * word,bool SegmentInitial)
    {
    if (buf)
        {
        size_t len = strlen(word);
        if (flex::baseformsAreLowercase)
            {
            size_t length = 0;
            word = changeCase(word, true, length);
            // This only works if changeCase is not called in the remainder of applyRules
            /*
            loword = new char[len+1];
            strcpy(loword,word);
            AllToLower(loword);
            word = loword;
            */
            }

        //--news;
        donotAddLemmaUnlessRuleHasPrefix = false;
        delete[] result;
        result = 0;
#if TESTING
        delete [] replacement;
        replacement = 0;
        char Start[30] = {'\0'};
        char Middle[30] = {'\0'};
        char End[30] = {'\0'};
        printf("WOORD [%.*s]\n",len,word);
#endif
        if (newStyleRules() == 3)
            {
            char ** lemmas = 0;
            if(SegmentInitial && !flex::baseformsAreLowercase && isUpper((const unsigned char)word[0]))
                {
                donotAddLemmaUnlessRuleHasPrefix = true;
                lemmas = lemmatiseerV3(word, word + len, buf, buf + buflen, 0, lemmas);
                size_t length = 0;
                word = changeCase(word, true, length);
                donotAddLemmaUnlessRuleHasPrefix = false;
                result = concat(pruneEquals(lemmatiseerV3(word, word + len, buf, buf + buflen, 0, lemmas)));
                }
            else
                {
                result = concat(pruneEquals(lemmatiseerV3(word, word + len, buf, buf + buflen, 0, lemmas)));
                }


            }
        else
            {
            lemmatiseer(word, word + len
                        , buf
                        , buf + buflen
#if TESTING
                        ,Start,Middle,End
#endif
                        );
            }

#if TESTING
        char temp[1000];
        sprintf(temp,"%s\t%s%s*%s->%s",result,Start,Middle,End,replacement);
        int newresult = strlen(temp)+1;
        delete [] result;
        result = new char[newresult];
        strcpy(result,temp);
#endif
        return result;
        }
    return 0;
    }


const char * rules::applyRules(const char * word, const char * tag,bool SegmentInitial)
    {
    if (buf)
        {
        size_t len = strlen(word);
        //char * loword = 0;
        if (flex::baseformsAreLowercase || SegmentInitial)
            {
            size_t length = 0;
            if (strchr(word, '/') > 0)
#if STREAM
                cout << "Strange word [" << word << "] tag [" << tag << "]" << endl;
#else
                printf("Strange word [%s] tag [%s]\n",word,tag);
#endif
            word = changeCase(word, true, length);
            /*
            loword = new char[len+1];
            strcpy(loword,word);
            AllToLower(loword);
            word = loword;
            */
            }
        //--news;
        delete[] result;
        result = 0;
#if TESTING
        delete [] replacement;
        replacement = 0;
#endif
        if (tag && *tag)
            {
            void * v;
            rules * Rules;
            if (!Hash)
                Hash = new hashmap::hash<rules>(&rules::tagName, 10);
            Rules = Hash->find(tag, v);
            if (!Rules)
                {
                Rules = new rules(tag);
                Hash->insert(Rules, v);
                }
            if (Rules->Buf())
                if (newStyleRules() == 3)
                    {
                    char ** lemmas = 0;
                    result = concat(pruneEquals(lemmatiseerV3(word, word + len, Rules->Buf(), Rules->Buf() + Rules->end(), 0, lemmas)));
                    }
                else
                    {
                    lemmatiseer(word, word + len, Rules->Buf(), Rules->Buf() + Rules->end());
                    }
            else
                {
                if (newStyleRules() == 3)
                    {
                    char ** lemmas = 0;
                    result = concat(pruneEquals(lemmatiseerV3(word, word + len, buf, buf + buflen, 0, lemmas)));
                    }
                else
                    {
                    lemmatiseer(word, word + len, buf, buf + buflen);
                    }
                }
            }
        else
            {
            if (newStyleRules() == 3)
                {
                char ** lemmas = 0;
                result = concat(pruneEquals(lemmatiseerV3(word, word + len, buf, buf + buflen, 0, lemmas)));
                }
            else
                {
                lemmatiseer(word, word + len, buf, buf + buflen);
                }
            }
        return result;
        }
    return 0;
    }

#endif
