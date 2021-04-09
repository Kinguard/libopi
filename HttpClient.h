#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <sstream>
#include <string>
#include <map>

#include <curl/curl.h>


using namespace std;

namespace OPI
{

class HttpClient
{
public:
	HttpClient(const string& host, bool verifyca = true);

	virtual ~HttpClient();

	void setPort(long value);
	void setTimeout( long value );
	void setDefaultCA(const string& path);
	void setCAPath(const string& path);

protected:
	void CurlPre();
	void CurlSetHeaders(const map<string, string> &headers);
	std::string DoGet(const std::string& path, const map<string, string>& data);
	std::string DoPost(const std::string& path, const map<string, string>& data);
	string CurlPerform();

	string MakeFormData(const map<string,string>& data);
	string EscapeString(const string& arg);

	static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

	CURL *curl;
	long result_code;
	string host;
	string unit_id;
	stringstream body;
	map<string,string> headers;
private:
	void setheaders();
	void clearheaders();
	struct curl_slist *slist;
	long port;
	long timeout;
	bool verifyca;
	string capath;
	string defaultca;
};

} // End NS
#endif // HTTPCLIENT_H
