#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <codecvt>
#include <chrono>

using namespace std;

int cutoffRank = 300;

int maxOOP = INT_MAX;   //If an N-gram is not in the category profile, it takes a maximum out-of-place value

int corpusSplit = 75;   //corpusSplit% go in the training set

int N = 5;              //N-grams

vector<int> counts;

bool preprocessCorpus();
bool removeRedundantChars();
bool buildProfiles();
string classifyFile(const string path);
bool testClassification();

struct Ngram
{
    wstring str;
    long long int count;
};

int main()
{
    int choice = 0;

    do {
        cout << "Please choose one of the following:\n1. Build categories' profiles;\n2. Classify document;\n3. Calculate efficiency.\n";

        cin >> choice;
    } while (choice < 1 && choice > 3);

    bool execution;

    if (choice == 1) {  //build profiles for categories

        // execution = preprocessCorpus();

        // execution = removeRedundantChars();

        execution = buildProfiles();

        if (!execution) {
            cout << "Error processing corpus!\n";
            return -1;
        }

    } else if (choice == 2) {

        string path;

        cout << "Please enter the path to the document: ";
        cin >> path;

        string exec = classifyFile(path);

        if (exec == "") {
            cout << "Error classifying file!\n";
            return -1;
        }

        cout << "Most likely language is: " << exec << endl;

    } else {

        execution = testClassification();

        if (!execution) {
            cout << "Error testing classification!\n";
            return -1;
        }
    }
    return 0;
}

bool preprocessCorpus()
{
    vector<string> folderVect = {"ParlaMint-AT.txt", "ParlaMint-ES.txt", "ParlaMint-GB.txt", "ParlaMint-HR.txt", "ParlaMint-SI.txt"};

    string languageName;
    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;

    for (int j = 0; j < folderVect.size(); ++j) {

        switch (j) {
            case 0:
                languageName = "german";
                break;

            case 1:
                languageName = "spanish";
                break;

            case 2:
                languageName = "english";
                break;

            case 3:
                languageName = "croatian";
                break;

            case 4:
                languageName = "slovene";
                break;
        }

        cout << languageName << endl;
        
        vector<string> fileVect;

        for (const auto & entry : filesystem::directory_iterator("korpus3\\" + folderVect[j])) {

                if (entry.path().extension() == ".txt") {

                    fileVect.push_back(entry.path().filename().u8string());
                }
        }

        string setFolder = "train";

        for (int i = 0; i < fileVect.size(); ++i) {

            cout << i + 1 << "/" << fileVect.size() << endl;

            wifstream fileToRead("korpus3\\" + folderVect[j] + "\\" + fileVect[i]);

            if (!fileToRead.is_open()) {
                cout << "Could not open file!\n";
                return false;
            }

            wstringstream fileStream;
            fileStream << fileToRead.rdbuf();

            fileToRead.close();

            wstring temp = fileStream.str();

            temp = regex_replace(temp, wregex(converter.from_bytes(fileVect[i].substr(0, fileVect[i].find(L'.'))) + L"[^\t]+\t"), L"");

            if ((((i + 1) * 100) / fileVect.size()) >= corpusSplit)
                setFolder = "test";

            wofstream fileToWrite("korpus3\\output\\" + languageName + "\\" + setFolder + "\\"+ fileVect[i]);

            fileToWrite << temp;

            fileToWrite.close();
        }
    }

    return true;
}

