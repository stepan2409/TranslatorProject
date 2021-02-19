#pragma once
#include <iostream>
#include <vector>
#include <string>

typedef std::pair<int, std::wstring> lexem;

class LexemChecker
{
public:
	LexemChecker(std::vector <lexem> lexems);

	bool checkProgram(); // <���������>
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
	bool checkShift(); // <�����>

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
	bool checkFuctionCall(); // <��������� �������>
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

	bool match(int type, std::wstring word);
	bool isEnd();

	inline int type(int p);
	inline std::wstring& value(int p);

	void runException(std::wstring reason);

	std::vector<lexem> term_;
	int p = 0;
};

