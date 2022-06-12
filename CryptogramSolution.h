#ifndef SOLUTION_H
#define SOLUTION_H

#include "TranslationKey.h"

class CryptogramSolution
{
public:
    CryptogramSolution()
    {
        totalFreq = 0;
        solvedWords = 0;
        key.clear();
    }

    CryptogramSolution(CryptogramSolution& lastSolved, TranslationKey keyToApply, int freqToAdd)
    {
        totalFreq = lastSolved.totalFreq + freqToAdd;
        solvedWords = lastSolved.solvedWords + 1;

        key = lastSolved.key;
        key.merge(keyToApply);
    }

    TranslationKey key;
    int solvedWords;
    int totalFreq;
};

#endif // SOLUTION_H
