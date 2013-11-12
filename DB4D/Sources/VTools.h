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
#ifndef __VTOOLS__
#define __VTOOLS__

#if FORMAC
#pragma segment Tools
#endif

#ifdef RANGECHECK
// #define CALLRANGECHECK(x) xDoRangeCheck(x)
#define CALLRANGECHECK(n) assert((n>=0)&&(n<fNbOccupe));
#else
#define CALLRANGECHECK(x)
#endif


extern VCppMemMgr *gAlternateCppMem;
extern VCppMemMgr *gCppMem;

inline void vYield() {VTask::Yield();}
inline void vYieldNow() {VTask::YieldNow();}

/* -------------------------------------------------------------------------------------------------------------- */

/*
template<class Type, sLONG NbAllocatedElems>
class Vx1ArrayOf : public VStackArrayOf<Type, NbAllocatedElems>
{
public:
	Type& operator[]( VIndex inIndex) {assert( inIndex > 0 && inIndex <= this->fCount); return this->fFirst[inIndex-1];}
	const Type& operator[]( VIndex inIndex) const {assert( inIndex > 0 && inIndex <= this->fCount); return this->fFirst[inIndex-1];}
};


template<class Type, sLONG NbAllocatedElems>
class Vx0ArrayOf : public VStackArrayOf<Type, NbAllocatedElems>
{
};
*/


template<class Type>
class V1ArrayOf : public VArrayOf<Type>
{
public:
	Type& operator[]( VIndex inIndex) {assert( inIndex > 0 && inIndex <= this->fCount); return this->fFirst[inIndex-1];}
	const Type& operator[]( VIndex inIndex) const {assert( inIndex > 0 && inIndex <= this->fCount); return this->fFirst[inIndex-1];}
};


template<class Type>
class V0ArrayOf : public VArrayOf<Type>
{
};


template<class Type, sLONG NbAllocatedElems>
class Vx1ArrayOf : public V1ArrayOf<Type>
{
};

template<class Type, sLONG NbAllocatedElems>
class Vx0ArrayOf : public V0ArrayOf<Type>
{
};


template<class Type>
class V1ArrayOfRetainedPtr : public VArrayRetainedPtrOf<Type>
{
public:
	V1ArrayOfRetainedPtr()
	{
		this->SetOwnership(true);
	}

	Type& operator[]( VIndex inIndex) {assert( inIndex > 0 && inIndex <= this->fCount); return this->fFirst[inIndex-1];}
	const Type& operator[]( VIndex inIndex) const {assert( inIndex > 0 && inIndex <= this->fCount); return this->fFirst[inIndex-1];}

	Boolean CopyAndRetainFrom(const VArrayOf<Type>& inFrom )
	{
		Boolean res = this->CopyFrom(inFrom);
		this->SetOwnership(true);
		if (res)
		{
			Type* p = this->fFirst;
			sLONG n = this->fCount;
			while (n>0)
			{
				if ( (*p) != nil)
					(*p)->Retain();
				p++;
				n--;
			}
		}

		return res;
	}
};

/*
template<class Type>
class V0ArrayOfRetainedPtr : public VArrayOf<Type>
{
	V0ArrayOfRetainedPtr()
	{
		this->SetOwnership(true);
	}

	Boolean CopyAndRetainFrom(const VArrayOf<Type>& inFrom )
	{
		Boolean res = CopyFrom(inFrom);
		if (res)
		{
			Type* p = this->fFirst;
			sLONG n = this->fCount;
			while (n>0)
			{
				if ( (*p) != nil)
					(*p)->Retain();
				p++;
				n--;
			}
		}

		return res;
	}
};
*/
/*
template<class Type, sLONG NbAllocatedElems>
class Vx1ArrayOfRetainedPtr : public VStackArrayRetainedPtrOf<Type, NbAllocatedElems>
{
public:
	Vx1ArrayOfRetainedPtr()
	{
		this->SetOwnership(true);
	}

	Type& operator[]( VIndex inIndex) {assert( inIndex > 0 && inIndex <= this->fCount); return this->fFirst[inIndex-1];}
	const Type& operator[]( VIndex inIndex) const {assert( inIndex > 0 && inIndex <= this->fCount); return this->fFirst[inIndex-1];}

	Boolean CopyAndRetainFrom(const VArrayOf<Type>& inFrom )
	{
		Boolean res = CopyFrom(inFrom);
		this->SetOwnership(true);
		if (res)
		{
			Type* p = this->fFirst;
			sLONG n = this->fCount;
			while (n>0)
			{
				if ( (*p) != nil)
					(*p)->Retain();
				n--;
				p++;
			}
		}

		return res;
	}
};
*/

