
#include <utility>

#include "JsonHelper.h"

namespace OPI {

namespace JsonHelper {


TypeChecker::TypeChecker(const vector<OPI::JsonHelper::TypeChecker::Check> &checks, ErrCallback err, void *data):
	checks(checks), errcallback(std::move(err)), data(data)
{

}

static inline bool
CheckArgument(const json& cmd, const string& member, TypeChecker::Type type)
{
	if( cmd.is_null() )
	{
		return false;
	}

	switch( type )
	{
	case TypeChecker::Type::STRING:
		return cmd.contains( member ) && cmd[member].is_string();
	case TypeChecker::Type::INT:
		return cmd.contains( member ) && cmd[member].is_number_integer();
	case TypeChecker::Type::BOOL:
		return cmd.contains( member ) && cmd[member].is_boolean();
	}
	// Should never get here!
	return false;
}


bool TypeChecker::Verify(int what, const json &val)
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

list<string> FromJsonArray(const json &jsonlist)
{
	list<string> ret;

	for(const auto& val: jsonlist)
	{
		ret.push_back( val.get<string>() );
	}

	return ret;
}

json ToJsonArray(const list<string> &list)
{
	json ret=json::array();
	for( const auto& val: list)
	{
		ret.push_back(val);
	}
	return ret;
}

map<string, string> FromJsonObject(const json &jsonobj)
{
	map<string,string> ret;

	for(const auto& val:jsonobj.items())
	{
		ret[val.key()] = val.value();
	}

	return ret;
}

json ToJsonObject(const map<string, string> &objmap)
{
	json ret;

	for( const auto& val: objmap)
	{
		ret[val.first] = val.second;
	}

	return ret;
}

} // End namespace JsonHelper

} // END namespace OPI
