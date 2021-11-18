/*
*	This file is part of libopi.
*
*	Copyright (c) 2018 Tor Krill <tor@openproducts.com>
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

#include <list>
#include <string>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

namespace OPI {

class SysConfig
{
public:
	SysConfig();
	SysConfig(bool writeable);
	SysConfig(string  path, bool writeable = false);

	void Writeable(bool writeable);
	bool IsWriteable();

	string GetKeyAsString(const string& scope, const string& key);
	list<string> GetKeyAsStringList(const string& scope, const string& key);
	int GetKeyAsInt(const string& scope, const string& key);
	list<int> GetKeyAsIntList(const string& scope, const string& key);
	bool GetKeyAsBool(const string& scope, const string& key);
	list<bool> GetKeyAsBoolList(const string& scope, const string& key);

	void PutKey(const string& scope, const string& key, const char* value);
	void PutKey(const string& scope, const string& key, const string& value);
	void PutKey(const string& scope, const string& key, const list<string>& value);
	void PutKey(const string& scope, const string& key, int value);
	void PutKey(const string& scope, const string& key, const list<int>& value);
	void PutKey(const string& scope, const string& key, bool value);
	void PutKey(const string& scope, const string& key, const list<bool>& value);

	void RemoveKey(const string& scope, const string& key);

	bool HasKey(const string& scope, const string& key);
	bool HasScope(const string& scope);

	virtual ~SysConfig();
private:

	// Convenience function
	// Open, Load and close file, for get operations
	json LoadDB();

	json GetKey(const json& db, const string& scope, const string& key);

	bool DBExists();
	bool HasScope(const json& val, const string& scope);
	bool HasKey(const json& val, const string& scope, const string& key);
	void OpenDB();
	void CloseDB();
	json ReadDB();
	void WriteDB(const json& db);

	string path;
	int fd;
	bool writeable;
};
}

#endif // SYSCONFIG_H
