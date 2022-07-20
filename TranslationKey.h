#ifndef TRANSLATION_KEY_H
#define TRANSLATION_KEY_H

#include<string>
#include<array>

class TranslationKey
{
    std::array<char, 26> arr; // arr[0] is what an 'a' gets translated to, when decrypting

public:
    TranslationKey();
    TranslationKey(std::string solved, std::string unsolved);

    void clear();
    void setFromSolvedUnsolved(std::string solved, std::string unsolved);
    bool compatible(TranslationKey& other);
    void merge(TranslationKey& other);
    bool solvesWord(std::string& input);
    std::string makeSolvedString(std::string& input);
};

#endif // TRANSLATION_KEY_H
