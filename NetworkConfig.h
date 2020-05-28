#ifndef NETWORKCONFIG_H
#define NETWORKCONFIG_H

#include <json/json.h>

#include <string>
#include <list>

namespace OPI
{

namespace NetUtils
{

using namespace std;

using Ipv4Address = std::array<uint8_t, 4>;

class IPv4Network
{
public:
	IPv4Network() = default;

	/**
	 * @brief Network create object from a network spec ie x.x.x.x/net
	 * @param net
	 */
	IPv4Network(uint8_t net);

	IPv4Network(const string& addr);

	IPv4Network(const Ipv4Address& addr);

	uint8_t asNetwork();

	Ipv4Address asAddress();

	std::string asString();

	virtual ~IPv4Network() = default;
private:
	Ipv4Address address = {0,0,0,0};
};

class NetworkConfig
{
public:

	NetworkConfig();

	virtual Json::Value GetInterface(const string& iface);
	virtual Json::Value GetInterfaces();

// We really dont want to allow this since it most likely
// will overwrite housekeeping info specific to device.
#if 0
	virtual void SetInterface(const string &iface, const Json::Value& v) = 0;
#endif
	virtual void SetDHCP( const string& iface) = 0;
	virtual void SetStatic(const string& iface,
					const string& ip,
					const string& nm,
					const string& gw = "",
					const list<string>& dnss = {}) = 0;

	virtual void WriteConfig() = 0;

	virtual bool RestartInterface(const string& interface);

	virtual ~NetworkConfig() = default;
protected:
	Json::Value cfg;
};

class NullConfig: public NetworkConfig
{
public:
	NullConfig() = default;

	virtual ~NullConfig() = default;

	// NetworkConfig interface
	void SetDHCP(const string &iface) override;
	void SetStatic(const string &iface, const string &ip, const string &nm, const string &gw, const list<string> &dnss) override;
	void WriteConfig() override;
	bool RestartInterface(const string &interface) override;
};

class DebianNetworkConfig: public NetworkConfig
{
public:
	DebianNetworkConfig(string  path = "/etc/network/interfaces");

	void SetDHCP( const string& iface) override;
	void SetStatic(const string& iface,
					const string& ip,
					const string& nm,
					const string& gw = "",
				   const list<string>& dnss = {}) override;
	void Dump();

	void WriteConfig() override;

	bool RestartInterface(const string &interface) override;

	virtual ~DebianNetworkConfig() = default;
private:
	string path;
	list<string> dnslist;

	void parse();
};

class RaspbianNetworkConfig: public NetworkConfig
{
public:
	RaspbianNetworkConfig(const string& path = "/etc/dhcpcd.conf");

	void SetDHCP(const string &iface) override;
	void SetStatic(const string &iface,
				   const string &ip,
				   const string &nm,
				   const string &gw,
				   const list<string>& dnss = {}) override;
	void WriteConfig() override;

	bool RestartInterface(const string &interface) override;

	void Dump();

	virtual ~RaspbianNetworkConfig() = default;
private:
	void WriteStaticEntry(stringstream& ss, const string& member);
	void ProcessOption(const string& iface, const string& key, const string& value);
	void Parse();
	string path = "";
};


class ResolverConfig
{
public:
	ResolverConfig( string  path = "/etc/resolv.conf");

	virtual ~ResolverConfig();

	string getDomain() const;
	void setDomain(const string &value);

	string getSearch() const;
	void setSearch(const string &value);

	list<string> getNameservers() const;
	void setNameservers(const list<string> &value);

	void WriteConfig();

	void Dump();
private:
	string path;
	string domain;
	string search;
	list<string> nss;
	void parse();
};


// Convenience routines

/**
 * @brief GetInterfaces return a list of network devices found in system
 * @return list with interfaces
 */
list<string> GetInterfaces();

string GetAddress(const string& ifname);

string GetNetmask(const string& ifname);

string GetDefaultRoute();

}

} //End namespace

#endif // NETWORKCONFIG_H
