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

#include "text.h"
#if defined PROGLEMMATISE

#include "field.h"
#include "word.h"
#include "basefrm.h"
#include "flex.h"
#include <stdlib.h>
#include <assert.h>
#include "hashmap.h"

static hash<Word> * Hash = 0;

#ifdef COUNTOBJECTS
int text::COUNT = 0;
#endif


int findSlashes(const char * buf)
    {
    if(!*buf || *buf == '/')
        return 0;
    const char * p = buf;
    int ret = 0;
    while((p = strchr(p,'/')) != 0)
        {
        ++p;
        if(*p == '/')
            return 0;
        ++ret;
        }
    if(ret && buf[strlen(buf + 1)] == '/')
        return 0;
    return ret;
    }


static int cmpBaseforms_twf(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmpt(n2);
    if(ret)
        return ret;
    ret = n1->cmps(n2);
    return ret;
    }

static int cmpBaseforms_w(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmps(n2);
    return ret;
    }

static int cmpBaseforms_wf(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmps(n2);
    if(ret)
        return ret;
    ret = n1->cmpf(n2);
    return ret;
    }

static int cmpBaseforms_ftw(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmpf(n2);
    if(ret)
        return ret;
    ret = n1->cmpt(n2);
    if(ret)
        return ret;
    ret = n1->cmps(n2);
    return ret;
    }

static int cmpBaseforms_fwt(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmpf(n2);
    if(ret)
        return ret;
    ret = n1->cmps(n2);
    if(ret)
        return ret;
    ret = n1->cmpt(n2);
    return ret;
    }

static int cmpBaseforms_tfw(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmpt(n2);
    if(ret)
        return ret;
    ret = n1->cmpf(n2);
    if(ret)
        return ret;
    ret = n1->cmps(n2);
    return ret;
    }

static int cmpBaseforms_wft(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmps(n2);
    if(ret)
        return ret;
    ret = n1->cmpf(n2);
    if(ret)
        return ret;
    ret = n1->cmpt(n2);
    return ret;
    }

static int cmpBaseforms_wtf(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmps(n2);
    if(ret)
        return ret;
    ret = n1->cmpt(n2);
    if(ret)
        return ret;
    ret = n1->cmpf(n2);
    return ret;
    }

static int cmpBaseforms_wt(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmps(n2);
    if(ret)
        return ret;
    ret = n1->cmpt(n2);
    return ret;
    }

static int cmpBaseforms_fw(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmpf(n2);
    if(ret)
        return ret;
    ret = n1->cmps(n2);
    return ret;
    }

static int (*pcmpBaseforms)  (const basefrm * elem1, const basefrm * elem2) = cmpBaseforms_w;
static int (*pcmpBaseforms_f)(const basefrm * elem1, const basefrm * elem2) = cmpBaseforms_fw;

static int compareBaseforms( const void *arg1, const void *arg2 )
    {
    const basefrm * n1 = *(const basefrm * const *)arg1;
    const basefrm * n2 = *(const basefrm * const *)arg2;
    return pcmpBaseforms(n1,n2);
    }

static int compareBaseforms_f( const void *arg1, const void *arg2 )
    {
    const basefrm * n1 = *(const basefrm * const *)arg1;
    const basefrm * n2 = *(const basefrm * const *)arg2;
    if(!n1)
        if(n2)
            return 1;
        else
            return 0;
    else if(!n2)
        return -1;
    return pcmpBaseforms_f(n1,n2);
    }

static int sortBaseforms(basefrm ** pbf,int cnt)
    {
    qsort((void *)pbf,cnt,sizeof(basefrm *),compareBaseforms);
    int i = 0;
    int j = 1;
    int k = 0;
//    int deleted = 0;
    while(j < cnt)
        {
        if(!pcmpBaseforms(pbf[i],pbf[j])) // 20050826 Error: pcmpBaseforms was cmpBaseforms_w
            {
            pbf[j]->getAbsorbedBy(pbf[i]);
            pbf[j] = 0;
//            ++deleted;
            }
        else
            {
            if(k != i)
                {
                pbf[k] = pbf[i];
                pbf[i] = 0;
                }
            i = j;
            ++k;
            }
        ++j;
        }
    if(k != i)
        {
        pbf[k] = pbf[i];
        pbf[i] = 0;
        }
//    assert(k+1 == cnt - deleted);
    return k+1;
    }

