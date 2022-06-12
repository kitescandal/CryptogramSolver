#include "PatternDictionary.h"
#include "englishLanguageData.h"

int PatternDictionary::intFromString(std::string& input)
{
    int output = 0;
    for(int i = 0; i < input.length(); i++)
        if(input[i] >= '0' && input[i] <= '9')
            output = (output*10) + (int)(input[i]-'0');
    return output;
}

std::string PatternDictionary::generatePatternString(std::string input)
{
    char lettersUsed[26] = {0};
    int uniqueLetters = 0;
    std::string pattern = input;

    for(int i = 0; i < input.length(); i++) {
        if(input[i] == '\'')
            continue;

        bool found = false;
        for(int j = 0; j < uniqueLetters; j++) {
            if(input[i] == lettersUsed[j]) {
                pattern[i] = ('a' + j);
                found = true;
                break;
            }
        }

        if(!found) {
            lettersUsed[uniqueLetters] = input[i];
            pattern[i] = ('a' + uniqueLetters);
            uniqueLetters++;
        }
    }

    return pattern;
}

void PatternDictionary::makeFromLanguageDataArray()
{
    std::string patternInput, wordInput, freqInput;
    std::stringstream ss(englishLanguageData);

    while(!ss.eof()) {
        getline(ss, wordInput);
        patternInput = generatePatternString(wordInput);
        getline(ss, freqInput);

        int freqValue = intFromString(freqInput);

        allPatterns[patternInput].emplace_back(wordInput, freqValue);
        allWords.insert(wordInput);
    }
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
