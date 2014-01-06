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

#include "makedict.h"
#if defined PROGMAKEDICT
#include "freqfile.h"
#include "lem.h"
#include "readlemm.h"
#include "readfreq.h"
#include "caseconv.h"
#include "utf8func.h"
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <ctype.h>
//#define NDEBUG
#include <assert.h>

#if STREAM
#include <iostream>
using namespace std;
#endif

class DictNode;
class Lemma;
typedef int tchildrencount; // type for variables that are optimal for counting
                            // small numbers, but the value of which evantually
                            // will be typecasted to tchildren.

static char *** pstrings = NULL;
static char ** strings = NULL;
static DictNode ** pLeafs = NULL;
static char * STRINGS0;
static char * STRINGS; // STRINGS0 == STRINGS - 1
static Lemma * LEMMAS;
static tcount istrings = 0;
static char nul[] = "";

static unsigned long notypematch = 0;
static int g_added = 0;
static int notadded = 0;
static char * globflexform;
static char * globbf;
static unsigned long totcnt = 0L;
static unsigned long notypematchcnt = 0L;
static unsigned long addedcnt = 0L;
static unsigned long notaddedcnt = 0L;

#ifdef COUNTOBJECTS
int LemmaCOUNT = 0;
int DictNodeCOUNT = 0;
#endif

class Lemma
    {
    private:
        DictNode * m_parent;
        Lemma * next;
        char * Type;
        char * BaseForm;
        tsundry S;
public:
    const char * getbaseform()
        {
        return BaseForm;
        }
    const char * type()
        {
        return Type;
        }
    int Offset()
        {
        return S.Offset;
        }
    int frequency()
        {
        return S.frequency;
        }
    void move(tcount pos)
        {
        LEMMAS[pos].Type = Type;
        LEMMAS[pos].BaseForm = BaseForm;
        LEMMAS[pos].S = S;
        if(next)
            {
            LEMMAS[pos].next = LEMMAS + pos + 1;
            next->move(pos + 1);
            }
        }
    int cmp(const Lemma * lt)
        {
        assert(Type != 0);
        assert(lt->Type != 0);
        int strdif = strcmp(Type,lt->Type);
        if(strdif)
            return strdif;

        assert(BaseForm != 0);
        assert(lt->BaseForm != 0);
        strdif = strcmp(BaseForm,lt->BaseForm);
        if(strdif)
            return strdif;

        if(S.Offset > lt->S.Offset)
            return 1;
        else if(S.Offset < lt->S.Offset)
            return -1;

        if(S.frequency > lt->S.frequency)
            return 1;
        else if(S.frequency < lt->S.frequency)
            return -1;

        if(next)
            {
            assert(lt->next);
            return next->cmp(lt->next);
            }
        else
            return 0;
        }
    void setParent(DictNode * parent)
        {
        this->m_parent = parent;
        }
    Lemma()
        {
        Type = NULL;
        BaseForm = NULL;
        S.Offset = 0;
        next = NULL;
        m_parent = NULL;
        S.frequency = 0;
#ifdef COUNTOBJECTS
        ++LemmaCOUNT;
#endif
        }
    Lemma(const char * lextype,const char * baseform,ptrdiff_t offset,DictNode * parent)/*20120709 int -> ptrdiff_t*/
        {
        Type = new char[strlen(lextype)+1];
        strcpy(Type,lextype);
        BaseForm = new char[strlen(baseform)+1];
        strcpy(BaseForm,baseform);
        S.Offset = offset;
        next = NULL;
        this->m_parent = parent;
        S.frequency = 0;
#ifdef COUNTOBJECTS
        ++LemmaCOUNT;
#endif
        }
    Lemma(char * lextype,char * baseform,ptrdiff_t offset)/*20120709 int -> ptrdiff_t*/
        {
        Type = new char[strlen(lextype)+1];
        strcpy(Type,lextype);
        BaseForm = new char[strlen(baseform)+1];
        strcpy(BaseForm,baseform);
        S.Offset = offset;
        next = NULL;
        m_parent = NULL;
        S.frequency = 0;
#ifdef COUNTOBJECTS
        ++LemmaCOUNT;
#endif
        }
    ~Lemma()
        {
#ifdef COUNTOBJECTS
        --LemmaCOUNT;
#endif
        }
    tchildrencount count()
        {
        if(next)
            return 1 + next->count();
        else
            return 1;
        }
    void Lemmacnt(tcount * nLemmas)
        {
        ++*nLemmas;
        if(next)
            next->Lemmacnt(nLemmas);
        }
    tcount strcnt()
        {
        tcount ret = 0;
        if(*Type)
            ret = 1;
        if(*BaseForm)
            ret += 1;
        if(next)
            ret += next->strcnt();
        return ret;
        }
    void getStrings()
        {
        if(*Type)
            pstrings[istrings++] = &Type;
        else
            {
            delete [] Type;
            Type = nul;
            }
        if(*BaseForm)
            pstrings[istrings++] = &BaseForm;
        else
            {
            delete [] BaseForm;
            BaseForm = nul;
            }
        if(next)
            next->getStrings();
        }
    bool add(char * lextype,char * baseform,ptrdiff_t offset)/*20120709 int -> ptrdiff_t*/
        {
        if(  strcmp(lextype,Type) 
          || strcmp(baseform,BaseForm)
          || (unsigned int)offset != S.Offset
          )
            {
            if(next)
                return next->add(lextype,baseform,offset);
            else
                {
                next = new Lemma(lextype,baseform,offset);
                return true;
                }
            }
        return false;
        }
    bool addFreq(char * lextype,int n,char * bf,bool relaxed,ptrdiff_t offset)/*20120709 unsigned int -> ptrdiff_t*/
        {
        bool added = false;
        ptrdiff_t off = offset - S.Offset;/*20120709 int -> ptrdiff_t*/
        if(!strcmp(lextype,Type) && (!bf || off >= 0))
            {
            if(relaxed) 
                {
                assert(bf);
                char * bfoff = bf + S.Offset;
                size_t len = strlen(bfoff); 
                if(!strncmp(BaseForm,bfoff,len) && BaseForm[len] == ',')
                    {
                    S.frequency += n;
                    added = true;
                    //return true;
                    }
                }
            else
                {
                if(!bf || !strcmp(bf + S.Offset,BaseForm))
                    {
                    S.frequency += n;
                    added = true;
                    //return true;
                    }
                }
            }
        /*else*/ // Full form may have several (equal or different) base forms
                 // all having the same type!
        if(next)
            return next->addFreq(lextype,n,bf,relaxed,offset) || added;
        else
            {
            if(added)
                {
                return true;
                }
            else
                {
                notypematch++;
                notypematchcnt += n;
                return false;
                }
            }
        }
    tcount printLemmas(tcount pos,DictNode * parent,FILE * fp);
    int print(FILE * fp)
        {
        fprintf(fp,"%d %s %s %d",S.Offset,BaseForm,Type,S.frequency);
        if(next)
            {
            fprintf(fp,",");
            return 1 + next->print(fp);
            }
        return 1;
        }
    void binprint(FILE * fp)
        {
        tindex tmp;
        if(Type == nul)
            tmp = 0;
        else
            tmp = (tindex)(Type - STRINGS0);
        fwrite(&tmp,sizeof(tmp),1,fp);
        if(BaseForm == nul)
            tmp = 0;
        else
            tmp = (tindex)(BaseForm - STRINGS0);
        fwrite(&tmp,sizeof(tmp),1,fp);
        fwrite(&S,sizeof(S),1,fp);
        }
    };

