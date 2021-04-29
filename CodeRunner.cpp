#include "CodeRunner.h"



CodeRunner::CodeRunner()
{
}


CodeRunner::~CodeRunner()
{
}

void CodeRunner::pushValue(value val)
{
	runtime_stack.push(val);
}

void CodeRunner::pushValue(lexem& val)
{
	int basic = val.first & ~(LEX_ADRESS | LEX_ARRAY);
	
	if (val.first & LEX_ADRESS)
	{
		int adress = _wtoi(val.second.c_str());
		std::any item;
		TYPE type(basic, 0, 1);
		if (val.first & LEX_ARRAY)
		{
			if (basic == LEX_INT)
			{
				type.depth = (*int_sizes)[adress].top().size();
				item = std::any(std::make_pair(adress, (*int_values_arr)[adress].top()));
			}
			else if (basic == LEX_BOOL)
			{
				type.depth = (*bool_sizes)[adress].top().size();
				item = std::any(std::make_pair(adress, (*bool_values_arr)[adress].top()));
			}
			else if (basic == LEX_FLOAT)
			{
				type.depth = (*float_sizes)[adress].top().size();
				item = std::any(std::make_pair(adress, (*float_values_arr)[adress].top()));
			}
			else if (basic == LEX_STRING)
			{
				type.depth = (*string_sizes)[adress].top().size();
				item = std::any(std::make_pair(adress, (*string_values_arr)[adress].top()));
			}
			else if (basic == LEX_CHAR)
			{
				type.depth = (*int_sizes)[adress].top().size();
				item = std::any(std::make_pair(adress, (*int_values_arr)[adress].top()));
			}
		}
		else if (basic == LEX_INT)
			item = std::any(&((*int_values)[adress].top()));
		else if (basic == LEX_BOOL)
			item = std::any(&((*bool_values)[adress].top()));
		else if (basic == LEX_FLOAT)
			item = std::any(&((*float_values)[adress].top()));
		else if (basic == LEX_STRING)
			item = std::any(&((*string_values)[adress].top()));
		else if (basic == LEX_CHAR)
			item = std::any(&((*int_values)[adress].top()));
		runtime_stack.push({ type, item });
		return;
	}
	const wchar_t* str = val.second.c_str();
	if (basic == LEX_INT)
		runtime_stack.push({ INT_TYPE, _wtoi(str) });
	else if (basic == LEX_BOOL)
	{
		if (str == L"0" || str == L"false")
			runtime_stack.push({ BOOL_TYPE, 0 });
		runtime_stack.push({ BOOL_TYPE, val.second.size() > 0});
	}
	else if (basic == LEX_FLOAT)
		runtime_stack.push({ FLOAT_TYPE,  _wtof(str) });
	else if (basic == LEX_STRING)
	{
		std::wstring str = val.second;
		if (str[0] == '"') str = val.second.substr(1, val.second.size() - 2);
		runtime_stack.push({ STRING_TYPE, str });
	}
	else if (basic == LEX_CHAR)
		runtime_stack.push({ CHAR_TYPE, int(val.second[val.second.size() > 1]) });
	else runtime_stack.push(NO_VALUE);
}

void CodeRunner::pushValue(int val)
{
	runtime_stack.push({ INT_TYPE, val });
}

void CodeRunner::pushValue(bool val)
{
	runtime_stack.push({ BOOL_TYPE, val });
}

void CodeRunner::pushValue(float val)
{
	runtime_stack.push({ FLOAT_TYPE, val });
}

void CodeRunner::pushValue(std::wstring val)
{
	runtime_stack.push({ STRING_TYPE, val });
}

void CodeRunner::pushValue(wchar_t val)
{
	runtime_stack.push({ CHAR_TYPE, int(val) });
}

void CodeRunner::runOperation(std::wstring op)
{
	if (op == L"=" || op == L"arg=")
		runEquation(op);
	else if (isIn(op, mem_operations))
		runMemOperation(op);
	else if (isIn(op, mat_operations))
		runMatOperation(op);
	else if (isIn(op, bin_operations))
		runBinOperation(op);
	else if (isIn(op, logic_operations))
		runLogicOperation(op);
}

