/*
* This file is part of Wakanda software, licensed by 4D under
*  (i) the GNU General Public License version 3 (GNU GPL v3), or
*  (ii) the Affero General Public License version 3 (AGPL v3) or
*  (iii) a commercial license.
* This file remains the exclusive property of 4D and/or its licensors
* and is protected by national and international legislations.
* In any event, Licensee's compliance with the terms and conditions
* of the applicable license constitutes a prerequisite to any use of this file.
* Except as otherwise expressly stated in the applicable license,
* such license does not include any other license or rights on this file,
* 4D's and/or its licensors' trademarks and/or other proprietary rights.
* Consequently, no title, copyright or other proprietary rights
* other than those specified in the applicable license is granted.
*/
#ifndef __HTTP_PUBLIC_TYPES_INCLUDED__
#define __HTTP_PUBLIC_TYPES_INCLUDED__


#define HTTP_SERVER_COMPONENT_SIGNATURE 'HTTP'


// If you change this enum, think about updating 4D's code (VValueController_ProcessProxy and eventually VObjectController_Process)
enum
{
	kHTTPTaskKind_WorkerPool_Server = HTTP_SERVER_COMPONENT_SIGNATURE,
	kHTTPTaskKind_Listener = 'HTLS'
};

typedef enum
{
	AUTH_NONE = 0,
	AUTH_BASIC,
	AUTH_DIGEST,
	AUTH_KERBEROS,
	AUTH_NTLM,
	AUTH_LAST
}
HTTPAuthenticationMethod;

/*
enum HTTPRequestMethod
{
	HTTP_UNKNOWN = -1,
	HTTP_GET = 0,
	HTTP_HEAD,
	HTTP_POST,
	HTTP_PUT,
	HTTP_DELETE,
	HTTP_TRACE,
	HTTP_OPTIONS
};
*/
typedef XBOX::HTTP_Method HTTPRequestMethod;

enum HTTPVersion
{
	VERSION_UNSUPPORTED,
	VERSION_UNKNOWN = VERSION_UNSUPPORTED,
	VERSION_1_0,
	VERSION_1_1
};


enum HTTPStatusCode
{
	HTTP_UNDEFINED,
	HTTP_CONTINUE = 100,
	HTTP_SWITCHING_PROTOCOLS = 101,
	HTTP_OK = 200,
	HTTP_CREATED = 201,
	HTTP_ACCEPTED = 202,
	HTTP_NON_AUTHORITATIVE_INFORMATION = 203,
	HTTP_NO_CONTENT = 204,
	HTTP_RESET_CONTENT = 205,
	HTTP_PARTIAL_CONTENT = 206,
	HTTP_MULTIPLE_CHOICE = 300,
	HTTP_MOVED_PERMANENTLY = 301,
	HTTP_FOUND = 302,
	HTTP_SEE_OTHER = 303,
	HTTP_NOT_MODIFIED = 304,
	HTTP_USE_PROXY = 305,
	HTTP_TEMPORARY_REDIRECT = 307,
	HTTP_BAD_REQUEST = 400,
	HTTP_UNAUTHORIZED = 401,
	HTTP_PAYMENT_REQUIRED = 402,
	HTTP_FORBIDDEN = 403,
	HTTP_NOT_FOUND = 404,
	HTTP_METHOD_NOT_ALLOWED = 405,
	HTTP_NOT_ACCEPTABLE = 406,
	HTTP_PROXY_AUTHENTICATION_REQUIRED = 407,
	HTTP_REQUEST_TIMEOUT = 408,
	HTTP_CONFLICT = 409,
	HTTP_GONE = 410,
	HTTP_LENGTH_REQUIRED = 411,
	HTTP_PRECONDITION_FAILED = 412,
	HTTP_REQUEST_ENTITY_TOO_LARGE = 413,
	HTTP_REQUEST_URI_TOO_LONG = 414,
	HTTP_UNSUPPORTED_MEDIA_TYPE = 415,
	HTTP_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
	HTTP_EXPECTATION_FAILED = 417,
	HTTP_INTERNAL_SERVER_ERROR = 500,
	HTTP_NOT_IMPLEMENTED = 501,
	HTTP_BAD_GATEWAY = 502,
	HTTP_SERVICE_UNAVAILABLE = 503,
	HTTP_GATEWAY_TIMEOUT = 504,
	HTTP_HTTP_VERSION_NOT_SUPPORTED = 505
};


