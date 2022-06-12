#include "TranslationKey.h"

TranslationKey::TranslationKey() {
    clear();
}

TranslationKey::TranslationKey(std::string solved, std::string unsolved)
{
    clear();
    setFromSolvedUnsolved(solved, unsolved);
}

void TranslationKey::clear() {
    std::fill(arr, arr+26, 0);
}

void TranslationKey::setFromSolvedUnsolved(std::string solved, std::string unsolved)
{
    if(solved.length() != unsolved.length())
        return;
    for(int i = 0; i < solved.length(); i++) {
        char unsolvedChar = unsolved[i];
        char solvedChar = solved[i];
        if(solvedChar >= 'a' && unsolvedChar >= 'a' && solvedChar <= 'z' && unsolvedChar <= 'z')
            arr[unsolvedChar-'a'] = solvedChar;
    }
}

bool TranslationKey::compatible(TranslationKey& other)
{
    int used[26] = {0};
    bool shared[26] = {false};

    for(int i = 0; i < 26; i++) {
        if(arr[i] && other.arr[i]) {
            if(arr[i] != other.arr[i])
                return false; // False because the same input translates to two different outputs
            else
                shared[arr[i]-'a'] = true;
        }

        if(arr[i])
            used[arr[i]-'a']++;
        if(other.arr[i])
            used[other.arr[i]-'a']++;
    }

    for(int i = 0; i < 26; i++)
        if(used[i] > 2 || (used[i] == 2 && !shared[i]))
            return false; // False because two different inputs translate to the same output

    return true;
}

void TranslationKey::merge(TranslationKey& other)
{
    for(int i = 0; i < 26; i++)
        if(!arr[i])
            arr[i] = other.arr[i];
}

bool TranslationKey::solvesWord(std::string& input)
{
    for(int i = 0; i < input.length(); i++) {
        if(input[i] >= 'a' && input[i] <= 'z' && !arr[input[i]-'a'])
            return false;
    }
    return true;
}

std::string TranslationKey::makeSolvedString(std::string& input)
{
    char underscoreArray[26];
    for(int i = 0; i < 26; i++)
        underscoreArray[i] = (arr[i] ? arr[i] : '_');

    std::string output = input;
    for(int i = 0; i < output.length(); i++) {
        if(output[i] >= 'a' && output[i] <= 'z')
            output[i] = underscoreArray[output[i]-'a'];
        else if(output[i] >= 'A' && output[i] <= 'Z') {
            char nextChar = underscoreArray[output[i]-'A'];
            if(nextChar != '_')
                nextChar -= 32; // Capitalize
            output[i] = nextChar;
        }
    }
    return output;
}
