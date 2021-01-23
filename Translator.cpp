#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <exception>

// Конфигуратор файлов. Берет из config.txt адреса файлов и присваивает их потокам. Если до какого-то файла не может достучаться кидает invalid_argument
void filesConfigurer (std::ifstream& inputFile, std::ofstream& outputFile, std::ifstream& serviceWordsFile, std::ifstream& regexesFile) {
    std::ifstream configIn ("config.txt");
    std::string inputFileName;
    std::getline (configIn, inputFileName);
    FILE* fileChecker = std::fopen (inputFileName.c_str (), "r");
    if (!fileChecker) {
        throw std::invalid_argument("Failed to open input file.");
    }
    std::fclose (fileChecker);
    inputFile = std::ifstream (inputFileName);

    std::string outputFileName;
    std::getline (configIn, outputFileName);
    fileChecker = std::fopen (outputFileName.c_str (), "w");
    if (!fileChecker) {
        throw std::invalid_argument ("Failed to open output file.");
    }
    std::fclose (fileChecker);
    outputFile = std::ofstream (outputFileName);

    std::string serviceWordsFileName;
    std::getline (configIn, serviceWordsFileName);
    fileChecker = std::fopen (serviceWordsFileName.c_str (), "r");
    if (!fileChecker) {
        throw std::invalid_argument ("Failed to open service words file.");
    }
    std::fclose (fileChecker);
    serviceWordsFile = std::ifstream (serviceWordsFileName);

    std::string regexesFileName;
    std::getline (configIn, regexesFileName);
    fileChecker = std::fopen (regexesFileName.c_str (), "r");
    if (!fileChecker) {
        throw std::invalid_argument ("Failed to open regexes file.");
    }
    std::fclose (fileChecker);
    regexesFile = std::ifstream (regexesFileName);
}

// Удаляет комменты из текста. Предварительно считывает из serviceWordsStream 6 символов:
// shortComOne, shortComTwo - символы для короткого коммента (1 строка)
// longComBeginOne, longComBeginTwo - символы для начала длинного коммента
// longComEndOne, longComEndTwo - символы для конца длинного коммента
void decomment (std::string& text, std::ifstream& serviceWordsStream) {
    if (text.size () <= 1) {
        return;
    }

    char shortComOne, shortComTwo, longComBeginOne, longComBeginTwo, longComEndOne, longComEndTwo;
    serviceWordsStream >> shortComOne >> shortComTwo >>
        longComBeginOne >> longComBeginTwo >>
        longComEndOne >> longComEndTwo;

    bool readingShortComment = false, readingLongComment = false, previousCharWasComment = false;
    char previousChar, currentChar;
    std::string bufferString;
    for (int i = 1; i <= text.size (); ++i) {
        previousChar = text[i - 1];
        currentChar = text[i];
        if (!readingShortComment && !readingLongComment && !previousCharWasComment) {
            if (previousChar == shortComOne && currentChar == shortComTwo) {
                readingShortComment = true;
            }
            else if (previousChar == longComBeginOne && currentChar == longComBeginTwo) {
                readingLongComment = true;
            }
            else {
                bufferString += previousChar;
            }
        }
        else if (readingShortComment) {
            if (currentChar == 10 || currentChar == 13) {
                readingShortComment = false;
            }
        }
        else if (readingLongComment) {
            if (previousChar == longComEndOne && currentChar == longComEndTwo) {
                readingLongComment = false;
                previousCharWasComment = true;
            }
        }
        else if (previousCharWasComment) {
            previousCharWasComment = false;
        }
    }
    /*if (!readingShortComment && !readingLongComment) {
        bufferString += currentChar;
    }*/
    text = bufferString;
}

int main()
{
    std::ifstream inputStream;
    std::ofstream outputStream;
    std::ifstream serviceWordsStream;
    std::ifstream regexesStream;
    try {
        filesConfigurer (inputStream, outputStream, serviceWordsStream, regexesStream);
    }
    catch (std::invalid_argument& e) {
        std::cerr << "Error 0-0. " << e.what();
        return 0;
    }

    std::string inputText;
    inputText.assign ((std::istreambuf_iterator<char> (inputStream)), (std::istreambuf_iterator<char> ()));
    decomment (inputText, serviceWordsStream);
    outputStream << inputText;

    std::cout << "My job is done!";
}
