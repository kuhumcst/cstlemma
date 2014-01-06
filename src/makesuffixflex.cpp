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
//#include "makesuffixflex.h"
#include "flex.h"
#if defined PROGMAKESUFFIXFLEX

#include "caseconv.h"
#include "readlemm.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#if STREAM
#include <iostream>
using namespace std;
#endif

bool WRITE = 0;

bool changes()
    {
    if(base::mutated || node::mutated || type::mutated)
        {
#if STREAM
        cout << "Mutations: baseforms " << base::mutated << " nodes " << node::mutated << " types " << type::mutated << endl;
#else
        printf("Mutations: baseforms %d nodes %d types %d\n",
            base::mutated,node::mutated,type::mutated);
#endif
        return true;
        }
    return false;
    }


void unchanged()
    {
    base::mutated = 0;
    node::mutated = 0;
    type::mutated = 0;
    }

#if defined PROGMAKESUFFIXFLEX
void base::removeNonFullWordsAsAlternatives()
    {
    while(m_next && !m_next->isFullWord())
        m_next = m_next->remove();
    if(m_next)
        m_next->removeNonFullWordsAsAlternatives();
    }

void base::removeUnusedPatterns(base *& prev)
    {
    if(refcnt == 0)
        {
        needed = false;
#if 1
        prev = remove();
        if(prev)
            prev->removeUnusedPatterns(prev);
#else
        if(m_next)
            m_next->removeUnusedPatterns(m_next/*,Type,buf*/);
#endif
        }
    else if(m_next)
        m_next->removeUnusedPatterns(m_next/*,Type,buf*/);
    }
#endif

#if TEST
void base::test(char * Text,char * save,char * buf,const char * Type)
    {
    char * bf;
    int borrow;
    if(Flex.Baseform(buf,Type,bf,borrow))
        {
        if(strcmp(save,bf))
            {
#if STREAM
            cout << Text << " Buf " << buf << " Difference: baseform should be: " << save << ", found: " << bf << endl;
#else
            printf("%s Buf %s Difference: baseform should be: %s, found: %s\n",Text,buf,save,bf);
#endif
            Exit();
            }
        }
    else
        {
#if STREAM
        cout << Text << " Baseform " << buf << " not found (should have baseform " << save << ")" << endl;
#else
        printf("%s Baseform %s not found (should have baseform %s)\n",Text,buf,save);
#endif
        Exit();
        }
    free(save);
    }
#endif

/*
Suppose we have a rule
[oid]id     (to constuct base form of mongolid, remove 'id' and insert 'oid')
This rather special rule would give wrong results for many other words ending in 'id'
(*) blid    ->  bloid 


*/

