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
#ifndef __DB4DServer__
#define __DB4DServer__

#include "../Headers/DB4D.h"
#include "ServerNet/VServerNet.h"


class ServerManager : public VObject, public IRefCountable
{
	public:
		ServerManager();
		virtual ~ServerManager();

		VError Start(sWORD inPortNumber, bool inSSL = false);
		void Stop();
	
	protected:
		VServer* fServer;

};


class DB4DConnectionHandler;

typedef set<DB4DConnectionHandler*> DB4DConnectionHandlerCollection;

class DB4DConnectionHandler : public VConnectionHandler
{
	public :

		enum	{ Handler_Type = 'DB4D' };

		DB4DConnectionHandler ( );
		virtual ~DB4DConnectionHandler ( );

		virtual VError SetEndPoint ( VEndPoint* inEndPoint );
		virtual bool CanShareWorker ( ) { return false; } /* Exclusive. */

		virtual enum VConnectionHandler::E_WORK_STATUS Handle ( VError& outError );

		virtual int GetType ( ) { return Handler_Type; }
		virtual VError Stop ( ) { fShouldStop = true; return VE_OK; }

		static void AddNewConnection(DB4DConnectionHandler* connection);

		static Boolean AcceptNewConnection() { return sAcceptNewConnection; };

		static void CloseAllConnections();

		static void CloseConnection(CDB4DContext* inContext);

		void ForceClose()
		{
			//fEndPoint->SetIsBlocking(false);
			fEndPoint->ForceClose();
			fShouldStop = true;
			VTask* taskowner = VTaskMgr::Get()->RetainTaskByID(fOwnerTask);
			if (taskowner != nil)
			{
				taskowner->Kill();
				taskowner->Release();
			}
		}

		inline CDB4DContext* GetContext() const
		{
			return fContext;
		}

	protected :
		VTCPEndPoint* fEndPoint;
		CDB4DContext* fContext;
		Boolean fClientNeedSwap;
		Boolean fClientIsSwapped;
		Boolean fShouldStop;
		VTaskID fOwnerTask;

		static DB4DConnectionHandlerCollection sAllConnections;
		static VCriticalSection sAllConnectionsMutex;
		static Boolean sAcceptNewConnection;
};


class DB4DConnectionHandlerFactory : public VTCPConnectionHandlerFactory
{
	public:

		virtual VConnectionHandler* CreateConnectionHandler ( VError& outError );
		virtual VError AddNewPort ( PortNumber iPort );
		virtual VError GetPorts ( vector<PortNumber>& oPorts );

		virtual int GetType ( ) { return DB4DConnectionHandler::Handler_Type; }

	protected:
		vector<PortNumber> fPorts;
};


class TCPOutputStream : public VStream
{
public:
	TCPOutputStream( VTCPEndPoint* inEndPoint):fEndPoint( inEndPoint), fSentBytes( 0), fNextPacketStatus( 0)	{;}

protected:
	// Inherited from VStream
	virtual VError	DoOpenWriting();
	virtual	VError	DoCloseWriting( Boolean inSetSize);
	virtual VError	DoOpenReading();
	virtual VError	DoPutData( const void* inBuffer, VSize inNbBytes);
	virtual	VError	DoGetData(void* inBuffer, VSize* ioCount);
	virtual VError	DoSetPos( sLONG8 inNewPos);
	virtual sLONG8	DoGetSize();
	virtual VError	DoSetSize( sLONG8 inNewSize);
	virtual VError	DoFlush ();

private:
	VTCPEndPoint*					fEndPoint;
	sLONG8							fSentBytes;
	XBOX::RequestStatusCode			fNextPacketStatus;
	XBOX::VMemoryBuffer<>			fBuffer;
};



class TCPRequestReply : public VObject, public IStreamRequestReply
{
	public:
		TCPRequestReply( VTCPEndPoint* inEndPoint, sWORD inRequestID, const void *inRequestData, size_t inRequestDataSize, Boolean inSwapForClient);
		virtual ~TCPRequestReply();

		sWORD GetRequestID() const { return fRequestID;}


