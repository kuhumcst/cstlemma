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

#include "option.h"
#if (defined PROGLEMMATISE) || (defined PROGMAKEDICT)
#include "freqfile.h"
#endif
#include "caseconv.h"
#include "argopt.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#if STREAM
#include <iostream>
using namespace std;
#endif

#ifdef COUNTOBJECTS
int optionStruct::COUNT = 0;
#endif

#if defined PROGLEMMATISE
const char optionStruct::DefaultSep[] = "|";
//const char DefaultCFormat[] = "$w\\t[$b]1[$b0$B][$b>1$B]\\t$t\\n";
const char optionStruct::DefaultCFormat[] = "$w\\t$b1[[$b?]~1$B]\\t$t\\n";
const char optionStruct::DefaultCFormat_NoDict[] = "$w\\t$B\\t$t\\n";
//const char DefaultCFormat_NoTags[] = "$w\\t[$b]1[$b0$B][$b>1$B]\\n";
const char optionStruct::DefaultCFormat_NoTags[] = "$w\\t$b1[[$b?]~1$B]\\n";
const char optionStruct::DefaultCFormat_NoTags_NoDict[] = "$w\\t$B\\n";
const char optionStruct::DefaultCFormatXML[] = "$b1[[$b?]~1$B]";
const char optionStruct::DefaultCFormatXML_NoDict[] = "$B";
const char optionStruct::Default_b_format[] = "$w";
const char * optionStruct::Default_B_format = optionStruct::Default_b_format;
#endif

static char opts[] = "?@:A:b:B:c:C:d:De:f:FH:hi:I:k:l:Lm:n:N:o:p:q:R:s:t:u:U:v:W:x:X:y:z:" /* GNU: */ "wr";
static char *** Ppoptions = NULL;
static char ** Poptions = NULL;
static int optionSets = 0;

char * dupl(const char * s)
    {
    char * d = new char[strlen(s) + 1];
    strcpy(d,s);
    return d;
    }

optionStruct::optionStruct()
    {
#if defined PROGLEMMATISE
    defaultbformat = true;
    defaultBformat = true;
    defaultCformat = true;
    dictfile = NULL;
    v = NULL;
    x = NULL;
    XML = false;
    ancestor = NULL; // if not null, restrict lemmatisation to elements that are offspring of ancestor
    element = NULL; // if null, analyse all PCDATA that is text
    wordAttribute = NULL; // if null, word is PCDATA
    POSAttribute = NULL; // if null, POS is PCDATA
    lemmaAttribute = NULL; // if null, Lemma is PCDATA
    lemmaClassAttribute = NULL; // if null, lemma class is PCDATA
    z = NULL;
#endif
#if (defined PROGLEMMATISE) || (defined PROGMAKESUFFIXFLEX)
    flx = NULL;
#endif
#if defined PROGLEMMATISE
    InputHasTags = true;
    keepPunctuation = 1;
    Sep = dupl(DefaultSep);
#endif
    whattodo = LEMMATISE;
    argi = NULL;
    argo = NULL;
    arge = NULL;
    cformat = NULL;//dupl(DefaultCFormat);
    nice = false;
#if defined PROGMAKEDICT
    CollapseHomographs = true;
    freq = NULL;
#endif
#if defined PROGLEMMATISE
    Wformat = NULL;
    bformat = NULL;//dupl(Default_b_format);
    Bformat = NULL;//dupl(Default_B_format);
    SortOutput = 0;
    RulesUnique = true;
    DictUnique = true;
    //Iformat = dupl("$w/$t");
    Iformat = 0;
    UseLemmaFreqForDisambiguation = 0;
    baseformsAreLowercase = //false;/*20090731*///
        true;
    size = ULONG_MAX;
    treatSlashAsAlternativesSeparator = false;
#endif
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
#if defined PROGMAKESUFFIXFLEX
    showRefcount = false;
    CutoffRefcount = 0;
#endif
    }

optionStruct::~optionStruct()
    {
    for(int i = 0;i < optionSets;++i)
        {
        delete [] Poptions[i];
        delete [] Ppoptions[i];
        }
    delete [] Poptions;
    delete [] Ppoptions;
    delete [] cformat;
#if defined PROGLEMMATISE
    delete [] bformat;
    delete [] Bformat;
    delete [] Wformat;
    delete [] Iformat;
    delete [] Sep;
    delete [] ancestor;
    delete [] element;
    delete [] wordAttribute;
    delete [] POSAttribute;
    delete [] lemmaAttribute;
    delete [] lemmaClassAttribute;
#endif
#ifdef COUNTOBJECTS
    --COUNT;
#endif
    }

