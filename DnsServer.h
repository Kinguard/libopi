#ifndef DNSSERVER_H
#define DNSSERVER_H

#include "HttpClient.h"
#include "Config.h"

#include <nlohmann/json.hpp>

#include <string>
#include <tuple>

using namespace std;
using json = nlohmann::json;

namespace OPI
{

class DnsServer : public HttpClient
{
public:
	DnsServer( const string& host = "https://auth.openproducts.com/");

	tuple<int, json> CheckOPIName( const string& opiname );

	bool RegisterPublicKey(const string& unit_id, const string& key, const string& token );

	bool UpdateDynDNS(const string& unit_id, const string& name);

	virtual ~DnsServer();
private:

	bool Auth(const string& unit_id);
	// Duplicated from authserver, consider refactoring?
	tuple<int, string> GetChallenge(const string &unit_id);
	tuple<int, json> SendSignedChallenge(const string &unit_id, const string &challenge);

	string token;
};

}
#endif // DNSSERVER_H
