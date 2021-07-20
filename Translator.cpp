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
#include "CodeRunner.h"

const int MAX_SIZE = 1 << 22;
const int MAX_STACK_SIZE = 1 << 16;
const int STRING_REGEX_NUMBER = 5;

const int MAX_ADDING = 10;
typedef std::pair<int, std::wstring> lexem;

std::map<std::wstring, std::wstring> serviceDict;
std::wstring regexServiceCode;

std::vector<std::stack<int>> int_values;
std::vector<std::stack<bool>> bool_values;
std::vector<std::stack<float>> float_values;
std::vector<std::stack<std::wstring>> string_values;

std::vector<std::stack<int *>> int_values_arr;
std::vector<std::stack<bool *>> bool_values_arr;
std::vector<std::stack<float *>> float_values_arr;
std::vector<std::stack<std::wstring *>> string_values_arr;

std::vector<std::stack<std::vector<int>> > int_sizes, bool_sizes, float_sizes, string_sizes;
std::vector<std::vector<std::vector<int> > > func_memory;

std::stack<int> empty_int_stack;
std::stack<bool> empty_bool_stack;
std::stack<float> empty_float_stack;
std::stack<std::wstring> empty_wstring_stack;
std::stack<std::vector<int>> empty_size_stack;
std::stack<int> function_stack;
std::stack<int> funcpos_stack;

std::vector<int> lexem_lines;
int lexem_codeline = 0;

CodeRunner cr;

void runException(int code, std::wstring eng, std::wstring rus)
{
	if (code == -2)
	{
		std::wcout << L"Косяк исполнения: " << rus;
		std::wcout << L". (Runtime error: " + eng + L")\n";
	}
	else 	if (code == -2)
	{
		std::wcout << L"Косяк сборки: " << rus;
		std::wcout << L". (Compole error: " + eng + L")\n";
	}
	else
	{
		int line = lexem_lines[min(lexem_lines.size()-1, code)] + 1;
		std::wcout << L"Синтаксический косяк: " << rus;
		std::wcout << L". Строка " << line << L" (Syntax error: " << eng << L". Line " << line << L")\n";
	}
	exit(0);
}

int push_value(TYPE type)
{
	int res = -1;
	if (type.basic == TYPES::INT_ && type.depth == 0) {
		int_values.push_back(empty_int_stack); res = int_values.size() - 1;
	}
	else if (type.basic == TYPES::CHAR_ && type.depth == 0) {
		int_values.push_back(empty_int_stack); res = int_values.size() - 1;
	}
	else if (type.basic == TYPES::BOOL_ && type.depth == 0) {
		bool_values.push_back(empty_bool_stack); res = bool_values.size() - 1;
	}
	else if (type.basic == TYPES::FLOAT_ && type.depth == 0) {
		float_values.push_back(empty_float_stack); res = float_values.size() - 1;
	}
	else if (type.basic == TYPES::STRING_ && type.depth == 0) {
		string_values.push_back(empty_wstring_stack); res = string_values.size() - 1;
	}
	if (res != -1)
	{
		if (function_stack.size())
			func_memory[function_stack.top()][type.basic-1].push_back(res);
		return res;
	}
	if (type.basic == TYPES::INT_ && type.depth > 0) {
		std::stack<int*> st;
		res = int_values_arr.size();
		int_values_arr.push_back(st);
		int_sizes.push_back({});
	}
	else if (type.basic == TYPES::CHAR_ && type.depth > 0) {
		std::stack<int*> st;
		res = int_values_arr.size();
		int_values_arr.push_back(st);
		int_sizes.push_back({});
	}
	else if (type.basic == TYPES::BOOL_ && type.depth > 0) {
		std::stack<bool*> st;
		res = bool_values_arr.size();
		bool_values_arr.push_back(st);
		bool_sizes.push_back({});
	}
	else if (type.basic == TYPES::FLOAT_ && type.depth > 0) {
		std::stack<float*> st;
		res = float_values_arr.size();
		float_values_arr.push_back(st);
		float_sizes.push_back({});
	}
	else if (type.basic == TYPES::STRING_ && type.depth > 0) {
		std::stack<std::wstring*> st;
		res = string_values_arr.size();
		string_values_arr.push_back(st);
		string_sizes.push_back({});
	}
	if (type != VOID_TYPE && function_stack.size())
		func_memory[function_stack.top()][4 + type.basic].push_back(res);
	return res;
}

