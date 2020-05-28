
#include <algorithm>
#include <iostream>
#include <sstream>

#include <cstdlib>

#include "NetworkConfig.h"

#include <libutils/Logger.h>
#include <libutils/String.h>
#include <libutils/Process.h>
#include <libutils/FileUtils.h>

#include "JsonHelper.h"

namespace OPI
{

namespace NetUtils
{

using namespace Utils;


constexpr const int NETWIDTH=32;

static uint32_t toval(Ipv4Address& addr)
{
	return (addr[0] << 24) | (addr[1] << 16) | (addr[2]<<8) | addr[3];
}

constexpr static uint32_t toval(uint8_t net)
{
	return (((uint64_t)1 << net) -1) << (NETWIDTH-net);
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
		logg << Logger::Error << "Netwoek configfile " << path << " not found"<<lend;
		throw std::runtime_error("Network configfile not found");
	}
	this->parse();
}

void DebianNetworkConfig::SetDHCP(const string &iface)
{
	if( ! this->cfg.isMember( iface ) )
	{
		logg << Logger::Error << "Interface "<< iface<< " not found" << lend;
		throw std::runtime_error("Interface not found");
	}
	Json::Value v;
	v["addressing"]="dhcp";
	v["auto"]=true;

	this->cfg[iface] = v;
	this->dnslist.clear();
}

void DebianNetworkConfig::SetStatic(const string &iface, const string &ip, const string &nm, const string &gw, const list<string> &dnss)
{
	if( ! this->cfg.isMember( iface ) )
	{
		logg << Logger::Error << "Interface "<< iface<< " not found" << lend;
		throw std::runtime_error("Interface not found");
	}
	Json::Value v;
	v["addressing"]="static";
	v["auto"]=true;
	v["options"]["address"].append(ip);
	v["options"]["netmask"].append(nm);
	v["options"]["dns"] = JsonHelper::ToJsonArray(dnss);
	if( gw != "" )
	{
		v["options"]["gateway"].append(gw);
	}

	this->cfg[iface] = v;

	this->dnslist = dnss;
}

void DebianNetworkConfig::Dump()
{
	cout << this->cfg.toStyledString()<<endl;
}

void DebianNetworkConfig::WriteConfig()
{
	stringstream ss;

	Json::Value::Members mems = this->cfg.getMemberNames();

	for( const auto& iface: mems)
	{
		Json::Value ifc = this->cfg[iface];
		if( ifc["auto"].asBool() )
		{
			ss << "auto " << iface<<endl;
		}
		ss << "iface " << iface << " inet "<< ifc["addressing"].asString()<<endl;

		Json::Value::Members opts = ifc["options"].getMemberNames();
		for( const auto& opt: opts )
		{
			// Cludge, dont write dns-option to config
			if( opt == "dns")
			{
				continue;
			}

			ss << "\t"<<opt;
			for( const auto& optval: ifc["options"][opt])
			{
				ss << " " << optval.asString();
			}
			ss << endl;
		}
		ss<< endl;
	}
	File::Write( this->path, ss.str(), 0644 );


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

	bool tmp, ret;
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
			if( words.front() == "auto")
			{
				words.pop_front();
				autoifs.insert(autoifs.end(), words.begin(), words.end() );
			}
			else if( words.front() == "iface" )
			{
				words.pop_front();
				cif = words.front();
				this->cfg[cif]["addressing"] = words.back();
				// Asume no auto for the time being
				this->cfg[cif]["auto"]=false;

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
					this->cfg[cif]["options"][key].append(option);
				}
			}
		}

	}
	// Add auto on ifs using it
	for( const auto& ifs: autoifs)
	{
		this->cfg[ifs]["auto"]=true;
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

	try {
		File::Write( this->path, ss.str(), 0644 );

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


string GetDefaultRoute()
{
	string defgateway;
	list<string> fil = File::GetContent("/proc/net/route");
	fil.pop_front();
	for( const auto& row: fil){
		list<string> line= String::Split( row, "\t");
		if(line.size()==11){
			auto lIt=line.begin();
			string iface = *lIt++;
			string destination = addresstostring(*lIt++);
			string gateway = addresstostring(*lIt++);

			if( destination == "0.0.0.0" )
			{
				defgateway = gateway;
			}

		}
	}
	return defgateway;
}
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>

static string sockaddrtostring(struct sockaddr* address)
{
	string ret;
	switch( address->sa_family ){
	case AF_UNIX :
		throw runtime_error("Currently unsupported network class AF_UNIX");
		break;
	case AF_INET:
	{
		struct sockaddr_in *addr = ( struct sockaddr_in* ) address;
		return inet_ntoa( addr->sin_addr );
		break;
	}
	case AF_INET6:
		throw runtime_error("Currently unsupported network class AF_INET6");
		break;
	case AF_UNSPEC:
		throw runtime_error("Currently unsupported network class AF_UNSPEC");
		break;
	default:
		throw runtime_error("Unknown network class");
	}
	return ret;
}

string GetNetmask(const string& ifname)
{
	int ret;
	struct ifreq req{};

	memset( &req, 0, sizeof(struct ifreq) );
	sprintf( req.ifr_name, "%s",ifname.c_str() );

	int sock = socket( AF_INET,SOCK_DGRAM, 0 );

	if( ( ret = ioctl( sock, SIOCGIFNETMASK, &req ) ) < 0 )
	{
		return "";
	}

	return sockaddrtostring( &req.ifr_netmask );
}

string GetAddress(const string& ifname)
{
		int ret;
		struct ifreq req{};

		memset( &req, 0, sizeof( struct ifreq ) );
		sprintf( req.ifr_name, "%s", ifname.c_str() );

		int sock = socket( AF_INET,SOCK_DGRAM, 0 );

		if( ( ret = ioctl( sock, SIOCGIFADDR, &req ) ) < 0 )
		{
				return "";
		}

		return sockaddrtostring(&req.ifr_addr);
}

RaspbianNetworkConfig::RaspbianNetworkConfig(const string &path): NetworkConfig(), path(std::move(path))
{
	this->Parse();
}

void RaspbianNetworkConfig::SetDHCP(const string &iface)
{
	if( ! this->cfg.isMember(iface) || !this->cfg[iface].isObject() )
	{
		logg << Logger::Error << "Interface "<< iface<< " not found" << lend;
		throw std::runtime_error(string("Unknown interface ")+iface);
	}

	if( this->cfg[iface]["addressing"].asString() == "static" )
	{
		this->cfg[iface]["addressing"]="dhcp";

		if( this->cfg[iface].isMember("options") )
		{
			this->cfg[iface].removeMember("options");
		}
	}
}

void RaspbianNetworkConfig::SetStatic(const string &iface, const string &ip,
									  const string &nm, const string &gw,
									  const list<string>& dnss)
{

	if( ! this->cfg.isMember(iface) || !this->cfg[iface].isObject() )
	{
		logg << Logger::Error << "Interface "<< iface<< " not found" << lend;
		throw std::runtime_error(string("Unknown interface ")+iface);
	}
	string adr = this->cfg[iface]["addressing"].asString();
	if( adr == "dhcp" || adr =="static" )
	{
		this->cfg[iface]["addressing"]="static";

		if( this->cfg[iface].isMember("options") )
		{
			this->cfg[iface].removeMember("options");
		}

		this->cfg[iface]["options"]["address"].append(ip);
		this->cfg[iface]["options"]["netmask"].append(nm);
		this->cfg[iface]["options"]["gateway"].append(gw);

		for(const auto& dns: dnss)
		{
			this->cfg[iface]["options"]["dns"].append(dns);
		}

	}

}

void RaspbianNetworkConfig::WriteStaticEntry(stringstream &ss, const string &member)
{
	Json::Value item = this->cfg[member];

	if( !item.isMember("options") || !item["options"].isObject() )
	{
		logg << Logger::Error << "Malformed network entry in dhcpcd config" << lend;
		throw std::runtime_error("Malformed network entry in dhcpcd config");
	}

	ss << "\n# Static configuration for " << member <<"\n";
	ss << "interface "<<member<<"\n";

	Json::Value::Members mems = item["options"].getMemberNames();
	for( const auto& mem: mems)
	{

		if( mem == "netmask" )
		{
			// Processed with address
			continue;
		}
		else if( mem == "address" )
		{
			if( ! item["options"].isMember("netmask") )
			{
				logg << Logger::Error << "Missing netmask in network configuration" << lend;
				throw std::runtime_error("Missing netmask in network configuration");
			}
			IPv4Network addr(item["options"]["address"][0].asString());
			IPv4Network nm(item["options"]["netmask"][0].asString());
			ss << "static ip_address=" << addr.asString() << "/" << std::to_string(nm.asNetwork())<<"\n";
		}
		else if( mem == "gateway")
		{
			bool first = true;
			ss << "static routers=";
			for( const auto& router: item["options"]["gateway"])
			{
				ss << (first?"":" ") << router.asString();
				first=false;
			}
			ss << "\n";
		}
		else if( mem == "dns")
		{
			bool first = true;
			ss << "static domain_name_servers=";
			for( const auto& dns: item["options"]["dns"])
			{
				ss << (first?"":" ") << dns.asString();
				first = false;
			}
			ss << "\n";
		}
		else
		{
			// Unknown parameter just copy back
			ss << "static " << mem << "="<< item["options"][mem][0].asString()  <<"\n";
		}
	}

}


void RaspbianNetworkConfig::WriteConfig()
{
	stringstream ss;

	ss << "# This is an autogenerated file.\n"
	   << "# Any structure and all comments will be lost upon update\n\n";

	// Start with all "unknown" options
	if( this->cfg.isMember("other") && this->cfg["other"].isArray() )
	{
		for( const auto& line: cfg["other"])
		{
			ss << line.asString() << "\n";
		}
	}

	Json::Value::Members mems = this->cfg.getMemberNames();
	for( const auto& mem: mems)
	{
		if( mem == "other" )
		{
			// Skip already processed section
			continue;
		}

		if( this->cfg[mem]["addressing"].asString() != "static" )
		{
			// Skip all none static mappings
			continue;
		}

		this->WriteStaticEntry(ss, mem);

	}

	File::Write(this->path, ss.str(), 0644);

}

bool RaspbianNetworkConfig::RestartInterface(const string &interface)
{
	const string upcmd("/sbin/ip link set " + interface + " up");
	const string downcmd("/sbin/ip link set " + interface + " down");

	bool tmp, ret;
	tie(ret, ignore) = Process::Exec( downcmd );
	tie(tmp, ignore) = Process::Exec( upcmd );
	ret = ret && tmp;

	tmp = NetworkConfig::RestartInterface(interface);

	return ret && tmp;
}

void RaspbianNetworkConfig::Dump()
{
	cout << this->cfg.toStyledString()<<endl;
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
			this->cfg[iface]["options"]["address"].append(addr.front());

			IPv4Network net(std::stoi(addr.back()));
			this->cfg[iface]["options"]["netmask"].append( net.asString() );
		}
		else
		{
			throw std::runtime_error("Failed to parse ipv4 address");
		}
	}
	else if( key == "routers" )
	{
		this->cfg[iface]["options"]["gateway"].append(value);
	}
	else if( key == "domain_name_servers")
	{
		this->cfg[iface]["options"]["dns"]= Json::arrayValue;
		list<string> dnss = String::Split(value, " ");
		for( const auto& dns: dnss)
		{
			this->cfg[iface]["options"]["dns"].append(dns);
		}
	}
	else
	{
		this->cfg[iface]["options"][key].append( value );
	}

}