enum HTTPExpireDate
{
	GMT_YESTERDAY = -1,		// Yesterday's Date / Time (usefull for Expiration dates)
	GMT_NOW,				// Current Date / Time
	GMT_TOMOROW,			// Tomorow's Date / Time (usefull for Expiration dates)
	GMT_FAR_FUTURE,			// Append 10 year to the current date/Time (Best practice for static pages when no Expires is defined)
	GMT_AFTER_TIMEOUT
};


// enum HTTPCommonHeaderCode
// {
// /*
// 	Some common HTTP Request headers
// */
// 	HEADER_ACCEPT,
// 	HEADER_ACCEPT_CHARSET,
// 	HEADER_ACCEPT_ENCODING,
// 	HEADER_ACCEPT_LANGUAGE,
// 	HEADER_AUTHORIZATION,
// 	HEADER_COOKIE,
// 	HEADER_EXPECT,
// 	HEADER_FROM,
// 	HEADER_HOST,
// 	HEADER_IF_MATCH,
// 	HEADER_IF_MODIFIED_SINCE,
// 	HEADER_IF_NONE_MATCH,
// 	HEADER_IF_RANGE,
// 	HEADER_IF_UNMODIFIED_SINCE,
// 	HEADER_KEEP_ALIVE,
// 	HEADER_MAX_FORWARDS,
// 	HEADER_PROXY_AUTHORIZATION,
// 	HEADER_RANGE,
// 	HEADER_REFERER,
// 	HEADER_TE,
// 	HEADER_USER_AGENT,
// /*
// 	Some common HTTP Response headers
// */
// 	HEADER_ACCEPT_RANGES,
// 	HEADER_AGE,
// 	HEADER_ALLOW,
// 	HEADER_CACHE_CONTROL,
// 	HEADER_CONNECTION,
// 	HEADER_DATE,
// 	HEADER_ETAG,
// 	HEADER_CONTENT_ENCODING,
// 	HEADER_CONTENT_LANGUAGE,
// 	HEADER_CONTENT_LENGTH,
// 	HEADER_CONTENT_LOCATION,
// 	HEADER_CONTENT_MD5,
// 	HEADER_CONTENT_RANGE,
// 	HEADER_CONTENT_TYPE,
// 	HEADER_EXPIRES,
// 	HEADER_LAST_MODIFIED,
// 	HEADER_LOCATION,
// 	HEADER_PRAGMA,
// 	HEADER_PROXY_AUTHENTICATE,
// 	HEADER_RETRY_AFTER,
// 	HEADER_SERVER,
// 	HEADER_SET_COOKIE,
// 	HEADER_STATUS,
// 	HEADER_VARY,
// 	HEADER_WWW_AUTHENTICATE,
// 	HEADER_X_STATUS,
// 	HEADER_X_POWERED_BY,
// 	HEADER_X_VERSION,
// };


enum HTTPCommonValueCode
{
/*
	Some common HTTP header values
*/
	HEADER_VALUE_CLOSE,			// to be used with "Connection"
	HEADER_VALUE_COMPRESS,		// to be used with "Content-Encoding"
	HEADER_VALUE_DEFLATE,		// to be used with "Content-Encoding"
	HEADER_VALUE_GZIP,			// to be used with "Content-Encoding"
	HEADER_VALUE_KEEP_ALIVE,	// to be used with "Connection"
	HEADER_VALUE_MAX_AGE,		// to be used with "Cache-Control" --> Cache-Control: max-age=0
	HEADER_VALUE_NONE,			// to be used with "Accept-Range"
	HEADER_VALUE_NO_CACHE,		// to be used with "Pragma"
	HEADER_VALUE_X_COMPRESS,	// to be used with "Content-Encoding"
	HEADER_VALUE_X_GZIP,			// to be used with "Content-Encoding"
};



enum HTTPCommonContentType
{
/*
	Some common HTTP Content-Types
*/
	HTTP_CONTENT_TYPE_HTML,
	HTTP_CONTENT_TYPE_JSON,
	HTTP_CONTENT_TYPE_MESSAGE,
	HTTP_CONTENT_TYPE_TEXT,
	HTTP_CONTENT_TYPE_XML,
	HTTP_CONTENT_TYPE_BINARY,
};


