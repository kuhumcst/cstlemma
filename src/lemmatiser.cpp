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
#include "lemmatiser.h"
#if defined PROGLEMMATISE
#include "applyrules.h"
#include "tags.h"
#endif
#include "option.h"
#if defined PROGMAKEDICT 
#include "makedict.h"
#endif

#if (defined PROGMAKESUFFIXFLEX || defined PROGLEMMATISE)
#include "flex.h"
#endif
#if defined PROGLEMMATISE
#include "basefrm.h"
#include "XMLtext.h"
#include "flattext.h"
#include "lemmtags.h"
#ifdef _MSC_VER
#include  <io.h>
#endif
#endif
#if (defined PROGMAKEDICT || defined PROGLEMMATISE)
#include <stdarg.h>
#include <time.h>
#include <limits.h>
#endif

#ifdef COUNTOBJECTS
int Lemmatiser::COUNT = 0;
#endif


#if STREAM
# include <fstream>
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



#if defined PROGLEMMATISE
static bool DoInfo = true;
tagpairs * Lemmatiser::TextToDictTags = 0;
#endif
int Lemmatiser::instance = 0;

#if defined PROGLEMMATISE
static int info(const char *fmt, ...)
    {
    if(DoInfo)
        {
        int ret;
        va_list ap;
        va_start(ap,fmt);
        ret = vprintf(fmt,ap);
        va_end(ap);
        printf("\n");
        return ret;
        }
    return 0;
    }


static void showtime(clock_t t0)
    {
    clock_t t1;
    unsigned long span,sec,msec;
    t1 = clock();
    span = t1 - t0;
    sec = span / CLOCKS_PER_SEC;
/*
    span *= 1000;
    msec = span / CLOCKS_PER_SEC - sec * 1000;
*/
    span -= sec * CLOCKS_PER_SEC;
    span *= 1000;
    msec = span / CLOCKS_PER_SEC/* - sec * 1000*/;
    info("\nTime: %ld.%.3ld",sec,msec);
    }



const char * Lemmatiser::translate(const char * tag)
    {
    return TextToDictTags ? TextToDictTags->translate(tag) : tag; // tag as found in the text
    }
#endif

Lemmatiser::Lemmatiser(optionStruct & a_Option) : listLemmas(0),SortInput(false),Option(a_Option),changed(true)
    {
    instance++;
    if(instance == 1)
        {
#if defined PROGLEMMATISE
        if(!Option.argo && !Option.argi)
            DoInfo = false; // suppress messages when using stdin and stdout
#endif
        switch(Option.whattodo)
            {
            case MAKEDICT:
                {
#if defined PROGMAKEDICT
                status = MakeDict();
#endif
                break;
                }
            case MAKEFLEXPATTERNS:
                {
#if defined PROGMAKESUFFIXFLEX
                status = MakeFlexPatterns();
#endif
                break;
                }
            default:
                {
#if defined PROGLEMMATISE
                status = LemmatiseInit();
#endif
                break;
                }
            }
        }
    else
        status = -3; // Only one instance of Lemmatiser allowed.
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }

Lemmatiser::~Lemmatiser()
    {
    instance--;
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
            LemmatiseEnd();
#endif
            }
        }
#ifdef COUNTOBJECTS
    --COUNT;
#endif
    }

#if defined PROGMAKEDICT
int Lemmatiser::MakeDict()
    {
    FILE * fpin;
    FILE * fpout;
    FILE * ffreq = 0;
    if(!Option.cformat)
        {
        printf("You need to specify a column-order with the -c option\n");
        return -1;
        }
    if(Option.argi)
        {
        fpin = fopen(Option.argi,"r");
        if(!fpin)
            {
            printf("Cannot open input file \"%s\" for reading\n",Option.argi);
            return -1;
            }
        }
    else
        fpin = stdin;

    if(Option.argo)
        {
        fpout = fopen(Option.argo,"wb");
        if(!fpout)
            {
            printf("Cannot open binary dictionary \"%s\" for writing\n",Option.argo);
            return -1;
            }
        }
    else
        fpout = stdout;
/*
    if(freq)
        {
        if(!fformat)
            {
            printf("Please specify the format of the frequency file with the -n option\n");
            }
        ffreq = fopen(freq,"r");
        if(!ffreq)
            {
            printf("Cannot open frequency file \"%s\" for reading\n",argo);
            return -1;
            }
        }
*/
    int ret = makedict(fpin,fpout,Option.nice,Option.cformat,Option.freq,Option.CollapseHomographs);
    if(fpin != stdin)
        fclose(fpin);
    if(fpout != stdout)
        fclose(fpout);
    if(ffreq)
        fclose(ffreq);
    return ret;
    }
