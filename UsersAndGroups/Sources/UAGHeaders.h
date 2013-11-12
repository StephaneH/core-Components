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
#ifndef __UAGHEADERS__
#define __UAGHEADERS__


#if COMPIL_GCC
#define UAG_API EXPORT_API
#else
#define UAG_API
#endif


#include "KernelIPC/VKernelIPC.h"
#include "DB4D/Headers/DB4D.h"
#include "UsersAndGroups.h"	// needs to be included before CComponentBridge


USING_TOOLBOX_NAMESPACE

using namespace std;
 
#include <map>
#include <set>
#include <vector>
#include <deque>
#include <algorithm>

#include "XML/VXML.h"
#include "JavaScript/VJavaScript.h"
#include "JavaScript/Sources/VJSJSON.h"



#if VERSIONWIN
#pragma warning (disable: 4800) // disable : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning (disable: 4355) // disable : 'this' : used in base member initializer list
#endif

#ifndef nil
#define nil NULL
#endif

const sLONG kMaxPositif = 0x7FFFFFFF;

#include "Utils.h"

#include "Users.h"

#include "Groups.h"

#include "UAG.h"

#include "JsUAG.h"


#endif
