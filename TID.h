#pragma once
#include <vector>
#include <map>
#include <string>

typedef std::pair<short, short> TYPE;
#define NO_TYPE {-1, -1}
// TYPE {ID, ARRAY_DEPTH} 
// ID: 0 - UNKNOWN, 1 - INT, 2 - BOOL, 3 - FLOAT, 4 - STRING, [5, +INF) - CUSTOM

class TID
{
public:
	TID();
	TID(TID* parent);
	~TID();

	bool push_id(std::wstring& name, TYPE type);
	std::vector<TYPE>* get_arguments(std::wstring & name);
	std::vector<TID*> children;
	TID* parent = nullptr;
private:

	std::map<std::wstring, TYPE> types_;
	std::map<std::wstring, int> func_order_;
	std::vector<std::vector<TYPE> > func_args_;

	bool deepsearch_id(std::wstring& name);
};

