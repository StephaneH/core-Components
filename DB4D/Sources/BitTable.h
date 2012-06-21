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
#ifndef __BITTABLE__
#define __BITTABLE__

const sLONG kNbLongParSet = 1024;
const sLONG kNbBitParSet = kNbLongParSet*32;
const sLONG kSizeSetDisk = kNbLongParSet*4;
const sLONG kSizeSetMem = (kNbLongParSet+1)*4;
const sLONG kRatioSet = 10+5; // 1024*32 
const uBYTE kBitPageNonVide	= 7;
const uBYTE kBitPageAllFalse = 0;
const uBYTE kBitPageAllTrue	= 1;

/* typedef uLONG bitset[kNbLongParSet]; */
typedef uLONG* bitsetptr;

typedef bitsetptr **tabbitsethandle;

class Selection;
class DataBaseObjectHeader;
class LongSel;

class Bittab : public ObjInCacheMemory, public IRefCountable
{
	friend class LongSel;
	
	public:
		Bittab(uBOOL isATransPart = false);
		virtual ~Bittab();
#if autocheckobj
		virtual uBOOL CheckObjInMem(void);
#endif
		VError ThrowError( VError inErrCode, ActionDB4D inAction) const;
		void vide(void);
		VError Reduit(sLONG nbrec);
		VError aggrandit(sLONG nbbits);
		VError aggranditParBlock(sLONG nbbits);
		uBOOL isOn(sLONG n);
		inline VError Set(sLONG n, uBOOL CanGrow = true)  { return(ClearOrSet(n,true, CanGrow)); }; // pour des raisons de vitesse penser ˆ locker et occuper
		inline VError Clear(sLONG n, uBOOL CanGrow = false) { return(ClearOrSet(n,false, CanGrow)); }; // la Bittab avant l'utilisation des set et clear
		VError ClearOrSet(sLONG n, uBOOL b, uBOOL CanGrow = false);
		void ClearOrSetAll(uBOOL b);
		inline void rau(void) { ClearOrSetAll(true); };
		inline void raz(void) { ClearOrSetAll(false); };
		sLONG FindNextBit(sLONG start);  // penser aussi a faire le use/unuse quand on utilise cette commande
		sLONG FindPreviousBit(sLONG start);  // penser aussi a faire le use/unuse quand on utilise cette commande
		void CalcEquiSel();
		sLONG PlaceDuNiemeBitAllume(sLONG place);
		sLONG CountHowManyBitAllumeAt(sLONG pos);
		sLONG Compte(void);
		Boolean IsEmpty();
		inline sLONG NbTotalBits(void) { return(nbbit); };
		VError And(Bittab *autre, Boolean cangrow = true);
		VError Or(Bittab *autre, Boolean cangrow = true);
		//VError XOr(Bittab *autre, Boolean cangrow = true);
		VError moins(Bittab *autre, Boolean cangrow = true);
		VError Invert(void);
		Boolean Intersect(Bittab *autre, VError& err);
		Boolean Intersect(Boolean autre, sLONG pagenum, VError& err);
		Boolean Intersect(bitsetptr autre, sLONG pagenum, VError& err, vector<sLONG>& outCollisions, sLONG maxCollisions);
		inline sLONG GetMaxBit(void) { return(nbbit); };
		
		inline void use(void) { occupe(); };
		inline void unuse(void) { libere(); };

		sLONG PourLibereMem(sLONG combien, uBOOL tout);
		uBOOL PourOkdel(void);
		sLONG PourSaveobj(DataAddr4D xaddr);
		void SetAddr(DataAddr4D addr, Base4D *xdb);
		bitsetptr loadmem(sLONG n, VError& err);
		bitsetptr loadmemAndBuildIfEmpty(sLONG n, VError& err);
		sLONG CalcLenOnDisk(void);
	//	VError SaveBits(BaseTaskInfo* context);
		VError LoadBitSel(DataBaseObjectHeader& tag);
		inline sLONG GetLenDisk(void) { return(antelen); };
		//sLONG DelOnDisk(void);
		//inline uBOOL FreeFromChain(void) { return(freefromchain); };
		//inline void SetFreeFromChain(uBOOL free) { freefromchain = free; };
		void Epure(void);
		virtual VError LibereEspaceDisk(VDB4DProgressIndicator* InProgress);
		VError FillArray(sLONG* tab, sLONG maxelem);
		VError FillFromArray(const char* tab, sLONG sizeElem, sLONG nbelem, sLONG maxrecords, bool useIndirection);
		VError AddBittabToSel(sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel);
		VError FillWithSel(Selection* sel, BaseTaskInfo* context);
		VError AddSel(Selection* sel, BaseTaskInfo* context);
		void ClearFrom(sLONG n);

