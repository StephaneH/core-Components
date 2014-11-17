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
#ifndef __INDCLUST__
#define __INDCLUST__


#include "index4D.h"

#if FORMAC
#pragma segment IndClust
#endif



//const sLONG MaxSizeClusterLong = 1023;
//const sLONG MaxSizeClusterLongMin = 512;


class ColumnFormulas;


class ClusterSelTreeMemHeader : public TreeMemHeader
{
	public:
		virtual ~ClusterSelTreeMemHeader()
		{
			OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
			PreDestructor(curstack);
		}

		virtual void PreDestructor(OccupableStack* curstack)
		{
			LibereEspaceMem(curstack);
		}

		virtual void KillElem(void* inObject);

		virtual bool IsElemPurged(void* InObject)
		{
			return false;
		}

		virtual bool TryToFreeElem(sLONG allocationBlockNumber, void* InObject, VSize& outFreed);

		virtual void RetainElem(void* inObject);

		virtual void OccupyElem(void* inObject, OccupableStack* curstack);

};


class ClusterSel : public ObjInCacheMem
{
	public:
		ClusterSel(IndexInfo* ind);
		void Init(Base4D *xdb, IObjToFlush *TheOwner, Table* WhatTable);
	#if autocheckobj
		virtual uBOOL CheckObjInMem(void);
	#endif
		sLONG AddToCluster(OccupableStack* curstack, sLONG nclust, sLONG n, BaseTaskInfo* context, VError& err);
		sLONG DelFromCluster(OccupableStack* curstack, sLONG nclust, sLONG n, BaseTaskInfo* context, VError& err);
		Selection* LoadSel(OccupableStack* curstack, sLONG vrainclust, BaseTaskInfo* context, VError& err);
		inline ClusterDISK* getIHCLUST(void) { return(&IHCLUST); };
		void AddSelToBittab(OccupableStack* curstack, sLONG nclust, Bittab *b, BaseTaskInfo* context, VError& err);
		VError AddSelToSel(OccupableStack* curstack, sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel, sLONG n, BaseTaskInfo* context, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil);
		Boolean IntersectSel(OccupableStack* curstack, sLONG nclust, Bittab *b, BaseTaskInfo* context, VError& err);
		sLONG GetNthRecNum(OccupableStack* curstack, sLONG nclust, sLONG Nth, BaseTaskInfo* context, VError& err);
		sLONG GetNextRecNum(OccupableStack* curstack, sLONG nclust, sLONG Nth, sLONG recnum, BaseTaskInfo* context, VError& err);

		VError CalculateColumnFormulasOnCluster(OccupableStack* curstack, ColumnFormulas* formules, BTitemIndex* u, sLONG nclust, 
												Bittab* filtre, BaseTaskInfo* context, Boolean& stop, BTreePageIndex* page);

		/*
		static VError Reloader(AddrTableHeader* obj, ObjAlmostInCache* Owner);
		static VError ReloaderForKill(ObjAlmostInCache* Owner);
		
		void SaveIHCLUSTForTrans(BaseTaskInfo* context);
		*/

		void TryToFreeMem(sLONG allocationBlockNumber, VSize combien, VSize& tot);

		VError ThrowError( VError inErrCode, ActionDB4D inAction);

		VError LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress = nil);
		void LibereEspaceMem(OccupableStack* curstack);

		VError AddSel(OccupableStack* curstack, Selection* sel, BaseTaskInfo* context);

		sLONG8 GetNBDiskAccessForAScan() { return IHCLUST.nbclust / 2; };

		VError InitTableAddr(OccupableStack* curstack);
		VError NormalizeTableAddr(OccupableStack* curstack);

		VError SetClusterAddr(OccupableStack* curstack, sLONG numclust, DataAddr4D SelAddr, sLONG SelLen)
		{
			return SelClusterAddr.PutxAddr(numclust, SelAddr, SelLen, nil, curstack);
		}

		Selection* GetSel(OccupableStack* curstack, sLONG nclust, BaseTaskInfo* context, VError& err);

		inline Boolean MustConvertPetiteSelToBitSel(Selection* sel)
		{
			return (sel->GetTypSel() == sel_petitesel) && ((sel->GetQTfic()+10) > (fWhatTable->GetMaxRecords(nil)/32));
		}

		inline Boolean MustConvertBitSelToPetiteSel(Selection* sel)
		{
			return (sel->GetTypSel() == sel_bitsel) && ((sel->GetQTfic()-10) < (fWhatTable->GetMaxRecords(nil)/32));
		}

		void SetDataTable(DataTable* df)
		{
			fWhatTable = df;
		}

	protected:
		Base4D *bd;
		ClusterDISK IHCLUST;
		ClusterSelTreeMemHeader SelClusterInMem;
		AddressTableHeader SelClusterAddr;
		DataTable* fWhatTable;
		VCriticalSection fLoadMutex;
		IndexInfo* fInd;
		//ClusterDISK IHCLUST_SavedByTrans;
};


									/* -----------------------------------------------  */


class BTreePageIndexCluster : public BTreePageIndex
{
	public:
		inline BTreePageIndexCluster(IndexInfo* xentete, sLONG datasize, sLONG xnum) : BTreePageIndex(xentete, datasize, xnum) { IsCluster=true; };

};