#if defined PROGMAKESUFFIXFLEX
bool base::removeUnneededPatterns(base *& prev,const char * Type,char tailBuffer[],node * parent)
    {
    size_t len = parent->itsLen();
    if(  !strncmp(m_baseform,tailBuffer,len) 
      && tailBuffer[len] /*We do not want to reduce the tail to nothing*/
      )
        // The deepest level of the tree makes no difference: the first 'len'
        // characters of the tail pattern are equal to the first 'len'
        // characters of the base form. Consider the possibility that we can
        // do without this last level. Perhaps we first have to get rid of a
        // seldomly used rule higher up in the tree that gives a wrong
        // prediction.
        // Example: can we 'shorten' the rule [bid]bid ?
        // On the other hand, we don't want to shorten the rule [b]b, as 
        // nothing would remain.
        {
        size_t offset;
        base * BaseOfHigherUpRule;
        if(Flex.Baseform2(tailBuffer+1,Type,BaseOfHigherUpRule,offset))
            // BaseOfHigherUpRule and offset now contain the prediction based 
            // on the left-trimmed tailBuffer. Offset is counted from the end
            // of the input (tailBuffer+1). BaseOfHigherUpRule is closer to
            // the root of the tree than this object, because the tail pattern
            // is shorter.
            {
            size_t wlen = strlen(tailBuffer+1);  // the length of the left-trimmed tail
            size_t borrow = wlen - offset;  //the number of characters that weren't
                            //used for deciding the base form by Flex.Baseform2
            if(  !BaseOfHigherUpRule->Next() // we only want to employ 
                                            // singe-valued baseforms
              && !strncmp(tailBuffer+1,m_baseform+1,borrow) // Apart from 
                 // the very first characters, also the borrowed characters 
                 // of the remaining tail must be equal to the base form.
                 // compare stems
              && !strcmp(m_baseform+1+borrow,BaseOfHigherUpRule->bf()) 
                // The prediction based on the shortened rule must also match.
              )
                {
                // Wow, we can just delete this rule; a rule higher up will
                // overtake.
                needed = false;
                BaseOfHigherUpRule->refcnt += refcnt/* - 1*/;
#if TEST
                char * save = strdup(baseform);
#endif
                prev = remove();
                if(prev)
                    prev->removeUnneededPatterns(prev,Type,tailBuffer,parent);
#if TEST
                test("Should be present already",save,tailBuffer,Type);
#endif
                return true;
                }
            else
                {
                /*
                If we just remove the current rule, we get false predictions.
                We will have to consider removing an odd rule higher up that
                spoils the prediction.
                */
                if(borrow > 0) // This ensures that we do not shorten the tail
                            //if all of the word was used to find the baseform.
                            //(Doing so would create ambivalent base forms.)
                    {
                    // There is room for creating a rule between this level 
                    // and the level of BaseOfHigherUpRule. So we do not have
                    // to delete BaseOfHigherUpRule!
#if 0
                    // Why don't we do anything? Answer: the program doesn't 
                    // terminate if we add a new rule here.
                    base * newbase = Flex.add(Type,tailBuffer+1,baseform+1,false);
                    needed = false;
                    if(newbase)
                        newbase->refcnt += refcnt - 1;
                    char * save = strdup(baseform);
                    prev = remove();
                    if(prev)
                        prev->removeUnneededPatterns(prev,Type,tailBuffer);
                    test("Shorten tail",save,tailBuffer,Type);
                    return true;
#else
                    if(m_next)
                        return m_next->removeUnneededPatterns(m_next,Type,tailBuffer,parent);
                    else
                        return false;
#endif
                    }
                else
                    if(  !BaseOfHigherUpRule->Next()    // We do not consider removing rules predicting ambiguous values, because they have nowhere to go.
                       && !BaseOfHigherUpRule->m_fullWord  // Also, rules that cannot be made longer have nowhere to go when deleted.
                       )
                    {
                    // NB: We cannot 'jump' over a level (borrow > 0): 
                    // the program doesn't terminate.
                    // Reason: the intention is that the odd high-up rule
                    // is resurrected at a lower level then were it was found.
                    // However, we can only garantee that it can be recreated 
                    // at the level just below its current level; it may then
                    // have the fullWord property. If it lands on the same 
                    // or lower level as the more general rule that we are
                    // going to create, then the problem isn't solved at all.
                    int rcnt = refcnt;
                    parent->unmark();
                    /*
                    See whether there are sister-rules that also would benifit
                    from deleting a higher up odd rule. How often are they
                    used?
                        [bid]bid
                        [did]did
                        [fid]fid
                        [gid]gid
                        [lid]lid
                        [oid]oid
                        [pid]pid
                        [rid]rid
                        [sid]sid
                        [uid]uid
                        [vid]vid
                    */
                    for(node * nxt = parent->Next();nxt;nxt = nxt->Next())
                        {
                        base * nbase = nxt->basef;
                        if(nbase && !nbase->m_next) // Homographs are not allowed to vote, they must stay put
                            {
                            char * nbaseform = nbase->m_baseform;
                            char * ntail = nxt->m_tail;
                            Strrev(ntail);
                            if(  !strncmp(ntail,nbaseform,nxt->m_len)
                              && !strcmp(nbaseform+nxt->m_len,m_baseform+len)
                              )
                                {
                                rcnt += nbase->refcnt;
                                nxt->mark();
                                }
                            Strrev(ntail);
                            }
                        }
                    // Compare usage. If the higher-up rule is not used
                    // as often as this rule and its sisters, them the
                    // higher-up rule has to give up.
                    if(BaseOfHigherUpRule->refcnt < rcnt/*refcnt*/)
                        {
                        // Delete existing rule having relatively few 
                          // applications. It will resurrect in longer format,
                          // deeper into the tree.
                        Flex.remove(BaseOfHigherUpRule);
                        base * newbase = Flex.add(Type,tailBuffer+1/*+borrow*/,m_baseform+1/*+borrow*/,false);
                        if(newbase)
                            {
                            newbase->refcnt = rcnt;
                            parent = parent->removeAllMarked();
                            }
                        needed = false;
#if TEST
                        char * save = strdup(baseform);
#endif
                        prev = remove();
                        if(prev)
                            prev->removeUnneededPatterns(prev,Type,tailBuffer,parent);
#if TEST
                        test("Replace incourant rule",save,tailBuffer,Type);
#endif
                        return true;
                        }
                    else 
                        {
                        if(m_next)
                            return m_next->removeUnneededPatterns(m_next,Type,tailBuffer,parent);
                        else
                            return false;
                        }
                    }
                else if(m_next)
                    return m_next->removeUnneededPatterns(m_next,Type,tailBuffer,parent);
                else
                    return false;
                }
            }
        else
            {
            WRITE = true;
            base * newbase = Flex.add(Type,tailBuffer+1,m_baseform+1,false);
            WRITE = false;
            needed = false;
            if(newbase)
                newbase->refcnt += refcnt - 1;
#if TEST
            char * save = strdup(baseform);
#endif
            prev = remove();
            if(prev)
                prev->removeUnneededPatterns(prev,Type,tailBuffer,parent);
#if TEST
            test("new rule",save,tailBuffer,Type);
#endif
            return true;
            }
        }
    else 
        {
        if(m_next)
            return m_next->removeUnneededPatterns(m_next,Type,tailBuffer,parent);
        else
            return false;
        }
    }

