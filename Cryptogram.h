#ifndef CRYPTOGRAM_H
#define CRYPTOGRAM_H

#include "usTimer.h"
#include "CryptogramSolution.h"
#include "PatternDictionary.h"

#include<queue>
#include<algorithm>
#include<iostream>

class Cryptogram
{
    static bool stringLengthGreater(const std::string& a, const std::string& b);
    bool allowedWordChar(char input);
    void splitWordsFromInputString(std::vector<std::string>& unsolvedWords, std::string input);
    int solveFromQueue(std::queue<CryptogramSolution>& solutions, PatternDictionary& dictionary, std::vector<std::string> unsolvedWords, int& solutionsTested, int skipWord);

public:
    Cryptogram(std::string input_) : input(input_), solveTime(0.0f), partiallySolved(false) {}
    void solve(PatternDictionary& dictionary);
    void printSolutions();

    std::string input;
    bool partiallySolved;
    std::vector<WordData> finalSolutions;
    float solveTime;
};

#endif // CRYPTOGRAM_H