#endif

#if defined PROGMAKESUFFIXFLEX
int Lemmatiser::MakeFlexPatterns()
    {
    FILE * fpdict;
    FILE * fpflex;
    if(!Option.cformat)
        {
        printf("You need to specify a column-order with the -c option\n");
        return -1;
        }


    if(Option.argi)
        {
        fpdict = fopen(Option.argi,"r");
        if(!fpdict)
            return -1;
        }
    else
        fpdict = stdin;

    if(Option.flx)
        {
        fpflex = fopen(Option.flx,"rb");
        if(fpflex)
            {
            Flex.readFromFile(fpflex);
            fclose(fpflex);
            }
        }
    if(Option.argo)
        {
        fpflex = fopen(Option.argo,"wb");
        if(!fpflex)
            return -1;
        }
    else
        fpflex = stdout;

    int failed;
    Flex.makeFlexRules
        (fpdict
        ,fpflex
        ,Option.nice
        ,Option.cformat
        ,failed
        ,Option.CutoffRefcount
        ,Option.showRefcount
        ,Option.argo
        );
    if(fpdict != stdin)
        fclose(fpdict);

    if(fpflex != stdout)
        fclose(fpflex);
    return 0;
    }
#endif

#if defined PROGLEMMATISE
int Lemmatiser::setFormats()
    {
    info("\nFormats:");
    listLemmas = 0;
    if(Option.Iformat)
        {
        info("-I\t%s\tInput format.",Option.Iformat);
        }
    if(!Option.Bformat && !Option.bformat && !Option.cformat && !Option.Wformat)
        {
        Option.setBformat(optionStruct::Default_B_format);
        if(Option.dictfile)
            {
            Option.setbformat(optionStruct::Default_b_format);
            Option.setcformat(
                  Option.XML          ? optionStruct::DefaultCFormatXML 
                : Option.InputHasTags ? optionStruct::DefaultCFormat 
                :                       optionStruct::DefaultCFormat_NoTags);
            }
        else
            {
            Option.setcformat(
                  Option.XML          ? optionStruct::DefaultCFormatXML_NoDict
                : Option.InputHasTags ? optionStruct::DefaultCFormat_NoDict 
                :                       optionStruct::DefaultCFormat_NoTags_NoDict);
            }
        }
    if(Option.Defaultbformat() && !Option.DefaultBformat())
        {
        delete [] Option.bformat;
        Option.bformat = 0;
        }
    else if(Option.DefaultBformat() && !Option.Defaultbformat())
        {
        delete [] Option.Bformat;
        Option.Bformat = 0;
        }
    else if(Option.DefaultBformat() && Option.Defaultbformat() && Option.Wformat)
        {
        printf("You need to specify -b or -B formats if you specify the -W format\n");
        return -1;
        }
    /*if(!cformat && !Wformat)
        {
//                cformat = "iwbBtTfF"; // TODO THIS IS A HACK
        printf("You need to specify an output cformat with the -c (or -W plus -b or -B) option\n");
        return -1;
        }
    else*/ if(!Option.DefaultCformat() && Option.Wformat)
        {
//                cformat = "iwbBtTfF"; // TODO THIS IS A HACK
        printf("You cannot specify both -c and -W format options\n");
        return -1;
        }
    else if(Option.cformat && !Option.Wformat)
        {
        /* 20120807
        if(!Option.InputHasTags && Option.DefaultCformat())
            Option.setcformat(Option.DefaultCFormat_NoTags);
            */
        info("-c\t%s\tOutput format",Option.cformat);
        if(Option.bformat)
            info("-b\t%s\tDictionary base form output format.",Option.bformat);
        if(Option.Bformat)
            info("-B\t%s\tComputed base form output format.",Option.Bformat);
        listLemmas = 0;
        }
    else
        {
        if(Option.bformat)
            {
            info("-b\t%s\tOutput format for data pertaining to the base form, according to the dictionary",Option.bformat);
            listLemmas |= 1;
            }
        if(Option.Bformat)
            {
            listLemmas |= 2;
            info("-B\t%s\tOutput format for data pertaining to the base form, as predicted by flex pattern rules.",Option.Bformat);
            }
        if(!listLemmas)
            {
            printf("You must specify at least one of -b and -B if you do not specify -c.\n");
            return -1;
            }
//                format = Wformat;
        if(Option.Wformat)
            info("-W\t%s\tOutput format for data pertaining to full forms.",Option.Wformat);
        }
    if(listLemmas)
        {
        SortInput = basefrm::setFormat(Option.Wformat,Option.bformat,Option.Bformat,Option.InputHasTags);
        if(SortInput)
            info("Input is sorted before processing (due to $f field in -W<format> argument)",Option.flx);
        }
    else
        {
        SortInput = text::setFormat(Option.cformat,Option.bformat,Option.Bformat,Option.InputHasTags);
        if(SortInput)
            info("Input is sorted before processing (due to $f field in -c<format> argument)",Option.flx);
        }
    if(!SortInput)
        {
        if(listLemmas || Option.UseLemmaFreqForDisambiguation < 2)
            SortInput = true;// performance

        }
    if(!Option.XML)
        info("-X-\tNot XML input.");
    else
        {
        info("-X\tXML-aware scanning of input");
        if(Option.ancestor)
            info("-Xa%s\tOnly analyse elements with ancestor %s",Option.ancestor,Option.ancestor);
        if(Option.element)
            info("-Xe%s\tOnly analyse elements %s",Option.element,Option.element);
        if(Option.wordAttribute)
            {
            info("-Xw%s\tLook for word in attribute %s",Option.wordAttribute,Option.wordAttribute);
            if(!Option.lemmaAttribute)
                info("\tStore lemma in attribute %s",Option.wordAttribute,Option.wordAttribute);
            }
        if(Option.POSAttribute)
            {
            if(Option.InputHasTags)
                info("-Xp%s\tLook for Part of Speech in attribute %s",Option.POSAttribute,Option.POSAttribute);
            else
                info("-Xp%s\tLook for Part of Speech in attribute %s (ignored because of option -t-)",Option.POSAttribute,Option.POSAttribute);
            }
        if(Option.lemmaAttribute)
            info("-Xl%s\tStore lemma in attribute %s",Option.lemmaAttribute,Option.lemmaAttribute);
        if(Option.lemmaClassAttribute)
            info("-Xc%s\tStore lemma class in attribute %s",Option.lemmaClassAttribute,Option.lemmaClassAttribute);
        }
    changed = false;
    return 0;
    }