void CodeRunner::runEquation(std::wstring op)
{
	value fr, to;
	if (op == L"=")
		fr = getValue(), to = getPointer();
	else to = getPointer(), fr = getValue();
	if (fr.first == STRING_TYPE && to.first.basic == CHAR_ && to.first.depth == 1)
	{
		std::wstring str = std::any_cast<std::wstring>(fr.second);
		int chsz = getSize(to);
		int* mem = std::any_cast<std::pair<int, int*> >(to.second).second;
		for (int i = 0; i < chsz; ++i, ++mem)
		{
			if (i < str.size())
				(*mem) = str[i];
			else  (*mem) = 0;
		}
		pushValue(to);
		return;
	}
	if (to.first == STRING_TYPE && fr.first.basic == CHAR_ && fr.first.depth == 1)
	{
		std::wstring str = L"";
		int chsz = getSize(fr);
		int* mem = std::any_cast<std::pair<int, int*>>(fr.second).second;
		for (int i = 0; i < std::min((int)str.size(), chsz); ++i, ++mem)
		{
			str += (*mem);
		}
		to.second = std::any(str);
		pushValue(to);
		return;
	}
	if (to.first.depth > 0)
	{
		int to_sz = getSize(to);
		int fr_sz = getSize(fr);
		if (to_sz != fr_sz)
		{
			// TODO: RE
		}
		if (to.first.basic == LEX_INT || to.first.basic == LEX_CHAR)
		{
			int* to_mem = std::any_cast<std::pair<int, int*>>(to.second).second;
			int* fr_mem = std::any_cast<std::pair<int, int*>>(fr.second).second;
			for (int i = 0; i < to_sz; ++i, ++to_mem, ++fr_mem)
			{
				(*to_mem) = (*fr_mem);
			}
		}
		else if (to.first.basic == LEX_BOOL)
		{
			bool* to_mem = std::any_cast<std::pair<int, bool*>>(to.second).second;
			bool* fr_mem = std::any_cast<std::pair<int, bool*>>(fr.second).second;
			for (int i = 0; i < to_sz; ++i, ++to_mem, ++fr_mem)
			{
				(*to_mem) = (*fr_mem);
			}
		}
		else if (to.first.basic == LEX_FLOAT)
		{
			float* to_mem = std::any_cast<std::pair<int, float*>>(to.second).second;
			float* fr_mem = std::any_cast<std::pair<int, float*>>(fr.second).second;
			for (int i = 0; i < to_sz; ++i, ++to_mem, ++fr_mem)
			{
				(*to_mem) = (*fr_mem);
			}
		}
		else if (to.first.basic == LEX_STRING)
		{
			std::wstring* to_mem = std::any_cast<std::pair<int, std::wstring*>>(to.second).second;
			std::wstring* fr_mem = std::any_cast<std::pair<int, std::wstring*>>(fr.second).second;
			for (int i = 0; i < to_sz; ++i, ++to_mem, ++fr_mem)
			{
				(*to_mem) = (*fr_mem);
			}
		}
		pushValue(to);
		return;
	}
	pushValue(fr);
	if (to.first.basic == LEX_INT)
	{
		int* to_mem = std::any_cast<int*>(to.second);
		int fr_mem = getInt();
		(*to_mem) = fr_mem;
	}
	else if (to.first.basic == LEX_BOOL)
	{
		bool* to_mem = std::any_cast<bool*>(to.second);
		bool fr_mem = getBool();
		(*to_mem) = fr_mem;
	}
	else if (to.first.basic == LEX_FLOAT)
	{
		float* to_mem = std::any_cast<float*>(to.second);
		float fr_mem = getFloat();
		(*to_mem) = fr_mem;
	}
	else if (to.first.basic == LEX_STRING)
	{
		std::wstring* to_mem = std::any_cast<std::wstring*>(to.second);
		std::wstring fr_mem = getString();
		(*to_mem) = fr_mem;
	}
	else if (to.first.basic == LEX_CHAR)
	{
		int* to_mem = std::any_cast<int*>(to.second);
		int fr_mem = getChar();
		(*to_mem) = fr_mem;
	}
	pushValue(to);
	if (op == L"arg=")
		pushValue(getValue());
}

