#ifndef AUTHSERVER_H
#define AUTHSERVER_H

#include <string>
#include <tuple>

#include <curl/curl.h>
#include <json/json.h>

#include "CryptoHelper.h"
#include "HttpClient.h"

#include "Config.h"

#define TMP_PRIV "/tmp/tmpkey.priv"
#define TMP_PUB "/tmp/tmpkey.pub"

//TODO: Refactor in implementation for updating/initializing keys with secret

using namespace std;

namespace OPI
{

using namespace CryptoHelper;

class AuthServer: public HttpClient
{
public:

	struct AuthCFG
	{
		string authserver;
		string pubkeypath;
		string privkeypath;
	};

	AuthServer(string  unit_id, const struct AuthCFG& cfg = {"https://auth.openproducts.com/", TMP_PUB, TMP_PRIV} );

	tuple<int,string> GetChallenge();

	tuple<int, Json::Value> SendSignedChallenge( const string& challenge);

	tuple<int, Json::Value> Login(bool usetempkeys=false);

	tuple<int, Json::Value> SendSecret(const string& secret, const string& pubkey);

	tuple<int, Json::Value> GetCertificate(const string& csr, const string& token);

	tuple<int, Json::Value> UpdateMXPointer(bool useopi, const string& token);

	tuple<int, Json::Value> CheckMXPointer(const string& name);

	/**
	 * @brief Setup, create keys and register in Secop if needed
	 */
	static void Setup();

	static RSAWrapperPtr GetKeysFromSecop();

	static RSAWrapperPtr GetKeysFromFile(const string& pubpath, const string& privpath);


	virtual ~AuthServer();
private:

	Json::Reader reader;
	Json::FastWriter writer;
	string unit_id;
	struct AuthCFG acfg;
};

}
#endif // AUTHSERVER_H
