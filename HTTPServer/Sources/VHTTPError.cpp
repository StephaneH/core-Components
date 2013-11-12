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


//--------------------------------------------------------------------------------------------------


VHTTPError::VHTTPError (XBOX::VError inErrorCode)
: VErrorBase (inErrorCode, 0)
{
}


VHTTPError::~VHTTPError()
{
}


void VHTTPError::GetLocation (XBOX::VString& outLocation) const
{
	outLocation.FromCString ("HTTP Server");
}


void VHTTPError::GetErrorString (XBOX::VString& outError) const
{
	GetErrorDescription (outError);
}


void VHTTPError::GetErrorDescription (XBOX::VString& outErrorString) const
{
	VHTTPServer::LocalizeErrorMessage (GetError(), outErrorString);
}
