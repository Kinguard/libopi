/*
*	This file is part of libopi.
*
*	libopi is free software: you can redistribute it and/or modify
*	it under the terms of the GNU Affero General Public License as published
*	by the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	libopi is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU Affero General Public License for more details.
*
*	You should have received a copy of the GNU Affero General Public
*	License along with libopi.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SYSCONFIG_H
#define SYSCONFIG_H

#include <string>
#include <json/json.h>

using namespace std;

namespace OPI {

class SysConfig
{
public:
	SysConfig();
	SysConfig(const string& path);

	string GetKeyAsString(const string& scope, const string& key);
	void PutKey(const string& scope, const string& key, const string& value);
	void RemoveKey(const string& scope, const string& key);

	virtual ~SysConfig();
private:

	// Convenience function
	// Open, Load and close file, for get operations
	Json::Value LoadDB();

	Json::Value GetKey(const Json::Value& db, const string& scope, const string& key);

	bool DBExists();
	bool HasScope(const Json::Value& val, const string& scope);
	bool HasKey(const Json::Value& val, const string& scope, const string& key);
	void OpenDB();
	void CloseDB();
	Json::Value ReadDB();
	void WriteDB(const Json::Value& db);

	string path;
	int fd;
};
}

#endif // SYSCONFIG_H