void CodeRunner::runMemOperation(std::wstring op)
{
	value v1, v2; 
	if (op.size() == 3)
	{
		v1 = getPointer();
		pushValue(1);
		v2 = getValue();
	}
	else v2 = getPointer(), v1 = getPointer();
	if (op == L"a++" || op == L"a--")
	{
		pushValue(v1);
		pushValue(getValue());
	}
	pushValue(v1);
	pushValue(v1);
	pushValue(v2);
	runOperation(op.substr(0, 1));
	runEquation(L"=");
	if (op[0] == L'a')
		getValue();
}

void CodeRunner::runMatOperation(std::wstring op)
{
	value v2 = getValue(), v1 = getValue();
	TYPE top_type = getTopType(v1.first, v2.first);
	pushValue(v2);
	pushValue(v1);
	if (op == L"+")
	{
		if (top_type == INT_TYPE)
			pushValue(getInt() + getInt());
		else if (top_type == BOOL_TYPE)
			pushValue(getBool() + getBool());
		else if (top_type == FLOAT_TYPE)
			pushValue(getFloat() + getFloat());
		else if (top_type == STRING_TYPE) {
			auto s1 = getString();
			auto s2 = getString();
			pushValue(s1 + s2);
		}
		else if (top_type == CHAR_TYPE)
			pushValue(getChar() + getChar());
	}
	else if (op == L"-")
	{
		if (top_type == INT_TYPE) {
			auto t1 = getInt();
			auto t2 = getInt();
			pushValue(t1 - t2);
		}
		else if (top_type == BOOL_TYPE) {
			auto t1 = getBool();
			auto t2 = getBool();
			pushValue(t1 - t2);
		}
		else if (top_type == FLOAT_TYPE) {
			auto t1 = getFloat();
			auto t2 = getFloat();
			pushValue(t1 - t2);
		}
		else if (top_type == CHAR_TYPE) {
			auto t1 = getChar();
			auto t2 = getChar();
			pushValue(t1 - t2);
		}
	}
	if (op == L"*")
	{
		if (top_type == INT_TYPE)
			pushValue(getInt() * getInt());
		else if (top_type == BOOL_TYPE)
			pushValue(getBool() * getBool());
		else if (top_type == FLOAT_TYPE)
			pushValue(getFloat() * getFloat());
		else if (top_type == CHAR_TYPE)
			pushValue(getChar() * getChar());
	}
	if (op == L"/")
	{
		if (top_type == INT_TYPE) {
			auto t1 = getInt();
			auto t2 = getInt();
			pushValue(t1 / t2);
		}
		else if (top_type == BOOL_TYPE) {
			auto t1 = getBool();
			auto t2 = getBool();
			pushValue(t1 / t2);
		}
		else if (top_type == FLOAT_TYPE) {
			auto t1 = getFloat();
			auto t2 = getFloat();
			pushValue(t1 / t2);
		}
		else if (top_type == CHAR_TYPE) {
			auto t1 = getChar();
			auto t2 = getChar();
			pushValue(t1 / t2);
		}
	}
	if (op == L"%")
	{
		if (top_type == INT_TYPE) {
			auto t1 = getInt();
			auto t2 = getInt();
			pushValue(t1 % t2);
		}
		else if (top_type == BOOL_TYPE) {
			auto t1 = getBool();
			auto t2 = getBool();
			pushValue(t1 % t2);
		}
		else if (top_type == CHAR_TYPE) {
			auto t1 = getChar();
			auto t2 = getChar();
			pushValue(t1 % t2);
		}
	}
}

