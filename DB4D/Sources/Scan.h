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
#ifndef __Scanner__
#define __Scanner__

enum {  DataObject_None = 0,
		DataObject_DataSegHeader = 1,
		DataObject_BittabPrim,
		DataObject_BittabSec,
		DataObject_Bittab,
		DataObject_SeqNum,
		DataObject_DataTable,
		DataObject_IndexDef,
		DataObject_TableDef,
		DataObject_RelationDef,
		DataObject_Blob,
		DataObject_Record,
		DataObject_TabAddr,
		DataObject_TabAddr_Record,
		DataObject_TabAddr_Blob,
		DataObject_TabAddr_Index,
		DataObject_TabAddr_IndexCluster,
		DataObject_ExtraProp,
		DataObject_BtreePage,
		DataObject_Cluster,
		DataObject_Cluster_Part,

		DataObject_Final };


class ToolLog;

class ObjectDiskPosInfo
{
	public :
		ObjectDiskPosInfo(DataAddr4D pos, sLONG len, sLONG type)
		{
			fPos = pos;
			fLen = len;
			fType = type;
		}

	protected:
		DataAddr4D fPos;
		sLONG fLen;
		sLONG fType;
};


class PartObjectDiskBitInfo
{
	public:
		inline PartObjectDiskBitInfo()
		{
			nbpassages = 0;
		}

		VRefPtr<Bittab> fBits;
		uLONG nbpassages;

};


typedef vector<PartObjectDiskBitInfo> BigBittab;

class ObjectDiskBitInfo
{
	public:
		inline ObjectDiskBitInfo()
		{
			fOverlapDetected = false;
			fErr = VE_OK;
		}

		inline Boolean OverlapDetected() const
		{
			return fOverlapDetected;
		}

		inline void SetOverLap()
		{
			fOverlapDetected = true;
		}

		Boolean IsOn(sLONG8 n);
		void Set(sLONG8 n);

		VError GetLastError() const
		{
			return fErr;
		}

	protected:
		BigBittab fEspace;
		Boolean fOverlapDetected;
		VError fErr;
};

typedef vector<ObjectDiskBitInfo> ObjectDiskBitInfoCollection;

class ObjectDiskPosInfoCollection
{
	public:
		
		inline ObjectDiskPosInfoCollection(Boolean KeepTempBase, Boolean WithBitsOnly)
		{
			fError = VE_OK;
			fDatas = nil;
			fAddrs = nil;
			fContext = nil;
			fDatasEncapsule = nil;
			fDeleteTempBaseOnClose = !KeepTempBase; 
			f1 = nil;
			f2 = nil;
			f3 = nil;
			f4 = nil;
			fWithBitsOnly = WithBitsOnly;
		}

		~ObjectDiskPosInfoCollection();

		VError Init(Base4D_NotOpened* source);

		FicheInMem* PrepareNewObject(DataAddr4D ou, sLONG len, sLONG type);
		FicheInMem* PrepareNewObject(DataAddr4D ou, sLONG len, sLONG type, sLONG param1);
		FicheInMem* PrepareNewObject(DataAddr4D ou, sLONG len, sLONG type, sLONG param1, sLONG param2);
		FicheInMem* PrepareNewObject(DataAddr4D ou, sLONG len, sLONG type, sLONG param1, sLONG param2, sLONG param3);
		FicheInMem* PrepareNewObject(DataAddr4D ou, sLONG len, sLONG type, sLONG param1, sLONG param2, sLONG param3, sLONG param4);

		void MarkAddr_SegHeader(DataAddr4D ou, sLONG len, sLONG segnum);
		void MarkAddr_PrimBittab(DataAddr4D ou, sLONG len, sLONG segnum);
		void MarkAddr_SecondaryBittab(DataAddr4D ou, sLONG len, sLONG segnum, sLONG SecBitNum);
		void MarkAddr_Bittab(DataAddr4D ou, sLONG len, sLONG segnum, sLONG SecBitNum, sLONG BittabNum);

		void MarkAddr_SeqNum(DataAddr4D ou, sLONG len, sLONG seqnum_num);

		void MarkAddr_DataTable(DataAddr4D ou, sLONG len, sLONG DataTable_num);

		void MarkAddr_StructElemDef(DataAddr4D ou, sLONG len, sLONG numobj, TypObjContainer xtypeobj);