int base::Count()
    {
    if(m_next)
        return 1 + m_next->Count();
    else
        return 1;
    }

void base::resetAdded()
    {
    added = false;
    if(m_next)
        m_next->resetAdded();
    }

void base::resetRefCount()
    {
    needed = true;
    refcnt = 0;
    if(m_next)
        m_next->resetRefCount();
    }

void base::write(FILE * fp,const char * Type,const char * ending,int indent)
    {
    if(added)
        fputc('+',fp);
    else
        fputc(' ',fp);

    if(needed)
        fputc(' ',fp);
    else
        fputc('#',fp);

    if(refcnt == 0)
        {
        if(*ending == ' ')
            fputc('%',fp);
        else
            fputc('&',fp);
        }
    else
        fputc(' ',fp);
            
    if(m_fullWord)
        fputc('*',fp);
    else
        fputc(' ',fp);
    if(indent < 0) // second, third,... baseform
        {
        fprintf(fp,"|%5d%15s %*c%s]%s\n",refcnt,Type,-indent,'[',m_baseform,ending);
        if(m_next)
            m_next->write(fp,Type,ending,indent);
        }
    else
        {
        if(m_next)
            {
            fprintf(fp,"|%5d%15s %*c%s]%s\n",refcnt,Type,indent,'[',m_baseform,ending);
            m_next->write(fp,Type,ending,-indent);
            }
        else
            {
            fprintf(fp," %5d%15s %*c%s]%s\n",refcnt,Type,indent,'[',m_baseform,ending);
            }
        }
    }

void base::write(FILE * fp,const char * Type,const char * ending)
    {
    if(refcnt > flex::CutoffRefcount)
        {
        if(flex::showRefcount)
            fprintf(fp,"%s\t%s#%d\t%s\n",Type,m_baseform,refcnt,ending);
        else
            fprintf(fp,"%s\t%s\t%s\n",Type,m_baseform,ending);
        }
    if(m_next)
        m_next->write(fp,Type,ending);
    }
/* Does not work as expected. Idea was to delete everything deeper than 
    n levels, n=1,2,3... and to rebuild the tree.

bool node::consolidate(node *& prev)
    {
    bool done = false;
    if(!consolidated)
        {
        if(!fixed)
            {
            delete sub;
            sub = 0;
            if(basef)
                consolidated = true;
            else
                {
                prev = remove();
                if(prev)
                    prev->consolidate(prev);
                return false;
                }
            }
        else
            {
            consolidated = true;
            if(sub)
                done = sub->consolidate(sub);
            }
        }
    else if(sub)
        done = sub->consolidate(sub);
    else
        done = true;

    if(next)
        done = next->consolidate(next) && done;

    return done;
    }

bool node::fix() 
    {
    if(!fixed)
        {
        if(sub)
            {
            fixed = sub->fix();
            }

        if(!fixed && basef && basef->Next())
            {
            fixed = true;
            }
        }
    // It should suffice to call fix only once. The paths to ambivalent 
    // baseforms are established after the first time 
    // extractFlexPatternsFromTaggedText is looped ready and they are never 
    // deleted.
    if(next)
        // If next is fixed, this node need not necessarily be fixed (it is 
        // not in the path to next)!
        return next->fix() || fixed;
    else
        return fixed;
    }
*/

