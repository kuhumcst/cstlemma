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
#ifndef FLEX_H
#define FLEX_H

#include "defines.h"
#if (defined PROGLEMMATISE) || (defined PROGMAKESUFFIXFLEX)

#include <stdio.h>

#if defined PROGMAKESUFFIXFLEX
#define TEST 0
typedef enum {normal,/*notdeeper,*/onlydeeper} traverseTp;
class node;
#endif

extern bool training;

class base
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        char * m_baseform;
        base * m_next; //If next != 0, then this baseform is only one of 
                     // several that are derivable from the same ending
                     // (ambiguity, very unwelcome)
        int refcnt;
    public:
        static int nn;
        static int mutated;

        base(char * baseform,bool fullWord,base * next = 0);
        ~base()
            {
            ++mutated;
            delete [] m_baseform;
            delete m_next;
#ifdef COUNTOBJECTS
            --COUNT;
#endif
            }
        char * bf(){return m_baseform;}
        base * add(char * baseform,bool fullWord,base *& prev);
        base * Next(){return m_next;}
        void incRefCount(){++refcnt;}
        base * remove();
#if defined PROGMAKESUFFIXFLEX
    private:
        bool m_fullWord; // if true: there exists at least one word having this 
        // baseform for which the ending extends over all of the word
        // example : baseform 'sindet' ending == word 'sindede'
        // If fullWord is true then this baseform object cannot be deleted. 
        // If fullWord is false, and if the same tail has more than one 
        // baseform, then the object can be deleted. In the next pass,
        // the tail will be extended to disambiguate between the baseforms.
        bool needed;
        bool added;
    public:
        void write(FILE * fp,const char * type,const char * ending,int indent);
        void write(FILE * fp,const char * type,const char * ending);
        void resetRefCount();
        void resetAdded();
        int Count();
        void removeNonFullWordsAsAlternatives();
        void removeUnusedPatterns(base *& prev/*,const char * type,char buf[]*/);
        bool removeUnneededPatterns(base *& prev,const char * type,char buf[],node * parent);
        bool isFullWord(){return m_fullWord;}
        void setFullWord(){m_fullWord = true;}
#if TEST
        void test(char * Text,char * save,char * buf,const char * type);
#endif
#endif
#if defined PROGLEMMATISE
        void print(int n);
        int RefCount(){return refcnt;}
#endif
    };

class node
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
    public:
        static int mutated;
        node * m_next;
        char * m_tail;
        size_t m_len;
        base * basef;
        node * m_sub;
    public:
        base * Base(){return basef ? basef : m_sub ? m_sub->Base() : 0;}
        node(node * next,char * tail,char * baseform,bool fullWord,bool empty);
        node(node * next,char * tail,int n,char * baseform,bool fullWord,bool empty,node * sub);
        node(node * next,char * tail,int n,node * sub);
        ~node()
            {
            delete [] m_tail;
            delete m_next;
            delete m_sub;
            delete basef;
            ++mutated;
#ifdef COUNTOBJECTS
            --COUNT;
#endif
            }
        node * Next(){return m_next;}
        base * addsub(char * tail,int n,char * baseform,bool fullWord,node *& prev);
        bool Baseform(char * invertedWord,base *& bf,size_t & ln);
        bool BaseformSub(char * word,base *& bf,size_t & ln);
        base * add(char * tail,char * baseform,bool fullWord,node *& prev,bool empty);
        void cut(int c);
        node * remove();
#if defined PROGMAKESUFFIXFLEX
    private:
        bool marked;
    public:
        void unmark(){marked = false;if(m_next)m_next->unmark();}
        void mark(){marked = true;}
        node * removeAllMarked();
        size_t itsLen(){return m_len;}
        void write(FILE * fp,const char * type,int indent,char buf[]);
        void write(FILE * fp,const char * type,char buf[]);
        void removeUnusedPatterns(node *& prev,bool first);
        void removeUnneededPatterns(node *& prev,const char * type,char buf[],bool first,traverseTp how);
        void resetRefCount();
        void resetAdded();
        int Count();
        void removeNonFullWordsAsAlternatives();
        bool remove(base * bf);
#endif
#if defined PROGLEMMATISE
        void print(int n);
        void removeAmbiguous(node *& prev);// Remove all rules that have equally good competitors, e.g.
                                //  ADJ		[lille]små
                                //  ADJ		[liden]små
#endif
    };

class type
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        char * m_tp;
        node * end;
        type * m_next;
    public:
        base * Base(){return end ? end->Base():0;}
        static int mutated;
        type(const char * tp,char * tail,char * baseform,bool fullWord,type * next = 0);
        ~type()
            {
            delete [] m_tp;
            delete end;
            delete m_next;
            ++mutated;
#ifdef COUNTOBJECTS
            --COUNT;
#endif
            }
        base * add(const char * tp,char * tail,char * baseform,bool fullWord,type *& prev);
        bool Baseform(char * invertedWord,const char * tag,base *& bf,size_t & ln);
        type * remove();
#if defined PROGMAKESUFFIXFLEX
        void write(FILE * fp,bool nice);
        void resetRefCount();
        void resetAdded();
        int Count();
        void removeNonFullWordsAsAlternatives();
        void removeUnusedPatterns(type *& prev);
        void removeUnneededPatterns(type *& prev);
        void remove(base * bf);
