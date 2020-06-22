#ifndef HOSTSCONFIG_H
#define HOSTSCONFIG_H

#include <list>
#include <string>
#include <memory>

#include "NetworkConfig.h"

using namespace std;

namespace OPI {

struct HostEntry;

typedef shared_ptr<HostEntry> HostEntryPtr;

struct HostEntry
{
	static HostEntryPtr MakeShared(){ return HostEntryPtr(new HostEntry); }

	int id;
	shared_ptr<NetUtils::IPNetwork> address;
	string hostname;
	list<string> aliases;
};



class HostsConfig
{
public:
	HostsConfig(string  hostfilepath = "/etc/hosts");

	void AddEntry(HostEntryPtr &e);
	void AddEntry(const string& ip, const string& hostname, const list<string>& aliases = {});
	void UpdateEntry(HostEntryPtr &e);
	void DeleteEntry(HostEntryPtr &e);

	HostEntryPtr GetEntry(const string& hostname);
	HostEntryPtr GetEntryByAddress(const string& address);
	HostEntryPtr GetEntry(const NetUtils::IPNetworkPtr& address);

	void Dump();

	void WriteBack();

	virtual ~HostsConfig() = default;
private:
	void parse();

	list<HostEntryPtr> entries;
	int idcounter = 0;
	string path;
};


} // Namespace OPI


#endif // HOSTSCONFIG_H