static void sortBaseforms_f(basefrm ** pbf,int cnt)
    {
    qsort((void *)pbf,cnt,sizeof(basefrm *),compareBaseforms_f);
    }

void text::AddField(field * fld)
    {
    if(fields == 0)
        fields = fld;
    else
        fields->addField(fld);
    }

static void invalidFormatString(char * Iformat,char * pformat)
    {
#if STREAM
    cout << "Invalid format string \"" << Iformat << "\"" << endl;
    cout << "                        " << setw((int)(strlen(Iformat) - strlen(pformat))) << "^" << endl;
#else
    printf("Invalid format string \"%s\"\n",Iformat);
    printf("                        %*c\n",(int)(strlen(Iformat) - strlen(pformat)),'^');
#endif
    }

char * globIformat = 0;
field * text::translateFormat(char * Iformat,field *& wordfield,field *& tagfield)
    {
    globIformat = Iformat;
    bool escape = false;
    bool afield = false;
    char * pformat;
    field * litteral = 0;
    for(pformat = Iformat;*pformat;++pformat)
        {
        if(afield)
            {
            afield = false;
            switch(*pformat)
                {
                case 'w':
                    if(wordfield)
                        {
                        invalidFormatString(Iformat,pformat);
                        exit(0);
                        }
                    wordfield = new readValue();
                    litteral = 0;
                    AddField(wordfield);
                    break;
                case 't':
                    if(tagfield)
                        {
                        invalidFormatString(Iformat,pformat);
                        exit(0);
                        }
                    tagfield = new readValue();
                    litteral = 0;
                    AddField(tagfield);
                    break;
                case 'd':
                    litteral = 0;
                    AddField(new readValue());
                    break;
                default:
                    {
                    invalidFormatString(Iformat,pformat);
                    exit(0);
                    }
                }
            }
        else if(escape)
            {
            escape = false;
            switch(*pformat)
                {
                case 's':
                    litteral = 0;
                    AddField(new readWhiteSpace);
                    break;
                case 'S':
                    litteral = 0;
                    AddField(new readAllButWhiteSpace);
                    break;
                case 't':
                    litteral = 0;
                    AddField(new readTab);
                    break;
                case 'n':
                    litteral = 0;
                    AddField(new readNewLine);
                    break;
                default:
                    {
                    invalidFormatString(Iformat,pformat);
                    exit(0);
                    }
                }
            }
        else if(*pformat == '\\')
            {
            escape = true;
            }
        else if(*pformat == '$')
            {
            afield = true;
            }
        else
            {
            if(!litteral)
                {
                litteral = new readLitteral(*pformat);
                AddField(litteral);
                }
            else
                litteral->add(*pformat);
            }
        }
    return fields;
    }





static int cmpUntagged( const void *arg1, const void *arg2 )
    {
    const Word * n1 = *(const Word * const *)arg1;
    const Word * n2 = *(const Word * const *)arg2;
    return (n1->*Word::cmp)(n2);
    }

static int cmpTagged( const void *arg1, const void *arg2 )
    {
    const taggedWord * n1 = *(const taggedWord * const *)arg1;
    const taggedWord * n2 = *(const taggedWord * const *)arg2;
    return (n1->*taggedWord::comp)(n2);
    }






#if STREAM
void text::Lemmatise(ostream * fpo
                    ,const char * Sep
                    ,tallyStruct * tally
                    ,unsigned int SortOutput
                    ,int UseLemmaFreqForDisambiguation
                    ,bool nice
                    ,bool DictUnique
                    ,bool baseformsAreLowercase
                    ,int listLemmas
                    ,bool mergeLemmas
                    )
#else
void text::Lemmatise(FILE * fpo
                    ,const char * Sep
                    ,tallyStruct * tally
                    ,unsigned int SortOutput
                    ,int UseLemmaFreqForDisambiguation
                    ,bool nice
                    ,bool DictUnique
                    ,bool baseformsAreLowercase
                    ,int listLemmas
                    ,bool mergeLemmas
                    )