OptReturnTp optionStruct::doSwitch(int c,char * locoptarg,char * progname)
    {
    switch (c)
        {
        case '@':
            readOptsFromFile(locoptarg,progname);
            break;
#if defined PROGLEMMATISE
        case 'A':
            if(locoptarg && *locoptarg == '-')
                {
                treatSlashAsAlternativesSeparator = false;
                }
            else
                {
                treatSlashAsAlternativesSeparator = true;
                }	    
            break;
        case 'b':
            setbformat(locoptarg);
//            bformat = dupl(locoptarg); 
  //          defaultbformat = false;
            break;
        case 'B':
            setBformat(locoptarg);
//            Bformat = dupl(locoptarg); 
  //          defaultBformat = false;
            break;
#endif
        case 'c':
            cformat = dupl(locoptarg);
            defaultCformat = false;
            break;
#if defined PROGMAKESUFFIXFLEX
        case 'C':
            //CutoffRefcount = locoptarg == NULL  || *locoptarg != '-'; 
            if(!locoptarg || *locoptarg == '-')
                CutoffRefcount = 0;
            else
                CutoffRefcount = strtol(locoptarg,NULL,10);
            break;

            break;
#endif
#if defined PROGLEMMATISE
        case 'd':
            dictfile = locoptarg;
            break;
#endif
        case 'D':
            whattodo = MAKEDICT;
            break;
        case 'e':
            arge = locoptarg;
            switch(*arge)
                {
                case '0':
                case '1':
                case '2':
                case '7':
                case '9':
                    setEncoding(*arge - '0');
                    break;
                case 'u':
                case 'U':
                    setEncoding(ENUNICODE);
                    break;
                }
            break;
#if (defined PROGLEMMATISE) || (defined PROGMAKESUFFIXFLEX)
        case 'f':
            flx = locoptarg;
            break;
#endif
        case 'F':
            whattodo = MAKEFLEXPATTERNS;
            break;
        case 'h':
        case '?':
            LOG1LINE("usage:\n============================");
#if defined PROGMAKEDICT
            LOG1LINE("    Create binary dictionary");
#if STREAM
            cout << progname << " -D \\" << endl;
#else
            printf("%s -D \\\n",progname);
#endif
            LOG1LINE("         -c<format> [-N<frequency file> -n<format>] [-y[-]] \\\n"
                   "        [-i<lemmafile>] [-o<binarydictionary>]\n"
                   "    -c  column format of dictionary (tab separated), e.g. -cBFT, which means:\n"
                   "        1st column B(ase form), 2nd column F(ull form), 3rd column T(ype)\n"
                   "    -n  column format of frequency file (tab separated)\n"
                   "        Example: -nN?FT, which means:\n"
                   "        1st column N(frequency), 2nd column irrelevant,\n"
                   "        3rd column F(ull form), 4th column T(ype)\n"
                   "    -y  test output\n    -y- release output (default)\n"
                   "    -k  collapse homographs (remove \",n\" endings)(default)\n"
                   "    -k- do not collapse homographs (keep \",n\" endings)\n"
                   "===============================");
#endif
#if defined PROGMAKESUFFIXFLEX
            LOG1LINE("    Create or add flex patterns");
#if STREAM
            cout << progname << " -F \\" << endl;
#else
            printf("%s -F \\\n",progname);
#endif
            LOG1LINE("         -c<format> [-y[-]] [-i<lemmafile>] \\\n"
                   "        [-f<old flexpatterns>] [-o<new flexpatterns>]\n"
                   "    -c  column format, e.g. -cBFT, which means:\n"
                   "        1st column B(aseform), 2nd column F(ullform), 3rd column T(ype)\n"
                   "        For lemmatising untagged text, suppress lexical type information by\n"
                   "        specifying '?' for the column containing the type.\n"
                   "    -y  test output\n    -y- release output (default)\n"
                   "    -R- Do not append refcount to base form (default)\n"
                   "    -R  Append refcount to base form (format: [<base form>#<refcount>])\n"
                   "    -C- Include all rules in output (default)\n"
                   "    -C<n> Do not include rules with refcount <= <n>\n"
                   "=============");
#endif
#if defined PROGLEMMATISE
            LOG1LINE("    Lemmatise\n");
#if STREAM
            cout << progname << " [-L] \\" << endl;
#else
            printf("%s [-L] \\\n",progname);
#endif
            LOG1LINE("         -f<flex patterns> [-d<binary dictionary>] [-u[-]] [-v[-]] \\\n"
                   "         [-I<input format>] [-i<input text>] [-o<output text>] \\\n"
                   "         [-c<format>] [-b<format>] [-B<format>] [-W<format>] [-s[<sep>]] \\\n"
                   "         [-x<Lexical type translation table>] [-v<tag friends file>] \\\n"
                   "         [-z<type conversion table>] [-@<option file>]\n"
                   "    -i<input text>\tIf -t- defined: any flat text. Otherwise: words must be\n");
#if STREAM
            LOG1LINE("        followed by tags, separated by '/'. Default: standard input, one line at a time.\n");
#else
            LOG1LINE("        followed by tags, separated by '/'. To use standard input, recompile\n"
                   "        with #define STREAM 1 in defines.h.\n");
#endif
            LOG1LINE("    -I<format>\tInput format (if not word/tag (-t) or word (-t-) or option -p).\n" 
                   "        $w word to be lemmatised\n" 
                   "        $t tag\n" 
                   "        $d dummy\n" 
                   "        \\t tab\n" 
                   "        \\n new line\n" 
                   "        \\s white space\n" 
                   "        \\S all except white space\n"
                   "    -o<output text>\tOutput format dependent on -b, -B, -c and -W arguments.\n"
                   "        Default output: standard output\n"
                   "    -d<binarydictionary>\tDictionary as produced with the -D option set.\n"  
                   "        If no dictionary is specified, only the flex patterns are used.\n"  
                   "        Without dictionary, wrong tags in the input can not be corrected.\n"
                   "    -f<flexpatterns>\tFile with flex patterns. (see -F). Best results for\n"
                   "        untagged input are obtained if the rules are made without lexical type\n"
                   "        information. See -c option above.");  
#if STREAM
            cout << "    -b<format string>\tdefault:" commandlineQuote << Default_b_format << commandlineQuote << endl;  
#else
            printf( "    -b<format string>\tdefault:" commandlineQuote "%s" commandlineQuote "\n",Default_b_format);  
#endif
            LOG1LINE("        Output format for data pertaining to the base form, according to the\n"
                   "        dictionary:\n"
                   "        $f sum of frequencies of the words $W having the base form $w\n"
                   "           (lemmafrequency).");
#if FREQ24
            LOG1LINE("        $n frequency of the full form $w/$t in \"standard\" corpus.");
#endif
            LOG1LINE("        $t lexical type\n"
                   "        $w base form\n"
                   "        $W full form(s)\n"
                   "        \\$ dollar\n"
                   "        \\[ [\n"
                   "        \\] ]\n"
                   "        Example: -b" commandlineQuote "$f $w/$t" commandlineQuote);
#if STREAM
            cout <<  "    -B<format string>\tdefault:" commandlineQuote << Default_B_format << commandlineQuote << endl;  
#else
            printf("    -B<format string>\tdefault:" commandlineQuote "%s" commandlineQuote "\n",Default_B_format);  
#endif
            LOG1LINE("        Output format for data pertaining to the base form, as predicted by\n"
                   "        flex pattern rules. See -b\n"
                   "    -W<format string>\tdefault: not present.\n"
                   "        Output format:\n"
                   "        $w full form\n"
                   "        $t lexical type(s) according to dictionary\n"
                   "        $f full form type frequency\n"
                   "        $i info:  -    full form not in dictionary\n"
                   "                  +    full form in dictionary, but other type\n"
                   "               (blank) full form in dictionary\n"
                   "        \\t tab\n"
                   "        $X?, [X]? Do not output X. (X can be tested, though).\n"
                   "        [X]+  Output X only if X occurs at least once. (X is an expression\n"
                   "              containing $b or $B)\n"
                   "        [X]>n Output X only if X occurs more than n times.\n"
                   "        [X]n  Output X only if X occurs exactly n times.\n"
                   "        [X]<n Output X only if X occurs less than n times.\n"
                   "        [X]   Output X if all nested conditions are met, or if X occurs\n"
                   "              at least once. ([X] itself is always met!)\n"
                   "        Example: -b" commandlineQuote "$w ($W)[>1[$W?]>1]" commandlineQuote "\n"
                   "                 -W" commandlineQuote "$w\\n" commandlineQuote "\n"
                   "                (Output lemma (full form|full form..)>1\n"
                   "                 if different words have same base form)");
#if STREAM
            cout <<  "    -c<format string>\tdefault:\t" commandlineQuote << DefaultCFormat << commandlineQuote << endl;
#else
            printf("    -c<format string>\tdefault:\t" commandlineQuote "%s" commandlineQuote "\n",DefaultCFormat);// word/lemma/tag lemma: if dictionary gives 1 solution, take dictionary, otherwise rules
#endif
            LOG1LINE("        Output format:\n"
                   "        $w full form\n"
                   "        $b base form(s) according to dictionary.\n"
                   "           (You also need to specify -b<format>)\n"
                   "           (If the full form is found in the dictionary and tag=lexical type,\n"
                   "            then only one base form is output.\n"
                   "            Otherwise all base forms are output)\n"
                   "        $B base form(s) according to flex pattern rules\n"
                   "           (You also need to specify -B<format>)\n"
                   "           (only if full form not in dictionary, or in dictionary,\n"
                   "            but with other lexical type.)\n"
                   "        $s word separator: new line character when the current word is the last\n"
                   "           word before a line break, blank otherwise\n"
                   "        $t lexical type(s) according to dictionary\n"
                   "        $f full form frequency\n"
                   "        $i info: indicates - full form not in dictionary\n"
                   "                           + full form in dictionary, but other type\n"
                   "                           (blank) full form in dictionary\n"
                   "        \\t tab\n"
                   "        $X?, [X]? Do not output X. (X can be tested, though).\n"
                   "        $b and $B are variables: they can occur any number of times,\n"
                   "        including zero. This number can be tested in conditions:\n"
                   "        $bn   Output $b only if $b occurs exactly n-times (n >= 0).\n"
                   "        $Bn   Output $B only if $B occurs exactly n-times (n >= 0).\n"
                   "        [X]+  Output X only if X occurs at least once. (X is an expression\n"
                   "              containing $b or $B)\n"
                   "        [X]>n Output X only if X occurs more than n times.\n"
                   "        [X]n  Output X only if X occurs exactly n times.\n"
                   "        [X]<n Output X only if X occurs less than n times.\n"
                   "        [X]   Output X if all nested conditions are met, or if X occurs\n"
                   "              at least once. ([X] itself is always met!)\n"
                   "        Example: -c" commandlineQuote "[+$b?]>0[-$b0]$w\\n" commandlineQuote "\n"
                   "                 -b" commandlineQuote "$w\t/$t" commandlineQuote "\n"
                   "                (Output +lemma if the word is found in the dictionary,\n"
                   "                 otherwise -lemma)\n"
                   "    -l  force lemma to all-lowercase (default)\n"
                   "    -l- make case of lemma similar to full form's case\n"
                   "    -p  keep punctuation (default)\n"
                   "    -p- ignore punctuation (only together with -t- and no -W or -I)\n"
                   "    -p+ treat punctuation as tokens (only together with -t- and no -W or -I)\n"
                   "    -q  sort output\n"
                   "    -q- do not sort output (default)\n"
                   "    -q# (equivalents:-qn -qN -qf -qF)sort output by frequency\n"
                   "    -ql (equivalents:-qL -qw -qW)sort output by word (if -c) or lemma (if -W)\n"
                   "    -qp (equivalents:-qP -qt -qT)sort output by POS tag\n"
                   "          Combinations are allowed: -qwft means sort by word, by frequency and finally by POS tag.\n"
                   "    -s<sep> multiple base forms (-b -B) are <sep>-separated. Example: -s" commandlineQuote " | " commandlineQuote);
#if STREAM
            cout << "    -s  multiple base forms (-b -B) are " commandlineQuote << DefaultSep << commandlineQuote "-separated (default)" << endl;
#else
            printf("    -s  multiple base forms (-b -B) are " commandlineQuote "%s" commandlineQuote "-separated (default)\n",DefaultSep);
#endif
            LOG1LINE("    -t  input text is tagged (default)\n    -t- input text is not tagged\n"
                   "    -U  enforce unique flex rules (default)\n"
                   "    -U- allow ambiguous flex rules\n"
                   "    -u  enforce unique dictionary look-up (default)\n"
                   "    -u- allow ambiguous dictionary look-up\n"
                   "    -Hn n = 0: use lemma frequencies for disambiguation (default)\n"
                   "        n = 1: use lemma frequencies for disambiguation,\n"
                   "               show candidates for pruning between << and >>\n"
                   "        n = 2: do not use lemma frequencies for disambiguation.\n"
                   "    -v<tag friends file>: Use this to coerce the nearest fit between input\n"
                   "        tag and the dictionary's lexical types if the dictionary has more than\n"
                   "        one readings of the input word and none of these has a lexical type\n"
                   "        that exactly agrees with the input tag. Format:\n"
                   "             {<dict type> {<space> <tag>}* <newline>}*\n"
                   "        The more to the left the tag is, the better the agreement with the\n"
                   "        dictionary'e lexical type\n"
                   "    -x<Lexical type translation table>: Use this to handle tagged texts with\n"
                   "        tags that do not occur in the dictionary. Format:\n"
                   "             {<dict type> {<space> <tag>}* <newline>}*\n"
                   "    -z<type conversion table>: Use this to change the meaning of $t in -b and\n"
                   "        -B formats. Without conversion table, $t is the lexical type of the\n"
                   "        full form. With conversion table, $t is the lexical type of the base\n"
                   "        form, as defined by the table. Format:\n"
                   "             {<full form type> <space> <base form type> <newline>}*\n"
                   "    -m<size>: Max. number of words in input. Default: 0 (meaning: unlimited)\n"
                   "    -A  Treat / as separator between alternative words.\n"
                   "    -A- Do not treat / as separator between alternative words (default)\n"
                   "    -e<n> ISO8859 Character encoding. 'n' is one of 1,2,7 and 9 (ISO8859-1,2, etc).\n"
                   "    -eU Unicode (UTF8) input.\n"
                   "    -e  Don't use case conversion.\n"
                   "    -X  XML input. Leave XML elements unchanged.\n"
                   "    The next options do not allow space between option letters and argument!\n"
                   "    -Xa<ancestor>  Only analyse elements with specified ancestor. e.g -Xabody\n"
                   "    -Xe<element>  Only analyse specified element. e.g -Xew\n"
                   "    -Xw<word>  Words are to be found in attribute. e.g -Xwword\n"
                   "    -Xp<pos>  Words' POS-tags are to be found in attribute. e.g -Xppos\n"
                   "    -Xl<lemma>  Destination of lemma is the specified attribute. e.g -Xllemma\n"
                   "    -Xc<lemmaclass>  Destination of lemma class is the specified attribute. e.g -Xllemmaclass");
#endif
            return Leave;
#if defined PROGLEMMATISE
        case 'H':
            if(locoptarg)
                {
                UseLemmaFreqForDisambiguation = *locoptarg - '0';
                if(UseLemmaFreqForDisambiguation < 0 || UseLemmaFreqForDisambiguation > 2)
                    {
#if STREAM
                    cout << "-H option: specify -H0, -H1 or -H2 (found -H" << locoptarg << ")" << endl;
#else
                    printf("-H option: specify -H0, -H1 or -H2 (found -H%s)\n",locoptarg);
#endif
                    return Error;
                    }
                }
            else
                {   
                LOG1LINE("-H option: specify -H0, -H1 or -H2");
                return Error;
                }
            break;
#endif
        case 'i':
            argi = locoptarg;
            break;
#if defined PROGLEMMATISE
        case 'I':
            delete [] Iformat;
            Iformat = dupl(locoptarg); 
            break;
        case 'l':
            baseformsAreLowercase = !locoptarg || *locoptarg != '-';
            break;
#endif
        case 'L':
            whattodo = LEMMATISE; // default action
            break;
#if defined PROGLEMMATISE
        case 'm':
            if(locoptarg)
                {
                size = strtoul(locoptarg,NULL,10);
                LOGANDFLUSH("Max. number of words in input: ");
                if(size == 0)
                    {
                    size = ULONG_MAX;
                    LOG1LINE("Unlimited");
                    }
                else
                    {
#if STREAM
                    cout << size << endl;
#else
                    printf("%lu\n",size);
#endif
                    }
                }
            else
                size = ULONG_MAX;
            break;
#endif
#if defined PROGMAKEDICT
        case 'k':
            CollapseHomographs = locoptarg == NULL || *locoptarg != '-';
            break;
        case 'n':
                {
                if(!freq)
                    {
                    freq = new FreqFile();
                    }
                (freq)->addFormat(locoptarg);
                }
            break;
        case 'N':
                {
                if(!freq)
                    {
                    freq = new FreqFile();
                    }
                (freq)->addName(locoptarg);
                }
            break;
#endif
        case 'o':
            argo = locoptarg;
            break;
#if defined PROGLEMMATISE
        case 'p':
            if(locoptarg)
                {
                if(*locoptarg == '-')
                    {
                    keepPunctuation = 0;
                    }
                else if(*locoptarg == '+')
                    {
                    keepPunctuation = 2;
                    }
                else if(*locoptarg == '\0')
                    {
                    keepPunctuation = 1;
                    }
                else
                    {
#if STREAM
                    cout << "Invalid argument " << locoptarg << "for -p option." << endl;
#else
                    printf("Invalid argument %s for -p option.\n",locoptarg);
#endif
                    return Error;
                    }
                }
            else
                {
                keepPunctuation = 1;
                }
            break;
        case 'q':
            if(!locoptarg)
                {
                static char whash[] = "w#";
                locoptarg = whash; // 20120710 assigning constant string generatates warning in newer gcc
                }
            else if(*locoptarg == '-')
                {
                SortOutput = 0;
                break;
                }

            SortOutput = 0;
            while(*locoptarg)
                {
                SortOutput <<= 2;
                switch(*locoptarg)
                    {
                    case '#':
                    case 'f':
                    case 'F':
                    case 'n':
                    case 'N':
                        SortOutput += SORTFREQ;
                        break;
                    case 'l':
                    case 'L':
                    case 'w':
                    case 'W':
                        SortOutput += SORTWORD;
                        break;
                    case 'p':
                    case 'P':
                    case 't':
                    case 'T':
                        SortOutput += SORTPOS;
                        break;
                    default:
                        SortOutput = SORTWORD;
                        break;
                    }
                ++locoptarg;
                }
            break;
#endif
// GNU >>
        case 'r':
            LOG1LINE("12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n"
            "WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n"
            "REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n"
            "INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n"
            "OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n"
            "TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n"
            "YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n"
            "PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n"
            "POSSIBILITY OF SUCH DAMAGES.\n");
            return Leave;
// << GNU
#if defined PROGMAKESUFFIXFLEX
        case 'R':
            showRefcount = locoptarg == NULL  || *locoptarg != '-';
            break;
#endif
#if defined PROGLEMMATISE
        case 's':
            if(locoptarg && *locoptarg)
                {
                for(char * p = locoptarg;*p;)
                    {
                    if(*p == '\\')
                        {
                        switch(*(p + 1))
                            {
                            case 't':
                                *p++ = '\t';
                                memmove(p,p+1,strlen(p));
                                break;
                            case 'n':
                                *p++ = '\n';
                                memmove(p,p+1,strlen(p));
                                break;
                            default:
                                *p = *(p+1);
                                ++p;
                                memmove(p,p+1,strlen(p));
                                break;
                            }
                        }
                    else
                        ++p;
                    }
                Sep = dupl(locoptarg);
                }
            else
                Sep = dupl(DefaultSep);
            break;
        case 't':
            InputHasTags = locoptarg == NULL || *locoptarg != '-';
            break;
        case 'u':
            DictUnique = locoptarg == NULL  || *locoptarg != '-';
            break;
        case 'U':
            RulesUnique = locoptarg == NULL  || *locoptarg != '-';
            break;
        case 'v':
            v = locoptarg;
            break;
#endif
// GNU >>
        case 'w':
            LOG1LINE("11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n"
            "FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n"
            "OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n"
            "PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n"
            "OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n"
            "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n"
            "TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n"
            "PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n"
            "REPAIR OR CORRECTION.");
            return Leave;
// << GNU
#if defined PROGLEMMATISE
        case 'W':
            Wformat = dupl(locoptarg);
            break;
        case 'x':
            x = locoptarg;
            break;
        case 'X':
            if(locoptarg)
                {
                if(*locoptarg == '-')
                    {
                    XML = false;
                    }
                else
                    {
                    XML = true;
                    switch(*locoptarg)
                        {
                        case 'a':
                            ancestor = dupl(locoptarg+1);
                            break;
                        case 'e':
                            element = dupl(locoptarg+1);
                            break;
                        case 'w':
                            wordAttribute = dupl(locoptarg+1);
                            break;
                        case 'p':
                            POSAttribute = dupl(locoptarg+1);
                            break;
                        case 'l':
                            lemmaAttribute = dupl(locoptarg+1);
                            if(defaultCformat)
                                {
                                if(Bformat)
                                    setcformat(DefaultCFormatXML);
                                }
                            break;
                        case 'c':
                            lemmaClassAttribute = dupl(locoptarg+1);
                            break;
                        }
                    }
                }
            else
                XML = true;
            break;
        case 'z':
            z = locoptarg;
            break;
#endif
        case 'y':
            nice = locoptarg == NULL  || *locoptarg != '-';
            break;
        }
    return GoOn;
    }