		VError GetFrom(VStream& buf);
		VError PutInto(VStream& buf);
		
		Bittab* GenerateSetFromRecordID(sLONG inRecordID, sLONG inRecordIndex2, VError& err);
		Bittab* GenerateSetFromRange(sLONG inRecordIndex1, sLONG inRecordIndex2, VError& err);

		VError FillArrayOfBits(void* outArray, sLONG inMaxElements);
		VError FillFromArrayOfBits(const void* inArray, sLONG inNbElements);
		VError RemoveSelectedRange(sLONG inRecordIndex1, sLONG inRecordIndex2);

		VError TransformIntoCluster(DataAddr4D addr, Base4D *xdb);

		void cut();

		Bittab* Clone(VError& err) const;

		VError SetRange(uBOOL b, sLONG pagenum);
		VError SetRange(bitsetptr autre, sLONG pagenum);

		VError PutIntoLongSel(LongSel* sel);

		VUUIDBuffer* GenerateID(Boolean forcenew = true) 
		{ 
			if (forcenew || !fIDHasBeenGenerated)
			{
				VUUID xid(true);
				xid.ToBuffer(fID);
				fIDHasBeenGenerated = true;
			}
			return &fID; 
		};

		VUUIDBuffer* GetID() 
		{ 
			return &fID; 
		};

		VError FromServer(VStream* from, CDB4DBaseContext* inContext);
		VError ToClient(VStream* into, CDB4DBaseContext* inContext);
		VError FromClient(VStream* from, CDB4DBaseContext* inContext);
		VError ToServer(VStream* into, CDB4DBaseContext* inContext, bool inKeepOnServer);
		VError GetPartReply(sLONG numpart, IRequestReply *inRequest);

		/*
		inline void SetRemoteContext(CDB4DBaseContext* inContext)
		{
			fRemoteContext = inContext;
			if (fRemoteContext != nil)
				fRemoteContext->Retain();
		}
		*/

		inline void SetAsSel()
		{
			fIsASel = true;
		}

		inline void FreeFromSel()
		{
			fIsASel = false;
		}



		inline Base4D* GetOwner() const
		{
			return db;
		}

		inline void SetOwner(Base4D* inDB)
		{
			db = inDB;
		}

		void SetCompte(sLONG count)
		{
			decompte = count;
		}

		void ClearRemoteCache(VStream* fromServer);
		void PutClearCacheInfo(VStream* toClient);

		/*
		CDB4DBaseContext* GetRemoteContext() const
		{
			return fRemoteContext;
		}
		*/

		inline Boolean IsRemote() const
		{
			return fIsRemote;
		}

		inline void PourSetModif()
		{
			nbpageToWrite = nbpage;
		}

		bool MatchAllocationNumber(sLONG allocationNumber, void* owner);


	private:
		VUUIDBuffer fID;
		//CDB4DBaseContext* fRemoteContext;
		sLONG nbbit;
		sLONG nbpage;
		sLONG nbpageToWrite;
		DataAddr4D SelAddr;
		Base4D *db;
		bitsetptr *tabmem;
		uBOOL *tabplein;
		DataAddr4D *TabPage;
		sLONG equisel;
		sLONG decompte;
		sLONG antelen;		
		sLONG fNeedPurgeCount;
		uBOOL SelisCluster/*,freefromchain*/,fIsRemote, fIDHasBeenGenerated, fIsASel, fAutoPurge;
		mutable uBOOL isempty;
};


template <sLONG BitBuffSize, sLONG maxbits>
class SmallBittab
{
	public:
		SmallBittab()
		{
			pBits = &fLocalBuf[0];
			fBufSize = BitBuffSize/4;
			std::fill(&fLocalBuf[0], &fLocalBuf[BitBuffSize/4], 0);
		}