#endif
    {
    flex::baseformsAreLowercase = baseformsAreLowercase;
    Word::DictUnique = DictUnique;
    baseformpointer::UseLemmaFreqForDisambiguation = UseLemmaFreqForDisambiguation;
    taggedWord::sep = Sep;
    basefrm::sep = Sep;
    if(InputHasTags)
        {
        pcmpBaseforms = cmpBaseforms_wt;
        switch(SortOutput)
            {
            case (SORTWORD<<4)+(SORTFREQ<<2)+SORTPOS:
            case (SORTWORD<<2)+SORTFREQ:
                pcmpBaseforms_f = cmpBaseforms_wft;
                taggedWord::comp = &taggedWord::cmp_wft;
                break;
            case (SORTWORD<<4)+(SORTPOS<<2)+SORTFREQ:
            case (SORTWORD<<2)+SORTPOS:
            case SORTWORD:
                pcmpBaseforms_f = cmpBaseforms_wtf;
                taggedWord::comp = &taggedWord::cmp_wtf;
                break;
            case (SORTPOS<<4)+(SORTFREQ<<2)+SORTWORD:
            case (SORTPOS<<2)+SORTFREQ:
                pcmpBaseforms_f = cmpBaseforms_tfw;
                taggedWord::comp = &taggedWord::cmp_tfw;
                break;
            case (SORTPOS<<4)+(SORTWORD<<2)+SORTFREQ:
            case (SORTPOS<<2)+SORTWORD:
                pcmpBaseforms_f = cmpBaseforms_twf;
                taggedWord::comp = &taggedWord::cmp_twf;
                break;
            case (SORTFREQ<<4)+(SORTWORD<<2)+SORTPOS:
            case (SORTFREQ<<2)+SORTWORD:
            case SORTFREQ:
                pcmpBaseforms_f = cmpBaseforms_fwt;
                taggedWord::comp = &taggedWord::cmp_fwt;
                break;
            case (SORTFREQ<<4)+(SORTPOS<<2)+SORTWORD:
            case (SORTFREQ<<2)+SORTPOS:
                pcmpBaseforms_f = cmpBaseforms_ftw;
                taggedWord::comp = &taggedWord::cmp_ftw;
                break;
            }
        }
    else
        {
        pcmpBaseforms = cmpBaseforms_w;
        switch(SortOutput)
            {
            case (SORTWORD<<4)+(SORTFREQ<<2)+SORTPOS:
            case (SORTWORD<<2)+SORTFREQ:
                pcmpBaseforms_f = cmpBaseforms_wf;
                Word::cmp = &Word::comp_wf;
                break;
            case (SORTWORD<<4)+(SORTPOS<<2)+SORTFREQ:
            case (SORTWORD<<2)+SORTPOS:
            case SORTWORD:
                pcmpBaseforms_f = cmpBaseforms_wf;
                Word::cmp = &Word::comp_wf;
                break;
            case (SORTPOS<<4)+(SORTFREQ<<2)+SORTWORD:
            case (SORTPOS<<2)+SORTFREQ:
                pcmpBaseforms_f = cmpBaseforms_fw;
                Word::cmp = &Word::comp_fw;
                break;
            case (SORTPOS<<4)+(SORTWORD<<2)+SORTFREQ:
            case (SORTPOS<<2)+SORTWORD:
                pcmpBaseforms_f = cmpBaseforms_wf;
                Word::cmp = &Word::comp_wf;
                break;
            case (SORTFREQ<<4)+(SORTWORD<<2)+SORTPOS:
            case (SORTFREQ<<2)+SORTWORD:
            case SORTFREQ:
                pcmpBaseforms_f = cmpBaseforms_fw;
                Word::cmp = &Word::comp_fw;
                break;
            case (SORTFREQ<<4)+(SORTPOS<<2)+SORTWORD:
            case (SORTFREQ<<2)+SORTPOS:
                pcmpBaseforms_f = cmpBaseforms_fw;
                Word::cmp = &Word::comp_fw;
                break;
            default:
                pcmpBaseforms_f = cmpBaseforms_wf;
                Word::cmp = &Word::comp_wf;
                break;
            }
        }

//    int cnt = 0;
    this->aConflict = this->aConflictTypes = this->newcnt = this->newcntTypes = 0;
    if(tally)
        {
        tally->totcnt = total;
        tally->totcntTypes = reducedtotal;
        }
//    unsigned long int k;
    cntD = 0;
    cntL = 0;
    if(nice)
        LOG1LINE("looking up words");
    if(Root)
        {
        for(size_t i = 0;i < N;++i)
            {
            Root[i]->lookup(this);
            }
        }
    if(tally)
        {
        tally->newhom = this->aConflict;
        tally->newhomTypes = this->aConflictTypes;
        tally->newcnt = this->newcnt;
        tally->newcntTypes = this->newcntTypes;
        }
    if(mergeLemmas)
        {
        basefrmarrD = new basefrm * [0];
        basefrmarrL = new basefrm * [cntL+cntD];
        ppD = &basefrmarrL[cntL];
        ppL = &basefrmarrL[0];
        cntL = cntL+cntD;
        cntD = 0;
        }
    else
        {
        basefrmarrD = new basefrm * [cntD];
        basefrmarrL = new basefrm * [cntL];
        ppD = &basefrmarrD[0];
        ppL = &basefrmarrL[0];
        }
    if(Root)
        {
        for(size_t i = 0;i < N;++i)
            Root[i]->assignTo(this->ppD,this->ppL);
        }
    if(mergeLemmas)
        {
        assert(cntD == 0);
        assert(cntL == ppD - &basefrmarrL[0]);
        }
    else
        {
        assert(cntD == ppD - &basefrmarrD[0]);
        assert(cntL == ppL - &basefrmarrL[0]);
        }
    sortBaseforms(basefrmarrD,cntD);
    sortBaseforms(basefrmarrL,cntL);

    if(UseLemmaFreqForDisambiguation != 2 /*Why?-> && lext::DictUnique*/)
        {
        if(nice)
            LOG1LINE("disambiguation by lemma frequency");
        if(Root)
            {
            for(size_t i = 0;i < N;++i)
                Root[i]->DissambiguateByLemmaFrequency();
            for(size_t i = 0;i < N;++i)
                Root[i]->decFreq();
            }
        if(nice)
            LOG1LINE("...disambiguated by lemma frequency");
        }
    if(TagFriends && InputHasTags)
        {
        if(nice)
            LOG1LINE("disambiguation by tag friends");
        if(Root)
            {
            for(size_t i = 0;i < N;++i)
                ((taggedWord**)Root)[i]->DissambiguateByTagFriends();
            }
        if(nice)
            LOG1LINE("...disambiguated by tag friends");
        }

    Word::setFile(fpo);
//    token::setFile(fpo);
    if(listLemmas) /* Make a list of lemmas, for each lemma listing all found word forms belonging to the same paradigm. 
                   Some word forms have ambiguous lemmas. Such word forms are listed under all possible lemmas. 
                   Lemma frequencies can therefore be too high.
                   */
        {
        if(  (pcmpBaseforms_f != cmpBaseforms_wf)
          && (pcmpBaseforms_f != cmpBaseforms_wtf)
          )
            {
            sortBaseforms_f(basefrmarrD,cntD);
            sortBaseforms_f(basefrmarrL,cntL);
            }
        if(nice)
            LOG1LINE("listing lemmas");
        basefrmarrD[0]->setFile(fpo);
        if(  (listLemmas & 1)
          && (listLemmas & 2)
          )
            {
            int d = 0;
            int l = 0;
            while(  (d < cntD)
                 && basefrmarrD[d] 
                 && (l < cntL)
                 && basefrmarrL[l]
                 )
                {
                if(pcmpBaseforms_f(basefrmarrD[d],basefrmarrL[l]) < 0)
                    {
                    if(basefrmarrD[d]->lemmaFreq())
                        basefrmarrD[d]->printb();
                    d++;
                    }
                else
                    {
                    if(basefrmarrL[l]->lemmaFreq())
                        basefrmarrL[l]->printB();
                    l++;
                    }
                }
            while(d < cntD && basefrmarrD[d])
                {
                if(basefrmarrD[d]->lemmaFreq())
                    basefrmarrD[d]->printb();
                d++;
                }
            while(l < cntL && basefrmarrL[l])
                {
                if(basefrmarrL[l]->lemmaFreq())
                    basefrmarrL[l]->printB();
                l++;
                }
            }
        else if(listLemmas & 1)
            {
            for(int K = 0;K < cntD && basefrmarrD[K];++K)
                basefrmarrD[K]->printb();
            }
        else if(listLemmas & 2)
            {
            for(int K = 0;K < cntL && basefrmarrL[K];++K)
                basefrmarrL[K]->printB();
            }
        if(nice)
            LOG1LINE("...listed lemmas");
        }
    else/* Make a list of word forms, for each word form listing all possible lemmas. */
        {
    //    unsorted[0]->setFile(fpo);
        if(nice)
            LOG1LINE("listing words");
        if(SortOutput)
            {
            if(nice)
                LOG1LINE("sorting words");
            if(Root)
                {
                if(InputHasTags)
                    qsort(Root,N,sizeof(Word *),cmpTagged);
                else
                    qsort(Root,N,sizeof(Word *),cmpUntagged);
                for(size_t i = 0;i < N;++i)
                    Root[i]->print();
                }
            }
        else 
            {
            if(nice)
                LOG1LINE("print Unsorted words");
            printUnsorted(fpo);
            }
        if(nice)
            LOG1LINE("...listed words");
        }
#if 0
    unsorted[0]->setFile(fpnew);
    for(k = 0;k < reducedtotal;++k)
        {
        u.unTaggedWords[k]->printnew(/*fpnew*/);
        }
    unsorted[0]->setFile(fpconflict);
    for(k = 0;k < reducedtotal;++k)
        {
        u.unTaggedWords[k]->printConflict(/*fpconflict*/);
        }
#endif
//    totcnt = k;
//    return cnt;
    if(nice)
        LOG1LINE("...text processed");
    delete [] basefrmarrD;
    delete [] basefrmarrL;
    
    if(Root)
        {
        for(size_t i = 0;i < N;++i)
            Root[i]->deleteSecondaryStuff();
        }
    }