bool removeRedundantChars()
{
    // vector<string> folderVect = {"croatian", "english", "german", "slovene", "spanish"};
    vector<string> folderVect = {"serbian_fixed"};
    // vector<string> setVect = {"train", "test"};
    vector<string> setVect = {"train"};

    wregex redundantChars(L"[^\\w']|\\d");

    for (int i = 0; i < folderVect.size(); ++i) {

        cout << folderVect[i] << endl;

        for (int j = 0; j < setVect.size(); ++j) {

            vector<string> fileVect;

            for (const auto & entry : filesystem::directory_iterator("parlamint_zagovor\\" + folderVect[i] + "\\" + setVect[j])) {

                if (entry.path().extension() == ".txt")
                    fileVect.push_back(entry.path().filename().u8string());

            }

            //for each file in the specified path
            for (int k = 0; k < fileVect.size(); ++k) {

                cout << k << "/" << fileVect.size() << endl;

                wifstream fileToRead("parlamint_zagovor\\" + folderVect[i] + "\\" + setVect[j] + "\\" + fileVect[k]);
                fileToRead.imbue(std::locale(std::locale::classic(), new std::codecvt_utf8<wchar_t>));

                if (!fileToRead.is_open()) {
                    cout << "Could not open file!\n";
                    return false;
                }

                wstringstream fileStream;
                fileStream << fileToRead.rdbuf();

                fileToRead.close();

                wstring temp;

                wofstream fileToWrite("parlamint_zagovor2\\" + folderVect[i] + "\\" + setVect[j] + "\\" + fileVect[k]);
                fileToWrite.imbue(std::locale(std::locale::classic(), new std::codecvt_utf8<wchar_t>));

                //for every string separated by a space
                while (getline(fileStream, temp, L' ')) {

                    temp = regex_replace(temp, redundantChars, L"");

                    // wcout << temp << endl;

                    fileToWrite << temp << " ";
                }

                fileToWrite.close();
            }
        }
    }

    return true;
}

bool buildProfiles()
{
    // vector<string> folderVect = {"croatian", "english", "german", "slovene", "spanish"};
    vector<string> folderVect = {"serbian_fixed"};

    string set = "train";

    long double totalTime = 0.0;

    //for every language
    for (int i = 0; i < folderVect.size(); ++i) {

        vector<string> fileVect;
        vector<Ngram> ngrams;

        //get the names of the files in the training set
        // for (const auto & entry : filesystem::directory_iterator("korpus3\\output2\\" + folderVect[i] + "\\" + set)) {
        for (const auto & entry : filesystem::directory_iterator("parlamint_zagovor2\\" + folderVect[i] + "\\" + set)) {

            if (entry.path().extension() == ".txt")
                fileVect.push_back(entry.path().filename().u8string());
        }

        auto start = chrono::steady_clock::now();

        //for each file
        for (int j = 0; j < fileVect.size(); ++j) {

            if (j > 300)
                break;

            cout << j + 1 << "/" << fileVect.size() << endl;

            wifstream fileToRead("parlamint_zagovor2\\" + folderVect[i] + "\\" + set + "\\" + fileVect[i]);
            // wifstream fileToRead("korpus3\\output2\\" + folderVect[i] + "\\" + set + "\\" + fileVect[i]);
            fileToRead.imbue(std::locale(std::locale::classic(), new std::codecvt_utf8<wchar_t>));

            wstringstream sstream;
            sstream << fileToRead.rdbuf();

            fileToRead.close();

            wstring temp;

            //for every token (word)
            while (getline(sstream, temp, L' ')) {

                if (temp == L"")
                    continue;

                //create N-grams
                for (int k = 1; k <= N; ++k) {

                    //padding
                    if (k > 1) {

                        if (k == 2)
                            temp = L"_" + temp;

                        temp += L"_";
                    }
                    
                    for (int l = 0; l < temp.size() + 1 - k; ++l) {

                        wstring str;

                        for (int m = 0; m < k; ++m) 
                            str += temp[l + m];

                        bool ngramFound = false;

                        //check if ngram exists with that value
                        for (int m = 0; m < ngrams.size(); ++m) {

                            //if it does, increment its count
                            if (ngrams[m].str == str) {
                                
                                ngrams[m].count++;

                                ngramFound = true;

                                break;
                            }
                        }

                        //if it doesn't, create a new one, add it to the vector
                        if (!ngramFound) {

                            Ngram newNgram;
                            newNgram.str = str;
                            newNgram.count = 1;

                            ngrams.push_back(newNgram);
                        }
                    }
                }
            }
        }

        totalTime += (chrono::steady_clock::now() - start).count();

        cout << "Sorting...\n";

        // sort by count
        for (int k = 0; k < ngrams.size() - 1; ++k) {
            for (int l = (k + 1); l < ngrams.size(); ++l) {
                
                if (ngrams[k].count < ngrams[l].count)
                    iter_swap(ngrams.begin() + k, ngrams.begin() + l);

            }
        }

        cout << "Writing to file...\n";

        //write to output
        wofstream categoryOutput("profiles\\profile_" + folderVect[i] + ".txt");
        categoryOutput.imbue(std::locale(std::locale::classic(), new std::codecvt_utf8<wchar_t>));

        for (int j = 0; j < ngrams.size(); ++j) {

            if (j == cutoffRank)
                break;

            categoryOutput << ngrams[j].str << " " << ngrams[j].count << endl;
        }

        categoryOutput.close();

        cout << "File closed!\n";
    }

    cout << "Elapsed time: " << totalTime << "s\n";

    return true;
}