		// IStreamRequestReply
		virtual	VStream* GetInputStream() { return &fInputStream;}
		virtual	VStream* GetOutputStream() { return &fOutputStream;}
		virtual	void InitReply( XBOX::RequestStatusCode inErrorCode);

		const void* GetRemainingInputDataPtr();
		XBOX::VSize GetRemainingInputDataSize();

	private:

		sWORD fRequestID;
		VConstPtrStream	fInputStream;
		TCPOutputStream	fOutputStream;
};



class TCPRequest : public VObject, public IStreamRequest
{
	public:
		TCPRequest( VTCPEndPoint* inEndPoint, Boolean inClientNeedSwap, sWORD inRequestID, CDB4DBaseContext *inContext);
		virtual ~TCPRequest();

		virtual	VStream* GetInputStream() { return &fInputStream;}
		virtual	VStream* GetOutputStream() { return &fOutputStream;}
		virtual	VError Send();

	protected:
		TCPOutputStream	fOutputStream;
		VPtrStream fInputStream;
		VError fSendError;
		VTCPEndPoint* fEndPoint;
		CDB4DBaseContext *fContext;
		sWORD fRequestID;
		Boolean fClientNeedSwap;
		Boolean fSent;
};


typedef map<CDB4DBaseContext*, VTCPEndPoint*> EndPointsCollection;

class DB4DNetManager;

class DB4DClientSession : public VTCPClientSession
{
	public :
		DB4DClientSession(DB4DNetManager* inNetManager)
		{
			fNetManager = inNetManager;
		}

		virtual VError Postpone ( VTCPEndPoint& inEndPoint );
		virtual VTCPEndPoint* Restore ( VTCPEndPoint& inEndPoint, VError& outError );

	private:
		DB4DNetManager* fNetManager;
};


class DB4DNetManager : public XBOX::VObject, public XBOX::IRefCountable, public DB4DNetworkManager
{
	public:
		inline DB4DNetManager(VTCPEndPoint* inEndPoint, const VString& inServerName, sWORD inPortNum, bool inSSL) 
		{ 
			fEndPoint = inEndPoint; 
			fClientNeedSwap = false;
			fServerName = inServerName;
			fPortNum = inPortNum;
			fSSL = inSSL;
			fClientSession = new DB4DClientSession(this);
		};

		virtual ~DB4DNetManager();
		VError OpenConnection();
		VError CloseConnection();

		static DB4DNetManager* NewConnection(const VString& inServerName, sWORD inPortNum, VError& outerr, bool inSSL = false);

		// from DB4DNetworkManager
		virtual IStreamRequest*	CreateRequest( CDB4DBaseContext *inContext, sWORD inRequestID);
		virtual	void GetServerAddressAndPort( XBOX::VString& outServerAddress, sLONG *outApplicationServerPortNumber, sLONG *outDB4DServerPortNumber, bool* outUseSSL) { xbox_assert( false);}
		virtual	sLONG GetServerVersion() { xbox_assert( false);return 0;}	// implemented only by legacy
		virtual NetWorkManager_Type		GetType() const
		{
			return DB4D_NetworkManager;
		}

		virtual void Dispose()
		{
			Release();
		}

		void RemoveContext(CDB4DBaseContext *inContext);
		VError SetContextIdleTimeOut(CDB4DBaseContext *inContext, uLONG inMilliseconds);
		VError SetIdleTimeOut(uLONG inMilliseconds);

		static void CloseAllConnections();
		static void CloseConnection(CDB4DBaseContext* inContext);

		Boolean ClientNeedsSwap ( ) { return fClientNeedSwap; }

	private:
		VTCPEndPoint* fEndPoint;
		EndPointsCollection fEndPointsByContext;
		VString fServerName;
		sWORD fPortNum;
		bool fSSL;
		Boolean fClientNeedSwap;
		VCriticalSection fMutex;
		DB4DClientSession* fClientSession;
		static EndPointsCollection sEndPointsByContext;
		static VCriticalSection sMutex;
		static bool sAcceptNewConnection;
};