int Lemmatiser::openFiles()
    {
    FILE * fpflex;
    FILE * fpdict = 0;
    FILE * fpv= 0;
    FILE * fpx = 0;
    FILE * fpz = 0;
    info("\nFiles:");
    if(Option.flx)
        {
        fpflex = fopen(Option.flx,"rb");
        if(fpflex)
            {
            info("-f\t%-20s\tFlexpatterns",Option.flx);
            }
        else if(Option.InputHasTags)
            {
            // 20110114 Check that folder exists
            char * tmp = new char[strlen(Option.flx) + 1];
            strcpy(tmp,Option.flx);
            char * slash = tmp + strlen(tmp);
            while(--slash >= tmp)
#ifdef _MSC_VER // VC
                if(*slash == '\\')
                    {
                    *slash = '\0';
                    /*
                    modes are as follows...
                    00 Existence only
                    02 Write permission
                    04 Read permission
                    06 Read and write permission
                    */
                    if(_access((*tmp ? tmp : "."),0) == -1)
                        {
                        info("-f\t%-20s\t(Flexpatterns): Folder %s does not exist or is not readable. Cannot open flex pattern files.",Option.flx,tmp);
                        delete [] tmp;
                        return -1;
                        }
                    break;
                    }
#else // *N?X
                if(*slash == '/')
                    {
                    *slash = '\0';
                    FILE * d = fopen((*tmp ? tmp : "."),"r");
                    if(d == NULL)
                        {
                        info("-f\t%-20s\t(Flexpatterns): Folder %s does not exist or is not readable. Cannot open flex pattern files.",Option.flx,tmp);
                        delete [] tmp;
                        return -1;
                        }
                    else
                        fclose(d);
                    break;
                    }
#endif
            /*
                /
                ./
                folder/
                /flexrules
                ./flexrules
                folder/flexrules
                flexrules
            */
            info("-f\t%-20s\t(Flexpatterns): Cannot open file. Assuming that tag-specific files exist in folder %s with prefix %s."
                ,Option.flx
                ,(slash < tmp) ? "." : tmp
                ,slash + 1
                );
            delete [] tmp;
            }
        else
            {
            info("-f\t%-20s\t(Flexpatterns): Cannot open file.",Option.flx);
            return -1;
            }
        }
    else
        {
        printf("-f  Flexpatterns: File not specified.\n");
        return -1;
        }
    if(Option.dictfile)
        {
        fpdict = fopen(Option.dictfile,"rb");
        if(!fpdict)
            {
            printf("-d\t%-20s\t(Dictionary): Cannot open file.\n",Option.dictfile);
            return -1;
            }
        else
            info("-d\t%-20s\tDictionary",Option.dictfile);
        }
    else
        {
        info("-d\tDictionary: File not specified.");
//                return -1;
        }

    if(Option.InputHasTags)
        {
        if(Option.v)
            {
            fpv = fopen(Option.v,"rb");  /* "r" -> "rb" 20130122 */
            if(!fpv)
                {
                printf("-v\t%-20s\t(Tag friends file): Cannot open file.\n",Option.v);
                return -1;
                }
            else
                info("-v\t%-20s\tTag friends file",Option.v);
            }
        else
            info("-v\tTag friends file: File not specified.");

        if(Option.x)
            {
            fpx = fopen(Option.x,"rb"); /* "r" -> "rb" 20130122 */
            if(!fpx)
                {
                printf("-x\t%-20s\t(Lexical type translation table): Cannot open file.\n",Option.x);
                return -1;
                }
            else
                info("-x\t%-20s\tLexical type translation table",Option.x);
            }
        else
            info("-x\tLexical type translation table: File not specified.");

        if(Option.z)
            {
            fpz = fopen(Option.z,"rb");
            if(!fpz)
                {
                printf("-z\t%-20s\t(Full form - Lemma type conversion table): Cannot open file.\n",Option.z);
                return -1;
                }
            else
                info("-z\t%-20s\tFull form - Lemma type conversion table",Option.z);
            }
        else
            info("-z\tFull form - Lemma type conversion table: File not specified.");
        }
    else
        { // Bart 20021105
        if(Option.z)
            {
            fpz = fopen(Option.z,"rb");  /* "r" -> "rb" 20130122 */
            if(!fpz)
                {
                printf("-z\t%-20s\t(Full form - Lemma type conversion table): Cannot open file.\n",Option.z);
                return -1;
                }
            else
                info("-z\t%-20s\tFull form - Lemma type conversion table",Option.z);
            }
        }

/*
    if(argn)
        {
        fpnew = fopen(argn,"w");
        if(!fpnew)
            {
            printf("-n  %-20s (New words): Cannot open file.\n",argn);
            return -1;
            }
        else
            printf("-n  %-20s New words\n",argn);
        }
    else
        {
        printf("-n  New words are not written to a file\n");
        fpnew = 0;
        }

    if(argm)
        {
        fphom = fopen(argm,"w");
        if(!fphom)
            {
            printf("-m  %-20s (Conflicts): Cannot open file.\n",argm);
            return -1;
            }
        else
            printf("-m  %-20s Conflicts\n",argm);
        }
    else
        {
        printf("-n  Conflicts are not written to a file\n");
        fphom = 0;
        }
*/
    if(fpflex)
        {
        Flex.readFromFile(fpflex,Option.InputHasTags ? Option.flx : 0); 
        // fpflex contains rules for untagged text. These can be used if tag-specific rules do not exist.
        if(Option.RulesUnique)
            Flex.removeAmbiguous();
        if(Option.nice)
            {
            printf("\n");
            Flex.print();
            }
        fclose(fpflex);
        }
    else if(!readRules(Option.flx))
        {
        if(Option.InputHasTags)
            ;//NewStyle = true; // Rules will be read as necessary, depending on which tags occur in the text.
        else
            return -1;
        }

    if(fpv)
        {
        if(TagFriends)
            delete TagFriends;
        TagFriends = new tagpairs(fpv,Option.nice);
/*                if(!readTags(fpx,nice))
            {
            fclose(fpx);
            return -1;
            }*/
        fclose(fpv);
        }

    if(fpx)
        {
        if(TextToDictTags)
            delete TextToDictTags;
        TextToDictTags = new tagpairs(fpx,Option.nice);
/*                if(!readTags(fpx,nice))
            {
            fclose(fpx);
            return -1;
            }*/
        fclose(fpx);
        }

    if(fpz)
        {
        if(!readLemmaTags(fpz,Option.nice))
            {
            fclose(fpz);
            return -1;
            }
        fclose(fpz);
        }

    if(Option.nice && fpdict)
        printf("\nreading dictionary \"%s\"\n",Option.dictfile);
    dict.initdict(fpdict);
    if(fpdict)
        fclose(fpdict);
//            dict.printall(fpout);
//            dict.printall2(fpout);
    return 0;
    }

