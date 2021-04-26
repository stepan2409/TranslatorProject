#pragma once
#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <exception>
#include <sstream>
#include <codecvt>
#include <locale>
#include <windows.h>
#include <map>
#include <stack>
#include "LexemChecker.h"


const int MAX_ADDING = 10;
typedef std::pair<int, std::wstring> lexem;
std::locale rusLoc = std::locale("Russian");

std::map<std::wstring, std::wstring> serviceDict;
std::wstring regexServiceCode;

std::vector<std::stack<int>> int_values;
std::vector<std::stack<bool>> bool_values;
std::vector<std::stack<float>> float_values;
std::vector<std::stack<std::wstring>> string_values;
std::vector<int> char_values;

std::vector<int *> int_values_arr;
std::vector<bool *> bool_values_arr;
std::vector<float *> float_values_arr;
std::vector<std::wstring *> string_values_arr;
std::vector<int *> char_values_arr;

std::vector<std::vector<int> > int_sizes, bool_sizes, float_sizes, string_sizes, char_sizes;

std::stack<int> empty_int_stack;
std::stack<bool> empty_bool_stack;
std::stack<float> empty_float_stack;
std::stack<std::wstring> empty_wstring_stack;

int push_value(TYPE type)
{
	if (type.basic == TYPES::INT_ && type.depth == 0) {
		int_values.push_back(empty_int_stack); return int_values.size() - 1;
	}
	if (type.basic == TYPES::CHAR_ && type.depth == 0) {
		int_values.push_back(empty_int_stack); return int_values.size() - 1;
	}
	if (type.basic == TYPES::BOOL_ && type.depth == 0) {
		bool_values.push_back(empty_bool_stack); return bool_values.size() - 1;
	}
	if (type.basic == TYPES::FLOAT_ && type.depth == 0) {
		float_values.push_back(empty_float_stack); return float_values.size() - 1;
	}
	if (type.basic == TYPES::STRING_ && type.depth == 0) {
		string_values.push_back(empty_wstring_stack); return string_values.size() - 1;
	}
	if (type.basic == TYPES::INT_ && type.depth == 1) {
		int_values_arr.push_back({}); return int_values_arr.size() - 1;
	}
	if (type.basic == TYPES::CHAR_ && type.depth == 1) {
		int_values_arr.push_back({}); return int_values_arr.size() - 1;
	}
	if (type.basic == TYPES::BOOL_ && type.depth == 1) {
		bool_values_arr.push_back({}); return bool_values_arr.size() - 1;
	}
	if (type.basic == TYPES::FLOAT_ && type.depth == 1) {
		float_values_arr.push_back({}); return float_values_arr.size() - 1;
	}
	if (type.basic == TYPES::STRING_ && type.depth == 1) {
		string_values_arr.push_back({}); return string_values_arr.size() - 1;
	}
	return -1;
}

void set_values_function(TID* tree)
{
	tree->push_value = push_value;

	int_values.push_back(empty_int_stack);
	bool_values.push_back(empty_bool_stack);
	float_values.push_back(empty_float_stack);
	string_values.push_back(empty_wstring_stack);
	int_values_arr.push_back({});
	bool_values_arr.push_back({});
	float_values_arr.push_back({});
	string_values_arr.push_back({});
	int_sizes.push_back({ 0 });
	bool_sizes.push_back({ 0 });
	float_sizes.push_back({ 0 });
	string_sizes.push_back({ 0 });
}

std::stack<lexem> execution_stack;

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
		int adding = MAX_ADDING;
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
						adding = MAX_ADDING;
					}
				}
			}
			if (adding == 0 && weFoundLexem) {
				break;
			}
			--adding;
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

std::wstring boolToLexString (bool b) {
	if (b) {
		return L"true";
	}
	else {
		return L"false";
	}
}

