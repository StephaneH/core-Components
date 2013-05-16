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
#ifndef __HTTP_PRECOMPILED_HEADER_INCLUDED__
#define __HTTP_PRECOMPILED_HEADER_INCLUDED__

// Add your Toolbox headers here
#include "Kernel/VKernel.h"
#include "KernelIPC/VKernelIPC.h"
#include "XML/Sources/VPreferences.h"


// ServerNet headers
#include "ServerNet/VServerNet.h"

#include "UsersAndGroups/Sources/UsersAndGroups.h"

USING_TOOLBOX_NAMESPACE

#ifndef VERSIONDEBUG
	#ifdef _DEBUG
		#define VERSIONDEBUG 1
	#else
		#define VERSIONDEBUG 0
	#endif
#endif

// Add needed Interfaces here
#include "../Interfaces/CHTTPServer.h"

#include <vector>
#include <map>
#include <cctype>

#include "Zip/Interfaces/CZipComponent.h"
#include "VAuthenticationReferee.h"
#include "HTTPProtocol.h"
#include "HTTPErrors.h"
#include "VMimeType.h"
#include "HTTPServerCache.h"
#include "HTTPServerTools.h"
#include "HTTPServerLog.h"
#include "VHTTPResource.h"
#include "HTTPServerSettings.h"
#include "VHTTPServerProject.h"
#include "VHTTPServerComponentLibrary.h"
#include "VHTTPServer.h"
#include "VHTTPResponse.h"
#include "VHTTPRequest.h"
#include "VNonce.h"
#include "VAuthenticationManager.h"
#include "VVirtualHost.h"
#include "VVirtualHostManager.h"
#include "VVirtualFolder.h"
#include "VHTTPServerSession.h"
#include "VHTTPError.h"


#endif	// __HTTP_PRECOMPILED_HEADER_INCLUDED__