void Lemmatiser::showSwitches()
    {
    info("\nSwitches:");
    if(Option.InputHasTags)
        {
        info("-t\tInput has tags.");
        }
    else
        {
        info("-t-\tInput has no tags.");
        if(Option.POSAttribute)
            {
            info("\tThe PoS-attrribute '%s' (-Xp%s) is ignored.",Option.POSAttribute,Option.POSAttribute);
            Option.POSAttribute = NULL;
            }
        if(!Option.Iformat)
            {
            if(Option.keepPunctuation == 1)
                info("-p\tKeep punctuation.");
            else if(Option.keepPunctuation == 2)
                info("-p+\tTreat punctuation as separate tokens.");
            else
                info("-p-\tIgnore punctuation.");
            }
        }

    if(!Option.Wformat)
        {
        if(Option.SortOutput)
            {
            /*if(Option.Wformat)
                printf("-q\t(Irrelevant option in combination with -W.)\n");
            else*/
                {
                SortInput = true;
                info("-q\tSort output.");
                info("Input is sorted before processing (due to option -q)\n",Option.flx);
                }
            if(SortFreq(Option.SortOutput))
                {
                /*if(Option.Wformat)
                    printf("-q\t(Irrelevant option in combination with -W.)\n");
                else*/
                    {
                    info("-q#\tSort output by frequence.");
                    }
                }
            }
        else
            {
            /*if(Option.Wformat)
                printf("-q\t(Irrelevant option in combination with -W.)\n");
            else*/
                info("-q-\tDo not sort output.(default)");
            }
        }

    if(!SortInput)
        info("Input is not sorted before processing (no option -q and no $f field in -c<format> or -W<format> argument)",Option.flx);
/*
    if(OutputHasFullForm)
        printf("-a    Output file contains full form of each word.\n");
    else
        printf("-a-   Output file does not contain full form of each word.\n");
*/
    if(!strcmp(Option.Sep,"\t"))
        info("-s\tAmbiguous output is tab-separated");
    else if(!strcmp(Option.Sep," "))
        info("-s" commandlineQuote " " commandlineQuote "\tAmbiguous output  is blank-separated");
    else if(!strcmp(Option.Sep,Option.DefaultSep))
        info("-s%s\tAmbiguous output is " commandlineQuote "%s" commandlineQuote "-separated (default)",Option.Sep,Option.Sep);
    else
        info("-s%s\tAmbiguous output is " commandlineQuote "%s" commandlineQuote "-separated",Option.Sep,Option.Sep);

    if(Option.RulesUnique)
        info("-U\tenforce unique flex rules (default)");
    else
        info("-U-\tallow ambiguous flex rules");

    if(Option.DictUnique)
        info("-u\tenforce unique dictionary look-up (default)");
    else
        info("-u-\tallow ambiguous dictionary look-up");
    switch(Option.UseLemmaFreqForDisambiguation)
        {
        case 0: info("-H0\tuse lemma frequencies for disambigation (default)");
            basefrm::hasW = true;
            break;
        case 1: info("-H1\tuse lemma frequencies for disambigation, show pruned lemmas between <<>>");
            basefrm::hasW = true;
            break;
        case 2: info("-H2\tdon't use lemma frequencies for disambigation");break;
        }
    if(Option.baseformsAreLowercase)
        info("-l\tlemmas are forced to lowercase (default)");
    else
        info("-l-\tlemmas are same case as full form");

    if(Option.size < ULONG_MAX)
        info("-m%lu\tReading max %lu words from input",Option.size,Option.size);
    else
        info("-m0\tReading unlimited number of words from input (default).");

    if(Option.arge)
        {
        if('0' < *Option.arge && *Option.arge <= '9')
            info("-e%s\tUse ISO8859-%s Character encoding for case conversion.",Option.arge,Option.arge);
        else
            info("-e%s\tUse Unicode Character encoding for case conversion.",Option.arge);
        }
    else
        info("-e-\tDon't use case conversion.");

    if(Option.nice)
        printf("reading text\n");
    }
