#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <vector>
#include <exception>
#include <sstream>

// Конфигуратор файлов. Берет из config.txt адреса файлов и присваивает их потокам. Если до какого-то файла не может достучаться кидает invalid_argument
void filesConfigurer(std::ifstream& inputFile, std::ofstream& outputFile, std::ifstream& serviceWordsFile, std::ifstream& regexesFile) {
	std::ifstream configIn("config.txt");
	std::string inputFileName;
	std::getline(configIn, inputFileName);
	FILE* fileChecker = std::fopen(inputFileName.c_str(), "r");
	if (!fileChecker) {
		throw std::invalid_argument("Failed to open input file.");
	}
	std::fclose(fileChecker);
	inputFile = std::ifstream(inputFileName);

	std::string outputFileName;
	std::getline(configIn, outputFileName);
	fileChecker = std::fopen(outputFileName.c_str(), "w");
	if (!fileChecker) {
		throw std::invalid_argument("Failed to open output file.");
	}
	std::fclose(fileChecker);
	outputFile = std::ofstream(outputFileName);

	std::string serviceWordsFileName;
	std::getline(configIn, serviceWordsFileName);
	fileChecker = std::fopen(serviceWordsFileName.c_str(), "r");
	if (!fileChecker) {
		throw std::invalid_argument("Failed to open service words file.");
	}
	std::fclose(fileChecker);
	serviceWordsFile = std::ifstream(serviceWordsFileName);

	std::string regexesFileName;
	std::getline(configIn, regexesFileName);
	fileChecker = std::fopen(regexesFileName.c_str(), "r");
	if (!fileChecker) {
		throw std::invalid_argument("Failed to open regexes file.");
	}
	std::fclose(fileChecker);
	regexesFile = std::ifstream(regexesFileName);
}

// Удаляет комменты из текста. Этой функции нужно передать 6 char'ов:
// shortComOne, shortComTwo - символы для короткого коммента (1 строка)
// longComBeginOne, longComBeginTwo - символы для начала длинного коммента
// longComEndOne, longComEndTwo - символы для конца длинного коммента
void decomment(std::string& text, char shortComOne, char shortComTwo, char longComBeginOne, char longComBeginTwo, char longComEndOne, char longComEndTwo) {
	if (text.size() <= 1) {
		return;
	}

	bool readingShortComment = false, readingLongComment = false, previousCharWasComment = false;
	char previousChar, currentChar;
	std::string bufferString;
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

void regexesReader(std::string& regexesText, std::vector<std::regex>& regexesVector) {
	std::istringstream ris(regexesText);
	int n = 0;
	ris >> n;
	if (n == 0) {
		throw std::invalid_argument("Failed to read amount of regexes from regexes file.");
	}
	++n;
	for (int i = 0; i < n; ++i) {
		std::string enteredRegexString;
		std::getline(ris, enteredRegexString);
		std::regex enteredRegex(enteredRegexString);
		regexesVector.push_back(enteredRegex);
	}
}

bool initFiles(std::ofstream& outputStream, std::string& inputText, std::vector<std::regex>& regexesVector)
{
	std::ifstream inputStream;
	std::ifstream serviceWordsStream;
	std::ifstream regexesStream;
	char shortComOne, shortComTwo, longComBeginOne, longComBeginTwo, longComEndOne, longComEndTwo;
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
	inputText.assign((std::istreambuf_iterator<char>(inputStream)), (std::istreambuf_iterator<char>()));
	decomment(inputText, shortComOne, shortComTwo, longComBeginOne, longComBeginTwo, longComEndOne, longComEndTwo);

	std::string regexesText;
	regexesText.assign((std::istreambuf_iterator<char>(regexesStream)), (std::istreambuf_iterator<char>()));
	decomment(regexesText, shortComOne, shortComTwo, longComBeginOne, longComBeginTwo, longComEndOne, longComEndTwo);
	try {
		regexesReader(regexesText, regexesVector);
	}
	catch (std::invalid_argument & e) {
		std::cerr << "Error 0-1. " << e.what();
		return false;
	}
	return true;
}

bool isCharDivider(char t)
{
	return t == ' ' || t == '\n' || t == '\t';
}

// Заполняет вектор lexems лексемами за O(n^2) в лучшем, и за e^n в худшем.
void getLexems(std::string text, std::vector<std::regex>& regexes, std::vector<std::pair<int, std::string> >& lexems)
{
	int p = 0;
	while (p < text.size())
	{
		while (p < text.size() && isCharDivider(text[p]))
		{
			text[p] = '$';
			++p;
		}
		if (p == text.size())
			return;
		int type = 0;
		std::string name = "";
		for (int i = 1; i < regexes.size(); ++i)
		{
			std::smatch token;
			if (std::regex_search(text, token, regexes[i]))
			{
				if (token.prefix().length() == p && name.size() < token.size())
				{
					type = i, name = token.str();
				}
			}
		}
		bool isundefined = (type == 0);
		for (int i = 0; i < name.size() || isundefined; ++i, ++p)
		{
			if (isCharDivider(text[p]))
				isundefined = 0;
			if (isundefined)
				name.push_back(text[p]);
			isundefined &= (p < text.size() - 1);
		}
		text = text.substr(p);
		p = 0;
		lexems.push_back({ type, name });
	}
}

// Делает тоже самое, что и getLexems, только с использованием regex_match вместо regex_search
void getLexemsUsingMatch (std::string text, std::vector<std::regex>& regexes, std::vector<std::pair<int, std::string> >& lexems) {
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
		std::string name = "";
		bool weFoundLexem = false;
		while (end < text.size ()) {
			int subType = 0;
			for (int i = 1; i < regexes.size (); ++i)
			{
				std::smatch token;
				std::string sub = text.substr (p, (end - p + 1));
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
	std::vector<std::pair<int, std::string> > lexems;
	std::vector<std::regex> regexes;
	std::string text;
	std::ofstream output;
	if (!initFiles(output, text, regexes))
		return 0;
	getLexemsUsingMatch(text, regexes, lexems);
	output << "Successfully created:\n";
	for (auto p : lexems)
	{
		output << "{" << p.first << ", \"" << p.second << "\"}\n";
	}
}