lexem castToBool (lexem input) {
	if (input.first == LEX_BOOL) {
		return input;
	}
	if (input.first == LEX_INT) {
		input.first = LEX_BOOL;
		bool castedBool = std::stoi (input.second);
		input.second = boolToLexString (castedBool);
		return input;
	}
	if (input.first == LEX_FLOAT) {
		input.first = LEX_BOOL;
		bool castedBool = std::stod (input.second);
		input.second = boolToLexString (castedBool);
		return input;
	}
	if (input.first == LEX_CHAR) {
		input.first = LEX_BOOL;
		bool castedBool = std::stoi (input.second);
		input.second = boolToLexString (castedBool);
		return input;
	}
	if (input.first == LEX_STRING) {
		input.first = LEX_INT;
		bool castedBool = input.second.size ()-2;
		input.second = boolToLexString (castedBool);
		return input;
	}
	throw std::invalid_argument ("I don't know how to cast " + std::to_string(input.first) + " to bool");
}

lexem castToInt (lexem input) {
	if (input.first == LEX_BOOL) {
		input.first = LEX_INT;
		if (input.second == L"true") {
			input.second = L"1";
		}
		else {
			input.second = L"0";
		}
	}
	if (input.first == LEX_INT) {
		return input;
	}
	if (input.first == LEX_FLOAT) {
		input.first = LEX_INT;
		int castedInt = std::stod (input.second);
		input.second = std::to_wstring (castedInt);
		return input;
	}
	if (input.first == LEX_CHAR) {
		input.first = LEX_INT;
		int castedInt = input.second[1];
		input.second = std::to_wstring (castedInt);
		return input;
	}
	if (input.first == LEX_STRING) {
		input.first = LEX_INT;
		std::wstring stringToCast = input.second.substr (1, input.second.size () - 2);
		int castedInt = std::stoi(stringToCast);
		input.second = std::to_wstring (castedInt);
		return input;
	}
	throw std::invalid_argument ("I don't know how to cast " + std::to_string (input.first) + " to int");
}

lexem castToFloat (lexem input) {
	if (input.first == LEX_BOOL) {
		input.first = LEX_FLOAT;
		if (input.second == L"true") {
			input.second = L"1,0";
		}
		else {
			input.second = L"0,0";
		}
	}
	if (input.first == LEX_INT) {
		input.first = LEX_FLOAT;
		double castedDouble = std::stod (input.second);
		input.second = std::to_wstring (castedDouble);
		return input;
	}
	if (input.first == LEX_FLOAT) {
		return input;
	}
	if (input.first == LEX_CHAR) {
		input.first = LEX_FLOAT;
		double castedDouble = std::stod (input.second);
		input.second = std::to_wstring (castedDouble);
		return input;
	}
	if (input.first == LEX_STRING) {
		input.first = LEX_FLOAT;
		std::wstring stringToCast = input.second.substr (1, input.second.size () - 2);
		double castedDouble = std::stod (stringToCast);
		input.second = std::to_wstring (castedDouble);
		return input;
	}
	throw std::invalid_argument ("I don't know how to cast " + std::to_string (input.first) + " to float");
}

lexem castToString (lexem input) {
	if (input.first != LEX_STRING) {
		input.first = LEX_STRING;
		input.second += L"\"";
		input.second = L"\"" + input.second;
	}
	return input; // оооо дааааа самый офигенный каст из всех )))
}

lexem castToChar (lexem input) {
	if (input.first == LEX_BOOL) {
		input.first = LEX_CHAR;
		if (input.second == L"true") {
			char castedChar = 1;
			input.second = L"'";
			input.second += castedChar;
			input.second += L"'";
		}
		else {
			char castedChar = 0;
			input.second = L"'";
			input.second += castedChar;
			input.second += L"'";
		}
	}
	if (input.first == LEX_INT) {
		input.first = LEX_CHAR;
		char castedChar = std::stoi (input.second);
		input.second = L"'";
		input.second += castedChar;
		input.second += L"'";
		return input;
	}
	if (input.first == LEX_FLOAT) {
		input.first = LEX_CHAR;
		char castedChar = int(std::stod (input.second));
		input.second = L"'";
		input.second += castedChar;
		input.second += L"'";
		return input;
	}
	if (input.first == LEX_CHAR) {
		return input;
	}
	if (input.first == LEX_STRING) {
		input.first = LEX_CHAR;
		std::wstring stringToCast = input.second.substr (1, input.second.size () - 2);
		char castedChar = stringToCast[0];
		input.second = L"'";
		input.second += castedChar;
		input.second += L"'";
		return input;
	}
	throw std::invalid_argument ("I don't know how to cast " + std::to_string (input.first) + " to char");
}