string classifyFile(const string path)
{
    vector<string> fileVect = {"profile_croatian", "profile_english", "profile_german", "profile_slovene", "profile_spanish", "profile_serbian_fixed", "profile_macedonian"};

    wifstream fileToRead(path);
    fileToRead.imbue(std::locale(std::locale::classic(), new std::codecvt_utf8<wchar_t>));
    
    if (!fileToRead.is_open()) {
        cout << "Could not open file!\n";

        return "";
    }

    wstringstream fileStream;
    fileStream << fileToRead.rdbuf();

    fileToRead.close();

    wstring temp;

    vector<Ngram> ngrams;

    while (getline(fileStream, temp, L' ')) {

        if (temp == L"")
            continue;

        //create N-grams
        for (int k = 1; k <= N; ++k) {

            if (k == 3)
                break;

            //padding
            if (k > 1) {

                if (k == 2)
                    temp = L"_" + temp;

                temp += L"_";
            }
            
            for (int l = 0; l < temp.size() + 1 - k; ++l) {

                wstring str;

                for (int m = 0; m < k; ++m) 
                    str += temp[l + m];

                bool ngramFound = false;

                //check if ngram exists with that value
                for (int m = 0; m < ngrams.size(); ++m) {

                    //if it does, increment its count
                    if (ngrams[m].str == str) {

                        ngrams[m].count++;

                        ngramFound = true;

                        break;
                    }
                }

                //if it doesn't, create a new one, add it to the vector
                if (!ngramFound) {

                    Ngram newNgram;
                    newNgram.str = str;
                    newNgram.count = 1;

                    ngrams.push_back(newNgram);
                }
            }
        }
    }

    // sort by count
    for (int k = 0; k < ngrams.size() - 1; ++k) {
        for (int l = (k + 1); l < ngrams.size(); ++l) {
            
            if (ngrams[k].count < ngrams[l].count)
                iter_swap(ngrams.begin() + k, ngrams.begin() + l);

        }
    }


    //keep only the first 300 elements
    if (ngrams.size() > 300)
        ngrams.erase(ngrams.begin() + 300, ngrams.end());

    //building category sums

    vector<long long int> scores;

    for (int i = 0; i < fileVect.size(); ++i) {

        long long int score = 0;
        vector<wstring> categoryProfile;

        wifstream categoryFile("profiles\\" + fileVect[i] + ".txt");
        categoryFile.imbue(std::locale(std::locale::classic(), new std::codecvt_utf8<wchar_t>));

        wstringstream sstream;
        sstream << categoryFile.rdbuf();

        categoryFile.close();

        wstring temp2;

        while (getline(sstream, temp2, L'\n'))
            categoryProfile.push_back(temp2.substr(0, temp2.find(L" ")));

        for (int j = 0; j < ngrams.size(); ++j) {

            bool ngramFound = false;

            for (int k = 0; k < categoryProfile.size(); ++k) {

                if (ngrams[j].str == categoryProfile[k]) {

                    ngramFound = true;

                    long long int distance = k - j;

                    if (distance < 0)
                        distance *= (-1);

                    score += distance;

                    break;
                }
            }

            if (!ngramFound)
                score += INT_MAX;
        }

        scores.push_back(score);
    }

    int minIterator = 0;

    for (int i = 0; i < fileVect.size(); ++i) {

        if (scores[minIterator] > scores[i])
            minIterator = i;

        // cout << fileVect[i] << " " << scores[i] << endl;
    }

    return fileVect[minIterator];
}