typedef enum {casesensitive,caseinsensitive} Case;

class DictNode
    {
    private:
    static tcount iLeafs;

    DictNode * m_parent;
    DictNode * next;
    char * m_flexform;
    union
        {
        DictNode * sub;
        Lemma * type;
        } u;
    bool leaf;
    tchildrencount m_n; // number of subnodes or types (max 256)
public:
    void copy(DictNode * nd)
        {
        assert(leaf);
        assert(nd->leaf);
        delete u.type;
        u.type = nd->u.type;
        }
    tcount moveLemma(tcount pos)
        {
        assert(leaf);
        u.type->move(pos);
        delete u.type;
        u.type = LEMMAS + pos;
        return pos + m_n;
        }
    int cmp(const DictNode * n2) const
        {
        assert(leaf);
        int d = m_n - n2->m_n;
        if(d)
            return d;
        return u.type->cmp(n2->u.type);
        }
    int numberOfLextypes()
        {
        assert(leaf);
        return m_n;
        }
    tchildrencount count()
        {
        if(leaf)
            m_n = u.type->count();
        else
            m_n = u.sub->count();
        if(next)
            return 1 + next->count();
        else
            return 1;
        }
    tcount LeafCount(tcount * nLemmas)
        {
        tcount ret;
        if(leaf)
            {
            ret = 1;
            u.type->Lemmacnt(nLemmas);
            }
        else
            ret = u.sub->LeafCount(nLemmas);

        if(next)
            ret += next->LeafCount(nLemmas);
        return ret;
        }
    tcount strcnt()
        {
        tcount ret;
        if(*m_flexform)
            ret = 1;
        else
            ret = 0;
        if(leaf)
            ret += u.type->strcnt();
        else
            ret += u.sub->strcnt();
        if(next)
            ret += next->strcnt();
        return ret;
        }
    void getLemmas()
        {
        if(leaf)
            pLeafs[iLeafs++] = this;
        else
            u.sub->getLemmas();
        if(next)
            next->getLemmas();
        }
    void getStrings()
        {
        if(*m_flexform)
            pstrings[istrings++] = &m_flexform;
        else
            {
            delete [] m_flexform;
            m_flexform = nul;
            }
        if(leaf)
            u.type->getStrings();
        else
            u.sub->getStrings();
        if(next)
            next->getStrings();
        }
    void init(const char * flexform,DictNode * parent)
        {
        this->m_flexform = new char[strlen(flexform)+1];
        strcpy(this->m_flexform,flexform);
        next = NULL;
        this->m_parent = parent;
        }
    DictNode(const char * flexform,const char * lextype,const char * baseform,ptrdiff_t offset,DictNode * parent = NULL)/*20120709 int -> ptrdiff_t*/
        {
        init(flexform,parent);
        leaf = true;
        u.type = new Lemma(lextype,baseform,offset,this);
#ifdef COUNTOBJECTS
        ++DictNodeCOUNT;
#endif
        }
    DictNode(const char * flexform,Lemma * lextype,DictNode * parent = NULL)
        {
        init(flexform,parent);
        leaf = true;
        u.type = lextype;
        u.type->setParent(this);
#ifdef COUNTOBJECTS
        ++DictNodeCOUNT;
#endif
        }
    DictNode(const char * flexform,DictNode * subnode,DictNode * parent = NULL)
        {
        init(flexform,parent);
        leaf = false;
        u.sub = subnode;
        u.sub->m_parent = this;
#ifdef COUNTOBJECTS
        ++DictNodeCOUNT;
#endif
        }
    ~DictNode()
        {
        if(leaf)
            {
            }
        else
            delete u.sub;
        delete next;
#ifdef COUNTOBJECTS
        --DictNodeCOUNT;
#endif
        }
    void rename(int cut)
        {
        char * newstrng = new char[strlen(m_flexform + cut)+1];
        strcpy(newstrng,m_flexform + cut);
        delete [] m_flexform;
        m_flexform = newstrng;
        }
    bool add(char * flexform,char * lextype,char * baseform,ptrdiff_t offset);/*20120709 int -> ptrdiff_t*/
    bool addFreq(char * flexform,char * lextype,int n,char * bf,ptrdiff_t offset,Case cse);/*20120709 unsigned int -> ptrdiff_t*/
    tcount printLemmas(tcount pos,FILE * fp)
        {
        if(leaf)
            {
            pos = u.type->printLemmas(pos,this,fp);
            fprintf(fp,"\n");
            }
        else
            {
            pos = u.sub->printLemmas(pos,fp);
            }
        if(next)
            return next->printLemmas(pos,fp);
        else
            return pos;
        }

    tcount BreadthFirst_position(tcount Pos,tchildrencount length);

    void BreadthFirst_print(size_t indent,tchildrencount length,FILE * fp);
    void BreadthFirst_print(size_t indent,tchildrencount length,FILE * fp,char * wrd);
    void BreadthFirst_printBin(FILE * fp);
    void print(size_t indent,FILE * fp);
    void print(size_t indent,FILE * fp,char * wrd);
    int printLeaf()
        {
        assert(leaf);
#if STREAM
        cout << m_flexform << "[" << m_n << "](";
        int ret = u.type->print(stdout);
#else
        printf("%s",m_flexform);
        printf("[%d]",m_n);
        printf("(");
        int ret = u.type->print(stdout);
#endif
        LOG1LINE(")");
        return ret;
        }
    };

