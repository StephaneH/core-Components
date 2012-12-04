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
#include "HTTPServer.h"
#include "VHTTPServerComponentLibrary.h"
#include "VHTTPServer.h"



//--------------------------------------------------------------------------------------------------


//jmo - La gestion des SIGPIPE, c'est pour ServerNet (Mac & Linux) ! Sous mac ca ne devrait plus etre utile, et sous Linux ca pause franchement des pb (on dirait que ca entre en conflit avec ce que ServerNet est oblige de faire pour se proteger des SIGPIPE d'OpenSSL). Je le supprime sous Linux et, pour l'instant, ne le laisse qu'en release sous mac.
#if VERSION_RELEASE && VERSIONMAC
static
void Signal_BrokenPipe (int error/*whatever*/)
{
#if VERSIONDEBUG
	::DebugMsg ("\n***HTTP Server received a SIGPIPE with error: %d!***\n", error);
#endif
	return;
}
#endif


VHTTPServerComponentLibrary::VHTTPServerComponentLibrary (const CImpDescriptor* inTypeList, VIndex inTypeCount)
: VComponentLibrary (inTypeList, inTypeCount)
{
}


VHTTPServerComponentLibrary::~VHTTPServerComponentLibrary()
{
}


void VHTTPServerComponentLibrary::DoRegister()
{
	VHTTPServer::Init();

	//jmo - voir mon commentaire juste au dessus !
	//      (cette API est maintenant deconseillee, il faut utiliser sigaction)
#if VERSION_RELEASE && VERSIONMAC  // YT 07-Dec-2009 - ACI0063626
	signal (SIGPIPE, Signal_BrokenPipe);
#endif

#define WITH_TESTS 0
#if VERSIONDEBUG && WITH_TESTS

#if VERSIONWIN /* Temp for tests WinSock will be inited later by NCs */
	WSADATA	wsaData = {0};
	int		wsResult = WSAStartup (MAKEWORD (2, 2), &wsaData);
#endif

#if 0
	XBOX::VError			error = XBOX::VE_OK;
	XBOX::VRegexMatcher *	matcher = XBOX::VRegexMatcher::Create (CVSTR (":8080$"), &error);
	bool					match = false;

	if (matcher && (XBOX::VE_OK == error))
	{
		match = matcher->Find (CVSTR ("127.0.0.1:8080"), 1, true, &error);		// should return true
		match = matcher->Find (CVSTR ("127.0.0.1:8080"), 1, false, &error);		// should return true
		match = matcher->Find (CVSTR ("127.0.0.1"), 1, true, &error);			// should return false
		match = matcher->Find (CVSTR ("127.0.0.1:8081"), 1, true, &error);		// should return false
		match = matcher->Find (CVSTR ("*"), 1, true, &error);					// should return false
		match = matcher->Find (CVSTR (""), 1, true, &error);					// should return false
		match = matcher->Find (CVSTR ("localhost:8080"), 1, true, &error);		// should return true
		match = matcher->Find (CVSTR ("192.168.1.121:8080"), 1, true, &error);	// should return true
	}

	XBOX::ReleaseRefCountable (&matcher);

	matcher = XBOX::VRegexMatcher::Create (CVSTR ("127.0.0.1:8080$"), &error);
	match = false;

	if (matcher && (XBOX::VE_OK == error))
	{
		match = matcher->Find (CVSTR ("127.0.0.1:8080"), 1, true, &error);		// should return true
		match = matcher->Find (CVSTR ("127.0.0.1:8080"), 1, false, &error);		// should return true
		match = matcher->Find (CVSTR ("127.0.0.1"), 1, true, &error);			// should return false
		match = matcher->Find (CVSTR ("127.0.0.1:8081"), 1, true, &error);		// should return false
		match = matcher->Find (CVSTR ("*"), 1, true, &error);					// should return false
		match = matcher->Find (CVSTR (""), 1, true, &error);					// should return false
		match = matcher->Find (CVSTR ("localhost:8080"), 1, true, &error);		// should return true
		match = matcher->Find (CVSTR ("192.168.1.121:8080"), 1, true, &error);	// should return true
	}

	XBOX::ReleaseRefCountable (&matcher);
#endif

#if 0
	sLONG ipv4 = 0;
	sLONG port = 0;
	XBOX::VString ipv4String;

	ParseHostString (CVSTR ("127.0.0.1:8080"), ipv4, port);
	ParseHostString (CVSTR ("127.0.0.1:8080"), ipv4String, port);

	ParseHostString (CVSTR ("127.0.0.1"), ipv4, port);
	ParseHostString (CVSTR ("127.0.0.1"), ipv4String, port);

	ParseHostString (CVSTR ("localhost"), ipv4, port);
	ParseHostString (CVSTR ("localhost"), ipv4String, port);

	ParseHostString (CVSTR ("www.4d.com"), ipv4, port);
	ParseHostString (CVSTR ("www.4d.com"), ipv4String, port);

	GetIPv4FromString (CVSTR ("127.0.0.1"), ipv4);
	GetIPv4FromString (CVSTR ("localhost"), ipv4);
	GetIPv4FromString (CVSTR ("www.4d.com"), ipv4);

#endif

#if 0
	HTTPCookie		cookie;
	XBOX::VString	string;

	XBOX::VString cookieStr1 (CVSTR ("Customer =\"WILE_E_COYOTE\"; version = \"1\"; Path = \"/ acme\" ; httponly ; SeCurE ;;"));
	cookie.FromString (cookieStr1);
	string = cookie.ToString ();

	XBOX::VString cookieStr2 (CVSTR ("Customer =\"WILE_E_COYOTE\"; Path = \"/ acme\" ; domain  =  \"acme.com\"   ;    max-age = 3600 ; comment= \" cookie comment\""));
	cookie.FromString (cookieStr2);
	string = cookie.ToString ();

	XBOX::VString cookieStr3 (CVSTR ("Customer =\"WILE_E_COYOTE\"; Expires=\""));
	string.Clear();
	HTTPProtocol::MakeRFC822GMTDateString (GMT_TOMOROW, string);
	cookieStr3.AppendString (string);
	cookieStr3.AppendCString ("\";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;");
	cookie.FromString (cookieStr3);
	string = cookie.ToString ();

	const XBOX::VString cookieStr4 (CVSTR (" ;;;Customer =\"WILE_E_COYOTE\";; vERsIoN=\"1\";; ; ;mAx-Age=\"   7200 ;;;;"));
	cookie.FromString (cookieStr4);
	string = cookie.ToString ();

	HTTPCookie	cookie2;
	cookie2 = cookie;

	HTTPCookie	cookie3 (cookie);

	bool isEqual = (cookie == cookie2);
	cookie.Clear();
	isEqual = (cookie == cookie2);
	cookie2.Clear();
	isEqual = (cookie == cookie2);
	isEqual = (cookie == cookie);

#endif

#if 0
	VHTTPHeader			header;
	const XBOX::VString	headerString (CVSTR ("Host: localhost\r\nCookie: name=\"ssid\"; value=\"0000111\"\r\n\r\nContent-Length: 100\r\n\r\n"));
	XBOX::VString		outHeaderString;

	header.FromString (headerString);
	header.ToString (outHeaderString);

#endif

#if 0
	XBOX::VString string1 ("\"\"\"\"\"\"\"    multiples leading & ending spaces and quotes      \"\"\"\"\"\"\"\"");
	XBOX::VString string2 ("\"    leading spaces with missing ending quote   ");
	XBOX::VString string3 ("no_spaces_no_quotes");

	HTTPServerTools::TrimUniChar (string1, CHAR_QUOTATION_MARK);
	HTTPServerTools::TrimUniChar (string1, CHAR_SPACE);

	HTTPServerTools::TrimUniChar (string2, CHAR_QUOTATION_MARK);
	HTTPServerTools::TrimUniChar (string2, CHAR_SPACE);

	HTTPServerTools::TrimUniChar (string3, CHAR_QUOTATION_MARK);
	HTTPServerTools::TrimUniChar (string3, CHAR_SPACE);

#endif

#if 0
	XBOX::VString string1 ("/app/.*$");
	XBOX::VString string2 (".*/app/.*");
	XBOX::VString string3 ("/app/.*");
	bool isOK = false;

	isOK = HTTPServerTools::BeginsWithASCIICString (string1, ".*");		// should return false
	isOK = HTTPServerTools::BeginsWithASCIICString (string1, "/");		// should return true
	isOK = HTTPServerTools::BeginsWithASCIICString (string2, "/");		// should return false
	isOK = HTTPServerTools::BeginsWithASCIICString (string2, ".*");		// should return true
	isOK = HTTPServerTools::BeginsWithASCIICString (string3, ".*");		// should return false
	isOK = HTTPServerTools::BeginsWithASCIICString (string3, "/");		// should return true

	isOK = HTTPServerTools::EndsWithASCIICString (string1, ".*");		// should return false
	isOK = HTTPServerTools::EndsWithASCIICString (string1, "$");		// should return true
	isOK = HTTPServerTools::EndsWithASCIICString (string2, ".*/");		// should return false
	isOK = HTTPServerTools::EndsWithASCIICString (string2, ".*");		// should return true
	isOK = HTTPServerTools::EndsWithASCIICString (string3, "/");		// should return false
	isOK = HTTPServerTools::EndsWithASCIICString (string3, "*");		// should return true

#endif

#if 0
	VHTTPHeader			header;
	const XBOX::VString	headerString (CVSTR ("Host: localhost\r\nCookie: name=\"ssid\"; value=\"0000111\"\r\nContent-Type: multipart/form-data; boundary=\"--BoundaryIdentifier_174211871619718\"\r\nContent-Length: 100\r\n\r\n"));
	XBOX::VString		outHeaderString;
	XBOX::VString		boundary;

	header.FromString (headerString);
	header.GetBoundary (boundary);
	header.ToString (outHeaderString);

#endif

#if 0
	const XBOX::VString	SAMPLE_REQUEST_FILE (CVSTR ("E:\\VCS\\depot\\Components\\Main\\HTTPServer\\SampleRequest.txt"));
	VHTTPRequest		request;
	VMIMEMessage		htmlForm;
	XBOX::VFile			file (SAMPLE_REQUEST_FILE);
	XBOX::VFileStream	fileStream (&file);
	XBOX::VError		error = request.ReadFromStream (fileStream);

	htmlForm.Load (request);
#endif

#endif	// VERSIONDEBUG
}


void VHTTPServerComponentLibrary::DoUnregister()
{
	VHTTPServer::Deinit();
}