		void MarkAddr_Blob(DataAddr4D ou, sLONG len, sLONG numtable, sLONG numblob);
		void MarkAddr_Record(DataAddr4D ou, sLONG len, sLONG numtable, sLONG numrec);

		void MarkAddr_Index(DataAddr4D ou, sLONG len, sLONG numindex, sLONG numpage);
		void MarkAddr_IndexCluster(DataAddr4D ou, sLONG len, sLONG numindex, sLONG numcluster);
		void MarkAddr_IndexCluster_part(DataAddr4D ou, sLONG len, sLONG numindex, sLONG numcluster, sLONG numpart);

		void MarkAddr_TabAddr_IndexCluster(DataAddr4D ou, sLONG len, sLONG numindex, sLONG pos, sLONG posparent);
		void MarkAddr_TabAddr_Index(DataAddr4D ou, sLONG len, sLONG numindex, sLONG pos, sLONG posparent);
		void MarkAddr_TabAddr_Record(DataAddr4D ou, sLONG len, sLONG numtable, sLONG pos, sLONG posparent);
		void MarkAddr_TabAddr_Blob(DataAddr4D ou, sLONG len, sLONG numtable, sLONG pos, sLONG posparent);
		void MarkAddr_TabAddr_Other(DataAddr4D ou, sLONG len, sLONG pos, sLONG posparent);

		void MarkAddr_ExtraProp(DataAddr4D ou, sLONG len);

		void TakeBits(DataAddr4D ou, sLONG len);

		inline void SetLastError(VError err)
		{
			if (err != VE_OK && fError == VE_OK)
				fError = err;
		}

		inline VError GetLastError()
		{
			return fError;
		}


		VError CheckOverlaps(ToolLog* log, VProgressIndicator* progress);

		enum { SegNum_FieldID = 1, Offset_FieldID, Len_FieldID, Type_FieldID, Param1_FieldID, Param2_FieldID, Param3_FieldID, Param4_FieldID, Numtable_FieldID, Numindex_FieldID };

	protected:
		CDB4DBase* fDatasEncapsule;
		Base4D* fDatas;
		Table* fAddrs;
		BaseTaskInfo* fContext;
		VError fError;
		Boolean fDeleteTempBaseOnClose, fWithBitsOnly;
		VFile* f1;
		VFile* f2;
		VFile* f3;
		VFile* f4;
		ObjectDiskBitInfoCollection fSegsBitInfo;
};


class ToolObject
{
	public:
		inline ToolObject(ToolObjectType xtyp, ToolObjectProblem xproblem, ToolObjectErrorLevel xlevel = TO_ErrorLevel_Normal) { typ = xtyp; problem = xproblem; level = xlevel; };
		inline ToolObjectType GetType() const { return typ; };
		inline ToolObjectProblem GetProblem() const { return problem; };
		inline void SetProblem(sLONG xproblem) { problem = xproblem; };
		inline void SetErrorLevel(ToolObjectErrorLevel xlevel) { level = xlevel; }
		inline ToolObjectErrorLevel GetErrorLevel() const { return level; }

		virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;

	protected:
		ToolObjectType typ;
		ToolObjectProblem problem;
		ToolObjectErrorLevel level;
};


class ToolObjectSimple : public ToolObject
{
	public:
		inline ToolObjectSimple(ToolObjectType xtyp, ToolObjectProblem xproblem,ToolObjectErrorLevel xlevel = TO_ErrorLevel_Normal) : ToolObject(xtyp, xproblem,xlevel) { ; };
	protected:
		sLONG param1, param2, param3, param4;
		DataAddr4D param5;
};



class DataBaseProblem : public ToolObjectSimple
{
	public:
		inline DataBaseProblem(ToolObjectProblem xproblem):ToolObjectSimple(TO_Base, xproblem) { ; };

		virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};

class DataSegProblem : public ToolObjectSimple
{
	public:
		inline DataSegProblem(ToolObjectProblem xproblem, sLONG numseg,ToolObjectErrorLevel xlevel = TO_ErrorLevel_Normal):ToolObjectSimple(TO_Seg, xproblem,xlevel) { param1 = numseg; };
		inline sLONG GetSegNum() const { return param1; };

		virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;

};

