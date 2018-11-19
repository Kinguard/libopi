
#include "JsonHelper.h"

namespace OPI {

namespace JsonHelper {


TypeChecker::TypeChecker(const vector<OPI::JsonHelper::TypeChecker::Check> &checks, ErrCallback err, void *data):
	checks(checks), errcallback(err), data(data)
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

TypeChecker::~TypeChecker()
{

}

} // End namespace JsonHelper

} // END namespace OPI
