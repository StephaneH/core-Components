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
#include "UAGHeaders.h"



namespace uag
{
	CREATE_BAGKEY(group);
	CREATE_BAGKEY(user);
	CREATE_BAGKEY(name);
	CREATE_BAGKEY(fullName);
	CREATE_BAGKEY(password);
	CREATE_BAGKEY(id);

};



VError ThrowError(VError err, const VString* param1, const VString* param2, const VString* param3)
{
	VErrorBase *ERR = new VErrorBase(err, 0);
	if (param1 != nil)
		ERR->GetBag()->SetString(L"p1", *param1);
	if (param2 != nil)
		ERR->GetBag()->SetString(L"p2", *param2);
	if (param3 != nil)
		ERR->GetBag()->SetString(L"p3", *param3);
	VTask::GetCurrent()->PushRetainedError( ERR);

	return err;
}
