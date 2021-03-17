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
	ADRESS_ = 5
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
};
#define NO_TYPE TYPE(TYPES::UNKNOWN, 0, 0)
#define INT_TYPE TYPE(TYPES::INT_, 0, 0)
#define FLOAT_TYPE TYPE(TYPES::FLOAT_, 0, 0)
#define BOOL_TYPE TYPE(TYPES::BOOL_, 0, 0)
#define STRING_TYPE TYPE(TYPES::STRING_, 0, 0)
// TYPE {ID, ARRAY_DEPTH} 
// ID: 0 - UNKNOWN, 1 - INT, 2 - BOOL, 3 - FLOAT, 4 - STRING, [5, +INF) - CUSTOM

class TID
{
public:
	TID();
	TID(TID* parent);
	~TID();

	bool push_id(std::wstring& name, TYPE type);
	bool is_template(std::wstring& name, TYPE type);
	bool push_code(std::wstring& name);
	TYPE get_type(std::wstring& name);
	std::vector<TYPE>* get_arguments(std::wstring& name);

	std::wstring get_any_template();

	std::vector<TID*> children;
	TID* parent = nullptr;
private:

	std::map<std::wstring, TYPE> types_;
	std::map<std::wstring, int> func_order_;
	std::vector<std::vector<TYPE> > func_args_;
	std::vector<bool> func_status_;

	bool search_id(std::wstring& name);
	bool deepsearch_id(std::wstring& name);
};

