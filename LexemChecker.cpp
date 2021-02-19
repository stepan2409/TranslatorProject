#include "LexemChecker.h"



LexemChecker::LexemChecker(std::vector <lexem> lexems)
{
	term_.swap(lexems);
}

bool LexemChecker::checkProgram() // <программа>
{
	checkOperators();
	if (p != term_.size())
		runException(L"Expected operator");
	return 1;
}
bool LexemChecker::checkBlock() // <блок>
{
	if (match(7, L"{"))
	{
		checkOperators();
		if (!match(7, L"}"))
			runException(L"Expected ending bracket");
		return 1;
	}
	return 0;
}
bool LexemChecker::checkUnsigned() // <беззнак> 
{
	if (isEnd()) return 0;
	if (type(p) == 0)
		runException(L"Unexpected symbol");
	if (type(p) != 3)
		return 0;
	if (value(p)[0] <= '9' && value(p)[0] >= '0')
	{
		++p;
		return 1;
	}
	return 0;
}
bool LexemChecker::checkNumber() // <число> 
{
	if (isEnd()) return 0;
	return checkInteger() || checkFloat();
}
bool LexemChecker::checkInteger() // <целое> 
{
	if (isEnd()) return 0;
	if (type(p) == 3)
	{
		++p;
		return 1;
	}
	return 0;
}
bool LexemChecker::checkFloat() // <десятичное> 
{
	if (isEnd()) return 0;
	if (type(p) == 4)
	{
		++p;
		return 1;
	}
	return 0;
}
bool LexemChecker::checkString() // <строка> 
{
	if (isEnd()) return 0;
	if (type(p) == 5)
	{
		++p;
		return 1;
	}
	return 0;
}
bool LexemChecker::checkLogic() // <строка> 
{
	return match(1, L"true") || match(1, L"false");
}
bool LexemChecker::checkPointer() // <указатель> 
{
	if (isEnd()) return 0;
	if (checkName())
	{
		checkFuctionCall();
		while (checkShift());
		if (match(7, L"."))
		{
			if (checkPointer())
				return 1;
			runException(L"Invalid variable path\n");
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkName() // <имя> 
{
	if (isEnd()) return 0;
	if (type(p) == 2)
	{
		++p;
		return 1;
	}
	return 0;
}
bool LexemChecker::checkNames() // <имена> 
{
	if (isEnd()) return 0;
	if (checkName())
	{
		if (match(7, L","))
		{
			if (checkNames())
				return 1;
			runException(L"Invalid identifier");
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkType() // <тип> 
{
	if (isEnd()) return 0;
	if (match(1, L"int"))
		return 1;
	if (match(1, L"float"))
		return 1;
	if (match(1, L"bool"))
		return 1;
	if (match(1, L"string"))
		return 1;
	return 0;
}
bool LexemChecker::checkShift() // <сдвиг> 
{
	if (isEnd()) return 0;
	int r = p;
	if (match(7, L"["))
	{
		if (checkPointer())
		{
			if (match(7, L"]"))
			{
				return 1;
			}
		}
		runException(L"Incorrect Array's index");
	}
	return 0;
}

bool LexemChecker::checkSign1() // <знак 1> 
{
	return match(6, L"=") || match(6, L"+=") || match(6, L"-=") || match(6, L"*=") 
		|| match(6, L"/=") || match(6, L"|=") || match(6, L"&=");
}
bool LexemChecker::checkSign2() // <знак 2> 
{
	return match(6, L"||");
}
bool LexemChecker::checkSign3() // <знак 3> 
{
	return match(6, L"&&");
}
bool LexemChecker::checkSign4() // <знак 4> 
{
	return match(6, L"|");
}
bool LexemChecker::checkSign5() // <знак 5> 
{
	return match(6, L"^");
}
bool LexemChecker::checkSign6() // <знак 6> 
{
	return match(6, L"&");
}
bool LexemChecker::checkSign7() // <знак 7> 
{
	return match(6, L"==") || match(6, L"!=");
}
bool LexemChecker::checkSign8() // <знак 8> 
{
	return
		match(6, L"<") ||
		match(6, L">") ||
		match(6, L"<=") ||
		match(6, L">=");
}
bool LexemChecker::checkSign9() // <знак 9> 
{
	return match(6, L"<<") || match(6, L">>");
}
bool LexemChecker::checkSign10() // <знак 10> 
{
	return match(6, L"+") || match(6, L"-");
}
bool LexemChecker::checkSign11() // <знак 11> 
{
	return
		match(6, L"*") ||
		match(6, L"/") ||
		match(6, L"%");
}
bool LexemChecker::checkSign12() // <знак 12> 
{
	return match(6, L"!");
}
bool LexemChecker::checkSign13() // <знак 13> 
{
	return match(6, L"~");
}
bool LexemChecker::checkSign14() // <знак 14> 
{
	return match(6, L"+") || match(6, L"+");
}
bool LexemChecker::checkSign15() // <знак 15> 
{
	return match(6, L"++") || match(6, L"--");
}
bool LexemChecker::checkSign16() // <знак 16> 
{
	return match(6, L"++") || match(6, L"--");
}
bool LexemChecker::checkExpression() // <выражение> 
{
	if (checkExp2())
	{
		while (checkSign1())
		{
			if (!checkExp2())
				runException(L"Expected value in expression");
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkExp2() // <выр 2> 
{
	if (checkExp3())
	{
		while (checkSign2())
		{
			if (!checkExp3())
				runException(L"Expected value in expression");
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkExp3() // <выр 3> 
{
	if (checkExp4())
	{
		while (checkSign3())
		{
			if (!checkExp4())
				runException(L"Expected value in expression");
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkExp4() // <выр 4> 
{
	if (checkExp5())
	{
		while (checkSign4())
		{
			if (!checkExp5())
				runException(L"Expected value in expression");
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkExp5() // <выр 5> 
{
	if (checkExp6())
	{
		while (checkSign5())
		{
			if (!checkExp6())
				runException(L"Expected value in expression");
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkExp6() // <выр 6> 
{
	if (checkExp7())
	{
		while (checkSign6())
		{
			if (!checkExp7())
				runException(L"Expected value in expression");
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkExp7() // <выр 7> 
{
	if (checkExp8())
	{
		while (checkSign7())
		{
			if (!checkExp8())
				runException(L"Expected value in expression");
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkExp8() // <выр 8> 
{
	if (checkExp9())
	{
		while (checkSign8())
		{
			if (!checkExp9())
				runException(L"Expected value in expression");
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkExp9() // <выр 9> 
{
	if (checkExp10())
	{
		while (checkSign9())
		{
			if (!checkExp10())
				runException(L"Expected value in expression");
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkExp10() // <выр 10> 
{
	if (checkExp11())
	{
		while (checkSign10())
		{
			if (!checkExp11())
				runException(L"Expected value in expression");
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkExp11() // <выр 11> 
{
	if (checkExp12())
	{
		while (checkSign11())
		{
			if (!checkExp12())
				runException(L"Expected value in expression");
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkExp12() // <выр 12> 
{
	bool sgn = 0;
	while (checkSign12())
		sgn = 1;
	bool exp = checkExp13();
	if (sgn && !exp)
		runException(L"Expected value after sign");
	return exp;
}
bool LexemChecker::checkExp13() // <выр 13> 
{
	bool sgn = 0;
	while (checkSign13())
		sgn = 1;
	bool exp = checkExp14();
	if (sgn && !exp)
		runException(L"Expected value after sign");
	return exp;
}
bool LexemChecker::checkExp14() // <выр 14> 
{
	bool sgn = 0;
	while (checkSign14())
		sgn = 1;
	bool exp = checkExp15();
	if (sgn && !exp)
		runException(L"Expected value after sign");
	return exp;
}
bool LexemChecker::checkExp15() // <выр 15> 
{
	bool sgn = 0;
	while (checkSign15())
		sgn = 1;
	bool exp = checkExp16();
	if (sgn && !exp)
		runException(L"Expected value after sign");
	return exp;
}
bool LexemChecker::checkExp16() // <выр 16> 
{
	if (checkValue())
	{
		while (checkSign16());
		return 1;
	}
	return 0;
}
bool LexemChecker::checkVariable() // <переменная> 
{
	return checkPointer();
}
bool LexemChecker::checkConstant() // <константа> 
{
	return checkNumber() || checkString() || checkLogic();
}
bool LexemChecker::checkFuctionCall() // <результат функции> 
{
	if (match(7, L"("))
	{
		if (checkExpression())
		{
			while (match(7, L","))
			{
				if (!checkExpression())
					runException(L"Expected argument");
			}
		}
		if (!match(7, L")"))
			runException(L"Invalid argument");
		return 1;
	}
	return 0;
}
bool LexemChecker::checkValue() // <значение> 
{
	if (checkVariable() || checkConstant())
		return 1;
	if (match(7, L"("))
	{
		if (!checkExpression())
			runException(L"Expected expession");
		if (!match(7, L")"))
			runException(L"Expected ending bracket");
		return 1;
	}
	return 0;
}
bool LexemChecker::checkOperator() // <оператор> 
{
	if (checkExpOperator()) return 1;
	if (checkStreamOperator()) return 1;
	if (checkSpecOperator()) return 1;
	if (checkDescription()) return 1;
	if (checkStructure()) return 1;
	return 0;
}
bool LexemChecker::checkOperators()
{
	bool isop = 0;
	do
	{
		isop = 0;
		if (checkFreeOperator() || checkBlock())
			isop = 1;
		else if (checkOperator())
		{
			if (!match(7, L";"))
				runException(L"Excepted ';'");
			isop = 1;
		}
	} while (isop);
	return 1;
}
bool LexemChecker::checkExpOperator() // <оператор выражения> 
{
	return checkExpression();
}
bool LexemChecker::checkStreamOperator() // <оператор потока> 
{
	if (match(1, L"in"))
	{
		if (!checkVariable())
			runException(L"Expected variable");
		return 1;
	}
	if (match(1, L"out"))
	{
		if (!checkExpression())
			runException(L"Expected expression");
		return 1;
	}
	return 0;
}
bool LexemChecker::checkSpecOperator() // <спец. оператор> 
{
	return checkIf() || checkWhile() || checkFor() || checkGoto();
}
bool LexemChecker::checkIf() // <оператор if> 
{
	if (match(1, L"if"))
	{
		if (!match(7, L"("))
			runException(L"Expected arguments in brackets");
		if (!checkExpression())
			runException(L"Expected expression");
		if (!match(7, L")"))
			runException(L"Expected ending bracket");
		if (!(checkBlock() || checkFreeOperator()))
		{
			if (!(checkOperator() && match(7, L";")))
				runException(L"Expected operator");
		}
		if (match(1, L"else"))
		{
			if (checkBlock() || checkFreeOperator())
				return 1;
			if (checkOperator() && match(7, L";"))
				return 1;
			runException(L"Expected operator");
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkWhile() // <оператор while> 
{
	if (match(1, L"while"))
	{
		if (!match(7, L"("))
			runException(L"Expected arguments in brackets");
		if (!checkExpression())
			runException(L"Expected expression");
		if (!match(7, L")"))
			runException(L"Expected ending bracket");
		if (checkBlock() || checkFreeOperator())
			return 1;
		if (checkOperator() && match(7, L";"))
			return 1;
		runException(L"Expected operator");
	}
	return 0;
}
bool LexemChecker::checkFor() // <оператор for> 
{
	if (match(1, L"for"))
	{
		if (!match(7, L"("))
			runException(L"Expected arguments in brackets");
		checkForOperator();
		if (!match(7, L";"))
			runException(L"Expected ';'");
		checkForOperator();
		if (!match(7, L";"))
			runException(L"Expected ';'");
		checkForOperator();
		if (!match(7, L")"))
			runException(L"Expected ending bracket");
		if (checkBlock() || checkFreeOperator())
			return 1;
		if (checkOperator() && match(7, L";"))
			return 1;
		runException(L"LExpected operator");
	}
	return 0;
}
bool LexemChecker::checkGoto() // <оператор перехода> 
{
	if (match(1, L"break")) return 1;
	if (match(1, L"continue")) return 1;
	if (match(1, L"return"))
	{
		if (!checkExpression())
			runException(L"Expected expression");
		return 1;
	}
	if (match(1, L"goto"))
	{
		if (!checkName())
			runException(L"Expected identifier");
		return 1;
	}
	if (match(1, L"label"))
	{
		if (!checkName())
			runException(L"Expected identifier");
		return 1;
	}
	return 0;
}
bool LexemChecker::checkDescription() // <описание> 
{
	if (checkType())
	{
		if (!checkName())
			runException(L"Expected identifier");
		int var_type = 0;
		if (match(6, L"="))
		{
			if (!checkExpression())
				runException(L"Expected expression");
			var_type = 1;
		}
		while (match(7, L","))
		{
			if (!checkName())
				runException(L"Expected identifier");
			if (match(6, L"="))
			{
				if (!checkExpression())
					runException(L"Expected expression");
			}
			var_type = 1;
		}
		if (!var_type && match(7, L"("))
		{
			bool has_args = 0;
			if (checkType())
			{
				has_args = 1;
				while (match(7, L","))
				{
					var_type = 2;
					if (!checkType())
						runException(L"Expected type");
				}
				if (!var_type && checkName())
				{
					var_type = 3;
					while (match(7, L","))
					{
						if (!checkType())
							runException(L"Expected type");
						if (!checkName())
							runException(L"Expected identifier");
					}
					if (!match(7, L")"))
						runException(L"Expected ending bracket");
					if (!checkBlock())
						runException(L"Expected description");
					return 1;
				}
			}
			if (!match(7, L")"))
				runException(L"Expected ending bracket");
			if (!var_type && !has_args)
				checkBlock();
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkFreeOperator()
{
	return checkIf() || checkWhile() || checkFor();
}
bool LexemChecker::checkForOperator()
{
	return checkExpression() || checkDescription();
}
bool LexemChecker::checkStructure() // <описание структуры> 
{
	if (match(1, L"struct"))
	{
		if (!checkName())
			runException(L"Expected identifier");
		if (!checkBlock())
			runException(L"Expected description");
		return 1;
	}
	return 0;
}

bool LexemChecker::match(int type, std::wstring word)
{

	if (isEnd()) return 0;
	if (term_[p].first == 0)
		runException(L"Unexpected symbol");
	if (term_[p].first != type)
		return 0;
	if (term_[p].second != word)
		return 0;
	++p;
	return 1;
}

bool LexemChecker::isEnd()
{
	return p >= term_.size();
}

inline int LexemChecker::type(int p)
{
	return term_[p].first;
}

inline std::wstring& LexemChecker::value(int p)
{
	return term_[p].second;
}

void LexemChecker::runException(std::wstring reason)
{
	std::wcout << L"Syntax Error: " << reason << '\n';
	exit(0);
}
