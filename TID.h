#pragma once
#include <vector>
#include <map>
#include <string>

typedef std::pair<int, std::wstring> lexem;

enum TYPES
{
	UNKNOWN = 0,
	INT_ = 1,
	BOOL_ = 2,
	FLOAT_ = 3,
	STRING_ = 4,
	CHAR_ = 5,
	VOID_ = 6
};

enum LEX
{
	LEX_UNKNOWN = 0,
	LEX_INT = 1,
	LEX_BOOL = 2,
	LEX_FLOAT = 3,
	LEX_STRING = 4,
	LEX_CHAR = 5,
	LEX_OPERATION = 6,
	LEX_ADRESS = 8,
	LEX_ARRAY = 16
};

struct TYPE {
	short basic;
	short depth;
	bool is_adress;
	TYPE(short basic, short depth = 0, bool is_adress = 0) : basic(basic), depth(depth), is_adress(is_adress)
	{

	}
	TYPE() : basic(TYPES::UNKNOWN), depth(0), is_adress(0) { };
	void operator = (const TYPE& type)
	{
		basic = type.basic;
		depth = type.depth;
		is_adress = type.is_adress;
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
#define VOID_TYPE TYPE(TYPES::VOID_, 0, 0)
#define CHAR_TYPE TYPE(TYPES::CHAR_, 0, 0)
// TYPE {ID, ARRAY_DEPTH} 
// ID: 0 - UNKNOWN, 1 - INT, 2 - BOOL, 3 - FLOAT, 4 - STRING, [5, +INF) - CUSTOM

class TID
{
public:
	TID();
	TID(TID* parent);
	~TID();

	void set_return_type(TYPE type);
	void set_return_adress(lexem adress);
	void push_return_line(int line);
	void set_is_adress(std::wstring& name, bool value);
	bool push_id(std::wstring& name, TYPE type);
	bool is_template(std::wstring& name, TYPE type);
	bool is_function(std::wstring& name);
	bool is_current(std::wstring& name);
	bool push_code(std::wstring& name, int to);
	 int get_code(std::wstring& name);
	TYPE get_type(std::wstring& name);
	TYPE get_return_type();
	lexem get_return_adress();
	lexem get_adress(std::wstring& name);
	std::vector<TYPE>* get_arguments(std::wstring& name);
	std::vector<int>* get_return_lines();

	std::wstring get_any_template();

	std::vector<TID*> children;
	TID* parent = nullptr;
	int (*push_value)(TYPE type);
private:

	std::map<std::wstring, TYPE> types_;
	TYPE return_type_ = NO_TYPE;
	lexem return_adress_;
	std::map<std::wstring, int> func_order_;
	std::map<std::wstring, int> var_adress_;
	std::vector<std::vector<TYPE> > func_args_;
	std::vector<int> func_code_;
	std::vector<int> return_lines_;

	bool search_id(std::wstring& name);
	bool deepsearch_id(std::wstring& name);

	friend void set_values_function(TID* tree);
};

