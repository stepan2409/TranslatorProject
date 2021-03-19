#include "TID.h"

TID::TID()
{}

TID::TID(TID* parent) : parent(parent)
{

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
			func_status_.push_back(1);
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
		if (func_status_[func.second] == 1)
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
	return_type = type;
}

bool TID::push_id(std::wstring & name, TYPE type)
{
	if (deepsearch_id(name))
		return 0;
	types_[name] = type;
	return 1;
}

bool TID::is_template(std::wstring & name, TYPE type)
{
	if (func_order_.find(name) != func_order_.end())
	{
		if (types_[name] != type)
			return 0;
		return func_status_[func_order_[name]];
	}
	if (parent != nullptr)
		return parent->is_template(name, type);
	return 0;
}

bool TID::is_function(std::wstring& name)
{
	if (func_order_.find(name) != func_order_.end())
		return 1;
	if (parent != nullptr)
		return parent->is_function(name);
	return 0;
}

bool TID::push_code(std::wstring & name)
{
	if (func_order_.find(name) == func_order_.end())
		return 0;
	if (func_status_[func_order_[name]] == 0)
		return 0;
	func_status_[func_order_[name]] = 0;
	return 1;
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
	if (return_type != NO_TYPE)
		return return_type;
	if (parent != nullptr)
		return parent->get_return_type();
	return NO_TYPE;
}
