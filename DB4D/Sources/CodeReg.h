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
#ifndef __CODEREG__
#define __CODEREG__


class CoupleCodeID
{
	public:
	
	Boolean operator == (const CoupleCodeID& other) const { return ID == other.ID; };
	
	uLONG ID;
	void* Code;
};


class CoupleAttributID
{
	public:
	
	Boolean operator == (const CoupleAttributID& other) const { return ID == other.ID; };

	uLONG ID;
	sLONG attribut;
};


typedef V0ArrayOf<CoupleAttributID> CoupleAttributIDArray;
typedef V0ArrayOf<CoupleCodeID> CoupleCodeIDArray;


class CodeReg : public VObject
{
	public:
		VError Register(uLONG id, void* Code);
		void* GetCodeFromID(uLONG id);
	
	protected:
		CoupleCodeIDArray ar1;
		CoupleCodeIDArray ar2;
};


class AttributReg : public VObject
{
	public:
		VError Register(uLONG id, sLONG attribut);
		sLONG GetAttributFromID(uLONG id);
	
	protected:
		CoupleAttributIDArray ar1;
		CoupleAttributIDArray ar2;
};




#endif
