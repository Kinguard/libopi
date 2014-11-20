#ifndef __MY_DNS_DEFINES_
#define __MY_DNS_DEFINES_

#include <unistd.h>
#include <arpa/inet.h>

#include <list>
#include <string>
#include <iostream>
#include <memory>
#include <map>

using namespace std;

namespace OPI
{
// Add second namespace since we have a lot of datastructures here
namespace Dns
{

struct dns_header
{
	unsigned short id; // identification number

	unsigned char rd :1; // recursion desired
	unsigned char tc :1; // truncated message
	unsigned char aa :1; // authoritive answer
	unsigned char opcode :4; // purpose of message
	unsigned char qr :1; // query/response flag

	unsigned char rcode :4; // response code
	unsigned char cd :1; // checking disabled
	unsigned char ad :1; // authenticated data
	unsigned char z :1; // its z! reserved
	unsigned char ra :1; // recursion available

	unsigned short q_count; // number of question entries
	unsigned short ans_count; // number of answer entries
	unsigned short auth_count; // number of authority entries
	unsigned short add_count; // number of resource entries
};

struct question
{
	uint16_t qtype;
	uint16_t qclass;
};

// Expanded query
struct query
{
	string name;
	unsigned short qtype;
	unsigned short qclass;
};

struct rr_body
{
	uint16_t type;
	uint16_t klass;
	int32_t ttl;
	uint16_t length;
};

class ResourceData
{
public:
	virtual void operator ()() const
	{
		cout << "Unparsed RR data"<<endl;
	}
};

typedef shared_ptr<ResourceData> RRDataPtr;

class MXData: public ResourceData
{
public:
	MXData(uint16_t prio, string exch): type(ns_t_mx), prio(prio),exchange(exch) {}
	virtual void operator ()() const
	{
		cout << dec
			 << "Type " << this->type << endl
			 << "Prio "<< this->prio<<endl
			 << "Exchange " << this->exchange  <<endl;
	}
	uint16_t type;
	uint16_t prio;
	string exchange;
};

class AData: public ResourceData
{
public:
	AData(string adr): address(adr){}
	virtual void operator ()() const
	{
		cout << "Address " << this->address<<endl;
	}
	string address;
};

class CNAMEData: public ResourceData
{
public:
	CNAMEData(string cname): cname(cname){}
	virtual void operator ()() const
	{
		cout << "Cname " << this->cname<<endl;
	}
	string cname;
};

class SOAData: public ResourceData
{
public:

	SOAData( const string& mn, const string& rn, uint32_t ser, int32_t ref, int32_t ret, int32_t exp, int32_t min):
		mname(mn), rname(rn), serial(ser), refresh(ref), retry(ret), expire(exp), minimum(min)
	{}

	virtual void operator ()() const
	{
		cout << "MName "<< this->mname << endl
			 << "RName "<< this->rname << endl
			 << "Serial " << this->serial << endl
			 << "Refresh " << this->refresh << endl
			 << "Retry " << this->retry <<endl
			 << "Expire " << this->expire << endl
			 << "Minimum " << this->minimum <<endl;
	}
	string mname;
	string rname;
	uint32_t serial;
	int32_t refresh;
	int32_t retry;
	int32_t expire;
	int32_t minimum;
};


struct rr
{
	string name;
	uint16_t type;
	uint16_t klass;
	int32_t ttl;
	uint16_t length;
	RRDataPtr data;
};

class DnsHelper
{
public:
	DnsHelper();

	void Query(const char *name, uint16_t type);

	list<query> getQueries() const;
	list<rr> getAnswers() const;
	list<rr> getAuthorative() const;
	list<rr> getAdditional() const;

	void dump();

	virtual ~DnsHelper();

private:

	void reset();
	void doquery(const char *name, uint16_t type);

	string parsename();
	uint32_t u32_parse();
	uint16_t u16_parse();
	int32_t s32_parse();
	int16_t s16_parse();

	void dumpqueries();
	void parsequery();
	void parsequeries();

	RRDataPtr parserrdata(struct rr& r);
	void parserr(back_insert_iterator<list<struct rr> > it);
	void dumprrs(list<rr> &rrs);
	void parse_answers();
	void parse_auth();
	void parse_additional();

	u_int16_t num_questions;
	u_int16_t num_answers;
	u_int16_t num_auth;
	u_int16_t num_additional;

	list<struct query> queries;
	list<struct rr> answers;
	list<struct rr> authorative;
	list<struct rr> additional;
	struct dns_header *header;
	unsigned char buffer[64*1024];
	size_t bufsize;
	unsigned char* cur_pos;
};
} // End namespace Dns
} // End namespace OPI
#endif
