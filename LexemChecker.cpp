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
	tid_tree_->set_return_type(VOID_TYPE);
	polis_.push_back({ LEX_INT, std::to_wstring(push_func_memory()) });
	polis_.push_back({ LEX_OPERATION, L"push-mem" });
	checkOperators();
	if (p != term_.size())
		runException(p, L"Expected operator", L"Хотелось оператор");
	popBlock();
	polis_.push_back({ LEX_INT, std::to_wstring(pop_func_memory()) });
	polis_.push_back({ LEX_OPERATION, L"pop-mem" });
	if (break_stack.size() > 0)
		runException(p, L"Operator 'break' must be used in cycle", L"Оператор 'break' вне цикла? Ясно");
	if (continue_stack.size() > 0)
		runException(p, L"Operator 'continue' must be used in cycle", L"Оператор 'continue' вне цикла? Ясно");
	while (goto_stack.size())
	{
		if (label_map.find(goto_stack.top().second) == label_map.end())
			runException(p, 
				L"Label '" + goto_stack.top().second + L"' must be defined",
				L"Метка '" + goto_stack.top().second + L"' должна быть установлена"
			);
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
			runException(p, L"Expected ending bracket", L"Тут могла быть ещё скобка");
		popBlock();
		return 1;
	}
	return 0;
}
bool LexemChecker::checkUnsigned() // <беззнак> 
{
	if (isEnd()) return 0;
	if (type(p) == 0)
		runException(p, L"Unexpected symbol", L"Чё за " + value(p) + L"? Непонятно");
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
	if (match(1, L"in"))
	{
		type_stack.push(TYPE(TYPES::STRING_));
		polis_.push_back({ LEX_OPERATION, L"in" });
		return 1;
	}
	return 0;
}
bool LexemChecker::checkChar()
{
	if (isEnd()) return 0;
	if (type(p) == 8)
	{
		type_stack.push(TYPE(TYPES::CHAR_));
		polis_.push_back({ LEX_CHAR, term_[p].second });
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
			runException(p, 
				L"Unknown identifier '" + nam + L"'",
				L"Неизвестный науке идентификатор '" + nam + L"'"
			);

		TYPE typ = tid_tree_->get_type(nam);
		int arg_count = checkFuctionCall();
		if (arg_count)
		{
			if (arg_count == -1)
				arg_count = 0;
			if (!tid_tree_->is_function(nam))
				runException(p, L"Identifier isn't a function", L"Увы, идентификатор не похож на функцию");
			std::vector<TYPE> * args = tid_tree_->get_arguments(nam);
			if (args == nullptr)
				runException(p, L"We have a bug in checkPointer", L"Ой, а это баг");
			if (arg_count < args->size())
				runException(p, L"Too few arguments to function", L"Нужно больше аргументов! ._");
			bool too_args = arg_count > args->size();
			while (arg_count > args->size())
			{
				--arg_count;
				popType();
			}
			for (int i = args->size() - 1; i >= 0; --i)
			{
				tryCast(*(args->begin() + i), popType());
				popType();
			}
			if (too_args)
				runException(p, L"Too many arguments in function", L"Нужно меньше аргументов! ._");
			int code_line = tid_tree_->get_code(nam);
			if (code_line == -1)
			{
				template_calls[nam].push_back(polis_.size());
			}
			polis_.push_back({ LEX_INT, std::to_wstring(code_line) });
			polis_.push_back({ LEX_OPERATION, L"push-stack" });
			polis_.push_back({ LEX_OPERATION, L"!!" });
		}
		else
		{
			if (tid_tree_->is_function(nam))
				runException(p, L"Required function call", L"Нужно вызвать эту функцию! ._");
			polis_.push_back(tid_tree_->get_adress(nam));
		}
		while (checkShift())
		{
			if (typ == STRING_TYPE)
			{
				typ = CHAR_TYPE;
				continue;
			}
			if (typ.depth == 0)
				runException(p, L"Invalid type for array subscript", L"Это плохой пример массива");
			--typ.depth;
		}

		/*if (match(7, L".")) // раскомментить, чтобы включить структуры
		{
			if (checkPointer())
				return 1;
			runException(p, L"Invalid variable path\n");
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
			runException(p, L"Invalid identifier", L"Слишком некрасивое имя");
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
	else if (match(1, L"char"))
		typ = TYPES::CHAR_;
	else if (match(1, L"void"))
		typ = TYPES::VOID_;
	else return 0;
	while (checkShift(true)) ++array_depth;
	if (array_depth > 0)
	{
		if (typ == TYPES::VOID_)
			runException(p, L"Type 'void' mustn't be in array", L"Тип 'void' недостоин быть элементом массива");
		polis_.push_back({ LEX_INT, std::to_wstring(array_depth) });
	}
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
		runException(p, L"Incorrect Array's index", L"Неверный индекс массива(банально, не правда ли?)");
	}
	return 0;
}

bool LexemChecker::checkSign1() // <знак 1> 
{
	return match(6, L"=") || match(6, L"+=") || match(6, L"-=") || match(6, L"*=") || match(6, L"%=")
		|| match(6, L"/=") || match(6, L"|=") || match(6, L"&=") || match(6, L"^=");
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
			if (!checkExpression())
				runException(p, L"Expected value in expression", L"Хочу операнд в выражения");
			TYPE type2 = popType();
			TYPE type1 = popType();
			if (!type1.is_adress)
				runException(p, L"Left value required as left operand of assignment", L"Хей! Хочу изменяемое левостороннее значение");
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
				runException(p, L"Expected value in expression", L"Хочу операнд в выражения");
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
			if (type1 == VOID_TYPE || type2 == VOID_TYPE)
				runException(p, L"Type 'void' mustn't be in expression", L"Тип 'void' недостоин использоваться в выражении");
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
				runException(p, L"Expected value in expression", L"Хочу операнд в выражения");
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
			if (type1 == VOID_TYPE || type2 == VOID_TYPE)
				runException(p, L"Type 'void' mustn't be in expression", L"Тип 'void' недостоин использоваться в выражении");
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
				runException(p, L"Expected value in expression", L"Хочу операнд в выражения");
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
			TYPE exp_type = BOOL_TYPE;
			if (type1 == VOID_TYPE || type2 == VOID_TYPE)
				runException(p, L"Type 'void' mustn't be in expression", L"Тип 'void' недостоин использоваться в выражении");
			if (type1 == FLOAT_TYPE || type1 == STRING_TYPE)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
			if (type2 == FLOAT_TYPE || type2 == STRING_TYPE)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
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
				runException(p, L"Expected value in expression", L"Хочу операнд в выражения");
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
			if (type1 == VOID_TYPE || type2 == VOID_TYPE)
				runException(p, L"Type 'void' mustn't be in expression", L"Тип 'void' недостоин использоваться в выражении");
			TYPE exp_type = BOOL_TYPE;
			if (type1 == FLOAT_TYPE || type1 == STRING_TYPE)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
			if (type2 == FLOAT_TYPE || type2 == STRING_TYPE)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
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
				runException(p, L"Expected value in expression", L"Хочу операнд в выражения");
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1.depth > 0 || type2.depth > 0)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
			if (type1 == VOID_TYPE || type2 == VOID_TYPE)
				runException(p, L"Type 'void' mustn't be in expression", L"Тип 'void' недостоин использоваться в выражении");
			TYPE exp_type = BOOL_TYPE;
			if (type1 == FLOAT_TYPE || type1 == STRING_TYPE)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
			if (type2 == FLOAT_TYPE || type2 == STRING_TYPE)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
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
				runException(p, L"Expected value in expression", L"Хочу операнд в выражения");
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1 == VOID_TYPE || type2 == VOID_TYPE)
				runException(p, L"Type 'void' mustn't be in expression", L"Тип 'void' недостоин использоваться в выражении");
			if (type1.depth > 0 || type2.depth > 0)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
			if ((type1 == STRING_TYPE) != (type2 == STRING_TYPE))
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
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
				runException(p, L"Expected value in expression", L"Хочу операнд в выражения");
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1 == VOID_TYPE || type2 == VOID_TYPE)
				runException(p, L"Type 'void' mustn't be in expression", L"Тип 'void' недостоин использоваться в выражении");
			if (type1.depth > 0 || type2.depth > 0)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
			if ((type1 == STRING_TYPE) != (type2 == STRING_TYPE))
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
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
				runException(p, L"Expected value in expression", L"Хочу операнд в выражения");
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1 == BOOL_TYPE)
				type1 = INT_TYPE;
			if (type2 == BOOL_TYPE)
				type2 = INT_TYPE;
			if (type1 == VOID_TYPE || type2 == VOID_TYPE)
				runException(p, L"Type 'void' mustn't be in expression", L"Тип 'void' недостоин использоваться в выражении");
			if (type1 != INT_TYPE || type2 != INT_TYPE)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
			if (type1.depth > 0 || type2.depth > 0)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
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
				runException(p, L"Expected value in expression", L"Хочу операнд в выражения");
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1 == VOID_TYPE || type2 == VOID_TYPE)
				runException(p, L"Type 'void' mustn't be in expression", L"Тип 'void' недостоин использоваться в выражении");
			if (type1.depth > 0 || type2.depth > 0)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
			if (type1 == STRING_TYPE || type2 == STRING_TYPE)
			{
				if (popSign(0).second == L"-")
					runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
				type_stack.push(STRING_TYPE);
				polis_.push_back(popSign());
				continue;
			}

			TYPE exp_type = INT_TYPE;
			if (type1 == CHAR_TYPE)
				exp_type = CHAR_TYPE;
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
				runException(p, L"Expected value in expression", L"Хочу операнд в выражения");
			TYPE type2 = popType();
			TYPE type1 = popType();
			type1.is_adress = 0;
			type2.is_adress = 0;
			if (type1 == VOID_TYPE || type2 == VOID_TYPE)
				runException(p, L"Type 'void' mustn't be in expression", L"Тип 'void' недостоин использоваться в выражении");
			if (type1 == STRING_TYPE || type2 == STRING_TYPE)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
			TYPE exp_type = INT_TYPE;
			if (type1.depth > 0 || type2.depth > 0)
				runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
			if (type1 == FLOAT_TYPE || type2 == FLOAT_TYPE)
			{
				if (popSign(0).second == L"%")
					runException(p, L"Incompatible types in expression", L"Эти типы несовместимы в выражении");
				exp_type = FLOAT_TYPE;
			}
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
		else if (type == INT_TYPE || type == CHAR_TYPE)
			type_stack.push(BOOL_TYPE);
		else runException(p, L"Incorrect type in expression", L"Неподходящий тип операнда для выражения");
		while (sgn--)
		{
			polis_.push_back(popSign());
		}
		return 1;
	}
	if (sgn && !exp)
		runException(p, L"Expected value after sign", L"И чо я тут жду выражение после знака? ._");
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
		if (type == INT_TYPE || type == BOOL_TYPE || type == CHAR_TYPE)
		{
			type.is_adress = 0;
			type_stack.push(type);
			while (sgn--)
			{
				polis_.push_back(popSign());
			}
			return 1;
		}
		else runException(p, L"Incorrect type in expression", L"Неподходящий тип операнда для выражения");
	}
	if (sgn && !exp)
		runException(p, L"Expected value after sign", L"");
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
		if (type == INT_TYPE || type == BOOL_TYPE || type == FLOAT_TYPE || type == CHAR_TYPE)
		{
			type.is_adress = 0;
			type_stack.push(type);
			while (sgn--)
			{
				lexem lexemToPush = popSign ();
				lexemToPush.second += L"!";
				polis_.push_back(lexemToPush);
			}
			return 1;
		}
		else runException(p, L"Incorrect type in expression", L"Неподходящий тип операнда для выражения");
	}
	if (sgn && !exp)
		runException(p, L"Expected value after sign", L"После знака всегда идёт выражение, малыш:) ._");
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
			runException(p, L"This isn't a variable", L"Прошу подать переменную");
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
		if (type == CHAR_TYPE)
			return 1;
		runException(p, L"Incorrect type in expression", L"Неподходящий тип операнда для выражения");
	}
	if (sgn && !exp)
		runException(p, L"Expected value after sign", L"После знака всегда идёт выражение, малыш:) ._");
	return exp;
}
bool LexemChecker::checkExp16() // <выр 16> 
{
	if (checkValue())
	{
		TYPE type = popType(0);
		if (checkSign16())
		{
			if (type == VOID_TYPE)
				runException(p, 
					L"Type '" + type.to_str() + L"' mustn't be in expression",
					L"Тип '" + type.to_str() + L"' недостоин этого выражения"
				);
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
				runException(p, L"This isn't a variable", L"Это не переменная. Подайте переменную");
			if (type == INT_TYPE)
				return 1;
			if (type == BOOL_TYPE)
				return 1;
			if (type == FLOAT_TYPE)
				return 1;
			if (type == CHAR_TYPE)
				return 1;
			runException(p, L"Incorrect type in expression", L"Неподходящий тип операнда для выражения");
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
	return checkNumber() || checkString() || checkChar() || checkLogic();
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
					runException(p, L"Expected argument", L"Хочу аргумент");
				++arg_count;
			}
		}
		if (!match(7, L")"))
			runException(p, L"Invalid argument", L"Просто неверный аргумент");
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
	TYPE cast = NO_TYPE;
	if (term_.size() > p + 1 && type(p) == 1)
	{
		if (term_[p+1].second == L"(")
		{
			if (checkType())
			{
				cast = popType();
				if (cast.depth > 0)
					runException(p, L"Value mustn't be casted to array", L"Хочешь приводить к массивам? Тогда приводи сам ");

			}
		}
	}
	if (match(7, L"("))
	{
		if (!checkExpression())
			runException(p, L"Expected expession", L"Ку-ку! Тут надо выражение");
		if (!match(7, L")"))
			runException(p, L"Expected ending bracket", L"Уходя, всегда закрывайте скобки");
		if (cast != NO_TYPE)
		{
			TYPE type = popType();
			type_stack.push(cast);
			if (type.depth > 0 && (type.basic != CHAR_ || type.depth != 1 || cast != STRING_TYPE))
				runException(p, L"Array mustn't be casted into another type", L"Массивы ни к какому типу не приводятся");
			polis_.push_back({ LEX_INT, std::to_wstring(cast.basic) });
			polis_.push_back({ LEX_OPERATION, L"cast" });
		}
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
				runException(p, L"Excepted ';'", L"Оооо да... Я даже не буду подсказывать тебе, где ошибка..");
			isop = 1;
		}
	} while (isop);
	return 1;
}
bool LexemChecker::checkExpOperator() // <оператор выражения> 
{
	if (checkExpression())
	{
		polis_.push_back({ LEX_OPERATION, L"forget" });
		popType();
		return 1;
	}
	return 0;
}
bool LexemChecker::checkStreamOperator() // <оператор потока> 
{
	/*if (match(1, L"in")) // синтаксис in изменён
	{
		if (!checkVariable())
			runException(p, L"Expected variable");
		polis_.push_back({ LEX_OPERATION, L"in" });
		return 1;
	}*/
	if (match(1, L"out"))
	{
		if (!checkExpression())
			runException(p, L"Expected expression", L"Подайте выражение");
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
			runException(p, L"Expected arguments in brackets", L"Мечталось об условии в скобках");
		if (!checkExpression())
			runException(p, L"Expected expression", L"Подайте выражение");
		popType();
		if (!match(7, L")"))
			runException(p, L"Expected ending bracket", L"Уходя, всегда закрывайте скобки");
		int code_line1 = polis_.size();
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"?!" });
		if (!(checkBlock() || checkFreeOperator()))
		{
			pushBlock();
			if (!(checkOperator() && match(7, L";")))
				runException(p, L"Expected operator", L"Оператор зело нужон тут");
			popBlock();
		}
		if (match(1, L"else"))
		{
			int code_line2 = polis_.size();
			polis_.push_back({ LEX_INT, L"@" });
			polis_.push_back({ LEX_OPERATION, L"!!" });
			polis_[code_line1].second = std::to_wstring(polis_.size());

			if (checkBlock() || checkFreeOperator())
			{
				polis_[code_line2].second = std::to_wstring(polis_.size());
				return 1;
			}
			pushBlock();
			if (!(checkOperator() && match(7, L";")))
				runException(p, L"Expected operator", L"Оператор зело нужон тут");
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
			runException(p, L"Expected arguments in brackets", L"Мечталось о чудном условии в скобках");
		if (!checkExpression())
			runException(p, L"Expected expression", L"Подайте выражение");
		popType();
		if (!match(7, L")"))
			runException(p, L"Expected ending bracket", L"Уходя, всегда закрывайте скобки");

		int code_line2 = polis_.size();
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"?!" });

		if (!(checkBlock() || checkFreeOperator()))
		{
			pushBlock();
			if (!(checkOperator() && match(7, L";")))
				runException(p, L"Expected operator", L"Оператор зело нужон тут");
			popBlock();
		}
		polis_.push_back({ LEX_INT, std::to_wstring(code_line1) });
		polis_.push_back({ LEX_OPERATION, L"!!" });
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
			runException(p, L"Expected arguments in brackets", L"Агрументы в скобках! Это ж фор, камон");
		checkForOperator();
		if (!match(7, L";"))
			runException(p, L"Expected ';'", L"Оооо да... Я даже не буду подсказывать тебе, где ошибка..");
		int code_line1 = polis_.size();
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"!!" });

		checkExpression();
		if (!match(7, L";"))
			runException(p, L"Expected ';'", L"Оооо да... Я даже не буду подсказывать тебе, где ошибка..");
		int code_line2 = polis_.size();
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"?!" });
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"!!" });

		checkForOperator();
		polis_.push_back({ LEX_INT, std::to_wstring(code_line1+2) });
		polis_.push_back({ LEX_OPERATION, L"!!" });

		if (!match(7, L")"))
			runException(p, L"Expected ending bracket", L"Уходя, всегда закрывайте скобки");

		polis_[code_line1].second = std::to_wstring(polis_.size());
		polis_[code_line2+2].second = std::to_wstring(polis_.size());

		if (!(checkBlock() || checkFreeOperator()))
		{
			pushBlock();
			if (!(checkOperator() && match(7, L";")))
				runException(p, L"LExpected operator", L"Да где опять оператор то? ._");
			popBlock();
		}
		popBlock();
		polis_.push_back({ LEX_INT, std::to_wstring(code_line2 + 4) });
		polis_.push_back({ LEX_OPERATION, L"!!" });
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
		polis_.push_back({ LEX_OPERATION, L"!!" });
		return 1;
	}
	if (match(1, L"continue"))
	{
		continue_stack.push(polis_.size());
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"!!" });
		return 1;
	}
	if (match(1, L"return"))
	{
		TYPE return_type = tid_tree_->get_return_type();
		if (return_type == VOID_TYPE)
		{
			if (checkExpression())
				runException(p, L"Void function cannot return any value", L"Вообще-то данная функция не возвращает значений");
			polis_.push_back({ LEX_INT, L"0" });
		}
		else
		{
			if (!checkExpression())
				runException(p, L"Expected expression", L"Подайте выражение");
			if (return_type == NO_TYPE)
				runException(p, L"This block cannot have return operator", L"В этом блоке не может быть return");
			tryCast(return_type, popType());
			popType();
			polis_.push_back(tid_tree_->get_return_adress());
			polis_.push_back({ LEX_OPERATION, L"arg=" });
		}
		tid_tree_->push_return_line(polis_.size());
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"!!" });
		return 1;
	}
	if (match(1, L"goto"))
	{
		if (!checkName())
			runException(p, L"Expected identifier", L"А хотелось сюда индентификатор..");
		goto_stack.push({ polis_.size(), popName() });
		polis_.push_back({ LEX_INT, L"@" });
		polis_.push_back({ LEX_OPERATION, L"!!" });
		return 1;
	}
	if (match(1, L"label"))
	{
		if (!checkName())
			runException(p, L"Expected identifier", L"А хотелось сюда индентификатор..");
		if (checkKnown(popName(0)))
			runException(p, 
				L"Identifier '" + popName(0) + L"' is already used",
				L"Имя '" + popName(0) + L"' уже кто-то занял"
			);
		if (label_map.find(popName(0)) != label_map.end())
			runException(p, 
				L"Label '" + popName(0) + L"' is already used",
				L"Метка '" + popName(0) + L"' уже используется"
			);
		label_map[popName(0)] = polis_.size();
		pushId(popName(), { TYPES::VOID_, 0 });
		return 1;
	}
	return 0;
}
bool LexemChecker::checkDescription() // <описание> 
{
	if (checkType())
	{
		if (!checkName())
			runException(p, L"Expected identifier", L"А хотелось сюда индентификатор..");
		std::vector<std::wstring> array_ids;
		int array_line;
		int var_type = 0;
		std::wstring nam = popName(0);
		bool is_temp = tid_tree_->is_template(nam, popType(0));
		if (tid_tree_->is_function(nam))
		{
			if (!tid_tree_->is_current(nam))
				runException(p, 
					L"Function can be descripted only in the same block",
					L"Да опишут глупцы функцию и её шаблон в разных блоках! А мудрецы рассмеются и напишут эту ошибку"
				);
			if (!is_temp)
				runException(p, L"Fuction is already descripted", L"Функция уже была описана");
		}
		TYPE typ = popType();
		if (typ.depth > 0)
		{
			array_line = polis_.size();
			polis_.push_back({ LEX_STRING, L"" });
			polis_.push_back({ LEX_INT, L"@" });
			polis_.push_back({ LEX_INT, L"@" });
			polis_.push_back({ LEX_OPERATION, L"set-arrays" });
		}
		if (!is_temp)
		{
			if (match(6, L"="))
			{
				int code_line = polis_.size();
				polis_.push_back({});
				if (!checkExpression())
					runException(p, L"Expected expression", L"Подайте выражение");
				TYPE exp_type = popType();
				tryCast(typ, exp_type);
				popType();
				polis_.push_back({ LEX_OPERATION, L"=" });
				polis_.push_back({ LEX_OPERATION, L"forget" });
				var_type = 1;
				pushId(popName(), typ);
				polis_[code_line] = tid_tree_->get_adress(nam);
			}
			else pushId(popName(), typ);
			if (typ.depth > 0)
				array_ids.push_back(tid_tree_->get_adress(nam).second);
			while (match(7, L","))
			{
				if (var_type == 0)
					polis_.push_back(tid_tree_->get_adress(nam));
				if (!checkName())
					runException(p, L"Expected identifier", L"А хотелось сюда индентификатор..");
				std::wstring nam2 = popName(0);
				if (match(6, L"="))
				{
					int code_line = polis_.size();
					polis_.push_back({});
					if (!checkExpression())
						runException(p, L"Expected expression", L"Подайте выражение");
					TYPE exp_type = popType();
					tryCast(typ, exp_type);
					popType();
					pushId(popName(), typ);
					polis_[code_line] = tid_tree_->get_adress(nam2);
					polis_.push_back({ LEX_OPERATION, L"=" });
					polis_.push_back({ LEX_OPERATION, L"forget" });
				}
				else pushId(popName(), typ);
				if (typ.depth > 0)
					array_ids.push_back(tid_tree_->get_adress(nam2).second);
				var_type = 1;
			}
		}
		if (typ.depth > 0)
		{
			polis_[array_line + 1].second = std::to_wstring(array_ids.size());
			for (std::wstring id : array_ids) {
				polis_[array_line].second += id + L" ";
			}
			polis_[array_line + 2].second = std::to_wstring(typ.basic);
		}
		if (!var_type && match(7, L"("))
		{
			tid_tree_->set_is_adress(nam, 0);
			TID* last_tid = tid_tree_;
			pushBlock();
			tid_tree_->set_return_type(typ);
			tid_tree_->set_return_adress(last_tid->get_adress(nam));
			bool has_args = 0;
			std::vector<TYPE>* args = tid_tree_->get_arguments(nam);
			if (args == nullptr)
				runException(p, L"We've found a bug in checkDescription", L"О-па, у нас тут баги");
			if (checkType())
			{
				has_args = 1;
				TYPE first_type = popType();
				if (first_type.depth > 0)
					runException(p, L"Array mustn't be an argument in function", L"Нельзя пихать массив в функцию! Это не культурно");
				if (is_temp)
				{
					if (args->size() == 0)
						runException(p, L"Too many arguments in description", L"Слииииишком много аргументов");
					if (*(args->begin()) != first_type)
						runException(p, L"Argument 1 has wrong type", L"У аргумента 1 не тот тип");
				}
				else args->push_back(first_type);
				while (match(7, L","))
				{
					var_type = 2;
					if (is_temp)
						runException(p, L"Only one template can be created", L"Если вы написали два шаблона одной функции, вам только что об этом напомнили");
					if (!checkType())
						runException(p, L"Expected type", L"Да я капирзный, но тут нужен именно тип");
					args->push_back(popType());
				}
				if (!var_type && checkName())
				{
					var_type = 3;
					int code_line1 = polis_.size();
					polis_.push_back({ LEX_INT, L"@" });
					polis_.push_back({ LEX_OPERATION, L"!!" });
					last_tid->push_code(nam, polis_.size());

					polis_.push_back({ LEX_INT, std::to_wstring(push_func_memory()) });
					polis_.push_back({ LEX_OPERATION, L"push-mem" });
					std::vector<std::wstring> new_names;

					if (checkKnown(popName(0)))
						runException(p, 
							L"Identifier '" + popName(0) + L"' is already used",
							L"Имя '" + popName(0) + L"' уже у кого-то в ходу"
						);
					new_names.push_back(popName(0));
					pushId(popName(), first_type);

					int temp_step = 1;
					while (match(7, L","))
					{
						if (!checkType())
							runException(p, L"Expected type", L"Да я капирзный, но тут нужен именно тип");
						if (is_temp)
						{
							if (temp_step >= args->size())
								runException(p, L"Too many arguments in description", L"");
							if (*(args->begin() + temp_step) != popType(0))
								runException(p, 
									L"Argument " + std::to_wstring(temp_step + 1) + L" has wrong type",
									L"У аргумента " + std::to_wstring(temp_step + 1) + L" несварение типов в функции и шаблоне"
								);
						}
						else args->push_back(popType(0));
						++temp_step;
						if (!checkName())
							runException(p, L"Expected identifier", L"А хотелось сюда индентификатор..");
						if (std::find(new_names.begin(), new_names.end(), popName(0)) != new_names.end())
							runException(p, 
								L"Argument '" + popName(0) + L"' is already used",
								L"Аргумент '" + popName(0) + L"' уже указан"
							);
						if (checkKnown(popName(0)))
							runException(p, 
								L"Identifier '" + popName(0) + L"' is already used",
								L"Имя '" + popName(0) + L"' уже у кого-то в ходу"
							);
						new_names.push_back(popName(0));
						if (popType(0).depth > 0)
							runException(p, 
								L"Array mustn't be an argument in function",
								L"Нельзя пихать массив в функцию! Это не культурно"
							);
						pushId(popName(), popType());
					}
					if (is_temp && temp_step < args->size())
						runException(p, L"Too few arguments in description", L"Нужно больше аргументов! ._");
					if (!match(7, L")"))
						runException(p, L"Expected ending bracket", L"Уходя, всегда закрывайте скобки");
					while (!new_names.empty())
					{
						polis_.push_back(tid_tree_->get_adress(new_names.back()));
						polis_.push_back({ LEX_OPERATION, L"arg=" });
						polis_.push_back({ LEX_OPERATION, L"forget" });
						new_names.pop_back();
					}
					if (!checkBlock())
						runException(p, L"Expected description", L"А описание?? ._");
					if (template_calls.find(nam) != template_calls.end()) {
						for (int line : template_calls[nam]) {
							polis_[line].second = std::to_wstring(code_line1+2);
						}
					}
					pushFunctionDefault(typ);
					if (tid_tree_->get_return_type() == VOID_TYPE)
						polis_.push_back({ LEX_INT, L"0" });
					else
					{
						polis_.push_back(tid_tree_->get_return_adress());
						polis_.push_back({ LEX_OPERATION, L"arg=" });
					}
					//polis_.push_back(tid_tree_->get_adress(nam));
					std::vector<int>& return_lines = *tid_tree_->get_return_lines();
					for (int line : return_lines) {
						polis_[line].second = std::to_wstring(polis_.size());
					}
					polis_.push_back({ LEX_INT, std::to_wstring(pop_func_memory()) });
					polis_.push_back({ LEX_OPERATION, L"pop-mem" });
					polis_.push_back({ LEX_OPERATION, L"pop-stack" });
					polis_.push_back({ LEX_OPERATION, L"!!" });

					popBlock();
					polis_[code_line1].second = std::to_wstring(polis_.size());
					return 1;
				}
			}
			if (!match(7, L")"))
				runException(p, L"Expected ending bracket", L"Уходя, всегда закрывайте скобки");
			if (!var_type && !has_args)
			{
				int code_line2 = polis_.size();
				polis_.push_back({ LEX_INT, L"@" });
				polis_.push_back({ LEX_OPERATION, L"!!" });
				polis_.push_back({ LEX_INT, std::to_wstring(push_func_memory()) });
				polis_.push_back({ LEX_OPERATION, L"push-mem" });
				if (checkBlock())
				{
					if (template_calls.find(nam) != template_calls.end()) {
						for (int line : template_calls[nam]) {
							polis_[line].second = std::to_wstring(code_line2+2);
						}
					}
					pushFunctionDefault(typ);
					if (tid_tree_->get_return_type() == VOID_TYPE)
						polis_.push_back({ LEX_INT, L"0" });
					else
					{
						polis_.push_back(tid_tree_->get_return_adress());
						polis_.push_back({ LEX_OPERATION, L"arg=" });
					}
					//polis_.push_back(tid_tree_->get_adress(nam));
					last_tid->push_code(nam, code_line2+2);
					std::vector<int>& return_lines = *tid_tree_->get_return_lines();
					for (int line : return_lines) {
						polis_[line].second = std::to_wstring(polis_.size());
					}
					polis_.push_back({ LEX_INT, std::to_wstring(pop_func_memory()) });
					polis_.push_back({ LEX_OPERATION, L"pop-mem" });
					polis_.push_back({ LEX_OPERATION, L"pop-stack" });
					polis_.push_back({ LEX_OPERATION, L"!!" });
					polis_[code_line2].second = std::to_wstring(polis_.size());
				}
				else if (is_temp)
					runException(p, L"Expected description", L"Ох уж эти описания. Они так необходимы компилятору, наверное");
				else polis_.pop_back(), polis_.pop_back();
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
		polis_.push_back({ LEX_OPERATION, L"forget" });
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
			runException(p, L"Expected identifier", L"А хотелось сюда индентификатор..");
		if (!checkBlock())
			runException(p, L"Expected description", L"Ох уж эти описания. Они так необходимы компилятору, наверное");
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
		if (from_type == STRING_TYPE && to_type.basic == CHAR_ && to_type.depth == 1)
		{
			type_stack.push(to_type);
			return 1;
		}
		if (to_type == STRING_TYPE && from_type.basic == CHAR_ && from_type.depth == 1)
		{
			type_stack.push(to_type);
			return 1;
		}
		std::wstring excep_text = L"Cannot convert type '" + from_type.to_str();
		excep_text += L"' to '" + to_type.to_str() + L"'";
		std::wstring excep_text_rus = L"Невозможно из типа '" + from_type.to_str();
		excep_text += L"' сделать тип '" + to_type.to_str() + L"'";
		runException(p, excep_text, excep_text_rus);
	}
	if (to_type == STRING_TYPE || to_type == BOOL_TYPE || to_type == CHAR_TYPE || from_type != STRING_TYPE)
	{
		if (to_type != VOID_TYPE && from_type != VOID_TYPE)
		{
			type_stack.push(to_type);
			return 1;
		}
		runException(p, 
			L"Do not use type 'void' in expressions. It doesn't have a value", 
			L"То, что void в принципе не имеет никакого значения, не остановлио вас от желания запихнуть его в выражение");
	}
	std::wstring excep_text = L"Cannot convert type '" + from_type.to_str();
	excep_text += L"' to '" + to_type.to_str() + L"'";
	std::wstring excep_text_rus = L"Невозможно из типа '" + from_type.to_str();
	excep_text += L"' сделать тип '" + to_type.to_str() + L"'";
	runException(p, excep_text, excep_text_rus);
	return 0;
}

bool LexemChecker::match(int type, std::wstring word)
{

	if (isEnd()) return 0;
	if (term_[p].first == 0)
		runException(p, L"Unexpected symbol", L"Что за лексему вы мне подсунули? ._");
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
		runException(p, 
			L"Template '" + temp + L"' must have definition in the same block",
			L"К шаблону функции '" + temp + L"' обязано быть определение в том же блоке"
		);
	if (tid_tree_->parent != nullptr)
		tid_tree_ = tid_tree_->parent;
}

void LexemChecker::pushId(std::wstring name, TYPE type)
{
	type.is_adress = 1;
	if (!tid_tree_->push_id(name, type))
		runException(p, 
			L"Identifier '" + name + L"' is already used",
			L"Имя '" + name + L"'  уже кто-то занял"
		);
}

std::wstring LexemChecker::popName(bool pop)
{
	if (name_stack.empty())
		runException(p, L"We've found a bug in popName", L"Тащите дихлофос! ._");
	auto res = name_stack.top();
	if (pop) name_stack.pop();
	return res;
}

TYPE LexemChecker::popType(bool pop)
{
	if (type_stack.empty())
		runException(p, L"We've found a bug in popName", L"Тащите дихлофос! ._");
	auto res = type_stack.top();
	if (pop) type_stack.pop();
	return res;
}

lexem LexemChecker::popSign(bool pop)
{
	if (sign_stack.empty())
		runException(p, L"We've found a bug in popSign", L"Тащите дихлофос! ._");
	auto res = sign_stack.top();
	if (pop) sign_stack.pop();
	return res;
}