#pragma once
#include <vector>
#include <map>
#include <string>

enum TYPES
{
	UNKNOWN = 0,
	INT_ = 1,
	BOOL_ = 2,
	FLOAT_ = 3,
	STRING_ = 4,
	ADRESS_ = 5,
	VOID_ = 6
};

struct TYPE {
	TYPES basic;
	short depth;
	bool is_adress;
	TYPE(TYPES basic, short depth = 0, bool is_adress = 0) : basic(basic), depth(depth), is_adress(is_adress)
	{

	}
	TYPE() : basic(TYPES::UNKNOWN), depth(0), is_adress(0) { };
	void operator = (const TYPE& type)
	{
		basic = type.basic;
		depth = type.depth;
		is_adress = is_adress;
	}
	bool operator == (const TYPE type)
	{
		return basic == type.basic &&
			depth == type.depth;
	}
	bool operator != (TYPE type)
	{
		return !(*this == type);
	}
	std::wstring to_str() {
		std::wstring res;
		if (basic == TYPES::INT_)
			res = L"int";
		else if (basic == TYPES::BOOL_)
			res = L"bool_";
		else if (basic == TYPES::FLOAT_)
			res = L"float";
		else if (basic == TYPES::STRING_)
			res = L"string";
		else if (basic == TYPES::VOID_)
			res = L"void";
		else if (basic == TYPES::ADRESS_)
			res = L"label";
		else res = L"unknown";
		res += std::wstring(depth, '*');
		return res;
	}
};
#define NO_TYPE TYPE(TYPES::UNKNOWN, 0, 0)
#define INT_TYPE TYPE(TYPES::INT_, 0, 0)
#define FLOAT_TYPE TYPE(TYPES::FLOAT_, 0, 0)
#define BOOL_TYPE TYPE(TYPES::BOOL_, 0, 0)
#define STRING_TYPE TYPE(TYPES::STRING_, 0, 0)
#define ADRESS_TYPE TYPE(TYPES::ADRESS_, 0, 0)
#define VOID_TYPE TYPE(TYPES::VOID_, 0, 0)
// TYPE {ID, ARRAY_DEPTH} 
// ID: 0 - UNKNOWN, 1 - INT, 2 - BOOL, 3 - FLOAT, 4 - STRING, [5, +INF) - CUSTOM

class TID
{
public:
	TID();
	TID(TID* parent);
	~TID();

	void set_return_type(TYPE type);
	bool push_id(std::wstring& name, TYPE type);
	bool is_template(std::wstring& name, TYPE type);
	bool is_function(std::wstring& name);
	bool is_current(std::wstring& name);
	bool push_code(std::wstring& name);
	TYPE get_type(std::wstring& name);
	TYPE get_return_type();
	std::vector<TYPE>* get_arguments(std::wstring& name);

	std::wstring get_any_template();

	std::vector<TID*> children;
	TID* parent = nullptr;
private:

	std::map<std::wstring, TYPE> types_;
	TYPE return_type = NO_TYPE;

	std::map<std::wstring, int> func_order_;
	std::vector<std::vector<TYPE> > func_args_;
	std::vector<bool> func_status_;

	bool search_id(std::wstring& name);
	bool deepsearch_id(std::wstring& name);
};

