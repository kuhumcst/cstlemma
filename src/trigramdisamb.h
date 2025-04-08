#ifndef TRIGRAMDISAMB_H
#define TRIGRAMDISAMB_H

#include <stdio.h>
struct trigramCnt;

class trigramdisamb
    {
    struct trigramCnt* trigramTable;
    unsigned long tabCount = 0;
    public:
        trigramdisamb() :trigramTable(0), tabCount(0) {}
        ~trigramdisamb();
        int init(FILE* fTrigramFrequencies);
        double weight(const char* lemma);
        const char* sortByWeight(/*const*/ char* strings, bool RulesUnique);
        bool hasTrigrams()
            {
            return trigramTable != 0 && tabCount > 0;
            }
    };

extern trigramdisamb trigrams;

#endif