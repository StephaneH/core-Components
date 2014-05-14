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
#ifndef __BLOB4D__
#define __BLOB4D__


enum { pasblob=0L, blobtext, blobimage, blobptr, blobtextUTF8 };




class Text4D : public VString
{
	public:
		inline Text4D(uBOOL CanDisposeOf=true) { disposable=CanDisposeOf; };
		inline uBOOL CanDelete(void) { return(disposable); };
		inline void SetDisposable(uBOOL b) { disposable=b; };
		inline void Dispose(void) { if (disposable) delete this; };
	protected:
		uBOOL disposable;
};

class BlobTempHeader;
class Transaction;

const sLONG kBlobTypeText = 1;
const sLONG kBlobTypeWithPtr = 2;
const sLONG kBlobTypeTextUTF8 = 3;

class PrimKey;

class Blob4D : public ObjInCacheMem, public IObjToFlush
{
	public:
			Blob4D( DataTable *inDataFile);
			// L.E. 28/12/00 now uses IRefCountable::Retain/Release instead of use/unuse

	#if debugOverlaps_strong
			virtual void setaddr(DataAddr4D addr, bool checknew = true)
			{
				debug_CheckBlobAddrInTrans(RecordIDInTrans(fDataFile->GetNum(), fBlobID), addr);
				fDataFile->Debug_CheckBlobAddrMatching(addr, fBlobID);
				ObjAlmostInCache::setaddr(addr, checknew);
			}

			virtual void ChangeAddr(DataAddr4D addr, Base4D* bd, BaseTaskInfo* context)
			{
				debug_CheckBlobAddrInTrans(RecordIDInTrans(fDataFile->GetNum(), fBlobID), addr);
				fDataFile->Debug_CheckBlobAddrMatching(addr, fBlobID);
				ObjAlmostInCache::ChangeAddr(addr, bd, context);
			}
	#endif

			virtual sLONG GetBlobType() = 0;

			sLONG GetAntelen() const { return fAnteLength;}
			
			void SetAntelen(sLONG inNbBytes) { fAnteLength = inNbBytes;}
			
			sLONG GetNum() const { return fBlobID; }
			void SetNum(sLONG inBlobID) { fBlobID = inBlobID; }

			void SetHintRecNum(sLONG TableNum, sLONG FieldNum, sLONG inHint)
			{
				fHintRecNum = inHint;
				fHintFieldNum = FieldNum;
				fHintTableNum = TableNum;
			}
			
			DataTable *GetDF() { return fDataFile; }
			const DataTable *GetDF() const { return fDataFile; }
			
			virtual Blob4D* loadobj(DataAddr4D xaddr, VError& err) = 0;
			
			
			virtual sLONG calclen() const = 0;
			
			virtual VError CopyDataFrom(const Blob4D *from)=0;
			virtual uBOOL IsEmpty()=0;

			virtual bool IsNull()
			{
				bool result = false;
				if (IsEmpty())
				{
					if (fInOutsidePath)
					{
						_ComputePath();
						if (fComputedPath.IsEmpty())
							result = true;
						else
						{
							VFile ff(fDataFile->GetDB()->GetFileSystemNamespace(), fComputedPath);
							result = !ff.Exists();
						}
					}
				}
				return result;
			}
			
			virtual VError GetFrom(const void* p, sLONG len, sLONG Offset = 0) = 0;
			virtual VError PutInto(void* p, sLONG len = -1, sLONG Offset = 0, sLONG* ActualLen = nil) const = 0;
			virtual VError GetFrom(VStream& buf) = 0;
			virtual VError PutInto(VStream& buf) const = 0;

			virtual void Kill();
			
			virtual VError ThrowError( VError inErrCode, ActionDB4D inAction) const;

			inline void SetOKToDelete(Boolean state) { fOKdel = state; };

			inline Boolean NeedDuplicate() const { return modifie() || InTrans() || GetRefCount() > 1; };

			virtual Blob4D* Clone(Boolean WithData) const = 0;
			void CopyBaseInto(Blob4D* other) const;
			void Detach();

			VError WriteSaveToLog(BaseTaskInfo* context, Boolean newone);
			VError WriteDeleteToLog(BaseTaskInfo* context);

			inline void SetNotExisting()
			{
				if (!modifie())
				{
					fBlobID = -1;
					fAnteLength = 0;
					//setaddr(0, false);
					ResetAddr();
					fAnteAddress = 0;
				}
			}

			void PutHeaderInto(BlobTempHeader& into);

			inline void SetAnteFromTrans(DataAddr4D anteaddr, sLONG antelen)
			{
				fAnteAddress = anteaddr;
				fAnteLength = antelen;
			};

			inline void SetReservedBlobNumber(sLONG inBlobNum)
			{
				fReservedBlobID = inBlobNum;
			}

			inline sLONG GetReservedBlobNumber() const
			{
				return fReservedBlobID;
			}

			virtual void RestoreFromPop(void* context);

			inline void SetNewInTrans(uBOOL b)
			{
				fNewInTrans = b;
			}

			inline uBOOL IsNewInTrans() const
			{
				return fNewInTrans;
			}


