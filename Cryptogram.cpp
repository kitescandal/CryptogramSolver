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

int Cryptogram::solveFromQueue(std::queue<CryptogramSolution>& solutions, PatternDictionary& dictionary, std::vector<std::string> unsolvedWords, int& solutionsTested, int skipWord)
{
    if(skipWord >= 0) {
        std::cout << "SKIPPING WORD: " << unsolvedWords[skipWord] << "\n";
        unsolvedWords.erase(unsolvedWords.begin() + skipWord);
    }

    std::vector<std::string> wordPatterns;
    for(int i = 0; i < unsolvedWords.size(); i++)
        wordPatterns.push_back(dictionary.generatePatternString(unsolvedWords[i]));

    while(solutions.size() && solutions.front().solvedWords < unsolvedWords.size()) {
        CryptogramSolution& frontSolution = solutions.front();
        std::string& wordToSolve = unsolvedWords[frontSolution.solvedWords];

        solutionsTested++;
        if(solutionsTested % 10000 == 0) {
            std::cout << "TESTED " << solutionsTested << ". REMAINING QUEUE = " << solutions.size()
                 << ". REMAINING WORDS = " << (unsolvedWords.size() - frontSolution.solvedWords) << ".\n";
        }

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
                    if(wordSolvedChars.compatible(frontSolution.key))
                        solutions.emplace(frontSolution, wordSolvedChars, validWords->at(i).freq);
                }
            }
        }

        solutions.pop();
    }

    return solutions.size();
}

void Cryptogram::solve(PatternDictionary& dictionary)
{
    usTimer solveTimer;
    solveTimer.start();

    std::queue<CryptogramSolution> solutions;
    solutions.emplace();

    partiallySolved = false;

    std::vector<std::string> unsolvedWords;
    splitWordsFromInputString(unsolvedWords, input);

    int solutionsTested = 0;
    int wordCount = unsolvedWords.size();

    int resultsCount = solveFromQueue(solutions, dictionary, unsolvedWords, solutionsTested, -1);

    for(int i = wordCount-1; !resultsCount && i >= 0; i--) { // If no solution is found, try again, excluding each word
        solutions.emplace();
        std::string skippedWord = unsolvedWords[i];
        resultsCount = solveFromQueue(solutions, dictionary, unsolvedWords, solutionsTested, i);

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
