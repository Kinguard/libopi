#include "AuthServer.h"
#include "Secop.h"
#include <stdexcept>
#include <utility>

#include <libutils/FileUtils.h>
#include <libutils/HttpStatusCodes.h>

using namespace std;
using namespace Utils::HTTP;
namespace OPI
{


AuthServer::AuthServer(string unit_id, const AuthCFG &cfg): HttpClient( cfg.authserver ), unit_id(std::move(unit_id)), acfg(cfg)
{

}

tuple<int, string> AuthServer::GetChallenge()
{
	string ret = "";
	map<string,string> arg = {{ "unit_id", this->unit_id }};

	string s_res = this->DoGet("auth.php", arg);
	json res;
	try
	{
		json res = json::parse(s_res);
		if( res.contains("challange") && res["challange"].is_string() )
		{
			ret = res["challange"];
		}

	} catch (json::parse_error& err)
	{
		//Todo, log errpr
		(void) err;
	}

	return tuple<int,string>(this->result_code,ret);
}

tuple<int, json> AuthServer::SendSignedChallenge(const string &challenge)
{
	json data;
	data["unit_id"] = this->unit_id;
	data["signature"] = challenge;

	map<string,string> postargs = {
		{"data", data.dump() }
	};

	string body = this->DoPost("auth.php", postargs);
	json retobj = json::parse(body);

	return tuple<int,json>(this->result_code, retobj );
}

tuple<int, json> AuthServer::Login(bool usetempkeys)
{
	json ret;
	CryptoHelper::RSAWrapperPtr c;

	try
	{
		// Secop operations might throw an exception
		if( usetempkeys )
		{
			c = AuthServer::GetKeysFromFile(this->acfg.pubkeypath, this->acfg.privkeypath);
		}
		else
		{
			c = AuthServer::GetKeysFromSecop();
		}
	}
	catch (std::runtime_error& err)
	{
		ret["desc"]=string("Failed to retrieve keys: ")+err.what();
		return tuple<int, json>(Status::ServiceUnavailable, ret);
	}

	if( ! c )
	{
		ret["desc"]="Failed to get retrieve keys";
		return tuple<int, json>(Status::ServiceUnavailable, ret);
	}

	string challenge;
	int resultcode = 0;
	tie(resultcode,challenge) = this->GetChallenge();

	if( resultcode != Status::Ok )
	{
		ret["desc"] = "Unknown reply from server";
		ret["value"] = resultcode;
		return tuple<int, json>(Status::InternalServerError, ret);
	}

	string signedchallenge = CryptoHelper::Base64Encode( c->SignMessage( challenge ) );

	json rep;
	tie(resultcode, rep) = this->SendSignedChallenge( signedchallenge );

	if( resultcode != Status::Ok )
	{
		ret["desc"] = "Unexpected reply from server";
		ret["value"] = resultcode;
		ret["reply"] = rep;
		return tuple<int, json>(resultcode, ret);
	}

	if( rep.contains("token") && rep["token"].is_string() )
	{
		ret["token"] = rep["token"];
		return tuple<int, json>(Status::Ok, ret);
	}

	ret["desc"] = "Malformed reply from server";
	return tuple<int, json>(Status::InternalServerError, ret);
}

tuple<int, json> AuthServer::SendSecret(const string &secret, const string &pubkey)
{
	json data;
	data["unit_id"] = this->unit_id;
	data["response"] = secret;
	data["PublicKey"] = pubkey;

	map<string,string> postargs = {
		{"data", data.dump() }
	};

	string body = this->DoPost("register_public.php", postargs);

	json retobj;
	try
	{
		retobj = json::parse(body);
	}
	catch (json::parse_error& err)
	{
		(void) err;
		//TODO: log error
		retobj = json::value_t::object;
		retobj["error"]=body;
	}

	return tuple<int,json>(this->result_code, retobj );
}

tuple<int, json> AuthServer::GetCertificate(const string &csr, const string &token)
{
	map<string,string> postargs = {
		{"unit_id", this->unit_id },
		{"csr", csr }
	};

	map<string,string> headers = {
		{"token", token}
	};

	this->CurlSetHeaders(headers);

	json retobj;
	string body = this->DoPost("get_cert.php", postargs);
	try
	{
		retobj = json::parse(body);
	}
	catch (json::parse_error& err)
	{
		(void)err;
		//TODO: log error
	}

	return tuple<int,json>(this->result_code, retobj );
}

tuple<int, json> AuthServer::UpdateMXPointer(bool useopi, const string &token)
{
	map<string,string> postargs = {
		{"unit_id", this->unit_id },
		{"enable_mx", useopi?"1":"0" }
	};

	map<string,string> headers = {
		{"token", token}
	};

	this->CurlSetHeaders(headers);

	json retobj;
	string body = this->DoPost("update_mx.php", postargs);

	try
	{
		retobj = json::parse(body);
	}
	catch (json::parse_error& err)
	{
		(void) err;
		//TODO: log error
	}

	return tuple<int,json>(this->result_code, retobj );
}

tuple<int, json> AuthServer::CheckMXPointer(const string &name)
{
	map<string,string> postargs = {
		{"fqdn", name },
		{"test_mx", "1" },
		{"type", "MX" }
	};

	json retobj;
	string body = this->DoPost("update_mx.php", postargs);

	try
	{
		retobj = json::parse(body);
	}
	catch (json::parse_error& err)
	{
		(void) err;
		//TODO: log error

	}

	return tuple<int,json>(this->result_code, retobj );
}

void AuthServer::Setup()
{
	Secop s;

	s.SockAuth();
	list<map<string,string>> ids;

	try
	{
		ids = s.AppGetIdentifiers("op-backend");
	}
	catch( __attribute__((unused)) runtime_error& err )
	{
		// Do nothing, appid is missing but thats ok.
	}

	if( ids.size() == 0 )
	{
		s.AppAddID("op-backend");

		RSAWrapper ob;
		ob.GenerateKeys();

		// Write to secop
		map<string,string> data;

		data["type"] = "backendkeys";
		data["pubkey"] = Base64Encode(ob.GetPubKeyAsDER());
		data["privkey"] = Base64Encode(ob.GetPrivKeyAsDER());
		s.AppAddIdentifier("op-backend", data);
	}

}

RSAWrapperPtr AuthServer::GetKeysFromSecop()
{
	CryptoHelper::RSAWrapperPtr c;

	Secop secop;
	secop.SockAuth();

	list<map<string,string>> ids =  secop.AppGetIdentifiers("op-backend");

	if( ids.size() == 0 )
	{
		return c;
	}

	for(auto id : ids )
	{
		if( id.find("type") != id.end() )
		{
			if( id["type"] == "backendkeys" )
			{
				c = RSAWrapperPtr(new RSAWrapper);
				// Key found
				c->LoadPrivKeyFromDER( CryptoHelper::Base64Decode( id["privkey"]) );
				c->LoadPubKeyFromDER( CryptoHelper::Base64Decode( id["pubkey"]) );
				break;
			}
		}
	}

	return c;
}

RSAWrapperPtr AuthServer::GetKeysFromFile(const string &pubpath, const string &privpath)
{
	CryptoHelper::RSAWrapperPtr c( new RSAWrapper );

	string pub_pem = Utils::File::GetContentAsString( pubpath, true );
	string priv_pem = Utils::File::GetContentAsString( privpath, true );

	c->LoadPubKeyFromPEM( pub_pem );
	c->LoadPrivKeyFromPEM( priv_pem );

	return c;
}

AuthServer::~AuthServer() = default;

} // End NS
