
#include <algorithm>
#include <iostream>
#include <sstream>

#include <cstdlib>
#include <utility>

#include "NetworkConfig.h"

#include <libutils/Logger.h>
#include <libutils/String.h>
#include <libutils/Process.h>
#include <libutils/NetUtils.h>
#include <libutils/FileUtils.h>

#include "JsonHelper.h"

namespace OPI
{

namespace NetUtils
{

using namespace Utils;


constexpr const int IPV4_NETWIDTH=32;

static uint32_t toval(Ipv4Address& addr)
{
	return (addr[0] << 24) | (addr[1] << 16) | (addr[2]<<8) | addr[3];
}

constexpr static uint32_t toval(uint8_t net)
{
	return (((uint64_t)1 << net) -1) << (IPV4_NETWIDTH-net);
}

static Ipv4Address toaddress(uint8_t netk)
{
	uint32_t addr = toval(netk);
	auto *val = (uint8_t*) &addr;
	return {val[3],val[2],val[1],val[0]};
}

IPv4Network::IPv4Network(uint8_t net)
{
	this->address = toaddress(net);
}

IPv4Network::IPv4Network(const string &addr)
{
	// First assume we have a dotted address
	vector<string> apart;
	String::Split(addr, apart, ".");
	if( apart.size() == 4 )
	{
		this->address[0] = std::stoi(apart[0]);
		this->address[1] = std::stoi(apart[1]);
		this->address[2] = std::stoi(apart[2]);
		this->address[3] = std::stoi(apart[3]);
	}
	else
	{
		// Lets try to parse this as a net-specifier part
		// i.e. x.x.x.x/(YYY)
		this->address  = toaddress(std::stoi(addr));
	}
}

IPv4Network::IPv4Network(const Ipv4Address &addr): address(addr)
{
}

uint8_t IPv4Network::asNetwork()
{
	uint32_t val = toval(this->address);

	if( val == 0)
	{
		return 0;
	}

	int net = 0;

	// count ones
	do{
		net++;
	}while( (val=val<<1) );

	return net;
}

Ipv4Address IPv4Network::asAddress()
{
	return this->address;
}

std::string IPv4Network::asString()
{
	stringstream ss;
	ss << (int) this->address[0]
			<< "." << (int)this->address[1]
			<< "." << (int)this->address[2]
			<< "." << (int)this->address[3];

	return ss.str();
}



DebianNetworkConfig::DebianNetworkConfig(string path): NetworkConfig(), path(std::move(path))
{
	if( ! File::FileExists( this->path ) )
	{
		logg << Logger::Error << "Network configfile " << path << " not found"<<lend;
		throw std::runtime_error("Network configfile not found");
	}
	this->parse();
}

void DebianNetworkConfig::SetDHCP(const string &iface)
{
	if( ! this->cfg.contains( iface ) )
	{
		logg << Logger::Error << "Interface "<< iface<< " not found" << lend;
		throw std::runtime_error("Interface not found");
	}
	json v;
	v["addressing"]="dhcp";
	v["auto"]=true;

	this->cfg[iface] = v;
	this->dnslist.clear();
}

void DebianNetworkConfig::SetStatic(const string &iface, const string &ip, const string &nm, const string &gw, const list<string> &dnss)
{
	if( ! this->cfg.contains( iface ) )
	{
		logg << Logger::Error << "Interface "<< iface<< " not found" << lend;
		throw std::runtime_error("Interface not found");
	}
	json v;
	v["addressing"]="static";
	v["auto"]=true;
	v["options"]["address"].push_back(ip);
	v["options"]["netmask"].push_back(nm);
	v["options"]["dns"] = JsonHelper::ToJsonArray(dnss);
	if( gw != "" )
	{
		v["options"]["gateway"].push_back(gw);
	}

	this->cfg[iface] = v;

	this->dnslist = dnss;
}

void DebianNetworkConfig::Dump()
{
	cout << this->cfg.dump(4)<<endl;
}

void DebianNetworkConfig::WriteConfig()
{
	stringstream ss;

	// Start with any initial globals
	if( this->fileglobals.contains("initial") )
	{
		for(const auto& line: this->fileglobals["initial"])
		{
			if( line.is_string() )
			{
				ss << line.get<string>() << "\n";
			}
		}
		ss <<"\n";
	}

	for( const auto& iface: this->cfg.items() )
	{
		json ifc = iface.value(); //this->cfg[iface.key()];
		if( ifc.contains("auto") && ifc["auto"].get<bool>() )
		{
			ss << "auto " << iface.key()<<endl;
		}

		if( ifc.contains("allow-hotplug") && ifc["allow-hotplug"].get<bool>() )
		{
			ss << "allow-hotplug " << iface.key()<<endl;
		}

		ss << "iface " << iface.key() << " inet "<< ifc["addressing"].get<string>()<<endl;

		for( const auto& opt: ifc["options"].items() )
		{
			// Cludge, dont write dns-option to config
			if( opt.key() == "dns")
			{
				continue;
			}

			ss << "\t"<<opt.key();
			for( const auto& optval: ifc["options"][opt.key()])
			{
				ss << " " << optval.get<string>();
			}
			ss << endl;
		}
		ss<< endl;
	}
	File::Write( this->path, ss.str(), File::UserRW | File::GroupRead | File::OtherRead );


	// Only write dns-server config if servers present
	// Not sure this is the "right" thing to do though
	if( this->dnslist.size()>0)
	{
		NetUtils::ResolverConfig rc;
		rc.setDomain("localdomain");
		rc.setSearch("");
		rc.setNameservers(this->dnslist);
		rc.WriteConfig();
	}

}


bool DebianNetworkConfig::RestartInterface(const string &interface)
{
	const string upcmd("/sbin/ifup ");
	const string downcmd("/sbin/ifdown ");

	bool tmp = false, ret = false;
	tie(ret, ignore) = Process::Exec( downcmd + interface );
	tie(tmp, ignore) = Process::Exec( upcmd + interface );
	ret = ret && tmp;

	tmp = NetworkConfig::RestartInterface(interface);

	return ret && tmp;
}


void DebianNetworkConfig::parse()
{

	// Read dns servers
	NetUtils::ResolverConfig rc;
	this->dnslist = rc.getNameservers();


	list<string> lines = File::GetContent( this->path );
	list<string> autoifs;
	list<string> hotplug;

	string cif; // Current interface;
	for( string line: lines)
	{
		line = String::Trimmed(line, " \t");
		if( line == "" || line[0]=='#')
		{
			continue;
		}

		list<string> words = String::Split(line, " ");
		if( words.size() > 1 )
		{
			if( words.front() == "source" )
			{
				this->fileglobals["initial"].push_back(line);
			}
			else if( words.front() == "auto")
			{
				words.pop_front();
				autoifs.insert(autoifs.end(), words.begin(), words.end() );
			}
			else if( words.front() == "allow-hotplug" )
			{
				words.pop_front();
				hotplug.insert(hotplug.end(), words.begin(), words.end() );
			}
			else if( words.front() == "iface" )
			{
				words.pop_front();
				cif = words.front();
				this->cfg[cif]["addressing"] = words.back();
				// Asume no auto nor hotplug for the time being
				this->cfg[cif]["auto"]=false;
				this->cfg[cif]["allow-hotplug"]=false;

				// Cludge to get dns-servers to UI, add to all ifaces
				this->cfg[cif]["options"]["dns"] = JsonHelper::ToJsonArray(this->dnslist);
			}
			else
			{
				// asume option to iface
				string key = words.front();
				words.pop_front();
				for( const auto& option: words)
				{
					this->cfg[cif]["options"][key].push_back(option);
				}
			}
		}

	}
	// Add auto on ifs using it
	for( const auto& ifs: autoifs)
	{
		this->cfg[ifs]["auto"]=true;
	}

	// Add hotplug on ifs using it
	for( const auto& hotifs: hotplug)
	{
		this->cfg[hotifs]["allow-hotplug"]=true;
	}

}


ResolverConfig::ResolverConfig(string path): path(std::move(path))
{
	this->parse();
}

ResolverConfig::~ResolverConfig()
= default;
string ResolverConfig::getDomain() const
{
	return this->domain;
}

void ResolverConfig::setDomain(const string &value)
{
	this->domain = value;
}
string ResolverConfig::getSearch() const
{
	return this->search;
}

void ResolverConfig::setSearch(const string &value)
{
	this->search = value;
}
list<string> ResolverConfig::getNameservers() const
{
	return this->nss;
}

void ResolverConfig::setNameservers(const list<string> &value)
{
	this->nss = value;
}

void ResolverConfig::WriteConfig()
{
	stringstream ss;

	if( this->domain != "")
	{
		ss << "domain "<< this->domain<<endl;
	}

	if( this->search != "")
	{
		ss << "search "<< this->search<<endl;
	}

	for( const auto& ns: this->nss )
	{
		ss << "nameserver "<< ns<<endl;
	}

	try
	{
		File::Write( this->path, ss.str(), File::UserRW | File::GroupRead | File::OtherRead );
	}
	catch (ErrnoException& err)
	{
		cerr << "Failed to write resolv.conf: " << err.what() << endl;
	}

}

void ResolverConfig::Dump()
{
	cout << "Search : "<< this->search<<endl;
	cout << "Domain : "<< this->domain<<endl;
	for( const auto& x: this->nss )
	{
		cout << "Nameserver: "<<x<<endl;
	}
}

void ResolverConfig::parse()
{
	list<string> lines = File::GetContent( this->path );

	for( string line: lines)
	{
		line = String::Trimmed(line, " \t");
		if( line == "" || line[0]=='#')
		{
			continue;
		}

		list<string> words = String::Split(line, " ");
		if( words.size() > 1 )
		{
			if( words.front() == "nameserver")
			{
				this->nss.push_back( words.back() );
			}
			else if( words.front() == "search" )
			{
				this->search = words.back();
			}
			else if( words.front() == "domain" )
			{
				this->domain = words.back();
			}
		}

	}

}


#include <endian.h>

static int hextoval(char a, char b){
		a=toupper(a)>57?a-55:a-48;
		b=toupper(b)>57?b-55:b-48;
		return a*16+b;
}

static string addresstostring(const string& address){

		char buf[128];
#if __BYTE_ORDER == __LITTLE_ENDIAN
		sprintf(buf,"%d.%d.%d.%d",
								hextoval(address[6],address[7]),
								hextoval(address[4],address[5]),
								hextoval(address[2],address[3]),
								hextoval(address[0],address[1]));
#elif __BYTE_ORDER == __BIG_ENDIAN
		sprintf(buf,"%d.%d.%d.%d",
								hextoval(address[0],address[1]),
								hextoval(address[2],address[3]),
								hextoval(address[4],address[5]),
								hextoval(address[6],address[7]));
#else
		#error "No endian defined"
#endif


		return string(buf);
}


static tuple<string,string> getDefaults()
{
	constexpr uint8_t ROUTECOLS = 11;
	string defgateway, definterface;
	list<string> fil = File::GetContent("/proc/net/route");
	fil.pop_front();
	for( const auto& row: fil){
		list<string> line= String::Split( row, "\t");
		if(line.size() == ROUTECOLS){
			auto lIt=line.begin();
			string iface = *lIt++;
			string destination = addresstostring(*lIt++);
			string gateway = addresstostring(*lIt++);

			if( destination == "0.0.0.0" )
			{
				defgateway = gateway;
				definterface = iface;
			}

		}
	}
	return {defgateway, definterface};
}


string GetDefaultDevice()
{
	string device;
	tie(std::ignore, device) = getDefaults();

	return device;
}


string GetDefaultRoute()
{
	string gw;
	tie(gw, std::ignore) = getDefaults();

	return gw;
}

string GetNetmask(const string& ifname)
{
	try
	{
		struct sockaddr addr = Utils::Net::GetNetmask(ifname);
		return Utils::Net::SockAddrToString(&addr);
	}
	catch (Utils::ErrnoException& err)
	{
		(void) err;
		return "";
	}
}

string GetAddress(const string& ifname)
{
		try
		{
			struct sockaddr addr = Utils::Net::GetIfAddr(ifname);

			return Utils::Net::SockAddrToString( &addr );
		}
		catch (Utils::ErrnoException& err)
		{
			(void) err;
			return "";
		}
}

RaspbianNetworkConfig::RaspbianNetworkConfig(string path): NetworkConfig(), path(std::move(path))
{
	this->Parse();
}

void RaspbianNetworkConfig::SetDHCP(const string &iface)
{
	if( ! this->cfg.contains(iface) || !this->cfg[iface].is_object() )
	{
		logg << Logger::Error << "Interface "<< iface<< " not found" << lend;
		throw std::runtime_error(string("Unknown interface ")+iface);
	}

	if( this->cfg[iface]["addressing"] == "static" )
	{
		this->cfg[iface]["addressing"]="dhcp";

		if( this->cfg[iface].contains("options") )
		{
			this->cfg[iface].erase("options");
		}
	}
}

void RaspbianNetworkConfig::SetStatic(const string &iface, const string &ip,
									  const string &nm, const string &gw,
									  const list<string>& dnss)
{

	if( ! this->cfg.contains(iface) || !this->cfg[iface].is_object() )
	{
		logg << Logger::Error << "Interface "<< iface<< " not found" << lend;
		throw std::runtime_error(string("Unknown interface ")+iface);
	}
	string adr = this->cfg[iface]["addressing"].get<string>();
	if( adr == "dhcp" || adr =="static" )
	{
		this->cfg[iface]["addressing"]="static";

		if( this->cfg[iface].contains("options") )
		{
			this->cfg[iface].erase("options");
		}

		this->cfg[iface]["options"]["address"].push_back(ip);
		this->cfg[iface]["options"]["netmask"].push_back(nm);
		this->cfg[iface]["options"]["gateway"].push_back(gw);

		for(const auto& dns: dnss)
		{
			this->cfg[iface]["options"]["dns"].push_back(dns);
		}

	}

}

void RaspbianNetworkConfig::WriteStaticEntry(stringstream &ss, const string &member)
{
	json item = this->cfg[member];

	if( !item.contains("options") || !item["options"].is_object() )
	{
		logg << Logger::Error << "Malformed network entry in dhcpcd config" << lend;
		throw std::runtime_error("Malformed network entry in dhcpcd config");
	}

	ss << "\n# Static configuration for " << member <<"\n";
	ss << "interface "<<member<<"\n";

	for( const auto& mem: item["options"].items())
	{

		if( mem.key() == "netmask" )
		{
			// Processed with address
			continue;
		}
		else if( mem.key() == "address" )
		{
			if( ! item["options"].contains("netmask") )
			{
				logg << Logger::Error << "Missing netmask in network configuration" << lend;
				throw std::runtime_error("Missing netmask in network configuration");
			}
			IPv4Network addr(item["options"]["address"][0].get<string>());
			IPv4Network nm(item["options"]["netmask"][0].get<string>());
			ss << "static ip_address=" << addr.asString() << "/" << std::to_string(nm.asNetwork())<<"\n";
		}
		else if( mem.key() == "gateway")
		{
			bool first = true;
			ss << "static routers=";
			for( const auto& router: item["options"]["gateway"])
			{
				ss << (first?"":" ") << router.get<string>();
				first=false;
			}
			ss << "\n";
		}
		else if( mem.key() == "dns")
		{
			bool first = true;
			ss << "static domain_name_servers=";
			for( const auto& dns: item["options"]["dns"])
			{
				ss << (first?"":" ") << dns.get<string>();
				first = false;
			}
			ss << "\n";
		}
		else
		{
			// Unknown parameter just copy back
			ss << "static " << mem.key() << "="<< item["options"][item.get<string>()][0].get<string>()  <<"\n";
		}
	}

}


void RaspbianNetworkConfig::WriteConfig()
{
	stringstream ss;

	ss << "# This is an autogenerated file.\n"
	   << "# Any structure and all comments will be lost upon update\n\n";

	// Start with all "unknown" options
	if( this->cfg.contains("other") && this->cfg["other"].is_array() )
	{
		for( const auto& line: cfg["other"])
		{
			ss << line << "\n";
		}
	}

	//json::Members mems = this->cfg.getMemberNames();
	for( const auto& item: this->cfg.items())
	{
		if( item.key() == "other" )
		{
			// Skip already processed section
			continue;
		}

		//if( this->cfg[mem.get<string>()]["addressing"].get<string>() != "static" )
		if( item.value()["addressing"].get<string>() != "static" )
		{
			// Skip all none static mappings
			continue;
		}

		this->WriteStaticEntry(ss, item.key());
	}

	File::Write(this->path, ss.str(), File::UserRW | File::GroupRead | File::OtherRead);
}

bool RaspbianNetworkConfig::RestartInterface(const string &interface)
{
	const string upcmd("/sbin/ip link set " + interface + " up");
	const string downcmd("/sbin/ip link set " + interface + " down");

	bool tmp = false, ret = false;
	tie(ret, ignore) = Process::Exec( downcmd );
	tie(tmp, ignore) = Process::Exec( upcmd );
	ret = ret && tmp;

	tmp = NetworkConfig::RestartInterface(interface);

	return ret && tmp;
}

void RaspbianNetworkConfig::Dump()
{
	cout << this->cfg.dump(4)<<endl;
}


/*
 * Translate dhcpcd options to frontend expected key/value pairs
 */
void RaspbianNetworkConfig::ProcessOption(const string &iface, const string &key, const string &value)
{

	if( key == "ip_address" )
	{
		list<string> addr = String::Split(value,"/",2);
		if( addr.size() == 2 )
		{
			this->cfg[iface]["options"]["address"].push_back(addr.front());

			IPv4Network net(std::stoi(addr.back()));
			this->cfg[iface]["options"]["netmask"].push_back( net.asString() );
		}
		else
		{
			throw std::runtime_error("Failed to parse ipv4 address");
		}
	}
	else if( key == "routers" )
	{
		this->cfg[iface]["options"]["gateway"].push_back(value);
	}
	else if( key == "domain_name_servers")
	{
		this->cfg[iface]["options"]["dns"]= json::array();
		list<string> dnss = String::Split(value, " ");
		for( const auto& dns: dnss)
		{
			this->cfg[iface]["options"]["dns"].push_back(dns);
		}
	}
	else
	{
		this->cfg[iface]["options"][key].push_back( value );
	}

}

void RaspbianNetworkConfig::Parse()
{
	if( ! File::FileExists(this->path) )
	{
		logg << Logger::Error << "Config file "<< this->path << " not found" << lend;
		throw std::runtime_error(string("Config file not found: ")+this->path);
	}
	this->cfg["other"]=json::array();
	try {
		list<string> lines = File::GetContent(this->path);

		string cif; // Current interface
		for(auto& line :lines)
		{
			line = String::Trimmed(line, " \t");
			if( line == "" || line[0] == '#')
			{
				continue;
			}

			list<string> words = String::Split(line, " ",2);

			if( words.front() == "interface" )
			{
				cif = words.back();
			}
			else if( words.front() == "static" )
			{
				this->cfg[cif]["addressing"] = "static";
				list<string> option = String::Split( words.back(), "=", 2);
				this->ProcessOption(cif, option.front(), option.back() );
			}
			else
			{
				this->cfg["other"].push_back(line);
			}
		}

	}
	catch ( std::runtime_error& err)
	{
		logg << Logger::Notice << "Caught exception: "<< err.what()<<lend;

	}

}

list<string> GetInterfaces()
{
	return Utils::Net::GetInterfaces();
}

NetworkConfig::NetworkConfig()
{
	// Get all network devices on system and add to config

	list<string> ifs = Utils::Net::GetInterfaces();

	for(const auto& iface: ifs)
	{
		this->cfg[iface] = json();
		if( iface == "lo" )
		{
			this->cfg[iface]["addressing"]="loopback";
		}
		else
		{
			// We start of by assuming all ifs are dynamically configured
			this->cfg[iface]["addressing"]="dhcp";
		}

	}

}

json NetworkConfig::GetInterface(const string &iface)
{
	if( ! this->cfg.contains( iface ) )
	{
		logg << Logger::Error << "Interface " << iface << " not found" << lend;
		throw std::runtime_error("Interface not found");
	}
	return this->cfg[iface];

}

json NetworkConfig::GetInterfaces()
{
	return this->cfg;
}

bool NetworkConfig::RestartInterface(const string &interface)
{
	(void) interface;
	bool tmp = false;

	tie(tmp, ignore) = Process::Exec( "/usr/share/opi-access/opi-access.sh" );

	return tmp;
}

void NullConfig::SetDHCP(const string &iface)
{
	(void) iface;

	logg << Logger::Notice << "Set DHCP config for " << iface << " called on nullconfig" << lend;

}

void NullConfig::SetStatic(const string &iface, const string &ip, const string &nm, const string &gw, const list<string> &dnss)
{
	(void) ip;
	(void) nm;
	(void) gw;
	(void) dnss;
	logg << Logger::Notice << "Set static config for " << iface << " called on nullconfig" << lend;
}

void NullConfig::WriteConfig()
{
	logg << Logger::Notice << "Write config called on nullconfig" << lend;
}

bool NullConfig::RestartInterface(const string &interface)
{
	logg << Logger::Notice << "Trying to restart " << interface << " on nulldevice" << lend;

	return false;
}

static constexpr int HexVal = 16;
static constexpr int Ip6Len = 8;


IPv6Network::IPv6Network(const Ipv6Address &addr): address(addr)
{

}

IPv6Network::IPv6Network(const string &addr)
{
	string::size_type pos = addr.find("::");

	if( pos != string::npos )
	{
		// This seems to be a "folded" address

		string front = addr.substr(0,pos);
		string back = addr.substr(pos+2);

		list<string> fvals = String::Split(front, ":");
		int i=0;
		for(const auto& val: fvals)
		{
			this->address[i++] = std::stoi(val, nullptr, HexVal);
		}

		list<string> bvals = String::Split(back, ":");
		i=Ip6Len-1;
		for( auto iT = bvals.rbegin(); iT != bvals.rend(); iT++)
		{
			this->address[i--] = std::stoi( (*iT), nullptr, HexVal );
		}

	}
	else
	{
		// Seems to be an full unfolded address
		list<string> fvals = String::Split(addr, ":");
		if( fvals.size() != Ip6Len )
		{
			logg << Logger::Error << "Malformed IPv6 address!" << lend;
			throw std::runtime_error("Malformed IPv6 address");
		}
		int i=0;
		for(const auto& val: fvals)
		{
			this->address[i++] = std::stoi(val, nullptr, HexVal);
		}
	}
}

IPv6Network::IPv6Network(uint8_t net)
{
	for(int i = 0; i<net; i++ )
	{
		this->address[i/16] |= 1 << (15-i%16);
	}
}

uint8_t IPv6Network::asNetwork()
{
	// Count all 1-bits from left to right
	uint8_t ret = 0;
	bool one = true;
	for( int i = 0; (i< Ip6Len) && one; i++)
	{
		for( int j = 15; (j>=0) && one; j-- )
		{
			one = ( this->address[i] & (1<<j) );
			if( one  )
			{
				ret++;
			}
		}
	}

	return ret;
}

Ipv6Address IPv6Network::asAddress()
{
	return this->address;
}

string IPv6Network::asString()
{
	stringstream ss;
	uint8_t statc = 0, longest = 0;
	int8_t at =-1, fold_start = -1, fold_end = -1;

	// Count zeros to see if we could fold this
	for( int i =0; i < Ip6Len; i++ )
	{
		if( this->address[i] == 0 )
		{
			statc++;
			if( statc > longest )
			{
				longest = statc;
				at = i;
			}
		}
		else
		{
			statc=0;
		}
	}

	// Calculate fold if found
	if( longest > 1 )
	{
		fold_start = at+1-longest;
		fold_end = at;
	}

	bool firstval = true, firstfold = true;
	for( int i = 0; i < Ip6Len; i++ )
	{
		if( i >= fold_start && i <= fold_end)
		{
			if( firstfold )
			{
				ss << "::";
				firstfold = false;
				firstval = true;
			}
		}
		else
		{
			ss << (firstval?"":":");
			firstval = false;
			ss << hex << this->address[i] ;
		}
	}

	return ss.str();
}

#include <arpa/inet.h>

bool IsIPv4address(const string &addr)
{
		struct in_addr ip4 = {};
		return inet_pton(AF_INET, addr.c_str(), &ip4) == 1;
}

bool IsIPv6address(const string &addr)
{
	struct in6_addr ip6 = {};
	return inet_pton(AF_INET6, addr.c_str(), &ip6) == 1;
}

} // End namespace
} // End namespace