bool node::remove(base * bf)
    {
    if(basef == bf)
        {
        basef = basef->remove();
        return true;
        }
    if(m_sub && m_sub->remove(bf))
        return true;
    if(m_next && m_next->remove(bf))
        return true;
    return false;
    }

node * node::removeAllMarked()
    {
    if(marked)
        {
        assert(basef);
        delete basef;
        basef = 0;
        if(!m_sub)
            {
            node * nxt = m_next;
            m_next = 0;
            delete this;
            return nxt ? nxt->removeAllMarked() : 0;
            }
        }

    if(m_next)
        m_next = m_next->removeAllMarked();
    return this;
    }

void node::removeNonFullWordsAsAlternatives()
    {
    if(m_sub)
        m_sub->removeNonFullWordsAsAlternatives();
    if(basef)
        {
        base * nxt = basef;
        while(nxt && !nxt->isFullWord())
            {
            nxt = nxt->Next();
            }
        if(nxt)
            {
            while(!basef->isFullWord())
                {
                basef = basef->remove();
                }
            basef->removeNonFullWordsAsAlternatives();
            }
        }
    if(m_next)
        m_next->removeNonFullWordsAsAlternatives();
    }
//------------------

void node::removeUnusedPatterns(node *& prev,bool first)
    {
    if(m_sub)
        m_sub->removeUnusedPatterns(m_sub,true);
    if(basef)
        {
        basef->removeUnusedPatterns(basef);
        }
    if(!m_sub && !basef)
        {
        prev = remove();
        if(prev)
            prev->removeUnusedPatterns(prev,first);
        }
    else if(m_next)
        m_next->removeUnusedPatterns(m_next,false);
    }

void node::removeUnneededPatterns(node *& prev,const char * Type,char tailBuffer[],bool first,traverseTp how)
    {
    bool done = false;
    bool ambivalent = false;

    size_t len = strlen(tailBuffer);
    if(m_sub || basef)
        strcpy(tailBuffer+len,m_tail);
    
    if(basef)
        {
        if(!basef->Next())
            {
            if(how == normal) // Homographs can not be shortened
                {
                Strrev(tailBuffer);
                done = basef->removeUnneededPatterns(basef,Type,tailBuffer,this);
                Strrev(tailBuffer);
                }
            }
        else
            ambivalent = true;
        }
    
    if(!done && m_sub && (how == normal || how == onlydeeper))
        m_sub->removeUnneededPatterns(m_sub,Type,tailBuffer,true,ambivalent ? onlydeeper : normal);

    tailBuffer[len] = '\0';
    if(!m_sub && !basef)
        {
        prev = remove();
        if(prev)
            prev->removeUnneededPatterns(prev,Type,tailBuffer,first,how);
        }
    else if(m_next)
        m_next->removeUnneededPatterns(m_next,Type,tailBuffer,false,how);
    }
//---------------
int node::Count()
    {
    int cnt = 0;
    if(m_sub)
        cnt = m_sub->Count();
    if(basef)
        cnt += basef->Count();
    if(m_next)
        cnt += m_next->Count();
    return cnt;
    }

void node::resetAdded()
    {
    if(m_sub)
        m_sub->resetAdded();
    if(basef)
        basef->resetAdded();
    if(m_next)
        m_next->resetAdded();
    }
void node::resetRefCount()
    {
    if(m_sub)
        m_sub->resetRefCount();
    if(basef)
        basef->resetRefCount();
    if(m_next)
        m_next->resetRefCount();
    }