int push_func_memory()
{
	function_stack.push(func_memory.size());
	std::vector<std::vector<int> > nv(10);
	func_memory.push_back(nv);
	return function_stack.top();
}

int pop_func_memory()
{
	int res = function_stack.top();
	function_stack.pop();
	return res;
}

void set_values_function(TID* tree)
{
	tree->push_value = push_value;

	/*empty_int_stack.push(0);
	empty_bool_stack.push(false);
	empty_float_stack.push(0);
	empty_wstring_stack.push(L"");
	empty_size_stack.push({ 0 });

	int_values.push_back(empty_int_stack);
	bool_values.push_back(empty_bool_stack);
	float_values.push_back(empty_float_stack);
	string_values.push_back(empty_wstring_stack);
	int_values_arr.push_back(std::stack<int*>({ nullptr }));
	bool_values_arr.push_back(std::stack<bool*>({ nullptr }));
	float_values_arr.push_back(std::stack<float*>({ nullptr }));
	string_values_arr.push_back(std::stack<std::wstring*>({ nullptr }));
	int_sizes.push_back(empty_size_stack);
	bool_sizes.push_back(empty_size_stack);
	float_sizes.push_back(empty_size_stack);
	string_sizes.push_back(empty_size_stack);*/

	cr.int_values = &int_values;
	cr.bool_values = &bool_values;
	cr.float_values = &float_values;
	cr.string_values = &string_values;
	cr.int_values_arr = &int_values_arr;
	cr.bool_values_arr = &bool_values_arr;
	cr.float_values_arr = &float_values_arr;
	cr.string_values_arr = &string_values_arr;
	cr.int_sizes = &int_sizes;
	cr.bool_sizes = &bool_sizes;
	cr.float_sizes = &float_sizes;
	cr.string_sizes = &string_sizes;
}

void setLocale(std::wistream &str)
{
	str.imbue(std::locale(".utf8"));
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
	int n = 0;
	ris >> n;
	if (n == 0) {
		runException(-1, 
			L"Failed to read amount of regexes from regexes file", 
			L"Не смог прочесть количество шаблонов из файла"
		);
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
			enteredRegex.assign(enteredRegexString);
			regexesVector.push_back (enteredRegex);
		}
		catch (std::exception& e) {
			runException(-1, 
				L"Failed to create regex number " + std::to_wstring(i), 
				L"Не прочёл шаблон " + std::to_wstring(i)
			);
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
	if (t == '\n') ++lexem_codeline;
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
		if (type == STRING_REGEX_NUMBER) {
			for (int i = 0; i + 1 < name.length (); ++i) {
				if (name[i] == '\\') {
					if (name[i + 1] == 'n') {
						name.erase (i, 2);
						name.insert (i, L"\n");
					}
					else if (name[i + 1] == 't') {
						name.erase (i, 2);
						name.insert (i, L"\t");
					}
				}
			}
		}
		lexem_lines.push_back(lexem_codeline);
		lexems.push_back ({ type, name });
	}
}

