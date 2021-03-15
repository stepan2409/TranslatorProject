#include "LexemChecker.h"



LexemChecker::LexemChecker(std::vector <lexem> lexems)
{
	term_.swap(lexems);
	tid_tree_ = new TID;
}

LexemChecker::~LexemChecker()
{
	while (tid_tree_->parent != nullptr)
	{
		tid_tree_ = tid_tree_->parent;
	}
	delete tid_tree_;
}

bool LexemChecker::checkProgram() // <���������>
{
	checkOperators();
	if (p != term_.size())
		runException(L"Expected operator");
	return 1;
}
bool LexemChecker::checkBlock() // <����>
{
	if (match(7, L"{"))
	{
		pushBlock();
		checkOperators();
		if (!match(7, L"}"))
			runException(L"Expected ending bracket");
		popBlock();
		return 1;
	}
	return 0;
}
bool LexemChecker::checkUnsigned() // <�������> 
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
bool LexemChecker::checkNumber() // <�����> 
{
	if (isEnd()) return 0;
	return checkInteger() || checkFloat();
}
bool LexemChecker::checkInteger() // <�����> 
{
	if (isEnd()) return 0;
	if (type(p) == 3)
	{
		++p;
		return 1;
	}
	return 0;
}
bool LexemChecker::checkFloat() // <����������> 
{
	if (isEnd()) return 0;
	if (type(p) == 4)
	{
		++p;
		return 1;
	}
	return 0;
}
bool LexemChecker::checkString() // <������> 
{
	if (isEnd()) return 0;
	if (type(p) == 5)
	{
		++p;
		return 1;
	}
	return 0;
}
bool LexemChecker::checkLogic() // <������> 
{
	return match(1, L"true") || match(1, L"false");
}
bool LexemChecker::checkPointer() // <���������> 
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
bool LexemChecker::checkName() // <���> 
{
	if (isEnd()) return 0;
	if (type(p) == 2)
	{
		name_stack.push_back(term_[p].second);
		++p;
		return 1;
	}
	return 0;
}
bool LexemChecker::checkNames() // <�����> 
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
bool LexemChecker::checkType() // <���> 
{
	if (isEnd()) return 0;
	int typ = 0, array_depth = 0;
	if (match(1, L"int"))
		typ = 1;
	else if (match(1, L"bool"))
		typ = 2;
	else if (match(1, L"float"))
		typ = 3;
	else if (match(1, L"string"))
		typ = 4;
	else return 0;
	while (checkShift()) ++array_depth;
	type_stack.push_back({ typ, array_depth });
	return 1;
}
bool LexemChecker::checkShift() // <�����> 
{
	if (isEnd()) return 0;
	int r = p;
	if (match(7, L"["))
	{
		if (checkExpression())
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

bool LexemChecker::checkSign1() // <���� 1> 
{
	return match(6, L"=") || match(6, L"+=") || match(6, L"-=") || match(6, L"*=") 
		|| match(6, L"/=") || match(6, L"|=") || match(6, L"&=");
}
bool LexemChecker::checkSign2() // <���� 2> 
{
	return match(6, L"||");
}
bool LexemChecker::checkSign3() // <���� 3> 
{
	return match(6, L"&&");
}
bool LexemChecker::checkSign4() // <���� 4> 
{
	return match(6, L"|");
}
bool LexemChecker::checkSign5() // <���� 5> 
{
	return match(6, L"^");
}
bool LexemChecker::checkSign6() // <���� 6> 
{
	return match(6, L"&");
}
bool LexemChecker::checkSign7() // <���� 7> 
{
	return match(6, L"==") || match(6, L"!=");
}
bool LexemChecker::checkSign8() // <���� 8> 
{
	return
		match(6, L"<") ||
		match(6, L">") ||
		match(6, L"<=") ||
		match(6, L">=");
}
bool LexemChecker::checkSign9() // <���� 9> 
{
	return match(6, L"<<") || match(6, L">>");
}
bool LexemChecker::checkSign10() // <���� 10> 
{
	return match(6, L"+") || match(6, L"-");
}
bool LexemChecker::checkSign11() // <���� 11> 
{
	return
		match(6, L"*") ||
		match(6, L"/") ||
		match(6, L"%");
}
bool LexemChecker::checkSign12() // <���� 12> 
{
	return match(6, L"!");
}
bool LexemChecker::checkSign13() // <���� 13> 
{
	return match(6, L"~");
}
bool LexemChecker::checkSign14() // <���� 14> 
{
	return match(6, L"+") || match(6, L"+");
}
bool LexemChecker::checkSign15() // <���� 15> 
{
	return match(6, L"++") || match(6, L"--");
}
bool LexemChecker::checkSign16() // <���� 16> 
{
	return match(6, L"++") || match(6, L"--");
}
bool LexemChecker::checkExpression() // <���������> 
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
bool LexemChecker::checkExp2() // <��� 2> 
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
bool LexemChecker::checkExp3() // <��� 3> 
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
bool LexemChecker::checkExp4() // <��� 4> 
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
bool LexemChecker::checkExp5() // <��� 5> 
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
bool LexemChecker::checkExp6() // <��� 6> 
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
bool LexemChecker::checkExp7() // <��� 7> 
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
bool LexemChecker::checkExp8() // <��� 8> 
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
bool LexemChecker::checkExp9() // <��� 9> 
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
bool LexemChecker::checkExp10() // <��� 10> 
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
bool LexemChecker::checkExp11() // <��� 11> 
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
bool LexemChecker::checkExp12() // <��� 12> 
{
	bool sgn = 0;
	while (checkSign12())
		sgn = 1;
	bool exp = checkExp13();
	if (sgn && !exp)
		runException(L"Expected value after sign");
	return exp;
}
bool LexemChecker::checkExp13() // <��� 13> 
{
	bool sgn = 0;
	while (checkSign13())
		sgn = 1;
	bool exp = checkExp14();
	if (sgn && !exp)
		runException(L"Expected value after sign");
	return exp;
}
bool LexemChecker::checkExp14() // <��� 14> 
{
	bool sgn = 0;
	while (checkSign14())
		sgn = 1;
	bool exp = checkExp15();
	if (sgn && !exp)
		runException(L"Expected value after sign");
	return exp;
}
bool LexemChecker::checkExp15() // <��� 15> 
{
	bool sgn = 0;
	while (checkSign15())
		sgn = 1;
	bool exp = checkExp16();
	if (sgn && !exp)
		runException(L"Expected value after sign");
	return exp;
}
bool LexemChecker::checkExp16() // <��� 16> 
{
	if (checkValue())
	{
		while (checkSign16());
		return 1;
	}
	return 0;
}
bool LexemChecker::checkVariable() // <����������> 
{
	return checkPointer();
}
bool LexemChecker::checkConstant() // <���������> 
{
	return checkNumber() || checkString() || checkLogic();
}
bool LexemChecker::checkFuctionCall() // <��������� �������> 
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
bool LexemChecker::checkValue() // <��������> 
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
bool LexemChecker::checkOperator() // <��������> 
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
bool LexemChecker::checkExpOperator() // <�������� ���������> 
{
	return checkExpression();
}
bool LexemChecker::checkStreamOperator() // <�������� ������> 
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
bool LexemChecker::checkSpecOperator() // <����. ��������> 
{
	return checkIf() || checkWhile() || checkFor() || checkGoto();
}
bool LexemChecker::checkIf() // <�������� if> 
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
			pushBlock();
			if (!(checkOperator() && match(7, L";")))
				runException(L"Expected operator");
			popBlock();
		}
		if (match(1, L"else"))
		{
			if (checkBlock() || checkFreeOperator())
				return 1;
			pushBlock();
			if (!(checkOperator() && match(7, L";")))
				runException(L"Expected operator");
			popBlock();
			return 1;
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkWhile() // <�������� while> 
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
		pushBlock();
		if (!(checkOperator() && match(7, L";")))
			runException(L"Expected operator");
		popBlock();
		return 1;
	}
	return 0;
}
bool LexemChecker::checkFor() // <�������� for> 
{
	if (match(1, L"for"))
	{
		pushBlock();
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
		pushBlock();
		if (!(checkOperator() && match(7, L";")))
			runException(L"LExpected operator");
		popBlock();
		return 1;

	}
	return 0;
}
bool LexemChecker::checkGoto() // <�������� ��������> 
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
bool LexemChecker::checkDescription() // <��������> 
{
	if (checkType())
	{
		if (!checkName())
			runException(L"Expected identifier");
		TYPE typ = type_stack.back();
		std::wstring nam = name_stack.back();
		pushId();
		int var_type = 0;
		if (match(6, L"="))
		{
			if (!checkExpression())
				runException(L"Expected expression");
			var_type = 1;
		}
		while (match(7, L","))
		{
			type_stack.push_back(typ);
			if (!checkName())
				runException(L"Expected identifier");
			pushId();
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
			std::vector<TYPE>* args = tid_tree_->get_arguments(nam);
			if (args == nullptr)
				runException(L"We've found a bug in checkDescription");
			if (checkType())
			{
				has_args = 1;
				args->push_back(type_stack.back());
				type_stack.pop_back();
				while (match(7, L","))
				{
					var_type = 2;
					if (!checkType())
						runException(L"Expected type");
					args->push_back(type_stack.back());
					type_stack.pop_back();
				}
				if (!var_type && checkName())
				{
					var_type = 3;
					while (match(7, L","))
					{
						if (!checkType())
							runException(L"Expected type");
						args->push_back(type_stack.back());
						type_stack.pop_back();
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
bool LexemChecker::checkStructure() // <�������� ���������> 
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

void LexemChecker::pushBlock()
{
	TID* block = new TID(tid_tree_);
	tid_tree_->children.push_back(block);
	tid_tree_ = block;
}

void LexemChecker::popBlock()
{
	if (tid_tree_->parent != nullptr)
		tid_tree_ = tid_tree_->parent;
}

void LexemChecker::pushId()
{
	if (!tid_tree_->push_id(name_stack.back(), type_stack.back()))
		runException(L"Identifier '" + name_stack.back() + L"' is already used");
	name_stack.pop_back();
	type_stack.pop_back();
}

void LexemChecker::runException(std::wstring reason)
{
	std::wcout << L"Syntax Error: " << reason << '\n';
	exit(0);
}
