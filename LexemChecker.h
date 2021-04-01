#pragma once
#include <iostream>
#include <stack>
#include "TID.h"

class LexemChecker
{
public:
	LexemChecker(std::vector <lexem> lexems);
	~LexemChecker();

	bool checkProgram(); // <���������>
	TID* get_tree();
	std::vector<lexem>& get_polis();

private:

	// �������������� ������

	bool checkBlock(); // <����>
	bool checkUnsigned(); // <�������>
	bool checkNumber(); // <�����>
	bool checkInteger(); // <�����>
	bool checkFloat(); // <����������>
	bool checkString(); // <������>
	bool checkLogic(); // <������>
	bool checkPointer(); // <���������>
	bool checkName(); // <���>
	bool checkNames(); // <�����>
	bool checkType(); // <���>
	bool checkShift(bool after_type = 0); // <�����> ���� after_type = 1, �� ����� ������������ �������� ������� 

	bool checkSign1(); // <���� 1>
	bool checkSign2(); // <���� 2>
	bool checkSign3(); // <���� 3>
	bool checkSign4(); // <���� 4>
	bool checkSign5(); // <���� 5>
	bool checkSign6(); // <���� 6>
	bool checkSign7(); // <���� 7>
	bool checkSign8(); // <���� 8>
	bool checkSign9(); // <���� 9>
	bool checkSign10(); // <���� 10>
	bool checkSign11(); // <���� 11>
	bool checkSign12(); // <���� 12>
	bool checkSign13(); // <���� 13>
	bool checkSign14(); // <���� 14>
	bool checkSign15(); // <���� 15>
	bool checkSign16(); // <���� 16>

	bool checkExpression(); // <���������>
	bool checkExp2(); // <��� 2>
	bool checkExp3(); // <��� 3>
	bool checkExp4(); // <��� 4>
	bool checkExp5(); // <��� 5>
	bool checkExp6(); // <��� 6>
	bool checkExp7(); // <��� 7>
	bool checkExp8(); // <��� 8>
	bool checkExp9(); // <��� 9>
	bool checkExp10(); // <��� 10>
	bool checkExp11(); // <��� 11>
	bool checkExp12(); // <��� 12>
	bool checkExp13(); // <��� 13>
	bool checkExp14(); // <��� 14>
	bool checkExp15(); // <��� 15>
	bool checkExp16(); // <��� 16>
	bool checkVariable(); // <����������>
	bool checkConstant(); // <���������>
	int checkFuctionCall(); // <��������� �������>
	bool checkValue(); // <��������>
	bool checkOperator(); // <��������>
	bool checkOperators(); // <���������>
	bool checkExpOperator(); // <�������� ���������>
	bool checkStreamOperator(); // <�������� ������>
	bool checkSpecOperator(); // <����. ��������>
	bool checkIf(); // <�������� if>
	bool checkWhile(); // <�������� while>
	bool checkFor(); // <�������� for>
	bool checkGoto(); // <�������� ��������>
	bool checkDescription(); // <��������>
	bool checkFreeOperator(); // <��������� ��������>
	bool checkForOperator(); // <�������� ������ for>
	bool checkStructure(); // <�������� ���������>


	// ������������� ������

	bool checkKnown(std::wstring name); // �������� �� �������������?
	bool tryCast(TYPE type1, TYPE type2);

	// ��������������� �������

	bool match(int type, std::wstring word);
	bool isEnd();

	inline int type(int p);
	inline std::wstring& value(int p);

	void pushBlock();
	void popBlock();
	void pushId(std::wstring name, TYPE type);

	void pushFunctionDefault(TYPE type)
	{
		lexem value = { LEX_UNKNOWN, L"" };
		if (type.basic == TYPES::INT_ && type.depth == 0)
			value = { LEX_INT, L"0" };
		if (type.basic == TYPES::BOOL_ && type.depth == 0)
			value = { LEX_BOOL, L"0" };
		if (type.basic == TYPES::FLOAT_ && type.depth == 0)
			value = { LEX_FLOAT, L"0" };
		if (type.basic == TYPES::STRING_ && type.depth == 0)
			value = { LEX_STRING, L"0" };
		if (type.basic == TYPES::INT_ && type.depth == 1)
			value = { LEX_ARRAY | LEX_INT, L"0" };
		if (type.basic == TYPES::BOOL_ && type.depth == 1) 
			value = { LEX_ARRAY | LEX_BOOL, L"0" };
		if (type.basic == TYPES::FLOAT_ && type.depth == 1) 
			value = { LEX_ARRAY | LEX_FLOAT, L"0" };
		if (type.basic == TYPES::STRING_ && type.depth == 1) 
			value = { LEX_ARRAY | LEX_STRING, L"0" };
		polis_.push_back(value);
	}

	std::wstring popName(bool pop = 1);
	TYPE popType(bool pop = 1);
	lexem popSign(bool pop = 1);

	void runException(std::wstring reason);

	std::vector<lexem> term_, polis_;
	TID* tid_tree_;
	std::stack<TYPE> type_stack;
	std::stack<std::wstring> name_stack;
	std::stack<lexem> sign_stack;

	std::map<std::wstring, int> label_map;
	std::stack<std::pair<int, std::wstring> > goto_stack;
	std::stack<int> break_stack, continue_stack;
	int p = 0;
};

