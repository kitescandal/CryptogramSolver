#ifndef PATTERN_DICTIONARY_H
#define PATTERN_DICTIONARY_H

#include<string>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<sstream>

#include "WordData.h"

class PatternDictionary
{
    int intFromString(std::string& input);

public:
    std::string generatePatternString(std::string input);
    void makeFromLanguageDataArray();
    int checkWordFrequency(std::string& word);
    std::vector<WordData>* getWordsFromPattern(std::string pattern);

    std::unordered_map<std::string, std::vector<WordData>> allPatterns;
    std::unordered_map<std::string, int> allWords;

    static const std::string englishLanguageData;
};

#endif // PATTERN_DICTIONARY_H
