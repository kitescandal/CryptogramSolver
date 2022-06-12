#ifndef PATTERN_DICTIONARY_H
#define PATTERN_DICTIONARY_H

#include<string>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<fstream>

#include "WordData.h"

class PatternDictionary
{
    int intFromString(std::string& input);

public:
    void makeFromFile(std::string filename);
    bool checkWordExists(std::string& word);
    std::vector<WordData>* getWordsFromPattern(std::string pattern);

    std::unordered_map<std::string, std::vector<WordData>> allPatterns;
    std::unordered_set<std::string> allWords;
};

#endif // PATTERN_DICTIONARY_H