void CodeRunner::runBinOperation(std::wstring op)
{
	value v2 = getValue(), v1 = getValue();
	TYPE top_type = BOOL_TYPE;
	if (v2.first == INT_TYPE || v1.first == INT_TYPE)
		top_type = INT_TYPE;
	pushValue(v2);
	pushValue(v1);
	if (op == L"|")
	{
		if (top_type == BOOL_TYPE)
			pushValue(getBool() | getBool());
		else pushValue(getInt() | getInt());
	}
	else if (op == L"&")
	{
		if (top_type == BOOL_TYPE)
			pushValue(getBool() & getBool());
		else pushValue(getInt() & getInt());
	}
	if (op == L"^")
	{
		if (top_type == BOOL_TYPE)
			pushValue(getBool() ^ getBool());
		else pushValue(getInt() ^ getInt());
	}
	if (op == L"~")
	{
		if (top_type == BOOL_TYPE)
			pushValue(~getBool());
		else pushValue(~getInt());
	}
	if (op == L">>")
	{
		if (top_type == BOOL_TYPE) {
			auto t1 = getBool();
			auto t2 = getBool();
			pushValue(t1 >> t2);
		}
		else {
			auto t1 = getInt();
			auto t2 = getInt();
			pushValue(t1 >> t2);
		}
	}
	if (op == L"<<")
	{
		if (top_type == BOOL_TYPE) {
			auto t1 = getBool();
			auto t2 = getBool();
			pushValue(t1 << t2);
		}
		else {
			auto t1 = getInt();
			auto t2 = getInt();
			pushValue(t1 << t2);
		}
	}
}