void pairCast () {
	lexem first = execution_stack.top (); execution_stack.pop ();
	lexem second = execution_stack.top (); execution_stack.pop ();
	if (first.first == LEX_STRING || second.first == LEX_STRING) {
		first = castToString (first);
		second = castToString (second);
		execution_stack.push (second);
		execution_stack.push (first);
		return;
	}
	if (first.first == LEX_FLOAT || second.first == LEX_FLOAT) {
		first = castToFloat (first);
		second = castToFloat (second);
		execution_stack.push (second);
		execution_stack.push (first);
		return;
	}
	if (first.first == LEX_INT || second.first == LEX_INT) {
		first = castToInt (first);
		second = castToInt (second);
		execution_stack.push (second);
		execution_stack.push (first);
		return;
	}
	if (first.first == LEX_CHAR || second.first == LEX_CHAR) {
		first = castToChar (first);
		second = castToChar (second);
		execution_stack.push (second);
		execution_stack.push (first);
		return;
	}
	execution_stack.push (second);
	execution_stack.push (first);
	return;
}

int main()
{
	empty_int_stack.push (0);
	empty_bool_stack.push (false);
	empty_float_stack.push (0);
	empty_wstring_stack.push (L"");
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
	set_values_function(mainChecker.get_tree());
	mainChecker.checkProgram();
	std::vector<lexem> &polis = mainChecker.get_polis();
	std::cout << "Syntax: OK \n";
	std::cout << "Generated " << polis.size() << " polis commands: \n";
	for (int i = 0; i < polis.size(); ++i)
	{
		std::wstring typis = L"";
		int type = polis[i].first;
		if (type & LEX_ADRESS)
			typis = L"LEX_ADRESS | ", type -= LEX_ADRESS;
		if (type & LEX_ARRAY)
			typis = L"LEX_ARRAY | ", type -= LEX_ARRAY;
		if (type == LEX_INT)
			typis += L"INT";
		if (type == LEX_BOOL)
			typis += L"BOOL";
		if (type == LEX_FLOAT)
			typis += L"FLOAT";
		if (type == LEX_STRING)
			typis += L"STRING";
		if (type == LEX_CHAR)
			typis += L"CHAR";
		if (type == LEX_OPERATION)
			typis += L"OPERATION";
		std::wcout << i << ". {" << typis << ", \"" << polis[i].second << "\"}\n";
	}
	try {
		int pos = 0; // текущая позиция
		while (pos < polis.size ()) {
			// если встретили адрес переменную или константу
			if (polis[pos].first != LEX_OPERATION) {
				// пихаем её в с тек
				execution_stack.push (polis[pos]);
				// и спокойно шуруем дальше
				++pos; continue;
			}
			// иначе мы словили оператор и надо его обработать
			// если мы словили !! то 
			if (polis[pos].second == L"!!") {
				// меняем pos на последнее значение в стеке
				pos = std::stoi (execution_stack.top ().second);
				// удаляем лишнее из стека
				execution_stack.pop ();
				// и идем дальше
				continue;
			}
			// если мы словили ?! то
			if (polis[pos].second == L"?!") {
				// берем цифирь из стека
				int posToJump = std::stoi (execution_stack.top ().second); execution_stack.pop ();
				std::wstring boolToAnalyze;
				// если у нас наверху не адрес
				if (!(execution_stack.top ().first & LEX_ADRESS)) {
					// конвертируем верхушку стека в bool
					lexem boolLexem = castToBool (execution_stack.top ());
					execution_stack.pop ();
					boolToAnalyze = boolLexem.second;
				}
				else {
					lexem boolLexem = execution_stack.top (); execution_stack.pop ();
					boolLexem.first -= LEX_ADRESS;
					if (boolLexem.first == LEX_INT) {
						boolLexem.second = std::to_wstring(int_values[std::stoi (boolLexem.second)].top ());
					} else if (boolLexem.first == LEX_BOOL) {
						boolLexem.second = boolToLexString (bool_values[std::stoi (boolLexem.second)].top ());
					}
					else if (boolLexem.first == LEX_FLOAT) {
						boolLexem.second = std::to_wstring (float_values[std::stoi (boolLexem.second)].top ());
					}
					else if (boolLexem.first == LEX_STRING) {
						boolLexem.second = string_values[std::stoi (boolLexem.second)].top ();
					}
					else if (boolLexem.first == LEX_CHAR) {
						boolLexem.second = L"'" + char(int_values[std::stoi (boolLexem.second)].top ());
						boolLexem.second += L"'";
					}
					boolLexem = castToBool (boolLexem);
				}
				// если булл = false то идем меняем pos на цифирь
				if (boolToAnalyze == L"false") {
					pos = posToJump;
					continue;
				}
				else {
					++pos; continue;
				}
			}
			// если мы словили out то выводим последнюю хрень из стека
			if (polis[pos].second == L"out") {
				lexem lexemForOutput = execution_stack.top (); execution_stack.pop ();
				std::wstring stringForOutput = lexemForOutput.second;
				if (lexemForOutput.first == LEX_STRING || lexemForOutput.first == LEX_CHAR) {
					stringForOutput = stringForOutput.substr (1, stringForOutput.size () - 2);
				}
				std::wcout << stringForOutput << std::endl;
				++pos;
				continue;
			}
			// если мы словили in то надо считать и запихать в стек строку
			if (polis[pos].second == L"in") {
				std::wstring enteredString;
				std::wcin >> enteredString;
				lexem newLexem;
				newLexem.first = LEX_STRING;
				newLexem.second = enteredString;
				execution_stack.push (newLexem);
				++pos; continue;
			}
			// если мы словили = то присваиваем
			if (polis[pos].second == L"=") {
				// тут должен быть код для присваивания
			}
			// если мы словили cast то берем во что кастить, что кастить, кастим и пихаем в стек
			if (polis[pos].second == L"cast") {
				lexem howToCast = execution_stack.top (); execution_stack.pop ();
				lexem whatToCast = execution_stack.top (); execution_stack.pop ();
				if (howToCast.second == L"1") {
					whatToCast = castToInt (whatToCast);
				} else if (howToCast.second == L"2") {
					whatToCast = castToBool (whatToCast);
				} else if (howToCast.second == L"3") {
					whatToCast = castToFloat (whatToCast);
				}
				else if (howToCast.second == L"4") {
					whatToCast = castToString (whatToCast);
				}
				else if (howToCast.second == L"5") {
					whatToCast = castToChar (whatToCast);
				}
				execution_stack.push (whatToCast);
				++pos; continue;
			}
			if (polis[pos].second == L"+") {
				pairCast();
				lexem first = execution_stack.top (); execution_stack.pop ();
				lexem second = execution_stack.top (); execution_stack.pop ();
				if (first.first == LEX_STRING) {
					std::wstring firstString = first.second.substr(1, first.second.size()-2);
					std::wstring secondString = second.second.substr(1, second.second.size()-2);
					std::wstring resultString = L"\"" + secondString + firstString + L"\"";
					lexem resultLexem;
					resultLexem.first = LEX_STRING;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_FLOAT) {
					double firstDouble = std::stod(first.second);
					double secondDouble = std::stod(second.second);
					std::wstring resultString = std::to_wstring(secondDouble + firstDouble);
					lexem resultLexem;
					resultLexem.first = LEX_FLOAT;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_INT) {
					int firstInt = std::stoi (first.second);
					int secondInt = std::stoi (second.second);
					std::wstring resultString = std::to_wstring (secondInt + firstInt);
					lexem resultLexem;
					resultLexem.first = LEX_INT;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_CHAR) {
					char firstChar = first.second[1];
					char secondChar = second.second[1];
					std::wstring resultString = L"'";
					resultString += char(firstChar+secondChar);
					resultString += L"'";
					lexem resultLexem;
					resultLexem.first = LEX_CHAR;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_BOOL) {
					bool firstBool = first.second[0] == 't';
					bool secondBool = second.second[0] == 't';
					std::wstring resultString = boolToLexString(firstBool + secondBool);
					lexem resultLexem;
					resultLexem.first = LEX_BOOL;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
			}
			if (polis[pos].second == L"-") {
				pairCast ();
				lexem first = execution_stack.top (); execution_stack.pop ();
				lexem second = execution_stack.top (); execution_stack.pop ();
				/*if (first.first == LEX_STRING) {
					std::wstring firstString = first.second.substr (1, first.second.size () - 2);
					std::wstring secondString = second.second.substr (1, second.second.size () - 2);
					std::wstring resultString = L"\"" + secondString + firstString + L"\"";
					lexem resultLexem;
					resultLexem.first = LEX_STRING;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}*/
				if (first.first == LEX_FLOAT) {
					double firstDouble = std::stod (first.second);
					double secondDouble = std::stod (second.second);
					std::wstring resultString = std::to_wstring (secondDouble - firstDouble);
					lexem resultLexem;
					resultLexem.first = LEX_FLOAT;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_INT) {
					int firstInt = std::stoi (first.second);
					int secondInt = std::stoi (second.second);
					std::wstring resultString = std::to_wstring (secondInt - firstInt);
					lexem resultLexem;
					resultLexem.first = LEX_INT;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_CHAR) {
					char firstChar = first.second[1];
					char secondChar = second.second[1];
					std::wstring resultString = L"'";
					resultString += char (secondChar - firstChar);
					resultString += L"'";
					lexem resultLexem;
					resultLexem.first = LEX_CHAR;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_BOOL) {
					bool firstBool = first.second[0] == 't';
					bool secondBool = second.second[0] == 't';
					std::wstring resultString = boolToLexString (secondBool - firstBool);
					lexem resultLexem;
					resultLexem.first = LEX_BOOL;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
			}
			if (polis[pos].second == L"*") {
				pairCast ();
				lexem first = execution_stack.top (); execution_stack.pop ();
				lexem second = execution_stack.top (); execution_stack.pop ();
				/*if (first.first == LEX_STRING) {
					std::wstring firstString = first.second.substr (1, first.second.size () - 2);
					std::wstring secondString = second.second.substr (1, second.second.size () - 2);
					std::wstring resultString = L"\"" + secondString + firstString + L"\"";
					lexem resultLexem;
					resultLexem.first = LEX_STRING;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}*/
				if (first.first == LEX_FLOAT) {
					double firstDouble = std::stod (first.second);
					double secondDouble = std::stod (second.second);
					std::wstring resultString = std::to_wstring (secondDouble * firstDouble);
					lexem resultLexem;
					resultLexem.first = LEX_FLOAT;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_INT) {
					int firstInt = std::stoi (first.second);
					int secondInt = std::stoi (second.second);
					std::wstring resultString = std::to_wstring (secondInt * firstInt);
					lexem resultLexem;
					resultLexem.first = LEX_INT;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_CHAR) {
					char firstChar = first.second[1];
					char secondChar = second.second[1];
					std::wstring resultString = L"'";
					resultString += char (secondChar * firstChar);
					resultString += L"'";
					lexem resultLexem;
					resultLexem.first = LEX_CHAR;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_BOOL) {
					bool firstBool = first.second[0] == 't';
					bool secondBool = second.second[0] == 't';
					std::wstring resultString = boolToLexString (secondBool * firstBool);
					lexem resultLexem;
					resultLexem.first = LEX_BOOL;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
			}
			if (polis[pos].second == L"/") {
				pairCast ();
				lexem first = execution_stack.top (); execution_stack.pop ();
				lexem second = execution_stack.top (); execution_stack.pop ();
				/*if (first.first == LEX_STRING) {
					std::wstring firstString = first.second.substr (1, first.second.size () - 2);
					std::wstring secondString = second.second.substr (1, second.second.size () - 2);
					std::wstring resultString = L"\"" + secondString + firstString + L"\"";
					lexem resultLexem;
					resultLexem.first = LEX_STRING;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}*/
				if (first.first == LEX_FLOAT) {
					double firstDouble = std::stod (first.second);
					if (firstDouble == 0) {
						throw std::exception ("Attempt of deviding by zero");
					}
					double secondDouble = std::stod (second.second);
					std::wstring resultString = std::to_wstring (secondDouble / firstDouble);
					lexem resultLexem;
					resultLexem.first = LEX_FLOAT;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_INT) {
					int firstInt = std::stoi (first.second);
					if (firstInt == 0) {
						throw std::exception ("Attempt of deviding by zero");
					}
					int secondInt = std::stoi (second.second);
					std::wstring resultString = std::to_wstring (secondInt / firstInt);
					lexem resultLexem;
					resultLexem.first = LEX_INT;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_CHAR) {
					char firstChar = first.second[1];
					if (firstChar == 0) {
						throw std::exception ("Attempt of deviding by zero");
					}
					char secondChar = second.second[1];
					std::wstring resultString = L"'";
					resultString += char (secondChar / firstChar);
					resultString += L"'";
					lexem resultLexem;
					resultLexem.first = LEX_CHAR;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_BOOL) {
					bool firstBool = first.second[0] == 't';
					if (firstBool == 0) {
						throw std::exception ("Attempt of deviding by zero");
					}
					bool secondBool = second.second[0] == 't';
					std::wstring resultString = boolToLexString (secondBool / firstBool);
					lexem resultLexem;
					resultLexem.first = LEX_BOOL;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
			}
			if (polis[pos].second == L"%") {
				pairCast ();
				lexem first = execution_stack.top (); execution_stack.pop ();
				lexem second = execution_stack.top (); execution_stack.pop ();
				/*if (first.first == LEX_STRING) {
					std::wstring firstString = first.second.substr (1, first.second.size () - 2);
					std::wstring secondString = second.second.substr (1, second.second.size () - 2);
					std::wstring resultString = L"\"" + secondString + firstString + L"\"";
					lexem resultLexem;
					resultLexem.first = LEX_STRING;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}*/
				if (first.first == LEX_FLOAT) {
					throw std::exception("Cannot use % with floats");
					double firstDouble = std::stod (first.second);
					double secondDouble = std::stod (second.second);
					std::wstring resultString = std::to_wstring (0);
					lexem resultLexem;
					resultLexem.first = LEX_FLOAT;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_INT) {
					int firstInt = std::stoi (first.second);
					if (firstInt == 0) {
						throw std::exception ("Attempt of deviding by zero");
					}
					int secondInt = std::stoi (second.second);
					std::wstring resultString = std::to_wstring (secondInt % firstInt);
					lexem resultLexem;
					resultLexem.first = LEX_INT;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_CHAR) {
					char firstChar = first.second[1];
					if (firstChar == 0) {
						throw std::exception ("Attempt of deviding by zero");
					}
					char secondChar = second.second[1];
					std::wstring resultString = L"'";
					resultString += char (secondChar % firstChar);
					resultString += L"'";
					lexem resultLexem;
					resultLexem.first = LEX_CHAR;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
				if (first.first == LEX_BOOL) {
					bool firstBool = first.second[0] == 't';
					if (!firstBool) {
						throw std::exception ("Attempt of deviding by zero");
					}
					bool secondBool = second.second[0] == 't';
					std::wstring resultString = boolToLexString (secondBool % firstBool);
					lexem resultLexem;
					resultLexem.first = LEX_BOOL;
					resultLexem.second = resultString;
					execution_stack.push (resultLexem);
					++pos; continue;
				}
			}
			//TODO вот эта херь нужна только для отладки чтоб прога не висла на ещё не дописанных функциях, потом её надо убрать
			pos++;
		}
		std::cout << std::endl << "Finished without any mistakes! :)";
	}
	catch (std::exception& e) {
		std::cout << e.what () << std::endl;
	}
}