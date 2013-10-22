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

#if 0
	VDebugTimer			timer;
	bool				isOK = false;
	sLONG				i = 0;
	XBOX::VString		SAMPLE_LONG_REQUEST_LINE;
	
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("GET /waf-optimize?files=%27+/lib/jquery-ui/themes/base/jquery.ui.core.css,+/lib/jquery-ui/themes/base/jquery.ui.resizable.css,+/lib/jquery-ui/themes/base/jquery.ui.selectable.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/lib/jquery-ui/themes/base/jquery.ui.accordion.css,+/lib/jquery-ui/themes/base/jquery.ui.autocomplete.css,+/lib/jquery-ui/themes/base/jquery.ui.button.css,+/lib/jquery-ui/themes/base/jquery.ui.dialog.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/lib/jquery-ui/themes/base/jquery.ui.slider.css,+/lib/jquery-ui/themes/base/jquery.ui.tabs.css,+/lib/jquery-ui/themes/base/jquery.ui.datepicker.css,+/lib/jquery-ui/themes/base/jquery.ui.progressbar.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/lib/jquery-ui/themes/base/jquery.ui.theme.css,+/lib/selectbox/jquery-selectbox.css,+/widget/css/widget-mobile.css,+/widget/skin/default/css/widget-skin-default.css,+/widget/skin/metal/css/widget-skin-metal.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/skin/light/css/widget-skin-light.css,+/widget/skin/cupertino/css/widget-skin-cupertino.css,+/widget/list/css/widget-list.css,+/widget/list/skin/cupertino/css/widget-list-skin-cupertino.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/popover/css/widget-popover.css,+/widget/popover/skin/cupertino/css/widget-popover-skin-cupertino.css,+/widget/accordion/css/widget-accordion.css,+/widget/accordion/skin/default/css/widget-accordion-skin-default.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/accordion/skin/metal/css/widget-accordion-skin-metal.css,+/widget/accordion/skin/light/css/widget-accordion-skin-light.css,+/widget/login/css/widget-login.css,+/widget/login/skin/default/css/widget-login-skin-default.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/login/skin/metal/css/widget-login-skin-metal.css,+/widget/login/skin/light/css/widget-login-skin-light.css,+/widget/datepicker/skin/default/css/widget-datepicker-skin-default.css,+/widget/datepicker/skin/metal/css/widget-datepicker-skin-metal.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/datepicker/skin/light/css/widget-datepicker-skin-light.css,+/widget/dataGrid/css/widget-dataGrid-mobile.css,+/widget/dataGrid/skin/default/css/widget-dataGrid-skin-default.css,+/widget/dataGrid/skin/metal/css/widget-dataGrid-skin-metal.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/dataGrid/skin/light/css/widget-dataGrid-skin-light.css,+/widget/dataGrid/skin/cupertino/css/widget-dataGrid-skin-cupertino.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/autoForm/css/widget-autoForm.css,+/widget/autoForm/skin/default/css/widget-autoForm-skin-default.css,+/widget/autoForm/skin/metal/css/widget-autoForm-skin-metal.css,+/widget/autoForm/skin/light/css/widget-autoForm-skin-light.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/autoForm/skin/roundy/css/widget-autoForm-skin-roundy.css,+/widget/container/css/widget-container.css,+/widget/container/skin/default/css/widget-container-skin-default.css,+/widget/container/skin/metal/css/widget-container-skin-metal.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/container/skin/light/css/widget-container-skin-light.css,+/widget/container/skin/cupertino/css/widget-container-skin-cupertino.css,+/widget/matrix/css/widget-matrix.css,+/widget/matrix/skin/metal/css/widget-matrix-skin-metal-mobile.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/matrix/skin/cupertino/css/widget-matrix-skin-cupertino.css,+/widget/image/css/widget-image.css,+/widget/icon/css/widget-icon.css,+/widget/textField/css/widget-textField.css,+/widget/textField/skin/default/css/widget-textField-skin-default.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/textField/skin/metal/css/widget-textField-skin-metal.css,+/widget/textField/skin/cupertino/css/widget-textField-skin-cupertino.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/button/css/widget-button.css,+/widget/button/skin/default/css/widget-button-skin-default.css,+/widget/button/skin/metal/css/widget-button-skin-metal.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/button/skin/light/css/widget-button-skin-light.css,+/widget/button/skin/image/css/widget-button-skin-image.css,+/widget/button/skin/cupertino/css/widget-button-skin-cupertino.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/buttonImage/css/widget-buttonImage.css,+/widget/buttonImage/skin/cupertino/css/widget-buttonImage-skin-cupertino.css,+/widget/richText/css/widget-richText.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/slider/css/widget-slider.css,+/widget/slider/skin/default/css/widget-slider-skin-default.css,+/widget/slider/skin/metal/css/widget-slider-skin-metal.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/slider/skin/light/css/widget-slider-skin-light.css,+/widget/slider/skin/cupertino/css/widget-slider-skin-cupertino.css,+/widget/progressBar/css/widget-progressBar.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/progressBar/skin/default/css/widget-progressBar-skin-default.css,+/widget/progressBar/skin/metal/css/widget-progressBar-skin-metal.css,+/widget/progressBar/skin/light/css/widget-progressBar-skin-light.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/progressBar/skin/roundy/css/widget-progressBar-skin-roundy.css,+/widget/progressBar/skin/cupertino/css/widget-progressBar-skin-cupertino.css,+/widget/yahooWeather/css/widget-yahooWeather.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/combobox/css/widget-combobox.css,+/widget/combobox/skin/default/css/widget-combobox-skin-default.css,+/widget/combobox/skin/metal/css/widget-combobox-skin-metal.css,+/widget/combobox/skin/light/css/widget-combobox-skin-light.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/section/css/widget-section.css,+/widget/section/skin/cupertino/css/widget-section-skin-cupertino.css,+/widget/select/css/widget-select.css,+/widget/select/skin/default/css/widget-select-skin-default.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/select/skin/metal/css/widget-select-skin-metal.css,+/widget/select/skin/light/css/widget-select-skin-light.css,+/widget/select/skin/cupertino/css/widget-select-skin-cupertino.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/radiogroup/css/widget-radiogroup.css,+/widget/radiogroup/skin/default/css/widget-radiogroup-skin-default.css,+/widget/radiogroup/skin/metal/css/widget-radiogroup-skin-metal.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/radiogroup/skin/light/css/widget-radiogroup-skin-light.css,+/widget/radiogroup/skin/cupertino/css/widget-radiogroup-skin-cupertino.css,+/widget/label/css/widget-label.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/checkbox/css/widget-checkbox.css,+/widget/checkbox/skin/default/css/widget-checkbox-skin-default.css,+/widget/checkbox/skin/metal/css/widget-checkbox-skin-metal.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/checkbox/skin/light/css/widget-checkbox-skin-light.css,+/widget/switchBox/css/widget-switchbox.css,+/widget/switchBox/skin/cupertino/css/widget-switchBox-skin-cupertino.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/splitView/css/widget-splitView.css,+/widget/splitView/skin/cupertino/css/widget-splitView-skin-cupertino.css,+/widget/menubar/css/widget-menubar.css,+/widget/menubar/skin/default/css/widget-menubar-skin-default.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/menubar/skin/metal/css/widget-menubar-skin-metal.css,+/widget/menubar/skin/light/css/widget-menubar-skin-light.css,+/widget/menubar/skin/cupertino/css/widget-menubar-skin-cupertino.css,+/widget/menuitem/css/widget-menuitem.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/menuitem/skin/default/css/widget-menuitem-skin-default.css,+/widget/menuitem/skin/metal/css/widget-menuitem-skin-metal.css,+/widget/menuitem/skin/light/css/widget-menuitem-skin-light.css,+/widget/menuitem/skin/cupertino/css/widget-menuitem-skin-cupertino.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/login/css/widget-login.css,+/widget/login/skin/default/css/widget-login-skin-default.css,+/widget/login/skin/metal/css/widget-login-skin-metal.css,+/widget/login/skin/light/css/widget-login-skin-light.css,+/widget/component/css/widget-component.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/chart/css/widget-chart.css,+/widget/fileUpload/css/widget-fileUpload.css,+/widget/errorDiv/css/widget-errorDiv.css,+/widget/tabview/css/widget-tabview.css,+/widget/tabview/skin/default/css/widget-tabview-skin-default.css,+/widget/tabview/skin/metal/css/widget-tabview-skin-metal.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/tabview/skin/light/css/widget-tabview-skin-light.css,+/widget/tabview/skin/cupertino/css/widget-tabview-skin-cupertino.css,+/widget/navigationView/css/widget-navigationView.css,+/widget/navigationView/skin/cupertino/css/widget-navigationView-skin-cupertino.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/tabview/css/widget-tabview.css,+/widget/tabview/skin/default/css/widget-tabview-skin-default.css,+/widget/tabview/skin/metal/css/widget-tabview-skin-metal.css,+/widget/tabview/skin/light/css/widget-tabview-skin-light.css,+/widget/dialog/css/widget-dialog.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/dialog/skin/default/css/widget-dialog-skin-default.css,+/widget/dialog/skin/metal/css/widget-dialog-skin-metal.css,+/widget/dialog/skin/light/css/widget-dialog-skin-light.css,+/widget/tinymce/css/widget-tinymce.css,+/widget/calendar/css/widget-calendar.css,");
	SAMPLE_LONG_REQUEST_LINE.AppendCString ("+/widget/calendar/css/datepicker.css,+/widget/frame/css/widget-frame.css,+/widget/video/css/widget-video.css,+/widget/canvas/css/widget-canvas.css,+/lib/beautytips/beautytips.css,+/lib/mobile/mobiscroll/css/mobiscroll-1.5.3.min.css,+/themes/cupertinoIpad.css%27 HTTP/1.1");

	for (i = 0; i < 100000; ++i)
	{
		if ((isOK = HTTPProtocol::IsValidRequestLine (SAMPLE_LONG_REQUEST_LINE)) == false)
			break;
	}

	XBOX::VString	string;
	string.Printf ("***HTTPProtocol::IsValidRequestLine run %d times with ", i);
	string.AppendCString (isOK ? "no error" : "errors");
	timer.DebugMsg (string);

#endif

#endif	// VERSIONDEBUG
}


void VHTTPServerComponentLibrary::DoUnregister()
{
	VHTTPServer::Deinit();
}
