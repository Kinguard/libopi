#ifndef EXTCERT_H
#define EXTCERT_H


#include <string>
#include <tuple>

using namespace std;

namespace OPI
{

class ExtCert
{
public:
	ExtCert();

	tuple<int, string> GetExternalCertificates(bool force);

	virtual ~ExtCert();
private:
	const string certhandler = "/usr/share/kinguard-certhandler/letsencrypt.sh";
};

}
#endif // EXTCERT_H
