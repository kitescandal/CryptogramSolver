#ifndef WORD_DATA_H
#define WORD_DATA_H

class WordData
{
public:
    WordData(std::string word_, int freq_) : word(word_), freq(freq_) {}

    bool operator< (const WordData &a) const {
        return (freq > a.freq);
    }

    std::string word;
    int freq;
};

#endif // WORD_DATA_H