void text::insert(const char * w)
    {
    if(!Hash)
        {
        Hash = new hash<Word>(&Word::itsWord,1000);
        }
    void * v;
    Word * wrd = Hash->find(w,v);
    if(wrd)
        {
        wrd->inc();
        }
    else
        {
        wrd = new Word(w);
        Hash->insert(wrd,v);
        }
    tunsorted[total] = wrd;
    Lines[lineno] = total;
    ++total;
    }

void text::insert(const char * w, const char * tag)
    {
    w = convert(w);
    tag = convert(tag);
    if(!Hash)
        {
        Hash = (hash<Word> *)new hash<taggedWord>(&Word::itsWord,1000);
        }
    void * v;
    taggedWord * wrd;
    for ( wrd = (taggedWord *)Hash->find(w,v)
        ; wrd && strcmp(wrd->m_tag,tag)
        ; wrd = (taggedWord *)Hash->findNext(w,v)
        )
        ;

    if(wrd)
        {
        wrd->inc();
        }
    else
        {
        wrd = new taggedWord(w,tag);
        Hash->insert((Word *)wrd,v);
        }
    tunsorted[total] = wrd;
    Lines[lineno] = total;
    ++total;
    }

void text::createUnTaggedAlternatives(
#ifndef CONSTSTRCHR
                                      const 
#endif
                                      char * w
                                      )
    {
    while(w && *w)
        {
        char * slash = strchr(w,'/');
        if(slash)
            *slash = '\0';
        insert(w);
        if(slash)
            {
            *slash = '/';
            w = slash + 1;
            }
        else
            w = 0;
        }
    }

