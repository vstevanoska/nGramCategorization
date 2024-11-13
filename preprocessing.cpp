#include <iostream>
#include <filesystem>
#include <regex>
#include <fstream>
#include <sstream>

using namespace std;

int main()
{
    //enter the filenames of the corpus into a vector
    vector<string> fileVect;

    for (const auto & entry : filesystem::directory_iterator("parlamint_zagovor\\serbian\\train")) {

        if (entry.path().extension() == ".xml")
            fileVect.push_back(entry.path().filename().u8string());
    }


    //create directories for each category
    // filesystem::create_directory("english");
    // filesystem::create_directory("slovene");
    // filesystem::create_directory("croatian");
    // filesystem::create_directory("german");
    // filesystem::create_directory("spanish");
    // filesystem::create_directory("serbian_fixed");



    //go through every file and create new files, filtering only the parts that correspond to the desired language

    //create the desired regex objects
    // wregex englishRegex(L"<t");
    // wregex sloveneRegex(L"<tuv lang=\"SL-01\">\\n<seg>.+<\\/seg>\\n<\\/tuv>\\n");
    // wregex croatianRegex(L"<tuv lang=\"SH-HR\">\\n<seg>.+<\\/seg>\\n<\\/tuv>\\n");
    // wregex germanRegex(L"<tuv lang=\"DE-DE\">\\n<seg>.+<\\/seg>\\n<\\/tuv>\\n");
    // wregex spanishRegex(L"<tuv lang=\"ES-ES\">\\n<seg>.+<\\/seg>\\n<\\/tuv>\\n");
    // wregex serbianRegex(L"xml:lang=\"sr\">.+<");

    // wsmatch m;

    // wstring copy = L""; //object that we will use for copying the original string (so as not to change the original string for every language)

    // cout << "Sorting documents...\n";

    //for every raw file
    for (int i = 0; i < fileVect.size(); ++i) {

        cout << "File: " << i << "/" << fileVect.size() << endl;

        // if (i == 3)
        //     break;

        //open and read the file
        wifstream fileToRead("parlamint_zagovor\\serbian\\train\\" + fileVect[i]);

        if (!fileToRead.is_open()) {
            cout << "Could not open file!\n";
            return -1;
        }

        cout << "Reading to the stream...\n";
        wstringstream sstream;
        sstream << fileToRead.rdbuf();

        fileToRead.close();

        //create a file with the same name in the respective directory
        wofstream serbianFile("parlamint_zagovor\\serbian_fixed\\" + fileVect[i]);

        wstring str;
        bool textFound = false;
        int counter = 0;

        cout << "Writing to file...\n";

        while (getline(sstream, str, L'\n')) {

            if (str == L"<s>") {
                textFound = true;

            } else if (str == L"</s>") {
                textFound = false;

            } else if (textFound) {

                // if (counter < 20)
                //     wcout << str.substr(0, str.substr(str.find(L"\t") + 1, str.length() - str.find(L"\t")).find(L"\t") - 1) << L" ";

                if (str == L"<g/>")
                    continue;

                counter++;

                serbianFile << str.substr(0, str.substr(str.find(L"\t") + 1, str.length() - str.find(L"\t")).find(L"\t")) << L" ";
            }
        }

        // copy = originalStr;

        // if (i == 0)
        //     wcout << copy << endl;

        //search for every occurrence of the selected language and write it into the new file
        // while (regex_search(copy, m, serbianRegex)) {

        //     if (i == 1) {
        //         wcout << L"m is: " << m.str() << endl;
        //     }

        //     serbianFile << m.str();
        //     copy = m.suffix();
        // }

        serbianFile.close();
    }

}