void RaspbianNetworkConfig::Parse()
{
	if( ! File::FileExists(this->path) )
	{
		logg << Logger::Error << "Config file "<< this->path << " not found" << lend;
		throw std::runtime_error(string("Config file not found: ")+this->path);
	}
	this->cfg["other"]=Json::Value(Json::arrayValue);
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
				this->cfg["other"].append(line);
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
	list<string> ifpaths = File::Glob("/sys/class/net/*");

	list<string> ifs;
	ifs.resize(ifpaths.size());

	transform(ifpaths.begin(), ifpaths.end(), ifs.begin(), File::GetFileName );

	return ifs;
}

NetworkConfig::NetworkConfig()
{
	// Get all network devices on system and add to config

	list<string> ifs = NetUtils::GetInterfaces();

	for(const auto& iface: ifs)
	{
		this->cfg[iface] = Json::Value();
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

Json::Value NetworkConfig::GetInterface(const string &iface)
{
	if( ! this->cfg.isMember( iface ) )
	{
		logg << Logger::Error << "Interface " << iface << " not found" << lend;
		throw std::runtime_error("Interface not found");
	}
	return this->cfg[iface];

}

Json::Value NetworkConfig::GetInterfaces()
{
	return this->cfg;
}

bool NetworkConfig::RestartInterface(const string &interface)
{
	(void) interface;
	bool tmp;

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

} // End namespace
} // End namespace
