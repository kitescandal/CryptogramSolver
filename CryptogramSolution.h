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
        chars.clear();
    }

    CryptogramSolution(CryptogramSolution& lastSolved, TranslationKey charsToApply, int freqToAdd)
    {
        totalFreq = lastSolved.totalFreq + freqToAdd;
        solvedWords = lastSolved.solvedWords + 1;

        chars = lastSolved.chars;
        chars.merge(charsToApply);
    }

    TranslationKey chars;
    int solvedWords;
    int totalFreq;
};

#endif // SOLUTION_H
