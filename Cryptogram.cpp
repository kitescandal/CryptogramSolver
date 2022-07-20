#include "Cryptogram.h"

bool Cryptogram::stringLengthGreater(const std::string& a, const std::string& b) {
    return (a.length() > b.length());
}

bool Cryptogram::allowedWordChar(char input) {
    return ((input >= 'a' && input <= 'z') || input == '\'');
}

void Cryptogram::splitWordsFromInputString(std::vector<std::string>& unsolvedWords, std::string input)
{
    bool inWord = false;
    int wordStart = 0;

    for(int i = 0; i < input.length(); i++) {
        if(input[i] >= 'A' && input[i] <= 'Z')
            input[i] += 32; // Make lowercase

        if(!inWord && allowedWordChar(input[i])) {
            inWord = true;
            wordStart = i;
        }
        else if(inWord && !allowedWordChar(input[i])) {
            std::string wordToAdd = input.substr(wordStart, i - wordStart);
            if(find(unsolvedWords.begin(), unsolvedWords.end(), wordToAdd) == unsolvedWords.end()) // Ignore duplicate words
                unsolvedWords.push_back(wordToAdd);
            inWord = false;
        }
    }

    if(inWord) {
        std::string wordToAdd = input.substr(wordStart);
        if(find(unsolvedWords.begin(), unsolvedWords.end(), wordToAdd) == unsolvedWords.end()) // Ignore duplicate words
            unsolvedWords.push_back(wordToAdd);
    }

    std::sort(unsolvedWords.begin(), unsolvedWords.end(), stringLengthGreater);
}

bool Cryptogram::finishedSolveAttempt() {
    return ((solutions.empty() && !processingThreads) || (!solutions.empty() && solutions.size() == solutionsFound));
}

bool Cryptogram::canProcessNextSolution() {
    return !solutions.empty() || Cryptogram::finishedSolveAttempt();
}

void Cryptogram::threadSolve(PatternDictionary& dictionary, std::vector<std::string>& unsolvedWords, std::vector<std::string>& wordPatterns)
{
    while(1) {
        std::unique_lock<std::mutex> queueLock(queueMutex);
        queueUpdateCondition.wait(queueLock, std::bind(canProcessNextSolution, this));

        if(finishedSolveAttempt())
            break;

        processingThreads++;

        CryptogramSolution frontSolution = solutions.front();
        solutions.pop();

        if(frontSolution.solvedWords == unsolvedWords.size()) {
            solutions.emplace(frontSolution);
            queueLock.unlock();
            queueUpdateCondition.notify_all();
            processingThreads--;
            continue;
        }

        queueLock.unlock();

        std::string& wordToSolve = unsolvedWords[frontSolution.solvedWords];

        solutionsTested++;

        bool firstWordIsCompleted = frontSolution.key.solvesWord(wordToSolve);
        bool firstWordIsValid = true;

        if(firstWordIsCompleted) {
            std::string firstWord = frontSolution.key.makeSolvedString(wordToSolve);
            firstWordIsValid = dictionary.checkWordExists(firstWord);
        }

        if(firstWordIsValid) {
            std::vector<WordData>* validWords = dictionary.getWordsFromPattern(wordPatterns[frontSolution.solvedWords]);
            if(validWords) {
                for(int i = 0; i < validWords->size(); i++) {
                    TranslationKey wordSolvedChars(validWords->at(i).word, wordToSolve);
                    if(wordSolvedChars.compatible(frontSolution.key)) {
                        if(frontSolution.solvedWords == unsolvedWords.size()-1)
                            solutionsFound++;

                        queueLock.lock();
                        solutions.emplace(frontSolution, wordSolvedChars, validWords->at(i).freq);
                        queueLock.unlock();
                        queueUpdateCondition.notify_all();
                    }
                }
            }
        }

        processingThreads--;
    }
}

int Cryptogram::solveFromQueue(PatternDictionary& dictionary, std::vector<std::string> unsolvedWords, int skipWord)
{
    if(skipWord >= 0) {
        std::cout << "SKIPPING WORD: " << unsolvedWords[skipWord] << "\n";
        unsolvedWords.erase(unsolvedWords.begin() + skipWord);
    }

    std::vector<std::string> wordPatterns;
    for(int i = 0; i < unsolvedWords.size(); i++)
        wordPatterns.push_back(dictionary.generatePatternString(unsolvedWords[i]));

    processingThreads = 0;

    std::vector<std::thread> solveThreads;
    int threadCount = std::max(1, (int)(std::thread::hardware_concurrency()));

    for(int i = 0; i < threadCount; i++)
        solveThreads.emplace_back(threadSolve, this, std::ref(dictionary), std::ref(unsolvedWords), std::ref(wordPatterns));

    for(int i = 0; i < threadCount; i++)
        solveThreads[i].join();

    return solutions.size();
}

void Cryptogram::solve(PatternDictionary& dictionary)
{
    usTimer solveTimer;
    solveTimer.start();

    solutions.emplace();

    partiallySolved = false;

    std::vector<std::string> unsolvedWords;
    splitWordsFromInputString(unsolvedWords, input);

    if(unsolvedWords.empty()) {
        solveTimer.stop();
        solveTime = solveTimer.getSeconds();
        return;
    }

    solutionsTested = 0;
    solutionsFound = 0;
    int wordCount = unsolvedWords.size();

    int resultsCount = solveFromQueue(dictionary, unsolvedWords, -1);

    for(int i = wordCount-1; !resultsCount && i >= 0; i--) { // If no solution is found, try again, excluding each word
        solutions.emplace();
        std::string skippedWord = unsolvedWords[i];
        resultsCount = solveFromQueue(dictionary, unsolvedWords, i);

        if(resultsCount && !solutions.front().key.solvesWord(skippedWord))
            partiallySolved = true;
    }

    stringSolutions.clear();
    while(solutions.size()) {
        stringSolutions.emplace_back(solutions.front().key.makeSolvedString(input), solutions.front().totalFreq);
        solutions.pop();
    }

    std::sort(stringSolutions.begin(), stringSolutions.end());

    solveTimer.stop();
    solveTime = solveTimer.getSeconds();
}

void Cryptogram::printSolutions()
{
    std::cout << "--------------------------------------------------------------------------------\n\n";

    int solutionsToOutput = std::min((int)stringSolutions.size(), 100);

    if(solutionsToOutput == 1 && partiallySolved)
        std::cout << "INCOMPLETE SOLUTION:\n\n";

    for(int i = 1; i <= solutionsToOutput; i++)
        std::cout << stringSolutions[solutionsToOutput-i].word << "\n\n";

    if(solutionsToOutput == 0)
        std::cout << "NO SOLUTION FOUND. :(\n\n";
    else if(solutionsToOutput == 100)
        std::cout << "(ONLY PRINTED TOP 100 SOLUTIONS)\n\n";

    if(solutionsToOutput > 1) {
        std::cout << "TOTAL " << (partiallySolved ? "INCOMPLETE " : "") << "SOLUTIONS FOUND: " << stringSolutions.size()
             << "\n\n\tMOST LIKELY " << (partiallySolved ? "INCOMPLETE " : "")
             << "SOLUTION:\n\t---------------------\n" << stringSolutions[0].word << "\n\n";
    }

    std::cout << "-------------------------------------- " << solveTime << " SEC --------------------------------------\n\n";
}