OptReturnTp optionStruct::readOptsFromFile(char * locoptarg,char * progname)
    {
    char ** poptions;
    char * options;
    FILE * fpopt = fopen(locoptarg,"r");
    OptReturnTp result = GoOn;
    if(fpopt)
        {
        char * p;
        char line[1000];
        int lineno = 0;
        size_t bufsize = 0;
        while(fgets(line,sizeof(line) - 1,fpopt))
            {
            lineno++;
            size_t off = strspn(line," \t");
            if(line[off] == ';')
                continue; // comment line
            if(line[off] == '-')
                {
                off++;
                if(line[off])
                    {
                    char * optarg2 = line + off + 1;
                    size_t off2 = strspn(optarg2," \t");
                    if(!optarg2[off2])
                        optarg2 = NULL;
                    else
                        optarg2 += off2;
                    if(optarg2)
                        {
                        for(p = optarg2 + strlen(optarg2) - 1;p >= optarg2;--p)
                            {
                            if(!isspace(*p))
                                break;
                            *p = '\0';
                            }
                        bool string = false;
                        if(*optarg2 == '\'' || *optarg2 == '"')
                            {

                            // -x 'jhgfjhagj asdfj\' hsdjfk' ; dfaasdhfg
                            // -x 'jhgfjhagj asdfj\' hsdjfk' ; dfa ' asdhfg
                            // -x "jhgfjhagj \"asdfj hsdjfk" ; dfaasdhfg
                            // -x "jhgfjhagj \"asdfj hsdjfk" ; dfa " asdhfg
                            for(p = optarg2 + strlen(optarg2) - 1;p > optarg2;--p)
                                {
                                if(*p == *optarg2)
                                    {
                                    string = true;
                                    for(char * q = p + 1;*q;++q)
                                        {
                                        if(*q == ';')
                                            break;
                                        if(!isspace(*q))
                                            {
                                            string = false;
                                            }
                                        }
                                    if(string)
                                        {
                                        *p = '\0';
                                        ++optarg2;
                                        }
                                    break;
                                    }
                                }
                            }
                        if(!*optarg2 && !string)
                            optarg2 = NULL;
                        }
                    if(optarg2)
                        {
                        bufsize += strlen(optarg2) + 1;
                        }
                    char * optpos = strchr(opts,line[off]);
                    if(optpos)
                        {
                        if(optpos[1] != ':')
                            {
                            if(optarg2)
                                {
#if STREAM
                                cout << "Option argument " << optarg2 << " provided for option letter " << line[off] << " that doesn't use it on line " << lineno << " in option file \"" << locoptarg << "\"" << endl;
#else
                                printf("Option argument %s provided for option letter %c that doesn't use it on line %d in option file \"%s\"\n",optarg2,line[off],lineno,locoptarg);
#endif
                                exit(1);
                                }
                            }
                        }
                    }
                else
                    {
#if STREAM
                    cout << "Missing option letter on line " << lineno << " in option file \"" << locoptarg << "\"" << endl;
#else
                    printf("Missing option letter on line %d in option file \"%s\"\n",lineno,locoptarg);
#endif
                    exit(1);
                    }
                }
            }
        rewind(fpopt);

        poptions = new char * [lineno];
        options = new char[bufsize];
        // update stacks that keep pointers to the allocated arrays.
        optionSets++;
        char *** tmpPpoptions = new char **[optionSets];
        char ** tmpPoptions = new char *[optionSets];
        int g;
        for(g = 0;g < optionSets - 1;++g)
            {
            tmpPpoptions[g] = Ppoptions[g];
            tmpPoptions[g] = Poptions[g];
            }
        tmpPpoptions[g] = poptions;
        tmpPoptions[g] = options;
        delete [] Ppoptions;
        Ppoptions = tmpPpoptions;
        delete [] Poptions;
        Poptions = tmpPoptions;

        lineno = 0;
        bufsize = 0;
        while(fgets(line,sizeof(line) - 1,fpopt))
            {
            poptions[lineno] = options+bufsize;
            size_t off = strspn(line," \t");
            if(line[off] == ';')
                continue; // comment line
            if(line[off] == '-')
                {
                off++;
                if(line[off])
                    {
                    char * optarg2 = line + off + 1;
                    size_t off2 = strspn(optarg2," \t");
                    if(!optarg2[off2])
                        optarg2 = NULL;
                    else
                        optarg2 += off2;
                    if(optarg2)
                        {
                        for(p = optarg2 + strlen(optarg2) - 1;p >= optarg2;--p)
                            {
                            if(!isspace(*p))
                                break;
                            *p = '\0';
                            }
                        bool string = false;
                        if(*optarg2 == '\'' || *optarg2 == '"')
                            {

                            // -x 'jhgfjhagj asdfj\' hsdjfk' ; dfaasdhfg
                            // -x 'jhgfjhagj asdfj\' hsdjfk' ; dfa ' asdhfg
                            // -x "jhgfjhagj \"asdfj hsdjfk" ; dfaasdhfg
                            // -x "jhgfjhagj \"asdfj hsdjfk" ; dfa " asdhfg
                            for(p = optarg2 + strlen(optarg2) - 1;p > optarg2;--p)
                                {
                                if(*p == *optarg2)
                                    {
                                    string = true;
                                    for(char * q = p + 1;*q;++q)
                                        {
                                        if(*q == ';')
                                            break;
                                        if(!isspace(*q))
                                            {
                                            string = false;
                                            }
                                        }
                                    if(string)
                                        {
                                        *p = '\0';
                                        ++optarg2;
                                        }
                                    break;
                                    }
                                }
                            }
                        if(!*optarg2 && /*allow empty string for e.g. -s option*/!string)
                            optarg2 = NULL;
                        }
                    if(optarg2)
                        {
                        strcpy(poptions[lineno],optarg2);
                        bufsize += strlen(optarg2) + 1;
                        }
                    else
                        {
                        strcpy(poptions[lineno],"");
                        //++bufsize;
                        }

                    /*else
                        optarg2 = "";
                    char * optpos = strchr(opts,line[off]);*/
                    OptReturnTp res = doSwitch(line[off],poptions[lineno],progname);
                    if(res > result)
                        result = res;
                    }
                }
            lineno++;
            }
        fclose(fpopt);
        }
    else
        {
#if STREAM
        cout << "Cannot open option file " << locoptarg << endl;
#else
        printf("Cannot open option file %s\n",locoptarg);
#endif
        }
    return result;
    }