bool testClassification()
{
    vector<vector<string>> testSets;

    vector<string> sloveneTestSet;
    vector<string> croatianTestSet;
    vector<string> englishTestSet;
    vector<string> germanTestSet;
    vector<string> spanishTestSet;

    for (const auto & entry : filesystem::directory_iterator("korpus3\\output2\\slovene\\test")) {

        if (entry.path().extension() == ".txt") {

            sloveneTestSet.push_back(entry.path().u8string());
        }
    }

    testSets.push_back(sloveneTestSet);

    for (const auto & entry : filesystem::directory_iterator("korpus3\\output2\\croatian\\test")) {

        if (entry.path().extension() == ".txt") {

            croatianTestSet.push_back(entry.path().u8string());
        }
    }

    testSets.push_back(croatianTestSet);

    for (const auto & entry : filesystem::directory_iterator("korpus3\\output2\\english\\test")) {

        if (entry.path().extension() == ".txt") {

            englishTestSet.push_back(entry.path().u8string());
        }
    }

    testSets.push_back(englishTestSet);

    for (const auto & entry : filesystem::directory_iterator("korpus3\\output2\\german\\test")) {

        if (entry.path().extension() == ".txt") {

            germanTestSet.push_back(entry.path().u8string());
        }
    }

    testSets.push_back(germanTestSet);

    for (const auto & entry : filesystem::directory_iterator("korpus3\\output2\\spanish\\test")) {

        if (entry.path().extension() == ".txt") {

            spanishTestSet.push_back(entry.path().u8string());
        }
    }

    testSets.push_back(spanishTestSet);

    string languageStr;

    ofstream resultFile("result.txt");

    for (int i = 0; i < testSets.size(); ++i) {

        switch (i) {
            case 0:
                languageStr = "profile_slovene";
                break;
            case 1:
                languageStr = "profile_croatian";
                break;
            case 2:
                languageStr = "profile_english";
                break;
            case 3:
                languageStr = "profile_german";
                break;
            case 4:
                languageStr = "profile_spanish";
                break;
            default:
                cout << "Default case!\n";
                return false; 
        }

        cout << languageStr << ": \n";

        vector<vector<string>> incorrectGuesses;
        long long int correctGuesses = 0;

        for (int j = 0; j < testSets[i].size(); ++j) {

            cout << j << "/" << testSets[i].size() << endl;

            string classification = classifyFile(testSets[i][j]);

            if (classification == languageStr) {
                correctGuesses++;
            } else {
                vector<string> incorrectGuess;
                incorrectGuess.push_back(testSets[i][j]);
                incorrectGuess.push_back(classification);
                incorrectGuess.push_back(languageStr);

                incorrectGuesses.push_back(incorrectGuess);
            }

        }

        cout << languageStr << " correct guesses: " << correctGuesses << endl;
        resultFile << languageStr << " correct guesses: " << correctGuesses << endl;

        if (incorrectGuesses.size() != 0) {
            cout << languageStr << " incorrect guesses: " << incorrectGuesses.size() << endl;
            resultFile << languageStr << " incorrect guesses: " << incorrectGuesses.size() << endl;

            for (int j = 0; j < incorrectGuesses.size(); ++j) {
                for (int k = 0; k < incorrectGuesses[j].size(); ++k) {
                    resultFile << incorrectGuesses[j][k] << " ";
                }
                resultFile << endl;
            }
        }
    }

    return true;
}