std::string runCode(std::vector<lexem> & polis)
{
	cr.runException = runException;
	int pos = 0; // текущая позиция
	while (pos < polis.size()) {
		//std::wcout << "p " << pos << ". { " << polis[pos].first << ", " << polis[pos].second << " }\n";
		// если встретили адрес переменную или константу
		if (polis[pos].first != LEX_OPERATION) {
			cr.pushValue(polis[pos]);
		}
		else if (polis[pos].second == L"in")
		{
			std::wstring input;
			std::wcin >> input;
			cr.pushValue(input);
		}
		else if (polis[pos].second == L"out")
		{
			std::wcout << cr.getString();
		}
		else if (polis[pos].second == L"!!")
		{
			pos = cr.getInt() - 1;
		}
		else if (polis[pos].second == L"?!")
		{
			int new_pos = cr.getInt();
			if (!cr.getBool())
				pos = new_pos - 1;
		}
		else if (polis[pos].second == L"->")
		{
			int index = cr.getInt();
			value arr = cr.getPointer();
			int depth = arr.first.depth;
			if (arr.first == LEX_STRING)
			{
				std::wstring str = std::any_cast<std::wstring>(arr.second);
				if (index >= str.size())
					runException(-2,
						L"Array's subscript out of range",
						L"Классику ошибок словил ты"
					);
				cr.pushValue(str[index]);
			}
			else
			{
				int size, part = cr.getSize(arr, size);
				if (size > 0)
					part /= size;
				if (index >= size)
					runException(-2,
						L"Array's subscript out of range",
						L"Классику ошибок словил ты"
					);
				--arr.first.depth;
				std::any item;
				if (arr.first.basic == LEX_INT) {
					auto arr_t = std::any_cast<std::pair<int, int*>>(arr.second);
					arr_t.second += index * part;
					if (arr.first.depth > 0)
						item = std::any(arr_t);
					else item = std::any(arr_t.second);
				}
				else if (arr.first.basic == LEX_BOOL) {
					auto arr_t = std::any_cast<std::pair<int, bool*>>(arr.second);
					arr_t.second += index * part;
					if (arr.first.depth > 0)
						item = std::any(arr_t);
					else item = std::any(arr_t.second);
				}
				else if (arr.first.basic == LEX_FLOAT) {
					auto arr_t = std::any_cast<std::pair<int, float*>>(arr.second);
					arr_t.second += index * part;
					if (arr.first.depth > 0)
						item = std::any(arr_t);
					else item = std::any(arr_t.second);
				}
				else if (arr.first.basic == LEX_STRING) {
					auto arr_t = std::any_cast<std::pair<int, std::wstring*>>(arr.second);
					arr_t.second += index * part;
					if (arr.first.depth > 0)
						item = std::any(arr_t);
					else item = std::any(arr_t.second);
				}
				else if (arr.first.basic == LEX_CHAR) {
					auto arr_t = std::any_cast<std::pair<int, int*>>(arr.second);
					arr_t.second += index * part;
					if (arr.first.depth > 0)
						item = std::any(arr_t);
					else item = std::any(arr_t.second);
				}
				arr.second = item;
				cr.pushValue(arr);
			}
		}
		else if (polis[pos].second == L"push-stack")
		{
			funcpos_stack.push(pos + 2);
			if (funcpos_stack.size() > MAX_STACK_SIZE)
			{
				runException(-2,
					L"Stack is overflow",
					L"Вас победила рекурсия"
				);
			}
		}
		else if (polis[pos].second == L"pop-stack")
		{
			cr.pushValue(funcpos_stack.top());
			funcpos_stack.pop();
		}
		else if (polis[pos].second == L"push-mem")
		{
			int index = cr.getInt();
			for (int p : func_memory[index][0])
				int_values[p].push(0);
			for (int p : func_memory[index][1])
				bool_values[p].push(0);
			for (int p : func_memory[index][2])
				float_values[p].push(0);
			for (int p : func_memory[index][3])
				string_values[p].push(L"");
			for (int p : func_memory[index][4])
				int_values[p].push(0);
			for (int p : func_memory[index][5]) {
				int_values_arr[p].push(nullptr);
				int_sizes[p].push({});
			}
			for (int p : func_memory[index][6]) {
				bool_values_arr[p].push(nullptr);
				bool_sizes[p].push({});
			}
			for (int p : func_memory[index][7]) {
				float_values_arr[p].push(nullptr);
				float_sizes[p].push({});
			}
			for (int p : func_memory[index][8]) {
				string_values_arr[p].push(nullptr);
				string_sizes[p].push({});
			}
			for (int p : func_memory[index][9]) {
				int_values_arr[p].push(nullptr);
				int_sizes[p].push({});
			}
		}
		else if (polis[pos].second == L"pop-mem")
		{
			int index = cr.getInt();
			for (int p : func_memory[index][0])
				int_values[p].pop();
			for (int p : func_memory[index][1])
				bool_values[p].pop();
			for (int p : func_memory[index][2])
				float_values[p].pop();
			for (int p : func_memory[index][3])
				string_values[p].pop();
			for (int p : func_memory[index][4])
				int_values[p].pop();
			for (int p : func_memory[index][5]) {
				if (int_values_arr[p].top())
					delete[] int_values_arr[p].top();
				int_values_arr[p].pop();
				int_sizes[p].pop();
			}
			for (int p : func_memory[index][6]) {
				if(bool_values_arr[p].top())
					delete[] bool_values_arr[p].top();
				bool_values_arr[p].pop();
				bool_sizes[p].pop();
			}
			for (int p : func_memory[index][7]) {
				if (float_values_arr[p].top())
					delete[] float_values_arr[p].top();
				float_values_arr[p].pop();
				float_sizes[p].pop();
			}
			for (int p : func_memory[index][8]) {
				if (string_values_arr[p].top())
					delete[] string_values_arr[p].top();
				string_values_arr[p].pop();
				string_sizes[p].pop();
			}
			for (int p : func_memory[index][9]) {
				if (int_values_arr[p].top())
				{
					int* todel = int_values_arr[p].top();
					delete[] todel;
				}
				int_values_arr[p].pop();
				int_sizes[p].pop();
			}
		}
		else if (polis[pos].second == L"cast")
		{
			int typ = cr.getInt();
			if (typ == LEX_INT)
				cr.pushValue(cr.getInt());
			else if (typ == LEX_BOOL)
				cr.pushValue(cr.getBool());
			else if (typ == LEX_FLOAT)
				cr.pushValue(cr.getFloat());
			else if (typ == LEX_STRING)
				cr.pushValue(cr.getString());
			else if (typ == LEX_CHAR)
				cr.pushValue(cr.getChar());
		}
		else if (polis[pos].second == L"set-arrays")
		{
			int typ = cr.getInt();
			int idc = cr.getInt();
			std::wstringstream ids(cr.getString());

			int depth = cr.getInt();
			long long size = 1;
			std::vector<int> sizes(depth), paths(idc);
			for (int i = 0; i < idc; ++i) {
				ids >> paths[i];
			}
			for (int i = 0; i < depth; ++i)
			{
				sizes[i] = cr.getInt();
				size *= sizes[i];
				if (size > MAX_SIZE || size < 0)
					runException(-2,
						L"Can't create an array with size '" + std::to_wstring(size) + L"'",
						L"Массив размера " + std::to_wstring(size) + L"' тупо не создать"
					);
			}
			for (int i = 0; i < idc; ++i)
			{
				if (typ == LEX_INT) {
					int_sizes[paths[i]].top().swap(sizes);
					int_values_arr[paths[i]].top() = new int[size];
				}
				else if (typ == LEX_BOOL) {
					bool_sizes[paths[i]].top().swap(sizes);
					bool_values_arr[paths[i]].top() = new bool[size];
				}
				else if (typ == LEX_FLOAT) {
					float_sizes[paths[i]].top().swap(sizes);
					float_values_arr[paths[i]].top() = new float[size];
				}
				else if (typ == LEX_STRING) {
					string_sizes[paths[i]].top().swap(sizes);
					string_values_arr[paths[i]].top() = new std::wstring[size];
				}
				else if (typ == LEX_CHAR) {
					int_sizes[paths[i]].top().swap(sizes);
					int_values_arr[paths[i]].top() = new int[size];
				}
			}
		}
		else if (polis[pos].second == L"forget")
		{
			cr.getValue();
		}
		else cr.runOperation(polis[pos].second);
		++pos;
	}
	return "\nFinished without mistakes! :)";
}