void SendError(IRequestReply *inRequest, CDB4DBaseContext *inContext, VError errToSend);




// ------------------------------------------------------------------------------------------------

class FieldsForCache;

class FieldsForCacheOnOneTable : public VObject, public IRefCountable
{
	public:
		
		inline FieldsForCacheOnOneTable(FieldsForCache* owner)
		{
			fOwner = owner;
		}

		inline void AddField(sLONG FieldNum)
		{
			//fOwner->SetSomeRequestedFieldsHaveChanged(true);
			fFields.Set(FieldNum);
		}

		inline void AddField(Field* cri)
		{
			//fOwner->SetSomeRequestedFieldsHaveChanged(true);
			if (cri != nil)
				fFields.Set(cri->GetPosInRec());
			else
				fFields.Set(0);
		}

		inline Boolean IsFieldIn(sLONG FieldNum)
		{
			return fFields.isOn(FieldNum);
		}

		inline Boolean IsFieldIn(Field* cri)
		{
			return fFields.isOn(cri->GetPosInRec());
		}

		FieldsForCacheOnOneTable* Clone() const;

		VError WriteToStream(VStream* ToStream)
		{
			return fFields.WriteToStream(ToStream);
		}

		VError ReadFromStream(VStream* FromStream)
		{
			return fFields.ReadFromStream(FromStream);
		}


		void SetOwner(FieldsForCache* owner)
		{
			fOwner = owner;
		}

		FieldsForCache* GetOwner() const
		{
			return fOwner;
		}


		VError FillStreamWithData(FicheInMem* rec, VStream* toStream, BaseTaskInfo* context);

		VError ReadDataFromStream(FicheInMem* rec, VStream* fromStream, BaseTaskInfo* context);


	protected:
		SmallBittab<32,65536> fFields;
		FieldsForCache* fOwner;

};


typedef std::map<sLONG, VRefPtr<FieldsForCacheOnOneTable> > FieldsForCacheMap;
typedef std::map<sLONG, RelationPath > RelationsCacheMap;

class RemoteRecordCache;
class RowCacheInfo;

class FieldsForCache
{
	public:

		inline FieldsForCache(Table* inTable)
		{
			fMainTable = RetainRefCountable(inTable);
			fStamp = 0;
		}

		inline ~FieldsForCache()
		{
			QuickReleaseRefCountable(fMainTable);
		}

		inline void Touch()
		{
			fStamp++;
		}

		inline sLONG GetStamp() const
		{
			return fStamp;
		}

		FieldsForCacheOnOneTable* RetainFieldsForTable(Table* inTable);
		FieldsForCacheOnOneTable* GetFieldsForTable(Table* inTable);
		VError AddField(Field* inField);

		void AddSel(sLONG TableNum);

		VError WriteToStream(VStream* ToStream);
		VError ReadFromStream(VStream* FromStream);

		void SetSomeRequestedFieldsHaveChanged(Boolean b)
		{
			fSomeRequestedFieldsHaveChanged = b;
		}

		Boolean HaveSomeRequestedFieldsChanged() const
		{
			return fSomeRequestedFieldsHaveChanged;
		}

		inline Table* GetMainTable() const
		{
			return fMainTable;
		}

		void GetTableNums(vector<sLONG>& outNums);

		VError BuildRelationsOnServer(BaseTaskInfo* context, const vector<uBYTE>& inWayOfLocking);

		VError FillStreamWithData(FicheInMem* mainrec, VStream* toStream, BaseTaskInfo* context);

		VError ReadDataFromStream(RowCacheInfo* row, VStream* fromStream, BaseTaskInfo* context, RemoteRecordCache* displaycache);


	protected:
		Table* fMainTable;
		FieldsForCacheMap fAllFields;
		RelationsCacheMap fRelPaths;
		sLONG fStamp;
		Boolean fSomeRequestedFieldsHaveChanged;

};


typedef std::map<sLONG, VRefPtr<FicheInMem> > RelatedRecordCacheInfoMap;

class RowCacheInfo
{
	public:

		RowCacheInfo()
		{
			fMainRec = nil;
		}

