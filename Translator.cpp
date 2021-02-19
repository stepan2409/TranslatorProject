#pragma once
#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <vector>
#include <exception>
#include <sstream>
#include <codecvt>
#include <locale>
#include <windows.h>
#include <map>
#include "LexemChecker.h"

typedef std::pair<int, std::wstring> lexem;
std::locale rusLoc = std::locale("Russian");

std::map<std::wstring, std::wstring> serviceDict;
std::wstring regexServiceCode;

void setLocale(std::wistream &str)
{
	str.imbue(std::locale(str.getloc(), new std::codecvt_utf8<wchar_t>));
}

// Конфигуратор файлов. Берет из config.txt адреса файлов и присваивает их потокам. Если до какого-то файла не может достучаться кидает invalid_argument
void filesConfigurer(std::wifstream& inputFile, std::wofstream& outputFile, std::wifstream& serviceWordsFile, std::wifstream& regexesFile) {
	std::wifstream configIn("config.txt");
	std::wstring inputFileName;
	std::getline(configIn, inputFileName);
	inputFile = std::wifstream(inputFileName);
	if (!inputFile)
	{
		throw std::invalid_argument("Failed to open input file.");
	}
	setLocale(inputFile);
	std::wstring outputFileName;
	std::getline(configIn, outputFileName);
	outputFile = std::wofstream(outputFileName);
	if (!outputFile)
	{
		throw std::invalid_argument("Failed to open output file.");
	}
	std::wstring serviceWordsFileName;
	std::getline(configIn, serviceWordsFileName);
	serviceWordsFile = std::wifstream(serviceWordsFileName);
	if (!serviceWordsFile)
	{
		throw std::invalid_argument("Failed to open service words file.");
	}
	setLocale(serviceWordsFile);
	std::wstring regexesFileName;
	std::getline(configIn, regexesFileName);
	regexesFile = std::wifstream(regexesFileName);
	if (!regexesFile)
	{
		throw std::invalid_argument("Failed to open regexes file.");
	}
	setLocale(regexesFile);
}

