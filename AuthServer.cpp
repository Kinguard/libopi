#include "AuthServer.h"
#include "Secop.h"
#include "CryptoHelper.h"
#include <stdexcept>

using namespace std;

namespace OPI
{

using namespace CryptoHelper;

AuthServer::AuthServer(const string &unit_id, const string &host): HttpClient( host ), unit_id(unit_id)
{
}

tuple<int, string> AuthServer::GetChallenge()
{
	string ret = "";
	map<string,string> arg = {{ "unit_id", this->unit_id }};

	string s_res = this->DoGet("auth.php", arg);

	Json::Value res;
	if( this->reader.parse(s_res, res) )
	{
		if( res.isMember("challange") && res["challange"].isString() )
		{
			ret = res["challange"].asString();
		}
	}
	return tuple<int,string>(this->result_code,ret);
}

tuple<int, Json::Value> AuthServer::SendSignedChallenge(const string &challenge)
{
	Json::Value data;
	data["unit_id"] = this->unit_id;
	data["signature"] = challenge;

	map<string,string> postargs = {
		{"data", this->writer.write(data) }
	};

	Json::Value retobj = Json::objectValue;
	string body = this->DoPost("auth.php", postargs);

	this->reader.parse(body, retobj);

	return tuple<int,Json::Value>(this->result_code, retobj );
}

tuple<int, Json::Value> AuthServer::Login()
{
	CryptoHelper::RSAWrapper c;

	Secop secop;
	secop.SockAuth();

	Json::Value ret;

	list<map<string,string>> ids =  secop.AppGetIdentifiers("op-backend");

	if( ids.size() == 0 )
	{
		ret["desc"]="Failed to get keys from secop";
		return tuple<int, Json::Value>(503, ret);
	}

	bool found = false;
	for(auto id : ids )
	{
		if( id.find("type") != id.end() )
		{
			if( id["type"] == "backendkeys" )
			{
				// Key found
				c.LoadPrivKeyFromDER( CryptoHelper::Base64Decode( id["privkey"]) );
				c.LoadPubKeyFromDER( CryptoHelper::Base64Decode( id["pubkey"]) );
				found = true;
				break;
			}
		}
	}

	if( ! found )
	{
		ret["desc"]="Failed to load keys from secop";
		return tuple<int, Json::Value>(503, ret);
	}

	string challenge;
	int resultcode;
	tie(resultcode,challenge) = this->GetChallenge();

	if( resultcode != 200 )
	{
		ret["desc"] = "Unknown reply from server";
		ret["value"] = resultcode;
		return tuple<int, Json::Value>(500, ret);
	}

	string signedchallenge = CryptoHelper::Base64Encode( c.SignMessage( challenge ) );

	Json::Value rep;
	tie(resultcode, rep) = this->SendSignedChallenge( signedchallenge );

	if( resultcode != 200 )
	{
		ret["desc"] = "Unexpected reply from server";
		ret["value"] = resultcode;
		return tuple<int, Json::Value>(500, ret);
	}

	if( rep.isMember("token") && rep["token"].isString() )
	{
		ret["token"] = rep["token"].asString();
		return tuple<int, Json::Value>(200, ret);
	}

	ret["desc"] = "Malformed reply from server";
	return tuple<int, Json::Value>(500, ret);
}

tuple<int, Json::Value> AuthServer::SendSecret(const string &secret, const string &pubkey)
{
	Json::Value data;
	data["unit_id"] = this->unit_id;
	data["response"] = secret;
	data["PublicKey"] = pubkey;

	map<string,string> postargs = {
		{"data", this->writer.write(data) }
	};

	string body = this->DoPost("register_public.php", postargs);

	Json::Value retobj = Json::objectValue;
	if( ! this->reader.parse(body, retobj) )
	{
		retobj = Json::objectValue;
		retobj["error"]=body;
	}

	return tuple<int,Json::Value>(this->result_code, retobj );
}

tuple<int, Json::Value> AuthServer::GetCertificate(const string &csr, const string &token)
{
	map<string,string> postargs = {
		{"unit_id", this->unit_id },
		{"csr", csr }
	};

	map<string,string> headers = {
		{"token", token}
	};

	this->CurlSetHeaders(headers);

	Json::Value retobj = Json::objectValue;
	string body = this->DoPost("get_cert.php", postargs);

	this->reader.parse(body, retobj);

	return tuple<int,Json::Value>(this->result_code, retobj );
}

tuple<int, Json::Value> AuthServer::UpdateMXPointer(bool useopi, const string &token)
{
	map<string,string> postargs = {
		{"unit_id", this->unit_id },
		{"enable_mx", useopi?"true":"false" }
	};

	map<string,string> headers = {
		{"token", token}
	};

	this->CurlSetHeaders(headers);

	Json::Value retobj = Json::objectValue;
	string body = this->DoPost("update_mx.php", postargs);

	this->reader.parse(body, retobj);

	return tuple<int,Json::Value>(this->result_code, retobj );
}

AuthServer::~AuthServer()
{
}

} // End NS
