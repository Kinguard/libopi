#include "TestAuthServer.h"

#include <libutils/FileUtils.h>

#include <unistd.h>
#include <algorithm>
#include "AuthServer.h"
#include "CryptoHelper.h"
#include <libutils/HttpStatusCodes.h>

CPPUNIT_TEST_SUITE_REGISTRATION ( TestAuthServer );

using namespace OPI;
using namespace OPI::CryptoHelper;

using namespace Utils;

constexpr const char* TMPPUB = "tmppub.pem";
constexpr const char* TMPPRIV = "tmppriv.pem";
constexpr const char* OP_CAPATH = "opca.pem";
constexpr const char* TESTUNITID = "7c7d0bb6-b0d8-4ec1-bf8b-baccd17d65b3";

constexpr const char* op_ca =

// Root CA
"-----BEGIN CERTIFICATE-----\n"
"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\n"
"rZDWpGoFwSnBVlnrS6u8E6E=\n"
"-----END CERTIFICATE-----\n"

// Site CA
"-----BEGIN CERTIFICATE-----\n"
"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\n"
"f48=\n"
"-----END CERTIFICATE-----\n"
"";


void TestAuthServer::setUp()
{
	RSAWrapper key;

	key.GenerateKeys();

	File::Write(TMPPUB, key.PubKeyAsPEM(), File::UserRW );
	File::Write(TMPPRIV, key.PrivKeyAsPEM(), File::UserRW);
	File::Write(OP_CAPATH, op_ca, strlen(op_ca), File::UserRW );
}

void TestAuthServer::tearDown()
{
	unlink(TMPPUB);
	unlink(TMPPRIV);
	unlink(OP_CAPATH);
}

void TestAuthServer::Test()
{
	RSAWrapperPtr c;

	CPPUNIT_ASSERT_NO_THROW( c = AuthServer::GetKeysFromFile( TMPPUB, TMPPRIV ) );

	string msg("Hello World!");
	vector<byte> sig = c->SignMessage(msg);

	CPPUNIT_ASSERT( c->VerifyMessage(msg, sig) );

}

void TestAuthServer::Login()
{
	using namespace HTTP;
	RSAWrapperPtr c;

	CPPUNIT_ASSERT_NO_THROW( c = AuthServer::GetKeysFromFile( TMPPUB, TMPPRIV ) );

	AuthServer s(TESTUNITID, {"https://auth.openproducts.com/", TMPPUB, TMPPRIV});
	s.setDefaultCA( OP_CAPATH );
	int res = 0;
	Json::Value ret;
	CPPUNIT_ASSERT_NO_THROW( tie(res,ret) = s.Login(true));
	vector<int> vals = {Status::Ok, Status::Forbidden};
	CPPUNIT_ASSERT( find( vals.begin(), vals.end(), res) != vals.end() );
	//cout << "Got reply "<< ret << " ("<<res<<")"<<endl;

}