class IndexHeaderBTreeCluster : public IndexHeaderBTree
{
	public:
		IndexHeaderBTreeCluster(IndexInfo *xentete);
		virtual void SetAssoc(IndexInfo* x);
		/*
		sLONG AddToClusterSel(sLONG nclust, sLONG n, BaseTaskInfo* context, VError& err);
		sLONG DelFromClusterSel(sLONG nclust, sLONG n, BaseTaskInfo* context, VError& err);
		void AddSelToBittab(sLONG nclust, Bittab *b, BaseTaskInfo* context, VError& err);
		VError AddSelToSel(sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel, sLONG n, BaseTaskInfo* context);
		Boolean IntersectSel(sLONG nclust, Bittab *b, BaseTaskInfo* context, VError& err);
		sLONG GetNthRecNum(sLONG nclust, sLONG Nth, BaseTaskInfo* context, VError& err);
		sLONG GetNextRecNum(sLONG nclust, sLONG Nth, sLONG recnum, BaseTaskInfo* context, VError& err);

		VError CalculateColumnFormulasOnCluster(ColumnFormulas* formules, BTitemIndex* u, sLONG nclust, 
												Bittab* filtre, BaseTaskInfo* context, Boolean& stop, BTreePageIndex* page);
		
		VError AddSel(Selection* sel, BaseTaskInfo* context);
		
		Selection* GetSel(sLONG nclust, BaseTaskInfo* context, VError& err)
		{
			return cls.GetSel(nclust, context, err);
		}
		*/
		
		virtual sLONG GetLen(void);
		virtual VError PutInto(VStream& buf);
		virtual VError GetFrom(VStream& buf);
		virtual BTreePageIndex* CrePage(void);
		virtual BTreePageIndex* LoadPage(OccupableStack* curstack, DataAddr4D addr, sLONG len, BaseTaskInfo* context, VError& err, sLONG xnum);
		
		virtual ClusterSel* GetClusterSel(OccupableStack* curstack) { return (&cls) ;};
		virtual const ClusterSel* GetClusterSel(OccupableStack* curstack) const { return (&cls) ;};
		virtual VError LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress = nil);
		virtual void LibereEspaceMem(OccupableStack* curstack);

		virtual void TryToFreeMem(sLONG allocationBlockNumber, VSize combien, VSize& tot);

		virtual sLONG8 GetNBDiskAccessForAScan(Boolean WithSelFilter) { return IHD.nbpage + (WithSelFilter ? cls.GetNBDiskAccessForAScan() : 0); };

		virtual VError InitTablesAddr(OccupableStack* curstack);
		virtual VError NormalizeTablesAddr(OccupableStack* curstack);

		virtual VError SetClusterAddr(OccupableStack* curstack, sLONG numclust, DataAddr4D SelAddr, sLONG SelLen)
		{
			if (numclust == kSpecialClustNulls)
				return IndexHeader::SetClusterAddr(curstack, numclust, SelAddr, SelLen);
			else
				return cls.SetClusterAddr(curstack, numclust, SelAddr, SelLen);
		}

		virtual void Update(Table* inTable);


	protected:
		ClusterSel cls;
	
};


IndexHeader* CreIndexHeaderBTreeCluster(IndexInfo *xentete);
									
									
									

									/* -----------------------------------------------  */


class IndexHeaderBTreeObjVals : public IndexHeaderBTreeCluster
{
	public:
		IndexHeaderBTreeObjVals(IndexInfo *xentete) :IndexHeaderBTreeCluster(xentete)
		{
			fNextPathID = 0;
		}

		virtual sLONG GetLen(void);
		virtual VError PutInto(VStream& buf);
		virtual VError GetFrom(VStream& buf);

		sLONG GetNextPathID()
		{
			++fNextPathID;
			return fNextPathID;
		}

protected:
		sLONG fNextPathID;
};


IndexHeader* CreIndexHeaderBTreeObjVals(IndexInfo *xentete);


									/* -----------------------------------------------  */

#if 0
class BTreePageIndexFixSizeCluster : public BTreePageFixSizeIndex
{
	public:
		inline BTreePageIndexFixSizeCluster(IndexInfo* xentete, sLONG inMaxSize) : BTreePageFixSizeIndex(xentete, inMaxSize) { IsCluster=true; };

};


class IndexHeaderBTreeFixSizeCluster : public IndexHeaderBTreeFixSize
{
	public:
		IndexHeaderBTreeFixSizeCluster(IndexInfo *xentete);
		sLONG AddToClusterSel(sLONG nclust, sLONG n, BaseTaskInfo* context, VError& err);
		sLONG DelFromClusterSel(sLONG nclust, sLONG n, BaseTaskInfo* context, VError& err);
		void AddSelToBittab(sLONG nclust, Bittab *b, BaseTaskInfo* context);
		VError AddSelToSel(sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel, sLONG n, BaseTaskInfo* context);

		virtual sLONG GetLen(void);
		virtual VError PutInto(VStream& buf);
		virtual VError GetFrom(VStream& buf);
		virtual BTreePageFixSizeIndex* CrePage(void);
		
		inline ClusterSel* GetClusterSel(void) { return (&cls) ;}; // J.A. Tools

	protected:
		ClusterSel cls;
	
};


IndexHeader* CreIndexHeaderBTreeFixSizeCluster(IndexInfo *xentete);

#endif
									
									
#endif




									