		inline void ReleaseBuf()
		{
			if (pBits != nil && pBits != &fLocalBuf[0])
				VObject::GetMainMemMgr()->Free(pBits);
			pBits = nil;
		}

		~SmallBittab()
		{
			ReleaseBuf();
		}

		void ClearAll()
		{
			std::fill(&pBits[0], &pBits[fBufSize], 0);
		}

		void SetAll()
		{
			std::fill(&pBits[0], &pBits[fBufSize], 0xFFFFFFFF);
		}

		Boolean Set(sLONG n)
		{
			Boolean ok = true;
			if (n >= 0 && n <= maxbits)
			{
				if ((n/32) >= fBufSize)
				{
					sLONG newbufsize = ((n/32)+63) & (-64);
					uLONG* newbuf = (uLONG*)VObject::GetMainMemMgr()->Malloc(newbufsize*4+4, false, 'sbit');
					if (newbuf == nil)
						ok = false;
					else
					{
						std::fill(&newbuf[fBufSize], &newbuf[newbufsize], 0);
						std::copy(&pBits[0], &pBits[fBufSize], &newbuf[0]);
						ReleaseBuf();
						pBits = newbuf;
						fBufSize = newbufsize;
					}
				}
				if (ok)
				{
					sLONG nL=n / 32;
					sLONG n1=n & 31;

					*(pBits+nL) = *(pBits+nL) | (1<<n1);
				}
			}
			else
				ok = false;

			return ok;
		}

		void Clear(sLONG n)
		{
			if (n >= 0 && n <= maxbits)
			{
				if ((n/32) < fBufSize)
				{
					sLONG nL=n / 32;
					sLONG n1=n & 31;
					*(pBits+nL) = *(pBits+nL) & (ALLFF ^ (1<<n1));
				}
			}
		}

		Boolean isOn(sLONG n)
		{
			Boolean res = false;
			if (n >= 0 && n <= maxbits)
			{
				if ((n/32) < fBufSize)
				{
					sLONG nL=n / 32;
					sLONG n1=n & 31;
					if ( ((uLONG)(1<<n1)) & (*(pBits+nL)) )
						res = true;
				}
			}
			return res;
		}

		Boolean CopyFrom(const SmallBittab<BitBuffSize, maxbits>& other)
		{
			Boolean ok = true;
			fBufSize = other.fBufSize;
			if (other.pBits == &other.fLocalBuf[0])
				pBits = &fLocalBuf[0];
			else
			{
				pBits = (uLONG*)VObject::GetMainMemMgr()->Malloc(fBufSize*4+4, false, 'sbit');
				if (pBits == nil)
				{
					ok = false;
					pBits = &fLocalBuf[0];
					fBufSize = BitBuffSize/4;
				}
			}
			if (ok)
				std::copy(&other.pBits[0], &other.pBits[fBufSize], &pBits[0]);
			return ok;
		}

		VError WriteToStream(VStream* ToStream)
		{
			VError err = ToStream->PutLong(fBufSize);
			if (err == VE_OK)
			{
				err = ToStream->PutLongs(pBits, fBufSize);
			}
			return err;
		}

		VError ReadFromStream(VStream* FromStream)
		{
			sLONG nb;
			VError err = FromStream->GetLong(nb);
			if (err == VE_OK)
			{
				ReleaseBuf();
				if (nb > (BitBuffSize/4))
				{
					fBufSize = nb;
					pBits = (uLONG*)VObject::GetMainMemMgr()->Malloc(fBufSize*4+4, false, 'sbit');
					if (pBits == nil)
					{
						err = ThrowBaseError(memfull, noaction);
						pBits = &fLocalBuf[0];
						fBufSize = BitBuffSize/4;
					}
				}
				else
				{
					fBufSize = BitBuffSize/4;
					pBits = &fLocalBuf[0];
					std::fill(&fLocalBuf[0], &fLocalBuf[BitBuffSize/4], 0);
				}
				if (err == VE_OK)
				{
					err = FromStream->GetLongs(pBits, &nb);
				}
			}
			return err;
		}


	protected:
		uLONG fLocalBuf[BitBuffSize/4];
		sLONG fBufSize; // size of buffer in longs
		uLONG* pBits;
};



typedef SmallBittab<64, 65536> SmallBittabForTables;


#endif
