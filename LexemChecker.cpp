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
	popBlock();
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
		type_stack.push(TYPE(TYPES::INT_));
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
		type_stack.push(TYPE(TYPES::FLOAT_));
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
		type_stack.push(TYPE(TYPES::STRING_));
		++p;
		return 1;
	}
	return 0;
}
bool LexemChecker::checkLogic() // <������> 
{
	if (match(1, L"true") || match(1, L"false"))
	{
		type_stack.push(TYPE(TYPES::BOOL_));
		return 1;
	}
	return 0;
}
bool LexemChecker::checkPointer() // <���������> 
{
	if (isEnd()) return 0;
	if (checkName())
	{
		std::wstring nam = popName();
		if (!checkKnown(nam))
			runException(L"Unknown identifier '" + nam + L"'");
		TYPE typ = tid_tree_->get_type(nam);
		typ.is_adress = 1;
		checkFuctionCall();
		while (checkShift())
		{
			if (typ.depth == 0)
				runException(L"Invalid type for array subscript");
			--typ.depth;
		}

		/*if (match(7, L".")) // �������������, ����� �������� ���������
		{
			if (checkPointer())
				return 1;
			runException(L"Invalid variable path\n");
		}*/
		type_stack.push(typ);
		return 1;
	}
	return 0;
}
bool LexemChecker::checkName() // <���> 
{
	if (isEnd()) return 0;
	if (type(p) == 2)
	{
		name_stack.push(term_[p].second);
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
	TYPES typ = TYPES::UNKNOWN;
	short array_depth = 0;
	if (match(1, L"int"))
		typ = TYPES::INT_;
	else if (match(1, L"bool"))
		typ = TYPES::BOOL_;
	else if (match(1, L"float"))
		typ = TYPES::FLOAT_;
	else if (match(1, L"string"))
		typ = TYPES::STRING_;
	else return 0;
	while (checkShift()) ++array_depth;
	type_stack.push(TYPE(typ, array_depth));
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
			popType();
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
	return match(6, L"+") || match(6, L"-");
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			if (!type1.is_adress)
				runException(L"Left value required as left operand of assignment");
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
			{
				if (type1 == type2)
				{
					type_stack.push(type1);
					continue;
				}
				runException(L"Incompatible types in expression");
			}
			if (type1 != STRING_TYPE && type1 != BOOL_TYPE && type2 == STRING_TYPE)
				runException(L"Incompatible types in expression");
			type_stack.push(BOOL_TYPE);
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(L"Incompatible types in expression");
			type_stack.push(BOOL_TYPE);
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(L"Incompatible types in expression");
			type_stack.push(BOOL_TYPE);
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(L"Incompatible types in expression");
			TYPE exp_type = BOOL_TYPE;
			if (type1 == FLOAT_TYPE || type1 == STRING_TYPE)
				runException(L"Incompatible types in expression");
			if (type2 == FLOAT_TYPE || type2 == STRING_TYPE)
				runException(L"Incompatible types in expression");
			if (type1 == INT_TYPE || type2 == INT_TYPE)
				exp_type = INT_TYPE;
			type_stack.push(exp_type);
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(L"Incompatible types in expression");
			TYPE exp_type = BOOL_TYPE;
			if (type1 == FLOAT_TYPE || type1 == STRING_TYPE)
				runException(L"Incompatible types in expression");
			if (type2 == FLOAT_TYPE || type2 == STRING_TYPE)
				runException(L"Incompatible types in expression");
			if (type1 == INT_TYPE || type2 == INT_TYPE)
				exp_type = INT_TYPE;
			type_stack.push(exp_type);
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(L"Incompatible types in expression");
			TYPE exp_type = BOOL_TYPE;
			if (type1 == FLOAT_TYPE || type1 == STRING_TYPE)
				runException(L"Incompatible types in expression");
			if (type2 == FLOAT_TYPE || type2 == STRING_TYPE)
				runException(L"Incompatible types in expression");
			if (type1 == INT_TYPE || type2 == INT_TYPE)
				exp_type = INT_TYPE;
			type_stack.push(exp_type);
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(L"Incompatible types in expression");
			if ((type1 == STRING_TYPE) != (type2 == STRING_TYPE))
				runException(L"Incompatible types in expression");
			type_stack.push(BOOL_TYPE);
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(L"Incompatible types in expression");
			if ((type1 == STRING_TYPE) != (type2 == STRING_TYPE))
				runException(L"Incompatible types in expression");
			type_stack.push(BOOL_TYPE);
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1 == BOOL_TYPE)
				type1 = INT_TYPE;
			if (type2 == BOOL_TYPE)
				type2 = INT_TYPE;
			if (type1 != INT_TYPE || type2 == INT_TYPE)
				runException(L"Incompatible types in expression");
			if (type1.depth > 0 || type2.depth > 0)
				runException(L"Incompatible types in expression");
			type_stack.push(INT_TYPE);
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if ((type1 == STRING_TYPE) != (type2 == STRING_TYPE))
				runException(L"Incompatible types in expression");
			TYPE exp_type = INT_TYPE;
			if (type1.depth > 0 || type2.depth > 0)
				runException(L"Incompatible types in expression");
			if (type1 == FLOAT_TYPE || type2 == FLOAT_TYPE)
				exp_type = FLOAT_TYPE;
			type_stack.push(exp_type);
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1 == STRING_TYPE || type2 == STRING_TYPE)
				runException(L"Incompatible types in expression");
			TYPE exp_type = INT_TYPE;
			if (type1.depth > 0 || type2.depth > 0)
				runException(L"Incompatible types in expression");
			if (type1 == FLOAT_TYPE || type2 == FLOAT_TYPE)
				exp_type = FLOAT_TYPE;
			type_stack.push(exp_type);
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
	if (sgn && exp)
	{
		TYPE type = popType();
		type.is_adress = 0;
		if (type == BOOL_TYPE)
			type_stack.push(type);
		if (type == INT_TYPE || type == FLOAT_TYPE)
			type_stack.push(BOOL_TYPE);
		else runException(L"Incorrect type in expression");
		return 1;
	}
	if (sgn && !exp)
		runException(L"Expected value after sign");
	return exp;
}
bool LexemChecker::checkExp13() // <��� 13> 
{
	bool sgn = 0;
	while (checkSign13())
	{
		sgn = 1;
	}
	bool exp = checkExp14();
	if (sgn && exp)
	{
		TYPE type = popType();
		if (type == INT_TYPE || type == BOOL_TYPE)
		{
			type.is_adress = 0;
			type_stack.push(type);
			return 1;
		}
		else runException(L"Incorrect type in expression");
	}
	if (sgn && !exp)
		runException(L"Expected value after sign");
	return exp;
}
bool LexemChecker::checkExp14() // <��� 14> 
{
	bool sgn = 0;
	while (checkSign14())
	{
		sgn = 1;
	}
	bool exp = checkExp15();
	if (sgn && exp)
	{
		TYPE type = popType();
		if (type == INT_TYPE || type == BOOL_TYPE || type == FLOAT_TYPE)
		{
			type.is_adress = 0;
			type_stack.push(type);
			return 1;
		}
		else runException(L"Incorrect type in expression");
	}
	if (sgn && !exp)
		runException(L"Expected value after sign");
	return exp;
}
bool LexemChecker::checkExp15() // <��� 15> 
{
	bool sgn = 0;
	while (checkSign15())
	{
		sgn = 1;
	}
	bool exp = checkExp16();
	if (sgn && exp)
	{
		TYPE type = popType(0);
		if (!type.is_adress)
			runException(L"This isn't a variable");
		if (type == INT_TYPE)
			return 1;
		if (type == BOOL_TYPE)
			return 1;
		if (type == FLOAT_TYPE)
			return 1;
		runException(L"Incorrect type in expression");
	}
	if (sgn && !exp)
		runException(L"Expected value after sign");
	return exp;
}
bool LexemChecker::checkExp16() // <��� 16> 
{
	if (checkValue())
	{
		if (checkSign16())
		{
			while (checkSign16());
			TYPE type = popType(0);
			if (!type.is_adress)
				runException(L"This isn't a variable");
			if (type == INT_TYPE)
				return 1;
			if (type == BOOL_TYPE)
				return 1;
			if (type == FLOAT_TYPE)
				return 1;
			runException(L"Incorrect type in expression");
		}
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
			popType();
			while (match(7, L","))
			{
				if (!checkExpression())
					runException(L"Expected argument");
				popType();
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
	//if (checkStructure()) return 1; // �������������, ����� ������� ���������
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
	if (checkExpression())
	{
		popType();
		return 1;
	}
	return 0;
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
		popType();
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
		popType();
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
		popType();
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
		{
			popBlock();
			return 1;
		}
		pushBlock();
		if (!(checkOperator() && match(7, L";")))
			runException(L"LExpected operator");
		popBlock();
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
		popType();
		return 1;
	}
	if (match(1, L"goto"))
	{
		if (!checkName())
			runException(L"Expected identifier");
		if (!checkKnown(popName(0)))
			runException(L"Unknown label '" + popName(0) + L"'");
		else popName();
		return 1;
	}
	if (match(1, L"label"))
	{
		if (!checkName())
			runException(L"Expected identifier");
		pushId(popName(), { TYPES::ADRESS_, 0 });
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
		int var_type = 0;
		std::wstring nam = popName(0);
		bool is_temp = tid_tree_->is_template(nam, popType(0));
		if (!is_temp)
		{
			pushId(popName(), popType(0));
			if (match(6, L"="))
			{
				if (!checkExpression())
					runException(L"Expected expression");
				popType();
				var_type = 1;
			}
			while (match(7, L","))
			{
				if (!checkName())
					runException(L"Expected identifier");
				pushId(popName(), popType(0));
				if (match(6, L"="))
				{
					if (!checkExpression())
						runException(L"Expected expression");
					popType();
				}
				var_type = 1;
			}
			popType();
		}
		if (!var_type && match(7, L"("))
		{
			TID* last_tid = tid_tree_;
			pushBlock();
			bool has_args = 0, temp_step = 0;
			std::vector<TYPE>* args = tid_tree_->get_arguments(nam);
			if (args == nullptr)
				runException(L"We've found a bug in checkDescription");
			if (checkType())
			{
				has_args = 1;
				TYPE first_type = popType();
				args->push_back(first_type);
				while (match(7, L","))
				{
					var_type = 2;
					if (is_temp)
						runException(L"Only one template can be created");
					if (!checkType())
						runException(L"Expected type");
					args->push_back(popType());
				}
				if (!var_type && checkName())
				{
					var_type = 3;
					last_tid->push_code(nam);
					std::vector<std::wstring> new_names;

					if (checkKnown(popName(0)))
						runException(L"Identifier '" + popName(0) + L"' is already used");
					new_names.push_back(popName(0));
					pushId(popName(), first_type);

					while (match(7, L","))
					{
						if (!checkType())
							runException(L"Expected type");
						if (!is_temp)
						{
							if (temp_step >= args->size())
								runException(L"Too few arguments in description");
							if (*(args->begin() + temp_step) != popType(0))
								runException(L"Argument " + std::to_wstring(temp_step + 1) + L" has wrong type");
						}
						else args->push_back(popType(0));
						if (!checkName())
							runException(L"Expected identifier");
						if (std::find(new_names.begin(), new_names.end(), popName(0)) != new_names.end())
							runException(L"Argument '" + popName(0) + L"' is already used");
						if (checkKnown(popName(0)))
							runException(L"Identifier '" + popName(0) + L"' is already used");
						new_names.push_back(popName(0));
						pushId(popName(), popType());
					}
					if (!match(7, L")"))
						runException(L"Expected ending bracket");
					if (!checkBlock())
						runException(L"Expected description");
					popBlock();
					return 1;
				}
			}
			if (!match(7, L")"))
				runException(L"Expected ending bracket");
			if (!var_type && !has_args)
			{
				if (!checkBlock())
					runException(L"Expected description");
				last_tid->push_code(nam);
			}
			popBlock();
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
	if (checkExpression())
	{
		popType();
		return 1;
	}
	return checkDescription();
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

bool LexemChecker::checkKnown(std::wstring name)
{
	return tid_tree_->get_type(name) != NO_TYPE;
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
	std::wstring temp = tid_tree_->get_any_template();
	if (!temp.empty())
		runException(L"Template '" + temp + L"' must have definition in the same block");
	if (tid_tree_->parent != nullptr)
		tid_tree_ = tid_tree_->parent;
}

void LexemChecker::pushId(std::wstring name, TYPE type)
{
	if (!tid_tree_->push_id(name, type))
		runException(L"Identifier '" + name + L"' is already used");
}

std::wstring LexemChecker::popName(bool pop)
{
	if (name_stack.empty())
		runException(L"We've found a bug in popName");
	auto res = name_stack.top();
	if (pop) name_stack.pop();
	return res;
}

TYPE LexemChecker::popType(bool pop)
{
	if (type_stack.empty())
		runException(L"We've found a bug in popName");
	auto res = type_stack.top();
	if (pop) type_stack.pop();
	return res;
}

void LexemChecker::runException(std::wstring reason)
{
	std::wcout << L"Syntax Error: " << reason << '\n';
	exit(0);
}