OptReturnTp optionStruct::readArgs(int argc, char * argv[])
    {
    int c;
#if defined PROGLEMMATISE
    SortOutput = 0;
    Wformat = NULL;
#endif
    OptReturnTp result = GoOn;
    while((c = getopt(argc,argv, opts)) != -1)
        {
        OptReturnTp res = doSwitch(c,myoptarg,argv[0]);
        if(res > result)
            result = res;
        }
    if(this->arge == NULL)
        {
        setEncoding(0);
        }
    return result;
    }

#if defined PROGLEMMATISE
void optionStruct::setIformat(const char * format)  // -I
    {
    delete [] Iformat;
    Iformat = dupl(format);
    }

void optionStruct::setBformat(const char * format)  // -B
    {
    delete [] Bformat;
    Bformat = dupl(format);
    defaultBformat = format == Default_B_format;
    }

void optionStruct::setbformat(const char * format)  // -b
    {
    delete [] bformat;
    bformat = dupl(format);
    defaultbformat = format == Default_b_format;
    }
#endif

#if defined PROGLEMMATISE
void optionStruct::setcformat(const char * format)  // -c
    {
    delete [] cformat;
    cformat = dupl(format);
#if defined PROGLEMMATISE
    defaultCformat = 
           format == DefaultCFormat 
        || format == DefaultCFormat_NoTags 
        || format == DefaultCFormatXML
        || format == DefaultCFormatXML_NoDict;
#endif
    }
#endif

#if defined PROGLEMMATISE

void optionStruct::setWformat(const char * format)  // -W
    {
    delete [] Wformat;
    Wformat = dupl(format);
    }

void optionStruct::setSep(const char * format)      // -s
    {
    delete [] Sep;
    Sep = dupl(format);
    }


void optionStruct::setUseLemmaFreqForDisambiguation(int n)    // -H 0, 1 or 2
    {
    assert(0 <= n && n <= 2);
    UseLemmaFreqForDisambiguation = n;
    }

void optionStruct::setkeepPunctuation(bool b)
    {
    keepPunctuation = b;
    }

void optionStruct::setsize(unsigned long int n)
    {
    size = n;
    }

void optionStruct::settreatSlashAsAlternativesSeparator(bool b)
    {
    treatSlashAsAlternativesSeparator = b;
    }

void optionStruct::setUseLemmaFreqForDisambiguation(bool b)
    {
    UseLemmaFreqForDisambiguation = b;
    }

void optionStruct::setDictUnique(bool b)
    {
    DictUnique = b;
    }

void optionStruct::setbaseformsAreLowercase(bool b)
    {
    baseformsAreLowercase = b;
    }

#endif
