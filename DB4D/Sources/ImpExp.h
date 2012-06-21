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
#ifndef __IMPEXP__
#define __IMPEXP__


class ImpExp : public Obj4D, public IObjCounter
{
	public:
		ImpExp(Table* inTable);
		
		Table* GetTarget(void) const { return target; };
		
		Boolean AddCol(Field* inField);
		sLONG CountCol(void) const { return cols.GetCount(); };
		Field* GetCol(sLONG n) const;
		
		void SetPath(const VFilePath& newpath);
		void SetPath(const VString& newpath);
		
		void GetPath(VFilePath& curpath) const;
		void GetPath(VString& curpath) const;
		
		void SetColDelimit(const VString& newColDelimit);
		void GetColDelimit(VString& curColDelimit) const;
		
		void SetRowDelimit(const VString& newRowDelimit);
		void GetRowDelimit(VString& curRowDelimit) const;
		
		void SetCharSet(CharSet newset);
		CharSet GetCharSet() { return curset; };
		

		VError RunImport(Selection* &outSel, BaseTaskInfo* context = nil, VDB4DProgressIndicator* InProgress = nil);
		VError RunExport(Selection* inSel, BaseTaskInfo* context = nil, VDB4DProgressIndicator* InProgress = nil);
		
	protected:
	
		Boolean NextToken(VFileDesc* f, VString& outToken, Boolean& outEOL, Boolean& outEOF, VError& err);
		UniChar NextChar(VFileDesc* f, Boolean& outEOF, VError& err);
		VError ThrowError( VError inErrCode, ActionDB4D inAction);
	
		VArrayRetainedOwnedPtrOf<Field*> cols;
		VString ColDelimit;
		VString RowDelimit;
		VFilePath path;
		Table* target;
		CharSet curset;
};



const VErrorDB4D	VE_DB4D_SYNCHELPER_NOT_VALID	= MAKE_VERROR(CDB4DManager::Component_Type, 3000);
const VErrorDB4D	VE_DB4D_SYNCHELPER_MISSING_TABLECAT	= MAKE_VERROR(CDB4DManager::Component_Type, 3001);

typedef map<Table*, Table*> TableMatchingMap;

class SynchroBaseHelper
{
	public:
		SynchroBaseHelper(Base4D* owner);

		~SynchroBaseHelper();

		Base4D* GetBase(VError& err, bool BuildIfMissing = true);

		VError SetSynchro(Table* tt, VectorOfVValue& primkeyvalues, uLONG8 syncstamp, uLONG8 timestamp, Sync_Action action, BaseTaskInfo* context);

		VError GetSynchroStamp(Table* tt, vector<VValueSingle*>& primkeyvalues, uLONG8& outSyncstamp, VTime& outTimestamp, Sync_Action& outAction, BaseTaskInfo* context);

		Table* GetTableSync(Table* tt, VError& err, bool BuildIfMissing = true);

		void RemoveTable(Table* tt);

		/*
		BaseTaskInfo* GetContext()
		{
			return fContext;
		}
		*/

		inline void Lock()
		{
			fMutex.Lock();
		}

		inline void Unlock()
		{
			fMutex.Unlock();
		}

	protected:
		Base4D* fBase;
		Base4D* fOwner;
		Table* fTableCat;
		TableMatchingMap fTableMap;
		//BaseTaskInfo* fContext;
		VCriticalSection fMutex;
};

#endif