		~RowCacheInfo()
		{
			for (RelatedRecordCacheInfoMap::iterator cur = fRelatedRecords.begin(), end = fRelatedRecords.end(); cur != end; cur++)
			{
				FicheInMem* rec = cur->second.Get();
				if (rec != nil)
					rec->ClearCacheDisplay();
			}

			if (fMainRec != nil)
				fMainRec->ClearCacheDisplay();
			QuickReleaseRefCountable(fMainRec);
		}

		FicheInMem* fMainRec;
		RelatedRecordCacheInfoMap fRelatedRecords;

};

class EmptyRelatedInfo
{
	public:
		/*
		inline EmptyRelatedInfo()
		{
			fFakeRecord = nil;
			fFakeSelection = nil;
		}

		inline ~EmptyRelatedInfo()
		{
			QuickReleaseRefCountable(fFakeRecord);
			QuickReleaseRefCountable(fFakeSelection);
		}
		*/

		ListOfRelation fRelPath;
		//FicheInMem* fFakeRecord;
		//Selection* fFakeSelection;
};


typedef std::vector<RowCacheInfo> RowCacheInfoArray;

typedef std::map<sLONG, EmptyRelatedInfo> EmptyRelatedInfoMap;

class TempRelInfo
{
	public:
		Table* fTable;
		Relation* fRel;
};

class RemoteRecordCache : public VObject, public IRefCountable
{
	public:

		inline RemoteRecordCache(FieldsForCache* inFields, BaseTaskInfo* inContext)
		{
			fFields = inFields;
			fContext = inContext;
			fCurRecIndex = -1;
			fStartRecIndex = -1;
			fEndRecIndex = -1;
			fSel = nil;
		}

		virtual ~RemoteRecordCache()
		{
			QuickReleaseRefCountable(fSel);
		}

		VError RetainCacheRecord(RecIDType inRecIndex, CDB4DRecord* &outRecord, vector<CachedRelatedRecord>& outRelatedRecords);

		VError InitAllRelatedRecordsOnClient();

		FicheInMem* RetainTrueRelatedRecord(Table* targetTable, VError& err);

		VError StartCachingRemoteRecords(Selection* inSel, RecIDType FromRecIndex, RecIDType ToRecIndex, const vector<uBYTE>& inWayOfLocking);

		inline bool OKTable(sLONG numtable) const
		{
			if (numtable <= fWayOfLocking.size() && fWayOfLocking[numtable-1] == 255)
				return false;
			else
				return true;
		}

	protected:

		VError _InitAllRelatedRecords(Table* onTable, vector<TempRelInfo>& outNewRels);

		FieldsForCache* fFields;
		RowCacheInfoArray fRows;
		SmallBittab<64, 65536> fDejaScanRel;
		BaseTaskInfo* fContext;
		sLONG fCurRecIndex;
		sLONG fStartRecIndex;
		sLONG fEndRecIndex;
		sLONG fStampCopy;
		Selection* fSel;
		vector<uBYTE> fWayOfLocking;

		EmptyRelatedInfoMap fEmptyRelatedInfos;
};


bool _WithSelectIo();


class VDBMgr;

class ServerProgressTask : public VTask
{
public:
	ServerProgressTask( VDBMgr* inManager):VTask(inManager, 128*1024L,eTaskStylePreemptive,NULL),fManager(inManager)
	{
		SetKind( kDB4DTaskKind_GarbageCollector);
	}

protected:
	virtual void DoInit();
	virtual void DoDeInit();
	virtual Boolean DoRun();

	VDBMgr *fManager;
	mutable vxSyncEvent fTimer;
};



class ClientProgressTask : public VTask
{
public:
	ClientProgressTask( VDBMgr* inManager):VTask(inManager, 128*1024L,eTaskStylePreemptive,NULL),fManager(inManager)
	{
		SetKind( kDB4DTaskKind_GarbageCollector);
	}

protected:
	virtual void DoInit();
	virtual void DoDeInit();
	virtual Boolean DoRun();

	VDBMgr *fManager;
	mutable vxSyncEvent fTimer;
};


#endif