/*
	Some common Compression Methods
*/
enum HTTPCompressionMethod
{
	COMPRESSION_UNKNOWN = -1,
	COMPRESSION_NONE,
	COMPRESSION_IDENTITY = COMPRESSION_NONE,	// Identity (RFC2616 - Chapter 3.5 The default (identity) encoding; the use of no transformation whatsoever.
	COMPRESSION_DEFLATE,
	COMPRESSION_GZIP,
	COMPRESSION_X_GZIP,		// basically the same as gzip
	COMPRESSION_COMPRESS,	// Only used for decompression (compression algorithm is patented)
	COMPRESSION_X_COMPRESS,	// basically the same as compress
	COMPRESSION_LAST_SUPPORTED_METHOD = COMPRESSION_X_GZIP
};


typedef enum
{
	STATS_SORT_NONE = 0,
	STATS_SORT_BY_LOADS,
	STATS_SORT_BY_AGE,
	STATS_SORT_BY_SIZE
}
HTTPServerCacheSortOption;


/*
	Some common HTTP Tokens
*/
const char HTTP_SP = ' ';
const char HTTP_HT = '\t';
const char HTTP_VT = '\v';
const char HTTP_CR = '\r';
const char HTTP_LF = '\n';
const char HTTP_COLON = ':';
const char HTTP_SEMICOLON = ';';
const char HTTP_COMMA = ',';
const char HTTP_MINUS = '-';
const char HTTP_LOWLINE = '_';
const char HTTP_SOLIDUS = '/';
const char HTTP_LWS [] = { HTTP_CR, HTTP_LF, HTTP_SP, 0 };
const char HTTP_LWT [] = { HTTP_CR, HTTP_LF, HTTP_HT, 0 };
const char HTTP_CRLF [] = { HTTP_CR, HTTP_LF, 0 };
const char HTTP_CRLFCRLF [] = { HTTP_CR, HTTP_LF, HTTP_CR, HTTP_LF, 0 };
const char HTTP_COLONSP [] = { HTTP_COLON, HTTP_SP, 0 };
const char HTTP_SEMICOLONSP [] = { HTTP_SEMICOLON, HTTP_SP, 0 };
const char HTTP_COMMASP [] = { HTTP_COMMA, HTTP_SP, 0 };


typedef enum 
{
	LRC_NO_ROTATION = 0,
	LRC_ROTATE_ON_FILE_SIZE,
	LRC_ROTATE_EVERY_HOUR,
	LRC_ROTATE_EVERY_DAY,
	LRC_ROTATE_EVERY_WEEK,
	LRC_ROTATE_EVERY_MONTH
}
ELogRotationMode;


typedef enum
{
	LOG_FORMAT_FIRST = 0,
	LOG_FORMAT_NO_LOG = LOG_FORMAT_FIRST,
	LOG_FORMAT_CLF,
	LOG_FORMAT_DLF,
	LOG_FORMAT_ELF,
	LOG_FORMAT_WLF,
	LOG_FORMAT_LAST = LOG_FORMAT_WLF
}
EHTTPServerLogFormat;


typedef enum
{ 
	LOG_TOKEN_NONE = -1,
	LOG_TOKEN_DATE = 0,
	LOG_TOKEN_TIME,
	LOG_TOKEN_HOST_NAME,
	LOG_TOKEN_URL,
	LOG_TOKEN_PATH_ARGS,
	LOG_TOKEN_SEARCH_ARGS,
	LOG_TOKEN_METHOD,
	LOG_TOKEN_ELF_URI,
	LOG_TOKEN_BYTES_SENT,
	LOG_TOKEN_TRANSFER_TIME,
	LOG_TOKEN_AGENT,
	LOG_TOKEN_USER,
	LOG_TOKEN_REFERER,
	LOG_TOKEN_CONNECTION_ID,
	LOG_TOKEN_STATUS,
	LOG_TOKEN_ELF_C_IP,
	LOG_TOKEN_ELF_C_DNS,
	LOG_TOKEN_ELF_CS_URI_STEM,
	LOG_TOKEN_ELF_CS_URI_QUERY,
	LOG_TOKEN_ELF_CS_HOST,
	LOG_TOKEN_CS_REFERER,
	LOG_TOKEN_CS_USER_AGENT,
	LOG_TOKEN_ELF_CS_COOKIE,
	LOG_TOKEN_ELF_S_IP,
	LOG_TOKEN_WLF_BYTES_RECEIVED,
	LOG_TOKEN_RFC_931,
	LOG_TOKEN_HTTP_REQUEST,
	LOG_TOKEN_END
}
EHTTPServerLogToken;

typedef std::vector<EHTTPServerLogToken> VectorOfLogToken;


#endif	// __HTTP_PUBLIC_TYPES_INCLUDED__
