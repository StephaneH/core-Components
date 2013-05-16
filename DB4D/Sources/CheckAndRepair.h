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
#ifndef __CHECKANDREPAIR__
#define __CHECKANDREPAIR__

/*
typedef enum {
	
} IncidentType;



class Incident : public VObject
{
	public:
		Incident(IncidentType Type) { fType = Type; };
		inline IncidentType GetType() const { return fType; };

		virtual void GetDescription(VString& outDesc) const ;


	protected:
		IncidentType fType;

};
*/


class CheckAndRepairAgent : public VObject
{
	public:
		CheckAndRepairAgent(Base4D *target);
		virtual ~CheckAndRepairAgent();

		VError Run(VStream* outMsg, ListOfErrors& OutList, BaseTaskInfo* Context, VDB4DProgressIndicator* InProgress = nil);
		void SetFullCheck();

		inline Base4D* GetTarget() { return fTarget; };
		inline VDB4DProgressIndicator* GetProgressIndicator() { return fProgress; };
		inline BaseTaskInfo* GetContext() { return fContext; };

		inline void SetCheckTableState(Boolean OnOff) { fCheckTables = OnOff; };
		inline void SetCheckAllTablesState(Boolean OnOff) { fCheckAllTables = OnOff; };

		inline void SetCheckIndexState(Boolean OnOff) { fCheckIndexes = OnOff; };
		inline void SetCheckAllIndexesState(Boolean OnOff) { fCheckAllIndexes = OnOff; };

		inline void SetCheckBlobsState(Boolean OnOff) { fCheckBlobs = OnOff; };

	protected:
		VError RunCheckTables();
		VError CheckOneTable(Table* target);

		VError ThrowError( VError inErrCode, ActionDB4D inAction);

		Base4D* fTarget;
		VDB4DProgressIndicator* fProgress;
		BaseTaskInfo* fContext;
		ListOfErrors* fErrors;

		Boolean fCheckTables;
		Boolean fCheckAllTables;
		VArrayOf<Table*> fTablesToCheck;

		Boolean fCheckBlobs;

		Boolean fCheckIndexes;
		Boolean fCheckAllIndexes;
		IndexArray fIndexesToCheck;

		Boolean fCheckAllocatedSpace;  // bittable

		Boolean fRecoverByTags;

		VStream* fTextLog;
		Boolean fTextLogIsOwned;
};



#endif
