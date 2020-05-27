#ifndef JSONHELPER_H
#define JSONHELPER_H

#include <json/json.h>

#include <map>
#include <list>
#include <string>
#include <vector>
#include <functional>

using namespace std;

namespace OPI {

namespace JsonHelper {

class TypeChecker
{
public:

	typedef function<void(const string& errmsg, void* data)> ErrCallback;

	enum Type{
		STRING,
		INT,
		BOOL
	};

	struct Check
	{
		int				check;
		const char*		member;
		Type			type;
	};

	TypeChecker(const vector<Check>& checks, ErrCallback err = nullptr, void *data = nullptr);

	bool Verify(int what, const Json::Value& val);

	virtual ~TypeChecker();
private:
	vector<Check> checks;
	ErrCallback errcallback;
	void *data;
};


list<string> FromJsonArray(const Json::Value& jsonlist);

Json::Value ToJsonArray(const list<string>& list);

map<string,string> FromJsonObject(const Json::Value& jsonobj);

Json::Value ToJsonObject(const map<string,string>& objmap);

} // End namespace JsonHelper

} // END namespace OPI

#endif // JSONHELPER_H
