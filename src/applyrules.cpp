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
//#include "option.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

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
#if PRINTRULE
    const char * rule;
#endif
    };

struct LemmaRule
    {
    const char * Lem;
#if PRINTRULE
    const char * rule;
#endif
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

static const char* wordInOriginalCasing;

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
        rules() : TagName(0), buf(bufbuf), buflen(sizeof(bufbuf) - 1), End(0), NewStyle(3)
            {
            }
        rules(const char * TagName) : buflen(sizeof(bufbuf) - 1), NewStyle(3)
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
                buf = 0;//bufbuf;
                End = 0;//sizeof(bufbuf) - 1;
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
        const char * applyRules(const char * word, bool SegmentInitial,bool RulesUnique);
        const char * applyRules(const char * word, const char * tag, bool SegmentInitial, bool RulesUnique);
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

bool readRules(FILE * flexrulefile, const char * FlexFileName)
    {
    if (taglessrules == 0)
        taglessrules = new rules();
    return taglessrules->readRules(flexrulefile, FlexFileName);
    }

const char * applyRules(const char * word, bool SegmentInitial, bool RulesUnique)
    {
    if (taglessrules == 0)
        taglessrules = new rules();

//    assert(taglessrules);
    return taglessrules->applyRules(word, SegmentInitial, RulesUnique);
    }

const char * applyRules(const char * word, const char * tag, bool SegmentInitial, bool RulesUnique)
    {
    assert(taglessrules);
    return taglessrules->applyRules(word, tag, SegmentInitial, RulesUnique);
    }

bool rules::readRules(FILE * flexrulefile, const char * FlexFileName)
    {
    if (FlexFileName)
        {
        ::flexFileName = FlexFileName;
        }
    long end;
    if (flexrulefile)
        return 0 != readRules(flexrulefile, end);
    return FlexFileName != 0;
    }

/*static*/ char * rules::readRules(FILE * flexrulefile, long & end)
    {
    if (flexrulefile)
        {
        int istart;
        if (fread(&istart, sizeof(int), 1, flexrulefile) != 1)
            return bufbuf;
        if (istart == 0)
            {
            if (!setNewStyleRules(2))
                return bufbuf;
            }
        else if (istart == *(int*)"\rV3\r")
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

bool readRules(const char * FlexFileName) // Does not read at all.
    { // Rules are read on a as-needed basis.
    if (FlexFileName)
        {
        ::flexFileName = FlexFileName;
        }
    if (Hash == NULL)
        Hash = new hashmap::hash<rules>(&rules::tagName, 10); // Memoizes the rule files that have been read.
    if (taglessrules == 0)
        taglessrules = new rules();
    return FlexFileName != 0;
    }

int newStyleRules()
    {
    if (taglessrules)
        return taglessrules->newStyleRules();
    return 3;
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

#if PRINTRULE
static const char * printpat(const char ** fields, int findex,const char * start, int startbytes,const char * end)
    {
    size_t length =
          startbytes
        + (int)(fields[2] - fields[1] - 1);
    if (findex > 2)
        {
        length += 1; // wildcard
        length += strlen(end) + 1; // last of end in pattern, separator
        length += 1 + (int)(fields[4] - fields[3] - 1); // wildcard, end in replacement
        for (int M = 5; M < findex; M += 2)
            {
            length += 1 + (int)(fields[M] - fields[M - 1] - 1); // wildcard, infix in pattern
            length += 1 + (int)(fields[M + 1] - fields[M] - 1); // wildcard, infix in replacement
            }
        }
    else
        {
        length += strlen(end) + 1; // end in pattern, separator
        }

    length += 1; // zero

    char * buf = new char[length];
    char * fm = buf;
    fm += sprintf(fm, "%.*s", startbytes,start);
    for (int M = 5; M < findex; M += 2)
        {
        fm += sprintf(fm, "*%.*s", (int)(fields[M] - fields[M - 1] - 1), fields[M - 1]);
        }
    if (findex > 2)
        {
        fm += sprintf(fm, "*%s\v", end);
        }
    else
        {// Whole word matched by prefix. No wildcard after prefix.
        fm += sprintf(fm, "%s\v", end);
        }

    fm += sprintf(fm, "%.*s", (int)(fields[2] - fields[1] - 1), fields[1]);
    for (int M = 5; M < findex; M += 2)
        {
        fm += sprintf(fm, "*%.*s", (int)(fields[M + 1] - fields[M] - 1), fields[M]);
        }
    if (findex > 2)
        {
        fm += sprintf(fm, "*%.*s", (int)(fields[4] - fields[3] - 1), fields[3]);
        }
    assert(length == strlen(buf) + 1);
    return buf;
    }
#endif

#if LEMMATIZEV0
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
#endif

#if LEMMATIZEV0
// return values:
// 0: failure
// 1: success, caller must add lemma to the end of the result string
// 2: success, caller must add lemma to the start of the result string
// 3: success, caller must not add lemma to the result string
static int oldStyleLemmatize(const char * word, const char * wordend, const char * buf, const char * maxpos
#if TESTING
                       ,char * start ,char * middle ,char * end
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
                        int hs = oldStyleLemmatize(word, wordend, p, maxpos
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
                int hs = oldStyleLemmatize(word, wordend, p, maxpos
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
                int lhs = oldStyleLemmatize(word, wordend, p + sizeof(int), p + altpos
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
                    int rhs = oldStyleLemmatize(word, wordend, p + altpos, maxpos
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
                        if (  mbuf >= mmaxpos                              // There is no subtree
                           || (subres = oldStyleLemmatize(vars[0].s, wend, mbuf, mmaxpos // There is a subtree, but it has no matching records
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
                            if (  !sub
                               || (  sub != result
                                  && sub[-1] != ' '
                                  )
                               || (  sub[printed] != '\0'
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
#endif

static char * rewrite(const char *& word, const char *& wordend, const char * p
#if PRINTRULE
                     , const char * beginOfWord, const char *& rule
#endif
                     )
    {
    startEnd vars[20];
    const char * fields[44]; // 44 = (2*20 + 3) + 1
    // The even numbered fields contain patterns
    // The odd numbered fields contain replacements
    // The first two fields (0,1) refer to the prefix
    // The third and fourth (2,3) fields refer to the suffix
    // The remaining fields (4,5,..,..) refer to infixes, from left to right
    // input =fields[0]+vars[0]+fields[4]+vars[1]+fields[6]+vars[2]+...+fields[2*n+2]+vars[n]+...+fields[2]
    // output=fields[1]+vars[0]+fields[5]+vars[1]+fields[7]+vars[2]+...+fields[2*n+3]+vars[n]+...+fields[3]
    // where 'vars[k]' is the value caught by the k-th wildcard, k >= 0
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
    // fields[findex] points to character after \n. 
    // When 1 is subtracted, it points to the character following the last replacement.
    // p is now within 3 bytes from the first Record of the subtree
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
#if PRINTRULE
            if (beginOfWord)
                {
                rule = printpat(fields, findex, beginOfWord, word-beginOfWord, wordend);
                }
#endif
            }
        else if (vars[0].e == vars[0].s) // whole-word match: everything matched by "prefix"
            {
            //++news;
            destination = new char[(fields[2] - fields[1] - 1) + 1];
            printed = sprintf(destination, "%.*s", (int)(fields[2] - fields[1] - 1), fields[1]);
#if PRINTRULE
            if (beginOfWord)
                {
                rule = printpat(fields, findex, beginOfWord, vars[0].s - beginOfWord, wordend);
                }
#endif
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

static LemmaRule * addLemma(LemmaRule * lemmas, lemmaCandidate * lemma)
    {
    if(donotAddLemmaUnlessRuleHasPrefix)
        {
        if(!lemma->ruleHasPrefix)
            {
            return lemmas;
            }
        }

    size_t len;

    if (lemma->L)
        {
        if(flex::baseformsAreLowercase == caseTp::emimicked)
            {
            const char * adapted = adaptCase(lemma->L, wordInOriginalCasing, len);
            char* newL = new char[strlen(adapted) + 1];
            strcpy(newL, adapted);
            delete[] lemma->L;
            lemma->L = newL;
            }
        if (lemmas)
            {
            int i;
            for (i = 0; lemmas[i].Lem; ++i)
                {
                if (!strcmp(lemmas[i].Lem, lemma->L))
                    {
                    return lemmas;
                    }
                }
            LemmaRule * nlemmas = new LemmaRule [i + 2];
            for (i = 0; lemmas[i].Lem; ++i)
                {
                nlemmas[i] = lemmas[i];
                }
            delete[] lemmas;
            lemmas = nlemmas;
            lemmas[i].Lem = lemma->L;
            lemma->L = 0;
#if PRINTRULE
            lemmas[i].rule = lemma->rule;
            lemma->rule = 0;
#endif
            lemmas[++i].Lem = 0;
#if PRINTRULE
            lemmas[i].rule = 0;
#endif
            }
        else
            {
            lemmas = new LemmaRule [2];
            lemmas[1].Lem = 0;
            lemmas[0].Lem = lemma->L;
            lemma->L = 0;
#if PRINTRULE
            lemmas[1].rule = 0;
            lemmas[0].rule = lemma->rule;
            lemma->rule = 0;
#endif
            }
        }
    return lemmas;
    }

static char * concat(LemmaRule * L)
    {
    if (L)
        {
        size_t lngth = 0;
        int i;
        for (i = 0; L[i].Lem; ++i)
            {
            lngth += strlen(L[i].Lem) + 1;
#if PRINTRULE
            lngth += strlen(L[i].rule) + 1;
#endif
            }
        ++lngth;
        char * ret = new char[lngth];
        ret[0] = 0;
        for (i = 0; L[i].Lem; ++i)
            {
            strcat(ret, L[i].Lem);
            delete[] L[i].Lem;
            L[i].Lem = 0;
#if PRINTRULE
            strcat(ret, "\v");
            strcat(ret, L[i].rule);
            delete[] L[i].rule;
            L[i].rule = 0;
#endif
            strcat(ret, " ");
            }
        delete[] L;
        ret[lngth-1] = 0;
        return ret;
        }
    else
        return 0;
    }

static LemmaRule * pruneEquals(LemmaRule * L, bool RulesUnique)
    {
    if (RulesUnique)
        {
        for (int i = 1; L[i].Lem; ++i)
            {
            delete[] L[i].Lem;
            L[i].Lem = 0;
#if PRINTRULE
            delete[] L[i].rule;
            L[i].rule = 0;
#endif
            }
        }
    else if (L)
        {
        for (int i = 0; L[i].Lem; ++i)
            {
            for (int j = i + 1; L[j].Lem; ++j)
                {
                if (!strcmp(L[i].Lem, L[j].Lem))
                    {
                    delete[] L[j].Lem;
#if PRINTRULE
                    delete[] L[j].rule;
#endif
                    for (int k = j; L[k].Lem; ++k)
                        {
                        L[k].Lem = L[k + 1].Lem;
#if PRINTRULE
                        L[k].rule = L[k + 1].rule;
#endif
                        }
                    }
                }
            }
        }
    return L;
    }

static LemmaRule * newStyleLemmatizeV3
    ( const char * word
    , const char * wordend
    , const char * buf
    , const char * maxpos
    , lemmaCandidate * parentcandidate
    , LemmaRule * lemmas
#if PRINTRULE
    , const char * beginOfWord
#endif
    );

static LemmaRule * chainV3
    ( const char * word
    , const char * wordend
    , const char * buf
    , const char * maxpos
    , lemmaCandidate * parentcandidate
    , LemmaRule * lemmas
#if PRINTRULE
    , const char * beginOfWord
#endif
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
            LemmaRule * temp = newStyleLemmatizeV3(word, wordend, buf + sizeof(int), next > 0 ? buf + next : maxpos, parentcandidate, lemmas
#if PRINTRULE
                                        , beginOfWord
#endif
                                        );
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

static LemmaRule * newStyleLemmatizeV3
    ( const char * word
    , const char * wordend
    , const char * buf
    , const char * maxpos
    , lemmaCandidate * parentcandidate
    , LemmaRule * lemmas
#if PRINTRULE
    , const char * beginOfWord
#endif
    )
    {
    if (maxpos <= buf)
        return 0;
    const char * cword = word;
    const char * cwordend = wordend;
    int pos = 0;
    pos = *(int*)buf;
    const char * until;
    LemmaRule * Result;
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
    candidate.L = rewrite(cword, cwordend, p
#if PRINTRULE
                         , beginOfWord, candidate.rule
#endif
                         );
    p = strchr(p, '\n');
    ptrdiff_t off = p - buf;
    off += sizeof(int);
    off /= sizeof(int);
    off *= sizeof(int);
    p = buf + off;

    if (candidate.L)
        {
        lemmaCandidate * defaultCandidate = candidate.L[0] ? &candidate : parentcandidate;
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
                LemmaRule * childcandidates = newStyleLemmatizeV3(cword, cwordend, p, until, defaultCandidate, lemmas
#if PRINTRULE
                                                       , beginOfWord
#endif
                                                       );
                Result = childcandidates ? childcandidates : addLemma(lemmas, defaultCandidate);
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
                LemmaRule * childcandidates = chainV3(cword, cwordend, p, until, defaultCandidate, lemmas
#if PRINTRULE
                                                 , beginOfWord
#endif
                                                 );
                Result = childcandidates ? childcandidates : addLemma(lemmas, defaultCandidate);
                delete[] candidate.L;
                break;
                }
            default:
                Result = lemmas;
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
                LemmaRule * childcandidates = newStyleLemmatizeV3(word, wordend, until, maxpos, parentcandidate, lemmas
#if PRINTRULE
                                                       , beginOfWord
#endif
                                                       );
                Result = childcandidates ? childcandidates : addLemma(lemmas, parentcandidate);
                break;
                }
            case 1:
            case 3:
                {
                /* Ambiguous siblings. If a sibling fails, the parent's
                candidate is taken. */
                LemmaRule * childcandidates = chainV3(word, wordend, until, maxpos, parentcandidate, lemmas
#if PRINTRULE
                                                 , beginOfWord
#endif
                                                 );
                Result = childcandidates ? childcandidates : addLemma(lemmas, parentcandidate);
                break;
                }
            default:
                Result = lemmas;
            }
        }
    return Result;
    }


void deleteRules()
    {
    delete[] result;
    result = 0;
#if TESTING
    delete [] replacement;
    replacement = 0;
#endif
    }

static const char* apply ( const char* word
                         , bool SegmentInitial
                         , bool RulesUnique
                         , const char* buf
                         , const char* maxpos
                         )
    {
    wordInOriginalCasing = word;
    size_t len = strlen(word);
    if(flex::baseformsAreLowercase == caseTp::elower)
        {
        //size_t length = 0;
        word = changeCase(word, true, len/*gth*/); //Non-destructive! 'word' points
                    // to temporary location with lower cased copy of original.
	//len = strlen(word);
        }
    donotAddLemmaUnlessRuleHasPrefix = false;
    delete[] result;
    result = 0;
#if TESTING
    delete[] replacement;
    replacement = 0;
    char Middle[30] = { '\0' };
    printf("WOORD [%.*s]\n", len, word);
#endif
    if(newStyleRules() == 3)
        {
        LemmaRule* lemmas = 0;
        if(flex::baseformsAreLowercase == caseTp::emimicked)
            {
            // Lemmatize word as-is
            lemmas = newStyleLemmatizeV3(word, word + len, buf, maxpos, 0, lemmas
#if PRINTRULE
                                         , word
#endif
                                        );
            //size_t length = 0;
            // Lemmatize word converted to lowercase
            word = changeCase(wordInOriginalCasing, true, len/*gth*/);
            //len = strlen(word);
            lemmas = newStyleLemmatizeV3(word, word + len, buf, maxpos, 0, lemmas
#if PRINTRULE
                                         , word
#endif
                                        );
            // Lemmatize word with initial capital, remainder in lowercase
            //length = 1; 
            word = CapitalizeAndLowercase(wordInOriginalCasing);
            len = strlen(word);
            result = concat(pruneEquals(newStyleLemmatizeV3(word, word + len, buf, maxpos, 0, lemmas
#if PRINTRULE
                                                           , word
#endif
                                                           ), RulesUnique));
            }
        else if(SegmentInitial
           && (flex::baseformsAreLowercase == caseTp::easis)
           && isUpperUTF8(word)
           )
            {
            /* A capitalized word at the start of a segment should be
            lemmatized in two ways, if the rules are case sensitive:
            As a word that has a lemma that starts with a capital and as a
            word that has a lemma starting with a lower case character.
            To be sure that a lemma starts with a capital, the capital
            character must be made explicit by the lemmatization rule
            constructing that lemma. Therefore such a rule must start
            with a prefix. */
            donotAddLemmaUnlessRuleHasPrefix = true;
            lemmas = newStyleLemmatizeV3(word, word + len, buf, maxpos, 0, lemmas
#if PRINTRULE
                                        , word
#endif
                                        );
            //size_t length = 0;
            word = changeCase(word, true, len/*gth*/);
            //len = strlen(word);
            donotAddLemmaUnlessRuleHasPrefix = false;
            result = concat(pruneEquals(newStyleLemmatizeV3(word, word + len, buf, maxpos, 0, lemmas
#if PRINTRULE
                                                           , word
#endif
                                                           ), RulesUnique));
            }
        else
            {
            result = concat(pruneEquals(newStyleLemmatizeV3(word, word + len, buf, maxpos, 0, lemmas
#if PRINTRULE
                                                           , word
#endif
                                                           ), RulesUnique));
            }
        }
    else
#if LEMMATIZEV0
        {
        oldStyleLemmatize(word, word + len
                          , buf
                          , maxpos
#if TESTING
                          , Start, Middle, End
#endif
        );
        }
#else
        {
        fprintf(stderr, "The rules %s have a deprecated structure.\nGenerate flexrules with a recent version of affixtrain or recompile cstlemma with LEMMATIZEV0 set to 1\n", flexFileName);
        exit(-1);
        }
#endif

#if TESTING
    char temp[1000];
    sprintf(temp, "%s\t%s%s*%s->%s", result, Start, Middle, End, replacement);
    int newresult = strlen(temp) + 1;
    delete[] result;
    result = new char[newresult];
    strcpy(result, temp);
#endif
    return result;
    }


const char * rules::applyRules(const char * word,bool SegmentInitial, bool RulesUnique)
    {
    if (buf)
        return apply(word, SegmentInitial, RulesUnique, buf, buf + buflen);
    return 0;
    }


const char * rules::applyRules(const char * word, const char * tag,bool SegmentInitial,bool RulesUnique)
    {
    if (buf)
        {
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
                return apply(word, SegmentInitial, RulesUnique, Rules->Buf(), Rules->Buf() + Rules->end());
            }

        return apply(word, SegmentInitial, RulesUnique, buf, buf + buflen);
        }
    return 0;
    }

#endif