bool DictNode::add(char * flexform,char * lextype,char * baseform,ptrdiff_t offset)/*20120709 int -> ptrdiff_t*/
    {
    ptrdiff_t i,j;
    strcmpN(flexform,this->m_flexform,i,j);
    if(i > 0)
        { // (partial) overlap
        if(!this->m_flexform[i])
            { // new string incorporates this string
            if(flexform[i])
                { // new string longer than this string
                if(leaf)
                    { // make this DictNode a non-terminal DictNode and chain a new node after the first subnode
                    Lemma * tmptype = u.type;
                    leaf = false;
                    u.sub = new DictNode("",tmptype,this); // The first subnode is a leaf containing this node's previously owned Lemma
                    u.sub->next = new DictNode(flexform + i,lextype,baseform,offset); // The next subnode is the new leaf
                    return true;
                    }
                else
                    {

                    if(UTF8char(flexform+i,UTF8) < UTF8char(u.sub->m_flexform,UTF8))
                        {
                        DictNode * tmpnode = u.sub;
                        u.sub = new DictNode(flexform + i,lextype,baseform,offset,this); // The new leaf becomes the first in the sub-sequence
                        u.sub->next = tmpnode;
                        u.sub->next->m_parent = NULL;
                        return true;
                        }
                    else
                        return u.sub->add(flexform + i,lextype,baseform,offset); // The new leaf comes somewhere after the first node in the sub-sequence
                    }
                }
            else if(!leaf)
                { // new string equal to this string, but this string is not a leaf
                if(u.sub->m_flexform[0])
                    { // create leaf with empty string at start of sub-sequence
                    DictNode * tmpnode = u.sub;
                    u.sub = new DictNode("",lextype,baseform,offset,this);
                    u.sub->next = tmpnode;
                    u.sub->next->m_parent = NULL;
                    return true;
                    }
                else
                    { // The first node in the sub-sequence has a flexform that matches the new flexform
                    return u.sub->add(flexform + i,lextype,baseform,offset);
                    }
                }
            else // only difference between new string and this string is (possibly) lextype
                {
                return u.type->add(lextype,baseform,offset);
                // add new lextype (if not equal to existing type)
                }
            }
        else 
            {
            if(!flexform[i])
                { // new string is incorporated by this string. This string is longer
                  // The new node must become the first in the sub-sequence
                if(leaf)
                    {
                    Lemma * tmptype = u.type;
                    leaf = false;
                    u.sub = new DictNode("",lextype,baseform,offset,this);
                    u.sub->next = new DictNode(this->m_flexform + i,tmptype);
                    }
                else
                    {
                    DictNode * tmpsub = u.sub;
                    u.sub = new DictNode("",lextype,baseform,offset,this);
                    u.sub->next = new DictNode(this->m_flexform + i,tmpsub);
                    }
                }
            else
                {
                if(UTF8char(flexform+i,UTF8) > UTF8char(this->m_flexform+i,UTF8))
                    { 
                    // The new node must come somewhere after the first node in the sub-sequence
                    if(leaf)
                        { // make this DictNode a non-terminal DictNode
                        Lemma * tmptype = u.type;
                        leaf = false;
                        u.sub = new DictNode(this->m_flexform + i,tmptype,this);
                        }
                    else
                        { // Insert a new level of nodes. Push the current sub-sequence one level down
                        DictNode * tmpsub = u.sub;
                        u.sub = new DictNode(this->m_flexform + i,tmpsub,this); 
                        }
                    // The new flexnode comes straight after the first node in the (new) subsequence
                    u.sub->next = new DictNode(flexform + i,lextype,baseform,offset);
                    }
                else
                    { // The new node is alphabetically before this node
                    if(leaf)
                        { // make this DictNode a non-terminal DictNode
                        Lemma * tmptype = u.type;
                        leaf = false;
                        u.sub = new DictNode(flexform + i,lextype,baseform,offset,this);
                        u.sub->next = new DictNode(this->m_flexform + i,tmptype);
                        }
                    else
                        {
                        DictNode * tmpsub = u.sub;
                        u.sub = new DictNode(flexform + i,lextype,baseform,offset,this);
                        u.sub->next = new DictNode(this->m_flexform + i,tmpsub); 
                        }
                    }
                }
            this->m_flexform[i] = '\0'; // shorten the flexform to the common string
            char * newstrng = new char[strlen(this->m_flexform)+1];
            strcpy(newstrng,this->m_flexform);
            delete [] this->m_flexform;
            this->m_flexform = newstrng;
            return true;
            }
        }
    else if(next)
        {
        
        if(UTF8char(next->m_flexform,UTF8) > UTF8char(flexform,UTF8))
            { // The new node is alphabetically before the next node
            if(UTF8char(this->m_flexform,UTF8) < UTF8char(flexform,UTF8))
                {// The new node is alphabetically after this node
                DictNode * tmpnode = next;
                next = new DictNode(flexform,lextype,baseform,offset);
                next->next = tmpnode;
                return true;
                }
            else
                return u.type->add(lextype,baseform,offset);
            }
        else // The new node is alphabetically after the next node
            return next->add(flexform,lextype,baseform,offset);
        }
    else
        {
        if(  !leaf 
          || strcmp(u.type->getbaseform(),baseform) 
          || strcmp(u.type->type(),lextype) 
          || strcmp(this->m_flexform,flexform) 
          || offset != u.type->Offset()
          )
            {
            next = new DictNode(flexform,lextype,baseform,offset);
            return true;
            }
        else
            {
            return false;
            }
        }
    }

