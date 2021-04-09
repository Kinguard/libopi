
#include <utility>

#include "JsonHelper.h"

namespace OPI {

namespace JsonHelper {


TypeChecker::TypeChecker(const vector<OPI::JsonHelper::TypeChecker::Check> &checks, ErrCallback err, void *data):
	checks(checks), errcallback(std::move(err)), data(data)
{

}

static inline bool
CheckArgument(const Json::Value& cmd, const string& member, TypeChecker::Type type)
{
	if( cmd.isNull() )
	{
		return false;
	}

	switch( type )
	{
	case TypeChecker::Type::STRING:
		return cmd.isMember( member ) && cmd[member].isString();
	case TypeChecker::Type::INT:
		return cmd.isMember( member ) && cmd[member].isInt();
	case TypeChecker::Type::BOOL:
		return cmd.isMember( member ) && cmd[member].isBool();
	}
	// Should never get here!
	return false;
}


bool TypeChecker::Verify(int what, const Json::Value &val)
{
	for( auto check: this->checks )
	{
		if( what & check.check && ! CheckArgument( val, check.member, check.type) )
		{
			if( this->errcallback )
			{
				this->errcallback(string("Failed to validate member: ") + check.member , data);
			}
			return false;
		}
	}
	return true;
}

TypeChecker::~TypeChecker() = default;

list<string> FromJsonArray(const Json::Value &jsonlist)
{
	list<string> ret;

	for(const auto& val: jsonlist)
	{
		ret.push_back( val.asString() );
	}

	return ret;
}

Json::Value ToJsonArray(const list<string> &list)
{
	Json::Value ret(Json::arrayValue);
	for( const auto& val: list)
	{
		ret.append(val);
	}
	return ret;
}

map<string, string> FromJsonObject(const Json::Value &jsonobj)
{
	map<string,string> ret;

	for(const auto& val:jsonobj.getMemberNames())
	{
		ret[val] = jsonobj[val].asString();
	}

	return ret;
}

Json::Value ToJsonObject(const map<string, string> &objmap)
{
	Json::Value ret(Json::objectValue);

	for( const auto& val: objmap)
	{
		ret[val.first] = val.second;
	}

	return ret;
}

} // End namespace JsonHelper

} // END namespace OPI
