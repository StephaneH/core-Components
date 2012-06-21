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
#ifndef __SORT4D__
#define __SORT4D__


class EntityAttribute;

class SortLine
{
	public:
	
		inline SortLine() { isfield = true; expression = nil; att = nil;};
		Boolean operator == (const SortLine& other) const { return (numfield == other.numfield) && (numfile == other.numfile); };

		VError PutInto(VStream* into);
		VError GetFrom(VStream* from);

		sLONG numfield;
		sLONG numfile;
		uBOOL ascendant;	
		uBOOL isfield;
		DB4DLanguageExpression* expression;
		EntityAttribute* att;
};

/*
class SortLineArray : public VArrayVIT
{
public:
	inline SortLineArray(sLONG pNbInit = 0,sLONG pNbToAdd = 1):VArrayVIT(sizeof(SortLine),pNbInit,pNbToAdd){;};
	inline SortLine& operator[](sLONG pIndex) {CALLRANGECHECK(pIndex);return (((SortLine *)(*fTabHandle))[pIndex]);};
};
*/

typedef Vx0ArrayOf<SortLine, 10> SortLineArray;


class SortTab : public Obj4D, public IObjCounter
{
	public:
		SortTab(Base4D* inBD) { fBD = inBD; };
		virtual ~SortTab();
		VError AddTriLineField(sLONG numfile, sLONG numfield, uBOOL ascendant = true);
		VError AddTriLineField(Field* cri, uBOOL ascendant = true);
		inline void SetTriLineAscendant(sLONG numline, Boolean ascent) { li[numline-1].ascendant = ascent; };
		inline void GetTriLine(sLONG numline, SortLine* xli) const { *xli = li[numline-1]; };
		inline SortLine* GetTriLineRef(sLONG numline) { return &(li[numline-1]); };
		inline const SortLine* GetTriLineRef(sLONG numline) const {return &(li[numline-1]); };
		inline sLONG GetNbLine(void) const { return(li.GetCount()); };
		Boolean AddExpression(DB4DLanguageExpression* inExpression, Boolean ascendant = true);
		Boolean AddAttribute(EntityAttribute* inAtt, Boolean ascendant = true);

		VError PutInto(VStream* into);
		VError GetFrom(VStream* from);

		SortLineArray* GetLI()
		{
			return &li;
		}
		
	protected:
		SortLineArray li;
		Base4D* fBD;
};

#endif