bool DictNode::addFreq(char * flexform,char * lextype,int n,char * bf,ptrdiff_t offset,Case cse)/*20120709 unsigned int -> ptrdiff_t*/
    {
    ptrdiff_t i,j;
    if(cse == casesensitive)
        {
        strcmpN(flexform,this->m_flexform,i,j);
        }
    else
        {
        strcasecmpN(flexform,this->m_flexform,i,j);
        }
    if(i > 0 || UTF8char(flexform+i,UTF8) == UTF8char(this->m_flexform+j/*i*/,UTF8))
        { // (partial) overlap
        if(!this->m_flexform[j/*i*/])
            { // looked-for string incorporates this string
            if(flexform[i])
                { // new string longer than this string
                if(leaf)
                    {
                    return false; // Nothing to do; word isn't in dictionary.
                    }
                else
                    {
                    if(UTF8char(flexform+i,UTF8) < UTF8char(u.sub->m_flexform,UTF8))
                        {
                        return false; // Nothing to do; word isn't in dictionary.
                        }
                    else
                        {
                        return u.sub->addFreq(flexform + i,lextype,n,bf,offset,cse);
                        }
                    }
                }
            else if(!leaf)
                { // new string equal to this string, but this string is not a leaf
                return u.sub->addFreq(flexform + i,lextype,n,bf,offset,cse);
                }
            else // only difference between new string and this string is (possibly) lextype
                {
                if(u.type->addFreq(lextype,n,bf,false,offset))
                    return true;
                if(bf == NULL)
                    return false;
                return u.type->addFreq(lextype,n,bf,true,offset);
                // add frequency to one of the lemmas
                }
            }
        else 
            {
            return false; // Nothing to do; word isn't in dictionary.
            }
        }
    else if(next && UTF8char(flexform,UTF8) > UTF8char(this->m_flexform,UTF8))
        {
        return next->addFreq(flexform,lextype,n,bf,offset,cse);
        }
    return false; // Nothing to do; word isn't in dictionary.
    }

        