class BittableSecTableProblem : public ToolObjectSimple
{
public:
	inline BittableSecTableProblem(ToolObjectProblem xproblem, sLONG numseg, sLONG numsec):ToolObjectSimple(TO_BittableSec, xproblem) { param1 = numseg; param2 = numsec; };
	inline sLONG GetSegNum() const { return param1; };
	inline sLONG GetBittableTableNum() const { return param2; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};

class BittableProblem : public ToolObjectSimple
{
public:
	inline BittableProblem(ToolObjectProblem xproblem, sLONG numseg, sLONG numsec, sLONG numbittable):ToolObjectSimple(TO_Bittable, xproblem) 
	{ param1 = numseg; param2 = numsec; param3 = numbittable; };
	inline sLONG GetSegNum() const { return param1; };
	inline sLONG GetBittableTableNum() const { return param2; };
	inline sLONG GetBittableNum() const { return param3; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class DataTableProblem : public ToolObjectSimple
{
public:
	inline DataTableProblem(ToolObjectProblem xproblem, sLONG numtable, sLONG numtabledef = 0):ToolObjectSimple(TO_DataTable, xproblem) { param1 = numtable; param2 = numtabledef;};
	inline sLONG GetTableNum() const { return param1; };
	inline sLONG GetTableDefNum() const { return param2; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class DataTable_RecTabAddrProblem : public ToolObjectSimple
{
public:
	inline DataTable_RecTabAddrProblem(ToolObjectProblem xproblem, sLONG numtable, sLONG numTabAddr, sLONG numtabledef = 0):ToolObjectSimple(TO_DataTable_RecTabAddr, xproblem) 
	{ param1 = numtable; param2 = numTabAddr; param3 = numtabledef;};
	inline sLONG GetTableNum() const { return param1; };
	inline sLONG GetTabAddrNum() const { return param2; };
	inline sLONG GetTableDefNum() const { return param3; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class DataTable_BlobTabAddrProblem : public ToolObjectSimple
{
public:
	inline DataTable_BlobTabAddrProblem(ToolObjectProblem xproblem, sLONG numtable, sLONG numTabAddr, sLONG numtabledef = 0):ToolObjectSimple(TO_DataTable_BlobTabAddr, xproblem) 
	{ param1 = numtable; param2 = numTabAddr; param3 = numtabledef;};
	inline sLONG GetTableNum() const { return param1; };
	inline sLONG GetTabAddrNum() const { return param2; };
	inline sLONG GetTableDefNum() const { return param3; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class BlobProblem : public ToolObjectSimple
{
public:
	inline BlobProblem(ToolObjectProblem xproblem, sLONG numtable, sLONG numblob, sLONG numtabledef = 0):ToolObjectSimple(TO_Blob, xproblem) 
	{ param1 = numtable; param2 = numblob; param3 = numtabledef;};
	inline sLONG GetTableNum() const { return param1; };
	inline sLONG GetBlobNum() const { return param2; };
	inline sLONG GetTableDefNum() const { return param3; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class RecordProblem : public ToolObjectSimple
{
public:
	inline RecordProblem(ToolObjectProblem xproblem, sLONG numtable, sLONG numrec, sLONG numtabledef = 0):ToolObjectSimple(TO_Record, xproblem) 
	{ param1 = numtable; param2 = numrec; param3 = numtabledef;};
	inline sLONG GetTableNum() const { return param1; };
	inline sLONG GetRecordNum() const { return param2; };
	inline sLONG GetTableDefNum() const { return param3; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};



class FieldProblem : public ToolObjectSimple
{
public:
	inline FieldProblem(ToolObjectProblem xproblem, sLONG numtable, sLONG numrec, sLONG numfield, sLONG numtabledef = 0):ToolObjectSimple(TO_DataField, xproblem) 
	{ param1 = numtable; param2 = numrec; param3 = numfield; param4 = numtabledef; };
	inline sLONG GetTableNum() const { return param1; };
	inline sLONG GetRecordNum() const { return param2; };
	inline sLONG GetFieldNum() const { return param3; };
	inline sLONG GetTableDefNum() const { return param4; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class TableDefHeaderProblem : public ToolObjectSimple
{
public:
	inline TableDefHeaderProblem(ToolObjectProblem xproblem):ToolObjectSimple(TO_TableDefHeader, xproblem) { ; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class TableDefProblem : public ToolObjectSimple
{
public:
	inline TableDefProblem(ToolObjectProblem xproblem, sLONG numtabledef):ToolObjectSimple(TO_TableDef, xproblem) { param1 = numtabledef; };
	inline sLONG GetTableDefNum() const { return param1; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class RelationDefHeaderProblem : public ToolObjectSimple
{
public:
	inline RelationDefHeaderProblem(ToolObjectProblem xproblem):ToolObjectSimple(TO_RelationDefHeader, xproblem) { ; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class RelationDefProblem : public ToolObjectSimple
{
public:
	inline RelationDefProblem(ToolObjectProblem xproblem, sLONG numrelationdef):ToolObjectSimple(TO_TableDef, xproblem) { param1 = numrelationdef; };
	inline sLONG GetRelationDefNum() const { return param1; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class SeqNumHeaderProblem : public ToolObjectSimple
{
public:
	inline SeqNumHeaderProblem(ToolObjectProblem xproblem):ToolObjectSimple(TO_SeqNumHeader, xproblem) { ; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class SeqNumProblem : public ToolObjectSimple
{
public:
	inline SeqNumProblem(ToolObjectProblem xproblem, sLONG numseqnum):ToolObjectSimple(TO_SeqNum, xproblem) { param1 = numseqnum; };
	inline sLONG GetSeqNumNum() const { return param1; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class IndexDefHeaderProblem : public ToolObjectSimple
{
public:
	inline IndexDefHeaderProblem(ToolObjectProblem xproblem):ToolObjectSimple(TO_IndexDefHeader, xproblem) { ; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class IndexDefProblem : public ToolObjectSimple
{
public:
	inline IndexDefProblem(ToolObjectProblem xproblem, sLONG numindexdef):ToolObjectSimple(TO_IndexDef, xproblem) { param1 = numindexdef; };
	inline sLONG GetIndexDefNum() const { return param1; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class IndexDefInStructHeaderProblem : public ToolObjectSimple
{
public:
	inline IndexDefInStructHeaderProblem(ToolObjectProblem xproblem):ToolObjectSimple(TO_IndexDefInStructHeader, xproblem) { ; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class IndexDefInStructProblem : public ToolObjectSimple
{
public:
	inline IndexDefInStructProblem(ToolObjectProblem xproblem, sLONG numindexdef):ToolObjectSimple(TO_IndexDefInStructHeader, xproblem) { param1 = numindexdef; };
	inline sLONG GetIndexDefNum() const { return param1; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class DataTableHeaderProblem : public ToolObjectSimple
{
public:
	inline DataTableHeaderProblem(ToolObjectProblem xproblem):ToolObjectSimple(TO_DataTableHeader, xproblem) { ; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class FieldDefProblem : public ToolObjectSimple
{
public:
	inline FieldDefProblem(ToolObjectProblem xproblem, sLONG numtabledef, sLONG numfield):ToolObjectSimple(TO_FieldDef, xproblem) 
	{ param1 = numtabledef; param2 = numfield; };
	inline sLONG GetTableDefNum() const { return param1; };
	inline sLONG GetFieldNum() const { return param2; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class FicheOnDisk;

class ObjectAddrProblem : public ToolObjectSimple
{
public:
	inline ObjectAddrProblem(ToolObjectProblem xproblem, FicheOnDisk* ficD):ToolObjectSimple(TO_ObjectAddr, xproblem) 
	{ param5 = (DataAddr4D)ficD; };
	inline FicheOnDisk* GetObjectDesc() const { return (FicheOnDisk*)param5; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};

class GlobalOverlapProblem : public ToolObjectSimple
{
public:
	inline GlobalOverlapProblem(ToolObjectProblem xproblem, sLONG segnum):ToolObjectSimple(TO_GlobalOverlap, xproblem) 
	{ param1 = segnum; };
	inline sLONG GetSegNum() const { return param1; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};



class UUIDConflictProblem : public ToolObject
{
public:
	inline UUIDConflictProblem(ToolObjectProblem xproblem, TypeOfUUID type1, const VString& name1, TypeOfUUID type2, const VString& name2):ToolObject(TO_UUIDConflict, xproblem)
	{
		fType1 = type1;
		fType2 = type2;
		fName1 = name1;
		fName2 = name2;
	};

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;

protected:
	TypeOfUUID fType1, fType2;
	VString fName1, fName2;

};


class Index_PageAddrProblem : public ToolObjectSimple
{
public:
	inline Index_PageAddrProblem(ToolObjectProblem xproblem, sLONG numindex, sLONG numTabAddr):ToolObjectSimple(TO_Index_PageTabAddr, xproblem) 
	{ param1 = numindex; param2 = numTabAddr; };
	inline sLONG GetIndexNum() const { return param1; };
	inline sLONG GetTabAddrNum() const { return param2; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class IndexPageProblem : public ToolObjectSimple
{
public:
	inline IndexPageProblem(ToolObjectProblem xproblem, sLONG numindex, sLONG numpage):ToolObjectSimple(TO_Index_Page, xproblem) 
	{ param1 = numindex; param2 = numpage; };
	inline sLONG GetIndexNum() const { return param1; };
	inline sLONG GetPageNum() const { return param2; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class IndexProblem : public ToolObjectSimple
{
public:
	inline IndexProblem(ToolObjectProblem xproblem, sLONG numindex):ToolObjectSimple(TO_Index, xproblem) { param1 = numindex; };
	inline sLONG GetIndexNum() const { return param1; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class IndexClusterProblem : public ToolObjectSimple
{
public:
	inline IndexClusterProblem(ToolObjectProblem xproblem, sLONG numindex, sLONG numcluster):ToolObjectSimple(TO_Index_Cluster, xproblem) 
	{ param1 = numindex; param2 = numcluster; };
	inline sLONG GetIndexNum() const { return param1; };
	inline sLONG GetClusterNum() const { return param2; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};

 
class Index_ClusterAddrProblem : public ToolObjectSimple
{
public:
	inline Index_ClusterAddrProblem(ToolObjectProblem xproblem, sLONG numindex, sLONG numTabAddr):ToolObjectSimple(TO_Index_ClusterTabAddr, xproblem) 
	{ param1 = numindex; param2 = numTabAddr; };
	inline sLONG GetIndexNum() const { return param1; };
	inline sLONG GetTabAddrNum() const { return param2; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class IndexClusterPageProblem : public ToolObjectSimple
{
public:
	inline IndexClusterPageProblem(ToolObjectProblem xproblem, sLONG numindex, sLONG numcluster, sLONG numpage):ToolObjectSimple(TO_Index_ClusterPage, xproblem) 
	{ param1 = numindex; param2 = numcluster; param3 = numpage; };
	inline sLONG GetIndexNum() const { return param1; };
	inline sLONG GetClusterNum() const { return param2; };
	inline sLONG GetClusterPage() const { return param3; };

	virtual void GetText(VValueBag& outBag, Base4D_NotOpened* bd) const;
};


class Base4D_NotOpened;

class ToolLog : public VObject
{
	public:
		ToolLog(IDB4D_DataToolsIntf* inDataToolLog);
		virtual ~ToolLog()
		{
			if (fDiskOffsets != nil)
				delete fDiskOffsets;
		}

		VError InitAddressKeeper(Base4D_NotOpened* source, Boolean KeepTempBase, Boolean withbitsonly);

		void ReleaseAddressKeeper()
		{
			if (fDiskOffsets != nil)
				delete fDiskOffsets;
			fDiskOffsets = nil;
		}

		inline ObjectDiskPosInfoCollection* GetAddressKeeper()
		{
			return fDiskOffsets;
		}

		VError Add(const ToolObject& TOx);
		VError Progress(sLONG8 currentValue);
		VError OpenProgressSession(const VString& sessionName, sLONG8 maxValue);
		VError CloseProgressSession();

		inline void SetCurrentBase(Base4D_NotOpened* base) { fCurrentBase = base; };
		inline Base4D_NotOpened* GetCurrentBase() const { return fCurrentBase; };

		inline void SetBaseToBeCompacted(Base4D* inBase) { fToBeCompacted = inBase; };
		inline Base4D* GetTargetCompact() const { return fToBeCompacted; };
		inline Boolean IsCompacting() const { return fToBeCompacted != nil; };

		inline Boolean MustSkipOrphanTables() const { return fSkipOrphanDataTables; };
		inline void SkipOrphanTables(Boolean skip) { fSkipOrphanDataTables = skip; };

		inline void ClearLastProblem() { fAtLeastOneProblemSinceReset = false; };
		inline Boolean SomeProblem() const { return fAtLeastOneProblemSinceReset; };

		inline void SetDataTableBeCompacted(DataTable* inDataTable) { fCurrentDataFileToCompact = inDataTable; };
		inline DataTable* GetTargetDataTableCompact() const { return fCurrentDataFileToCompact; };

		inline void SetIndexToBeCompacted(IndexInfo* inIndex) { fIndexToBeCompacted = inIndex; };
		inline IndexInfo* GetTargetIndexCompact() const { return fIndexToBeCompacted; };

		void MarkAddr_SegHeader(DataAddr4D ou, sLONG len, sLONG segnum)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_SegHeader(ou, len, segnum);
		}

		void MarkAddr_PrimBittab(DataAddr4D ou, sLONG len, sLONG segnum)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_PrimBittab(ou, len, segnum);
		}

		void MarkAddr_SecondaryBittab(DataAddr4D ou, sLONG len, sLONG segnum, sLONG SecBitNum)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_SecondaryBittab(ou, len, segnum, SecBitNum);
		}

		void MarkAddr_Bittab(DataAddr4D ou, sLONG len, sLONG segnum, sLONG SecBitNum, sLONG BittabNum)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_Bittab(ou, len, segnum, SecBitNum, BittabNum);
		}

		void MarkAddr_SeqNum(DataAddr4D ou, sLONG len, sLONG seqnum_num)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_SeqNum(ou, len, seqnum_num);
		}

		void MarkAddr_DataTable(DataAddr4D ou, sLONG len, sLONG DataTable_num)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_DataTable(ou, len, DataTable_num);
		}

		void MarkAddr_StructElemDef(DataAddr4D ou, sLONG len, sLONG numobj, TypObjContainer xtypeobj)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_StructElemDef(ou, len, numobj, xtypeobj);
		}

		void MarkAddr_Blob(DataAddr4D ou, sLONG len, sLONG numtable, sLONG numblob)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_Blob(ou, len, numtable, numblob);
		}

		void MarkAddr_Record(DataAddr4D ou, sLONG len, sLONG numtable, sLONG numrec)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_Record(ou, len, numtable, numrec);
		}

		void MarkAddr_Index(DataAddr4D ou, sLONG len, sLONG numindex, sLONG numpage)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_Index(ou, len, numindex, numpage);
		}

		void MarkAddr_IndexCluster(DataAddr4D ou, sLONG len, sLONG numindex, sLONG numcluster)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_IndexCluster(ou, len, numindex, numcluster);
		}

		void MarkAddr_IndexCluster_part(DataAddr4D ou, sLONG len, sLONG numindex, sLONG numcluster, sLONG numpart)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_IndexCluster_part(ou, len, numindex, numcluster, numpart);
		}

		void MarkAddr_TabAddr_Index(DataAddr4D ou, sLONG len, sLONG numindex, sLONG pos, sLONG posparent)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_TabAddr_Index(ou, len, numindex, pos, posparent);
		}

		void MarkAddr_TabAddr_IndexCluster(DataAddr4D ou, sLONG len, sLONG numindex, sLONG pos, sLONG posparent)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_TabAddr_IndexCluster(ou, len, numindex, pos, posparent);
		}

		void MarkAddr_TabAddr_Record(DataAddr4D ou, sLONG len, sLONG numtable, sLONG pos, sLONG posparent)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_TabAddr_Record(ou, len, numtable, pos, posparent);
		}

		void MarkAddr_TabAddr_Blob(DataAddr4D ou, sLONG len, sLONG numtable, sLONG pos, sLONG posparent)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_TabAddr_Blob(ou, len, numtable, pos, posparent);
		}

		void MarkAddr_TabAddr_Other(DataAddr4D ou, sLONG len, sLONG pos, sLONG posparent)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_TabAddr_Other(ou, len, pos, posparent);
		}

		void MarkAddr_ExtraProp(DataAddr4D ou, sLONG len)
		{
			if (fDiskOffsets != nil)
				fDiskOffsets->MarkAddr_ExtraProp(ou, len);
		}

		void GetVerifyOrCompactString(uLONG inStringID, XBOX::VString &outLabel) const
		{
			gs(IsCompacting() ? 1008 : 1004, inStringID, outLabel);
		}


	protected:
		VLocalizationManager* fLocal;
		sLONG fLastDisplayTime;
		IDB4D_DataToolsIntf* fDataTools;
		vector<sLONG8> fSessions;
		sLONG fCurrentSession;
		Base4D_NotOpened* fCurrentBase;
		Base4D* fToBeCompacted;
		DataTable* fCurrentDataFileToCompact;
		IndexInfo* fIndexToBeCompacted;
		ObjectDiskPosInfoCollection* fDiskOffsets;
		Boolean fSkipOrphanDataTables;
		Boolean fAtLeastOneProblemSinceReset;
		

};

void Get_TypeOfUUID_Name(TypeOfUUID inType, VString& outName);

#endif