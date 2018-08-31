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

#include "SysConfig.h"
#include "Config.h"

#include <libutils/Exceptions.h>
#include <libutils/FileUtils.h>
#include <libutils/Logger.h>
#include <sys/file.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <vector>
#include <ext/stdio_filebuf.h>
#include <fstream>

using namespace Utils;

namespace OPI {

SysConfig::SysConfig(): path(SYSCONFIGDBPATH),fd(0), writeable(false)
{

}

SysConfig::SysConfig(bool writeable): path(SYSCONFIGDBPATH),fd(0), writeable(writeable)
{

}

SysConfig::SysConfig(const string &path, bool writeable): path(path), fd(0), writeable(writeable)
{

}

void SysConfig::Writeable(bool writeable)
{
	this->writeable = writeable;
}

bool SysConfig::IsWriteable()
{
	return this->writeable;
}

string SysConfig::GetKeyAsString(const string &scope, const string &key)
{
	Json::Value db = this->LoadDB();

	Json::Value val = this->GetKey( db, scope, key );

	if( val.isString() )
	{
		 return val.asString();
	}

	logg << Logger::Error << "Key " << key << " not of wanted type" << lend;
	throw runtime_error("Key not of wanted type in config db");
}

list<string> SysConfig::GetKeyAsStringList(const string &scope, const string &key)
{
	Json::Value db = this->LoadDB();

	Json::Value val = this->GetKey( db, scope, key );

	if( val.isArray() )
	{
		list<string> ret;
		for( Json::ArrayIndex i = 0; i < val.size(); i++ )
		{
			if( val[i].isString() )
			{
				ret.push_back(val[i].asString() );
			}
			else
			{
				logg << Logger::Error << "ConfigDB: unexpected data type in list"<<lend;
				throw runtime_error("Unexpected data type in list");
			}
		}
		return ret;
	}

	logg << Logger::Error << "Key " << key << " not of wanted type" << lend;
	throw runtime_error("Key not of wanted type in config db");
}

int SysConfig::GetKeyAsInt(const string &scope, const string &key)
{
	Json::Value db = this->LoadDB();

	Json::Value val = this->GetKey( db, scope, key );

	if( val.isIntegral() )
	{
		 return val.asInt();
	}

	logg << Logger::Error << "Key " << key << " not of wanted type" << lend;

	throw runtime_error("Key not of wanted type in config db");
}

list<int> SysConfig::GetKeyAsIntList(const string &scope, const string &key)
{
	Json::Value db = this->LoadDB();

	Json::Value val = this->GetKey( db, scope, key );

	if( val.isArray() )
	{
		list<int> ret;
		for( Json::ArrayIndex i = 0; i < val.size(); i++ )
		{
			if( val[i].isInt() )
			{
				ret.push_back(val[i].asInt() );
			}
			else
			{
				logg << Logger::Error << "ConfigDB: unexpected data type in list"<<lend;
				throw runtime_error("Unexpected data type in list");
			}
		}
		return ret;
	}

	logg << Logger::Error << "Key " << key << " not of wanted type" << lend;
	throw runtime_error("Key not of wanted type in config db");
}

bool SysConfig::GetKeyAsBool(const string &scope, const string &key)
{
	Json::Value db = this->LoadDB();

	Json::Value val = this->GetKey( db, scope, key );

	if( val.isBool() )
	{
		 return val.asBool();
	}

	logg << Logger::Error << "Key " << key << " not of wanted type" << lend;

	throw runtime_error("Key not of wanted type in config db");
}

list<bool> SysConfig::GetKeyAsBoolList(const string &scope, const string &key)
{
	Json::Value db = this->LoadDB();

	Json::Value val = this->GetKey( db, scope, key );

	if( val.isArray() )
	{
		list<bool> ret;
		for( Json::ArrayIndex i = 0; i < val.size(); i++ )
		{
			if( val[i].isBool() )
			{
				ret.push_back(val[i].asBool() );
			}
			else
			{
				logg << Logger::Error << "ConfigDB: unexpected data type in list"<<lend;
				throw runtime_error("Unexpected data type in list");
			}
		}
		return ret;
	}

	logg << Logger::Error << "Key " << key << " not of wanted type" << lend;
	throw runtime_error("Key not of wanted type in config db");

}

void SysConfig::PutKey(const string &scope, const string &key, const string &value)
{
	this->OpenDB();

	Json::Value db = this->ReadDB();

	db[scope][key]=value;

	this->WriteDB(db);

	this->CloseDB();
}

void SysConfig::PutKey(const string &scope, const string &key, const list<string> &value)
{
	this->OpenDB();

	Json::Value db = this->ReadDB();

	Json::Value l(Json::arrayValue);

	for(const auto& val: value)
	{
		l.append(val);
	}

	db[scope][key] = l;

	this->WriteDB(db);

	this->CloseDB();
}

void SysConfig::PutKey(const string &scope, const string &key, int value)
{
	this->OpenDB();

	Json::Value db = this->ReadDB();

	db[scope][key]=value;

	this->WriteDB(db);

	this->CloseDB();
}

void SysConfig::PutKey(const string &scope, const string &key, const list<int> &value)
{
	this->OpenDB();

	Json::Value db = this->ReadDB();

	Json::Value l(Json::arrayValue);

	for(const auto& val: value)
	{
		l.append(val);
	}

	db[scope][key] = l;

	this->WriteDB(db);

	this->CloseDB();
}

void SysConfig::PutKey(const string &scope, const string &key, bool value)
{
	this->OpenDB();

	Json::Value db = this->ReadDB();

	db[scope][key]=value;

	this->WriteDB(db);

	this->CloseDB();
}

void SysConfig::PutKey(const string &scope, const string &key, const list<bool> &value)
{
	this->OpenDB();

	Json::Value db = this->ReadDB();

	Json::Value l(Json::arrayValue);

	for(const auto& val: value)
	{
		l.append(val);
	}

	db[scope][key] = l;

	this->WriteDB(db);

	this->CloseDB();
}

void SysConfig::RemoveKey(const string &scope, const string &key)
{
	this->OpenDB();

	Json::Value db = this->ReadDB();

	if( this->HasKey( db, scope, key ) )
	{
		db[scope].removeMember(key);
		if(db[scope].empty() )
		{
			db.removeMember(scope);
		}

	}
	this->WriteDB(db);
	this->CloseDB();
}

bool SysConfig::HasKey(const string &scope, const string &key)
{
	Json::Value db = this->LoadDB();

	bool res = this->HasKey( db, scope, key);

	this->CloseDB();

	return res;
}

bool SysConfig::HasScope(const string &scope)
{
	Json::Value db = this->LoadDB();

	bool res = this->HasScope( db, scope );

	this->CloseDB();

	return res;
}

SysConfig::~SysConfig()
{
	this->CloseDB();
}

Json::Value SysConfig::LoadDB()
{

	this->OpenDB();

	Json::Value val = this->ReadDB();

	this->CloseDB();

	return val;
}

Json::Value SysConfig::GetKey(const Json::Value &db, const string &scope, const string &key)
{
	if( this->HasKey(db, scope, key)  )
	{
		return db[scope][key];
	}
	else
	{
		logg << Logger::Error << "ConfigDB: No such key [" << key << "] or scope [" << scope << "] in database" << lend;
	}
	throw runtime_error("ConfigDB: key or scope not found");
}

bool SysConfig::DBExists()
{
	return File::FileExists( SYSCONFIGDBPATH );
}

bool SysConfig::HasScope(const Json::Value &val, const string &scope)
{
	return val.isMember( scope ) && val[scope].isObject();
}

bool SysConfig::HasKey(const Json::Value &val, const string &scope, const string &key)
{
	return this->HasScope(val, scope) && val[scope].isMember(key);
}

void SysConfig::OpenDB()
{
	if( this->fd > 0 )
	{
		throw runtime_error("Configdb already open");
	}

	int flags = this->writeable ? O_RDWR|O_CREAT : O_RDONLY;

    if((this->fd=open(this->path.c_str(), flags, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))<0)
	{
		logg << Logger::Error << "Unable to open file sysconfig database"<< lend;
		throw ErrnoException("Unable to open file '"+path+"'");
	}

	if( flock(this->fd, LOCK_EX) == -1 )
	{
		close( this->fd );
		this->fd = 0;
		logg << Logger::Error << "Unable to lock sysconfig database"<< lend;
		throw ErrnoException("Unable to lock file '"+ this->path +"'");
	}

}

void SysConfig::CloseDB()
{
	if ( this->fd > 0 )
	{
		close( this->fd );
		this->fd = 0;
	}
}

Json::Value SysConfig::ReadDB()
{
	Json::Value val;
	struct stat st;

	if(fstat(this->fd,&st))
	{
		return val;
	}

	// Empty file
	if( st.st_size == 0 )
	{
		return val;
	}

	// Make sure we have room for data
	vector<char> data(st.st_size);
	data.resize(st.st_size);


	size_t read_total = 0;
	ssize_t bytes_read;
	do
	{
		bytes_read = read(fd, &data[read_total], st.st_size - read_total);
		if(bytes_read < 0 )
		{
			logg << Logger::Error << "Unable to read sysconfig database"<< lend;
			throw ErrnoException("Failed to read file '"+path+"'");
		}
		if( bytes_read > 0 )
		{
			read_total += bytes_read;
		}

	}while( bytes_read>0);

	Json::Reader reader;

	if( ! reader.parse(&data.front(), &data.back(), val, false) )
	{
		logg << Logger::Error << "Failed to parse sysconfig datbaase"<<lend;
	}

	return val;
}

void SysConfig::WriteDB(const Json::Value &db)
{
	if( ! this->writeable )
	{
		logg << Logger::Error << "SysConfig: tried to write readonly sysconfig" << lend;
		throw std::runtime_error("Attempt to write readonly config database");
	}

	if( lseek( this->fd, SEEK_SET, 0 ) < 0 )
	{
		throw ErrnoException("ConfigDB: failed to rewind db-file before write");
	}

	if( ftruncate( this->fd, 0 ) < 0 )
	{
		throw ErrnoException("ConfigDB: failed to truncate db-file before write");
	}

	Json::StyledWriter writer;
	string out = writer.write( db );

	__gnu_cxx::stdio_filebuf<char> fb(this->fd,std::ios_base::out);
	iostream of(&fb);

	of<< out <<flush;
}

} // END NameSpace OPI