tcount DictNode::BreadthFirst_position(tcount Pos,tchildrencount length)
    {
    // Unnecessary complex function. Previously used to compute index of each
    // DictNode in array. Now only used to return the size of the array.
    tcount subPos;
    subPos = Pos + length;
    for(DictNode * nxt = this;nxt;nxt = nxt->next)
        {
        if(!nxt->leaf)
            subPos = nxt->u.sub->BreadthFirst_position(subPos,nxt->m_n);
        }
    return subPos;
    }

void DictNode::BreadthFirst_print(size_t indent,tchildrencount length,FILE * fp,char * wrd)
    {
    DictNode * nxt;
    fprintf(fp,"%d\n",(int)length);
    size_t len = strlen(wrd); 
    for(nxt = this;nxt;nxt = nxt->next)
        {
        for(size_t i = indent;i;--i)
            fputc(' ',fp);
        fprintf(fp,"%s%s",wrd,nxt->m_flexform);
        fprintf(fp,"[%d]",nxt->m_n);
        if(nxt->leaf)
            {
            fprintf(fp,"(");
            nxt->u.type->print(fp);
            fprintf(fp,")");
            }
        fprintf(fp,"\n");
        }
    for(nxt = this;nxt;nxt = nxt->next)
        {
        if(!nxt->leaf)
            {
            strcpy(wrd+len,nxt->m_flexform);
            nxt->u.sub->BreadthFirst_print(indent + strlen(nxt->m_flexform),nxt->m_n,fp,wrd);
            }
        }
    wrd[len] = '\0';
    }

void DictNode::BreadthFirst_print(size_t indent,tchildrencount length,FILE * fp)
    {
    DictNode * nxt;
    fprintf(fp,"%d\n",(int)length);
    for(nxt = this;nxt;nxt = nxt->next)
        {
        for(size_t i = indent;i;--i)
            fputc(' ',fp);
        fprintf(fp,"%s",nxt->m_flexform);
        fprintf(fp,"[%d]",nxt->m_n);
        if(nxt->leaf)
            {
            fprintf(fp,"(");
            nxt->u.type->print(fp);
            fprintf(fp,")");
            }
        fprintf(fp,"\n");
        }
    for(nxt = this;nxt;nxt = nxt->next)
        {
        if(!nxt->leaf)
            {
            nxt->u.sub->BreadthFirst_print(indent + strlen(nxt->m_flexform),nxt->m_n,fp);
            }
        }
    }

