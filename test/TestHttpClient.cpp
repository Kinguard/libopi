#include "TestHttpClient.h"

#include <libutils/FileUtils.h>
#include "HttpClient.h"

using namespace OPI;
using namespace Utils;

CPPUNIT_TEST_SUITE_REGISTRATION ( TestHttpClient );


static const string ca =
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



void TestHttpClient::setUp()
{
	File::Write("op_ca.pem", ca,0660);
}

void TestHttpClient::tearDown()
{
	unlink("op_ca.pem");
}


class TestHttp: public HttpClient
{
public:

	TestHttp(const string& host, bool useca = true):
		HttpClient(host, useca)
	{

	}

	tuple<int,string> Get(const string& path, map<string, string> data)
	{
		string body = this->DoGet(path, data);
		return make_tuple(this->result_code, body);
	}

	tuple<int, string> Post(const string& path, map<string, string> data)
	{
		string body = this->DoPost(path, data);
		return make_tuple(this->result_code, body);
	}

	virtual ~TestHttp();
};

TestHttp::~TestHttp()
{

}

void TestHttpClient::TestNoCA()
{
	int rc;
	string data;

	// Don't verify CA, should work
	{
		TestHttp th("https://auth.openproducts.com", false);
		CPPUNIT_ASSERT_NO_THROW( tie(rc,data) = th.Get("/",{}) );
		CPPUNIT_ASSERT_EQUAL( 200, rc);
	}

	// Verify with default CAs, should fail
	/*
	 * Disable this test for now, we install OP-ca systemwide
	 * and then this unittest does not throw
	{
		TestHttp th("https://auth.openproducts.com");
		th.setDefaultCA("");
		CPPUNIT_ASSERT_THROW( tie(rc,data) = th.Get("/",{}), std::runtime_error );
	}
	*/

	// Verify "ok" site using default CA, should work
	{
		TestHttp th("https://www.google.com");
		th.setDefaultCA("");
		CPPUNIT_ASSERT_NO_THROW( tie(rc,data) = th.Get("/",{}) );
		// Google seems to have changed this
		// Should really rewrite this to accept 2xx and 3xx
		//CPPUNIT_ASSERT_EQUAL( 302, rc);
		CPPUNIT_ASSERT( rc == 302 || rc == 200) ;
	}

	// Verify with default OP CAs, should work
	{
		TestHttp th("https://auth.openproducts.com");
		th.setDefaultCA("op_ca.pem");
		CPPUNIT_ASSERT_NO_THROW( tie(rc,data) = th.Get("/",{}) );
		CPPUNIT_ASSERT_EQUAL( 200, rc);
	}

}