template<class Type, sLONG NbAllocatedElems>
class Vx1ArrayOfRetainedPtr : public V1ArrayOfRetainedPtr<Type>
{
};

/*
template<class Type, sLONG NbAllocatedElems>
class Vx0ArrayOfRetainedPtr : public VStackArrayOf<Type, NbAllocatedElems>
{
	Vx0ArrayOfRetainedPtr()
	{
		this->SetOwnership(true);
	}

	Boolean CopyAndRetainFrom(const VArrayOf<Type>& inFrom )
	{
		Boolean res = CopyFrom(inFrom);
		if (res)
		{
			Type* p = this->fFirst;
			sLONG n = this->fCount;
			while (n>0)
			{
				if ( (*p) != nil)
					(*p)->Retain();
				p++;
				n--;
			}
		}

		return res;
	}
};
*/

/* -------------------------------------------------------------------------------------------------------------- */

#ifdef obsoleteCode

class OrderdByKeyCollection : public VObject
{
	public:
		OrderdByKeyCollection(void) { ; };
		virtual sLONG xPutAt(void* key, void* data) = 0;
		virtual sLONG xDeleteAt(void* key) = 0;
		virtual sLONG xGetAt(void* key, void* data) = 0;
		virtual void xAlterCurrent(void* data) = 0;
		virtual sLONG PosToFirst(void) = 0;
		virtual sLONG xGetNext(void* data) = 0;
		virtual sLONG xGetNextEqual(void* data) = 0;
		virtual sLONG GetNbElem(void) = 0;
		
	protected:
	
};


class BinaryTreeNode : public VObject
{
	public:
		BinaryTreeNode(void) { left = nil; right = nil; father = nil; bf = 0; };
		BinaryTreeNode* LeftMost(void);
		BinaryTreeNode* Next(void);
		BinaryTreeNode* NextFather(void);
		void DisposeAll(void);
		BinaryTreeNode* Insert(BinaryTreeNode* x, uBOOL *unbalanced);
		BinaryTreeNode* FindNode(BinaryTreeNode* x);

		BinaryTreeNode* left_rotation(uBOOL *unbalanced);
		BinaryTreeNode* right_rotation(uBOOL *unbalanced);
		void ClearFather(void) { father = nil; };

		virtual void CopyDataTo(void* data) = 0;
		virtual void CopyDataFrom(void* data) = 0;
		virtual void CopyKeyTo(void* key) = 0;
		virtual void CopyKeyFrom(void* key) = 0;
		virtual uBOOL isInf(BinaryTreeNode* other) = 0;
		virtual uBOOL isEqual(BinaryTreeNode* other) = 0;

#if VERSIONDEBUG
		void checktree(BinaryTreeNode* supposedFather);
#endif
		
	protected:
		BinaryTreeNode* left;
		BinaryTreeNode* right;
		BinaryTreeNode* father;
		short bf;
		
};


class BinaryTree : public OrderdByKeyCollection
{
	public:
		BinaryTree(void);
		virtual ~BinaryTree();
		virtual sLONG xPutAt(void* key, void* data);
		virtual sLONG xDeleteAt(void* key);
		virtual sLONG xGetAt(void* key, void* data);
		virtual void xAlterCurrent(void* data);
		virtual sLONG PosToFirst(void);
		virtual sLONG xGetNext(void* data);
		virtual sLONG xGetNextEqual(void* data);
		virtual sLONG GetNbElem(void);
		
		virtual BinaryTreeNode* NewNode(void) = 0;
		
#if VERSIONDEBUG
		void checktree(void);
#endif

	protected:
		BinaryTreeNode* root;
		BinaryTreeNode* current;
		sLONG nbelem;
		
};


								/* ---------------------------------------- */