void DictNode::BreadthFirst_printBin(FILE * fp)
    {
    DictNode * nxt;
    for(nxt = this;nxt;nxt = nxt->next)
        {
        tindex tmp;
        if(nxt->m_flexform == nul)
            tmp = 0;
        else
            tmp = (tindex)(nxt->m_flexform - STRINGS0);
        fwrite(&tmp,sizeof(tmp),1,fp);
        tchildren nxtn = tchildren(nxt->m_n);
        fwrite(&nxtn,sizeof(nxtn),1,fp);
        if(nxt->leaf)
            tmp = (tindex)(nxt->u.type - LEMMAS);
        else
            tmp = -1; // index to first child DictNode. reconstructed upon reading
        fwrite(&tmp,sizeof(tmp),1,fp);
        }
    for(nxt = this;nxt;nxt = nxt->next)
        {
        if(!nxt->leaf)
            {
            nxt->u.sub->BreadthFirst_printBin(fp);
            }
        }
    }

void DictNode::print(size_t indent,FILE * fp,char * wrd)
    {
    size_t len = strlen(wrd); 
    for(size_t i = indent;i;--i)
        fputc(' ',fp);
    fprintf(fp,"%s|%s",wrd,m_flexform);
    fprintf(fp,"[%d]",m_n);
    if(leaf)
        {
        fprintf(fp,"(");
        u.type->print(fp);
        fprintf(fp,")\n");
        }
    else
        {
        fprintf(fp,"\n");
        strcpy(wrd+len,m_flexform);
        u.sub->print(indent + strlen(m_flexform),fp,wrd);
        wrd[len] = '\0';
        }
    if(next)
        next->print(indent,fp,wrd);
    }

void DictNode::print(size_t indent,FILE * fp)
    {
    for(size_t i = indent;i;--i)
        fputc(' ',fp);
    fprintf(fp,"%s",m_flexform);
    fprintf(fp,"[%d]",m_n);
    if(leaf)
        {
        fprintf(fp,"(");
        u.type->print(fp);
        fprintf(fp,")\n");
        }
    else
        {
        fprintf(fp,"\n");
        u.sub->print(indent + strlen(m_flexform),fp);
        }
    if(next)
        next->print(indent,fp);
    }

tcount DictNode::iLeafs = 0;

tcount Lemma::printLemmas(tcount pos,DictNode * parent,FILE * fp)
    {
    if(parent)
        {
        fprintf(fp,PERCLD ": ",pos);
        }
    fprintf(fp,"%d %s %s %d",S.Offset,BaseForm,Type,S.frequency);
    if(next)
        {
        fprintf(fp,",");
        return next->printLemmas(pos + 1,NULL,fp);
        }
    else
        return pos + 1;
    }

static DictNode * root;

static bool add(char * baseform,char * flexform,char * lextype)
    {
    ptrdiff_t i;
    ptrdiff_t j;
    strcmpN(baseform,flexform,i,j);
    if(root->add(flexform,lextype,baseform+i,i))
        return true;
    else
        {
        return false;
        }
    }

static void addFreq(int n,char * flexform,char * lextype,char * bf)
    {
    globflexform = flexform;
    globbf = bf;
    totcnt += n;
    ptrdiff_t i = 0;
    if(bf)
        {
        ptrdiff_t j;
        strcmpN(bf,flexform,i,j);
        }
    if(root->addFreq(flexform,lextype,n,bf ? bf : NULL,i,casesensitive))
        {
        g_added++;
        addedcnt += n;
        }
    else
        {
        if(bf)
            {
            ptrdiff_t j;
            strcasecmpN(bf,flexform,i,j);
            }
        if(root->addFreq(flexform,lextype,n,bf ? bf/*+i*/ : NULL,i,caseinsensitive))
            {
            g_added++;
            addedcnt += n;
            }
        else
            {
            notadded++;
            notaddedcnt += n;
            }
        }
    }


static int compare(const void * arg1, const void * arg2)
    {
    return strcmp(**(const char * const * const * )arg1, **(const char * const * const * )arg2);
    }