void CodeRunner::runLogicOperation(std::wstring op)
{
	value v2 = getValue(), v1 = getValue();
	TYPE top_type = getTopType(v1.first, v2.first);
	pushValue(v2);
	pushValue(v1);
	if (op == L"||")
	{
		if (top_type == INT_TYPE)
			pushValue(getInt() || getInt());
		else if (top_type == BOOL_TYPE)
			pushValue(getBool() || getBool());
		else if (top_type == FLOAT_TYPE)
			pushValue(getFloat() || getFloat());
		else if (top_type == STRING_TYPE)
			pushValue(getBool() || getBool());
		else if (top_type == CHAR_TYPE)
			pushValue(getChar() || getChar());
	}
	if (op == L"&&")
	{
		if (top_type == INT_TYPE)
			pushValue(getInt() && getInt());
		else if (top_type == BOOL_TYPE)
			pushValue(getBool() && getBool());
		else if (top_type == FLOAT_TYPE)
			pushValue(getFloat() && getFloat());
		else if (top_type == STRING_TYPE)
			pushValue(getBool() && getBool());
		else if (top_type == CHAR_TYPE)
			pushValue(getChar() && getChar());
	}
	if (op == L"==")
	{
		if (top_type == INT_TYPE)
			pushValue(getInt() == getInt());
		else if (top_type == BOOL_TYPE)
			pushValue(getBool() == getBool());
		else if (top_type == FLOAT_TYPE)
			pushValue(getFloat() == getFloat());
		else if (top_type == STRING_TYPE)
			pushValue(getBool() == getBool());
		else if (top_type == CHAR_TYPE)
			pushValue(getChar() == getChar());
	}
	if (op == L"!=")
	{
		if (top_type == INT_TYPE)
			pushValue(getInt() != getInt());
		else if (top_type == BOOL_TYPE)
			pushValue(getBool() != getBool());
		else if (top_type != FLOAT_TYPE)
			pushValue(getFloat() != getFloat());
		else if (top_type != STRING_TYPE)
			pushValue(getBool() != getBool());
		else if (top_type == CHAR_TYPE)
			pushValue(getChar() != getChar());
	}
	if (op == L">")
	{
		if (top_type == INT_TYPE) {
			auto t1 = getInt();
			auto t2 = getInt();
			pushValue(t1 > t2);
		}
		else if (top_type == BOOL_TYPE) {
			auto t1 = getBool();
			auto t2 = getBool();
			pushValue(t1 > t2);
		}
		else if (top_type == FLOAT_TYPE) {
			auto t1 = getFloat();
			auto t2 = getFloat();
			pushValue(t1 > t2);
		}
		else if (top_type == STRING_TYPE) {
			auto t1 = getString();
			auto t2 = getString();
			pushValue(t1 > t2);
		}
		else if (top_type == CHAR_TYPE) {
			auto t1 = getChar();
			auto t2 = getChar();
			pushValue(t1 > t2);
		}
	}
	if (op == L"<")
	{
		if (top_type == INT_TYPE) {
			auto t1 = getInt();
			auto t2 = getInt();
			pushValue(t1 < t2);
		}
		else if (top_type == BOOL_TYPE) {
			auto t1 = getBool();
			auto t2 = getBool();
			pushValue(t1 < t2);
		}
		else if (top_type == FLOAT_TYPE) {
			auto t1 = getFloat();
			auto t2 = getFloat();
			pushValue(t1 < t2);
		}
		else if (top_type == STRING_TYPE) {
			auto t1 = getString();
			auto t2 = getString();
			pushValue(t1 < t2);
		}
		else if (top_type == CHAR_TYPE) {
			auto t1 = getChar();
			auto t2 = getChar();
			pushValue(t1 < t2);
		}
	}
	if (op == L">=")
	{
		if (top_type == INT_TYPE) {
			auto t1 = getInt();
			auto t2 = getInt();
			pushValue(t1 >= t2);
		}
		else if (top_type == BOOL_TYPE) {
			auto t1 = getBool();
			auto t2 = getBool();
			pushValue(t1 >= t2);
		}
		else if (top_type == FLOAT_TYPE) {
			auto t1 = getFloat();
			auto t2 = getFloat();
			pushValue(t1 >= t2);
		}
		else if (top_type == STRING_TYPE) {
			auto t1 = getString();
			auto t2 = getString();
			pushValue(t1 >= t2);
		}
		else if (top_type == CHAR_TYPE) {
			auto t1 = getChar();
			auto t2 = getChar();
			pushValue(t1 >= t2);
		}
	}
	if (op == L"<=")
	{
		if (top_type == INT_TYPE) {
			auto t1 = getInt();
			auto t2 = getInt();
			pushValue(t1 <= t2);
		}
		else if (top_type == BOOL_TYPE) {
			auto t1 = getBool();
			auto t2 = getBool();
			pushValue(t1 <= t2);
		}
		else if (top_type == FLOAT_TYPE) {
			auto t1 = getFloat();
			auto t2 = getFloat();
			pushValue(t1 <= t2);
		}
		else if (top_type == STRING_TYPE) {
			auto t1 = getString();
			auto t2 = getString();
			pushValue(t1 <= t2);
		}
		else if (top_type == CHAR_TYPE) {
			auto t1 = getChar();
			auto t2 = getChar();
			pushValue(t1 <= t2);
		}
	}
}



value CodeRunner::getValue()
{
	if (runtime_stack.top().first.basic == LEX_BOOL && runtime_stack.top().first.depth == 1)
	{
		value arr = runtime_stack.top();
		runtime_stack.pop();
		auto arr_t = std::any_cast<std::pair<int, int*>>(arr.second);
		int size = (*int_sizes)[arr_t.first].top()[0];
		std::string str(size, 0);
		for (int i = 0; i < size; ++i)
		{
			str[i] = *arr_t.second;
			++arr_t.second;
		}
		return { STRING_TYPE, str };
	}
	if (runtime_stack.top().first.depth > 0)
		return getPointer();
	value res = runtime_stack.top();
	if (runtime_stack.top().first.is_adress)
	{
		res.first.is_adress = 0;
		if (res.first.basic == LEX_INT)
			res.second = std::any(*std::any_cast<int*>(res.second));
		else if (res.first.basic == LEX_BOOL)
			res.second = std::any(*std::any_cast<bool*>(res.second));
		else if (res.first.basic == LEX_FLOAT)
			res.second = std::any(*std::any_cast<float*>(res.second));
		else if (res.first.basic == LEX_STRING)
			res.second = std::any(*std::any_cast<std::wstring*>(res.second));
		else if (res.first.basic == LEX_CHAR)
			res.second = std::any(*std::any_cast<int*>(res.second));
	}
	runtime_stack.pop();
	return res;
}