template<class elem>
class LBinTreeNode : public BinaryTreeNode
{
	public:
		LBinTreeNode(void) {;};
		virtual void CopyDataTo(void* data) { *((elem*)data) = fdata; };
		virtual void CopyDataFrom(void* data) { fdata = *((elem*)data); };
		virtual void CopyKeyTo(void* key) { *((sLONG*)key) = fkey; };
		virtual void CopyKeyFrom(void* key) { fkey = *((sLONG*)key); };
		virtual uBOOL isInf(BinaryTreeNode* other) { return( fkey < ((LBinTreeNode<elem>*)other)->fkey ); };
		virtual uBOOL isEqual(BinaryTreeNode* other) { return( fkey == ((LBinTreeNode<elem>*)other)->fkey ); };
		
	protected:
		sLONG fkey;
		elem fdata;
};

template<class elem>
class LBinTree : public BinaryTree
{
	public:
		LBinTree(void) {;};
		sLONG PutAt(sLONG key, elem& data) { return(xPutAt(&key, &data)); };
		sLONG GetAt(sLONG key, elem& data) { return(xGetAt(&key, &data)); };
		void AlterCurrent(elem& data) { xAlterCurrent(&data); };
		sLONG DeleteAt(sLONG key) { return(xDeleteAt(&key)); };
		sLONG GetNext(elem& data) { return(xGetNext(&data)); };
		sLONG GetNextEqual(elem& data) { return(xGetNextEqual(&data)); };
		
		virtual BinaryTreeNode* NewNode(void) { return(new LBinTreeNode<elem>); };
		
};


								/* ---------------------------------------- */


// ATTENTION : NE PAS UTILISER GenBinTree


template<class elem, class KEY>
class GenBinTreeNode : public BinaryTreeNode
{
	public:
		GenBinTreeNode(void) {;};
		virtual void CopyDataTo(void* data) { *((elem*)data) = fdata; };
		virtual void CopyDataFrom(void* data) { fdata = *((elem*)data); };
		virtual void CopyKeyTo(void* key) { *((KEY*)key) = fkey; };
		virtual void CopyKeyFrom(void* key) { fkey = *((KEY*)key); };
		virtual uBOOL isInf(BinaryTreeNode* other) { return( fkey < ((GenBinTreeNode<elem,KEY>*)other)->fkey ); };
		virtual uBOOL isEqual(BinaryTreeNode* other) { return( fkey == ((GenBinTreeNode<elem,KEY>*)other)->fkey ); };
		
	protected:
		KEY fkey;
		elem fdata;
};


template<class elem, class KEY>
class GenBinTree : public BinaryTree
{
	public:
		GenBinTree(void) {;};
		sLONG Put(elem& data) { KEY key = data->GetKey(); return(xPutAt(&key, &data)); };
		sLONG Get(elem& data) { KEY key = data->GetKey(); return(xGetAt(&key, &data)); };
		void AlterCurrent(elem& data) { xAlterCurrent(&data); };
		sLONG Delete(elem& data) { KEY key = data->GetKey(); return(xDeleteAt(&key)); };
		sLONG GetNext(elem& data) { return(xGetNext(&data)); };
		sLONG GetNextEqual(elem& data) { return(xGetNextEqual(&data)); };
		
		virtual BinaryTreeNode* NewNode(void) { return(new GenBinTreeNode<elem,KEY>); };
		
};


								/* ---------------------------------------- */

// la class elem doit implementer la methode suivante :
// slONG elem::Compare(elem& other, uBOOL strict);  	==> return -1 si *this < other
// 														==> return 0  si *this = other
// 														==> return 1  si *this > other

template<class elem>
class BinTreeNode : public BinaryTreeNode
{
	public:
		BinTreeNode(void) {;};
		virtual void CopyDataTo(void* data) { *((elem*)data) = fdata; };
		virtual void CopyDataFrom(void* data) { fdata = *((elem*)data); };
		virtual void CopyKeyTo(void* key) { ; };
		virtual void CopyKeyFrom(void* key) { ; };
		virtual uBOOL isInf(BinaryTreeNode* other) { return( fdata.Compare(((BinTreeNode<elem>*)other)->fdata, false ) == -1); };
		virtual uBOOL isEqual(BinaryTreeNode* other) { return( fdata.Compare(((BinTreeNode<elem>*)other)->fdata, true ) == 0); };
		
	protected:
		elem fdata;
};