int main()
{
	std::locale::global(std::locale(".utf8"));
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	std::vector<lexem> lexems;
	std::vector<std::wregex> regexes;
	std::wstring text;
	std::wofstream output;
	if (!initFiles(output, text, regexes))
		return 0;
	output << "Lexical analyzing..." << std::endl;
	getLexemsUsingMatch(text, regexes, lexems);
	output << "Successfully created:" << std::endl;
	for (auto p : lexems)
	{
		output << "{" << p.first << ", \"" << p.second << "\"}\n";
	}
	output << "Lexical analyzing completed!" << std::endl;
	output << "Generated " << lexems.size() << " lexems:\n";
	for (int i = 0; i < lexems.size(); ++i)
	{
		output << i+1 << ". {" << lexems[i].first << ", \"" << lexems[i].second << "\"}\n";
		// TODO: потом перенести это в лексический анализатор
		if (lexems[i].first == 1 && serviceDict.find(lexems[i].second) != serviceDict.end())
			lexems[i].second = serviceDict[lexems[i].second];
	}
	LexemChecker mainChecker = LexemChecker(lexems);
	set_values_function(mainChecker.get_tree());
	mainChecker.push_func_memory = push_func_memory;
	mainChecker.pop_func_memory = pop_func_memory;
	mainChecker.runException = runException;
	mainChecker.checkProgram();
	std::vector<lexem> &polis = mainChecker.get_polis();
	output << "Syntax: OK \n";
	output << "Generated " << polis.size() << " polis commands: \n";
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
		output << i << ". {" << typis << ", \"" << polis[i].second << "\"}\n";
	}
	std::wcout << runCode(polis).c_str() << '\n';
}