			inline void SetInTrans(uBOOL b = true)
			{
				fInTrans = b;
			}

			inline uBOOL InTrans() const
			{
				return fInTrans;
			}

			inline void* AllocateMem(VSize inNbBytes, sLONG Tag)
			{
				void* result = GetFastMem(inNbBytes, false, Tag);
				if (result == nil)
				{
					result = VObject::GetMainMemMgr()->NewPtr(inNbBytes, false, Tag);
					fInMainMem = true;
				}
				else
					fInMainMem = false;
				return result;
			}

			inline void ReleaseMem(void* inPtr)
			{
				if (inPtr != nil)
				{
					if (fInMainMem)
					{
						VObject::GetMainMemMgr()->DisposePtr(inPtr);
					}
					else
					{
						FreeFastMem(inPtr);
					}
				}
			}

			inline void* SetPtrSize(void* inPtr, VSize inNbBytes, VSize oldSize)
			{
				VSize nb = oldSize;
				if (inNbBytes < nb)
					nb = inNbBytes;

				uBOOL foldInMainMem = fInMainMem;
				void* result = AllocateMem(inNbBytes, 'bbll');
				if (result != nil)
				{
					vBlockMove(inPtr, result, nb);
					uBOOL fnewInMainMem = fInMainMem;
					fInMainMem = foldInMainMem;
					ReleaseMem(inPtr);
					fInMainMem = fnewInMainMem;
				}
				return result;
			}

			VError _CreatePathIfEmpty();

			VError LoadPathFrom(void* from);
			VError SavePathTo(void* into);
			VSize GetPathLength();
			inline bool IsOutsidePath() const
			{
				return fInOutsidePath;
			}

			virtual VError SetOutsidePath(const VString& path, bool isRelative);

			void SetOutsideSuffixe(const VString& suffixe)
			{
				if (!fComputedPath.IsEmpty() && !fOldPathIsValid)
				{
					fOldOutsidePath = fComputedPath;
					fOldPathIsValid = true;
				}
				fOutsideSuffixe = suffixe;
				fComputedPath.Clear();
			}

			inline const VString& GetOutsideSuffixe() const
			{
				return fOutsideSuffixe;
			}

			const VString& GetComputedPath(bool buildPathIfEmpty = true)
			{
				if (buildPathIfEmpty)
					_CreatePathIfEmpty();
				_ComputePath();
				return fComputedPath;
			}

			inline const VString& GetOutsidePath() const
			{
				return fOutsidePath;
			}

			inline VString GetTrueOutsidePath() const
			{
				return fOutsidePath+fOutsideSuffixe;
			}


			bool SomethingToDelete();
			inline bool IsPathRelative()
			{
				return fPathIsRelative;
			}

			void _ComputePath();

			//VError DeleteOutsidePath();
			void MarkDeleteOutsidePath();
			void UnMarkDeleteOutsidePath();

			virtual VError LoadDataFromPath() = 0;

			virtual VSize GetDataLength() = 0;

			virtual VError ReloadFromOutsidePath()
			{
				return LoadDataFromPath();
			}

			void ExtractSuffixe(VString& ioPath, VString& outSuffixe);

			void GetOutsideID(VString& outID)
			{
				outID = fOutsidePath;
				outID += fOutsideSuffixe;
			}

			void GetOldOutsideID(VString& outID)
			{
				if (fOldPathIsValid)
					outID = fOldOutsidePath;
				else
					outID.Clear();
			}

			virtual bool MatchAllocationNumber(sLONG allocationNumber) = 0;

			virtual VError CheckIfMustDeleteOldBlobID(BaseTaskInfo* context, Transaction* trans);
			virtual VError CheckIfMustDeleteOldPath(BaseTaskInfo* context, Transaction* trans);

			void AssociateRecord(PrimKey* primkey, sLONG FieldNum);

			PrimKey* GetAssociatedPrimKey() const
			{
				return fPrimKey;
			}

			sLONG GetAssociatedFieldNumber() const
			{
				return fFieldNum;
			}


			
	protected:

			virtual ~Blob4D();
			VError xThrowError( VError inErrCode, ActionDB4D inAction, ValueKind inBlobType) const;
		
	protected:
		DataTable	*fDataFile;	//DF
		sLONG		fBlobID;	// num
		sLONG		fOldBlobID;
		sLONG		fReservedBlobID;
		DataAddr4D fAnteAddress;// anteaddr;
		sLONG 	fAnteLength;	// antelen
		sLONG	fHintRecNum;
		sLONG	fHintTableNum;
		sLONG	fHintFieldNum;
		Boolean fOKdel;
		uBOOL fNewInTrans, fInTrans, fInMainMem, fInOutsidePath, fOldPathIsValid, fPathIsRelative, fOldWasInData;
		VString fOutsidePath;
		VString fOutsideSuffixe;
		VString fOldOutsidePath;
		VString fComputedPath;
		PrimKey* fPrimKey;
		sLONG fFieldNum;
};