//-L -eU -p+ -qwft -t- -U- -H2 -frules_0.lem -B"$f $w ($W)\n" -l -W"$f $w" -i tests_.txt -o tests_.txt.lemma 

int Lemmatiser::LemmatiseInit()
    {
    changed = true;
    int ret = setFormats();
    if(ret)
        return ret;

    ret = openFiles();
    if(ret)
        return ret;


    showSwitches();
    return 0;
}

#if STREAM
void Lemmatiser::LemmatiseText(istream * fpin,ostream * fpout,tallyStruct * tally)
#else
void Lemmatiser::LemmatiseText(FILE * fpin,FILE * fpout,tallyStruct * tally)
#endif
    {
    if(changed)
        setFormats();
    text * Text;
    if(Option.XML)
        {
        Text  = new XMLtext
            (fpin
            ,Option
            /*
            .InputHasTags
            ,Option.Iformat
            //,Option.keepPunctuation
            ,Option.nice
            //,Option.size
            ,Option.treatSlashAsAlternativesSeparator
            ,Option.XML
            ,Option.ancestor
            ,Option.element
            ,Option.wordAttribute
            ,Option.POSAttribute
            ,Option.lemmaAttribute
            ,Option.lemmaClassAttribute
            ,Option.SortOutput
            */
            );
        }
    else
        {
        Text  = new flattext(fpin,Option.InputHasTags,Option.Iformat,Option.keepPunctuation,Option.nice,Option.size,Option.treatSlashAsAlternativesSeparator
            /*
            ,Option.XML
            ,Option.ancestor
            ,Option.element
            ,Option.wordAttribute
            ,Option.POSAttribute
            ,Option.lemmaAttribute
            ,Option.lemmaClassAttribute
            */
            );
        }
#if STREAM
    if(fpin != cin)
        delete fpin;
#else
    if(fpin != stdin)
        fclose(fpin);
#endif

    if(Option.nice)
        printf("processing\n");
    Text->Lemmatise(fpout
                   ,/*fpnew,fphom
                   ,*/Option.Sep
                   ,tally
                   ,Option.SortOutput
                   ,Option.UseLemmaFreqForDisambiguation
                   ,Option.nice
                   ,Option.DictUnique
                   ,Option.baseformsAreLowercase
                   ,listLemmas
                   ,   Option.Wformat != NULL // list lemma's with all word forms
                    && ((listLemmas & 3) == 3) // both of -b and -B are specified
                    && !strcmp(Option.Bformat,Option.bformat) // -b and -B options are the same format
                       // true: outputs must be merged
                       // Bart 20101102
                   );
    delete Text;
    }

