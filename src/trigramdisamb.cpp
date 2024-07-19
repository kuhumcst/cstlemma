#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "trigramdisamb.h"

/*𝕌𝕋𝔽-𝟠*/
trigramdisamb trigrams;

struct trigramCnt
    {
    char trigram[16];
    double count;
    } trigramCnt;

trigramdisamb::~trigramdisamb()
    {
    delete[] trigramTable;
    }
int trigramdisamb::init(FILE* fTrigramFrequencies)
    {
    if(fTrigramFrequencies == 0)
        return 0;
    unsigned long lineCount = 0;
    int kar, prevkar = 0;
    enum expectations { tab, newline };
    expectations expect = tab;
    while((kar = fgetc(fTrigramFrequencies)) != EOF)
        {
        if(kar == '\n')
            {
            if(expect == tab)
                return 0;
            expect = tab;
            ++lineCount;
            }
        else if(kar == '\t')
            {
            if(expect == newline)
                return 0;
            expect = newline;
            ++tabCount;
            }
        prevkar = kar;
        }
    if((lineCount == tabCount && prevkar == '\n') || (lineCount + 1 == tabCount && prevkar != '\n' && prevkar != '\t'))
        {
        trigramTable = new struct trigramCnt[tabCount];
        rewind(fTrigramFrequencies);
        expect = tab;
        unsigned long index = 0;
        char numberBuffer[256];
        double sumlog = 0.0;
        lineCount = tabCount = 0;
        while((kar = fgetc(fTrigramFrequencies)) != EOF)
            {
            if(kar == '\n')
                {
                numberBuffer[index] = 0;
                double tmp = strtod(numberBuffer, 0);
                if(tmp > 0)
                    {
                    trigramTable[lineCount].count = log(tmp);
                    sumlog += trigramTable[lineCount].count;
                    }
                else
                    {
                    trigramTable[lineCount].count = 0;
                    }
                expect = tab;
                index = 0;
                ++lineCount;
                }
            else if(kar == '\t')
                {
                index = 0;
                expect = newline;
                ++tabCount;
                }
            else if(expect == tab)
                {
                if(index >= sizeof(trigramTable->trigram) - 1)
                    {
                    delete[] trigramTable;
                    return 0;
                    }
                trigramTable[lineCount].trigram[index++] = kar;
                trigramTable[lineCount].trigram[index] = 0;
                }
            else if(expect == newline)
                {
                if(index >= sizeof(numberBuffer) - 1)
                    {
                    delete[] trigramTable;
                    return 0;
                    }
                numberBuffer[index++] = kar;
                }
            prevkar = kar;
            }
        double offset = sumlog / tabCount;
        for(index = 0; index < tabCount; ++index)
            {
            trigramTable[index].count -= offset;
            }
        }
    else
        return 0;
    return 1;
    }

double trigramdisamb::weight(const char* lemma)
    {
    char buffer[64];
    if(strlen(lemma) > sizeof(buffer) + 3)
        return 0.0;
    strcpy(buffer + 1, lemma);
    buffer[0] = ' ';
    strcpy(buffer + (1 + strlen(lemma)), " ");
    int bufferindex = 0;
    int nextbufferindex = 0;
    int trigramindex = 0;
    char trigram[16];
    double wght = 0.0;
    int notfound = 0;
    double length = 0;
    for(;;)
        {
        int cnt;
        for(cnt = 0; cnt < 3; ++cnt)
            {
            if(cnt == 1)
                nextbufferindex = bufferindex;
            char first = buffer[bufferindex];
            if(first == 0)
                break;
            trigram[trigramindex++] = first;
            int off = 0;
            if((first << off) & 0x80)
                {
                for(;;)
                    {
                    ++off;
                    if((first << off) & 0x80)
                        {
                        trigram[trigramindex++] = buffer[bufferindex + off];
                        continue;
                        }
                    break;
                    }
                bufferindex += off;
                }
            else
                {
                bufferindex++;
                }
            }
        if(cnt == 3)
            {
            trigram[trigramindex] = 0;
            trigramindex = 0;
            bufferindex = nextbufferindex;
            struct trigramCnt* result = (struct trigramCnt*)bsearch(trigram, trigramTable, tabCount, sizeof(struct trigramCnt), (int (*)(const void*, const void*))strcmp);
            if(result)
                {
                wght += result->count;
                length += 1.0;
                }
            else
                {
                ++notfound;
                }
            }
        else
            break;
        }
    if(notfound == 0)
        return wght / log(length); /* There is no theory behind taking the log of length.It seems to strike a good balance.
                                    Not dividing gives long lemmas too much weight compared to shorter ones.
                                    Dividing by length does the opposite.
                                 */
    else
        return -INFINITY;
    }

struct stuff
    {
    char* string;
    char* nothing;
    double weight;
    };

static int stuffcmp(const void* A, const void* B)
    {
    double a, b;
    stuff* X = *(stuff**)A;
    stuff* Y = *(stuff**)B;
    a = X->weight;
    b = Y->weight;
    if(a < b)
        return 1;
    else if(a > b)
        return -1;
    else return 0;
    }

const char* trigramdisamb::sortByWeight(char* strings, bool RulesUnique)
    {
    char* q;
    if(!trigramTable || (q = strchr(strings, ' ')) == 0 || q[1] == 0)
        return strings;
    size_t len = strlen(strings);
    char* tmp = new char[len + 1];
    int cnt = 0;
    strcpy(tmp, strings);
    for(int i = strlen(tmp); --i >= 0;)
        {
        if(tmp[i] == ' ')
            {
            tmp[i] = 0;
            ++cnt;
            }
        }
    stuff* stuffs = new stuff[cnt + 1];
    stuff** pstuffs = new stuff * [cnt + 1];
    int j = 0;
    stuffs[j].string = tmp;
    char* p = tmp + 1;
    int vcnt = 0;
    for(; p < tmp + len; ++p)
        {
        if(*p == 0)
            {
            vcnt = 0;
            if(p[1])
                stuffs[++j].string = p + 1;
            }
        else if(*p == '\v' && vcnt == 0)
            {
            ++vcnt;
            *p = 0;
            stuffs[j].nothing = p + 1;
            }
        }
    for(int k = 0; k < cnt; ++k)
        {
        stuffs[k].weight = weight(stuffs[k].string);
        pstuffs[k] = stuffs + k;
        }
    qsort(pstuffs, cnt, sizeof(stuff*), stuffcmp);
    *strings = 0;
    if(RulesUnique)
        cnt = 1;
    for(int k = 0; k < cnt; ++k)
        {
        strcat(strings, pstuffs[k]->string);
        strcat(strings, "\v");
        strcat(strings, pstuffs[k]->nothing);
        strcat(strings, " ");
        }
    delete[] tmp;
    return strings;
    }