value CodeRunner::getPointer()
{
	if (runtime_stack.top().first.is_adress == 0)
		return getValue();
	value res = runtime_stack.top();
	runtime_stack.pop();
	return res;
}

int CodeRunner::getInt()
{
	value val = getValue();
	if (val.first == INT_TYPE)
		return std::any_cast<int>(val.second);
	if (val.first == BOOL_TYPE)
		return std::any_cast<bool>(val.second);
	if (val.first == FLOAT_TYPE)
		return std::any_cast<float>(val.second);
	if (val.first == STRING_TYPE)
	{
		std::wstring str = std::any_cast<std::wstring>(val.second);
		if (str == L"0")
			return 0;
		int v = _wtoi(str.c_str());
		if (v == 0)
		{
			return 0;
		}
		return v;
	}
	if (val.first == CHAR_TYPE)
		return std::any_cast<int>(val.second);
}

bool CodeRunner::getBool()
{
	value val = getValue();
	if (val.first == INT_TYPE)
		return std::any_cast<int>(val.second) != 0;
	if (val.first == BOOL_TYPE)
		return std::any_cast<bool>(val.second);
	if (val.first == FLOAT_TYPE)
		return std::any_cast<float>(val.second) != 0;
	if (val.first == STRING_TYPE)
	{
		std::wstring str = std::any_cast<std::wstring>(val.second);
		if (str == L"0")
			return 0;
		else if (str == L"1")
			return 1;
		return str.size() != 0;
	}
	if (val.first == CHAR_TYPE)
		return std::any_cast<int>(val.second) != 0;
}

float CodeRunner::getFloat()
{
	value val = getValue();
	if (val.first == INT_TYPE)
		return std::any_cast<int>(val.second);
	if (val.first == BOOL_TYPE)
		return std::any_cast<bool>(val.second);
	if (val.first == FLOAT_TYPE)
		return std::any_cast<float>(val.second);
	if (val.first == STRING_TYPE)
	{
		std::wstring str = std::any_cast<std::wstring>(val.second);
		if (str == L"0")
			return 0;
		float v = _wtof(str.c_str());
		if (v == 0)
		{
			return 0;
		}
		return v;
	}
	if (val.first == CHAR_TYPE)
		return std::any_cast<int>(val.second);
}

std::wstring CodeRunner::getString()
{
	value val = getValue();
	if (val.first == INT_TYPE)
		return std::to_wstring(std::any_cast<int>(val.second));
	if (val.first == BOOL_TYPE)
		return (std::any_cast<bool>(val.second) ? L"1" : L"0");
	if (val.first == FLOAT_TYPE)
		return std::to_wstring(std::any_cast<float>(val.second));
	if (val.first == STRING_TYPE)
		return std::any_cast<std::wstring>(val.second);
	if (val.first == CHAR_TYPE)
	{
		int ch = std::any_cast<int>(val.second);
		return std::wstring(1, wchar_t(ch));
	}
	if (val.first.basic == LEX_CHAR && val.first.depth == 1)
	{
		int chsz = getSize(val);
		int* ch = std::any_cast<std::pair<int, int*>>(val.second).second;
		std::wstring res = L"";
		for (int i = 0; i < chsz; ++i, ++ch) {
			res += wchar_t(*ch);
		}
		return res;
	}
}

