#ifndef NETWORKCONFIG_H
#define NETWORKCONFIG_H

#include <nlohmann/json.hpp>

#include <memory>
#include <string>
#include <list>

using json = nlohmann::json;

//TODO: This should most likely be refactored to use/interoperate
// with in_addr in6_addr and utilize inet_pton and inet_ntop

namespace OPI
{

namespace NetUtils
{

using namespace std;

// Ipv4Address [0] first value of address i.e. [192].168.1.1
using Ipv4Address = std::array<uint8_t, 4>;
// Ipv6Address [0] first value of address i.e. [ff00]:fe0:fed::0
using Ipv6Address = std::array<uint16_t, 8>;

class IPNetwork
{
public:
	virtual std::string asString() = 0;
};

typedef shared_ptr<IPNetwork> IPNetworkPtr;

class IPv6Network: public IPNetwork
{
public:
	IPv6Network() = default;

	IPv6Network(const Ipv6Address& addr);

	IPv6Network(const string& addr);

	IPv6Network(uint8_t net);

	uint8_t asNetwork();

	Ipv6Address asAddress();

	std::string asString() override;

	virtual ~IPv6Network() = default;
private:
	Ipv6Address address = {0};
};

class IPv4Network: public IPNetwork
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

	std::string asString() override;

	virtual ~IPv4Network() = default;
private:
	Ipv4Address address = {0};
};

class NetworkConfig
{
public:

	NetworkConfig();

	virtual json GetInterface(const string& iface);
	virtual json GetInterfaces();

// We really dont want to allow this since it most likely
// will overwrite housekeeping info specific to device.
#if 0
	virtual void SetInterface(const string &iface, const json& v) = 0;
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
	json cfg;
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
	RaspbianNetworkConfig(string  path = "/etc/dhcpcd.conf");

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
 *
 * @deprecated Please use alternative in Utils::NetUtils
 *
 * @return list with interfaces
 */
[[deprecated]]
list<string> GetInterfaces();

string GetAddress(const string& ifname);

string GetNetmask(const string& ifname);

/**
 * @brief GetDefaultDevice get device that is connected to the default route
 * @return device name or ""
 */
string GetDefaultDevice();

/**
 * @brief GetDefaultRoute get IP address of default gw as string
 * @return GW IP or ""
 */
string GetDefaultRoute();

bool IsIPv4address(const string& addr);

bool IsIPv6address(const string& addr);


}

} //End namespace

#endif // NETWORKCONFIG_H
