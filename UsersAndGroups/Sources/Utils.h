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
#ifndef __UTILS__
#define __UTILS__


namespace uag
{
	EXTERN_BAGKEY(group);
	EXTERN_BAGKEY(user);
	EXTERN_BAGKEY(name);
	EXTERN_BAGKEY(fullName);
	EXTERN_BAGKEY(password);
	EXTERN_BAGKEY(id);

};


VError ThrowError(VError err, const VString* param1 = nil, const VString* param2 = nil, const VString* param3 = nil);

inline VError ThrowError(VError err, const VString& param1)
{
	return ThrowError(err, &param1);
}

inline VError ThrowError(VError err, const VString& param1, const VString& param2)
{
	return ThrowError(err, &param1, &param2);
}

inline VError ThrowError(VError err, const VString& param1, const VString& param2, const VString& param3)
{
	return ThrowError(err, &param1, &param2, &param3);
}

#endif

