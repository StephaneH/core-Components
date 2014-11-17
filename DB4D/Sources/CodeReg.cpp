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

VError CodeReg::Register(uLONG id, void* Code)
{
	VError err;
	CoupleCodeID CCID;
	
	err=VE_OK;
	if (id<10000)
	{
		if (((sLONG)id+1)>ar1.GetCount())
		{
			if (ar1.AddNSpaces((id+1)-ar1.GetCount(),true))
			{
				ar1[id].ID=id;
				ar1[id].Code=Code;
			}
			else 
				err=vThrowError(memfull);
		}
		else
		{
			ar1[id].ID=id;
			ar1[id].Code=Code;
		}
	}
	else
	{
		CCID.ID=id;
		CCID.Code=Code;
		if (!ar2.Add(CCID)) 
			err=vThrowError(memfull);
	}
	
	if (err != VE_OK)
			err=vThrowError(VE_DB4D_CANNOTREGISTERCODE);
	return(err);
}


void* CodeReg::GetCodeFromID(uLONG id)
{
	sLONG i,nb;
	CoupleCodeID *p;
	void* res;
	
	if (id<10000)
	{
		if (id < (uLONG)ar1.GetCount())
			res = ar1[id].Code;
		else
			res = nil;
	}
	else
	{
		nb=ar2.GetCount();
		res=nil;
		p=nb > 0 ? (CoupleCodeID*) &(ar2[0]) : nil; /* SGT - 25 May - Fix xbox_assert. */
		for (i=0;i<nb;i++)
		{
			if (p->ID==id)
			{
				res=p->Code;
				break;
			}
			p++;
		}
	}
	return (res);
}




VError AttributReg::Register(uLONG id, sLONG attribut)
{
	VError err;
	CoupleAttributID CCID;
	
	err=VE_OK;
	if (id<10000)
	{
		if (((sLONG)id+1)>ar1.GetCount())
		{
			if (ar1.AddNSpaces(id+1-ar1.GetCount(),true))
			{
				ar1[id].ID=id;
				ar1[id].attribut=attribut;
			}
			else
				err=vThrowError(memfull);
		}
		else
		{
			ar1[id].ID=id;
			ar1[id].attribut=attribut;
		}
	}
	else
	{
		CCID.ID=id;
		CCID.attribut=attribut;
		if (!ar2.Add(CCID))
			err=vThrowError(memfull);

	}
	
	if (err != VE_OK)
			err=vThrowError(VE_DB4D_CANNOTREGISTERCODE);
	return(err);
}


sLONG AttributReg::GetAttributFromID(uLONG id)
{
	sLONG i,nb;
	CoupleAttributID *p;
	sLONG res;
	
	if (id<10000)
	{
		return(ar1[id].attribut);
	}
	else
	{
		nb=ar2.GetCount();
		res=0;
		p=(CoupleAttributID*) &(ar2[0]);
		for (i=1;i<nb;i++)
		{
			if (p->ID==id)
			{
				res=p->attribut;
				break;
			}
			p++;
		}
		return(res);
	}
}