#endif
#if defined PROGLEMMATISE
        char * Baseform(char * invertedWord,base *& bf,size_t & ln);
        void print();
        void removeAmbiguous(type *& prev);
#endif
    };

class flex
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        type * types;
    public:
        static long CutoffRefcount;
        static bool showRefcount;
        flex():types(0)
            {
#ifdef COUNTOBJECTS
            ++COUNT;
#endif
            };
        ~flex();
        void trim(char * s);
        base * add(char * line);
        bool Baseform2(char * word,const char * tag,base *& bf,size_t & offset);
        bool readFromFile(FILE * fpflex);
#if defined PROGMAKESUFFIXFLEX
    private:
        int notadded;
    public:
        base * update(char * baseForm,char * word,const char * tag,bool partial);
        base * add(const char * tp,char * tail,char * baseform,bool fullWord);
        void write(FILE * fp,bool nice);
        int updateFlexRulesIfNeeded(char * dictBaseform,char * dictFlexform, char * dictType);
        void resetRefCount();
        void resetAdded();
        int Count();
        void removeNonFullWordsAsAlternatives();
        void removeUnusedPatterns();
        void removeUnneededPatterns();
        void remove(base * bf);
        void makeFlexRules
            (FILE * fpdict
            ,FILE * fpflex
            ,bool nice
            ,const char * format
            ,int & failed
            ,long CutoffRefcount
            ,bool showRefcount
            ,const char * flexrulefilename
            );
#endif
#if defined PROGLEMMATISE
    public:
        static bool baseformsAreLowercase;
        bool Baseform(const char * word,const char * tag,const char *& bf,size_t & borrow);
        char * Baseform(const char * word,const char *& bf,size_t & borrow); // returns tag
        void print();
        void removeAmbiguous();
        bool readFromFile(FILE * fpflex,const char * flexFileName);
#endif
    };

extern flex Flex;
void Strrev(char * s);
#if defined PROGMAKESUFFIXFLEX
bool changes();
void unchanged();
#endif

/*

    1            ADJ  [øderativ]ederativ t 
   10            ADJ      [knyt]k n y t 
    1            ADJ      [ny] n y t 
    0            ADJ    [yt] y t                    <----- can be removed
  598            ADJ  [] t 


        |
        |
        V


    1            ADJ  [øderativ]ederativ t 
   10            ADJ      [knyt]k n y t 
    1            ADJ      [ny] n y t 
  598            ADJ  [] t 

Now, we would like to promote the second line
   10            ADJ      [knyt]k n y t 
by cutting off the 'k'

        |
        |
        V

    1            ADJ  [øderativ]ederativ t 
   10            ADJ      [nyt] n y t 
  598            ADJ  [] t 

but
    1            ADJ      [ny] n y t 

has no place to go, instead creating an extra baseform on the same ending

        |
        |
        V

    1            ADJ  [øderativ]ederativ t 
   10            ADJ      [nyt|ny] n y t 
  598            ADJ  [] t 


+++++++++++++++

     12            ADJ      [alid]a l i d           <----- remove
*    12            ADJ      [blid]b l i d           <----- remove
      1            ADJ        [goloid]g o l i d
     12            ADJ        [olid] o l i d        <----- remove
      0            ADJ      [loid] l i d            <----- replace stem by [lid], because alid, blid and olid are more frequent than loid and their stem+1 is equal
    227            ADJ    [id] i d 


      1            ADJ      [goloid]go l i d 
      x            ADJ      [lid] l i d 
    227            ADJ    [id] i d 




      1            ADJ        [goloid]g o l i d 
    227            ADJ    [id] i d 


*/

/*
cycle

read lemmas

 34500    *    12            ADJ       [bred]b r e d 
 31955    *    12            ADJ       [vred]v r e d 
 15091          2            ADJ     [rede]r e d 

shorten [rede]r e d to [ede]e d (unoccupied as yet)

 34500    *    12            ADJ       [bred]b r e d 
 31955    *    12            ADJ       [vred]v r e d 
 46525 +        2            ADJ   [ede]e d 

reread lemmas. fed, hed and led are exceptions to the now prominent [ede]e d rule

 48549 +  *    12            ADJ     [fed]f e d 
 48752 +  *    12            ADJ     [hed]h e d 
 49003 +  *    12            ADJ     [led]l e d 
 34500    *    12            ADJ       [bred]b r e d 
 31955    *    12            ADJ       [vred]v r e d 
 46525          2            ADJ   [ede]e d 

replace [ede]e d by the more general [ed]e d rule, derived from [fed]f e d. All other entries fall in.

 49499 +       56            ADJ   [ed]e d 

reread lemmas. [rede]r e d comes in as an exception to the [ed]e d rule, followed by [bred]b r e d and [vred]v r e d.

 51131 +  *    12            ADJ       [bred]b r e d 
 51093 +  *    12            ADJ       [vred]v r e d 
 50553 +        2            ADJ     [rede]r e d 
 49499         36            ADJ   [ed]e d 

remove the now unnecessary [ed]e d rule (incorporated by the [d]d rule).

 51131    *    12            ADJ       [bred]b r e d 
 51093    *    12            ADJ       [vred]v r e d 
 50553          2            ADJ     [rede]r e d 
*/
#endif
#endif