base * flex::update(char * baseForm,char * word,const char * tag,bool partial)
    {
    base * ret = 0;
    size_t i = 0;
    if(WRITE)
#if STREAM
        cout << "[" << baseForm << "]" << word << endl;
#else
        printf("[%s]%s\n",baseForm,word);
#endif
    // Find the shortest [basef]end combination that is distinct from
    // already existing combinations by removing a common "stem" from the start
    // such that stem+basef = baseForm and stem+end = word.
    if(types)
        {
        size_t wlen = strlen(word); 
        size_t offset = 0;
        Strrev(word);
        // Now the full form word is inverted!
        base * Base;
        // Retrieve current prediction into Base and offset. 
        // The length of the matched tail is stored in offset.
        if(types->Baseform(word,tag,Base,offset))
            {
            Strrev(word);
            // The full form is back to normal again.
            i = wlen - offset; // compute the length of the full form that 
                                    // isn't touched by the rule (the 'stem').
            if(  i > strlen(baseForm) // The true base form is shorter than
                    // the implied stem, or, in other words, the stem as  
                    // implied by the rule is too long.
              || strncmp(baseForm,word,i) // The implied stem is not a stem 
                                // at all: the first i characters of the
                                // word and the true base form are not equal
              )
                {
                i = 0;
                // compute the stem's (maximum) length
                while(word[i] && word[i] == baseForm[i])
                    {
                    ++i;
                    }
                }
            else
                {
#if 1
                if(i == 0 && !partial)
                    {
                    while(Base)
                        {
                        if(!strcmp(Base->bf(),baseForm + i))
                            // Agreement!
                            {
                            Base->setFullWord();
                            Base->incRefCount();
                            return Base; // nothing to do
                            }
                        
                        Base = Base->Next(); // Look at the next prediction 
                                            // (in case of ambiguous rule)
                        }
                    }
                else
                    {
                    // If Base holds a genuine ambiguity (two whole words) and
                    // this word is longer then we have to add a longer rule!
                    int cnt = 0;
                    base * Base2 = Base;
                    while(Base2)
                        {
                        if(Base2->isFullWord())
                            ++cnt;
                        Base2 = Base2->Next(); // Look at the next prediction 
                                        // (in case of ambiguous rule)
                        }
                    if(cnt < 2)
                        {
                        while(Base)
                            {
                            if(!strcmp(Base->bf(),baseForm + i))
                                // Agreement!
                                {
                                Base->incRefCount();
                                return Base; // nothing to do
                                }
                            Base = Base->Next(); // Look at the next prediction 
                                                // (in case of ambiguous rule)
                            }
                        }
                    }
                // The predicted stem is compatible with the true base form.
                // Now check that the predicted base form is good.
#else
    /*
    This code produces a smaller rule set, but does not disambiguate fully.
    Some non-genuine ambiguity is left.
    */
                while(Base)
                    {
                    if(!strcmp(Base->bf(),baseForm + i))
                        // Agreement!
                        {
                        if(WRITE)
#if STREAM
                            cout << "nothing to do [" << baseForm + i << "] == [" << Base->bf() << "]" << endl;
#else
                            printf("nothing to do [%s] == [%s]\n",baseForm + i,Base->bf());
#endif
                        if(i == 0 && !partial)
                            {
                            Base->setFullWord();
                            }
                        Base->incRefCount();
                        return Base; // nothing to do
                        }
                    Base = Base->Next(); // Look at the next prediction 
                                        // (in case of ambiguous rule)
                    }
                // None of the constructed baseforms is correct. Extend the tail of
                // the non-baseform ending with one character and store it with the
                // correct baseform ending.
#endif
                if(i > 0)
                    // No agreement. We need a more specific rule.
                    --i; // shorten the stem, or, make the tail pattern longer.
                }
            }
        else // Could not predict a base form. Possible causes: tag isn't 
            // known yet or no tail pattern matches end of word.
            {
            Strrev(word);
            // Full form back to normal.
            // compute the stem's (maximum) length
            while(word[i] && word[i] == baseForm[i])
                {
                ++i;
                }
            }
        }
    else // Just beginning: we do not have any rules at all.
        {
        // compute the stem's (maximum) length
        while(word[i] && word[i] == baseForm[i])
            {
            ++i;
            }
        }
    char * end = word+i;
    if(!*end && i && baseForm[i])
        {           // do not allow entries such as [e] , i.e. a rule saying 
                    // that, per default, a word can be extended with 'e'.
                    // The rule [] (a word remains unchanged) is not in the 
                    // rule base, but assumed a priori.
        --i;
        --end;
        }
    if(  (i == 0 && !partial) // We can allow an empty ending of the full form 
                              // or the base form if the stem has zero length 
                              // and the word is a complete word.
      || *end 
      || baseForm[i]
      )
        {
        ret = add(tag,end,baseForm+i,i == 0 && !partial);
        }
    else
        notadded++;
    return ret;
    }

