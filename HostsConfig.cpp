#include "HostsConfig.h"

#include <libutils/FileUtils.h>
#include <libutils/String.h>

#include <algorithm>
#include <utility>
#include <sstream>
#include <cctype>

#include <iostream>

using namespace std;

using namespace Utils;

// If String::Split had allowed for multiple separators that would have been enough
// Unfortunately we need skipping of all blanks thus not sufficient.
//
// Something like this should be added to libutils later. Most likely a refactored
// or updated string split. (Perhaps with a regex as seprartor)

class Tokenizer
{
public:
	Tokenizer(string  text): text(std::move(text)){}

	string NextToken()
	{
		string ret;
		this->skipblanks();
		this->skipcoments();
		while( ! this->eol() && !isblank(this->text[this->pos]) )
		{
			ret.push_back(this->text[this->pos++] );
		}
		return ret;
	}

	void Reset()
	{
		this->pos = 0;
	}

private:

	void skipblanks()
	{
		while( ! this->eol() && isblank(this->text[this->pos]) )
		{
			this->pos++;
		}
	}

	void skipcoments()
	{
		if(this->text[this->pos] == '#')
		{
			this->pos = this->text.length();
		}
	}

	bool eol()
	{
		return this->pos == this->text.size();
	}

	string text;
	string::size_type pos =0;
};


namespace OPI
{

HostsConfig::HostsConfig(string hostfilepath): path(std::move(hostfilepath))
{
	this->parse();
}

void HostsConfig::AddEntry(HostEntryPtr &e)
{
	e->id = this->idcounter++;
	this->entries.push_back( e );
}

void HostsConfig::AddEntry(const string &ip, const string &hostname, const list<string> &aliases)
{
	if( ! NetUtils::IsIPv4address(ip) && ! NetUtils::IsIPv6address(ip) )
	{
		throw std::runtime_error("Not an ipv4 or ipv6 address");
	}

	if( hostname == "" )
	{
		throw std::runtime_error("Missing hostname");
	}

	HostEntryPtr e = HostEntry::MakeShared();

	if( NetUtils::IsIPv4address(ip) )
	{
		e->address = shared_ptr<NetUtils::IPNetwork>(new NetUtils::IPv4Network(ip) );
	}
	else
	{
		e->address = shared_ptr<NetUtils::IPNetwork>(new NetUtils::IPv6Network(ip) );
	}

	e->hostname = hostname;

	e->aliases = aliases;

	this->AddEntry(e);
}

void HostsConfig::UpdateEntry(HostEntryPtr &e)
{
	const auto& it = find_if( this->entries.begin(), this->entries.end(),
						[&e](const HostEntryPtr& n){return  n->id == e->id; }
						);

	if( it != this->entries.end() )
	{
		auto pos = this->entries.erase(it);
		this->entries.insert(pos, e);
	}
}

void HostsConfig::DeleteEntry(HostEntryPtr &e)
{
	const auto& it = find_if( this->entries.begin(), this->entries.end(),
						[&e](const HostEntryPtr& n){return  n->id == e->id; }
						);

	if( it != this->entries.end() )
	{
		this->entries.erase(it);
	}
}

HostEntryPtr HostsConfig::GetEntry(const string &hostname)
{
	HostEntryPtr entry;
	const auto& it = find_if( this->entries.begin(), this->entries.end(),
						[&hostname](const HostEntryPtr& n){return  n->hostname == hostname; }
						);


	if( it != this->entries.end() )
	{
		entry = *it;
	}
	return entry;
}

HostEntryPtr HostsConfig::GetEntryByAddress(const string &address)
{
	HostEntryPtr entry;

	const auto& it = find_if( this->entries.begin(), this->entries.end(),
						[&address](const HostEntryPtr& n){
							return  n->address->asString() == address;
						});


	if( it != this->entries.end() )
	{
		entry = *it;
	}

	return entry;
}

HostEntryPtr HostsConfig::GetEntry(const NetUtils::IPNetworkPtr& address)
{
	return this->GetEntryByAddress( address->asString() );
}


void HostsConfig::Dump()
{
	for(const auto& e: this->entries )
	{
		cout << e->id << " "  << e->address->asString() << " " << e->hostname;
		for( const auto& alias: e->aliases)
		{
			cout << " " << alias;
		}
		cout << "\n";
	}
}

void HostsConfig::WriteBack()
{
	stringstream ss;
	for(const auto& e: this->entries )
	{
		ss << e->address->asString() << " " << e->hostname;
		for( const auto& alias: e->aliases)
		{
			ss << " " << alias;
		}
		ss << "\n";
	}

	File::SafeWrite(this->path, ss.str(), 0644);
}

void HostsConfig::parse()
{
	list<string> lines = File::GetContent(this->path);

	this->entries.clear();

	for( const auto& rawline: lines)
	{
		Tokenizer t(rawline);
		HostEntryPtr e = HostEntry::MakeShared();

		string address = t.NextToken();
		if( address == "" )
		{
			// Line with only comment, skip
			continue;
		}

		if( ! NetUtils::IsIPv4address(address) && ! NetUtils::IsIPv6address(address) )
		{
			cerr << "Address not valid " << address << endl;
			continue;
		}

		string host = t.NextToken();
		if( host == "" )
		{
			cerr << "Missing hostname in hosts file!" << endl;
			continue;
		}

		if( NetUtils::IsIPv4address(address) )
		{
			e->address = shared_ptr<NetUtils::IPNetwork>(new NetUtils::IPv4Network(address) );
		}
		else
		{
			e->address = shared_ptr<NetUtils::IPNetwork>(new NetUtils::IPv6Network(address) );
		}

		e->hostname = host;

		string token;
		while( (token = t.NextToken()) != "" )
		{
			e->aliases.push_back(token);
		}

		e->id = this->idcounter++;

		this->entries.push_back(e);
	}

}


} // Namespace OPI