void text::createUnTagged(const char * w)
    {
    if(*w)
        {
        insert(w);
        }
    }

void text::createTaggedAlternatives(
#ifndef CONSTSTRCHR
                                    const 
#endif
                                    char * w, const char * tag
                                    )
    {
    while(w && *w)
        {
        char * slash = strchr(w,'/');
        if(slash)
            {
            *slash = '\0';
            }
        insert(w, tag);
        if(slash)
            {
            *slash = '/';
            w = slash + 1;
            }
        else
            w = 0;
        }
    }

void text::createTagged(const char * w, const char * tag)
    {
    insert(w, tag);
    }



text::text(bool a_InputHasTags,bool nice)
           :Root(0)
           ,lineno(0)
           ,total(0)
           ,reducedtotal(0)
           ,fields(0)
           ,basefrmarrD(0)
           ,basefrmarrL(0)
           ,InputHasTags(a_InputHasTags)
           
    {
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    if(nice)
        LOG1LINE("counting words");
    }


    
text::~text()
    {
    delete fields;
    delete Root;
#ifdef COUNTOBJECTS
    --COUNT;
#endif
    }

bool text::setFormat(const char * cformat,const char * bformat,const char * Bformat,bool a_InputHasTags)
    {
    return Word::setFormat(cformat,bformat,Bformat,a_InputHasTags);
    }

void text::makeList()
    {
    if(Hash)
        {
        Root = Hash->convertToList(N);
        delete Hash;
        Hash = NULL;
        }
    }
#endif