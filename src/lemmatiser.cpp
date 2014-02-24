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
#include "lemmatiser.h"
#if defined PROGLEMMATISE
#include "applyrules.h"
#include "tags.h"
#endif
#include "option.h"
#if defined PROGMAKEDICT 
#include "makedict.h"
#endif

#if (defined PROGLEMMATISE) || (defined PROGMAKESUFFIXFLEX)
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
#if (defined PROGLEMMATISE) || (defined PROGMAKEDICT)
#include <stdarg.h>
#include <time.h>
#include <limits.h>
#endif

#ifdef COUNTOBJECTS
int Lemmatiser::COUNT = 0;
#endif


#if STREAM
#include <iomanip> 
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


int Lemmatiser::instance = 0;

#if defined PROGLEMMATISE
static bool DoInfo = true;
tagpairs * Lemmatiser::TextToDictTags = 0;

#if STREAM
#else
static int info(const char *fmt, ...)
    {
    if(DoInfo)
        {
        int ret;
        va_list ap;
        va_start(ap,fmt);
        ret = vprintf(fmt,ap);
        va_end(ap);
        LOG1LINE("");
        return ret;
        }
    return 0;
    }
#endif

static void showtime(clock_t t0)
    {
    clock_t t1;
    unsigned long span,sec,msec;
    t1 = clock();
    span = t1 - t0;
    sec = span / CLOCKS_PER_SEC;
    span -= sec * CLOCKS_PER_SEC;
    span *= 1000;
    msec = span / CLOCKS_PER_SEC;
#if STREAM
    clog << "\nTime: " << sec << "." << msec << endl ;
#else
    info("\nTime: %ld.%.3ld",sec,msec);
#endif
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
        if(!Option.argo /*&& !Option.argi*/)
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

#if (defined PROGLEMMATISE) || (defined PROGMAKEDICT)
static void cannotOpenFile(const char * s1,const char * name, const char * s2)
    {
#if STREAM
    cout << s1 << " \"" << name  << "\" " << s2 << endl;
#else
    printf("%s \"%s\" %s\n",s1,name,s2);
#endif
    }
#endif

#if defined PROGMAKEDICT
int Lemmatiser::MakeDict()
    {
    FILE * fpin;
    FILE * fpout;
    FILE * ffreq = 0;
    if(!Option.cformat)
        {
        LOG1LINE("You need to specify a column-order with the -c option");
        return -1;
        }
    if(Option.argi)
        {
        fpin = fopen(Option.argi,"r");
        if(!fpin)
            {
            cannotOpenFile("Cannot open input file",Option.argi,"for reading");
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
            cannotOpenFile("Cannot open binary dictionary",Option.argo,"for writing");
            return -1;
            }
        }
    else
        fpout = stdout;
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
        LOG1LINE("You need to specify a column-order with the -c option");
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
#if STREAM
    clog << "\nFormats:" << endl;
#else
    info("\nFormats:");
#endif
    listLemmas = 0;
    if(Option.Iformat)
        {
#if STREAM
        clog << "-I\t" << Option.Iformat << "\tInput format." << endl;
#else
        info("-I\t%s\tInput format.",Option.Iformat);
#endif
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
        LOG1LINE("You need to specify -b or -B formats if you specify the -W format");
        return -1;
        }
    
    if(!Option.DefaultCformat() && Option.Wformat)
        {
        LOG1LINE("You cannot specify both -c and -W format options");
        return -1;
        }
    else if(Option.cformat && !Option.Wformat)
        {
#if STREAM
        clog << "-c\t" << Option.cformat << "\tOutput format" << endl;
#else
        info("-c\t%s\tOutput format",Option.cformat);
#endif
        if(Option.bformat)
#if STREAM
            clog << "-b\t" << Option.bformat << "\tDictionary base form output format." << endl;
#else
            info("-b\t%s\tDictionary base form output format.",Option.bformat);
#endif
        if(Option.Bformat)
#if STREAM
            clog << "-B\t" << Option.Bformat << "\tComputed base form output format." << endl;
#else
            info("-B\t%s\tComputed base form output format.",Option.Bformat);
#endif
        listLemmas = 0;
        }
    else
        {
        if(Option.bformat)
            {
#if STREAM
            clog << "-b\t" << Option.bformat << "\tOutput format for data pertaining to the base form, according to the dictionary" << endl;
#else
            info("-b\t%s\tOutput format for data pertaining to the base form, according to the dictionary",Option.bformat);
#endif
            listLemmas |= 1;
            }
        if(Option.Bformat)
            {
            listLemmas |= 2;
#if STREAM
            clog << "-B\t" << Option.Bformat << "\tOutput format for data pertaining to the base form, as predicted by flex pattern rules." << endl;
#else
            info("-B\t%s\tOutput format for data pertaining to the base form, as predicted by flex pattern rules.",Option.Bformat);
#endif
            }
        if(!listLemmas)
            {
            LOG1LINE("You must specify at least one of -b and -B if you do not specify -c.");
            return -1;
            }
//                format = Wformat;
        if(Option.Wformat)
#if STREAM
            clog << "-W\t" << Option.Wformat << "\tOutput format for data pertaining to full forms." << endl;
#else
            info("-W\t%s\tOutput format for data pertaining to full forms.",Option.Wformat);
#endif
        }
    if(listLemmas)
        {
        SortInput = basefrm::setFormat(Option.Wformat,Option.bformat,Option.Bformat,Option.InputHasTags);
        if(SortInput)
#if STREAM
            clog << "Input is sorted before processing (due to $f field in -W<format> argument)" << endl;
#else
            info("Input is sorted before processing (due to $f field in -W<format> argument)");
#endif
        }
    else
        {
        SortInput = text::setFormat(Option.cformat,Option.bformat,Option.Bformat,Option.InputHasTags);
        if(SortInput)
#if STREAM
            clog << "Input is sorted before processing (due to $f field in -c<format> argument)" << endl;
#else
            info("Input is sorted before processing (due to $f field in -c<format> argument)");
#endif
        }
    if(!SortInput)
        {
        if(listLemmas || Option.UseLemmaFreqForDisambiguation < 2)
            SortInput = true;// performance

        }
    if(!Option.XML)
#if STREAM
        clog << "-X-\tNot XML input." << endl;
#else
        info("-X-\tNot XML input.");
#endif
    else
        {
        if(  Option.ancestor 
          || Option.element 
          || Option.wordAttribute 
          || Option.POSAttribute 
          || Option.lemmaAttribute 
          || Option.lemmaClassAttribute
          )
            {
#if STREAM
            clog << "\tXML-aware scanning of input" << endl;
#else
            info("\tXML-aware scanning of input");
#endif
            if(Option.ancestor)
#if STREAM
                clog << "-Xa" << Option.ancestor << "\tOnly analyse elements with ancestor " << Option.ancestor << endl;
#else
                info("-Xa%s\tOnly analyse elements with ancestor %s",Option.ancestor,Option.ancestor);
#endif
            if(Option.element)
#if STREAM
                clog << "-Xe" << Option.element << "\tOnly analyse elements " << Option.element << endl;
#else
                info("-Xe%s\tOnly analyse elements %s",Option.element,Option.element);
#endif
            if(Option.wordAttribute)
                {
#if STREAM
                clog << "-Xw" << Option.wordAttribute << "\tLook for word in attribute " << Option.wordAttribute << endl;
#else
                info("-Xw%s\tLook for word in attribute %s",Option.wordAttribute,Option.wordAttribute);
#endif
                if(!Option.lemmaAttribute)
#if STREAM
                    clog << "\tStore lemma in attribute " << Option.wordAttribute << endl;
#else
                    info("\tStore lemma in attribute %s",Option.wordAttribute);
#endif
                }
            if(Option.POSAttribute)
                {
                if(Option.InputHasTags)
#if STREAM
                    clog << "-Xp" << Option.POSAttribute << "\tLook for Part of Speech in attribute " << Option.POSAttribute << endl;
#else
                    info("-Xp%s\tLook for Part of Speech in attribute %s",Option.POSAttribute,Option.POSAttribute);
#endif
                else
#if STREAM
                    clog << "-Xp" << Option.POSAttribute << "\tLook for Part of Speech in attribute " << Option.POSAttribute << " (ignored because of option -t-)" << endl;
#else
                    info("-Xp%s\tLook for Part of Speech in attribute %s (ignored because of option -t-)",Option.POSAttribute,Option.POSAttribute);
#endif
                }
            if(Option.lemmaAttribute)
#if STREAM
                clog << "-Xl" << Option.lemmaAttribute << "\tStore lemma in attribute " << Option.lemmaAttribute << endl;
#else
                info("-Xl%s\tStore lemma in attribute %s",Option.lemmaAttribute,Option.lemmaAttribute);
#endif
            if(Option.lemmaClassAttribute)
#if STREAM
                clog << "-Xc" << Option.lemmaClassAttribute << "\tStore lemma class in attribute " << Option.lemmaClassAttribute << endl;
#else
                info("-Xc%s\tStore lemma class in attribute %s",Option.lemmaClassAttribute,Option.lemmaClassAttribute);
#endif
            }
        else
            {
#if STREAM
            clog << "-X+\tXML-aware scanning of input" << endl;
#else
            info("-X\tXML-aware scanning of input");
#endif
            }
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
    if(Option.flx)
        {
        fpflex = fopen(Option.flx,"rb");
        if(fpflex)
            {
#if STREAM
            clog << "-f" << Option.flx << "\t File with flex patterns." << endl;
#else
            info("-f%s\tFile with flex patterns.",Option.flx);
#endif
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
#if STREAM
                        clog << "-f\t" << std::setw(20) << Option.flx << "\t(Flexpatterns): Folder " << tmp << " does not exist or is not readable. Cannot open flex pattern files." << endl;
#else
                        info("-f\t%-20s\t(Flexpatterns): Folder %s does not exist or is not readable. Cannot open flex pattern files.",Option.flx,tmp);
#endif
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
#if STREAM
                        clog << "-f\t" << std::setw(20) << Option.flx << "\t(Flexpatterns): Folder " << tmp << " does not exist or is not readable. Cannot open flex pattern files." << endl;
#else
                        info("-f\t%-20s\t(Flexpatterns): Folder %s does not exist or is not readable. Cannot open flex pattern files.",Option.flx,tmp);
#endif
                        delete [] tmp;
                        return -1;
                        }
                    else
                        fclose(d);
                    break;
                    }
#endif
#if STREAM
            clog << "-f\t" << std::setw(20) << Option.flx << "\t(Flexpatterns): Cannot open file. Assuming that tag-specific files exist in folder " << ((slash < tmp) ? "." : tmp) << " with prefix " << slash + 1 << "." << endl;
#else
            info("-f\t%-20s\t(Flexpatterns): Cannot open file. Assuming that tag-specific files exist in folder %s with prefix %s."
                ,Option.flx
                ,(slash < tmp) ? "." : tmp
                ,slash + 1
                );
#endif
            delete [] tmp;
            }
        else
            {
#if STREAM
            clog << "-f\t" << std::setw(20) << Option.flx << "\t(Flexpatterns): Cannot open file." << endl;
#else
            info("-f\t%-20s\t(Flexpatterns): Cannot open file.",Option.flx);
#endif
            return -1;
            }
        }
    else
        {
        LOG1LINE("-f  Flexpatterns: File not specified.");
        return -1;
        }
    if(Option.dictfile)
        {
        fpdict = fopen(Option.dictfile,"rb");
        if(!fpdict)
            {
            cannotOpenFile("-d\t",Option.dictfile,"\t(Dictionary): Cannot open file.");
            return -1;
            }
        else
#if STREAM
            clog << "-d\t" << std::setw(20) << Option.dictfile << "\tDictionary" << endl;
#else
            info("-d\t%-20s\tDictionary",Option.dictfile);
#endif
        }
    else
        {
#if STREAM
        clog << "-d\tDictionary: File not specified." << endl;
#else
        info("-d\tDictionary: File not specified.");
#endif
//                return -1;
        }

    if(Option.InputHasTags)
        {
        if(Option.v)
            {
            fpv = fopen(Option.v,"rb");  /* "r" -> "rb" 20130122 */
            if(!fpv)
                {
                cannotOpenFile("-v\t",Option.v,"\t(Tag friends file): Cannot open file.");
                return -1;
                }
            else
#if STREAM
                clog << "-v\t" << std::setw(20) << Option.v << "\tTag friends file" << endl;
#else
                info("-v\t%-20s\tTag friends file",Option.v);
#endif
            }
        else
#if STREAM
            clog << "-v\tTag friends file: File not specified." << endl;
#else
            info("-v\tTag friends file: File not specified.");
#endif

        if(Option.x)
            {
            fpx = fopen(Option.x,"rb"); /* "r" -> "rb" 20130122 */
            if(!fpx)
                {
                cannotOpenFile("-x\t",Option.x,"\t(Lexical type translation table): Cannot open file.");
                return -1;
                }
            else
#if STREAM
            clog << "-v\tTag friends file: File not specified." << endl;
#else
            info("-v\tTag friends file: File not specified.");
#endif
            }
        else
#if STREAM
            clog << "-x\tLexical type translation table: File not specified." << endl;
#else
            info("-x\tLexical type translation table: File not specified.");
#endif

        if(Option.z)
            {
            fpz = fopen(Option.z,"rb");
            if(!fpz)
                {
                cannotOpenFile("-z\t",Option.z,"\t(Full form - Lemma type conversion table): Cannot open file.");
                return -1;
                }
            else
#if STREAM
                clog << "-z\t" << std::setw(20) << Option.z << "\tFull form - Lemma type conversion table" << endl;
#else
                info("-z\t%-20s\tFull form - Lemma type conversion table",Option.z);
#endif
            }
        else
#if STREAM
            clog << "-z\tFull form - Lemma type conversion table: File not specified." << endl;
#else
            info("-z\tFull form - Lemma type conversion table: File not specified.");
#endif
        }
    else
        {
        if(Option.z)
            {
            fpz = fopen(Option.z,"rb");  /* "r" -> "rb" 20130122 */
            if(!fpz)
                {
                cannotOpenFile("-z\t",Option.z,"\t(Full form - Lemma type conversion table): Cannot open file.");
                return -1;
                }
            else
#if STREAM
                clog << "-z\t" << std::setw(20) << Option.z << "\tFull form - Lemma type conversion table" << endl;
#else
                info("-z\t%-20s\tFull form - Lemma type conversion table",Option.z);
#endif
            }
        }

    if(fpflex)
        {
        Flex.readFromFile(fpflex,Option.InputHasTags ? Option.flx : 0); 
        // fpflex contains rules for untagged text. These can be used if tag-specific rules do not exist.
        if(Option.RulesUnique)
            Flex.removeAmbiguous();
        if(Option.nice)
            {
            LOG1LINE("");
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
#if STREAM
        cout << "\nreading dictionary \"" << Option.dictfile << "\"" << endl;
#else
        printf("\nreading dictionary \"%s\"\n",Option.dictfile);
#endif
    dict.initdict(fpdict);
    if(fpdict)
        fclose(fpdict);
//            dict.printall(fpout);
//            dict.printall2(fpout);
    return 0;
    }

void Lemmatiser::showSwitches()
    {
#if STREAM
    clog << "\nSwitches:" << endl;
#else
    info("\nSwitches:");
#endif
    if(Option.InputHasTags)
        {
#if STREAM
        clog << "-t\tInput has tags." << endl;
#else
        info("-t\tInput has tags.");
#endif
        }
    else
        {
#if STREAM
        clog << "-t-\tInput has no tags." << endl;
#else
        info("-t-\tInput has no tags.");
#endif
        if(Option.POSAttribute)
            {
#if STREAM
            clog << "\tThe PoS-attrribute '" << Option.POSAttribute << "' (-Xp" << Option.POSAttribute << ") is ignored." << endl;
#else
            info("\tThe PoS-attrribute '%s' (-Xp%s) is ignored.",Option.POSAttribute,Option.POSAttribute);
#endif
            Option.POSAttribute = NULL;
            }
        if(!Option.Iformat)
            {
            if(Option.keepPunctuation == 1)
#if STREAM
                clog << "-p\tKeep punctuation." << endl;
#else
                info("-p\tKeep punctuation.");
#endif
            else if(Option.keepPunctuation == 2)
#if STREAM
                clog << "-p+\tTreat punctuation as separate tokens." << endl;
#else
                info("-p+\tTreat punctuation as separate tokens.");
#endif
            else
#if STREAM
                clog << "-p-\tIgnore punctuation." << endl;
#else
                info("-p-\tIgnore punctuation.");
#endif
            }
        }

    if(!Option.Wformat)
        {
        if(Option.SortOutput)
            {
            SortInput = true;
#if STREAM
            clog << "-q\tSort output." << endl;
#else
            info("-q\tSort output.");
#endif
#if STREAM
            clog << "Input is sorted before processing (due to option -q)\n" << endl;
#else
            info("Input is sorted before processing (due to option -q)\n");
#endif
            if(SortFreq(Option.SortOutput))
                {
#if STREAM
                clog << "-q#\tSort output by frequence." << endl;
#else
                info("-q#\tSort output by frequence.");
#endif
                }
            }
        else
            {
#if STREAM
            clog << "-q-\tDo not sort output.(default)" << endl;
#else
            info("-q-\tDo not sort output.(default)");
#endif
            }
        }

    if(!SortInput)
#if STREAM
        clog << "Input is not sorted before processing (no option -q and no $f field in -c<format> or -W<format> argument)" << endl;
#else
        info("Input is not sorted before processing (no option -q and no $f field in -c<format> or -W<format> argument)");
#endif
    if(!strcmp(Option.Sep,"\t"))
#if STREAM
        clog << "-s\tAmbiguous output is tab-separated" << endl;
#else
        info("-s\tAmbiguous output is tab-separated");
#endif
    else if(!strcmp(Option.Sep," "))
#if STREAM
        clog << "-s" commandlineQuote " " commandlineQuote "\tAmbiguous output  is blank-separated" << endl;
#else
        info("-s" commandlineQuote " " commandlineQuote "\tAmbiguous output  is blank-separated");
#endif
    else if(!strcmp(Option.Sep,Option.DefaultSep))
#if STREAM
        clog << "-s" << Option.Sep << "\tAmbiguous output is " commandlineQuote  << Option.Sep << commandlineQuote "-separated (default)" << endl;
#else
        info("-s%s\tAmbiguous output is " commandlineQuote "%s" commandlineQuote "-separated (default)",Option.Sep,Option.Sep);
#endif
    else
#if STREAM
        clog << "-s" << Option.Sep << "\tAmbiguous output is " commandlineQuote << Option.Sep << commandlineQuote "-separated" << endl;
#else
        info("-s%s\tAmbiguous output is " commandlineQuote "%s" commandlineQuote "-separated",Option.Sep,Option.Sep);
#endif

    if(Option.RulesUnique)
#if STREAM
        clog << "-U\tenforce unique flex rules (default)" << endl;
#else
        info("-U\tenforce unique flex rules (default)");
#endif
    else
#if STREAM
        clog << "-U-\tallow ambiguous flex rules" << endl;
#else
        info("-U-\tallow ambiguous flex rules");
#endif

    if(Option.DictUnique)
#if STREAM
        clog << "-u\tenforce unique dictionary look-up (default)" << endl;
#else
        info("-u\tenforce unique dictionary look-up (default)");
#endif
    else
#if STREAM
        clog << "-u-\tallow ambiguous dictionary look-up" << endl;
#else
        info("-u-\tallow ambiguous dictionary look-up");
#endif
    switch(Option.UseLemmaFreqForDisambiguation)
        {
        case 0: 
#if STREAM
            clog << "-H0\tuse lemma frequencies for disambigation (default)" << endl;
#else
            info("-H0\tuse lemma frequencies for disambigation (default)");
#endif
            basefrm::hasW = true;
            break;
        case 1: 
#if STREAM
            clog << "-H1\tuse lemma frequencies for disambigation, show pruned lemmas between <<>>" << endl;
#else
            info("-H1\tuse lemma frequencies for disambigation, show pruned lemmas between <<>>");
#endif
            basefrm::hasW = true;
            break;
        case 2: 
#if STREAM
            clog << "-H2\tdon't use lemma frequencies for disambigation" << endl;
#else
            info("-H2\tdon't use lemma frequencies for disambigation");
#endif
            break;
        }
    if(Option.baseformsAreLowercase)
#if STREAM
        clog << "-l\tlemmas are forced to lowercase (default)" << endl;
#else
        info("-l\tlemmas are forced to lowercase (default)");
#endif
    else
#if STREAM
        clog << "-l-\tlemmas are same case as full form" << endl;
#else
        info("-l-\tlemmas are same case as full form");
#endif

    if(Option.size < ULONG_MAX)
#if STREAM
        clog << "-m" << Option.size << "\tReading max " << Option.size << " words from input" << endl;
#else
        info("-m%lu\tReading max %lu words from input",Option.size,Option.size);
#endif
    else
#if STREAM
        clog << "-m0\tReading unlimited number of words from input (default)." << endl;
#else
        info("-m0\tReading unlimited number of words from input (default).");
#endif

    if(Option.arge)
        {
        if('0' < *Option.arge && *Option.arge <= '9')
#if STREAM
            clog << "-e" << Option.arge << "\tUse ISO8859-" << Option.arge << " Character encoding for case conversion." << endl;
#else
            info("-e%s\tUse ISO8859-%s Character encoding for case conversion.",Option.arge,Option.arge);
#endif
        else
#if STREAM
            clog << "-e" << Option.arge << "\tUse Unicode Character encoding for case conversion." << endl;
#else
            info("-e%s\tUse Unicode Character encoding for case conversion.",Option.arge);
#endif
        }
    else
#if STREAM
        clog << "-e-\tDon't use case conversion." << endl;
#else
        info("-e-\tDon't use case conversion.");
#endif

    if(Option.nice)
        LOG1LINE("reading text\n");
    }

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
        Text  = new XMLtext(fpin,Option);
        }
    else
        {
        Text  = new flattext(fpin,Option.InputHasTags,Option.Iformat,Option.keepPunctuation,Option.nice,Option.size,Option.treatSlashAsAlternativesSeparator);
        }
    if(Option.nice)
        LOG1LINE("processing");
    Text->Lemmatise(fpout
                   ,Option.Sep
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
                   );
    delete Text;
    }