int Lemmatiser::LemmatiseFile()
    {
    if(changed)
        setFormats();
    clock_t t0;
    t0 = clock();
#if STREAM
    istream * fpin;
    ostream * fpout;
#else
    FILE * fpin;
    FILE * fpout;
#endif
    if(Option.argi)
        {
//                fpin = fopen(argi,"rb");
#if STREAM
        fpin = new ifstream(Option.argi,ios::in|ios::binary);
        if(!fpin || !fpin->good())
#else
        fpin = fopen(Option.argi,"rb"); // Bart 20090107 rt -> rb
        if(!fpin)
#endif
            {
            printf("-i  %-20s (Input text): Cannot open file.\n",Option.argi);
            return -1;
            }
        else
            info("-i\t%-20s\tInput text",Option.argi);
        }
    else
        {
        info("-i\tInput text: Using standard input.");
#if STREAM
        fpin = &cin;
#else
        fpin = stdin;
#endif
        }
    if(Option.argo)
        {
#if STREAM
        fpout = new ofstream(Option.argo,ios::out|ios::binary);
#else
        fpout = fopen(Option.argo,"wb");
#endif
        if(!fpout)
            {
            printf("-o  %-20s (Output text): Cannot open file.\n",Option.argo);
            return -1;
            }
        else
            info("-o\t%-20s\tOutput text",Option.argo);
        }
    else
        {
        if(Option.argi)
            {
            char buffer[256];
            Option.argo = buffer;
            sprintf(buffer,"%s.lemma",Option.argi);
#if STREAM
            fpout = new ofstream(Option.argo,ios::out|ios::binary);
#else
            fpout = fopen(Option.argo,"wb");
#endif
            if(!fpout)
                {
                printf("-o  %-20s (Output text): Cannot open file.\n",Option.argo);
                return -1;
                }
            else
                info("-o\t%-20s\tOutput text",Option.argo);
            }
        else
            {
            DoInfo = false;
            info("-o\tOutput text: Using standard output.");
#if STREAM
            fpout = &cout;
#else
            fpout = stdout;
#endif
            }
        }
    switch(Option.keepPunctuation)
        {
        case 0:
            info("-p-\tignore punctuation (only together with -t- and no -W format)\n");
            break;
        case 1:
            info("-p\tkeep punctuation (default)\n");
            break;
        case 2:
            info("-p+\ttreat punctuation as tokens (only together with -t- and no -W format)\n");
            break;
        default:
            info("-p:\tUnknown argument.\n");
        }

    tallyStruct tally;
    LemmatiseText(fpin,fpout,&tally);
    showtime(t0);
#if STREAM
    if(fpout != &cout)
        delete fpout;
#else
    if(fpout != stdout)
        fclose(fpout);
#endif
    info("\nall words      %10.lu\n"
             "unknown words  %10.lu (%lu%%)\n"
             "conflicting    %10.lu (%lu%%)\n"
             ,tally.totcnt
             ,tally.newcnt,tally.totcnt ? (tally.newcnt*200+1)/(2*tally.totcnt) : 100
             ,tally.newhom,tally.totcnt ? (tally.newhom*200+1)/(2*tally.totcnt) : 100
             );
    if(SortInput)
        info("\nall types      %10.lu\n"
                 "unknown types  %10.lu (%lu%%)\n"
                 "conflicting    %10.lu (%lu%%)"
                 ,tally.totcntTypes
                 ,tally.newcntTypes,tally.totcntTypes ? (tally.newcntTypes*200+1)/(2*tally.totcntTypes) : 100UL
                 ,tally.newhomTypes,tally.totcntTypes ? (tally.newhomTypes*200+1)/(2*tally.totcntTypes) : 100UL
                 );
    return 0;
    }