base * flex::add(const char * tag,char * end,char * baseform,bool fullWord)
    {
    base * ret;
    Strrev(end);
    if(types)
        {
        ret = types->add(tag,end,baseform,fullWord,types);
        }
    else
        {
        types = new type(tag,end,baseform,fullWord,0);
        ret = types->Base();
        }
    Strrev(end);
    return ret;
    }


int flex::updateFlexRulesIfNeeded(char * dictBaseform,char * dictFlexform, char * dictType)
    {
    update(dictBaseform,dictFlexform,dictType,false);
    return 2;
    }

void node::write(FILE * fp,const char * Type,int indent,char tailBuffer[])
    {
    if(m_sub)
        {
        size_t Len = strlen(tailBuffer);
        strcpy(tailBuffer+Len," ");
        strcpy(tailBuffer+Len+1,m_tail);
        m_sub->write(fp,Type,indent+2,tailBuffer);
        tailBuffer[Len] = '\0';
        }
    if(basef)
        {
        size_t Len = strlen(tailBuffer);
        strcpy(tailBuffer+Len," ");
        strcpy(tailBuffer+Len+1,m_tail);
        Strrev(tailBuffer);
        basef->write(fp,Type,tailBuffer,indent);
        Strrev(tailBuffer);
        tailBuffer[Len] = '\0';
        }
    if(m_next)
        m_next->write(fp,Type,indent,tailBuffer);
    }

void node::write(FILE * fp,const char * Type,char tailBuffer[])
    {
    if(m_sub)
        {
        size_t Len = strlen(tailBuffer);
        strcpy(tailBuffer+Len,m_tail);
        m_sub->write(fp,Type,tailBuffer);
        tailBuffer[Len] = '\0';
        }
    if(basef)
        {
        size_t Len = strlen(tailBuffer);
        strcpy(tailBuffer+Len,m_tail);
        Strrev(tailBuffer);
        basef->write(fp,Type,tailBuffer);
        Strrev(tailBuffer);
        tailBuffer[Len] = '\0';
        }
    if(m_next)
        m_next->write(fp,Type,tailBuffer);
    }

void type::remove(base * bf)
    {
    if(end && end->remove(bf))
        return;
    if(m_next)
        m_next->remove(bf);
    }
void type::write(FILE * fp,bool nice)
    {
    char tailBuffer[256];
    tailBuffer[0] = '\0';
    if(end)
        {
        if(nice)
            end->write(fp,m_tp,1,tailBuffer);
        else
            end->write(fp,m_tp,tailBuffer);
        }
    if(m_next)
        m_next->write(fp,nice);
    }

void type::removeNonFullWordsAsAlternatives()
    {
    if(end)
        end->removeNonFullWordsAsAlternatives();
    if(m_next)
        m_next->removeNonFullWordsAsAlternatives();
    }

void type::removeUnusedPatterns(type *& prev)
    {
    if(end)
        {
        end->removeUnusedPatterns(end,true);
        if(!end)
            {
            prev = remove();
            if(prev)
                prev->removeUnusedPatterns(prev);
            }
        else if(m_next)
            m_next->removeUnusedPatterns(m_next);
        }
    else if(m_next)
        m_next->removeUnusedPatterns(m_next);
    }

void type::removeUnneededPatterns(type *& prev)
    {
    char tailBuffer[256];
    tailBuffer[0] = '\0';
    if(end)
        {
        end->removeUnneededPatterns(end,m_tp,tailBuffer,true,normal);
        if(!end)
            {
            prev = remove();
            if(prev)
                prev->removeUnneededPatterns(prev);
            }
        else if(m_next)
            m_next->removeUnneededPatterns(m_next);
        }
    else if(m_next)
        m_next->removeUnneededPatterns(m_next);
    }

int type::Count()
    {
    int cnt = 0;
    if(end)
        cnt = end->Count();
    if(m_next)
        cnt += m_next->Count();
    return cnt;
    }

void type::resetAdded()
    {
    if(end)
        end->resetAdded();
    if(m_next)
        m_next->resetAdded();
    }