// Удаляет комменты из текста. Этой функции нужно передать 6 char'ов:
// shortComOne, shortComTwo - символы для короткого коммента (1 строка)
// longComBeginOne, longComBeginTwo - символы для начала длинного коммента
// longComEndOne, longComEndTwo - символы для конца длинного коммента
void decomment(std::wstring& text, wchar_t shortComOne, wchar_t shortComTwo, wchar_t longComBeginOne, wchar_t longComBeginTwo, wchar_t longComEndOne, wchar_t longComEndTwo) {
	if (text.size() <= 1) {
		return;
	}

	bool readingShortComment = false, readingLongComment = false, previousCharWasComment = false;
	wchar_t previousChar, currentChar;
	std::wstring bufferString;
	for (int i = 1; i <= text.size(); ++i) {
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
	text = bufferString;
}

void regexesReader(std::wstring& regexesText, std::vector<std::wregex>& regexesVector) {
	std::wistringstream ris(regexesText);
	ris.imbue(rusLoc);
	int n = 0;
	ris >> n;
	if (n == 0) {
		throw std::invalid_argument("Failed to read amount of regexes from regexes file.");
	}
	++n;
	std::wstring enteredRegexString;
	std::getline(ris, enteredRegexString);
	std::getline(ris, enteredRegexString);
	enteredRegexString = regexServiceCode;
	regexesVector.push_back(std::wregex());
	for (int i = 1; i < n; ++i) {
		if (i > 1)
			std::getline(ris, enteredRegexString);
		try {
			std::wregex enteredRegex;
			enteredRegex.imbue(rusLoc);
			enteredRegex.assign(enteredRegexString);
			regexesVector.push_back (enteredRegex);
		}
		catch (std::exception& e) {
			throw std::runtime_error ("Failed to create regex number " + std::to_string (i));
		}
	}
}

void serviceWordsReader(std::wifstream &serviceWordsStream)
{
	int n;
	std::wstring rusWord, engWord;
	while (serviceWordsStream >> engWord)
	{
		if (!(serviceWordsStream >> rusWord))
			throw std::runtime_error("Incorrect service words format");
		serviceDict[rusWord] = engWord;
	}
	// generate 1st regex
	regexServiceCode = L"(";
	for (auto dictPair : serviceDict)
	{
		if (regexServiceCode.size() > 1)
			regexServiceCode += L'|';
		regexServiceCode += dictPair.first;
		regexServiceCode += L"|";
		regexServiceCode += dictPair.second;
	}
	regexServiceCode += L')';
}

bool initFiles(std::wofstream& outputStream, std::wstring& inputText, std::vector<std::wregex>& regexesVector)
{
	std::wifstream inputStream;
	std::wifstream serviceWordsStream;
	std::wifstream regexesStream;
	wchar_t shortComOne, shortComTwo, longComBeginOne, longComBeginTwo, longComEndOne, longComEndTwo, streamChar;
	try {
		filesConfigurer(inputStream, outputStream, serviceWordsStream, regexesStream);
	}
	catch (std::invalid_argument & e) {
		std::cerr << "Error 0-0. " << e.what();
		return false;
	}
	serviceWordsStream >> shortComOne >> shortComTwo >>
		longComBeginOne >> longComBeginTwo >>
		longComEndOne >> longComEndTwo;
	serviceWordsReader(serviceWordsStream);
	inputText.assign((std::istreambuf_iterator<wchar_t>(inputStream)), (std::istreambuf_iterator<wchar_t>()));
	decomment(inputText, shortComOne, shortComTwo, longComBeginOne, longComBeginTwo, longComEndOne, longComEndTwo);

	std::wstring regexesText = L"";
	regexesText.assign((std::istreambuf_iterator<wchar_t>(regexesStream)), (std::istreambuf_iterator<wchar_t>()));
	decomment(regexesText, shortComOne, shortComTwo, longComBeginOne, longComBeginTwo, longComEndOne, longComEndTwo);
	try {
		regexesReader(regexesText, regexesVector);
	}
	catch (std::invalid_argument & e) {
		std::cerr << "Error 0-1. " << e.what();
		return false;
	}
	catch (std::runtime_error& e) {
		std::cerr << "Error 0-2. " << e.what ();
		return false;
	}
	return true;
}

bool isCharDivider(char t)
{
	return t == ' ' || t == '\n' || t == '\t';
}

void getLexemsUsingMatch (std::wstring text, std::vector<std::wregex>& regexes, std::vector<lexem>& lexems) {
	int p = 0, end = 1;
	while (p < text.size ())
	{
		end = p;
		while (end < text.size () && isCharDivider (text[p]))
		{
			text[p] = '$';
			++p;
			++end;
		}
		if (end == text.size ())
			return;
		int type = 0;
		std::wstring name = L"";
		bool weFoundLexem = false;
		while (end < text.size ()) {
			int subType = 0;
			std::wstring sub = text.substr (p, (end - p + 1));
			for (int i = 1; i < regexes.size (); ++i)
			{
				std::wsmatch token;
				if (std::regex_match (sub, token, regexes[i]))
				{
					if (name.size () < token[0].str().size())
					{
						type = i, name = token.str ();
						weFoundLexem = true;
						subType = i;
					}
				}
			}
			if (subType == 0 && weFoundLexem) {
				break;
			}
			++end;
		}
		bool isundefined = (type == 0);
		for (int i = 0; i < name.size () || isundefined; ++i, ++p)
		{
			if (isCharDivider (text[p]))
				isundefined = 0;
			if (isundefined)
				name.push_back (text[p]);
			isundefined &= (p < text.size () - 1);
		}
		text = text.substr (p);
		p = 0;
		lexems.push_back ({ type, name });
	}
}

int main()
{
	setlocale(LC_ALL, "Russian");
	std::wcout.imbue(rusLoc);
	std::wcin.imbue(rusLoc);
	//setLocale(std::wcin);
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	std::vector<lexem> lexems;
	std::vector<std::wregex> regexes;
	std::wstring text;
	std::wofstream output;
	/*std::wofstream kpt("input.txt");
	kpt << "kek";
	kpt.close();*/
	output.imbue(rusLoc);
	if (!initFiles(output, text, regexes))
		return 0;
	std::wcout << "Lexical analyzing..." << std::endl;
	getLexemsUsingMatch(text, regexes, lexems);
	output << "Successfully created:" << std::endl;
	for (auto p : lexems)
	{
		output << "{" << p.first << ", \"" << p.second << "\"}\n";
	}
	std::wcout << "Lexical analyzing completed!" << std::endl;
	std::cout << "Generated " << lexems.size() << " lexems:\n";
	for (int i = 0; i < lexems.size(); ++i)
	{
		std::wcout << i+1 << ". {" << lexems[i].first << ", \"" << lexems[i].second << "\"}\n";
		// TODO: потом перенести это в лексический анализатор
		if (lexems[i].first == 1 && serviceDict.find(lexems[i].second) != serviceDict.end())
			lexems[i].second = serviceDict[lexems[i].second];
	}
	LexemChecker mainChecker = LexemChecker(lexems);
	mainChecker.checkProgram();
	std::cout << "Syntax: OK \n";
}