wchar_t CodeRunner::getChar()
{
	value val = getValue();
	if (val.first == INT_TYPE)
		return std::any_cast<int>(val.second);
	if (val.first == BOOL_TYPE)
		return std::any_cast<bool>(val.second);
	if (val.first == FLOAT_TYPE)
		return std::any_cast<float>(val.second);
	if (val.first == STRING_TYPE)
	{
		std::wstring str = std::any_cast<std::wstring>(val.second);
		if (str == L"")
			return 0;
		return str[0];
	}
	if (val.first == CHAR_TYPE)
		return std::any_cast<int>(val.second);
}

int CodeRunner::getSize(value& arr)
{
	int sz;
	return getSize(arr, sz);
}

bool CodeRunner::isIn(std::wstring op, std::vector<const wchar_t*>& ops)
{
	for (auto t : ops)
	{
		if (op == t)
			return 1;
	}
	return 0;
}

int CodeRunner::getSize(value& arr, int & size)
{
	if (arr.first == STRING_TYPE)
	{
		std::wstring val = std::any_cast<std::wstring>(arr.second);
		return val.size();
	}
	int part = 1;
	if (arr.first.basic == LEX_INT)
	{
		auto arr_t = std::any_cast<std::pair<int, int*>>(arr.second);
		int sz1 = (*int_sizes)[arr_t.first].top().size();
		size = (*int_sizes)[arr_t.first].top()[sz1 - arr.first.depth];
		for (int i = 0; i < arr.first.depth; ++i) {
			part *= (*int_sizes)[arr_t.first].top()[sz1 - i - 1];
		}
	}
	else if (arr.first.basic == LEX_BOOL)
	{
		auto arr_t = std::any_cast<std::pair<int, bool>>(arr.second);
		int sz1 = (*bool_sizes)[arr_t.first].top().size();
		size = (*bool_sizes)[arr_t.first].top()[sz1 - arr.first.depth];
		for (int i = 0; i < arr.first.depth; ++i) {
			part *= (*bool_sizes)[arr_t.first].top()[sz1 - i - 1];
		}
	}
	else if (arr.first.basic == LEX_FLOAT)
	{
		auto arr_t = std::any_cast<std::pair<int, float*>>(arr.second);
		int sz1 = (*float_sizes)[arr_t.first].top().size();
		size = (*float_sizes)[arr_t.first].top()[sz1 - arr.first.depth];
		for (int i = 0; i < arr.first.depth; ++i) {
			part *= (*float_sizes)[arr_t.first].top()[sz1 - i - 1];
		}
	}
	else if (arr.first.basic == LEX_STRING)
	{
		auto arr_t = std::any_cast<std::pair<int, std::wstring*>>(arr.second);
		int sz1 = (*string_sizes)[arr_t.first].top().size();
		size = (*string_sizes)[arr_t.first].top()[sz1 - arr.first.depth];
		for (int i = 0; i < arr.first.depth; ++i) {
			part *= (*string_sizes)[arr_t.first].top()[sz1 - i - 1];
		}
	}
	else if (arr.first.basic == LEX_CHAR)
	{
		auto arr_t = std::any_cast<std::pair<int, int*>>(arr.second);
		int sz1 = (*int_sizes)[arr_t.first].top().size();
		size = (*int_sizes)[arr_t.first].top()[sz1 - arr.first.depth];
		for (int i = 0; i < arr.first.depth; ++i) {
			part *= (*int_sizes)[arr_t.first].top()[sz1 - i - 1];
		}
	}
	return part;
}

TYPE CodeRunner::getTopType(TYPE t1, TYPE t2)
{
	TYPE res = NO_TYPE;
	if (t1 == t2) return t1;
	if (t1 == STRING_TYPE || t2 == STRING_TYPE)
		return STRING_TYPE;
	if (t1 == FLOAT_TYPE || t2 == FLOAT_TYPE)
		return FLOAT_TYPE;
	if (t1 == INT_TYPE || t2 == INT_TYPE)
		return INT_TYPE;
	if (t1 == CHAR_TYPE || t2 == CHAR_TYPE)
		return CHAR_TYPE;

	return t1;
}
