/*
  Name: Cryptogram Solver
  Author: Daniel Stack
  Created: 05/06/2022
*/

#include<iostream>

#include "usTimer.h"
#include "PatternDictionary.h"
#include "Cryptogram.h"

int main()
{
    PatternDictionary dictionary;
    usTimer timer;

    timer.start();
    dictionary.makeFromLanguageDataArray();
    timer.stop();
    std::cout << "LOADED WORD PATTERN DATA IN " << timer.getSecondsRounded(0.001f) << " SEC.\n\n";

    Cryptogram::runTestCases(dictionary);

    while(1) {
        std::cout << "ENTER A CRYPTOGRAM:\n";
        std::string input;
        std::getline(std::cin, input);

        Cryptogram crypt(input);
        crypt.solve(dictionary);
        crypt.printSolutions();
    }

    return 0;
}
