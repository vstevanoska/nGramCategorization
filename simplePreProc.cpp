#include <iostream>
#include <regex>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <filesystem>

using namespace std;

int N = 5;

struct Ngram
{
    wstring str;
    long long int count;
};

string classifyFile(const string path);
bool testClassification();

int main()
{
    int choice = 0;

    do {
        cout << "Please choose one of the following:\n1. Build categories' profiles;\n2. Classify document;\n3. Calculate efficiency.\n";

        cin >> choice;
    } while (choice < 1 && choice > 3);

    bool execution;

    if (choice == 1) {

    } else if (choice == 2) {

        string path;

        cout << "Please enter the path to the document: ";
        cin >> path;

        // path = "korpus3\\output2\\croatian\\test\\ParlaMint-HR_2018-10-29-0.txt";

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

string classifyFile(const string path)
{
    vector<string> fileVect = {"profile_croatian", "profile_german", "profile_slovene", "profile_spanish"};

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

                //check if ngram exists with that hash value
                for (int m = 0; m < ngrams.size(); ++m) {

                    //if it does, increment its count
                    if (ngrams[m].str == str) {

                        // wcout << ngrams[m].hash << endl;
                        
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

    // cout << "Sorting...\n";

    // sort by count
    for (int k = 0; k < ngrams.size() - 1; ++k) {
        for (int l = (k + 1); l < ngrams.size(); ++l) {
            
            if (ngrams[k].count < ngrams[l].count)
                iter_swap(ngrams.begin() + k, ngrams.begin() + l);

        }
    }

    // cout << "Vec size: " << ngrams.size() << endl;

    //keep only the first 300 elements
    if (ngrams.size() > 300)
        ngrams.erase(ngrams.begin() + 300, ngrams.end());        //check if 300 exists actually

    // wofstream categoryOutput("profiles\\built_profile.txt");
    // categoryOutput.imbue(std::locale(std::locale::classic(), new std::codecvt_utf8<wchar_t>));

    // for (int i = 0; i < ngrams.size(); ++i)
    //     categoryOutput << ngrams[i].str << L" " << ngrams[i].count << endl;

    // categoryOutput.close();

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

    // cout << "Language: " << fileVect[minIterator] << endl;

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

    // for (const auto & entry : filesystem::directory_iterator("korpus3\\output2\\english\\test")) {

    //     if (entry.path().extension() == ".txt") {

    //         englishTestSet.push_back(entry.path().u8string());
    //     }
    // }

    // testSets.push_back(englishTestSet);

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
            // case 2:
            //     languageStr = "profile_english";
            //     break;
            case 2:
                languageStr = "profile_german";
                break;
            case 3:
                languageStr = "profile_spanish";
                break;
            // default:
            //     cout << "Default case!\n";
            //     return false; 
        }

        cout << languageStr << ": \n";

        vector<vector<string>> incorrectGuesses;
        long long int correctGuesses = 0;

        for (int j = 0; j < testSets[i].size(); ++j) {

            // if (j == 5)
            //     break;

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
                cout << endl;
            }
        }
    }

    return true;
}