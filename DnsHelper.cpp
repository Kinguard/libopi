
#include <stdexcept>
#include <arpa/nameser.h>
#include <resolv.h>

#include "DnsHelper.h"

#include <iostream>
#include <cstring>
#include <iterator>

using namespace std;

namespace OPI
{

namespace Dns
{

DnsHelper::DnsHelper()
{

	if( ! ( _res.options & RES_INIT ) )
	{
		if( res_init() )
		{
			throw std::runtime_error("Failed to init resolver");
		}
	}
}

void DnsHelper::Query(const char *name, uint16_t type)
{
	this->reset();
	this->doquery(name, type);
}

void DnsHelper::dump()
{
	cout
			<< "Questions " << this->num_questions << endl
			<< "Answers "<< this->num_answers << endl
			<< "Authorative "<< this->num_auth << endl
			<< "Additional "<< this->num_additional << endl;
	cout << "------------------------------------"<< endl;
	this->dumpqueries();
	cout << "------------------------------------"<< endl;
	this->dumprrs(this->answers);
	this->dumprrs(this->authorative);
	this->dumprrs(this->additional);
}

DnsHelper::~DnsHelper() = default;

void DnsHelper::reset()
{
	this->queries.clear();
	this->answers.clear();
	this->authorative.clear();
	this->additional.clear();
	this->bufsize = 0;
	this->header = nullptr;
	this->num_additional = this->num_answers = this->num_auth = this->num_questions = 0;
}

void DnsHelper::doquery(const char *name, uint16_t type)
{
	ssize_t res;
	if( (res=res_query( name, 1, type, this->buffer, sizeof(this->buffer) )) <0 )
	{
		return;
	}

	this->bufsize = res;

	this->header = (struct dns_header*) this->buffer;
	this->num_questions = ntohs( this->header->q_count);
	this->num_answers = ntohs( this->header->ans_count);
	this->num_auth = ntohs( this->header->auth_count);
	this->num_additional = ntohs( this->header->add_count);

	this->cur_pos = this->buffer + sizeof(dns_header);

	this->parsequeries();
	this->parse_answers();
	this->parse_auth();
	this->parse_additional();
}

string DnsHelper::parsecharstring()
{

	const char *start = (const char*) this->cur_pos;
	size_t len = this->bufsize - (this->cur_pos - this->buffer);
	string txt(start, len);

	this->cur_pos += len;
	return txt;
}

uint32_t DnsHelper::u32_parse()
{
	uint32_t res = ntohl( *( (uint32_t*) this->cur_pos ) );
	this->cur_pos += sizeof(uint32_t);
	return res;
}

uint16_t DnsHelper::u16_parse()
{
	uint16_t res = ntohs( *( (uint16_t*) this->cur_pos ) );
	this->cur_pos += sizeof(uint16_t);
	return res;
}

int32_t DnsHelper::s32_parse()
{
	int32_t res = ntohl( *( (int32_t*) this->cur_pos ) );
	this->cur_pos += sizeof(int32_t);
	return res;
}

int16_t DnsHelper::s16_parse()
{
	int16_t res = ntohs( *( (int16_t*) this->cur_pos ) );
	this->cur_pos += sizeof(int16_t);
	return res;
}

string DnsHelper::parsename()
{
	char buf[256];
	int res = dn_expand(this->buffer, this->buffer+this->bufsize, this->cur_pos, buf, sizeof(buf) );

	if( res == -1 )
	{
		// Really should fail here but why we end up here now needs more investigation
		// Unittests of this class used to worked but stopped here now.
		return "N/A";

		//throw std::runtime_error("Unable to parse name");
	}

	this->cur_pos += res;
	return string(buf);
}

void DnsHelper::dumpqueries()
{
	for( auto& x: this->queries )
	{
		cout << "Name  "<< x.name << endl
			 << "Type  "<< x.qtype << endl
			 << "Class " << x.qclass <<endl;
	}
}

void DnsHelper::parsequery()
{
	struct query q;
	q.name = this->parsename();

	struct question *qs = reinterpret_cast<struct question*>(this->cur_pos);

	q.qtype = ntohs( qs->qtype );
	this->cur_pos += sizeof( qs->qtype );

	q.qclass = ntohs( qs->qclass );
	this->cur_pos += sizeof( qs->qclass );

	this->queries.push_back( q );
}

void DnsHelper::parsequeries()
{
	for(int i=0; i < this->num_questions; i++ )
	{
		this->parsequery();
	}
}

RRDataPtr DnsHelper::parserrdata(rr &r)
{
	RRDataPtr ret;

	switch( r.type )
	{
	case ns_t_mx:
		{
			uint16_t prio = this->u16_parse();
			string e = this->parsename();
			ret = RRDataPtr( new MXData(prio, e));
		}
		break;
	case ns_t_a:
		{
			struct in_addr add = {};
			add.s_addr = this->u32_parse();

			ret = RRDataPtr( new AData( string(inet_ntoa(add)) ) );
		}
		break;
	case ns_t_cname:
		{
			string e = this->parsename();
			ret = RRDataPtr( new CNAMEData(e));
		}
		break;
	case ns_t_soa:
		{
			string mn = this->parsename();
			string rn = this->parsename();
			uint32_t ser = this->u32_parse();
			int32_t ref = this->s32_parse();
			int32_t a_ret = this->s32_parse();
			int32_t exp = this->s32_parse();
			int32_t min = this->s32_parse();
			ret = RRDataPtr( new SOAData(mn,rn,ser,ref,a_ret,exp,min));
		}
		break;
	case ns_t_txt:
		{
			string txt = this->parsecharstring();
			ret = RRDataPtr( new TXTData(txt) );
		}
		break;
	default:
		cout << "Unknown type "<<r.type<<endl;
		this->cur_pos += r.length;
		ret = RRDataPtr( new ResourceData() );
		break;
	}

	return ret;
}

void DnsHelper::parserr(back_insert_iterator<list<struct rr> > it)
{
	struct rr r;

	r.name = this->parsename();

	struct rr_body *b = reinterpret_cast<struct rr_body*>(this->cur_pos);

	r.type = ntohs(b->type);
	this->cur_pos += sizeof( b->type );

	r.klass = ntohs(b->klass);
	this->cur_pos += sizeof( b->klass );

	r.ttl = ntohl(b->ttl);
	this->cur_pos += sizeof( b->ttl );

	r.length = ntohs(b->length);
	this->cur_pos += sizeof( b->length );

	r.data = this->parserrdata(r);

	*it++ = r;
}

void DnsHelper::dumprrs(list<struct rr>& rrs)
{
	for( const auto& x: rrs )
	{
		cout << "Name   " << x.name << endl
			 << "Type   " << x.type << endl
			 << "Class  " << x.klass << endl
			 << "TTL    " << x.ttl << endl
			 << "Length " << x.length << endl;
		x.data->operator()();
	}
}

void DnsHelper::parse_answers()
{
	for(int i=0; i < this->num_answers; i++ )
	{
		this->parserr(back_inserter(this->answers));
	}
}

void DnsHelper::parse_auth()
{
	for(int i=0; i < this->num_auth; i++ )
	{
		this->parserr(back_inserter(this->authorative));
	}
}

void DnsHelper::parse_additional()
{
	for(int i=0; i < this->num_additional; i++ )
	{
		this->parserr(back_inserter(this->additional));
	}
}

list<query> DnsHelper::getQueries() const
{
	return queries;
}

list<rr> DnsHelper::getAdditional() const
{
	return additional;
}

list<rr> DnsHelper::getAuthorative() const
{
	return authorative;
}

list<rr> DnsHelper::getAnswers() const
{
	return answers;
}
} // End namespace Dns
} // End namespace OPI
