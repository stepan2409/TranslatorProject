#pragma once
#include <iostream>
#include <stack>
#include "TID.h"

typedef std::pair<int, std::wstring> lexem;

class LexemChecker
{
public:
	LexemChecker(std::vector <lexem> lexems);
	~LexemChecker();

	bool checkProgram(); // <программа>

private:

	// Синтаксический анализ

	bool checkBlock(); // <блок>
	bool checkUnsigned(); // <беззнак>
	bool checkNumber(); // <число>
	bool checkInteger(); // <целое>
	bool checkFloat(); // <десятичное>
	bool checkString(); // <строка>
	bool checkLogic(); // <строка>
	bool checkPointer(); // <указатель>
	bool checkName(); // <имя>
	bool checkNames(); // <имена>
	bool checkType(); // <тип>
	bool checkShift(); // <сдвиг>

	bool checkSign1(); // <знак 1>
	bool checkSign2(); // <знак 2>
	bool checkSign3(); // <знак 3>
	bool checkSign4(); // <знак 4>
	bool checkSign5(); // <знак 5>
	bool checkSign6(); // <знак 6>
	bool checkSign7(); // <знак 7>
	bool checkSign8(); // <знак 8>
	bool checkSign9(); // <знак 9>
	bool checkSign10(); // <знак 10>
	bool checkSign11(); // <знак 11>
	bool checkSign12(); // <знак 12>
	bool checkSign13(); // <знак 13>
	bool checkSign14(); // <знак 14>
	bool checkSign15(); // <знак 15>
	bool checkSign16(); // <знак 16>

	bool checkExpression(); // <выражение>
	bool checkExp2(); // <выр 2>
	bool checkExp3(); // <выр 3>
	bool checkExp4(); // <выр 4>
	bool checkExp5(); // <выр 5>
	bool checkExp6(); // <выр 6>
	bool checkExp7(); // <выр 7>
	bool checkExp8(); // <выр 8>
	bool checkExp9(); // <выр 9>
	bool checkExp10(); // <выр 10>
	bool checkExp11(); // <выр 11>
	bool checkExp12(); // <выр 12>
	bool checkExp13(); // <выр 13>
	bool checkExp14(); // <выр 14>
	bool checkExp15(); // <выр 15>
	bool checkExp16(); // <выр 16>
	bool checkVariable(); // <переменная>
	bool checkConstant(); // <константа>
	 int checkFuctionCall(); // <результат функции>
	bool checkValue(); // <значение>
	bool checkOperator(); // <оператор>
	bool checkOperators(); // <операторы>
	bool checkExpOperator(); // <оператор выражения>
	bool checkStreamOperator(); // <оператор потока>
	bool checkSpecOperator(); // <спец. оператор>
	bool checkIf(); // <оператор if>
	bool checkWhile(); // <оператор while>
	bool checkFor(); // <оператор for>
	bool checkGoto(); // <оператор перехода>
	bool checkDescription(); // <описание>
	bool checkFreeOperator(); // <свободный оператор>
	bool checkForOperator(); // <оператор внутри for>
	bool checkStructure(); // <описание структуры>


	// Семантический анализ

	bool checkKnown(std::wstring name); // известен ли идентификатор?
	bool tryCast(TYPE type1, TYPE type2);

	// Вспомогательные функции

	bool match(int type, std::wstring word);
	bool isEnd();

	inline int type(int p);
	inline std::wstring& value(int p);

	void pushBlock();
	void popBlock();
	void pushId(std::wstring name, TYPE type);

	std::wstring popName(bool pop = 1);
	TYPE popType(bool pop = 1);

	void runException(std::wstring reason);

	std::vector<lexem> term_;
	TID* tid_tree_;
	std::stack<TYPE> type_stack;
	std::stack<std::wstring> name_stack;
	int p = 0;
};

