#ifndef MAILCONFIG_H
#define MAILCONFIG_H

#include <libutils/FileUtils.h>
#include <libutils/String.h>
#include <libutils/Logger.h>
#include "SysConfig.h"
#include <map>
#include <string>
#include <tuple>

#include "Config.h"

// Convenience defines
#define SCFG	(OPI::SysConfig())
#define SAREA (SCFG.GetKeyAsString("filesystem","storagemount"))
#define MCFG(opt)  (SAREA+SCFG.GetKeyAsString("mail", opt))

using namespace std;
using namespace Utils;

namespace OPI
{

/**
 * @brief The MailAliasFile class, manages postfix virtual aliases
 *
 * The virtual aliases files has an (possibly regexp) address alias and a list
 * of recipients for that match.
 *
 * I.e.
 * /^postmaster@/  user1@domain.net,user2@domain.net
 *
 * redirects all mail for postmaster on any domain to user1 and user2
 *
 */
class MailAliasFile
{
public:
	/**
	 * @brief MailAliasFile constructs a new MailAliasFile object
	 * @param file, which file to operate at
	 */
	MailAliasFile(string  file);

	/**
	 * @brief ReadConfig  re-reads config from file
	 */
	void ReadConfig();

	/**
	 * @brief WriteConfig write config to file
	 */
	void WriteConfig();

	/**
	 * @brief GetUsers get all users for an alias
	 * @param alias, alias to find
	 * @return list of users for alias
	 */
	list<string> GetUsers( const string& alias );

	/**
	 * @brief GetAliases get all aliases in file
	 * @return list of aliases
	 */
	list<string> GetAliases( );

	/**
	 * @brief AddUser add user to alias
	 * @param alias, alias to add user to
	 * @param user, user to add
	 */
	void AddUser(const string& alias, const string& user);
	void RemoveUser(const string& alias, const string& user);

	/**
	 * @brief Dump debug dump to stdout
	 */
	void Dump();

	virtual ~MailAliasFile();
protected:
	string filename;
	map<string,list<string> > config;
};

/**
 * @brief The MailMapFile class manages postfix virtual mailbox files
 *
 * The virtual mailbox tells the system where to store email for a certain
 * address and have the form
 *
 * user@example.com  path/where/to/store/email/
 *
 */
class MailMapFile
{
public:
	MailMapFile(string  aliasfile);

	virtual void ReadConfig();
	virtual void WriteConfig();

	/**
	 * @brief ChangeDomain replace one domain with a new one
	 *
	 * Moves all users in one domain to a new, to, one deleting the
	 * old, from, domain.
	 *
	 * @param from currently existing domain on system
	 * @param to none existant, new, domain in system
	 */
	virtual void ChangeDomain(const string& from, const string& to);

	/**
	 * @brief SetAddress update or create an address in this domain
	 * @param domain domain to use
	 * @param address local part of address
	 * @param user user that should receive email to above address
	 */
	void SetAddress(const string& domain, const string& address, const string& user);

	/**
	 * @brief DeleteAddress delete address in domain
	 *
	 * Delete address from domain and possibly deleting the domain as well. If last
	 * address in domain.
	 *
	 * @param domain domain to operate on
	 * @param address address to delete
	 */
	void DeleteAddress(const string& domain, const string& address);

	/**
	 * @brief GetAddresses get addresses for specific domain
	 * @param domain domain to retrieve adresses for
	 * @return list with tuples localpart,username
	 */
	list<tuple<string,string>> GetAddresses(const string& domain);

	/**
	 * @brief GetAddress gets user for specific domain and local part
	 * @param domain domain to use
	 * @param address adress to retrieve
	 * @return tuple localpart of address and destination username
	 */
	tuple<string,string> GetAddress(const string& domain, const string& address);

	bool hasDomain(const string& domain);
	bool hasAddress(const string& domain, const string& address);

	virtual ~MailMapFile();
protected:

	// <Domain, <remote user, local user>>
	map<string, map<string,string> > config;

	string aliasesfile;
};

/**
 * @brief The MailConfig class extends MailMapFile with domain file support
 *
 * Apart from reading a vmailboxfile this class also reads the domain file
 * and uses that to complement the MailMapFile
 *
 */
class MailConfig: public MailMapFile
{
public:
	MailConfig(const string& aliasfile=MCFG("vmailbox"),
			   string  domainfile=MCFG("vdomains"));

	virtual void ReadConfig();
	virtual void WriteConfig();

	/**
	 * @brief AddDomain add empty domain domain to config
	 * @param domain
	 */
	void AddDomain(const string& domain);

	/**
	 * @brief DeleteDomain delete domain and possible users within it
	 * @param domain
	 */
	void DeleteDomain(const string& domain);

	/**
	 * @brief GetDomains retrieve all domains that system manages mail for
	 * @return
	 */
	list<string> GetDomains();


	virtual ~MailConfig();
private:
	string domainfile;
};

} // End namespace
#endif // MAILCONFIG_H
