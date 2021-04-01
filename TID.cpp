#include "TID.h"

TID::TID()
{}

TID::TID(TID* parent) : parent(parent)
{
	push_value = parent->push_value;
}

TID::~TID()
{
	for (auto t : children)
	{
		delete t;
	}
}

std::vector<TYPE>* TID::get_arguments(std::wstring& name)
{
	if (types_.find(name) != types_.end())
	{
		if (func_order_.find(name) == func_order_.end())
		{
			func_order_[name] = func_args_.size();
			func_args_.push_back({});
			func_code_.push_back(-1);
		}
		return &func_args_[func_order_[name]];
	}
	if (parent != nullptr)
		return parent->get_arguments(name);
	return nullptr;
}

std::wstring TID::get_any_template()
{
	for (auto func : func_order_)
	{
		if (func_code_[func.second] == -1)
			return func.first;
	}
	return L"";
}

bool TID::search_id(std::wstring & name)
{
	return types_.find(name) != types_.end();
}

bool TID::deepsearch_id(std::wstring & name)
{
	if (types_.find(name) != types_.end())
		return 1;
	if (parent != nullptr)
		return parent->deepsearch_id(name);
	return 0;
}

void TID::set_return_type(TYPE type)
{
	return_type_ = type;
}

bool TID::push_id(std::wstring & name, TYPE type)
{
	if (search_id(name))
		return 0;
	types_[name] = type;
	var_adress_[name] = push_value(type);
	return 1;
}

bool TID::is_template(std::wstring & name, TYPE type)
{
	if (func_order_.find(name) != func_order_.end())
	{
		if (types_[name] != type)
			return 0;
		return func_code_[func_order_[name]] == -1;
	}
	if (parent != nullptr)
		return parent->is_template(name, type);
	return 0;
}

bool TID::is_function(std::wstring & name)
{
	if (func_order_.find(name) != func_order_.end())
		return 1;
	if (parent != nullptr)
		return parent->is_function(name);
	return 0;
}

bool TID::is_current(std::wstring & name)
{
	return types_.find(name) != types_.end();
}

bool TID::push_code(std::wstring& name, int to)
{
	if (func_order_.find(name) == func_order_.end())
		return 0;
	if (func_code_[func_order_[name]] != -1)
		return 0;
	func_code_[func_order_[name]] = to;
	return 1;
}
int TID::get_code(std::wstring& name)
{
	if (func_order_.find(name) != func_order_.end())
		return func_code_[func_order_[name]];
	if (parent != nullptr)
		return parent->get_code(name);
	return -1;
}
TYPE TID::get_type(std::wstring & name)
{
	if (search_id(name))
		return types_[name];
	if (deepsearch_id(name))
		return parent->get_type(name);
	return NO_TYPE;
}

TYPE TID::get_return_type()
{
	if (return_type_ != NO_TYPE)
		return return_type_;
	if (parent != nullptr)
		return parent->get_return_type();
	return NO_TYPE;
}

lexem TID::get_adress(std::wstring& name)
{
	if (types_.find(name) != types_.end())
	{
		int type_code = LEX_ADRESS;
		TYPE type = types_[name];
		type_code |= type.basic;
		if (type.depth > 0)
			type_code |= LEX_ARRAY;
		return { type_code, std::to_wstring(var_adress_[name]) };
	}
	if (parent != nullptr)
		return parent->get_adress(name);
	return { UNKNOWN, L"" };
}
