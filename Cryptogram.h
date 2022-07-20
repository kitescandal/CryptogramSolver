#ifndef CRYPTOGRAM_H
#define CRYPTOGRAM_H

#include "usTimer.h"
#include "CryptogramSolution.h"
#include "PatternDictionary.h"

#include<queue>
#include<algorithm>
#include<iostream>
#include<thread>
#include<mutex>
#include<atomic>
#include<condition_variable>

class Cryptogram
{
    static bool stringLengthGreater(const std::string& a, const std::string& b);
    bool allowedWordChar(char input);
    void splitWordsFromInputString(std::vector<std::string>& unsolvedWords, std::string input);

    bool finishedSolveAttempt();
    bool canProcessNextSolution();
    void threadSolve(PatternDictionary& dictionary, std::vector<std::string>& unsolvedWords, std::vector<std::string>& wordPatterns);
    int solveFromQueue(PatternDictionary& dictionary, std::vector<std::string> unsolvedWords, int skipWord);

    std::string input;
    bool partiallySolved;
    std::vector<WordData> stringSolutions;
    float solveTime;

    std::queue<CryptogramSolution> solutions;
    std::atomic<int> solutionsTested;
    std::atomic<int> solutionsFound;

    std::mutex queueMutex;
    std::condition_variable queueUpdateCondition;
    std::atomic<int> processingThreads;

public:
    Cryptogram(std::string input_) : input(input_), solveTime(0.0f), partiallySolved(false), solutionsTested(0) {}
    void solve(PatternDictionary& dictionary);
    void printSolutions();
};

#endif // CRYPTOGRAM_H