int Lemmatiser::LemmatiseFile()
    {
    if(Option.XML && !Option.Iformat)
        {
        // 20140224
        // Set default for input format
        if(Option.InputHasTags)
            Option.Iformat = dupl("$w/$t\\s");
        else
            Option.Iformat = dupl("$w\\s");
        }
    if(Option.XML && (Option.keepPunctuation != 1))
        {
#if STREAM
        cout << "Automatic tokenization (-p option) is not supported for XML input." << endl;
#else
        printf("Automatic tokenization (-p option) is not supported for XML input.\n");
#endif
        return -1;
        }
    else if((Option.Iformat != 0) && (Option.keepPunctuation != 1))
        {
#if STREAM
        cout << "You can specify -I or -p, not both." << endl;
#else
        printf("You can specify -I or -p, not both.\n");
#endif
        return -1;
        }
    if(changed)
        setFormats();
    clock_t t0;
    t0 = clock();
#if STREAM
    ostream * fpout;
#else
    FILE * fpout;
#endif
    if(Option.argo)
        {
#if STREAM
        fpout = new ofstream(Option.argo,ios::out|ios::binary);
#else
        fpout = fopen(Option.argo,"wb");
#endif
        if(!fpout)
            {
            cannotOpenFile("-o\t",Option.argo,"\t(Output text): Cannot open file.");
            return -1;
            }
        else
#if STREAM
            clog << "-o\t" << std::setw(20) << Option.argo << "\tOutput text" << endl;
#else
            info("-o\t%-20s\tOutput text",Option.argo);
#endif
        }
    else
        {
        DoInfo = false;
#if STREAM
        clog << "-o\tOutput text: Using standard output." << endl;
#else
        info("-o\tOutput text: Using standard output.");
#endif
#if STREAM
        fpout = &cout;
#else
        fpout = stdout;
#endif
        }
    switch(Option.keepPunctuation)
        {
        case 0:
#if STREAM
            clog << "-p-\tignore punctuation (only together with -t- and no -W format)\n" << endl;
#else
            info("-p-\tignore punctuation (only together with -t- and no -W format)\n");
#endif
            break;
        case 1:
#if STREAM
            clog << "-p\tkeep punctuation (default)\n" << endl;
#else
            info("-p\tkeep punctuation (default)\n");
#endif
            break;
        case 2:
#if STREAM
            clog << "-p+\ttreat punctuation as tokens (only together with -t- and no -W format)\n" << endl;
#else
            info("-p+\ttreat punctuation as tokens (only together with -t- and no -W format)\n");
#endif
            break;
        default:
#if STREAM
            clog << "-p:\tUnknown argument.\n" << endl;
#else
            info("-p:\tUnknown argument.\n");
#endif
        }

    tallyStruct tally;

    if(Option.argi)
        {
#if STREAM
        istream * fpin;
#else
        FILE * fpin;
#endif
#if STREAM
        fpin = new ifstream(Option.argi,ios::in|ios::binary);
        if(!fpin || !fpin->good())
#else
        fpin = fopen(Option.argi,"rb");
        if(!fpin)
#endif
            {
            cannotOpenFile("-i\t",Option.argi,"\t(Input text): Cannot open file.");
            return -1;
            }
        else
            {
#if STREAM
            clog << "-i\t" << std::setw(20) << Option.argi << "\tInput text" << endl;
#else
            info("-i\t%-20s\tInput text",Option.argi);
#endif
            }
        LemmatiseText(fpin,fpout,&tally);
        showtime(t0);
#if STREAM
        delete fpin;
#else
        fclose(fpin);
#endif
        }
    else
        {
#if STREAM
        clog << "-i\tInput text: Using standard input." << endl;
#else
        info("-i\tInput text: Not sepcified.");
#endif
#if STREAM
        while(!cin.eof())
            {
            string line;
            getline(cin,line);
#if defined __BORLANDC__ || defined __GNUG__ && __GNUG__ < 3
            strstream strin;
            strin << line;
#else
            stringstream strin(line);
#endif
            LemmatiseText(&strin,fpout,&tally);
            }
#else
        LOG1LINE(
            "No input file specified. (Option -i). If you want to use standard input,\n"
            "recompile with #define STREAM 1 in defines.h\n"
            "But notice: CSTlemma works best with complete texts, due to heuristics for\n"
            "disambiguation. When reading from standard input, each line of text is\n"
            "lemmatized independently of the rest of the text, reducing the possibility\n"
            "to guess the right lemma in case of ambiguity.");
#endif
        }

#if STREAM
    if(fpout != &cout)
        delete fpout;
#else
    if(fpout != stdout)
        fclose(fpout);
#endif
#if STREAM
    clog << "\nall words      "
        << tally.totcnt 
        << "\nunknown words  " 
        << tally.newcnt 
        << " (" 
        << (tally.totcnt ? (tally.newcnt*200+1)/(2*tally.totcnt) : 100) 
        << "%%)\nconflicting    " 
        << tally.newhom 
        << " (" 
        << (tally.totcnt ? (tally.newhom*200+1)/(2*tally.totcnt) : 100) 
        << "%%)\n" << endl;
#else
    info("\nall words      %10.lu\n"
             "unknown words  %10.lu (%lu%%)\n"
             "conflicting    %10.lu (%lu%%)\n"
             ,tally.totcnt
             ,tally.newcnt,tally.totcnt ? (tally.newcnt*200+1)/(2*tally.totcnt) : 100
             ,tally.newhom,tally.totcnt ? (tally.newhom*200+1)/(2*tally.totcnt) : 100
             );
#endif
    if(SortInput)
#if STREAM
        clog << "\nall types      " 
        << tally.totcntTypes 
        << "\nunknown types  " 
        << tally.newcntTypes 
        << " (" 
        << (tally.totcntTypes ? (tally.newcntTypes*200+1)/(2*tally.totcntTypes) : 100UL) 
        << "%%)\nconflicting    " 
        << tally.newhomTypes 
        << " (" 
        << (tally.totcntTypes ? (tally.newhomTypes*200+1)/(2*tally.totcntTypes) : 100UL) 
        << "%%)" << endl;
#else
        info("\nall types      %10.lu\n"
                 "unknown types  %10.lu (%lu%%)\n"
                 "conflicting    %10.lu (%lu%%)"
                 ,tally.totcntTypes
                 ,tally.newcntTypes,tally.totcntTypes ? (tally.newcntTypes*200+1)/(2*tally.totcntTypes) : 100UL
                 ,tally.newhomTypes,tally.totcntTypes ? (tally.newhomTypes*200+1)/(2*tally.totcntTypes) : 100UL
                 );
#endif
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

#endif