static tlength compressStrings(tcount nstrings,tcount * nUniqueStrings)
    {
    pstrings = new char ** [nstrings];
    root->getStrings();

#if STREAM
    cout << "sorting " << nstrings << " strings..." << flush;
    qsort( (void *)pstrings, nstrings, sizeof( char * ), compare );
#else
    printf("sorting " PERCLD " strings...",nstrings);
    fflush(stdout);
    qsort( (void *)pstrings, nstrings, sizeof( char * ), compare );
#endif
    LOG1LINE("done");
    tcount i,j,k;
    tlength len = (tlength)strlen(*pstrings[0]) + 1;
    for(i = 0, j = 1;j < nstrings;++j)
        {
        if(strcmp(*pstrings[i],*pstrings[j]))
            {
            i = j;
            len += (tlength)strlen(*pstrings[i]) + 1;
            }
        }
    STRINGS = new char[len];
    STRINGS0 = STRINGS - 1;
    tlength pos;
    strcpy(STRINGS,*pstrings[0]);
    delete [] *pstrings[0];
    *pstrings[0] = STRINGS;
    pos = (tlength)strlen(*pstrings[0])+1;
#if STREAM
    cout << "compacting strings..." << endl;
#else
    printf("compacting strings...");
    fflush(stdout);
#endif
    for(i = 0, j = 1,k = 1;j < nstrings;++j)
        {
        if(strcmp(*pstrings[i],*pstrings[j]))
            {
            i = j;
            strcpy(STRINGS+pos,*pstrings[i]);
            delete [] *pstrings[i];
            *pstrings[i] = STRINGS+pos;
            pos += (tlength)strlen(*pstrings[i])+1;
            pstrings[k++] = pstrings[j];
            }
        else
            {
            delete [] *pstrings[j];
            *pstrings[j] = *pstrings[i];
            }
        }
    if(k < nstrings)
        pstrings[k] = NULL;
    strings = new char * [k];
    for(i = 0;i < k;++i)
        strings[i] = *pstrings[i];
    delete [] pstrings;
    pstrings = NULL;
    *nUniqueStrings = k;
#if STREAM
    cout << "resulting in " << k << " strings" << endl;
#else
    printf("resulting in " PERCLD " strings\n",k);
#endif
    return len;
    }

static int compareLeaf(const void *arg1, const void * arg2)
    {
    const DictNode * n1 = *(const DictNode * const *)arg1;
    const DictNode * n2 = *(const DictNode * const *)arg2;
    return n1->cmp(n2);
    }

static tcount compressLeafs(tcount nLeaf,tcount * nUniqueLemmas)
    {
    pLeafs = new DictNode * [nLeaf];
    root->getLemmas();
    tcount i,j,k;
#if STREAM
    cout << "sorting " << nLeaf << " leafs..." << flush;
    qsort( (void *)pLeafs, nLeaf, sizeof( DictNode * ), compareLeaf );
#else
    printf("sorting " PERCLD " leafs...",nLeaf);
    fflush(stdout);
    qsort( (void *)pLeafs, nLeaf, sizeof( DictNode * ), compareLeaf );
#endif
    LOG1LINE("done");
    tcount len = pLeafs[0]->numberOfLextypes();
    for(i = 0, j = 1;j < nLeaf;++j)
        {
        if(pLeafs[i]->cmp(pLeafs[j]))
            {
            i = j;
            len += pLeafs[i]->numberOfLextypes();
            }
        }
    LEMMAS = new Lemma[len];
    tcount pos = 0;
    pos = pLeafs[0]->moveLemma(pos);
    LOGANDFLUSH("compacting leafs...");
    for(i = 0, j = 1,k = 1;j < nLeaf;++j)
        {
        if(pLeafs[i]->cmp(pLeafs[j]))
            {
            i = j;
            pos = pLeafs[i]->moveLemma(pos);
            pLeafs[k++] = pLeafs[j];
            }
        else
            {
            pLeafs[j]->copy(pLeafs[i]);
            }
        }
    if(k < nLeaf)
        pLeafs[k/*++*/] = NULL;
    *nUniqueLemmas = k;
#if STREAM
    cout << "resulting in " << k << " leafs" << endl;
#else
    printf("resulting in " PERCLD " leafs\n",k);
#endif
    return len;
    }
    