void type::resetRefCount()
    {
    if(end)
        end->resetRefCount();
    if(m_next)
        m_next->resetRefCount();
    }

void flex::remove(base * bf)
    {
    if(types)
        types->remove(bf);
    }

void flex::removeNonFullWordsAsAlternatives()
    {
    if(types)
        types->removeNonFullWordsAsAlternatives();
    }

void flex::removeUnusedPatterns()
    {
    if(types)
        types->removeUnusedPatterns(types);
    }

void flex::removeUnneededPatterns()
    {
    if(types)
        types->removeUnneededPatterns(types);
    }

int flex::Count()
    {
    if(types)
        return types->Count();
    else
        return 0;
    }

void flex::resetAdded()
    {
    if(types)
        types->resetAdded();
    }

void flex::resetRefCount()
    {
    notadded = 0;
    unchanged();
    if(types)
        types->resetRefCount();
    }

void flex::write(FILE * fp,bool nice)
    {
    if(types)
        types->write(fp,nice);
    }

static int cnt = 0;
bool  addrule(char * baseform,char * flexform,char * lextype)
    {
    cnt += Flex.updateFlexRulesIfNeeded(baseform,flexform,lextype);
    return true;
    }

void flex::makeFlexRules
        (FILE * fpdict
        ,FILE * fpflex
        ,bool nice
        ,const char * format
        ,int & failed
        ,long CutoffRefcount
        ,bool showRefcount
        ,const char * flexrulefilename
        )
    {
    int count;
    char name[256];
    flex::CutoffRefcount = CutoffRefcount;
    flex::showRefcount = showRefcount;
    training = true;
    for(int SRep = 0;/*see test below*/;++SRep)
        {
        for(int Rep = 0;;++Rep)
            {
            for(int rep = 0;;++rep)
                {
                rewind(fpdict);
                resetRefCount();
                bool T;
                readLemmas(fpdict,format,addrule,true,failed,T);
                if(nice)
                    {
                    count = Count();
#if STREAM
                    cout << "rep " << SRep << "." << Rep << "." << rep << endl << count << " COUNT\n" << endl;
#else
                    printf("rep %d.%d.%d\n",SRep,Rep,rep);
                    printf("%d COUNT\n\n",count);
#endif
                    }
                if(!changes())
                    {
                    break;
                    }
                }

            // Unused alternatives are removed by removeNonFullWordsAsAlternatives!
            removeNonFullWordsAsAlternatives();
            
            if(nice)
                {
                count = Count();
                LOG1LINE("removeNonFullWordsAsAlternatives");
#if STREAM
                cout << count << " COUNT\n" << endl;
#else
                printf("%d COUNT\n\n",count);
#endif
                }
            if(!changes())
                {
                break;
                }
            }
        
        if(nice)
            {
            sprintf(name,"before%d.txt",SRep);
            FILE * fpflexTMP = fopen(name,"w");
            if(fpflexTMP)
                {
                write(fpflexTMP,nice);
                fclose(fpflexTMP);
                }
            }
        
        resetAdded();
        
        removeUnneededPatterns();
        
        
        if(nice)
            {
            count = Count();
            LOG1LINE("removeUnneededPatterns");
#if STREAM
            cout << count << " COUNT\n" << endl;
#else
            printf("%d COUNT\n\n",count);
#endif
            
            sprintf(name,"after%d.txt",SRep);
            FILE *fpflexTMP = fopen(name,"w");
            if(fpflexTMP)
                {
                write(fpflexTMP,nice);
                fclose(fpflexTMP);
                }
            }
        
        if(!changes())
            {
            break;
            }
        resetAdded();
        }
    if(nice)
        {
        LOG1LINE("Remove unused");
        }
    
    removeUnusedPatterns();
    write(fpflex,nice);
    if(flexrulefilename)
        {
        char name[256];
        for( flex::CutoffRefcount = 0
           ; flex::CutoffRefcount <= CutoffRefcount
           ; ++flex::CutoffRefcount
           )
            {
            sprintf(name,"%s%ld",flexrulefilename,flex::CutoffRefcount);
            FILE * f = fopen(name,"wb");
            if(f)
                {
                write(f,nice);
                fclose(f);
                }
            }
        }
    }

void Exit()
    {
    FILE * fp = fopen("tree.txt","w");
    Flex.write(fp,true);
    fclose(fp);
    exit(0);
    }

#endif
#endif