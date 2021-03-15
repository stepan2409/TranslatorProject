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
		}
		return &func_args_[func_order_[name]];
	}
	if (parent != nullptr)
		return parent->get_arguments(name);
	return nullptr;
}

bool TID::deepsearch_id(std::wstring& name)
{
	if (types_.find(name) != types_.end())
		return 1;
	if (parent != nullptr)
		return parent->deepsearch_id(name);
	return 0;
}

bool TID::push_id(std::wstring& name, TYPE type)
{
	if(deepsearch_id(name))
		return 0;
	types_[name] = type;
	return 1;
}
