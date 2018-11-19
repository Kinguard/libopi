#ifndef JSONHELPER_H
#define JSONHELPER_H

#include <json/json.h>

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


} // End namespace JsonHelper

} // END namespace OPI

#endif // JSONHELPER_H
