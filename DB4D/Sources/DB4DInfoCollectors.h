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
#ifndef __DB4D_INFO_COLLECTORS__ 
#define __DB4D_INFO_COLLECTORS__

#if WITH_RTM_DETAILLED_ACTIVITY_INFO

/**
 * This file contains concrete database monitoring information collectors.
 * VDB4DBasicInfoCollector is the foundation for all other collectors.
 * Usage guidelines: 
 * <ul>
 * <li> the collector shall be created on the stack (automatic storage class) </li>
 * <li> the collector shall only be created and destroyed in the thread that will perform the monitored operation </li>
 * </ul>
 */

/**
* \class VDB4DBasicInfoCollector
* \brief Base class for all database monitoring information collectors.
* \details Retrieves generic database monitoring information from a database context
*/
class VDB4DBasicInfoCollector : public XBOX::VObject, public XBOX::IProgressInfoCollector
{
public:
	enum OperationType
	{
		kDeleteRecords = 0,
		kDistinctValues,
		kSelectionToArray,
		kArrayToSelection,
		kOrderBy,
		kQuery,
		kLast
	};

	VDB4DBasicInfoCollector(CDB4DBaseContext* inBaseContext);

	virtual ~VDB4DBasicInfoCollector();

	virtual	XBOX::VJSONObject*	CollectInfo(VProgressIndicator *inIndicator, bool inForRootSession);

	bool					LoadMessageString(XBOX::VString& ioMessage){ return false; }
protected:
	/**
	 * \brief Factors out code for building the user and client info
	 * \param inBaseContextLockInfo source object containing user and client info
	 * \param outContainerObject object that will contain the info from @inBaseContextLockInfo
	 */
	void _BuildDbContextInfoProperty(const VJSONObject* inBaseContextLockInfo, VJSONObject* outContainerObject);

	void _GetOperationTypeCaption(OperationType inType, XBOX::VString& outCaption)const;

protected:
	/// database context from which to retrieve user and client related info
	CDB4DBaseContext* fBaseContext;

private:
	VDB4DBasicInfoCollector(const VDB4DBasicInfoCollector&);
	VDB4DBasicInfoCollector();
};

/**
* \class VDB4DQueryInfoCollector
* \brief Retrieves query related information and packages them fro the real time moniroting
*/
class VDB4DQueryInfoCollector : public VDB4DBasicInfoCollector
{
public:
	VDB4DQueryInfoCollector(CDB4DBaseContext* inContext,OptimizedQuery* inQuery,VProgressIndicator* ioProgressIndicator = NULL);

	virtual ~VDB4DQueryInfoCollector();

	/**
	* \brief Retrieves the the target query's plan
	* \param inIndicator the progress indicator which is being queried for progress information
	*/
	virtual	VJSONObject*	CollectInfo(VProgressIndicator *inIndicator, bool inForRootSession);

private:
	VDB4DQueryInfoCollector();
	VDB4DQueryInfoCollector(const VDB4DQueryInfoCollector&);

	//Query related info that's been retrieved
	XBOX::VJSONObject* fCollectedInfo;

	//Query being monitored
	OptimizedQuery*		fQuery;

	//Monitoring progress indicator (i.e. the one we're a collector for)
	XBOX::VProgressIndicator* fProgressIndicator;
	
};

/**
* \class VDB4DQueryInfoCollector
* \brief Retrieves monitoring information about a sort operation
*/
class VDB4DSortInfoCollector : public VDB4DBasicInfoCollector
{
public:

	VDB4DSortInfoCollector(CDB4DBaseContext* inBaseContext, Table* inTable, SortTab* inSortLines,XBOX::VProgressIndicator* inProgressIndicator);
	virtual ~VDB4DSortInfoCollector();
	virtual	XBOX::VJSONObject*	CollectInfo(VProgressIndicator *inIndicator, bool inForRootSession);

private:
	VDB4DSortInfoCollector();
	VDB4DSortInfoCollector(const VDB4DSortInfoCollector&);
private:
	Table* fTable;
	const SortTab* fSortLines;
	XBOX::VJSONObject* fCollectedInfo;
	XBOX::VProgressIndicator* fProgressIndicator;
};

