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

bool LexemChecker::checkProgram() // <программа>
{
	checkOperators();
	if (p != term_.size())
		runException(L"Expected operator");
	popBlock();
	if (break_stack.size() > 0)
		runException(L"Operator 'break' must be used in cycle");
	if (continue_stack.size() > 0)
		runException(L"Operator 'continue' must be used in cycle");
	while (goto_stack.size())
	{
		if (label_map.find(goto_stack.top().second) == label_map.end())
			runException(L"Label '" + goto_stack.top().second + L"' must be defined");
		int code_line = label_map[goto_stack.top().second];
		polis_[goto_stack.top().first].second = std::to_wstring(code_line);
		goto_stack.pop();
	}
	return 1;
}
TID* LexemChecker::get_tree()
{
	return tid_tree_;
}
std::vector<lexem>& LexemChecker::get_polis()
{
	return polis_;
}
bool LexemChecker::checkBlock() // <блок>
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
		type_stack.push(TYPE(TYPES::INT_));
		polis_.push_back({ LEX_INT, term_[p].second });
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
		type_stack.push(TYPE(TYPES::FLOAT_));
		polis_.push_back({ LEX_FLOAT, term_[p].second });
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
		type_stack.push(TYPE(TYPES::STRING_));
		polis_.push_back({ LEX_STRING, term_[p].second });
		++p;
		return 1;
	}
	return 0;
}
bool LexemChecker::checkLogic() // <строка> 
{
	if (match(1, L"true") || match(1, L"false"))
	{
		type_stack.push(TYPE(TYPES::BOOL_));
		polis_.push_back({ LEX_BOOL, term_[p - 1].second });
		return 1;
	}
	return 0;
}
bool LexemChecker::checkPointer() // <указатель> 
{
	if (isEnd()) return 0;
	if (checkName())
	{
		std::wstring nam = popName();
		if (!checkKnown(nam))
			runException(L"Unknown identifier '" + nam + L"'");

		TYPE typ = tid_tree_->get_type(nam);
		typ.is_adress = 1;
		int arg_count = checkFuctionCall();
		if (tid_tree_->is_template(nam, typ))
			runException(L"Fuction must be decripted before call");
		if (arg_count)
		{
			if (arg_count == -1)
				arg_count = 0;
			if (!tid_tree_->is_function(nam))
				runException(L"Identifier isn't a function");
			std::vector<TYPE> * args = tid_tree_->get_arguments(nam);
			if (args == nullptr)
				runException(L"We have a bug in checkPointer");
			if (arg_count < args->size())
				runException(L"Too few arguments to function");
			bool too_args = arg_count > args->size();
			while (arg_count > args->size())
			{
				--arg_count;
				popType();
			}
			for (int i = args->size() - 1; i >= 0; --i)
			{
				tryCast(*(args->begin() + i), popType());

			}
			if (too_args)
				runException(L"Too many arguments in function");
			int code_line = tid_tree_->get_code(nam);
			if (code_line == -1)
				runException(L"We have a bug in checkPointer");
			polis_.push_back({ LEX_INT, std::to_wstring(code_line) });
			polis_.push_back({ LEX_OPERATION, L"push-stack" });
			polis_.push_back({ LEX_OPERATION, L"!" });
		}
		else
		{
			if (tid_tree_->is_function(nam))
				runException(L"Required function call");
			polis_.push_back(tid_tree_->get_adress(nam));
		}
		while (checkShift())
		{
			if (typ.depth == 0)
				runException(L"Invalid type for array subscript");
			--typ.depth;
		}

		/*if (match(7, L".")) // раскомментить, чтобы включить структуры
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
bool LexemChecker::checkName() // <имя> 
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
bool LexemChecker::checkShift(bool after_type) // <сдвиг> если after_type = 1, то полиз обрабатывает создание массива 
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
				if (!after_type)
					polis_.push_back({ LEX_OPERATION, L"->" });
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
	return match(6, L"+") || match(6, L"-");
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			if (!type1.is_adress)
				runException(L"Left value required as left operand of assignment");
			type2.is_adress = 0;
			tryCast(type1, type2);
			polis_.push_back(popSign());
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(L"Incompatible types in expression");
			type_stack.push(BOOL_TYPE);
			polis_.push_back(popSign());
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(L"Incompatible types in expression");
			type_stack.push(BOOL_TYPE);
			polis_.push_back(popSign());
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
			polis_.push_back(popSign());
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
			polis_.push_back(popSign());
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
			polis_.push_back(popSign());
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(L"Incompatible types in expression");
			if ((type1 == STRING_TYPE) != (type2 == STRING_TYPE))
				runException(L"Incompatible types in expression");
			type_stack.push(BOOL_TYPE);
			polis_.push_back(popSign());
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
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(L"Incompatible types in expression");
			if ((type1 == STRING_TYPE) != (type2 == STRING_TYPE))
				runException(L"Incompatible types in expression");
			type_stack.push(BOOL_TYPE);
			polis_.push_back(popSign());
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
			polis_.push_back(popSign());
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
			polis_.push_back(popSign());
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
			polis_.push_back(popSign());
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkExp12() // <выр 12> 
{
	int sgn = 0;
	while (checkSign12())
	{
		++sgn;
	}
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
		while (sgn--)
		{
			polis_.push_back(popSign());
		}
		return 1;
	}
	if (sgn && !exp)
		runException(L"Expected value after sign");
	return exp;
}
bool LexemChecker::checkExp13() // <выр 13> 
{
	int sgn = 0;
	while (checkSign13())
	{
		++sgn;
	}
	bool exp = checkExp14();
	if (sgn && exp)
	{
		TYPE type = popType();
		if (type == INT_TYPE || type == BOOL_TYPE)
		{
			type.is_adress = 0;
			type_stack.push(type);
			while (sgn--)
			{
				polis_.push_back(popSign());
			}
			return 1;
		}
		else runException(L"Incorrect type in expression");
	}
	if (sgn && !exp)
		runException(L"Expected value after sign");
	return exp;
}
bool LexemChecker::checkExp14() // <выр 14> 
{
	int sgn = 0;
	while (checkSign14())
	{
		++sgn;
	}
	bool exp = checkExp15();
	if (sgn && exp)
	{
		TYPE type = popType();
		if (type == INT_TYPE || type == BOOL_TYPE || type == FLOAT_TYPE)
		{
			type.is_adress = 0;
			type_stack.push(type);
			while (sgn--)
			{
				polis_.push_back(popSign());
			}
			return 1;
		}
		else runException(L"Incorrect type in expression");
	}
	if (sgn && !exp)
		runException(L"Expected value after sign");
	return exp;
}
bool LexemChecker::checkExp15() // <выр 15> 
{
	int sgn = 0;
	while (checkSign15())
	{
		++sgn;
	}
	bool exp = checkExp16();
	if (sgn && exp)
	{
		TYPE type = popType(0);
		if (!type.is_adress)
			runException(L"This isn't a variable");
		while (sgn--)
		{
			lexem sign = popSign();
			sign.second += L'a';
			polis_.push_back(sign);
		}
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
bool LexemChecker::checkExp16() // <выр 16> 
{
	if (checkValue())
	{
		TYPE type = popType(0);
		if (type == VOID_TYPE || type == ADRESS_TYPE)
			runException(L"Type '" + type.to_str() + L"' is prohibited in expression");
		if (checkSign16())
		{
			int sgn = 1;
			while (checkSign16())
			{
				++sgn;
			}
			while (sgn--)
			{
				lexem sign = popSign();
				sign.second = L'a' + sign.second;
				polis_.push_back(sign);
			}
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
bool LexemChecker::checkVariable() // <переменная> 
{
	return checkPointer();
}
bool LexemChecker::checkConstant() // <константа> 
{
	return checkNumber() || checkString() || checkLogic();
}
int LexemChecker::checkFuctionCall() // <результат функции> 
{
	int arg_count = 0;
	if (match(7, L"("))
	{
		if (checkExpression())
		{
			++arg_count;
			while (match(7, L","))
			{
				if (!checkExpression())
					runException(L"Expected argument");
				++arg_count;
			}
		}
		if (!match(7, L")"))
			runException(L"Invalid argument");
		if (arg_count == 0)
			arg_count = -1;
		return arg_count;
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
	//if (checkStructure()) return 1; // раскомментить, чтобы вернуть структуры
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
	if (checkExpression())
	{
		popType();
		return 1;
	}
	return 0;
}
bool LexemChecker::checkStreamOperator() // <оператор потока> 
{
	if (match(1, L"in"))
	{
		if (!checkVariable())
			runException(L"Expected variable");
		polis_.push_back({ LEX_OPERATION, L"in" });
		return 1;
	}
	if (match(1, L"out"))
	{
		if (!checkExpression())
			runException(L"Expected expression");
		polis_.push_back({ LEX_OPERATION, L"out" });
		popType();
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
		popType();
		if (!match(7, L")"))
			runException(L"Expected ending bracket");
		int code_line1 = polis_.size();
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"?!" });
		if (!(checkBlock() || checkFreeOperator()))
		{
			pushBlock();
			if (!(checkOperator() && match(7, L";")))
				runException(L"Expected operator");
			popBlock();
		}
		if (match(1, L"else"))
		{
			int code_line2 = polis_.size();
			polis_.push_back({ LEX_INT, L"@" });
			polis_.push_back({ LEX_OPERATION, L"!" });
			polis_[code_line1].second = std::to_wstring(polis_.size());

			if (checkBlock() || checkFreeOperator())
			{
				polis_[code_line2].second = std::to_wstring(polis_.size());
				return 1;
			}
			pushBlock();
			if (!(checkOperator() && match(7, L";")))
				runException(L"Expected operator");
			popBlock();
			polis_[code_line2].second = std::to_wstring(polis_.size());
			return 1;
		}
		polis_[code_line1].second = std::to_wstring(polis_.size());
		return 1;
	}
	return 0;
}
bool LexemChecker::checkWhile() // <оператор while> 
{
	if (match(1, L"while"))
	{
		int break_depth = break_stack.size(), continue_depth = continue_stack.size();
		int code_line1 = polis_.size();

		if (!match(7, L"("))
			runException(L"Expected arguments in brackets");
		if (!checkExpression())
			runException(L"Expected expression");
		popType();
		if (!match(7, L")"))
			runException(L"Expected ending bracket");

		int code_line2 = polis_.size();
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"?!" });

		if (!(checkBlock() || checkFreeOperator()))
		{
			pushBlock();
			if (!(checkOperator() && match(7, L";")))
				runException(L"Expected operator");
			popBlock();
		}
		polis_.push_back({ LEX_INT, std::to_wstring(code_line1) });
		polis_.push_back({ LEX_OPERATION, L"!" });
		polis_[code_line2].second = std::to_wstring(polis_.size());

		while (break_stack.size() > break_depth)
		{
			polis_[break_stack.top()].second = std::to_wstring(polis_.size());
			break_stack.pop();
		}
		while (continue_stack.size() > break_depth)
		{
			polis_[continue_stack.top()].second = std::to_wstring(code_line1);
			continue_stack.pop();
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkFor() // <оператор for> 
{
	if (match(1, L"for"))
	{
		int break_depth = break_stack.size(), continue_depth = continue_stack.size();
		pushBlock();
		if (!match(7, L"("))
			runException(L"Expected arguments in brackets");
		checkForOperator();
		if (!match(7, L";"))
			runException(L"Expected ';'");
		int code_line1 = polis_.size();
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"!" });

		checkForOperator();
		if (!match(7, L";"))
			runException(L"Expected ';'");
		int code_line2 = polis_.size();
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"?!" });
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"!" });

		checkForOperator();
		polis_.push_back({ LEX_INT, std::to_wstring(code_line1+2) });
		polis_.push_back({ LEX_OPERATION, L"!" });

		if (!match(7, L")"))
			runException(L"Expected ending bracket");

		polis_[code_line1].second = std::to_wstring(polis_.size());
		polis_[code_line2+2].second = std::to_wstring(polis_.size());

		if (!(checkBlock() || checkFreeOperator()))
		{
			pushBlock();
			if (!(checkOperator() && match(7, L";")))
				runException(L"LExpected operator");
			popBlock();
		}
		popBlock();
		polis_.push_back({ LEX_INT, std::to_wstring(code_line2 + 4) });
		polis_.push_back({ LEX_OPERATION, L"!" });
		polis_[code_line2].second = std::to_wstring(polis_.size());

		while (break_stack.size() > break_depth)
		{
			polis_[break_stack.top()].second = std::to_wstring(polis_.size());
			break_stack.pop();
		}
		while (continue_stack.size() > break_depth)
		{
			polis_[continue_stack.top()].second = std::to_wstring(code_line2+4);
			continue_stack.pop();
		}
		return 1;
	}
	return 0;
}
bool LexemChecker::checkGoto() // <оператор перехода> 
{
	if (match(1, L"break"))
	{
		break_stack.push(polis_.size());
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"!" });
		return 1;
	}
	if (match(1, L"continue"))
	{
		continue_stack.push(polis_.size());
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"!" });
		return 1;
	}
	if (match(1, L"return"))
	{
		if (!checkExpression())
			runException(L"Expected expression");
		TYPE return_type = tid_tree_->get_return_type();
		if (return_type == NO_TYPE)
			runException(L"This block cannot have return operator");
		tryCast(return_type, popType());
		polis_.push_back({ LEX_OPERATION, L"ret" });
		return 1;
	}
	if (match(1, L"goto"))
	{
		if (!checkName())
			runException(L"Expected identifier");
		goto_stack.push({ polis_.size(), popName() });
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"!" });
		return 1;
	}
	if (match(1, L"label"))
	{
		if (!checkName())
			runException(L"Expected identifier");
		if (checkKnown(popName(0)))
			runException(L"Identifier '" + popName(0) + L"' is already used");
		if (label_map.find(popName(0)) != label_map.end())
			runException(L"Label '" + popName(0) + L"' is already used");
		label_map[popName(0)] = polis_.size();
		pushId(popName(), { TYPES::ADRESS_, 0 });
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
		std::wstring nam = popName(0);
		bool is_temp = tid_tree_->is_template(nam, popType(0));
		if (tid_tree_->is_function(nam))
		{
			if (!tid_tree_->is_current(nam))
				runException(L"Function can be descripted only in the same block");
			if (!is_temp)
				runException(L"Fuction is already descripted");
		}
		TYPE typ = popType();
		if (typ.depth > 0)
			polis_.push_back({ LEX_OPERATION, L"set-array" });
		if (!is_temp)
		{
			pushId(popName(), typ);
			if (match(6, L"="))
			{
				polis_.push_back(tid_tree_->get_adress(nam));
				if (!checkExpression())
					runException(L"Expected expression");
				TYPE exp_type = popType();
				if (tryCast(typ, exp_type))
					popType();
				polis_.push_back({ LEX_OPERATION, L"=" });
				var_type = 1;
			}
			while (match(7, L","))
			{
				if (!checkName())
					runException(L"Expected identifier");
				std::wstring nam2 = popName(0);
				pushId(popName(), typ);
				if (match(6, L"="))
				{
					polis_.push_back(tid_tree_->get_adress(nam2));
					if (!checkExpression())
						runException(L"Expected expression");
					TYPE exp_type = popType();
					if (tryCast(typ, exp_type))
						popType();
					polis_.push_back({ LEX_OPERATION, L"=" });
				}
				var_type = 1;
			}
		}
		if (!var_type && match(7, L"("))
		{
			int code_line1 = polis_.size();
			polis_.push_back({ LEX_INT, L"@" });
			polis_.push_back({ LEX_OPERATION, L"!" });
			TID* last_tid = tid_tree_;
			pushBlock();
			tid_tree_->set_return_type(typ);
			bool has_args = 0;
			std::vector<TYPE>* args = tid_tree_->get_arguments(nam);
			if (args == nullptr)
				runException(L"We've found a bug in checkDescription");
			if (checkType())
			{
				has_args = 1;
				TYPE first_type = popType();
				if (is_temp)
				{
					if (args->size() == 0)
						runException(L"Too many arguments in description");
					if (*(args->begin()) != first_type)
						runException(L"Argument 1 has wrong type");
				}
				else args->push_back(first_type);
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
					last_tid->push_code(nam, polis_.size());
					std::vector<std::wstring> new_names;

					if (checkKnown(popName(0)))
						runException(L"Identifier '" + popName(0) + L"' is already used");
					new_names.push_back(popName(0));
					pushId(popName(), first_type);

					int temp_step = 1;
					while (match(7, L","))
					{
						if (!checkType())
							runException(L"Expected type");
						if (is_temp)
						{
							if (temp_step >= args->size())
								runException(L"Too many arguments in description");
							if (*(args->begin() + temp_step) != popType(0))
								runException(L"Argument " + std::to_wstring(temp_step + 1) + L" has wrong type");
						}
						else args->push_back(popType(0));
						++temp_step;
						if (!checkName())
							runException(L"Expected identifier");
						if (std::find(new_names.begin(), new_names.end(), popName(0)) != new_names.end())
							runException(L"Argument '" + popName(0) + L"' is already used");
						if (checkKnown(popName(0)))
							runException(L"Identifier '" + popName(0) + L"' is already used");
						new_names.push_back(popName(0));
						pushId(popName(), popType());
					}
					if (is_temp && temp_step < args->size())
						runException(L"Too few arguments in description");
					if (!match(7, L")"))
						runException(L"Expected ending bracket");
					if (!checkBlock())
						runException(L"Expected description");
					polis_.push_back({ LEX_UNKNOWN, L"" });
					polis_.push_back({ LEX_OPERATION, L"ret" });
					while (!new_names.empty())
					{
						polis_.push_back(tid_tree_->get_adress(new_names.back()));
						polis_.push_back({ LEX_OPERATION, L"arg=" });
					}
					popBlock();
					polis_[code_line1].second = std::to_wstring(polis_.size());
					return 1;
				}
			}
			if (!match(7, L")"))
				runException(L"Expected ending bracket");
			if (!var_type && !has_args)
			{
				int code_line2 = polis_.size();
				if (checkBlock())
					last_tid->push_code(nam, code_line2);
				else if (is_temp)
					runException(L"Expected description");
			}
			popBlock();
			pushFunctionDefault(typ);
			polis_.push_back({ LEX_OPERATION, L"ret" });
			polis_[code_line1].second = std::to_wstring(polis_.size());
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

bool LexemChecker::checkKnown(std::wstring name)
{
	return tid_tree_->get_type(name) != NO_TYPE;
}

bool LexemChecker::tryCast(TYPE to_type, TYPE from_type)
{
	if (to_type.depth > 0 || from_type.depth > 0)
	{
		if (to_type == from_type)
		{
			type_stack.push(to_type);
			return 1;
		}
		std::wstring excep_text = L"Cannot convert type '" + from_type.to_str();
		excep_text += L"' to '" + to_type.to_str() + L"'";
		runException(excep_text);
	}
	if (to_type != STRING_TYPE && to_type != BOOL_TYPE && from_type == STRING_TYPE)
	{
		std::wstring excep_text = L"Cannot convert type '" + from_type.to_str();
		excep_text += L"' to '" + to_type.to_str() + L"'";
		runException(excep_text);
	}
	type_stack.push(to_type);
	return 1;
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
	if (type == 6)
		sign_stack.push(term_[p]);
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

lexem LexemChecker::popSign(bool pop)
{
	if (sign_stack.empty())
		runException(L"We've found a bug in popSign");
	auto res = sign_stack.top();
	if (pop) sign_stack.pop();
	return res;
}

void LexemChecker::runException(std::wstring reason)
{
	std::wcout << L"Syntax Error: " << reason << ". Lexem: " << p << '\n';
	exit(0);
}
