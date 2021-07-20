#pragma once
#include <stack>
#include <string>
#include <any>
#include <algorithm>
#include "TID.h"

typedef std::pair<TYPE, std::any> value;
#define NO_VALUE {NO_TYPE, NULL}

class CodeRunner
{

public:
	CodeRunner();
	~CodeRunner();

	void pushValue(value val);
	void pushValue(lexem& val);
	void pushValue(int val);
	void pushValue(bool val);
	void pushValue(float val);
	void pushValue(std::wstring val);
	void pushValue(wchar_t val);

	void runOperation(std::wstring op);
	void runEquation(std::wstring op);
	void runMemOperation(std::wstring op);
	void runMatOperation(std::wstring op);
	void runBinOperation(std::wstring op);
	void runLogicOperation(std::wstring op);
	void (*runException)(int, std::wstring, std::wstring);

	value getValue();
	value getPointer();
	int getInt();
	bool getBool();
	float getFloat();
	std::wstring getString();
	wchar_t getChar();
	int getSize(value& arr);
	int getSize(value& arr, int& size);

private:
	// ссылки на массивы из Translator
	std::vector<std::stack<int>>* int_values;
	std::vector<std::stack<bool>>* bool_values;
	std::vector<std::stack<float>>* float_values;
	std::vector<std::stack<std::wstring>>* string_values;

	std::vector<std::stack<int*>>* int_values_arr;
	std::vector<std::stack<bool*>>* bool_values_arr;
	std::vector<std::stack<float*>>* float_values_arr;
	std::vector<std::stack<std::wstring*>>* string_values_arr;

	std::vector<std::stack<std::vector<int>> >* int_sizes, * bool_sizes, * float_sizes, * string_sizes;

	std::stack<std::pair<TYPE, std::any>> runtime_stack;

	std::vector<const wchar_t*> mem_operations = { L"+=", L"-=", L"*=", L"/=", L"%=", L"|=", L"&=", L"^=", L"++a", L"--a", L"a++", L"a--" };
	std::vector<const wchar_t*> mat_operations = { L"+", L"-", L"*", L"/", L"%" };
	std::vector<const wchar_t*> bin_operations = { L"|", L"&", L"^", L"~", L">>", L"<<" };
	std::vector<const wchar_t*> logic_operations = { L"||", L"&&", L"==", L"!=", L">", L"<", L">=", L"<=" };
	bool isIn(std::wstring op, std::vector<const wchar_t*>& ops);

	TYPE getTopType(TYPE t1, TYPE t2);

	friend void set_values_function(TID*);
};

