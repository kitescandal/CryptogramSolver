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
    return ((solutionsQueue.empty() && !processingThreads) || (!solutionsQueue.empty() && solutionsQueue.size() == solutionsFound));
}

bool Cryptogram::canProcessNextSolution() {
    return (!solutionsQueue.empty() || Cryptogram::finishedSolveAttempt());
}

void Cryptogram::threadSolve(PatternDictionary& dictionary, std::vector<std::string>& unsolvedWords, std::vector<std::string>& wordPatterns)
{
    while(1) {
        std::list<CryptogramSolution> appendList;

        std::unique_lock<std::mutex> queueLock(queueMutex);
        queueUpdateCondition.wait(queueLock, std::bind(canProcessNextSolution, this));

        if(finishedSolveAttempt()) {
            queueUpdateCondition.notify_all();
            break;
        }

        processingThreads++;

        if(solutionsQueue.front().solvedWords == unsolvedWords.size()) { // If already solved, move to back of queue
            solutionsQueue.splice(solutionsQueue.end(), solutionsQueue, solutionsQueue.begin());
            queueLock.unlock();
            queueUpdateCondition.notify_all();
            processingThreads--;
            continue;
        }

        appendList.splice(appendList.begin(), solutionsQueue, solutionsQueue.begin()); // Move first in solve queue to new list
        queueLock.unlock();

        CryptogramSolution& frontSolution = appendList.front();
        solutionsTested++;

        std::string& wordToSolve = unsolvedWords[frontSolution.solvedWords];

        if(frontSolution.key.solvesWord(wordToSolve)) { // Check if next word is already solved by letters known
            std::string firstWord = frontSolution.key.makeSolvedString(wordToSolve);
            int wordFreq = dictionary.checkWordFrequency(firstWord);

            if(wordFreq != -1)
                appendList.emplace_back(frontSolution, frontSolution.key, wordFreq);
        }
        else {
            std::vector<WordData>* validWords = dictionary.getWordsFromPattern(wordPatterns[frontSolution.solvedWords]);
            if(validWords) {
                for(int i = 0; i < validWords->size(); i++) {
                    TranslationKey wordSolvedChars(validWords->at(i).word, wordToSolve);
                    if(wordSolvedChars.compatible(frontSolution.key))
                        appendList.emplace_back(frontSolution, wordSolvedChars, validWords->at(i).freq);
                }
            }
        }

        appendList.pop_front();

        if(!appendList.empty()) {
            if(frontSolution.solvedWords == unsolvedWords.size()-1)
                solutionsFound += appendList.size();

            queueLock.lock();
            solutionsQueue.splice(solutionsQueue.end(), appendList);
            queueLock.unlock();
            queueUpdateCondition.notify_all();
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
    int threadCount = std::max(1, (int)(std::thread::hardware_concurrency() - 1));

    for(int i = 0; i < threadCount; i++)
        solveThreads.emplace_back(threadSolve, this, std::ref(dictionary), std::ref(unsolvedWords), std::ref(wordPatterns));

    for(int i = 0; i < threadCount; i++)
        solveThreads[i].join();

    return solutionsQueue.size();
}

void Cryptogram::solve(PatternDictionary& dictionary)
{
    usTimer solveTimer;
    solveTimer.start();

    solutionsQueue.emplace_back();

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
        solutionsQueue.emplace_back();
        std::string skippedWord = unsolvedWords[i];
        resultsCount = solveFromQueue(dictionary, unsolvedWords, i);

        if(resultsCount && !solutionsQueue.front().key.solvesWord(skippedWord))
            partiallySolved = true;
    }

    stringSolutions.clear();
    while(solutionsQueue.size()) {
        stringSolutions.emplace_back(solutionsQueue.front().key.makeSolvedString(input), solutionsQueue.front().totalFreq);
        solutionsQueue.pop_front();
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

bool Cryptogram::foundSolution() {
    return (stringSolutions.size() > 0);
}

void Cryptogram::runTestCases(PatternDictionary& dictionary)
{
    std::vector<std::string> testStrings = {
        "VLG CJMAFVCJXVG VLUJR XIACV VLUD QAFTP UD VLXV RAAP LXIUVD XFG DA OCYL GXDUGF VA RUZG CW VLXJ IXP AJGD.",
        "RMG NVILU NZYG: RMG VUG DMV KLBK TR ALUUVR QG XVUG KMVZYX UGOGN TURGNNZFR RMG VUG DMV TK XVTUC TR.",
        "BR ZBN ACXRT L ONQVCNM NY BCT VCJR LME VLIRMV VN LM CMXRTVCALVCNM NY JLVBRJLVCGLI VQWVB ZCII GNJR VN LII NVBRQ PWRTVCNMT ZCVB L ERGCERE LEXLMVLAR.",
        "GTAYA FYA LANAYFC MWWE HYWGAIGVWSL FMFVSLG GADHGFGVWSL, KZG GTA LZYALG VL IWRFYEVIA.",
        "GQLOSDCM DR BFWCQSOCH UHXOQRH SF LOS DR QSGOIIM HNXHIC UM GDR FPS BOQJC.",
        "JU JT UIF SFTQPOTJCJMJUZ PG JOUFMMFDUVBMT UP TQFBL UIF USVUI BOE FYQPTF MJFT.",
        "SPETBSC TEJ CJYNGX WEJJ; SDJ ITEJC TBN SDJ LBCSEMXJBSC GW SDJLE SPETBBP JBCYTRJ SDJX.",
        "FNWRGVWRF NJR PVURF QNNPVFB ORNOPR QNT GBRVT QNPPI, ERGGRT GBXJ LVFR ORNOPR QNT GBRVT LVFSNW.",
        "UKOTWGM GUNS HF KZ JXMG QUGUNCMGM XSV MEUA WUSGNXWGUNM.",
        "KXY'G GNKX QFLBFZU ELUQODGDOH ZSDXLUD?",
        "EAVY S OYCEYCBY ARJ IUOQ ER BRCLYKE UCER S BKAVERHKSF QYKY:",
        "Ndh, uh qfklfkgkazgkly zoblfkgnu xddux gl td slfikyb!",
        "FXA SRZ YNJIJS UC SRZ YZXYUT VOXEEZT AUHI RZXIS XTE TUHIJYR AUHI YUHO.",
        "Ewmrpgn icllqpkfoqy nrdq r fwmh eohprlowm lpos qdqpx xqrp. Lnqx bwcpmqx kpwe lnq Hpqrl Frjqy lw lnq Hcfk wk Eqzogw, riwcl luw lnwcyrmv eofqy, rmv pqlcpm lw lnq mwpln rhrom om lnq yspomh. Lnrl pqeomvy eq wk lnq wfv bwjq: O bcyl kfqu om kpwe Eqzogw rmv iwx rpq ex rpey lopqv.",
        "ICHLH ULH RELH ICUS IQHSID-YENL ICENOUSV OTHWZHO EY JNIIHLYKZHO, JNI REICO ULH HGHS RELH SNRHLENO. ICHLH ULH UJENI ESH CNSVLHV USV YELID ICENOUSV OTHWZHO EY REICO ASEQS IE HMZOI ZS ICH QELKV. ICH ESKD WESIZSHSI QCHLH SE JNIIHLYKZHO KZGH ZO USIULWIZWU. Z QENKVS'I QUSI IE KZGH ICHLH HZICHL.",
        "RXLLFYCHKFD SYF KTDFOLD QKLN S NFSA, S LNVYSE, STA ST SRAVGFT, LQV STLFTTSF VT LNF NFSA STA S HVTB OXYHFA JYVRVDOKD. LNF STLFTTSF SYF TVL CVY CFFHKTB, LNFI SYF CVY DGFHHKTB. LNFI CFFA VT LNF TFOLSY CYVG CHVQFY NFSAD. LNFI XTOXYH LNFKY JYVRVDOKD DV LNFI OST DXOZ LNF TFOLSY LNYVXBN KL."
    };

    usTimer timer;
    timer.start();

    int solutionsFound = 0;

    for(int i = 0; i < testStrings.size(); i++) {
        Cryptogram testCrypt(testStrings[i]);
        testCrypt.solve(dictionary);
        if(testCrypt.foundSolution())
            solutionsFound++;
    }

    timer.stop();

    std::cout << "RAN ALL TESTS IN " << timer.getSecondsRounded(0.001f) << " SEC.\n";
    std::cout << "SOLVED " << solutionsFound << " / " << testStrings.size() << " TEST INPUTS.\n\n";
}