template<class elem>
class BinTree : public BinaryTree
{
	public:
		BinTree(void) {;};
		sLONG Put(elem& data) { return(xPutAt(nil, &data)); };
		sLONG Get(elem& data) { return(xGetAt(&data, &data)); };
		void AlterCurrent(elem& data) { xAlterCurrent(&data); };
		sLONG Delete(elem& data) { return(xDeleteAt(&data)); };
		sLONG GetNext(elem& data) { return(xGetNext(&data)); };
		sLONG GetNextEqual(elem& data) { return(xGetNextEqual(&data)); };
		
		virtual BinaryTreeNode* NewNode(void) { return(new BinTreeNode<elem>); };
		
};


// la class elem doit implementer la methode suivante :
// slONG elem::Compare(const elem *other, Booloean strict) const;
//					  	==> return -1 si *this < other
// 						==> return 0  si *this = other
// 						==> return 1  si *this > other

template<class elem>
class BinTreeNodePtr : public BinaryTreeNode
{
	public:
		BinTreeNodePtr(void) {;};
		virtual void CopyDataTo(void* data) { *((elem*)data) = fdata; };
		virtual void CopyDataFrom(void* data) { fdata = *((elem*)data); };
		virtual void CopyKeyTo(void* key) { ; };
		virtual void CopyKeyFrom(void* key) { ; };
		virtual uBOOL isInf(BinaryTreeNode* other) { return( fdata->Compare(((BinTreeNodePtr<elem>*)other)->fdata, false ) == -1); };
		virtual uBOOL isEqual(BinaryTreeNode* other) { return( fdata->Compare(((BinTreeNodePtr<elem>*)other)->fdata, true ) == 0); };

	protected:
		elem fdata;
};


template<class elem>
class BinTreePtr : public BinaryTree
{
	public:
		BinTreePtr(void) {;};
		sLONG Put(elem& data) { return(xPutAt(nil, &data)); };
		sLONG Get(elem& data) { return(xGetAt(&data, &data)); };
		void AlterCurrent(elem& data) { xAlterCurrent(&data); };
		sLONG Delete(elem& data) { return(xDeleteAt(&data)); };
		sLONG GetNext(elem& data) { return(xGetNext(&data)); };
		sLONG GetNextEqual(elem& data) { return(xGetNextEqual(&data)); };
		
		virtual BinaryTreeNode* NewNode(void) { return(new BinTreeNodePtr <elem>); };
		
};


/* -------------------------------------------------------------------------------------------------------------- */


typedef struct ListBlockGenDataBlock
{
	ListBlockGenDataBlock* nextpage;
	ListBlockGenDataBlock* prevpage;
	sLONG nbelem;
} ListBlockGenDataBlock;

class ListByBlockGen : public VObject
{
	public:
		ListByBlockGen(sLONG SizeElem, sLONG NbAlloc = 20);
		virtual ~ListByBlockGen();
		sLONG xAddElem(void* x);
		void xDelCurElem(void);
		void* xGetPtrOnFirstElem(void);
		void* xGetPtrOnNextElem(void);
		void* xGetPtrOnPrevElem(void);
		void* xGetPtrOnLastElem(void);
		
	protected:
		ListBlockGenDataBlock* firstpage;
		sLONG nbelem;
		ListBlockGenDataBlock* currentpage;
		ListBlockGenDataBlock* lastpage;
		sLONG curelem;
		sLONG lastelem;
		sLONG sizealloc, nballoc;
};


template<class elem>
class ListByBlock : public ListByBlockGen
{
	public:
		ListByBlock(sLONG NbAlloc = 20) : ListByBlockGen(sizeof(elem), NbAlloc) { ; };
		sLONG AddElem(elem& data) { return(xAddElem(&data)); };
		elem* GetPtrOnFirstElem(void) { return((elem*)xGetPtrOnFirstElem()); };
		elem* GetPtrOnLastElem(void) { return((elem*)xGetPtrOnLastElem()); };
		elem* GetPtrOnNextElem(void) { return((elem*)xGetPtrOnNextElem()); };
		elem* GetPtrOnPrevElem(void) { return((elem*)xGetPtrOnPrevElem()); };
		void DelCurElem(void) { xDelCurElem(); }; // juste pour la symetrie
		
};


#endif


#endif