void Lemmatiser::LemmatiseEnd()
    {

//    Flex.write(fpflex);
//            fclose(fpflex);
//            fclose(fpnew);
    delete TextToDictTags;
    delete TagFriends;
    }


        // functions to change Option on the fly
/*
void Lemmatiser::setIformat(const char * format)
    {            // -I
    Option.setIformat(format);
    }

void Lemmatiser::setBformat(const char * format)            // -B
    {            // -I
    Option.setBformat(format);
    changed = true;
    }

void Lemmatiser::setbformat(const char * format)            // -b
    {            // -I
    Option.setbformat(format);
    changed = true;
    }

void Lemmatiser::setcformat(const char * format)            // -c
    {            // -I
    Option.setcformat(format);
    changed = true;
    }

void Lemmatiser::setWformat(const char * format)            // -W
    {            // -I
    Option.setWformat(format);
    changed = true;
    }

void Lemmatiser::setSep(const char * a)
    {            // -I
    Option.setSep(a);
    }

void Lemmatiser::setUseLemmaFreqForDisambiguation(int n)    // -H 0, 1 or 2
    {
    Option.setUseLemmaFreqForDisambiguation(n);
    }

void Lemmatiser::setkeepPunctuation(bool b)
    {
    Option.setkeepPunctuation(b);
    }

void Lemmatiser::setsize(unsigned long int n)
    {
    Option.setsize(n);
    }

void Lemmatiser::settreatSlashAsAlternativesSeparator(bool b)
    {
    Option.settreatSlashAsAlternativesSeparator(b);
    }

void Lemmatiser::setUseLemmaFreqForDisambiguation(bool b)
    {
    Option.setUseLemmaFreqForDisambiguation(b);
    changed = true;
    }

void Lemmatiser::setDictUnique(bool b)
    {
    Option.setDictUnique(b);
    }

void Lemmatiser::setbaseformsAreLowercase(bool b)
    {
    Option.setbaseformsAreLowercase(b);
    }
*/
#endif
