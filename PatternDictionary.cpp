#include "PatternDictionary.h"

int PatternDictionary::intFromString(std::string& input)
{
    int output = 0;
    for(int i = 0; i < input.length(); i++)
        if(input[i] >= '0' && input[i] <= '9')
            output = (output*10) + (int)(input[i]-'0');
    return output;
}

void PatternDictionary::makeFromFile(std::string filename)
{
    std::string patternInput, wordInput, freqInput;

    std::ifstream filein(filename);
    while(!filein.eof()) {
        getline(filein, patternInput);
        getline(filein, wordInput);
        getline(filein, freqInput);

        int freqValue = intFromString(freqInput);

        allPatterns[patternInput].emplace_back(wordInput, freqValue);
        allWords.insert(wordInput);
    }
    filein.close();
}

bool PatternDictionary::checkWordExists(std::string& word) {
    return (allWords.find(word) != allWords.end());
}

std::vector<WordData>* PatternDictionary::getWordsFromPattern(std::string pattern)
{
    if(allPatterns.find(pattern) != allPatterns.end())
        return &allPatterns[pattern];
    return nullptr;
}
