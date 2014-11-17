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
#include "4DDBHeaders.h"

VError EnlargeCollection(const VValueSingle& val, DB4DCollectionManager& outCollection, distinctvalue_iterator& xx)
{
	VError err = VE_OK;
	xx.nbelem++;
	if (xx.nbelem > xx.truenbelem)
	{
		xx.truenbelem += 4096;
		err = outCollection.SetCollectionSize(xx.truenbelem, false);
		if (err != VE_OK)
		{
			xx.truenbelem = xx.truenbelem - 4096;
			xx.nbelem--;
		}
	}
	if (err == VE_OK)
	{
		outCollection.SetNthElement(xx.nbelem, 1, val);
	}
	return err;
}

						
						
						
						