class BlobText : public Blob4D
{
	public:
			inline BlobText(DataTable *xDF) : Blob4D(xDF),fNbBytes(0),fString(nil) 
			{
				fCharsCanBeSwapped = true;
				fOutsideSuffixe = L".txt";
			}

			virtual sLONG GetBlobType() { return kBlobTypeText; };

			virtual bool SaveObj(VSize& outSizeSaved);
			
			virtual sLONG calclen(void) const;
			virtual Blob4D* loadobj(DataAddr4D xaddr, VError& err);

			virtual VError CopyDataFrom(const Blob4D* from);
			virtual uBOOL IsEmpty(void);

			virtual VError GetFrom(const void* p, sLONG len, sLONG Offset = 0);
			virtual VError PutInto(void* p, sLONG len = -1, sLONG Offset = 0, sLONG* ActualLen = nil) const;
			virtual VError GetFrom(VStream& buf);
			virtual VError PutInto(VStream& buf) const;
			
			void GetCString( UniPtr *outCString, sLONG *outLength);
	//		void	SetCString( UniPtr inCString, sLONG inLength);

			virtual VError ThrowError( VError inErrCode, ActionDB4D inAction) const;

			virtual Blob4D* Clone(Boolean WithData) const;

			inline UniPtr GetCPointer() { return fString; };
			inline const UniChar* GetCPointer() const { return fString; };

			virtual VError LoadDataFromPath();

			virtual VSize GetDataLength()
			{
				return (VSize)fNbBytes;
			}

			virtual bool MatchAllocationNumber(sLONG allocationNumber);

			virtual VError SetOutsidePath(const VString& path, bool isRelative);


	protected:
			virtual ~BlobText();
			UniPtr fString;
			sLONG fNbBytes;
			bool fCharsCanBeSwapped;
};


class BlobTextUTF8 : public BlobText
{
	public:
		inline BlobTextUTF8(DataTable *xDF) : BlobText(xDF) 
		{
			fCharsCanBeSwapped = false;
		}
		virtual sLONG GetBlobType() { return kBlobTypeTextUTF8; };

		virtual VError GetFrom(const void* p, sLONG len, sLONG Offset = 0);
		virtual VError PutInto(void* p, sLONG len = -1, sLONG Offset = 0, sLONG* ActualLen = nil) const;

		virtual Blob4D* Clone(Boolean WithData) const;

		void GetRawData( void* *outData, sLONG *outnbbytes)
		{
			*outData = (void*)fString;
			*outnbbytes = fNbBytes;
		}

};


Blob4D* CreBlobText(DataTable *df);
Blob4D* CreBlobTextUTF8(DataTable *df);


																				/* ***************************** */
																				

class BlobWithPtr : public Blob4D
{
	public:
		inline BlobWithPtr(DataTable *xDF) : Blob4D(xDF) { fData = nil; fDataLen = 0; fOutsideSuffixe = L".blob";};

		virtual sLONG GetBlobType() { return kBlobTypeWithPtr; };

		virtual bool SaveObj(VSize& outSizeSaved);
		
		virtual sLONG calclen(void) const;
		virtual Blob4D* loadobj(DataAddr4D xaddr, VError& err);

		virtual VError CopyDataFrom(const Blob4D* from);
		virtual uBOOL IsEmpty(void);
		
		inline sLONG AdjusteSize(sLONG size) { return (size+1023) & (~1023); };
		VError SetSize(sLONG inNewSize);
		virtual VError GetFrom(const void* p, sLONG len, sLONG Offset = 0);
		virtual VError PutInto(void* p, sLONG len = -1, sLONG Offset = 0, sLONG* ActualLen = nil) const;
		virtual VError GetFrom(VStream& buf);
		virtual VError PutInto(VStream& buf) const;

		//void SetData(void* data, sLONG datalen);
		inline void* GetDataPtr() { return (void*)fData; };
		inline const void* GetDataPtr() const { return (const void*)fData; };
		inline sLONG GetDataLen() const { return fDataLen; };

		virtual VError ThrowError( VError inErrCode, ActionDB4D inAction) const;

		virtual Blob4D* Clone(Boolean WithData) const;

		VError ReadFromStream( VStream* ioStream);
		VError WriteToStream( VStream* ioStream, bool withLength = true) const;

		virtual VError LoadDataFromPath();

		virtual VSize GetDataLength()
		{
			return (VSize)fDataLen;
		}

		virtual bool MatchAllocationNumber(sLONG allocationNumber);


	protected:
		virtual ~BlobWithPtr();

		VPtr fData;
		sLONG fDataLen;
};

Blob4D* CreBlobWithPtr(DataTable *df);


																				/* ***************************** */



struct BlobInRec
{
	sLONG numfield;
	tHandle hh;
};




																				/* ***************************** */


extern CodeReg *Blob_CodeReg;

inline void RegisterBlob(uLONG id, CreBlob_Code Code) { Blob_CodeReg->Register(id,(void*)Code); };
inline CreBlob_Code FindBlobCode(uLONG id) { return( (CreBlob_Code)(Blob_CodeReg->GetCodeFromID(id)) ); };

void InitBlob4D();
void DeInitBlob4D();


#endif