/**
* \class VDB4DSelectionInfoCollector
* \brief Retrieves monitoring information for operations working on record selections
*/
class VDB4DSelectionInfoCollector : public VDB4DBasicInfoCollector
{
public:
	
	VDB4DSelectionInfoCollector(CDB4DBaseContext* inContext, DataTable* inTargetTable, OperationType inOperationType);
	VDB4DSelectionInfoCollector(CDB4DBaseContext* inContext, Selection* inSelection, DB4DCollectionManager* inCollectionManager, OperationType inOperationType);
	VDB4DSelectionInfoCollector(CDB4DBaseContext* inContext, Field* inTargetField, IndexInfo* inIndexInfo, XBOX::VProgressIndicator * inIndicator, OperationType inOperationType);
	virtual ~VDB4DSelectionInfoCollector();

	virtual	VJSONObject*	CollectInfo(XBOX::VProgressIndicator * inIndicator, bool inForRootSession);

protected:
	DB4DCollectionManager*		fCollectionManager;
	Table*						fTargetTable;
	Field*						fTargetField;
	IndexInfo*					fTargetIndex;
	XBOX::VJSONObject*			fCollectedInfo;
	XBOX::VProgressIndicator*	fProgressIndicator;
	OperationType				fDB4DOperation;
private:
	VDB4DSelectionInfoCollector();
	VDB4DSelectionInfoCollector(const VDB4DSelectionInfoCollector&);
};

/**
* \class VIndexInfoProvider
* \brief Retrieves monitoring information for indexer operations
*/
class VIndexInfoProvider : public VDB4DBasicInfoCollector
{
public:
	VIndexInfoProvider(IndexAction& inIndexAction);
	virtual ~VIndexInfoProvider();

	virtual	XBOX::VJSONObject*	CollectInfo(VProgressIndicator *inIndicator,bool inForRootSession);

private:
	VIndexInfoProvider(const VIndexInfoProvider&);
	VIndexInfoProvider();

private:
	IndexInfo* fIndexInfo;
	XBOX::VJSONObject* fCollectedInfo;
	const XBOX::VJSONObject* fLockInfo;
};
#else
/**
* \class VDB4DQueryInfoCollector
* \brief Adapts query-related information as progress indicator extra info.
*/
class VDB4DQueryInfoCollector : public XBOX::VObject, public XBOX::IProgressInfoCollector
{
public:
	VDB4DQueryInfoCollector(CDB4DBaseContext* inContext, OptimizedQuery* inQuery);

	virtual ~VDB4DQueryInfoCollector();

	/**
	* \brief Retrieves the the target query's plan
	* \param inIndicator the progress indicator which is being queried for progress information
	*/
	virtual	XBOX::VJSONObject*	CollectInfo(XBOX::VProgressIndicator *inIndicator);

	virtual	bool			LoadMessageString(XBOX::VString& ioMessage);

protected:
	///Base context to retrieve the query path from
	CDB4DBaseContext*	fBaseContext;
	OptimizedQuery*		fQuery;
	XBOX::VJSONObject*	fCollectedInfo;

private:
	VDB4DQueryInfoCollector();
	VDB4DQueryInfoCollector(const VDB4DQueryInfoCollector&);
};

class VIndexInfoProvider : public XBOX::VObject, public XBOX::IProgressInfoCollector
{
public:
	VIndexInfoProvider(IndexAction& inIndexAction);

	virtual	XBOX::VJSONObject*	CollectInfo(VProgressIndicator *inIndicator);

	bool					LoadMessageString(XBOX::VString& ioMessage){ return false; }
private:
	VIndexInfoProvider(const VIndexInfoProvider&);

private:
	IndexInfo* fIndexInfo;
};

#endif //WITH_RTM_DETAILLED_ACTIVITY_INFO

#endif //__DB4D_INFO_COLLECTORS__