int makedict(FILE * fpin,FILE * fpout,bool nice,const char * format,const FreqFile * freq,bool CollapseHomographs)
    {
    root = new DictNode("","","",0);
#if STREAM
    cout << "reading lemmas" << endl;
#else
    printf("reading lemmas\n");
#endif
    int failed;
    bool T;
    int cnt = readLemmas(fpin,format,add,CollapseHomographs,failed,T);
#if STREAM
    cout << cnt << " lemmas read, " << failed << " discarded" << endl;
#else
    printf("%d lemmas read, %d discarded\n",cnt,failed);
#endif
    if(failed)
        LOG1LINE("(see file \"discarded\")");
    while(freq)
        {
        if(!freq->itsName())
            {
#if STREAM
            cout << "No file name matching format " << freq->itsFormat() << endl;
#else
            printf("No file name matching format %s\n",freq->itsFormat());
#endif
            break;
            }
        if(!freq->itsFormat())
            {
#if STREAM
            cout << "No format matching file name " << freq->itsName() << endl;
#else
            printf("No format matching file name %s\n",freq->itsName());
#endif
            break;
            }

        FILE * ffreq = fopen(freq->itsName(),"r");
        if(ffreq)
            {
#if STREAM
            cout << "reading frequencies from " << freq->itsName() << " with format " << freq->itsFormat() << endl;
#else
            printf("reading frequencies from %s with format %s\n",freq->itsName(),freq->itsFormat());
#endif
            readFrequencies(ffreq,freq->itsFormat(),addFreq,T);
            }
        else
#if STREAM
            cout << "*** CANNOT OPEN " << freq->itsName() << endl;
#else
            printf("*** CANNOT OPEN %s\n",freq->itsName());
#endif
        freq = freq->Next();
        }
    LOG1LINE("counting children");
    tchildrencount nroot = root->count();

    LOG1LINE("counting strings");
    tcount nstrings = root->strcnt();
    tcount nUniqueStrings = 0;
    LOG1LINE("compressing strings");
    tlength stringBufferLen = compressStrings(nstrings,&nUniqueStrings);
    tcount nLemmas = -1; // compensate for root
    LOG1LINE("counting leafs");
    tcount nLeaf = root->LeafCount(&nLemmas);


    tcount nUniqueLemmas = 0;
    LOG1LINE("compressing leafs");
    tcount LemmaBufferLen = compressLeafs(nLeaf,&nUniqueLemmas);
    LOG1LINE("writing strings");
    tcount i;
    if(nice)
        {
        fprintf(fpout,"*** STRINGS ***\n" PERCLD "\n",stringBufferLen);
        for(i = 0;i < nUniqueStrings;++i)
            {
            fprintf(fpout,PERCLD " " PERCLD " %s\n",i,(tindex)(strings[i] - STRINGS),strings[i]);
            }
        }
    else
        {
        fwrite(&stringBufferLen,sizeof(stringBufferLen),1,fpout);
        fwrite(STRINGS,stringBufferLen,1,fpout);
        }
    LOG1LINE("writing lemmas");
    if(nice)
        {
        fprintf(fpout,"*** LEMMAS ***\n" PERCLD "\n",LemmaBufferLen);
        for(i = 0;i < nUniqueLemmas;++i)
            {
            fprintf(fpout,PERCLD " ",i);
            LEMMAS[i].print(fpout);
            fprintf(fpout,"\n");
            }
        }
    else
        {
        fwrite(&LemmaBufferLen,sizeof(LemmaBufferLen),1,fpout);
        for(i = 0;i < LemmaBufferLen;++i)
            LEMMAS[i].binprint(fpout);
        }
#if STREAM
    cout << "strings: " << nstrings << " unique: " << nUniqueStrings << endl;
    cout << "flexforms: "  << nLeaf << " lemmas: " << nLemmas << " unique: " << nUniqueLemmas << endl;
#else
    printf("strings: " PERCLD " unique: " PERCLD "\n",nstrings,nUniqueStrings);
    printf("flexforms: " PERCLD " lemmas: " PERCLD " unique: " PERCLD "\n",nLeaf,nLemmas,nUniqueLemmas);
#endif
    tcount nnodes = root->BreadthFirst_position(0,nroot);
    LOG1LINE("writing nodes");
    if(nice)
        {
        fprintf(fpout,"*** nodes ***\n" PERCLD "\n",nnodes);
        root->BreadthFirst_print(0,nroot,fpout);
        }
    else
        {
        fwrite(&nnodes,sizeof(nnodes),1,fpout);
        tchildren nrootwrite = (tchildren)nroot;
        fwrite(&nrootwrite,sizeof(nrootwrite),1,fpout);
        root->BreadthFirst_printBin(fpout);
        }

    delete root;
    delete [] strings;
    delete [] STRINGS;
    delete [] LEMMAS;

    if(totcnt > 0)
        {
#if STREAM
        cout << "frequencies added from " << g_added << " words (" << (double)addedcnt*100.0/(double)totcnt << "% of reference text)" << endl;
        cout << "frequencies from " << notadded - notypematch << " words are not added because they weren't found in the dictionary (" << (double)notaddedcnt*100.0/(double)totcnt << "% of reference text)" << endl;
        cout << "frequencies from " << notypematch << " words are not added because the types didn't agree. (" << (double)notypematchcnt*100.0/(double)totcnt << "% of reference text)" << endl;
#else
        printf("frequencies added from %d words (%f%% of reference text)\n",g_added,(double)addedcnt*100.0/(double)totcnt);
        printf("frequencies from %ld words are not added because they weren't found in the dictionary (%f%% of reference text)\n",notadded - notypematch,(double)notaddedcnt*100.0/(double)totcnt);
        printf("frequencies from %ld words are not added because the types didn't agree. (%f%% of reference text)\n",notypematch,(double)notypematchcnt*100.0/(double)totcnt);
#endif
        }
    return 0;
    }

#endif