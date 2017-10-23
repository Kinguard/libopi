#include "TestAuthServer.h"

#include <libutils/FileUtils.h>

#include <unistd.h>
#include <algorithm>
#include "AuthServer.h"
#include "CryptoHelper.h"

CPPUNIT_TEST_SUITE_REGISTRATION ( TestAuthServer );

using namespace OPI;
using namespace OPI::CryptoHelper;

#define TMPPUB "tmppub.pem"
#define TMPPRIV "tmppriv.pem"

#define OP_CAPATH "opca.pem"

#define TESTUNITID "7c7d0bb6-b0d8-4ec1-bf8b-baccd17d65b3"

static const char *op_ca =
"-----BEGIN CERTIFICATE-----\n"
"MIIEVDCCAzygAwIBAgIBBDANBgkqhkiG9w0BAQUFADB1MQswCQYDVQQGEwJTRTEO\n"
"MAwGA1UECBMFU2thbmUxFTATBgNVBAcTDExvZGRla29waW5nZTEVMBMGA1UEChMM\n"
"T3BlblByb2R1Y3RzMQowCAYDVQQKEwEuMQowCAYDVQQLEwEuMRAwDgYDVQQDEwdS\n"
"T09UIENBMB4XDTE0MDYwMzEyMzQyMFoXDTI0MDUzMTEyMzQyMFowgYIxCzAJBgNV\n"
"BAYTAlNFMQ4wDAYDVQQIEwVTa2FuZTEVMBMGA1UEBxMMTG9kZGVrb3BpbmdlMRUw\n"
"EwYDVQQKEwxPcGVuUHJvZHVjdHMxCjAIBgNVBAoTAS4xCjAIBgNVBAsTAS4xHTAb\n"
"BgNVBAMTFE9wZW5Qcm9kdWN0cyBTaXRlIENBMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"
"AQ8AMIIBCgKCAQEAsmVTn4wG1rWQHHnz0vIVWBVXIb76xaKbzN5Y9dsAobYFZeRP\n"
"MOzDl5XoYVAUMTuZsVX9J/IS81zQtD7/ylEdgLlfP+GKP8ed8/niYgnWOgUPdJ7S\n"
"CYb2rijHFH/ohJZYrU3xRQrGh7wl7khh3diLQtW3stcBUU0KQEEoRNWeyFoVtN3Q\n"
"yATWO24whdB2u9vgzgx/Sq7ROYYXk8Fm5DKBGRZ1YJer5kWZgzNA5EdWbcsmpbK7\n"
"8rDk6yBBAv36kVS6wffyp6OHWN904zyctAMaeMKWKKWGgDCzlA101O9YQ4LZWdTw\n"
"BjSllA6InvHenXKlF5GG0S4jjyL6+MTEvJv8fwIDAQABo4HgMIHdMB0GA1UdDgQW\n"
"BBRwC337o+/w00ApKt5bfOJjxp+hYTCBpwYDVR0jBIGfMIGcgBQYzDc476ytLJuA\n"
"+mjPsbUqXF64VKF5pHcwdTELMAkGA1UEBhMCU0UxDjAMBgNVBAgTBVNrYW5lMRUw\n"
"EwYDVQQHEwxMb2RkZWtvcGluZ2UxFTATBgNVBAoTDE9wZW5Qcm9kdWN0czEKMAgG\n"
"A1UEChMBLjEKMAgGA1UECxMBLjEQMA4GA1UEAxMHUk9PVCBDQYIJAMlBMSgX5GFJ\n"
"MBIGA1UdEwEB/wQIMAYBAf8CAQEwDQYJKoZIhvcNAQEFBQADggEBAGoOkyyu892H\n"
"t5Z5WwGNWkTMTClH+73MDJ9e0ZRr4qfzZo61f9j92jMpfIsYrtRsUxDPt384jTgJ\n"
"2rt1cgBM0TMcShye6udNukYkv4FIOYvx9f5XFqgJi9gOyg+YAWoLfvTsOWM5wBXB\n"
"YPApKaWYinYiRu/JHupuAuqWtzh5jYogBb3Rb6eACleGNlo1XgcuV74hFA9UBHk4\n"
"R4dmEzFFJJ1SKILVpVZyGBMKHPaHGF9zc5/rKu28jyKujzXm7uk0aeCPOiFXUSkt\n"
"0STFrtDqAAQF5hYPWwnOw3VV1Q09C1/hHzhbrQBbjUXomFvPMXzfwDlatS2ZBE4t\n"
"4jQo7Kotgrg=\n"
"-----END CERTIFICATE-----\n"
"-----BEGIN CERTIFICATE-----\n"
"MIIESDCCAzCgAwIBAgIJAMlBMSgX5GFJMA0GCSqGSIb3DQEBBQUAMHUxCzAJBgNV\n"
"BAYTAlNFMQ4wDAYDVQQIEwVTa2FuZTEVMBMGA1UEBxMMTG9kZGVrb3BpbmdlMRUw\n"
"EwYDVQQKEwxPcGVuUHJvZHVjdHMxCjAIBgNVBAoTAS4xCjAIBgNVBAsTAS4xEDAO\n"
"BgNVBAMTB1JPT1QgQ0EwHhcNMTQwNTEyMTAwMDU2WhcNMjQwNTA5MTAwMDU2WjB1\n"
"MQswCQYDVQQGEwJTRTEOMAwGA1UECBMFU2thbmUxFTATBgNVBAcTDExvZGRla29w\n"
"aW5nZTEVMBMGA1UEChMMT3BlblByb2R1Y3RzMQowCAYDVQQKEwEuMQowCAYDVQQL\n"
"EwEuMRAwDgYDVQQDEwdST09UIENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB\n"
"CgKCAQEAuAaK7v2HfF2TiVJ8XEcGO9o/hSn67Au/25VDYNwOfa5BUa9k1URKp1Hj\n"
"vIA8M+fDKuTQKWs2+drU0Ny/4JLicVCOXEdTEz5ZZJi/oIfG9TE8E8xxOU0ySoR3\n"
"FWSPfwniP+Icq3ILCNI3xyUeys6EteBzkXU0bQG6xpE+VQnlAXQkQNWwWUv4rTR5\n"
"gjYcAXCLCsL1jufJgFPlpcdGjaLd87mCVpUUZnaHCFTLoP3EagDH2afinKpFawTw\n"
"qmF2g5ZCU5w0UqxsEKbzyCQKfvHaPNeoW7ASrLt9z4emkRNQ/XIH1hmW54nhbH3y\n"
"Lwy8T2fWdKMF+97qLWk9hI60mF0g2wIDAQABo4HaMIHXMB0GA1UdDgQWBBQYzDc4\n"
"76ytLJuA+mjPsbUqXF64VDCBpwYDVR0jBIGfMIGcgBQYzDc476ytLJuA+mjPsbUq\n"
"XF64VKF5pHcwdTELMAkGA1UEBhMCU0UxDjAMBgNVBAgTBVNrYW5lMRUwEwYDVQQH\n"
"EwxMb2RkZWtvcGluZ2UxFTATBgNVBAoTDE9wZW5Qcm9kdWN0czEKMAgGA1UEChMB\n"
"LjEKMAgGA1UECxMBLjEQMA4GA1UEAxMHUk9PVCBDQYIJAMlBMSgX5GFJMAwGA1Ud\n"
"EwQFMAMBAf8wDQYJKoZIhvcNAQEFBQADggEBAIUBbV8wMrTx4/Jw9fYqtrj4MgRo\n"
"yfybw8S5ZK4Ut9XpFzVjl9j69qSmssoMe9YodrZP9dOwB6YdowXxJVMGeKnmy4nj\n"
"bYsFus6WBF7Gpn9Xco+lWyFpwHhgOH1rAyZMi8lMFK7BWm7xrOirmFcgwDqjFxJ7\n"
"dJMNb0Bjk4cIxMG6jPd7Ub3vEG1P4TxhbFmyzL5lMRFEpfsnC/2fbFG60AxIsEsM\n"
"cM+Aj4RsB6UDFUF+VcNEV6BEfhDSkWQglSarbWIzGTUtWp57dnYSa+//n24rNrmf\n"
"eHRmKkAvGcZxdSNdJC5lyQUxPTp65oqpFrVdeovb1yhumNpm+I36+a5bUq8=\n"
"-----END CERTIFICATE-----\n";

void TestAuthServer::setUp()
{
	RSAWrapper key;

	key.GenerateKeys();

	Utils::File::Write(TMPPUB, key.PubKeyAsPEM(), 0600);
	Utils::File::Write(TMPPRIV, key.PrivKeyAsPEM(), 0600);
	Utils::File::Write(OP_CAPATH, op_ca, strlen(op_ca), 0600 );
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
	RSAWrapperPtr c;

	CPPUNIT_ASSERT_NO_THROW( c = AuthServer::GetKeysFromFile( TMPPUB, TMPPRIV ) );

	AuthServer s(TESTUNITID, {"https://auth.openproducts.com/", TMPPUB, TMPPRIV});
	s.setDefaultCA( OP_CAPATH );
	int res;
	Json::Value ret;
	CPPUNIT_ASSERT_NO_THROW( tie(res,ret) = s.Login(true));
	vector<int> vals = {200, 403};
	CPPUNIT_ASSERT( find( vals.begin(), vals.end(), res) != vals.end() );
	//cout << "Got reply "<< ret << " ("<<res<<")"<<endl;

}
