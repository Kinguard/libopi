#ifndef HTTPSTATUSCODES_H
#define HTTPSTATUSCODES_H

#include <map>
#include <string>

namespace OPI
{

namespace HTTP {

namespace Status
{

constexpr unsigned int Continue						= 100;
constexpr unsigned int SwitchingProtocol			= 101;
constexpr unsigned int Processing					= 102;
constexpr unsigned int EarlyHints					= 103;

constexpr unsigned int Ok							= 200;
constexpr unsigned int Created						= 201;
constexpr unsigned int Accepted						= 202;
constexpr unsigned int NonAuthInformation			= 203;
constexpr unsigned int NoContent					= 204;
constexpr unsigned int ResetContent					= 205;
constexpr unsigned int PartialContent				= 206;
constexpr unsigned int MultiStatus					= 207;
constexpr unsigned int AlreadyReported				= 208;
constexpr unsigned int IMUsed						= 226;

constexpr unsigned int MultipleChoice				= 300;
constexpr unsigned int MovedPermanently				= 301;
constexpr unsigned int Found						= 302;
constexpr unsigned int SeeOther						= 303;
constexpr unsigned int NotModified					= 304;
constexpr unsigned int UseProxy						= 305;
constexpr unsigned int TemporaryRedirect			= 307;
constexpr unsigned int PermanentRedirect			= 308;

constexpr unsigned int BadRequest					= 400;
constexpr unsigned int Unauthorized					= 401;
constexpr unsigned int PaymentRequired				= 402;
constexpr unsigned int Forbidden					= 403;
constexpr unsigned int NotFound						= 404;
constexpr unsigned int MethodNotAllowed				= 405;
constexpr unsigned int NotAcceptable				= 406;
constexpr unsigned int ProxyAuthRequired			= 407;
constexpr unsigned int RequestTimeout				= 408;
constexpr unsigned int Conflict						= 409;
constexpr unsigned int Gone							= 410;
constexpr unsigned int LengthRequired				= 411;
constexpr unsigned int PreconditionFailed			= 412;
constexpr unsigned int PayloadTooLarge				= 413;
constexpr unsigned int URITooLong					= 414;
constexpr unsigned int UnsupportedMediaType			= 415;
constexpr unsigned int RangeNotSatisfiable			= 416;
constexpr unsigned int ExpectationFailed			= 417;
constexpr unsigned int ImATeapot					= 418;
constexpr unsigned int MisdirectedRequest			= 421;
constexpr unsigned int UnprocessableEntity			= 422;
constexpr unsigned int Locked						= 423;
constexpr unsigned int FailedDependency				= 424;
constexpr unsigned int TooEarly						= 425;
constexpr unsigned int UpgradeRequired				= 426;
constexpr unsigned int PreconditionRequired			= 428;
constexpr unsigned int TooManyRequests				= 429;
constexpr unsigned int RequestHeaderFieldsTooLarge	= 431;
constexpr unsigned int UnavailableForLegalReasons	= 451;

constexpr unsigned int InternalServerError			= 500;
constexpr unsigned int NotImplemented				= 501;
constexpr unsigned int BadGateway					= 502;
constexpr unsigned int ServiceUnavailable			= 503;
constexpr unsigned int GatewayTimeout				= 504;
constexpr unsigned int HTTPVersionNotSupported		= 505;
constexpr unsigned int VariantAlsoNegotiates		= 506;
constexpr unsigned int InsufficientStorage			= 507;
constexpr unsigned int LoopDetected					= 508;
constexpr unsigned int NotExtended					= 510;
constexpr unsigned int NetworkAuthenticationRequired= 511;

const std::map<unsigned int,const char*> toString =
{
	{HTTP::Status::Continue						,"Continue"},
	{HTTP::Status::SwitchingProtocol			,"Switching Protocol"},
	{HTTP::Status::Processing					,"Processing"},
	{HTTP::Status::EarlyHints					,"Early Hints"},

	{HTTP::Status::Ok							,"Ok"},
	{HTTP::Status::Created						,"Created"},
	{HTTP::Status::Accepted						,"Accepted"},
	{HTTP::Status::NonAuthInformation			,"Non-Authorative Information"},
	{HTTP::Status::NoContent					,"No Content"},
	{HTTP::Status::ResetContent					,"Reset Content"},
	{HTTP::Status::PartialContent				,"Partial Content"},
	{HTTP::Status::MultiStatus					,"Multi-Status"},
	{HTTP::Status::AlreadyReported				,"Already Reported"},
	{HTTP::Status::IMUsed						,"IM Used"},

	{HTTP::Status::MultipleChoice				,"MultipleChoice"},
	{HTTP::Status::MovedPermanently				,"MovedPermanently"},
	{HTTP::Status::Found						,"Found"},
	{HTTP::Status::SeeOther						,"SeeOther"},
	{HTTP::Status::NotModified					,"NotModified"},
	{HTTP::Status::UseProxy						,"UseProxy"},
	{HTTP::Status::TemporaryRedirect			,"TemporaryRedirect"},
	{HTTP::Status::PermanentRedirect			,"PermanentRedirect"},

	{HTTP::Status::BadRequest					,"Bad Request"},
	{HTTP::Status::Unauthorized					,"Unauthorized"},
	{HTTP::Status::PaymentRequired				,"Payment Required"},
	{HTTP::Status::Forbidden					,"Forbidden"},
	{HTTP::Status::NotFound						,"Not Found"},
	{HTTP::Status::MethodNotAllowed				,"Method Not Allowed"},
	{HTTP::Status::NotAcceptable				,"Not Acceptable"},
	{HTTP::Status::ProxyAuthRequired			,"Proxy Authentication Required"},
	{HTTP::Status::RequestTimeout				,"Request Timeout"},
	{HTTP::Status::Conflict						,"Conflict"},
	{HTTP::Status::Gone							,"Gone"},
	{HTTP::Status::LengthRequired				,"Length Required"},
	{HTTP::Status::PreconditionFailed			,"Precondition Failed"},
	{HTTP::Status::PayloadTooLarge				,"Payload Too Large"},
	{HTTP::Status::URITooLong					,"URI Too Long"},
	{HTTP::Status::UnsupportedMediaType			,"Unsupported Media Type"},
	{HTTP::Status::RangeNotSatisfiable			,"Range Not Satisfiable"},
	{HTTP::Status::ExpectationFailed			,"Expectation Failed"},
	{HTTP::Status::ImATeapot					,"Im a Teapot"},
	{HTTP::Status::MisdirectedRequest			,"Misdirected Request"},
	{HTTP::Status::UnprocessableEntity			,"Unprocessable Entity"},
	{HTTP::Status::Locked						,"Locked"},
	{HTTP::Status::FailedDependency				,"Failed Dependency"},
	{HTTP::Status::TooEarly						,"Too Early"},
	{HTTP::Status::UpgradeRequired				,"Upgrade Required"},
	{HTTP::Status::PreconditionRequired			,"Precondition Required"},
	{HTTP::Status::TooManyRequests				,"Too Many Requests"},
	{HTTP::Status::RequestHeaderFieldsTooLarge	,"Request Header Fields Too Large"},
	{HTTP::Status::UnavailableForLegalReasons	,"Unavailable For Legal Reasons"},

	{HTTP::Status::InternalServerError			,"InternalServerError"},
	{HTTP::Status::NotImplemented				,"NotImplemented"},
	{HTTP::Status::BadGateway					,"BadGateway"},
	{HTTP::Status::ServiceUnavailable			,"ServiceUnavailable"},
	{HTTP::Status::GatewayTimeout				,"GatewayTimeout"},
	{HTTP::Status::HTTPVersionNotSupported		,"HTTPVersionNotSupported"},
	{HTTP::Status::VariantAlsoNegotiates		,"VariantAlsoNegotiates"},
	{HTTP::Status::InsufficientStorage			,"InsufficientStorage"},
	{HTTP::Status::LoopDetected					,"LoopDetected"},
	{HTTP::Status::NotExtended					,"NotExtended"},
	{HTTP::Status::NetworkAuthenticationRequired,"NetworkAuthenticationRequired"},
};

} // Status
} // HTTP
} // OPI

#endif // HTTPSTATUSCODES_H
