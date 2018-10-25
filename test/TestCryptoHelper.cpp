#include "TestCryptoHelper.h"

#include <unistd.h>
#include "CryptoHelper.h"
#include <libutils/FileUtils.h>
#include <libutils/Process.h>

CPPUNIT_TEST_SUITE_REGISTRATION ( TestCryptoHelper );

using namespace OPI;
using namespace Utils;
void TestCryptoHelper::setUp()
{
}

void TestCryptoHelper::tearDown()
{
}

void TestCryptoHelper::TestSelfSigned()
{
	CryptoHelper::RSAWrapper rsa;
	rsa.GenerateKeys();

	File::Write("testpriv.pem", rsa.PrivKeyAsPEM(),0600);
	File::Write("testpub.pem", rsa.PubKeyAsPEM(),0600);

	CryptoHelper::MakeSelfSignedCert("testpriv.pem", "testcert.pem", "localhost", "OpenProducts");

	// Make sure we have a valid cert
	bool ret;
	tie(ret, ignore) = Process::Exec("openssl x509 -in testcert.pem -noout");

	CPPUNIT_ASSERT(ret);

	// Verify that key and cert matches
	string cert_mod, key_mod;
	tie(ret, cert_mod) = Process::Exec("openssl x509 -noout -modulus -in testcert.pem");
	CPPUNIT_ASSERT(ret);

	tie(ret, key_mod) = Process::Exec("openssl rsa -noout -modulus -in testpriv.pem");
	CPPUNIT_ASSERT(ret);

	CPPUNIT_ASSERT( key_mod == cert_mod );

	unlink("testpriv.pem");
	unlink("testpub.pem");
	unlink("testcert.pem");
}
