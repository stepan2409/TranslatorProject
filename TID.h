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

typedef std::pair <TYPES, int > TYPE;
#define NO_TYPE std::make_pair(TYPES::UNKNOWN, 0);
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
	std::vector<TYPE>* get_arguments(std::wstring & name);

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

