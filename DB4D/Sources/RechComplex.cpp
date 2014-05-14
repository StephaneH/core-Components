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
#include "4DDBHeaders.h"
 

ComplexSelection::ComplexSelection(const CQTableDefVector& inTargets)
{
	fTargets = inTargets;
	xinit();
}


ComplexSelection::ComplexSelection(const CQTableDefVector& inTargets, bool leftJoin, bool rightJoin)
{
	fTargets = inTargets;
	xinit();
	fKeepNulls[0] = rightJoin;
	fKeepNulls[1] = leftJoin;
}


ComplexSelection::ComplexSelection(const CQTableDef& inTarget)
{
	fTargets.push_back(inTarget);
	xinit();
}


void ComplexSelection::xinit()
{
	fNbRows = 0;
	fNbCols = (sLONG)fTargets.size();
	fKeepNulls.resize(fNbCols, false);
	fNbRowsPerPage = kNbElemInComplexSel / fNbCols;
	fFileDesc = nil;
	fTempFile = nil;
	fCurLockPage = -1;
	fCurPageFree = -1;
	fFreeMemInAction = 0;
	fFileSize = 0;
	fThreadLockAll = 0;
}


ComplexSelection::~ComplexSelection()
{
	RemoveFromCache();
	dispose();
	if ( fFileDesc != nil )
	{
		delete fFileDesc;
	}
	if (fTempFile != nil)
	{
		fTempFile->Delete();
		fTempFile->Release();
	}
}


void ComplexSelection::dispose()
{
	WaitForFreeMemAndLockAll(false);
	for (ComplexSelVector::iterator cur = fTabSel.begin(), end = fTabSel.end(); cur != end; cur++)
	{
		ComplexSel* p = *cur;
		if (p != nil)
		{
			FreeFastMem((void*)p);
		}
	}
	fTabSel.clear();
	fNbRows = 0;
	ClearCurLockPage();
}



bool ComplexSelection::FreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed)
{
	sLONG i,nb;
	VError err;
	bool okfree = false;

	fFreeMemInAction = 1;

	VSize tot=0;

	if (combien==0) 
		combien=kMaxPositif;

	
	sLONG curlock = VInterlocked::AtomicGet(&fCurLockPage);
	if (curlock != -2 || fThreadLockAll == VTask::GetCurrentID())
	{
		sLONG npage = 0;
		for (ComplexSelVector::iterator cur = fTabSel.begin(), end = fTabSel.end(); cur != end; cur++, npage++)
		{
			fCurPageFree = npage;
			sLONG curlockpage = VInterlocked::AtomicGet(&fCurLockPage);
			if (curlockpage != npage)
			{
				ComplexSel* p = *cur;
				
				if (p != nil && OKAllocationNumber(p, allocationBlockNumber))
				{
					RecIDType x = (*p)[kNbElemInComplexSel];
					if (x == 0x10000)
					{
						StErrorContextInstaller errs(false, false);
						if (OpenSelFile())
						{
							DataAddr4D ou = CalcPagePos(npage);
							
							VError err2 = fFileDesc->PutData(p, kSizeComplexSelOnDisk, ou);
							if (err2 == VE_OK)
								x = 0;
						}
					}
					if (x == 0)
					{
						FreeFastMem((void*)p);
						tot += kSizeComplexSelInMem;
						*cur = nil;
					}
				}
			}
			fCurPageFree = -1;
		}

		
		okfree = true;
		ClearMemRequest();
	}

	fFreeMemInAction = 0;

	outSizeFreed = tot;

	return(okfree);
}


void ComplexSelection::WaitForFreeMemAndLockAll(bool withThreadID) const
{
	fCurLockPage = -2;
	while (fFreeMemInAction != 0)
	{
		VTask::YieldNow();
	}
	if (withThreadID)
	{
		fThreadLockAll = VTask::GetCurrentID();
	}
}


ComplexSel* ComplexSelection::loadmem(sLONG numpage, VError& err, bool retain, void* debugIterator) const
{
	ComplexSel* result = nil;

	if (numpage >= 0 && numpage < (sLONG)fTabSel.size())
	{
		SetCurLockPage(numpage);
		while (fCurPageFree == numpage)
		{
			VTask::YieldNow();
		}
		err = VE_OK;
		result = fTabSel[numpage];
		if (result == nil)
		{
			DataAddr4D ou = CalcPagePos(numpage);
			if (fFileDesc == nil)
				err = ThrowBaseError(memfull);
			else
			{
				result = (ComplexSel*)GetFastMem(kSizeComplexSelInMem, false, 'csel');
				if (result == nil)
				{
					err = ThrowBaseError(memfull);
				}
				else
				{
					(*result)[kNbElemInComplexSel] = 0;
					err = fFileDesc->GetData( result, kSizeComplexSelOnDisk, ou);
					if (err	== VE_OK)
					{
						ComplexSelection* that = (ComplexSelection*)this;
						that->fTabSel[numpage] = result;
						if (retain)
							tempretain(result, debugIterator);
						else
							templock(result);
					}
					else
					{
						FreeFastMem((void*)result);
						result = nil;
					}
				}
			}
		}
		else
		{
			if (retain)
				tempretain(result, debugIterator);
			else
				templock(result);
		}
	}
	else
	{
		err = ThrowBaseError(VE_DB4D_INDICE_OUT_OF_RANGE);
	}

	ClearCurLockPage();

	return result;
}


bool ComplexSelection::OpenSelFile()
{
	bool ok = false;
	VError err = VE_OK;
	StErrorContextInstaller errs(false, false);

	if (fTempFile == nil)
	{
		fFileSize = 0;
		fFileDesc = nil;
		Base4D* db = fTargets[0].GetTable()->GetOwner();
		VUUID id(true);
		VString s;
		id.GetString(s);
		s = "sel_"+s;
		s += ".complexSel";

		VFolder *tempfolder = db->RetainTemporaryFolder( true, &err);
		if (tempfolder != nil)
		{
			fTempFile=new VFile(*tempfolder, s);
			tempfolder->Release();

			if (fTempFile->Exists())
			{
				fTempFile->Delete(); 
			}
			err = fTempFile->Create( true);
		}

		if (err == VE_OK)
		{
			err=fTempFile->Open( FA_READ_WRITE, &fFileDesc );
			if (err == VE_OK && fFileDesc != nil)
			{
				ok=true;
			}
		}

	}
	else
		ok = true;

	if (ok)
	{
		DataAddr4D newsize = ((DataAddr4D)kSizeComplexSelOnDisk) * (DataAddr4D)fTabSel.size();
		if (newsize > fFileSize)
		{
			err = fFileDesc->SetSize(newsize);
			fFileSize = newsize;
		}
	}

	return ok;
}

RecIDType ComplexSelection::GetRecID(sLONG Column, RecIDType Row, VError& err) const
{
	err = VE_OK;
	if (Column >= fNbCols || Column < 0)
	{
		err = VE_DB4D_INDICE_OUT_OF_RANGE;
		return -1;
	}
	else
	{
		if (Row >= fNbRows || Row < 0)
		{
			err = VE_DB4D_INDICE_OUT_OF_RANGE;
			return -1;
		}
		else
		{
			ComplexSel* p = loadmem(Row / fNbRowsPerPage, err);
			if (p != nil)
			{
				RecIDType result = (*p)[(Row % fNbRowsPerPage)*fNbCols + Column];
				tempunlock(p);
				return result;
			}
			else
				return -1;
		}
	}
}


RecIDType* ComplexSelection::calcIterator(RecIDType row, RecIDType* storage) const
{
	VError err;
	if (row >= fNbRows || row < 0)
	{
		return nil;
	}
	else
	{
		ComplexSel* p = loadmem(row / fNbRowsPerPage, err);
		if (p != nil)
		{
			RecIDType* px = &(*p)[(row % fNbRowsPerPage)*fNbCols];
			std::copy(px, px+fNbCols, storage);
			px = storage;
			tempunlock(p);
			return px;
		}
		else
		{
			throw ThrowBaseError(memfull, DBaction_AccessingComplexSelection);
			return nil;
		}
	}

}


RecIDType* ComplexSelection::calcIteratorLock(RecIDType row, ComplexSel* &xLock, void* debugIterator) const
{
	RecIDType* px = nil;
	VError err;
	ComplexSel* oldlock = xLock;
	xLock = nil;
	if (row >= fNbRows || row < 0)
	{
		px = nil; // put a break here
	}
	else
	{
		ComplexSel* p = loadmem(row / fNbRowsPerPage, err, true, debugIterator);
		if (p != nil)
		{
			px = &(*p)[(row % fNbRowsPerPage)*fNbCols];
			xLock = p;
			tempmodify(p);
		}
		else
		{
			if (oldlock != nil)
			{
				temprelease(oldlock, debugIterator);
			}
			throw ThrowBaseError(memfull, DBaction_AccessingComplexSelection);
		}
	}

	if (oldlock != nil)
	{
		temprelease(oldlock, debugIterator);
	}
	return px;
}


VError ComplexSelection::GetFullRow(RecIDType Row, ComplexSelRow& outRow) const
{
	VError err = VE_OK;
	if (Row >= fNbRows || Row < 0)
	{
		err = VE_DB4D_INDICE_OUT_OF_RANGE;
	}
	else
	{
		ComplexSel* p = loadmem(Row / fNbRowsPerPage, err);
		if (p != nil)
		{
			RecIDType* px = &(*p)[(Row % fNbRowsPerPage)*fNbCols];
			try
			{
				outRow.resize(fNbCols, -1);
				copy(px, px+fNbCols, outRow.begin());
				/*
				ComplexSelRow::iterator q = outRow.begin();
				for (sLONG i = 0; i < fNbCols; i++, px++, q++)
				{
					*q = *px;
				}
				*/
			}
			catch (...)
			{
				err = ThrowBaseError(memfull, DBaction_AccessingComplexSelection);
			}
			tempunlock(p);
		}
	}
	
	return err;
}


Bittab* ComplexSelection::GenereBittab(sLONG column, BaseTaskInfo* context, VError &outErr) const
{
	outErr = VE_OK;
	Bittab* result = new Bittab;

	if (result == nil)
		outErr = ThrowBaseError(memfull, DBaction_BuildingSelection);
	else
	{
		if ( fNbCols > column )
		{
			Table* target = fTargets[column].GetTable();
			outErr = result->aggrandit(target->GetDF()->GetMaxRecords(context));
			if (outErr == VE_OK)
			{
				sLONG nbpage = CalcNbPage();
				for (sLONG i = 0; i < nbpage && outErr == VE_OK; i++)
				{
					ComplexSel* p = loadmem(i, outErr);
					if (p != nil)
					{
						RecIDType* px = &(*p)[column];
						sLONG nbelem = fNbRowsPerPage;
						if (i == (nbpage-1))
							nbelem = (fNbRows % fNbRowsPerPage);
						for (sLONG j = 0; j < nbelem && outErr == VE_OK; j++)
						{
							outErr = result->Set(*px, false);
							px = px + fNbCols;
						}
						tempunlock(p);
					}
				}
			}
		}
	}
	
	if (outErr != VE_OK && result != nil)
	{
		result->Release();
		result = nil;
	}
	return result;
}


VError ComplexSelection::FromBittab(sLONG column, BaseTaskInfo* context, Bittab* from, Boolean canStealBittab)
{
	VError err = VE_OK;
	dispose();
	if (from != nil)
	{
		WaitForFreeMemAndLockAll(true);
		fNbRows = from->Compte();
		sLONG nbpage = CalcNbPage();
		try
		{
			fTabSel.resize(nbpage, nil);
			sLONG curfic = from->FindNextBit(0);
			sLONG curpage = -1;
			sLONG curelem;
			ComplexSel* p = nil;
			RecIDType* px;
			while (curfic != -1 && err == VE_OK)
			{
				if (p == nil)
				{
					curpage++;
					p = (ComplexSel*)GetFastMem(kSizeComplexSelInMem, false, 'csel');
					if (p != nil)
					{
						if (fNbCols != 1)
						{
							std::fill(&(*p)[0], &(*p)[kNbElemInComplexSel], -1);
						}
						(*p)[kNbElemInComplexSel] = 0;
						tempmodify(p);
						templock(p);
						fTabSel[curpage] = p;
						px = &(*p)[column];
						curelem = column;
					}
					else
						err = ThrowBaseError(memfull, DBaction_BuildingSelection);
				}
				if (p != nil)
				{
					*px = curfic;
					px = px + fNbCols;
					curelem = curelem + fNbCols;
					if (curelem >= kNbElemInComplexSel)
					{
						tempunlock(p);
						p = nil;
					}
				}
				curfic = from->FindNextBit(curfic+1);
			}

			if (p != nil)
				tempunlock(p);
		}
		catch (...)
		{
			err = ThrowBaseError(memfull, DBaction_BuildingSelection);
		}

		if (err != VE_OK)
			dispose();

		if (canStealBittab)
			from->Release();
		ClearCurLockPage();
	}
	return err;
}


template <sLONG nbcol>
bool SortComplexSelPredicate<nbcol>::operator ()(const typename ComplexSelectionIterator<nbcol>::value_type& val1, const typename ComplexSelectionIterator<nbcol>::value_type& val2)
{
	VTask::Yield();
	for (vector<sLONG>::iterator cur = fColsToComp.begin(), end = fColsToComp.end(); cur != end; cur++)
	{
		sLONG column = (*cur) & 65535;
		if (val1.storage[column] < val2.storage[column])
			return true;
		else
		{
			if (val1.storage[column] > val2.storage[column])
				return false;
		}
	}
	return false;
}


template <sLONG nbcol>
VError SortComplexSelByRecIDs(ComplexSelection* sel, const vector<sLONG>& whichcolums)
{
	ComplexSelectionIterator<nbcol> start(sel, 0);
	ComplexSelectionIterator<nbcol> end(sel, sel->GetNbRows());

	std::sort(start, end, SortComplexSelPredicate<nbcol>(whichcolums));
	return VE_OK;
}

VError ComplexSelection::SortByRecIDs(const vector<sLONG>& whichcolums)
{
	VError err = VE_DB4D_NOTIMPLEMENTED;
	switch(fNbCols)
	{
		case 1:
			err = SortComplexSelByRecIDs<1>(this, whichcolums);
			break;
		case 2:
			err = SortComplexSelByRecIDs<2>(this, whichcolums);
			break;
		case 3:
			err = SortComplexSelByRecIDs<3>(this, whichcolums);
			break;
		case 4:
			err = SortComplexSelByRecIDs<4>(this, whichcolums);
			break;
		case 5:
			err = SortComplexSelByRecIDs<5>(this, whichcolums);
			break;
		case 6:
			err = SortComplexSelByRecIDs<6>(this, whichcolums);
			break;
		case 7:
			err = SortComplexSelByRecIDs<7>(this, whichcolums);
			break;
		case 8:
			err = SortComplexSelByRecIDs<8>(this, whichcolums);
			break;
		case 9:
			err = SortComplexSelByRecIDs<9>(this, whichcolums);
			break;
		case 10:
			err = SortComplexSelByRecIDs<10>(this, whichcolums);
			break;
		case 11:
			err = SortComplexSelByRecIDs<11>(this, whichcolums);
			break;
		case 12:
			err = SortComplexSelByRecIDs<12>(this, whichcolums);
			break;
		case 13:
			err = SortComplexSelByRecIDs<13>(this, whichcolums);
			break;
		case 14:
			err = SortComplexSelByRecIDs<14>(this, whichcolums);
			break;
		case 15:
			err = SortComplexSelByRecIDs<15>(this, whichcolums);
			break;
		case 16:
			err = SortComplexSelByRecIDs<16>(this, whichcolums);
			break;
		case 17:
			err = SortComplexSelByRecIDs<17>(this, whichcolums);
			break;
		case 18:
			err = SortComplexSelByRecIDs<18>(this, whichcolums);
			break;
		case 19:
			err = SortComplexSelByRecIDs<19>(this, whichcolums);
			break;
		case 20:
			err = SortComplexSelByRecIDs<20>(this, whichcolums);
			break;
	}
	return err;
}


VError ComplexSelection::ReOrderColumns(const CQTableDefVector& neworder)
{
	VError err = VE_OK;
	if (!(neworder == fTargets))
	{
		if (neworder.size() == fTargets.size())
		{
			vector<sLONG> colpos;
			colpos.resize(neworder.size());
			vector<sLONG>::iterator curpos = colpos.begin();
			CQTableDefVector::const_iterator cur = neworder.begin(), end = neworder.end();

			for ( ; cur != end && err == VE_OK; cur++, curpos++)
			{
				CQTableDefVector::iterator found = find(fTargets.begin(), fTargets.end(), *cur);
				if (found == fTargets.end())
					err = VE_DB4D_INDICE_OUT_OF_RANGE;
				else
				{
					*curpos = found - fTargets.begin();
				}
			}

			sLONG nbcol = (sLONG)colpos.size();

			if (err == VE_OK)
			{
				sLONG nbpage = CalcNbPage();
				for (sLONG i = 0; i < nbpage && err == VE_OK; i++)
				{
					ComplexSel* p = loadmem(i, err);
					if (p != nil)
					{
						RecIDType* px = (RecIDType*)p;

						sLONG nbelem = fNbRowsPerPage;
						if (i == (nbpage-1))
							nbelem = (fNbRows % fNbRowsPerPage);
						for (sLONG j = 0; j < nbelem; j++)
						{
							RecIDType temp[512];
							curpos = colpos.begin();
							for (sLONG k = 0; k < fNbCols; k++, curpos++)
							{
								temp[k] = px[*curpos];
							}
							copy(&temp[0], &temp[fNbCols], px);
							px = px + nbcol;
						}
						tempunlock(p);
					}
				}
			}

			if (err == VE_OK)
				fTargets = neworder;

		}
		else
			err = VE_DB4D_INDICE_OUT_OF_RANGE;
	}

	return err;
}


sLONG ComplexSelection::FindTarget(const CQTableDef& inTarget)
{
	CQTableDefVector::iterator found = find(fTargets.begin(), fTargets.end(), inTarget);
	if (found == fTargets.end())
		return -1;
	else
		return (sLONG)(found - fTargets.begin());
}


VError ComplexSelection::GetColumn(sLONG column, CQTableDef& outTarget)
{
	if (column>=0 && column<fNbCols)
	{
		outTarget = fTargets[column];
		return VE_OK;
	}
	else
		return ThrowBaseError(VE_DB4D_INDICE_OUT_OF_RANGE, DBaction_AccessingComplexSelection);
}


VError ComplexSelection::RetainColumns(QueryTargetVector& outColumns)
{
	try
	{
		outColumns.clear();
		for (CQTableDefVector::iterator cur = fTargets.begin(), end = fTargets.end(); cur != end; cur++)
		{
			CDB4DBase* base = cur->GetTable()->GetOwner()->RetainBaseX();
			CDB4DTable* theTable = new VDB4DTable( VDBMgr::GetManager(), dynamic_cast<VDB4DBase*>(base), cur->GetTable());
			base->Release();
			QueryTarget xx;
			xx.first = theTable;
			xx.second = cur->GetInstance();
			outColumns.push_back(xx);
		}
		return VE_OK;
	}
	catch (...)
	{
		return ThrowBaseError(memfull, DBaction_AccessingComplexSelection);
	}
}


VErrorDB4D ComplexSelection::ToDB4D_ComplexSel(DB4D_ComplexSel& outSel)
{
	try
	{
		outSel.reserve(fNbCols);
		for (sLONG i = 0; i<fNbCols; i++)
		{
			ComplexSelColumn* col = new ComplexSelColumn;
			if (col == nil)
				throw ThrowBaseError(memfull, DBaction_BuildingSelection);
			outSel.push_back(col);
			col->reserve(fNbRows);
		}
		for (sLONG i = 0; i < fNbRows; i++)
		{
			RecIDType storage[30];
			RecIDType* p = calcIterator(i, &storage[0]);
			DB4D_ComplexSel::iterator cur = outSel.begin();
			for (sLONG j = 0; j < fNbCols; j++, p++, cur++)
			{
				(*cur)->push_back(*p);
			}
		}
		return VE_OK;
	}
	catch (...)
	{
		return ThrowBaseError(memfull, DBaction_BuildingSelection);
	}
}

typedef pair<sLONG, bool> cqtMapElem;
typedef map<CQTableDef, cqtMapElem> cqtMap;


VError ComplexSelection::AddRow(const ComplexSelRow& inRow)
{
	VError err = VE_OK;
	VTask::Yield();
	//try
	{
		if (fNbRows%fNbRowsPerPage == 0)
		{
			xbox_assert(CalcNbPage() == (sLONG)fTabSel.size());
			ComplexSel* p = (ComplexSel*)GetFastMem(kSizeComplexSelInMem, false, 'csel');
			if (p == nil)
				throw ThrowBaseError(memfull, DBaction_BuildingSelection);
			(*p)[kNbElemInComplexSel] = 0;
			templock(p);
			WaitForFreeMemAndLockAll(true);
			fTabSel.push_back(p);
			ClearCurLockPage();
		}
		fNbRows++;
		ComplexSel* p = loadmem((fNbRows-1) / fNbRowsPerPage, err);
		if (p != nil)
		{
			RecIDType* px = &(*p)[((fNbRows-1) % fNbRowsPerPage)*fNbCols];
			copy(inRow.begin(), inRow.end(), px);
			tempmodify(p);
			tempunlock(p);
		}
	}
	/*
	catch (...)
	{
		err = ThrowBaseError(memfull, DBaction_BuildingSelection);
	}
	*/
	return err;
}


VError ComplexSelection::MergeAndAddRow(RecIDType* px1, RecIDType* px2, vector<sLONG>& cfinal, ComplexSelRow& FinalRow)
{
	sLONG finalpos = 0;
	for (vector<sLONG>::iterator curx = cfinal.begin(), endx = cfinal.end(); curx != endx; curx++, finalpos++)
	{
		sLONG pos = *curx;
		if (pos >= 0)
		{
			if (px1 == nil)
				FinalRow[finalpos] = -2;
			else
				FinalRow[finalpos] = px1[pos];
		}
		else
		{
			pos = -pos - 1;
			if (px2 == nil)
				FinalRow[finalpos] = -2;
			else
				FinalRow[finalpos] = px2[pos];
		}
	}
	return AddRow(FinalRow);
}


CompareResult CompareRowIds(RecIDType* px1, RecIDType* px2, vector<sLONG>& c1, vector<sLONG>& c2)
{
	for (vector<sLONG>::iterator curx1 = c1.begin(), endx1 = c1.end(), curx2 = c2.begin(); curx1 != endx1; curx1++, curx2++)
	{
		sLONG pos1 = *curx1;
		sLONG pos2 = *curx2;
		bool keepnull1 = (pos1 > 65535);
		bool keepnull2 = (pos2 > 65535);
		pos1 = pos1 & 65535;
		pos2 = pos2 & 65535;

		RecIDType recid1 = px1[pos1];
		RecIDType recid2 = px2[pos2];
		if (recid1 == -2 && keepnull1)
		{
			return CR_EQUAL_NULL_LEFT;
		}
		else if (recid2 == -2 && keepnull2)
		{
			return CR_EQUAL_NULL_RIGHT;
		}
		else if (recid1 < recid2)
		{
			return CR_SMALLER;
		}
		else
		{
			if (recid1 > recid2)
				return CR_BIGGER;
		}
	}
	return CR_EQUAL;
}


ComplexSelection* ComplexSelection::CartesianProduct(const ComplexSelection& other, VError& err) const
{
	err = VE_OK;
	ComplexSelection* result = nil;

#if debuglr
	set<CQTableDef> alltargets;

	for (CQTableDefVector::const_iterator cur = fTargets.begin(), end = fTargets.end(); cur != end; cur++)
	{
		alltargets.insert(*cur);
	}

	for (CQTableDefVector::const_iterator cur = other.fTargets.begin(), end = other.fTargets.end(); cur != end; cur++)
	{
		xbox_assert (alltargets.find(*cur) == alltargets.end());
	}
#endif

	CQTableDefVector targetsfinal;
	targetsfinal = fTargets;
	std::copy(other.fTargets.begin(), other.fTargets.end(), back_insert_iterator<CQTableDefVector>(targetsfinal));

	result = new ComplexSelection(targetsfinal);
	if (result == nil)
		err = ThrowBaseError(memfull, DBaction_BuildingSelection);
	else
	{
		result->PutInCache();
		sLONG8 nbrow1 = fNbRows, nbrow2 = other.fNbRows;
		sLONG8 nbrow = nbrow1 * nbrow2;
		if (nbrow > (sLONG8)0x7FFFFFFE)
			err = ThrowBaseError(memfull, DBaction_BuildingSelection);
		else
		{
			ComplexSelRow rowfinal;
			rowfinal.resize(fNbCols+other.fNbCols);
			for (RecIDType i = 0; i < fNbRows && err == VE_OK; i++)
			{
				ComplexSelRow row1;
				err = GetFullRow(i, row1);
				if (err == VE_OK)
				{
					copy(row1.begin(), row1.end(), rowfinal.begin());
					ComplexSelRow::iterator start2 = rowfinal.begin() + row1.size();
					for (RecIDType j = 0; j < other.fNbRows && err == VE_OK; j++)
					{
						ComplexSelRow row2;
						err = GetFullRow(j, row2);
						if (err == VE_OK)
						{
							copy(row2.begin(), row2.end(), start2 );
							err = result->AddRow(rowfinal);
							VTask::Yield();
						}
					}
				}
			}
		}
	}


	if (err != VE_OK)
	{
		if (result != nil)
			delete result;
		result = nil;
		err = ThrowBaseError(VE_DB4D_CARTESIAN_PRODUCT_FAILED, DBaction_BuildingSelection);
	}

	return result;
}


ComplexSelection* ComplexSelection::MergeSel(ComplexSelection& WithOther, VError& err)
{
	ComplexSelection* result = nil;
	try
	{
		cqtMap m1,m2;
		sLONG pos = 0;
		for (CQTableDefVector::const_iterator cur = fTargets.begin(), end = fTargets.end(); cur != end; cur++, pos++)
		{
			m1[*cur] = make_pair(pos, fKeepNulls[pos]);
		}
		pos = 0;
		for (CQTableDefVector::const_iterator cur = WithOther.fTargets.begin(), end = WithOther.fTargets.end(); cur != end; cur++, pos++)
		{
			m2[*cur] = make_pair(pos, WithOther.fKeepNulls[pos]);
		}

		cqtMap::iterator cur1 = m1.begin(), end1 = m1.end();
		cqtMap::iterator cur2 = m2.begin(), end2 = m2.end();
		vector<bool> keepnulls;

		vector<sLONG> c1, c2, cfinal;
		CQTableDefVector targetsfinal;

		while (cur1 != end1 && cur2 != end2)
		{
			if (cur1->first == cur2->first)
			{
				keepnulls.push_back(cur1->second.second || cur2->second.second);
				cfinal.push_back(cur1->second.first);
				targetsfinal.push_back(cur1->first);
				if (cur1->second.second)
					c1.push_back(cur1->second.first | 65536);
				else
					c1.push_back(cur1->second.first);
				if (cur2->second.second)
					c2.push_back(cur2->second.first | 65536);
				else
					c2.push_back(cur2->second.first);
				cur1++;
				cur2++;
			}
			else
			{
				if (cur1->first < cur2->first)
				{
					keepnulls.push_back(cur1->second.second);
					cfinal.push_back(cur1->second.first);
					targetsfinal.push_back(cur1->first);
					cur1++;
				}
				else
				{
					keepnulls.push_back(cur2->second.second);
					cfinal.push_back(-(cur2->second.first+1));
					targetsfinal.push_back(cur2->first);
					cur2++;
				}
			}
		}

		for (; cur1 != end1; cur1++)
		{
			cfinal.push_back(cur1->second.first);
			keepnulls.push_back(cur1->second.second);
			targetsfinal.push_back(cur1->first);
		}

		for (; cur2 != end2; cur2++)
		{
			cfinal.push_back(-(cur2->second.first+1));
			keepnulls.push_back(cur2->second.second);
			targetsfinal.push_back(cur2->first);
		}

		if (c1.size() == 0)
		{
			err = ThrowBaseError(VE_DB4D_INDICE_OUT_OF_RANGE, DBaction_BuildingSelection);
		}
		else
		{
			err = SortByRecIDs(c1);
			if (err == VE_OK)
				WithOther.SortByRecIDs(c2);

			if (err == VE_OK)
			{
				result = new ComplexSelection(targetsfinal);
				if (result == nil)
					err = ThrowBaseError(memfull, DBaction_BuildingSelection);
				else
				{
					result->fKeepNulls = keepnulls;
					result->PutInCache();
					ComplexSelRow FinalRow; 
					FinalRow.resize(targetsfinal.size(), -1);
					sLONG row1 = 0, row2 = 0;
					RecIDType* px1 = nil;
					RecIDType* px2 = nil;
					while (row1 < fNbRows && row2< WithOther.fNbRows && err == VE_OK)
					{
						RecIDType storage1[30];
						RecIDType storage2[30];
						px1 = calcIterator(row1, &storage1[0]);
						px2 = WithOther.calcIterator(row2, &storage2[0]);
						xbox_assert(px1 != nil);
						xbox_assert(px2 != nil);
						CompareResult comp = CompareRowIds(px1, px2, c1, c2);

						if (comp == CR_EQUAL_NULL_LEFT)
						{
							err = result->MergeAndAddRow(px1, nil, cfinal, FinalRow);
							row1++;
						}
						else if (comp == CR_EQUAL_NULL_RIGHT)
						{
							err = result->MergeAndAddRow(nil, px2, cfinal, FinalRow);
							row2++;
						}
						else if (comp == CR_EQUAL)
						{
							sLONG row1x = row1;
							Boolean cont1 = true;
							sLONG lastrow2x;
							do
							{
								if (row1x >= fNbRows)
								{
									cont1 = false;
								}
								else
								{
									RecIDType storage1x[30];
									RecIDType* px1x = calcIterator(row1x, &storage1x[0]);
									CompareResult comp = CompareRowIds(px1x, px2, c1, c2);
									if (comp == CR_EQUAL)
									{							
										sLONG row2x = row2;
										Boolean cont2 = true;
										do
										{
											if (row2x >= WithOther.fNbRows)
											{
												lastrow2x = row2x;
												cont2 = false;
											}
											else
											{
												RecIDType storage2x[30];
												RecIDType* px2x = WithOther.calcIterator(row2x, &storage2x[0]);
												CompareResult comp = CompareRowIds(px1x, px2x, c1, c2);
												if (comp == CR_EQUAL)
												{
													err = result->MergeAndAddRow(px1x, px2x, cfinal, FinalRow);
													row2x++;
												}
												else
												{
													lastrow2x = row2x;
													cont2 = false;
												}
											}
										} while (cont2 && err == VE_OK);
										row1x++;
									}
									else
									{
										cont1 = false;
									}
								}

							} while (cont1 && err == VE_OK);

							row1 = row1x;
							row2 = lastrow2x;
						}
						else
						{
							if (comp == CR_SMALLER)
								row1++;
							else
								row2++;
						}
					}
				}
			}
		}

	}
	catch (...)
	{
		err = ThrowBaseError(memfull, DBaction_BuildingSelection);
	}

	if (err != VE_OK)
	{
		if (result != nil)
			result->Release();
		result = nil;
		err = ThrowBaseError(VE_DB4D_CANNOT_MERGE_SELECTIONS, DBaction_BuildingSelection);
	}

	return result;
}




					// ----------------------------------------------------------


ComplexRechTokenSimpleComp::ComplexRechTokenSimpleComp(Field* xcri, sLONG instance, sLONG comp, ValPtr val, const VCompareOptions& inOptions)
{
	bool checkfornull = false;
	CheckForNullOn(comp, checkfornull);
	xbox_assert(xcri != nil && val != nil);
	TypToken = rc_Line; 
	fOptions = inOptions; 
	SetCompOptionWithOperator(fOptions, comp);
	fIsNeverNull = 2;
	fCheckForNull = checkfornull;
	ch = val;
	cri = xcri;
	xcri->Retain();
	comparaison = comp;
	fNumInstance = instance;
}


ComplexRechTokenSimpleComp::~ComplexRechTokenSimpleComp()
{
	cri->Release();
	if (ch != nil)
		delete ch;
}


void ComplexRechTokenSimpleComp::CheckIfDataIsDirect()
{
	fIsDataDirect = cri->IsDataDirect();
}


			/* -------------------------------- */


ComplexRechTokenScriptComp::ComplexRechTokenScriptComp(DB4DLanguageExpression* inExpression, Table* inTable, sLONG numinstance)
{ 
	xbox_assert(inTable != nil && inExpression != nil);
	TypToken = rc_Script; 
	expression = inExpression;
	expression->Retain();
	fNumInstance = numinstance;
	fTable = inTable;
	fTable->Retain();
}


ComplexRechTokenScriptComp::~ComplexRechTokenScriptComp()
{
	expression->Release();
	fTable->Release();
}




			/* -------------------------------- */


ComplexRechTokenSQLScriptComp::ComplexRechTokenSQLScriptComp(DB4DSQLExpression* inExpression, Table* inTable, sLONG numinstance)
{ 
	xbox_assert(inTable != nil && inExpression != nil);
	TypToken = rc_SQLScript; 
	expression = inExpression;
	expression->Retain();
	fNumInstance = numinstance;
	fTable = inTable;
	fTable->Retain();
	fSQLContext = nil;
}


ComplexRechTokenSQLScriptComp::~ComplexRechTokenSQLScriptComp()
{
	expression->Release();
	fTable->Release();
}


			/* -------------------------------- */


ComplexRechTokenArrayComp::ComplexRechTokenArrayComp(Field* xcri, sLONG instance, sWORD comp, DB4DArrayOfValues* val, const VCompareOptions& inOptions) 
{ 
	TypToken = rc_LineArray; 

	xbox_assert(xcri != nil && val != nil);
	fOptions = inOptions; 
	SetCompOptionWithOperator(fOptions, comp);
	fIsNeverNull = 2;
	values = val;
	values = GenerateConstArrayOfValues(val, xcri->GetTyp(), inOptions);
	//values->Retain();
	cri = xcri;
	xcri->Retain();
	comparaison = comp;
	fNumInstance = instance;
}


ComplexRechTokenArrayComp::~ComplexRechTokenArrayComp()
{
	cri->Release();
	values->Release();
}


void ComplexRechTokenArrayComp::CheckIfDataIsDirect()
{
	fIsDataDirect = cri->IsDataDirect();
}



			/* -------------------------------- */


ComplexRechTokenJoin::ComplexRechTokenJoin(Field* xcri1, sLONG numinstance1, sWORD comp, Field* xcri2, sLONG numinstance2, const VCompareOptions& inOptions, bool leftjoin, bool rightjoin)
{
	xbox_assert(xcri1 != nil && xcri2 != nil);
	TypToken = rc_LineJoin;
	cri1 = xcri1;
	cri1->Retain();
	fNumInstance1 = numinstance1;

	comparaison = comp;

	cri2 = xcri2;
	cri2->Retain();
	fNumInstance2 = numinstance2;
	fOptions = inOptions; 
	fLeftJoin = leftjoin;
	fRightJoin = rightjoin;
	SetCompOptionWithOperator(fOptions, comp);
}


ComplexRechTokenJoin::~ComplexRechTokenJoin()
{
	cri1->Release();
	cri2->Release();
}



			/* -------------------------------- */

ComplexRechTokenComplexSQLScriptComp::ComplexRechTokenComplexSQLScriptComp(DB4DSQLExpression* inExpression, const QueryTargetVector& inTargets)
{
	TypToken = rc_ComplexSQLScript; 
	expression = inExpression;
	expression->Retain();
	for (QueryTargetVector::const_iterator cur = inTargets.begin(), end = inTargets.end(); cur != end; cur++)
	{
		CDB4DTable* xtable = cur->first;
		Table* tt = dynamic_cast<VDB4DTable*>(xtable)->GetTable();
		CQTableDef cqt(tt, cur->second);
		fTargets.Add(cqt);
	}
}

ComplexRechTokenComplexSQLScriptComp::~ComplexRechTokenComplexSQLScriptComp()
{
	expression->Release();
}




										/* --------------------------------------------------- */


ComplexRech::ComplexRech(Table* inSimpleTarget) // result is a simple selection
{
	xbox_assert(inSimpleTarget != nil);
	fOwner = inSimpleTarget->GetOwner();
	fOwner->Retain();
	fResultType = CQResult_Selection;
	fSimpleTarget = inSimpleTarget;
	fSimpleTarget->Retain();
	AddComplexTarget(inSimpleTarget, 0);
}


void ComplexRech::SetSimpleTarget(Table* inSimpleTarget)
{
	xbox_assert(inSimpleTarget != nil);
	fResultType = CQResult_Selection;
	fSimpleTarget = inSimpleTarget;
	fSimpleTarget->Retain();
	AddComplexTarget(inSimpleTarget, 0);
}


ComplexRech::ComplexRech(Base4D* owner) // if result is a complex selection
{
	fSimpleTarget = nil;
	fResultType = CQResult_ComplexSel;
	fOwner = owner;
	fOwner->Retain();
}


VError ComplexRech::SetComplexTarget(const QueryTargetVector& inTargets)
{
	try
	{
		copy(inTargets.begin(), inTargets.end(), insert_iterator<CQTableDefVector>(fComplexTarget.fTargets, fComplexTarget.fTargets.begin()));
		return VE_OK;
	}
	catch (...)
	{
		return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
	}
}



inline void dispose_token(ComplexRechToken* tok)
{
	delete tok;
}


ComplexRech::~ComplexRech()
{
	for_each(fTokens.begin(), fTokens.end(), dispose_token);
	if (fSimpleTarget != nil)
		fSimpleTarget->Release();
	fOwner->Release();
}



VError ComplexRech::AddComplexTarget(Table* inTable, sLONG inNumInstance)
{
	CQTableDef cqt(inTable, inNumInstance);
	if (fComplexTarget.Add(cqt))
		return VE_OK;
	else
		return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);

}


VError ComplexRech::AddSearchExpression(DB4DLanguageExpression* inExpression, Table* inTable, sLONG inNumInstance)
{
	ComplexRechTokenScriptComp* rt = new ComplexRechTokenScriptComp(inExpression, inTable, inNumInstance);
	if (rt == nil)
		return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
	else
	{
		try
		{
			fTokens.push_back(rt);
		}
		catch (...)
		{
			return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
		}
	}
	return VE_OK;
}


VError ComplexRech::AddSearchSQLExpression(DB4DSQLExpression* inExpression, Table* inTable, sLONG inNumInstance)
{
	ComplexRechTokenSQLScriptComp* rt = new ComplexRechTokenSQLScriptComp(inExpression, inTable, inNumInstance);
	if (rt == nil)
		return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
	else
	{
		try
		{
			fTokens.push_back(rt);
		}
		catch (...)
		{
			return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
		}
	}
	return VE_OK;
}


VError ComplexRech::AddSearchComplexSQLExpression(DB4DSQLExpression* inExpression, const QueryTargetVector& inTargets)
{
	ComplexRechTokenComplexSQLScriptComp* rt = new ComplexRechTokenComplexSQLScriptComp(inExpression, inTargets);
	if (rt == nil)
		return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
	else
	{
		try
		{
			fTokens.push_back(rt);
		}
		catch (...)
		{
			return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
		}
	}
	return VE_OK;
}


VError ComplexRech::AddSearchSimple(Field* cri, sLONG numinstance, sLONG comp, const VValueSingle* ValueToCompare, Boolean isDiacritic)
{
	VCompareOptions options;
	options.SetDiacritical(isDiacritic);
	return AddSearchSimple(cri, numinstance, comp, ValueToCompare, options);
}


VError ComplexRech::AddSearchSimple(Field* cri, sLONG numinstance, sLONG comp, const VValueSingle* ValueToCompare, const VCompareOptions& inOptions)
{
	VValueSingle* cv = ValueToCompare->ConvertTo(cri->GetTyp());
	ComplexRechTokenSimpleComp* rt = new ComplexRechTokenSimpleComp(cri, numinstance, comp, cv, inOptions);
	if (rt == nil)
		return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
	else
	{
		try
		{
			fTokens.push_back(rt);
		}
		catch (...)
		{
			return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
		}
	}
	return VE_OK;
}


VError ComplexRech::AddSearchArray(Field* cri, sLONG numinstance, sLONG comp, DB4DArrayOfValues *Values, Boolean isDiacritic)
{
	VCompareOptions options;
	options.SetDiacritical(isDiacritic);
	return AddSearchArray(cri, numinstance, comp, Values, options);
}


VError ComplexRech::AddSearchArray(Field* cri, sLONG numinstance, sLONG comp, DB4DArrayOfValues *Values, const VCompareOptions& inOptions)
{
	ComplexRechTokenArrayComp* rt = new ComplexRechTokenArrayComp(cri, numinstance, comp, Values, inOptions);
	if (rt == nil)
		return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
	else
	{
		try
		{
			fTokens.push_back(rt);
		}
		catch (...)
		{
			return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
		}
	}
	return VE_OK;
}


VError ComplexRech::AddSearchJoin(Field* cri, sLONG numinstance, sLONG comp, Field* OtherCri, sLONG numOtherInstance, Boolean isdiacritic, bool leftjoin, bool rightjoin)
{
	VCompareOptions options;
	options.SetDiacritical(isdiacritic);
	options.SetLike(false);
	options.SetBeginsWith(false);
	return AddSearchJoin(cri, numinstance, comp, OtherCri, numOtherInstance, options, leftjoin, rightjoin);
}


VError ComplexRech::AddSearchJoin(Field* cri, sLONG numinstance, sLONG comp, Field* OtherCri, sLONG numOtherInstance, const VCompareOptions& inOptions, bool leftjoin, bool rightjoin)
{
	ComplexRechTokenJoin* rt = new ComplexRechTokenJoin(cri, numinstance, comp, OtherCri, numOtherInstance, inOptions, leftjoin, rightjoin);
	if (rt == nil)
		return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
	else
	{
		try
		{
			fTokens.push_back(rt);
		}
		catch (...)
		{
			return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
		}
	}
	return VE_OK;
}


VError ComplexRech::AddSearchBoolOper(sLONG BoolOper)
{
	ComplexRechTokenOper* rt = new ComplexRechTokenOper(BoolOper);
	if (rt == nil)
		return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
	else
	{
		try
		{
			if (BoolOper == DB4D_Except)
			{
				rt->BoolLogic = DB4D_And;
			}
			fTokens.push_back(rt);
			if (BoolOper == DB4D_Except)
			{
				ComplexRechTokenNot* rtnot = new ComplexRechTokenNot;
				fTokens.push_back(rtnot);
			}
		}
		catch (...)
		{
			return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
		}
	}
	return VE_OK;
}


VError ComplexRech::AddGenericToken(sWORD whatToken)
{
	ComplexRechToken* rt = new ComplexRechToken(whatToken);
	if (rt == nil)
		return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
	else
	{
		try
		{
			fTokens.push_back(rt);
		}
		catch (...)
		{
			return ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
		}
	}
	return VE_OK;
}


VError ComplexRech::AddSearchNotOperator()
{
	return AddGenericToken(rc_Not);
}


VError ComplexRech::AddSearchOpenParenthesis()
{
	return AddGenericToken(rc_ParentG);
}


VError ComplexRech::AddSearchCloseParenthesis()
{
	return AddGenericToken(rc_ParentD);
}


ComplexRechToken* ComplexRech::GetNextToken() 
{ 
	if (curToken != fTokens.end())
	{
		ComplexRechToken* res = *curToken;
		curToken++;
		return res;
	}
	else 
		return nil; 
}



static Boolean isfullterminator(UniChar c)
{
	if (c == '(' || c == ')')
		return true;
	else
		return false;
}

static Boolean isterminator(UniChar c)
{
	if (c == '<' || c == '>' || c == '=' || c == '(' || c == ')' || c == '#' || c == '&' || c == '^' || c == '|' || c == '[' || c == ',')
		return true;
	else
		return false;
}

static void removeextraspace(VString& s)
{
	Boolean cont = true;

	sLONG len = s.GetLength();
	while (len>0 && cont)
	{
		if (s[len-1] == 32)
			len--;
		else
			cont = false;
	}
	s.Truncate(len);
}

/*
static void GetNextWord(const VString& input, sLONG& curpos, VString& result)
{
	UniChar c;
	result.Clear();
	Boolean first = true;
	Boolean isregularword;
	Boolean insidequotes = false;

	do {
		if (curpos<input.GetLength())
		{
			c = input[curpos];
			if (c == 13 || c == 10)
				c = 32;
			if (insidequotes && c == 39)
			{
				curpos++;
				c = 0;
			}
			else
			{
				if (first && c == 32)
				{
					curpos++;
				}
				else
				{
					if (!insidequotes && isterminator(c))
					{
						if (first)
						{
							isregularword = false;
							result.AppendUniChar(c);
							curpos++;
							first = false;
						}
						else
						{
							if (isfullterminator(c))
							{
								c = 0;
							}
							else
							{
								if (isregularword)
								{
									c = 0;
								}
								else
								{
									result.AppendUniChar(c);
									curpos++;
								}
							}
						}
					}
					else
					{
						if (first)
						{
							if (c == 39)
							{
								insidequotes = true;
							}
							else
							{
								result.AppendUniChar(c);
							}
							isregularword = true;
							curpos++;
							first = false;
						}
						else
						{
							if (isregularword)
							{
								result.AppendUniChar(c);
								curpos++;
							}
							else
							{
								c = 0;
							}
						}
					}
				}
			}
		}
		else
			c = 0;

	} while(c != 0);
}
*/

VError ComplexRech::BuildTargetsFromString(const VString& inTargetsText)
{
	VError err = VE_OK;
	sLONG curpos = 0;
	Boolean stop = false;
	VStr<80> s;
	do {
		GetNextWord(inTargetsText, curpos, s);
		if (s.IsEmpty())
			stop = true;
		else
		{
			removeextraspace(s);
			sLONG numinstance = 0;
			sLONG p = s.FindUniChar('{');
			if (p != 0)
			{
				VString sinstance = s;
				s.Remove(p, s.GetLength()-p+1);
				sinstance.Remove(1,p);
				sLONG len = sinstance.GetLength();
				if (len > 0)
				{
					sinstance.Remove(len-1,1);
					numinstance = sinstance.GetLong();
				}

			}
			Table* tt = fOwner->FindAndRetainTableRef(s);
			if (tt != nil)
			{
				AddComplexTarget(tt, numinstance);
				tt->Release();
			}
		}
	} while(!stop && err == VE_OK);

	return err;
}


VError ComplexRech::BuildFromString(const VString& input)
{
	sLONG curpos = 0;
	Boolean stop = false;
	VStr<80> s;
	Boolean waitforfield = true;
	Boolean waitforoper = false;
	Boolean waitforvalue = false;
	Boolean waitforcunj = false;
	VError err = VE_OK;
	Field* cri = nil;
	Field* cri2 = nil;
	sLONG numinstance1 = 0, numinstance2 = 0;
	DB4DComparator oper = (DB4DComparator)0;
	ValPtr cv = nil;
	CreVValue_Code Code;

	do {
		GetNextWord(input, curpos, s);
		if (s.IsEmpty())
			stop = true;
		else
		{
			removeextraspace(s);
			if (waitforfield)
			{
				cri = nil;
				cri2 = nil;
				if (s == L"not" || s == L"!")
				{
					AddSearchNotOperator();
				}
				else
				{
					if (s == L"(")
					{
						AddSearchOpenParenthesis();
					}
					else
					{
						VString	stable;
						VString sfield;
						sLONG p = s.FindUniChar(CHAR_FULL_STOP);

						if (p != 0)
						{
							stable = s;
							sfield = s;
							stable.Remove(p,stable.GetLength()-p+1);
							sfield.Remove(1,p);
							numinstance1 = 0;
							p = sfield.FindUniChar('{');
							if (p != 0)
							{
								VString sinstance = sfield;
								sfield.Remove(p, sfield.GetLength()-p+1);
								sinstance.Remove(1,p);
								sLONG len = sinstance.GetLength();
								if (len > 0)
								{
									sinstance.Remove(len-1,1);
									numinstance1 = sinstance.GetLong();
								}

							}
							cri = fOwner->FindAndRetainFieldRef(stable, sfield);
						}

						if (cri == nil)
						{
							err = ThrowBaseError(VE_DB4D_WRONGFIELDREF, DBaction_BuildingComplexQuery);
						}
						else
						{
							waitforfield = false;
							waitforoper = true;
						}
					}
				}
			}
			else
			{
				if (waitforoper)
				{
					oper = (DB4DComparator)0;
					{
						if (s == L"=")
							oper = DB4D_Like;
						else if (s == L"==")
							oper = DB4D_Equal;
						else if (s == L"#")
							oper = DB4D_NotLike;
						else if (s == L"!=")
							oper = DB4D_NotEqual;
						else if (s == L"##")
							oper = DB4D_NotEqual;
						else if (s == L">")
							oper = DB4D_Greater;
						else if (s == L">=")
							oper = DB4D_GreaterOrEqual;
						else if (s == L"<")
							oper = DB4D_Lower;
						else if (s == L"<=")
							oper = DB4D_LowerOrEqual;
						else if (s == L"[=")
							oper = DB4D_BeginsWith;
						else if (s == L"]=")
							oper = DB4D_EndsWith;
					}

					if (oper == 0)
					{
						err = ThrowBaseError(VE_DB4D_WRONG_COMP_OPERATOR, DBaction_BuildingComplexQuery);
					}
					else
					{
						waitforoper = false;
						waitforvalue = true;
					}

				}
				else
				{
					if (waitforvalue)
					{
						VString	stable;
						VString sfield;
						sLONG p = s.FindUniChar(CHAR_FULL_STOP);

						if (p != 0)
						{
							stable = s;
							sfield = s;
							stable.Remove(p,stable.GetLength()-p+1);
							sfield.Remove(1,p);
							numinstance2 = 0;
							p = sfield.FindUniChar('{');
							if (p != 0)
							{
								VString sinstance = sfield;
								sfield.Remove(p, sfield.GetLength()-p+1);
								sinstance.Remove(1,p);
								sLONG len = sinstance.GetLength();
								if (len > 0)
								{
									sinstance.Remove(len-1,1);
									numinstance1 = sinstance.GetLong();
								}

							}
							cri2 = fOwner->FindAndRetainFieldRef(stable, sfield);
						}

						if (cri2 == nil)
						{
							/*
							Code=FindCV(cri->GetTyp());
							if (Code != nil)
							{
							*/
								//cv = (*Code)(destFile->GetDF(),cri->GetLimitingLen(),nil,false,false,NULL);
								cv = new VString;
								if (cv == nil)
									err = ThrowBaseError(memfull, DBaction_BuildingComplexQuery);
								else
								{
									cv->FromString(s);
									AddSearchSimple(cri, numinstance1, oper, cv);
									cri->Release();
									cri = nil;
									delete cv;
									cv = nil;
									oper = (DB4DComparator)0;
									waitforvalue = false;
									waitforcunj = true;
								}
							/*
							}
							else
								err = VE_DB4D_FIELDDEFCODEMISSING;
							*/
						}
						else
						{
							AddSearchJoin(cri, numinstance1, oper, cri2, numinstance2);
							cri->Release();
							cri2->Release();
							cri = nil;
							cri2 = nil;
							cv = nil;
							oper = (DB4DComparator)0;
							waitforvalue = false;
							waitforcunj = true;
						}
					}
					else
					{
						if (waitforcunj)
						{
							if (s == L")")
							{
								AddSearchCloseParenthesis();
							}
							else
							{
								sLONG cunj = 0;
								{
									if (s == L"&")
										cunj = DB4D_And;
									else if (s == L"|")
										cunj = DB4D_OR;
									else if (s == L"^")
										cunj = DB4D_Except;
								}
								if (cunj == 0)
									err = VE_DB4D_WRONG_COMP_OPERATOR;
								else
								{
									AddSearchBoolOper(cunj);
									waitforcunj = false;
									waitforfield = true;
								}
							}
						}
					}
				}
			}

		}

	} while(!stop && err == VE_OK);

	if (cri != nil)
		cri->Release();

	if (cv != nil)
		delete cv;

	return err;
}









			/* ----------------------------------------------------------- */

static VString BuildStringFromCrit(const Field* cri, sLONG numinstance)
{
	VString s, s2;

	if (cri == nil)
		s = L"<<Null Field>>";
	else
	{
		cri->GetOwner()->GetName(s);
		s += L".";
		cri->GetName(s2);
		s += s2;
		if (numinstance != 0)
			s += L"{"+ToString(numinstance)+L"}";
	}

	return s;
}


static VString BuildStringFromTarget(const CQTableDef& target)
{
	VString s, s2;

	target.GetTable()->GetName(s);
	if (target.GetInstance() != 0)
		s += L"{"+ToString(target.GetInstance())+L"}";

	return s;
}


VError QueryNode::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	VString slevel;
	fOwner->FillLevelString(slevel, level);
	descriptor += slevel;
	return VE_OK;
}




CQTargetsDef* SimpleQueryNode::GenerateTargets(VError& err)
{
	err = VE_OK;
	CQTargetsDef* result = new CQTargetsDef;
	if (result == nil)
		err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
	else
	{
		if (!result->Add(fTarget))
		{
			err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
			result->Release();
			result = nil;
		}
	}
	return result;
}


VError SimpleQueryNode::AddToTargets(SetOfTargets& fullTarget)
{
	try
	{
		fullTarget.insert(fTarget);
		return VE_OK;
	}
	catch (...)
	{
		return ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
	}
}



VError SimpleQueryNode::Perform(Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, VValueBag* curdesc)
{
	Bittab* b;
	VError err = VE_OK;

	uLONG start = fOwner->GetStartingTime();

	b = fTarget.GetTable()->GetDF()->Search(err, this, filtre, InProgress, context, HowToLock, exceptions, limit, Nulls);

	if (fOwner->IsQueryDescribed())
	{
		VString s;
		FullyDescribe(s);
		fOwner->AddToDescription(curdesc, s, start, b);
	}

	*super = b;
	return(err);
}


VError SimpleQueryNode::PerformComplex(ComplexSelection* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, ComplexSelection* &outSel, DB4D_Way_of_Locking HowToLock, sLONG limit, VValueBag* curdesc)
{
	VError err = VE_OK;
	Bittab* result;
	Bittab* bitfiltre = nil;
	if (filtre != nil)
	{
		bitfiltre = filtre->GenereBittab(0, context, err);
	}
	Bittab nulls;
	if (err == VE_OK)
	{
		if (IsConst())
		{
			result = new Bittab();
		}
		else
			err = Perform(&result, bitfiltre, InProgress, context, HowToLock, nil, limit, nulls, curdesc);
	}
	ReleaseRefCountable(&bitfiltre);
	if (err == VE_OK)
	{
		outSel = new ComplexSelection(fTarget);
		if (outSel != nil)
		{
			outSel->PutInCache();
			err = outSel->FromBittab(0, context, result, true);
			if (err != VE_OK)
			{
				outSel->Release();
				outSel = nil;
			}
		}
	}
	return err;
}



			/* -------------------------- */

uBOOL SimpleQueryNodeSeq::PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
{
	uBOOL ok = false;
	ValPtr cv;
	VError err;
	ValueKind vk;

	cv=nil;

	cv=curfic->GetFieldValue(rt->cri, err, false, true);

	if (cv!=nil)
	{
		if (cv->IsNull())
		{
			if (rt->fCheckForNull)
			{
				ok = (rt->comparaison == DB4D_Like) || (rt->comparaison == DB4D_Equal);
			}
			else
				ok = 2;
		}
		else
		{
			if (rt->fCheckForNull)
			{
				ok = !((rt->comparaison == DB4D_Like) || (rt->comparaison == DB4D_Equal));
			}
			else
			{
				switch (rt->comparaison)
				{
					case DB4D_BeginsWith:
						ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) == CR_EQUAL);
						break;

					case DB4D_Equal:
						ok = cv->EqualToSameKindWithOptions(rt->ch, rt->fOptions);
						break;

					case DB4D_Like:
						ok = cv->EqualToSameKindWithOptions(rt->ch, rt->fOptions);
						break;

					case DB4D_NotEqual:
						ok = !(cv->EqualToSameKindWithOptions(rt->ch, rt->fOptions));
						break;

					case DB4D_NotLike:
						ok=! (cv->EqualToSameKindWithOptions(rt->ch, rt->fOptions));
						break;

					case DB4D_Greater_Like:
					case DB4D_Greater:
						ok=(cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions)==CR_BIGGER);
						break;

					case DB4D_GreaterOrEqual_Like:
					case DB4D_GreaterOrEqual:
						ok=(cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions)!=CR_SMALLER);
						break;

					case DB4D_Lower_Like:
					case DB4D_Lower:
						ok=(cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions)==CR_SMALLER);
						break;

					case DB4D_LowerOrEqual_Like:
					case DB4D_LowerOrEqual:
						ok=(cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions)!=CR_BIGGER);
						break;

					case DB4D_Contains_KeyWord:	
					case DB4D_Contains_KeyWord_Like:
					case DB4D_Contains_KeyWord_BeginingWith:
						vk = cv->GetValueKind();
						if (vk == VK_STRING || vk == VK_TEXT || vk == VK_STRING_UTF8 || vk == VK_TEXT_UTF8) 
						{
							ValueKind vk2 = rt->ch->GetValueKind();
							if (vk2 == VK_STRING || vk2 == VK_TEXT || vk2 == VK_STRING_UTF8 || vk2 == VK_TEXT_UTF8)
							{
								if (cv != nil)
								{
									VError err2;
									ok = VDBMgr::ContainsKeyword( *(VString*)(cv), *(VString*)(rt->ch), rt->fOptions, err2);
								}
							}
						}
						break;
				}
			}
		}

	} // du if cv!=nil
	else
	{
		if (rt->fCheckForNull)
		{
			ok = (rt->comparaison == DB4D_Like) || (rt->comparaison == DB4D_Equal);
		}
		else
			ok = 2;
	}

	return(ok);

}


uBOOL SimpleQueryNodeSeq::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
{
	uBOOL ok = false;
	sLONG typondisk;
	void* data = (void*)(ficD->GetDataPtrForQuery(rt->cri->GetPosInRec(), &typondisk, false));
	rt->fIsDataDirect = rt->cri->IsDataDirect();
	uLONG tempdata[4];

	if (data == nil)
	{
		if (rt->fIsNeverNull == 2)
		{
			rt->fIsNeverNull = rt->cri->IsNeverNull();
		}
		if (rt->fIsNeverNull)
		{
			data = &tempdata;
			typondisk = (sLONG)rt->ch->GetTrueValueKind();
			std::fill(&tempdata[0], &tempdata[4], 0);
		}
	}

	if (data!=nil)
	{
		if (rt->fCheckForNull)
		{
			ok = !((rt->comparaison == DB4D_Like) || (rt->comparaison == DB4D_Equal));
		}
		else
		{
			if (rt->ch->IsNull())
			{
				ok = 2;
			}
			else
			{
				sLONG rechtyp = (sLONG)rt->ch->GetTrueValueKind();
				if (rt->comparaison == DB4D_Contains_KeyWord || rt->comparaison == DB4D_Contains_KeyWord_Like || rt->comparaison == DB4D_Contains_KeyWord_BeginingWith)
				{
					if (rechtyp == VK_STRING || rechtyp == VK_TEXT || rechtyp == VK_STRING_UTF8 || rechtyp == VK_TEXT_UTF8) 
					{
						ValPtr cv = NewValPtr(VK_STRING, data, typondisk, ficD->GetOwner(), context);
						if (cv != nil)
						{
							VError err2;
							ok = VDBMgr::ContainsKeyword( *(VString*)(cv), *(VString*)(rt->ch), rt->fOptions, err2);
							delete cv;
						}

					}

				}
				else
				{
					if (typondisk == rechtyp && rt->fIsDataDirect)
					{
						switch (rt->comparaison)
						{
						case DB4D_BeginsWith:
							ok = rt->ch->Swap_CompareToSameKindPtrWithOptions(data, rt->fOptions) == CR_EQUAL ;
							break;

						case DB4D_Equal:
							ok = rt->ch->EqualToSameKindPtrWithOptions(data, rt->fOptions);
							break;

						case DB4D_Like:
							ok = rt->ch->Swap_CompareToSameKindPtrWithOptions(data, rt->fOptions) == CR_EQUAL;
							break;

						case DB4D_NotEqual:
							ok = ! rt->ch->EqualToSameKindPtrWithOptions(data, rt->fOptions);
							break;

						case DB4D_NotLike:
							ok = rt->ch->Swap_CompareToSameKindPtrWithOptions(data, rt->fOptions) != CR_EQUAL;
							break;

						case DB4D_Greater:
							ok = rt->ch->CompareToSameKindPtrWithOptions(data, rt->fOptions) == CR_SMALLER;
							break;

						case DB4D_GreaterOrEqual:
							ok = rt->ch->CompareToSameKindPtrWithOptions(data, rt->fOptions) != CR_BIGGER;
							break;

						case DB4D_Lower:
							ok = rt->ch->CompareToSameKindPtrWithOptions(data, rt->fOptions) == CR_BIGGER;
							break;

						case DB4D_LowerOrEqual:
							ok = rt->ch->CompareToSameKindPtrWithOptions(data, rt->fOptions) != CR_SMALLER;
							break;

						case DB4D_Greater_Like:
							ok = XBOX::VValue::InvertCompResult(rt->ch->Swap_CompareToSameKindPtrWithOptions(data, rt->fOptions)) == CR_SMALLER;
							break;

						case DB4D_GreaterOrEqual_Like:
							ok = XBOX::VValue::InvertCompResult(rt->ch->Swap_CompareToSameKindPtrWithOptions(data, rt->fOptions)) != CR_BIGGER;
							break;

						case DB4D_Lower_Like:
							ok = XBOX::VValue::InvertCompResult(rt->ch->Swap_CompareToSameKindPtrWithOptions(data, rt->fOptions)) == CR_BIGGER;
							break;

						case DB4D_LowerOrEqual_Like:
							ok = XBOX::VValue::InvertCompResult(rt->ch->Swap_CompareToSameKindPtrWithOptions(data, rt->fOptions)) != CR_SMALLER;
							break;

						}
					}
					else
					{
						ValPtr cv = NewValPtr(rechtyp, data, typondisk, ficD->GetOwner(), context);
						if (cv != nil)
						{
							switch (rt->comparaison)
							{
							case DB4D_BeginsWith:
								ok = (cv->CompareToSameKindPtrWithOptions(rt->ch, rt->fOptions) == CR_EQUAL);
								break;

							case DB4D_Equal:
								ok = cv->EqualToSameKindWithOptions(rt->ch, rt->fOptions);
								break;

							case DB4D_Like:
								ok = cv->EqualToSameKindWithOptions(rt->ch, rt->fOptions);
								break;

							case DB4D_NotEqual:
								ok=! (cv->EqualToSameKindWithOptions(rt->ch, rt->fOptions));
								break;

							case DB4D_NotLike:
								ok=! (cv->EqualToSameKindWithOptions(rt->ch, rt->fOptions));
								break;

							case DB4D_Greater_Like:
							case DB4D_Greater:
								ok=(cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions)==CR_BIGGER);
								break;

							case DB4D_GreaterOrEqual_Like:
							case DB4D_GreaterOrEqual:
								ok=(cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions)!=CR_SMALLER);
								break;

							case DB4D_Lower_Like:
							case DB4D_Lower:
								ok=(cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions)==CR_SMALLER);
								break;

							case DB4D_LowerOrEqual_Like:
							case DB4D_LowerOrEqual:
								ok=(cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions)!=CR_BIGGER);
								break;
							}

							delete cv;
						}
					}
				}
			}
		}
	}
	else // quand la valeur sur disque est null
	{
		if (rt->fCheckForNull)
		{
			ok = (rt->comparaison == DB4D_Like) || (rt->comparaison == DB4D_Equal);
		}
		else
			ok = 2;
	}

	return(ok);
}


VError SimpleQueryNodeSeq::Describe(VString& result)
{
	//VString s = L"[seq] : ";
	VString s;
	VString s2;

	s += BuildStringFromCrit(rt->cri, rt->fNumInstance);

	switch (rt->comparaison)
	{
	case DB4D_BeginsWith:
		s2 = L" Begin with ";
		break;

	case DB4D_Equal:
		s2 = L" = ";
		break;

	case DB4D_Like:
		s2 = L" LIKE ";
		break;

	case DB4D_NotEqual:
		s2 = L" # ";
		break;

	case DB4D_NotLike:
		s2 = L" NOT LIKE ";
		break;

	case DB4D_Greater:
	case DB4D_Greater_Like:
		s2 = L" > ";
		break;

	case DB4D_GreaterOrEqual:
	case DB4D_GreaterOrEqual_Like:
		s2 = L" >= ";
		break;

	case DB4D_Lower:
	case DB4D_Lower_Like:
		s2 = L" < ";
		break;

	case DB4D_LowerOrEqual:
	case DB4D_LowerOrEqual_Like:
		s2 = L" <= ";
		break;

	case DB4D_Contains_KeyWord:
		s2 = L" contains ";
		break;

	case DB4D_Contains_KeyWord_Like:
		s2 = L" contains like ";
		break;

	default:
		s2 = L" <Nop> ";
		break;
	}

	s += s2;

	if (rt->ch != nil && !rt->fCheckForNull)
		s2.FromValue(*(rt->ch));
	else
		s2 = L"<Null>";

	s += s2;

	result = s;

	return VE_OK;
}


VError SimpleQueryNodeSeq::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	VString s;

	QueryNode::AddToQueryDescriptor(descriptor, level);

	Describe(s);
	s += L"\n";
	descriptor += s;

	return VE_OK;
}

			/* -------------------------- */


VError SimpleQueryNodeArraySeq::Describe(VString& result)
{
	//VString s = L"[array seq] : ";
	VString s;
	VString s2;

	s += BuildStringFromCrit(rt->cri, rt->fNumInstance);

	switch (rt->comparaison)
	{
	case DB4D_BeginsWith:
		s2 = L" Begin with ";
		break;

	case DB4D_Equal:
		s2 = L" = ";
		break;

	case DB4D_Like:
		s2 = L" LIKE ";
		break;

	case DB4D_NotEqual:
		s2 = L" # ";
		break;

	case DB4D_NotLike:
		s2 = L" NOT LIKE ";
		break;

	case DB4D_Greater:
	case DB4D_Greater_Like:
		s2 = L" > ";
		break;

	case DB4D_GreaterOrEqual:
	case DB4D_GreaterOrEqual_Like:
		s2 = L" >= ";
		break;

	case DB4D_Lower:
	case DB4D_Lower_Like:
		s2 = L" < ";
		break;

	case DB4D_LowerOrEqual:
	case DB4D_LowerOrEqual_Like:
		s2 = L" <= ";
		break;

	case DB4D_Contains_KeyWord:
		s2 = L" contains ";
		break;

	case DB4D_Contains_KeyWord_Like:
		s2 = L" contains like ";
		break;

	default:
		s2 = L" <Nop> ";
		break;
	}

	s += s2;

	s2 = L"<Array of Values>";

	s += s2;

	result = s;

	return VE_OK;
}


VError SimpleQueryNodeArraySeq::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	VString s;

	QueryNode::AddToQueryDescriptor(descriptor, level);

	Describe(s);
	s += L"\n";
	descriptor += s;

	return VE_OK;
}


uBOOL SimpleQueryNodeArraySeq::PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
{
	uBOOL ok = false;
	ValPtr cv;
	VError err;

	cv=nil;
	cv=curfic->GetFieldValue(rt->cri, err, false, true);

	if (cv!=nil)
	{
		rt->fOptions.SetBeginsWith(rt->comparaison == DB4D_BeginsWith);
		rt->fOptions.SetLike(rt->comparaison == DB4D_Like);
		ok = rt->values->Find(*cv, rt->fOptions);
	} // du if cv!=nil

	return(ok);
}


uBOOL SimpleQueryNodeArraySeq::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
{
	uBOOL ok = false;
	sLONG typondisk;
	void* data = (void*)(ficD->GetDataPtrForQuery(rt->cri->GetPosInRec(), &typondisk, false));
	rt->fIsDataDirect = rt->cri->IsDataDirect();
	uLONG tempdata[4];

	if (data == nil)
	{
		if (rt->fIsNeverNull == 2)
		{
			rt->fIsNeverNull = rt->cri->IsNeverNull();
		}
		if (rt->fIsNeverNull)
		{
			data = &tempdata;
			typondisk = rechtyp;
			std::fill(&tempdata[0], &tempdata[4], 0);
		}
	}

	if (data!=nil)
	{
		if (rechtyp == -1)
		{
			rechtyp = rt->cri->GetTyp();
		}

		if (typondisk == rechtyp && rt->fIsDataDirect)
		{
			ok = rt->values->FindWithDataPtr(data, rt->fOptions);
		}
		else
		{
			ValPtr cv = NewValPtr(rechtyp, data, typondisk, ficD->GetOwner(), context);
			if (cv != nil)
			{
				ok = rt->values->Find(*cv, rt->fOptions);
				delete cv;
			}
		}
	} 

	return(ok);

}


			/* -------------------------- */


VError SimpleQueryNodeLangExpression::Describe(VString& result)
{
	VString s = L"([Script]: ";

	VString s2;
	if (rt->expression != nil)
		rt->expression->GetDescription(s2);

	s += s2;
	s += L")";

	result = s;

	return VE_OK;
}


VError SimpleQueryNodeLangExpression::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	VString s;
	QueryNode::AddToQueryDescriptor(descriptor, level);

	Describe(s);
	s += L"\n";

	descriptor += s;

	return VE_OK;
}


uBOOL SimpleQueryNodeLangExpression::PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
{
	//## tbd
	return false;
}


uBOOL SimpleQueryNodeLangExpression::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
{
	//## tbd
	return false;
}


			/* -------------------------- */


VError SimpleQueryNodeSQLExpression::Describe(VString& result)
{
	VString s = L"([SQL Expression]: ";

	VString s2;
	if (rt->expression != nil)
		rt->expression->GetDescription(s2);

	s += s2;
	s += L")";

	result = s;

	return VE_OK;
}


VError SimpleQueryNodeSQLExpression::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	VString s;
	QueryNode::AddToQueryDescriptor(descriptor, level);

	Describe(s);
	s += L"\n";

	descriptor += s;

	return VE_OK;
}


uBOOL SimpleQueryNodeSQLExpression::PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
{
	//## tbd
	return false;
}


uBOOL SimpleQueryNodeSQLExpression::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
{
	VError err = VE_OK;
	uBOOL result = rt->expression->Execute(context->GetEncapsuleur(), rt->fSQLContext, (void*)ficD, err);

	if (err == VE_OK)
		return result;
	else
		return 4;
}


			/* -------------------------- */


SimpleQueryNodeIndex::~SimpleQueryNodeIndex()
{
	if (cle1 != cle2)
	{
		if (cle2 != nil) fInd->FreeKey(cle2);
	}
	if (cle1 != nil) fInd->FreeKey(cle1);

	fInd->ReleaseValid();
	fInd->Release();
}


VError SimpleQueryNodeIndex::Perform(Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, VValueBag* curdesc)
{
	Bittab *b = nil;
	VError err = VE_OK;
	bool checknulls = false;

	if (rt != nil && rt->fCheckForNull)
	{
		checknulls = true;
	}

	if ((CountDiskAccess()/10) > CalculateNBDiskAccessForSeq(filtre, context, CountBlobSources()))
	{
		SimpleQueryNode* seq = (SimpleQueryNode*)fOwner->xTransformSeq(this, true, err, context);
		if (seq != nil)
		{
			seq->AdjusteIntl(GetContextIntl(context));
			err = seq->Perform(super, filtre, InProgress, context, HowToLock, exceptions, limit, Nulls, curdesc);
			seq->Release();
		}
	}
	else
	{
		//## pour tester performance
		//InProgress = nil;

		uLONG start = fOwner->GetStartingTime();

		fInd->Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		ComplexRechTokenSimpleComp* xrt = rt;
		if (xrt == nil)
		{
			xbox_assert(!fNodes.empty());
			xrt = fNodes.begin()->Get()->rt;
		}

		if (checknulls)
		{
			BitSel* sel = fInd->GetHeader()->RetainNulls(curstack, err, context);
			if (err == VE_OK)
			{
				if (sel == nil)
					b = new Bittab();
				else
				{
					b = sel->GetBittab()->Clone(err);
				}
			}

		}
		else if (fParseAllIndex)
		{
			b = fInd->ParseAll(cle1, context, err, xrt->fOptions, InProgress);
		}
		else
		{
			b=fInd->Fourche(cle1,xstrict1,cle2,xstrict2, context, err, InProgress, xrt->fOptions, nil, nil);
		}
		if (b!=nil && err == VE_OK)
		{
			if (inverse)
			{
				fInd->Close();
				err=b->Invert();
				fTarget.GetTable()->GetDF()->TakeOffBits(b, context);

				if (!checknulls)
				{
					fInd->Open(index_read);
					BitSel* sel = fInd->GetHeader()->RetainNulls(curstack, err, context);
					if (sel != nil)
					{
						err = b->moins(sel->GetBittab(), false);
						if (err == VE_OK)
							err = Nulls.Or(sel->GetBittab(), true);
						sel->Release();
					}
					fInd->Close();
				}
			}
			else
			{
				if (!checknulls)
				{
					BitSel* sel = fInd->GetHeader()->RetainNulls(curstack, err, context);
					if (sel != nil)
					{
						err = Nulls.Or(sel->GetBittab(), true);
						sel->Release();
					}
					fInd->Close();
				}
			}

		}
		else
			fInd->Close();


		if (err == VE_OK)
		{
			if (filtre != nil) 
				err = b->And(filtre);
		}

		if (HowToLock == DB4D_Keep_Lock_With_Transaction && err == VE_OK)
		{
			if (context != nil)
			{
				Transaction* trans = context->GetCurrentTransaction();
				if (trans != nil)
				{
					err = trans->TryToLockSel(fTarget.GetTable()->GetDF(), b, exceptions);
				}
			}
		}

		if (fOwner->IsQueryDescribed())
		{
			VString s;
			FullyDescribe(s);
			fOwner->AddToDescription(curdesc, s, start, b);
		}


		if (err != VE_OK)
		{
			ReleaseRefCountable(&b);
		}

		*super = b;
	}

	return(err);
}


uBOOL SimpleQueryNodeIndex::PeutFourche(void)
{
	if ( (cle1==nil) || (cle2==nil) )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}


uBOOL SimpleQueryNodeIndex::PeutFourche(SimpleQueryNodeIndex* autre)
{
	uBOOL ok;

	ok=false;
	if (fInd->MayBeSorted() && fInd==autre->fInd)
	{
		if (   ((cle1==nil) && (autre->cle1!=nil) && (autre->cle2==nil))
			|| ((cle2==nil) && (autre->cle2!=nil) && (autre->cle1==nil))  )
		{
			ok=true;
			rt2=autre->rt;
			fIncludingFork = autre->fNodeSeq;
			isfourche=true;
			if (cle1==nil)
			{
				cle1=autre->cle1;
				autre->cle1=nil;
				xstrict1=autre->xstrict1;
			}
			else
			{
				cle2=autre->cle2;
				autre->cle2=nil;
				xstrict2=autre->xstrict2;
			}
		}
	}

	return(ok);
}


VError SimpleQueryNodeIndex::SubBuildCompFrom(sLONG comp, BTitemIndex *val)
{
	switch (comp)
	{
		case DB4D_Equal:
		case DB4D_NotEqual:
		case DB4D_Like:
		case DB4D_NotLike:
		case DB4D_Contains_KeyWord:
		case DB4D_DoesntContain_KeyWord:
		case DB4D_Contains_KeyWord_Like:
		case DB4D_Doesnt_Contain_KeyWord_Like:
			cle1=val;
			cle2=val;
			xstrict1=false;
			xstrict2=false;
			inverse=(comp==DB4D_NotEqual || comp==DB4D_DoesntContain_KeyWord 
				|| comp == DB4D_NotLike || comp == DB4D_Doesnt_Contain_KeyWord_Like);

			fOptions.SetLike(comp == DB4D_NotLike || comp == DB4D_Like 
				|| comp == DB4D_Contains_KeyWord_Like || comp == DB4D_Doesnt_Contain_KeyWord_Like);
			break;

		case DB4D_Greater:
		case DB4D_GreaterOrEqual:
		case DB4D_Greater_Like:
		case DB4D_GreaterOrEqual_Like:
			cle1=val;
			xstrict1=(comp==DB4D_Greater || comp==DB4D_Greater_Like);
			//cle2=nil;
			//xstrict2=false;
			inverse=false;
			fOptions.SetLike(comp == DB4D_Greater_Like || comp == DB4D_GreaterOrEqual_Like);
			break;

		case DB4D_Lower:
		case DB4D_LowerOrEqual:
		case DB4D_Lower_Like:
		case DB4D_LowerOrEqual_Like:
			//cle1=nil;
			//xstrict1=false;
			cle2=val;
			xstrict2=(comp==DB4D_Lower || comp==DB4D_Lower_Like);
			inverse=false;
			fOptions.SetLike(comp == DB4D_LowerOrEqual_Like || comp == DB4D_Lower_Like);
			break;

		case DB4D_Contains_KeyWord_BeginingWith:
		case DB4D_BeginsWith:
			cle1=val;
			cle2=val;
			xstrict1=false;
			xstrict2=false;
			fOptions.SetBeginsWith(true);
			break;
	}

	return VE_OK;
}


VError SimpleQueryNodeIndex::BuildFrom(ComplexRechTokenSimpleComp *xrt)
{
	VError err = VE_OK;
	BTitemIndex *val;

	fOptions = xrt->fOptions;
	fOptions.SetBeginsWith(false);
	fOptions.SetLike(false);
	rt = xrt;
	rt2 = nil;
	cle1 = nil;
	cle2 = nil;
	xstrict1 = false;
	xstrict2 = false;

	val=((IndexInfoFromField*)fInd)->BuildKeyFromVValue(rt->ch, err);
	SubBuildCompFrom(rt->comparaison, val);
	return(err);
}


VError SimpleQueryNodeIndex::Add(SimpleQueryNodeSeq* nodeseq)
{
	try
	{
		fNodes.push_back(nodeseq);
		return VE_OK;
	}
	catch (...)
	{
		return ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
	}
}


VError SimpleQueryNodeIndex::BuildFromMultiple(SimpleQueryNodeSeq *IncludingFork, BaseTaskInfo* context)
{
	VError err = VE_OK;
	BTitemIndex *val = nil, *val2 = nil;

	rt = nil;
	rt2 = nil;
	cle1 = nil;
	cle2 = nil;
	xstrict1 = false;
	xstrict2 = false;

	xbox_assert(fInd->MatchType(DB4D_Index_OnMultipleFields));
	IndexInfoFromMultipleField* indm = (IndexInfoFromMultipleField*)fInd;

	Vx0ArrayOf<VValueSingle*, 10> values;
	sLONG ival = 0;
	sLONG ilast = (sLONG)fNodes.size()-1;
	fParseAllIndex = false;
	for (SimpleQueryNodeSeqVector::iterator cur = fNodes.begin(), end = fNodes.end(); cur != end; cur++, ival++)
	{
		VValueSingle* val = (*cur)->rt->ch;
		if (val->GetValueKind() == VK_STRING)
		{
			VString* s = (VString*)val;
			sLONG p = s->FindUniChar(GetWildChar(context));
			bool compatible = GetContextIntl(context)->GetCollator()->IsPatternCompatibleWithDichotomyAndDiacritics( s->GetCPointer(), s->GetLength());

			if (ival != ilast)
			{
				if (!(p == 0 || p == -1))				
					fParseAllIndex = true;
				if (!compatible)
					fParseAllIndex = true;
			}
			else
			{
				if (!(p == 0 || p == -1 || (p == s->GetLength() && compatible) ))
					fParseAllIndex = true;
			}
		}
		values.Add(val);
		fOptions = (*cur)->rt->fOptions;
	}

	val = indm->BuildKeyFromVValues(values, err);
	if (err == VE_OK)
	{
		fIncludingFork = IncludingFork;

		if (IncludingFork != nil)
		{
			values.SetCount(values.GetCount()-1);
			values.Add(IncludingFork->rt->ch);
			val2 = indm->BuildKeyFromVValues(values, err);
		}

		if (err == VE_OK)
		{
			SubBuildCompFrom((*fNodes.rbegin())->rt->comparaison, val);
			if (val2 != nil)
			{
				isfourche = true;
				SubBuildCompFrom(IncludingFork->rt->comparaison, val2);
			}
		}
	}

	fOptions.SetBeginsWith(false);
	fOptions.SetLike(false);

	if (err != VE_OK)
	{
		if (val != nil)
			fInd->FreeKey(val);
		if (val2 != nil)
			fInd->FreeKey(val2);
		val = nil;
		val2 = nil;
	}

	return err;
}


sLONG8 SimpleQueryNodeIndex::CountDiskAccess()
{
	return fInd->GetNBDiskAccessForAScan(false);
}

sLONG SimpleQueryNodeIndex::CountBlobSources()
{
	if (fInd->SourceIsABlob())
		return 1;
	else
		return 0;
}


sLONG SimpleQueryNodeIndex::GetPoids(BaseTaskInfo* context)
{
	if (cle1 == cle2) // egal
	{
		switch (fInd->GetHeader()->GetRealType())
		{
			case DB4D_Index_Htable:
				return 0;
			case DB4D_Index_Btree:
				return 1;
			case DB4D_Index_BtreeWithCluster:
				return 2;
			default:
				return 3;
		}
	}
	else
	{
		if (cle1 != nil && cle2 != nil) // fourche
		{
			return 5;
		}
		else
		{
			return 7;
		}
	}
}


VError SimpleQueryNodeIndex::Describe(VString& result)
{
	VString s = L"[Index : ";
	VString s2;

	if (fInd != nil)
	{
		fInd->GetDebugString(s2, 0);
	}
	else
		s2 = L"<Null Index>";

	s += s2;
	s += L"]";

	if (rt != NULL && rt->fCheckForNull)
	{
		if (inverse)
			s+=L" is not <Null>";
		else
			s+=L" is <Null>";
	}
	else
	{
		if (cle1 == cle2)
		{
			if (fOptions.IsBeginsWith())
				s2 = L" Begin with ";
			else
			{
				if (inverse)
					s2 = L" # ";
				else
				{
					if (fOptions.IsLike())
						s2 = L" LIKE ";
					else
						s2 = L" = ";
				}
			}
		}
		else
		{
			if (cle1 == nil)
			{
				if (cle2 != nil)
				{
					if (xstrict2)
						s2 = L" < ";
					else
						s2 = L" <= ";
				}
				else
				{
					s2 = L" <Nop> ";
				}
			}
			else
			{
				if (cle2 == nil)
				{
					if (xstrict1)
						s2 = L" > ";
					else
						s2 = L" >= ";
				}
				else
				{
					s2 = L" Between ";
				}
			}
		}
	}

	s += s2;

	if (cle1 != nil)
	{
		cle1->GetDebugString(fInd, s2);
		s += s2;
		if (cle2 != nil && cle2 != cle1)
		{
			s += L" -And- ";
			cle2->GetDebugString(fInd, s2);
			s += s2;
		}
	}
	else
	{
		if (cle2 != nil)
		{
			cle2->GetDebugString(fInd, s2);
			s += s2;
		}
		else
			s += L"<Null>";
	}

	result = s;

	return VE_OK;
}


VError SimpleQueryNodeIndex::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	VString s;
	QueryNode::AddToQueryDescriptor(descriptor, level);

	Describe(s);
	s += L"\n";

	descriptor += s;

	return VE_OK;
}



			/* -------------------------- */


VError SimpleQueryNodeArrayIndex::Perform(Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, VValueBag* curdesc)
{
	Bittab *b;
	VError err = VE_OK;

	if ((CountDiskAccess()/10) > CalculateNBDiskAccessForSeq(filtre, context, CountBlobSources()))
	{
		SimpleQueryNode* seq = (SimpleQueryNode*)fOwner->xTransformSeq(this, true, err, context);
		if (seq != nil)
		{
			err = seq->Perform(super, filtre, InProgress, context, HowToLock, exceptions, limit, Nulls, curdesc);
			seq->Release();
		}
	}
	else
	{
		//## pour tester performance
		//InProgress = nil;

		uLONG start = fOwner->GetStartingTime();

		fInd->Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		b=fInd->FindKeyInArray(rt->values, context, err, InProgress, rt->fOptions, nil);
		if (b!=nil && err == VE_OK)
		{
			if (inverse)
			{
				err=b->Invert();
				fTarget.GetTable()->GetDF()->TakeOffBits(b, context);
				fInd->Open(index_read);
				BitSel* sel = fInd->GetHeader()->RetainNulls(curstack, err, context);
				if (sel != nil)
				{
					err = b->moins(sel->GetBittab(), false);
					if (err == VE_OK)
						err = Nulls.Or(sel->GetBittab(), true);
					sel->Release();
				}
				fInd->Close();
			}
			else
			{
				BitSel* sel = fInd->GetHeader()->RetainNulls(curstack, err, context);
				if (sel != nil)
				{
					err = Nulls.Or(sel->GetBittab(), true);
					sel->Release();
				}
				fInd->Close();
			}

		}
		else
			fInd->Close();

		if (err == VE_OK)
		{
			if (filtre != nil) 
				err = b->And(filtre);
		}

		if (HowToLock == DB4D_Keep_Lock_With_Transaction)
		{
			if (context != nil)
			{
				Transaction* trans = context->GetCurrentTransaction();
				if (trans != nil)
				{
					err = trans->TryToLockSel(fTarget.GetTable()->GetDF(), b, exceptions);
				}
			}
		}

		if (fOwner->IsQueryDescribed())
		{
			VString s;
			FullyDescribe(s);
			fOwner->AddToDescription(curdesc, s, start, b);
		}

		if (err != VE_OK)
		{
			ReleaseRefCountable(&b);
		}

		*super = b;
	}

	return(err);
}


VError SimpleQueryNodeArrayIndex::BuildFrom(ComplexRechTokenArrayComp *xrt)
{
	VError err = VE_OK;

	inverse = false;
	fOptions = xrt->fOptions;
	fOptions.SetBeginsWith(false);
	fOptions.SetLike(false);

	rt = xrt;

	switch (rt->comparaison)
	{
		case DB4D_Like:
			fOptions.SetLike(true);
			break;

		case DB4D_NotLike:
			fOptions.SetLike(true);
			inverse = true;
			break;

		case DB4D_NotEqual:
			inverse=true;
			break;

		case DB4D_BeginsWith:
			fOptions.SetBeginsWith(true);
			break;
	}
	return(err);
}


sLONG8 SimpleQueryNodeArrayIndex::CountDiskAccess()
{
	return fInd->GetNBDiskAccessForAScan(false);
}

sLONG SimpleQueryNodeArrayIndex::CountBlobSources()
{
	if (fInd->SourceIsABlob())
		return 1;
	else
		return 0;
}

VError SimpleQueryNodeArrayIndex::Describe(VString& result)
{
	VString s = L"[Index : ";
	VString s2;

	if (fInd != nil)
	{
		fInd->GetDebugString(s2, 0);
	}
	else
		s2 = L"<Null Index>";

	s += s2;
	s += L"]";

	if (fOptions.IsBeginsWith())
		s2 = L" Begin with ";
	else
	{
		if (inverse)
			s2 = L" # ";
		else
			s2 = L" = ";
	}

	s += s2;

	s += L"<Array of Values>";

	result = s;

	return VE_OK;
}


VError SimpleQueryNodeArrayIndex::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	VString s;
	QueryNode::AddToQueryDescriptor(descriptor, level);

	Describe(s);
	s += L"\n";

	descriptor += s;

	return VE_OK;
}



			/* -------------------------- */



class QueryNodeLess
{
	public:
		QueryNodeLess(BaseTaskInfo* inContext)
		{
			fContext = inContext;
		}

		bool operator ()(QueryNode* r1, QueryNode* r2)
		{
			if (r1 == nil)
				return false;
			if (r2 == nil)
				return true;
			if (r1->IsAllIndexe(fContext))
			{
				if (r2->IsAllIndexe(fContext))
				{
					return r1->GetPoids(fContext) < r2->GetPoids(fContext);
				}
				else
					return true;
			}
			else
			{
				if (r2->IsAllIndexe(fContext))
					return false;
				else
				{
					if (r1->IsIndexe(fContext))
					{
						if (r2->IsIndexe(fContext))
						{
							return r1->GetPoids(fContext) < r2->GetPoids(fContext);
						}
						else
							return true;
					}
					else
					{
						if (r2->IsIndexe(fContext))
							return false;
						else
							return r1->GetPoids(fContext) < r2->GetPoids(fContext);
					}
				}
			}
			//return  r1->GetPoids() < r2->GetPoids();
		}

	protected:
		BaseTaskInfo* fContext;

};



VError SimpleQueryNodeBoolOper::Perform(Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, VValueBag* curdesc)
{
	Bittab *b1, *b2;
	VError err = VE_OK;

	if (IsIndexe(context) && ((CountDiskAccess()/10) > CalculateNBDiskAccessForSeq(filtre, context, CountBlobSources())))
	{
		SimpleQueryNode* seq = (SimpleQueryNode*)fOwner->xTransformSeq(this, true, err, context);
		if (seq != nil)
		{
			err = seq->Perform(super, filtre, InProgress, context, HowToLock, exceptions, limit, Nulls, curdesc);
			seq->Release();
		}

	}
	else
	{
		if (!IsIndexe(context))
		{
			err = SimpleQueryNode::Perform(super, filtre, InProgress, context, HowToLock, exceptions, limit, Nulls, curdesc);
		}
		else
		{
			if (fNodes.size() == 0)
			{
				*super = nil;
			}
			else
			{
				if (fNodes.size() == 1)
				{
					err = (*fNodes.begin())->Perform(super, filtre, InProgress, context, HowToLock, exceptions, limit, Nulls, curdesc);
				}
				else
				{
					uLONG start = fOwner->GetStartingTime();

					VValueBag* subline = nil;
					if (fOwner->IsQueryDescribed())
					{
						VString s;
						if (fOperBool == DB4D_And)
							s = L"And";
						else
							s = L"Or";
						subline = fOwner->AddToDescription(curdesc, s, start, nil);
					}

					b1 = nil;
					bool canperformindex = true;

					SimpleQueryNodeVector::iterator cur = fNodes.begin(), end = fNodes.end();
					for (;cur != end && err == VE_OK && canperformindex; cur++)
					{
						if (testAssert(*cur != nil))
						{
							SimpleQueryNode* nod = *cur;
							if (nod->IsConst())
							{
								// normalement rien a faire car on se retrouve dans les cas non filtres par SimpleQueryNodeBoolOper::IsConst
								// donc la constante ne va rien modifier a l'operateur
							}
							else
							{
								if (b1 == nil)
								{
									err = nod->Perform(&b1, filtre, InProgress, context, HowToLock, exceptions, limit, Nulls, subline);
								}
								else
								{
									b2 = nil;

									if (fOperBool == DB4D_And)
									{
										sLONG recfiltre = b1->Compte();
										if (recfiltre == 0)
										{
											canperformindex = false;
										}
										else
										{

											if ((cur+1) != end)
											{
												sLONG8 tot = 0;
												for (SimpleQueryNodeVector::iterator curx = cur, endx = fNodes.end(); curx != endx; curx++)
												{
													SimpleQueryNode* nodx = *curx;
													if (nodx != nil)
														tot = tot + nodx->CountDiskAccess();
												}
												if ( ((sLONG8)recfiltre * ((sLONG8)CountBlobSources()*3+1)) < (tot/10))
												{
													SimpleQueryNodeBoolOper* rnoper = new SimpleQueryNodeBoolOper(fOwner, fTarget, DB4D_And);
													for (SimpleQueryNodeVector::iterator curx = cur, endx = fNodes.end(); curx != endx; curx++)
													{
														SimpleQueryNode* nodx = *curx;
														if (nodx != nil)
															rnoper->AddNode(nodx);
													}
													SimpleQueryNode* newnode = (SimpleQueryNode*)fOwner->xTransformSeq(rnoper, true, err, context);
													if (newnode != nil)
													{
														if (err == VE_OK)
															err = newnode->Perform(&b2, b1, InProgress, context, HowToLock, exceptions, limit, Nulls, subline);
														if (err == VE_OK)
														{
															err=b1->And(b2);
														}
														newnode->Release();
													}
													canperformindex = false;
												}
											}
										}

										if (canperformindex)
										{
											err = nod->Perform(&b2, b1, InProgress, context, HowToLock, exceptions, limit, Nulls, subline);
											if (err == VE_OK)
											{
												err=b1->And(b2);
											}
										}
									}
									else
									{
										Bittab subNulls;
										err = nod->Perform(&b2, filtre, InProgress, context, HowToLock, exceptions, limit, subNulls, subline);
										if (err == VE_OK)
										{
											err=b1->Or(b2);
											if (err == VE_OK)
												err = Nulls.And(&subNulls, true);
										}
									}
									ReleaseRefCountable(&b2);
								}
							}
						}
					}
					*super = b1;
					if (fOwner->IsQueryDescribed())
					{
						subline->SetLong(QueryKeys::time, VSystem::GetCurrentTime() - start);
						subline->SetLong(QueryKeys::recordsfounds, b1 == nil ? 0 : b1->Compte());
					}
				}
			}
		}
	}

	return(err);
}


uBOOL SimpleQueryNodeBoolOper::PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
{
	uBOOL ok = fOperBool == DB4D_And, ok2;

	SimpleQueryNodeVector::iterator cur = fNodes.begin(), end = fNodes.end();
	for (;cur != end; cur++)
	{
		if (testAssert(*cur != nil))
		{
			SimpleQueryNode* nod = *cur;
			ok2 = nod->PerformSeq(curfic, tfb, InProgress, context, HowToLock, exceptions, limit);
			if (ok2 == 4)
			{
				 ok = 4;
				 break;
			}
			else
			{
				switch (fOperBool)
				{
					case DB4D_OR:
					case DB4D_NOTCONJ:
						if (ok == 2)
						{
							if (ok2 == 1)
								ok = true;
						}
						else
						{
							if (ok2 == 2)
							{
								if (ok == 1)
									ok = true;
								else
									ok = 2;
							}
							else
								ok=ok || ok2;
						}
						break;

					case DB4D_And:
						if (ok == 2)
						{
							if (ok2 == false)
								ok = false;
						}
						else
						{
							if (ok2 == 2)
							{
								if (ok == false)
									ok = false;
								else
									ok = 2;
							}
							else
								ok=ok && ok2;
						}
						break;
				}
			}
		}
	}

	return(ok);
}


uBOOL SimpleQueryNodeBoolOper::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
{
	uBOOL ok = fOperBool == DB4D_And, ok2;

	SimpleQueryNodeVector::iterator cur = fNodes.begin(), end = fNodes.end();
	for (;cur != end; cur++)
	{
		if (testAssert(*cur != nil))
		{
			SimpleQueryNode* nod = *cur;
			ok2 = nod->PerformSeq(ficD, tfb, InProgress, context, HowToLock, exceptions, limit);
			if (ok2 == 4)
			{
				 ok = 4;
				 break;
			}
			else
			{
				switch (fOperBool)
				{
					case DB4D_OR:
					case DB4D_NOTCONJ:
						if (ok == 2)
						{
							if (ok2 == 1)
								ok = true;
						}
						else
						{
							if (ok2 == 2)
							{
								if (ok == 1)
									ok = true;
								else
									ok = 2;
							}
							else
								ok=ok || ok2;
						}
						break;

					case DB4D_And:
						if (ok == 2)
						{
							if (ok2 == false)
								ok = false;
						}
						else
						{
							if (ok2 == 2)
							{
								if (ok == false)
									ok = false;
								else
									ok = 2;
							}
							else
								ok=ok && ok2;
						}
						break;
				}
			}
		}
	}

	return(ok);
}


Boolean SimpleQueryNodeBoolOper::AddNode(SimpleQueryNode* rn)
{
	recalcIndex = true;
	recalcAllIndex = true;
	if (rn != nil)
	{
		try
		{
			fNodes.push_back(rn);
		}
		catch (...)
		{
			return false;
		}
	}
	return true;
}


void SimpleQueryNodeBoolOper::DeleteNode(SimpleQueryNodeVector::iterator NodePos)
{
	recalcIndex = true;
	recalcAllIndex = true;
	fNodes.erase(NodePos);
}


void SimpleQueryNodeBoolOper::DeleteNode(sLONG pos)
{
	recalcIndex = true;
	recalcAllIndex = true;
	fNodes.erase(fNodes.begin()+pos);
}


Boolean SimpleQueryNodeBoolOper::IsConst()
{
	uBOOL result = false;

	if (recalcConst)
	{
		SimpleQueryNodeVector::iterator cur = fNodes.begin(), end = fNodes.end();
		for (;cur != end; cur++)
		{
			if (testAssert(*cur != nil))
			{
				if ((*cur)->IsConst())
				{
					if (fOperBool == DB4D_OR)
					{
						resultconst = (*cur)->GetConstResult();
						if (resultconst == 1)
						{
							result = true;
							break;
						}
					}
					else
					{
						resultconst = (*cur)->GetConstResult();
						if (resultconst == 0 || resultconst == 2)
						{
							result = true;
							break;
						}

					}
				}
			}
		}
		isconst = result;
		recalcConst = false;
	}
	else
	{
		result = isconst;
	}

	return result;
}


uBOOL SimpleQueryNodeBoolOper::GetConstResult()
{
	return resultconst;
}


void SimpleQueryNodeBoolOper::AdjusteIntl(VIntlMgr* inIntl)
{
	SimpleQueryNodeVector::iterator cur = fNodes.begin(), end = fNodes.end();
	for (;cur != end; cur++)
	{
		if (testAssert(*cur != nil))
		{
			(*cur)->AdjusteIntl(inIntl);
		}
	}
}


uBOOL SimpleQueryNodeBoolOper::IsIndexe(BaseTaskInfo* context)
{
	uBOOL result = (fOperBool == DB4D_OR);
	uBOOL result2 = true;

	if (recalcIndex)
	{
		SimpleQueryNodeVector::iterator cur = fNodes.begin(), end = fNodes.end();
		for (;cur != end; cur++)
		{
			if (testAssert(*cur != nil))
			{
				result2 = (*cur)->IsIndexe(context);
				if (fOperBool == DB4D_OR)
				{
					result = result && result2;
				}
				else
				{
					result = result || result2;
				}
			}
		}
		recalcIndex = false;
		isindexe = result;
	}
	else
	{
		result = isindexe;
	}

	return result;
}

uBOOL SimpleQueryNodeBoolOper::IsAllIndexe(BaseTaskInfo* context)
{
	uBOOL result = true;

	if (recalcAllIndex)
	{
		SimpleQueryNodeVector::iterator cur = fNodes.begin(), end = fNodes.end();
		for (;cur != end; cur++)
		{
			if (testAssert(*cur != nil))
			{
				result = result && (*cur)->IsAllIndexe(context);
			}
		}
		isallindexe = result;
		recalcAllIndex = false;
	}
	else
	{
		result = isallindexe;
	}

	return result;
}


Boolean SimpleQueryNodeBoolOper::isAllJoin()
{
	if (recalcAllJoin)
	{
		isalljoin = true;
		for (SimpleQueryNodeVector::iterator cur = fNodes.begin(), end = fNodes.end(); cur != end; cur++)
		{
			if (testAssert(*cur != nil))
			{
				isalljoin = isalljoin && (*cur)->isAllJoin();
			}
		}
		recalcAllJoin = false;
	}
	return isalljoin;
}


sLONG8 SimpleQueryNodeBoolOper::CountDiskAccess()
{
	sLONG8 tot = 0;
	for (SimpleQueryNodeVector::iterator cur = fNodes.begin(), end = fNodes.end(); cur != end; cur++)
	{
		SimpleQueryNode* node = *cur;
		tot = tot + node->CountDiskAccess();
	}

	return tot;
}


sLONG SimpleQueryNodeBoolOper::CountBlobSources()
{
	sLONG tot = 0;
	for (SimpleQueryNodeVector::iterator cur = fNodes.begin(), end = fNodes.end(); cur != end; cur++)
	{
		SimpleQueryNode* node = *cur;
		tot = tot + node->CountBlobSources();
	}

	return tot;
}


sLONG SimpleQueryNodeBoolOper::GetPoids(BaseTaskInfo* context)
{
	if (recalcPoids)
	{
		fPoids = 0;
		sLONG cumulPoids = 0;
		for (SimpleQueryNodeVector::iterator cur = fNodes.begin(), end = fNodes.end(); cur != end; cur++)
		{
			sLONG curpoids = 0;
			SimpleQueryNode* node = *cur;
			if (node->isAllJoin())
			{
				cumulPoids += node->GetJoinPoids(context);
				curpoids = node->GetSubPoids(context);
			}
			else
			{
				curpoids = node->GetPoids(context);
			}
			if (curpoids > fPoids)
				fPoids = curpoids;
		}
		fPoids += cumulPoids;
	}
	return fPoids;
}


VError SimpleQueryNodeBoolOper::Describe(VString& result)
{
	switch (fOperBool)
	{
	case DB4D_And:
		result = L"And";
		break;

	case DB4D_OR:
		result = L"Or";
		break;

	default:
		result = L"<Error Boolean Operator>";
		break;
	}
	return VE_OK;
}


VError SimpleQueryNodeBoolOper::FullyDescribe(VString& result)
{
	Boolean first = true;
	for (SimpleQueryNodeVector::iterator cur = fNodes.begin(), end = fNodes.end(); cur != end; cur++)
	{
		if (*cur != nil)
		{
			if (first)
			{
				first = false;
			}
			else
			{
				if (fOperBool == DB4D_And)
					result += L" And ";
				else
					result += L" Or ";
			}

			SimpleQueryNode* nod = *cur;
			if (nod->IsComposed())
			{
				result += L"(";
			}
			VString s;
			nod->FullyDescribe(s);
			result += s;
			if (nod->IsComposed())
			{
				result += L")";
			}
		}
	}

	return VE_OK;
}


VError SimpleQueryNodeBoolOper::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	VString s;
	VError err = VE_OK;
	Boolean deja = false;

	Describe(s);
	s += L"\n";

	for (SimpleQueryNodeVector::iterator cur = fNodes.begin(), end = fNodes.end(); cur != end && err == VE_OK; cur++)
	{
		if (*cur != nil)
		{
			if (deja)
			{
				QueryNode::AddToQueryDescriptor(descriptor, level);
				descriptor += s;
			}
			else
				deja = true;
			err = (*cur)->AddToQueryDescriptor(descriptor, level+1);
		}
	}

	return err;
}



void SimpleQueryNodeBoolOper::PermuteAnds(BaseTaskInfo* context)
{
	for (SimpleQueryNodeVector::iterator cur = fNodes.begin(), end = fNodes.end(); cur != end; cur++)
	{
		SimpleQueryNode* node = *cur;
		node->PermuteAnds(context);
	}
	if (fOperBool == DB4D_And)
	{
		sort(fNodes.begin(), fNodes.end(), QueryNodeLess(context));
	}
}



			/* -------------------------- */



uBOOL SimpleQueryNodeNot::IsIndexe(BaseTaskInfo* context)
{
	if (fRoot == nil)
		return false;
	else
		return fRoot->IsIndexe(context);
}



uBOOL SimpleQueryNodeNot::IsAllIndexe(BaseTaskInfo* context)
{
	if (fRoot == nil)
		return false;
	else
		return fRoot->IsAllIndexe(context);
}


sLONG8 SimpleQueryNodeNot::CountDiskAccess()
{
	if (fRoot == nil)
		return 0;
	else
		return fRoot->CountDiskAccess();
}


sLONG SimpleQueryNodeNot::CountBlobSources()
{
	if (fRoot == nil)
		return 0;
	else
		return fRoot->CountBlobSources();
}


VError SimpleQueryNodeNot::Describe(VString& result)
{
	result = L"Not";
	return VE_OK;
}


VError SimpleQueryNodeNot::FullyDescribe(VString& result)
{
	if (fRoot != nil)
	{
		result += L"Not ";
		if (fRoot->IsComposed())
		{
			result += L"(";
		}
		VString s1;
		fRoot->FullyDescribe(s1);
		result += s1;
		if (fRoot->IsComposed())
		{
			result += L")";
		}
	}
	return VE_OK;
}


VError SimpleQueryNodeNot::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	VString s;
	Describe(s);
	s += L"\n";
	QueryNode::AddToQueryDescriptor(descriptor, level);
	descriptor += s;

	if (fRoot != nil)
		fRoot->AddToQueryDescriptor(descriptor, level+1);

	return VE_OK;
}



uBOOL SimpleQueryNodeNot::IsConst() 
{
	if (fRoot != nil)
	{
		return fRoot->IsConst();
	}
	else
		return false;
}


uBOOL SimpleQueryNodeNot::GetConstResult()
{
	uBOOL res = 0;
	if (fRoot != nil)
	{
		res = fRoot->GetConstResult();
		if (res != 2)
		{
			res = ! res;
		}
	}
	return res;
}


VError SimpleQueryNodeNot::Perform(Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, VValueBag* curdesc)
{
	Bittab *b1, *b2;
	VError err = VE_OK;

	if (!IsIndexe(context))
	{
		err = SimpleQueryNode::Perform(super, filtre, InProgress, context, HowToLock, exceptions, limit, Nulls, curdesc);
	}
	else
	{
		if (fRoot != nil)
		{
			uLONG start = fOwner->GetStartingTime();
			VValueBag* subline = nil;
			if (fOwner->IsQueryDescribed())
			{
				VString s;
				FullyDescribe(s);
				subline = fOwner->AddToDescription(curdesc, s, start, nil);
			}

			b1 = nil;
			err = fRoot->Perform(&b1, nil, InProgress, context, HowToLock, exceptions, limit, Nulls, subline);
			if (b1 != nil)
			{
				err = b1->Invert();
				if (err == VE_OK)
				{
					fTarget.GetTable()->GetDF()->TakeOffBits(b1, context);
					err = b1->moins(&Nulls, true);
					if (filtre != nil) 
						err = b1->And(filtre);
				}
			}
			*super = b1;

			if (fOwner->IsQueryDescribed())
			{
				subline->SetLong(QueryKeys::time, VSystem::GetCurrentTime() - start);
				subline->SetLong(QueryKeys::recordsfounds, b1 == nil ? 0 : b1->Compte());
			}
		}
		else
		{
			*super = nil;
		}
	}

	return(err);
}


uBOOL SimpleQueryNodeNot::PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
{
	uBOOL ok;

	if (fRoot != nil) ok = fRoot->PerformSeq(curfic, tfb, InProgress, context, HowToLock, exceptions, limit);
	else ok = true;

	if (ok != 2 && ok != 4)
		ok = !ok;

	return(ok);
}

uBOOL SimpleQueryNodeNot::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
{
	uBOOL ok;

	if (fRoot != nil) ok = fRoot->PerformSeq(ficD, tfb, InProgress, context, HowToLock, exceptions, limit);
	else ok = true;

	if (ok != 2 && ok != 4)
		ok = !ok;

	return(ok);
}



			/* -------------------------- */


uBOOL QuerySimpleNodeCacheSel::IsIndexe(BaseTaskInfo* context)
{
	if (fRoot == nil)
		return false;
	else
		return fRoot->IsIndexe(context);
}



uBOOL QuerySimpleNodeCacheSel::IsAllIndexe(BaseTaskInfo* context)
{
	if (fRoot == nil)
		return false;
	else
		return fRoot->IsAllIndexe(context);
}



VError QuerySimpleNodeCacheSel::Perform(Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, VValueBag* curdesc)
{
	Bittab *b1, *b2;
	VError err = VE_OK;
	Boolean calcul = false;

	*super = nil;

	if (fRoot != nil)
	{
		if (fCachedSel == nil)
		{
			calcul = true;
			err = fRoot->Perform(&fCachedSel, filtre, InProgress, context, HowToLock, exceptions, limit, fNulls, curdesc);
			if (err != VE_OK)
			{
				ReleaseRefCountable(&fCachedSel);
			}
		}
		
		if (fCachedSel != nil && err == VE_OK)
		{
			uLONG start = fOwner->GetStartingTime();
			*super = new Bittab;
			if (*super == nil)
				err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
			else
			{
				err = (*super)->Or(fCachedSel, true);
				if (err == VE_OK)
				{
					err = Nulls.Or(&fNulls, true);
				}
				if (err != VE_OK)
				{
					ReleaseRefCountable(super);
				}
				else
				{
					if (fOwner->IsQueryDescribed() && !calcul)
					{
						VString s;
						FullyDescribe(s);
						fOwner->AddToDescription(curdesc, s, start, *super);
					}
				}
			}
		}
	}

	return(err);
}


uBOOL QuerySimpleNodeCacheSel::PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
{
	uBOOL ok;

	if (fRoot != nil) ok = fRoot->PerformSeq(curfic, tfb, InProgress, context, HowToLock, exceptions, limit);
	else ok = true;

	return(ok);
}

uBOOL QuerySimpleNodeCacheSel::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
{
	uBOOL ok;

	if (fRoot != nil) ok = fRoot->PerformSeq(ficD, tfb, InProgress, context, HowToLock, exceptions, limit);
	else ok = true;

	return(ok);
}


VError QuerySimpleNodeCacheSel::Describe(VString& result)
{
	result = L"[Cached Selection] : ";
	return VE_OK;
}


VError QuerySimpleNodeCacheSel::FullyDescribe(VString& result)
{
	if (fRoot != nil)
	{
		VString s;
		Describe(s);
		result += s;
		if (fRoot->IsComposed())
		{
			result += L"(";
		}
		VString s1;
		fRoot->FullyDescribe(s1);
		result += s1;
		if (fRoot->IsComposed())
		{
			result += L")";
		}
	}
	return VE_OK;
}


VError QuerySimpleNodeCacheSel::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	QueryNode::AddToQueryDescriptor(descriptor, level);
	
	VString s;
	Describe(s);
	s += L"\n";

	descriptor += s;

	if (fRoot != nil)
		fRoot->AddToQueryDescriptor(descriptor, level+1);

	return VE_OK;
}


sLONG8 QuerySimpleNodeCacheSel::CountDiskAccess()
{
	if (fRoot == nil || fCachedSel != nil)
	{
		return 0;
	}
	else
	{
		return fRoot->CountDiskAccess();
	}
}


sLONG QuerySimpleNodeCacheSel::CountBlobSources()
{
	if (fRoot == nil || fCachedSel != nil)
		return 0;
	else
		return fRoot->CountBlobSources();
}




			/* -------------------------- */


Bittab* SimpleQueryNodeJoin::PerformJoin(Bittab* sel, Bittab* selJoin, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, VError& err, VValueBag* curdesc)
{
	Bittab* result = nil;
	DataTable* df = fTarget.GetTable()->GetDF();
	xbox_assert(df != nil);
	Boolean CanDoArrayIndexSearch = false;
	Field* cri;
	Field* criJoin;

	//## pour tester performance
	//InProgress = nil;

	uLONG start = fOwner->GetStartingTime();

	rt->fOptions.SetLike(false);
	rt->fOptions.SetBeginsWith(false);

	xbox_assert(rt->comparaison == DB4D_Like || rt->comparaison == DB4D_BeginsWith || rt->comparaison == DB4D_Equal);
	if (rt->comparaison == DB4D_Like)
		rt->fOptions.SetLike(true);
	else
	{
		if (rt->comparaison == DB4D_BeginsWith)
		{
			rt->fOptions.SetBeginsWith(true);
		}
	}
	rt->fOptions.SetIntlManager(GetContextIntl(context));

	if (rt->cri1->GetOwner() == fTarget.GetTable() && rt->fNumInstance1 == fTarget.GetInstance())
	{
		cri = rt->cri1;
		criJoin = rt->cri2;
	}
	else
	{
		cri = rt->cri2;
		criJoin = rt->cri1;
	}

	sLONG countJoin = selJoin->Compte();
	sLONG8 count;
	if (sel == nil)
	{
		count = df->GetNbRecords(context);
	}
	else
		count = sel->Compte();

	if (err == VE_OK)
	{
		IndexInfo* ind = cri->FindAndRetainIndexSimple(false, true, context);
		if (ind != nil)
		{
			if (countJoin > 1)
			{
				if (count > 5)
					CanDoArrayIndexSearch = true;
			}
			else
			{
				if (countJoin > 100 || (countJoin*df->GetSeqRatioCorrector()) > ind->GetNBDiskAccessForAScan(false))
					CanDoArrayIndexSearch = true;
			}
		}

		if (CanDoArrayIndexSearch)
		{
			BitSel* bseljoin = new BitSel(criJoin->GetOwner()->GetDF(), selJoin);
			QuickDB4DArrayOfValues* temp2 = CreateDB4DArrayOfDirectValues(criJoin->GetTyp(), rt->fOptions, (VJSArray*)nil);

			VError err2;
			{
				StErrorContextInstaller errs;
				err2 = bseljoin->QuickGetDistinctValues(criJoin, temp2, context, InProgress, rt->fOptions);
			}

			if (err2 == VE_OK)
			{
				DB4DArrayOfValues* temp = temp2->GenerateConstArrayOfValues();

				if (temp != nil)
				{
					result = ind->FindKeyInArray(temp, context, err, InProgress, rt->fOptions, nil);
					if (err == VE_OK)
					{
						if (sel != nil) 
							err = result->And(sel);
					}
					temp->Release();
				}

			}
			else
				CanDoArrayIndexSearch = false;
			

			temp2->Release();
			bseljoin->Release();
		}

		if (ind != nil)
		{
			ind->ReleaseValid();
			ind->Release();
		}
		
		if (!CanDoArrayIndexSearch)
		{
			SortTab sortJoin(criJoin->GetOwner()->GetOwner());
			sortJoin.AddTriLineField(criJoin);
			Selection* selJoinx = nil;

			SortTab sort1(cri->GetOwner()->GetOwner());
			sort1.AddTriLineField(cri);
			BitSel* TrueSel = nil;
			if (sel != nil)
				TrueSel = new BitSel(df, sel, nil);

			BitSel* TrueSelJoin = new BitSel(criJoin->GetOwner()->GetDF(), selJoin, false);

			Selection* selx = cri->GetOwner()->GetDF()->SortSel(sort1, TrueSel, context, InProgress, err);
			if (err == VE_OK)
			{
				selJoinx = criJoin->GetOwner()->GetDF()->SortSel(sortJoin, TrueSelJoin, context, InProgress, err);
			}

			if (TrueSel != nil)
				TrueSel->Release();

			if (TrueSelJoin != nil)
				TrueSelJoin->Release();

			if (err == VE_OK)
			{
				result = new Bittab();
				if (result != nil)
				{
					err = result->aggrandit(df->GetMaxRecords(context));
					if (err == VE_OK)
					{
						DataTable* dfJoin = criJoin->GetOwner()->GetDF();

						SelectionIterator itersel(selx);
						SelectionIterator iterselJoin(selJoinx);
						sLONG cur = itersel.FirstRecord();
						sLONG curJoin = iterselJoin.FirstRecord();
						FicheInMem* curFiche = nil;
						FicheInMem* curFicheJoin = nil;
						ValPtr cv = nil, cvJoin = nil;

						while (cur != -1 && curJoin != -1 && err == VE_OK)
						{
							Boolean cancompare = true;
							Boolean Step1 = false, StepJoin = false;

							if (curFiche == nil)
							{
								cv = nil;
								curFiche = df->LoadRecord(cur, err, DB4D_Do_Not_Lock, context, false, false);
								if (err == VE_OK && curFiche != nil)
									cv = curFiche->GetFieldValue(cri, err, false, true);
							}

							if (err == VE_OK)
							{
								if (cv == nil)
								{
									Step1 = true;
									cancompare = false;
								}
								else
								{
									if (cv->IsNull())
									{
										cancompare = false;
										Step1 = true;
									}
								}


								if (curFicheJoin == nil)
								{
									cvJoin = nil;
									curFicheJoin = dfJoin->LoadRecord(curJoin, err, DB4D_Do_Not_Lock, context, false, false);
									if (err == VE_OK && curFicheJoin != nil)
										cvJoin = curFicheJoin->GetFieldValue(criJoin, err, false, true);
								}
							}

							if (err == VE_OK)
							{
								if (cvJoin == nil)
								{
									StepJoin = true;
									cancompare = false;
								}
								else
								{
									if (cvJoin->IsNull())
									{
										StepJoin = true;
										cancompare = false;
									}
								}
							}

							if (cancompare)
							{
								CompareResult res;

								if (cvJoin->GetValueKind() != cv->GetValueKind())
								{
									ValPtr cvtemp = cvJoin->ConvertTo(cv->GetValueKind());
									if (cvtemp == nil)
										err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
									else
									{
										res = cv->CompareToSameKindWithOptions(cvtemp, rt->fOptions);
										delete cvtemp;
									}
								}
								else
								{
									res = cv->CompareToSameKindWithOptions(cvJoin, rt->fOptions);
								}

								if (err == VE_OK)
								{
									if (res == CR_EQUAL)
									{
										Step1 = true;
										err = result->Set(cur, true);
									}
									else
									{
										if (res == CR_BIGGER)
											StepJoin = true;
										else
											Step1 = true;
									}
								}
							}

							if (Step1)
							{
								if (curFiche != nil)
									curFiche->Release();
								curFiche = nil;
								cv = nil;
								cur = itersel.NextRecord();
							}

							if (StepJoin)
							{
								if (curFicheJoin != nil)
									curFicheJoin->Release();
								curFicheJoin = nil;
								cvJoin = nil;
								curJoin = iterselJoin.NextRecord();
							}

						}

						if (curFiche != nil)
							curFiche->Release();

						if (curFicheJoin != nil)
							curFicheJoin->Release();
					}
				}
				else
					err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);

			}

			if (selx != nil)
				selx->Release();
			if (selJoinx != nil)
				selJoinx->Release();
		}
	}

	if (err != VE_OK)
	{
		ReleaseRefCountable(&result);
	}
	else
	{
		if (fOwner->IsQueryDescribed())
		{
			VString s;
			FullyDescribe(s);
			fOwner->AddToDescription(curdesc, s, start, result);
		}
	}

	return result;
}


uBOOL SimpleQueryNodeJoin::IsIndexe(BaseTaskInfo* context)
{
	if (recalcIndex)
	{
		if (rt->cri1->GetOwner() == fTarget.GetTable())
			isindex = rt->cri1->IsIndexed();
		else
			isindex = rt->cri2->IsIndexed();
		recalcIndex = false;
	}
	return isindex;
}


uBOOL SimpleQueryNodeJoin::IsAllIndexe(BaseTaskInfo* context)
{
	return IsIndexe(context);
}


VError SimpleQueryNodeJoin::Describe(VString& result)
{
	VString s = L"[Join] : ";
	VString s2;

	s += BuildStringFromCrit(rt->cri1, rt->fNumInstance1);

	switch (rt->comparaison)
	{
	case DB4D_BeginsWith:
		s2 = L" Begin with ";
		break;

	case DB4D_Equal:
		s2 = L" = ";
		break;

	case DB4D_Like:
		s2 = L" LIKE ";
		break;

	case DB4D_NotEqual:
		s2 = L" # ";
		break;

	case DB4D_NotLike:
		s2 = L" NOT LIKE ";
		break;

	case DB4D_Greater:
	case DB4D_Greater_Like:
		s2 = L" > ";
		break;

	case DB4D_GreaterOrEqual:
	case DB4D_GreaterOrEqual_Like:
		s2 = L" >= ";
		break;

	case DB4D_Lower:
	case DB4D_Lower_Like:
		s2 = L" < ";
		break;

	case DB4D_LowerOrEqual:
	case DB4D_LowerOrEqual_Like:
		s2 = L" <= ";
		break;

	case DB4D_Contains_KeyWord:
		s2 = L" contains ";
		break;

	default:
		s2 = L" <Nop> ";
		break;
	}

	s += s2;

	s += BuildStringFromCrit(rt->cri2, rt->fNumInstance2);

	result = s;

	return VE_OK;
}


VError SimpleQueryNodeJoin::FullyDescribe(VString& result)
{
	return Describe(result);
}


VError SimpleQueryNodeJoin::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	VString s;

	QueryNode::AddToQueryDescriptor(descriptor, level);

	Describe(s);
	s += L"\n";

	descriptor += s;

	return VE_OK;
}


			/* -------------------------- */


VError SimpleQueryNodeBaseJoin::Describe(VString& result)
{
	return VE_OK;
}


VError SimpleQueryNodeBaseJoin::FullyDescribe(VString& result)
{
	return VE_OK;
}


VError SimpleQueryNodeBaseJoin::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	VString s = L"[Merge] : into selection of ";
	QueryNode::AddToQueryDescriptor(descriptor, level);

	VString s2;

	/*
	fTarget.GetTable()->GetName(s2);
	s += s2;
	*/

	s += BuildStringFromTarget(fTarget);

	s += L"\n";
	descriptor += s;

	if (fJoinRoot != nil)
		fJoinRoot->AddToQueryDescriptor(descriptor, level+1);

	if (fRoot != nil)
	{
		QueryNode::AddToQueryDescriptor(descriptor, level+1);
		descriptor += L"With filter :\n";
		fRoot->AddToQueryDescriptor(descriptor, level+2);
	}

	return VE_OK;
}

uBOOL SimpleQueryNodeBaseJoin::IsAllIndexe(BaseTaskInfo* context)
{
	if (fRoot == nil)
		return true;
	else
	{
		if (fJoinRoot == nil)
			return false;
		else
			return fRoot->IsAllIndexe(context) && fJoinRoot->IsAllIndexe(context);
	}
}



uBOOL SimpleQueryNodeBaseJoin::IsIndexe(BaseTaskInfo* context)
{
	if (fRoot == nil)
		return true;
	else
		if (fJoinRoot == nil)
			return false;
		else
			return fRoot->IsIndexe(context) && fJoinRoot->IsIndexe(context);
}



VError SimpleQueryNodeBaseJoin::Perform(Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, VValueBag* curdesc)
{
	VError err = VE_OK;
	*super = nil;

	if (fRoot != nil)
	{
		Bittab* FiltreJoin;
		Bittab* exceptionsJoin = nil;

		err = fRoot->Perform(&FiltreJoin, nil, InProgress, context, HowToLock, exceptionsJoin, 0, Nulls, curdesc);
		if (err == VE_OK && FiltreJoin != nil)
		{
			Bittab *b = fJoinRoot->PerformJoin(filtre, FiltreJoin, InProgress, context, err, curdesc);
		}
		ReleaseRefCountable(&FiltreJoin);
		ReleaseRefCountable(&exceptionsJoin);
	}
	else
	{
		Bittab *b = new Bittab();
		if (b == nil)
			err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
		else
		{
			DataTable* df = fJoinTarget.GetTable()->GetDF();
			if (df != nil)
			{
				err = b->aggrandit(df->GetMaxRecords(context));
				if (err == VE_OK)
				{
					b->ClearOrSetAll(true);
					err = df->TakeOffBits(b, context);
				}
			}
			else
				err = ThrowBaseError(VE_DB4D_WRONGTABLEREF, DBaction_ExecutingComplexQuery);

			if (err == VE_OK)
			{
				*super = b;
			}
			else
				ReleaseRefCountable(&b);
		}
		return VE_OK;
	}

	return err;
}


sLONG SimpleQueryNodeBaseJoin::GetSubPoids(BaseTaskInfo* context)
{
	if (fRoot != nil)
		return fRoot->GetPoids(context);
	else
		return 0;
}


sLONG SimpleQueryNodeBaseJoin::GetJoinPoids(BaseTaskInfo* context)
{
	if (fJoinRoot->IsIndexe(context))
		return 1000;
	else
		return 10000;
}




			/* ----------------------------------------------------------- */



Boolean QueryNodeBoolOper::AddNode(QueryNode* rn)
{
	if (rn != nil)
	{
		try
		{
			fNodes.Add(rn);
		}
		catch (...)
		{
			return false;
		}
	}
	mustRecalcTarget = true;
	return true;
}


void QueryNodeBoolOper::DeleteNode(QueryNodeVector::iterator NodePos)
{
	mustRecalcTarget = true;
	fNodes.fnodes.erase(NodePos);
}


void QueryNodeBoolOper::DeleteNode(sLONG pos)
{
	mustRecalcTarget = true;
	fNodes.fnodes.erase(fNodes.fnodes.begin()+pos);
}



CQTargetsDef* QueryNodeBoolOper::GenerateTargets(VError& err)
{
	CQTargetsDef* result = nil;
	if (mustRecalcTarget)
	{
		SetOfTargets alltargets;
		err = AddToTargets(alltargets);
		if (err == VE_OK)
		{
			result = new CQTargetsDef;
			if (result == nil)
				err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
			else
			{
				for (SetOfTargets::iterator cur = alltargets.begin(), end = alltargets.end(); cur != end && err == VE_OK; cur++)
				{
					if (!result->Add(*cur))
					{
						err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
					}
				}
				if (err != VE_OK)
				{
					result->Release();
					result = nil;
				}
			}
		}
		if (err == VE_OK)
		{
			fCachedTargets = result;
			mustRecalcTarget = false;
		}
	}
	else
	{
		result = fCachedTargets;
		if (result != nil)
			result->Retain();
	}
	return result;
}


VError QueryNodeBoolOper::AddToTargets(SetOfTargets& fullTarget)
{
	VError err = VE_OK;
	for (QueryNodeVector::iterator cur = fNodes.fnodes.begin(), end = fNodes.fnodes.end(); cur != end && err == VE_OK; cur++)
	{
		QueryNode* node = *cur;
		err = node->AddToTargets(fullTarget);
	}
	return err;
}



void QueryNodeBoolOper::AdjusteIntl(VIntlMgr* inIntl)
{
	for (QueryNodeVector::iterator cur = fNodes.fnodes.begin(), end = fNodes.fnodes.end(); cur != end; cur++)
	{
		if (testAssert(*cur != nil))
		{
			(*cur)->AdjusteIntl(inIntl);
		}
	}
}


Boolean QueryNodeBoolOper::isAllJoin()
{
	if (recalcAllJoin)
	{
		isalljoin = true;
		for (QueryNodeVector::iterator cur = fNodes.fnodes.begin(), end = fNodes.fnodes.end(); cur != end; cur++)
		{
			if (testAssert(*cur != nil))
			{
				isalljoin = isalljoin && (*cur)->isAllJoin();
			}
		}
		recalcAllJoin = false;
	}
	return isalljoin;
}


SimpleQueryNode* QueryNodeBoolOper::ConvertToSimple(const CQTableDef& target, VError& err)
{
	err = VE_OK;

	SimpleQueryNodeBoolOper* bn = new SimpleQueryNodeBoolOper(fOwner, target, fOperBool);
	if (bn == nil)
		err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
	else
	{
		for (QueryNodeVector::iterator cur = fNodes.fnodes.begin(), end = fNodes.fnodes.end(); cur != end && err == VE_OK; cur++)
		{
			QueryNode* node = *cur;
			if (node != nil)
			{
				VRefPtr<SimpleQueryNode> sn(node->ConvertToSimple(target, err), false);
				if (err == VE_OK)
				{
					if (!bn->AddNode(sn))
						err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
				}
			}
		}
		if (err != VE_OK)
		{
			bn->Release();
			bn = nil;
		}
	}
	return bn;
}


VError QueryNodeBoolOper::Describe(VString& result)
{
	switch (fOperBool)
	{
		case DB4D_And:
			result = L".AND.";
			break;

		case DB4D_OR:
			result = L".OR.";
			break;

		default:
			result = L"<Error Boolean Operator>";
			break;
	}
	return VE_OK;
}


VError QueryNodeBoolOper::FullyDescribe(VString& result)
{
	Boolean first = true;
	for (QueryNodeVector::iterator cur = fNodes.fnodes.begin(), end = fNodes.fnodes.end(); cur != end; cur++)
	{
		if (*cur != nil)
		{
			if (first)
			{
				first = false;
			}
			else
			{
				if (fOperBool == DB4D_And)
					result += L" .AND. ";
				else
					result += L" .OR. ";
			}

			QueryNode* nod = *cur;
			if (nod->IsComposed())
			{
				result += L"(";
			}
			VString s;
			nod->FullyDescribe(s);
			result += s;
			if (nod->IsComposed())
			{
				result += L")";
			}
		}
	}

	return VE_OK;
}


VError QueryNodeBoolOper::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	VString s;
	VError err = VE_OK;
	Boolean deja = false;

	Describe(s);
	s += L"\n";

	for (QueryNodeVector::iterator cur = fNodes.fnodes.begin(), end = fNodes.fnodes.end(); cur != end && err == VE_OK; cur++)
	{
		if (*cur != nil)
		{
			if (deja)
			{
				QueryNode::AddToQueryDescriptor(descriptor, level);
				descriptor += s;
			}
			else
				deja = true;
			err = (*cur)->AddToQueryDescriptor(descriptor, level+1);
		}
	}

	return err;
}


void QueryNodeBoolOper::PermuteAnds(BaseTaskInfo* context)
{
	for (QueryNodeVector::iterator cur = fNodes.fnodes.begin(), end = fNodes.fnodes.end(); cur != end; cur++)
	{
		QueryNode* node = *cur;
		node->PermuteAnds(context);
	}
	if (fOperBool == DB4D_And)
	{
		sort(fNodes.fnodes.begin(), fNodes.fnodes.end(), QueryNodeLess(context));
	}
}


sLONG QueryNodeBoolOper::GetPoids(BaseTaskInfo* context)
{
	if (recalcPoids)
	{
		fPoids = 0;
		sLONG cumulPoids = 0;
		for (QueryNodeVector::iterator cur = fNodes.fnodes.begin(), end = fNodes.fnodes.end(); cur != end; cur++)
		{
			sLONG curpoids = 0;
			QueryNode* node = *cur;
			if (node->isAllJoin())
			{
				cumulPoids += node->GetJoinPoids(context);
				curpoids = node->GetSubPoids(context);
			}
			else
			{
				curpoids = node->GetPoids(context);
			}
			if (curpoids > fPoids)
				fPoids = curpoids;
		}
		fPoids += cumulPoids;
	}
	return fPoids;
}

Boolean CompareLessNodeTarget(QueryNode* node1, QueryNode* node2)
{
	VError err;

	VRefPtr<CQTargetsDef> t1(node1->GenerateTargets(err), false);
	VRefPtr<CQTargetsDef> t2(node2->GenerateTargets(err), false);

	return CompareLessCQTableDefVector(t1, t2);
}


VError QueryNodeBoolOper::PerformComplex(ComplexSelection* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, ComplexSelection* &outSel, DB4D_Way_of_Locking HowToLock, sLONG limit, VValueBag* curdesc)
{
	VError err = VE_OK;
	QueryNodeVector nodes = fNodes.fnodes;
	outSel = nil;

	uLONG start = fOwner->GetStartingTime();

	VValueBag* subline = nil;
	if (fOwner->IsQueryDescribed())
	{
		VString s;
		if (fOperBool == DB4D_And)
			s = L"And";
		else
			s = L"Or";
		subline = fOwner->AddToDescription(curdesc, s, start, nil);
	}

	sort(nodes.begin(), nodes.end(), CompareLessNodeTarget);

	if (nodes.size() > 2) // changer l'ordre pour etre sur que chaque noeud suivant sera connecte a au moins une target precedente
	{
		CQTargetsDef* dejaTargets = nil;

		bool first = true;
		for (QueryNodeVector::iterator cur = nodes.begin(), end = nodes.end(); cur != end && err == VE_OK; cur++)
		{
			QueryNode* node = *cur;
			if (node != nil)
			{
				if (first)
				{
					dejaTargets = node->GenerateTargets(err);
					first = false;
				}
				else
				{
					CQTargetsDef* nodeTargets = node->GenerateTargets(err);
					if (!dejaTargets->MatchAtLeastOne(*nodeTargets))
					{
						QueryNodeVector::iterator cur2 = cur+1;
						bool matching = false;
						while (cur2 != end && !matching)
						{
							QueryNode* node2 = *cur2;
							if (node2 != nil)
							{
								CQTargetsDef* node2Targets = node2->GenerateTargets(err);
								if (dejaTargets->MatchAtLeastOne(*node2Targets))
								{
									dejaTargets->AddTargets(*node2Targets);
									VRefPtr<QueryNode> xnode = *cur;
									*cur = *cur2;
									*cur2 = xnode;
									matching = true;
								}
								QuickReleaseRefCountable(node2Targets);
							}
							cur2++;
						}
					}
					else
					{
						dejaTargets->AddTargets(*nodeTargets);
					}
					QuickReleaseRefCountable(nodeTargets);
				}
			}

		}
		QuickReleaseRefCountable(dejaTargets);
	}

	for (QueryNodeVector::iterator cur = nodes.begin(), end = nodes.end(); cur != end && err == VE_OK; cur++)
	{
		QueryNode* node = *cur;
		if (node != nil)
		{
			ComplexSelection* tempsel = nil;
			err = node->PerformComplex(filtre, InProgress, context, tempsel, HowToLock, limit, subline);
			if (err == VE_OK)
			{
				if (outSel == nil)
					outSel = tempsel;
				else
				{
					ComplexSelection* tempsel2 = outSel->MergeSel(*tempsel, err);
					tempsel->Release();
					outSel->Release();
					outSel = tempsel2;
				}
			}
		}
	}

	if (outSel != nil && err != VE_OK)
	{
		outSel->Release();
		outSel = nil;
	}

	if (outSel != nil)
	{
		if (fOwner->IsQueryDescribed())
		{
			subline->SetLong(QueryKeys::time, VSystem::GetCurrentTime() - start);
			subline->SetLong(QueryKeys::recordsfounds, outSel->GetNbRows());
		}
	}

	return err;
}


				/* ---------------------------- */


SimpleQueryNode* QueryNodeNot::ConvertToSimple(const CQTableDef& target, VError& err)
{
	SimpleQueryNodeNot* sqnnot = new SimpleQueryNodeNot(fOwner, target);
	if (sqnnot == nil)
		err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
	else
	{
		if (fRoot != nil)
		{
			sqnnot->AdoptRoot(fRoot->ConvertToSimple(target, err));
		}
	}
	return sqnnot;
}


VError QueryNodeNot::Describe(VString& result)
{
	result = L"Not";
	return VE_OK;
}


VError QueryNodeNot::FullyDescribe(VString& result)
{
	if (fRoot != nil)
	{
		result += L"Not ";
		if (fRoot->IsComposed())
		{
			result += L"(";
		}
		VString s1;
		fRoot->FullyDescribe(s1);
		result += s1;
		if (fRoot->IsComposed())
		{
			result += L")";
		}
	}
	return VE_OK;
}


VError QueryNodeNot::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	VString s;
	Describe(s);
	s += L"\n";
	QueryNode::AddToQueryDescriptor(descriptor, level);
	descriptor += s;

	if (fRoot != nil)
		fRoot->AddToQueryDescriptor(descriptor, level+1);

	return VE_OK;
}


VError QueryNodeNot::PerformComplex(ComplexSelection* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, ComplexSelection* &outSel, DB4D_Way_of_Locking HowToLock, sLONG limit, VValueBag* curdesc)
{
	return VE_DB4D_NOTIMPLEMENTED;
}


				/* ---------------------------- */


VError QueryNodeCompositeJoin::AddToTargets(SetOfTargets& fullTarget)
{
	try
	{
		fullTarget.insert(fTarget1);
		fullTarget.insert(fTarget2);
		return VE_OK;
	}
	catch (...)
	{
		return ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
	}
}



CQTargetsDef* QueryNodeCompositeJoin::GenerateTargets(VError& err)
{
	err = VE_OK;
	CQTargetsDef* result = new CQTargetsDef;
	if (result == nil)
		err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
	else
	{
		if (!result->Add(fTarget1) || !result->Add(fTarget2))
		{
			err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
			result->Release();
			result = nil;
		}
	}
	return result;
}


SimpleQueryNode* QueryNodeCompositeJoin::ConvertToSimple(const CQTableDef& target, VError& err)
{
	err = ThrowBaseError(VE_DB4D_NOTIMPLEMENTED);
	return nil;
}



VError QueryNodeCompositeJoin::Describe(VString& result)
{
	result = L"[Composite Join] : ";
	bool first = true;
	for (vector<Field*>::iterator cur1 = fFields1.begin(), cur2 = fFields2.begin(), end1 = fFields1.end(), end2 = fFields2.end(); cur1 != end1 && cur2 != end2; cur1++, cur2++)
	{
		VString s; 
		VString s2;
		if (first)
			first = false;
		else
			s += L" and ";
		s += BuildStringFromCrit(*cur1, fTarget1.GetInstance());

		s += L" = ";

		s += BuildStringFromCrit(*cur2, fTarget2.GetInstance());

		result += s;
	}


	return VE_OK;
}


VError QueryNodeCompositeJoin::FullyDescribe(VString& result)
{
	return Describe(result);
}


VError QueryNodeCompositeJoin::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	VString s;

	QueryNode::AddToQueryDescriptor(descriptor, level);

	Describe(s);

	s += L"\n";

	descriptor += s;

	return VE_OK;
}

/*
IndexInfo* FindMatchingIndex(Table* curtarget, vector<Field*>& fields, BaseTaskInfo* context)
{
	IndexInfo* result = nil;
	curtarget->occupe();
	IndexArray indexes;
	curtarget->CopyIndexDep(indexes);
	curtarget->libere();

	IndexArray::Iterator cur = indexes.First(), end = indexes.End();

	for (; cur != end && result == nil; cur++)
	{
		IndexInfo* ind = *cur;
		if (ind != nil)
		{
			if (ind->MatchType(DB4D_Index_OnMultipleFields))
			{
				if (ind->AskForValid(context))
				{
					bool match = true;
					IndexInfoFromMultipleField* indm = (IndexInfoFromMultipleField*)ind;
					FieldRefArray* indexfields = indm->GetFields();
					for (FieldRefArray::Iterator cur = indexfields->First(), end = indexfields->End(); cur != end && match; cur++)
					{
						Field* cri = cur->crit;
						bool found = false;
						for (vector<Field*>::iterator curx = fields.begin(), endx = fields.end(); curx != endx && !found; curx++)
						{
							if (*curx == cri)
								found = true;
						}
						if (!found)
							match = false;
					}

					if (match)
					{
						result = ind;
						result->Retain();
					}
					else
						ind->ReleaseValid();
				}
			}
		}
	}
	return result;
}
*/

uBOOL QueryNodeCompositeJoin::IsIndexe(BaseTaskInfo* context)
{
	if (recalcIndex)
	{
		ForgetIndexes();
		bool isindex = true;

		Table* curtarget = fTarget1.GetTable();
		curtarget->occupe();
		IndexArray indexes;
		curtarget->CopyIndexDep(indexes);
		curtarget->libere();

		Table* curtarget2 = fTarget2.GetTable();
		curtarget2->occupe();
		IndexArray indexes2;
		curtarget2->CopyIndexDep(indexes2);
		curtarget2->libere();

		IndexArray::Iterator cur = indexes.First(), end = indexes.End();

		for (; cur != end && fInd1 == nil; cur++)
		{
			IndexInfo* ind = *cur;
			if (ind != nil)
			{
				if (ind->MatchType(DB4D_Index_OnMultipleFields))
				{
					if (ind->AskForValid(context))
					{
						vector<sLONG> fieldOrder;
						fieldOrder.resize(fFields1.size(), -1);

						bool match = true;
						IndexInfoFromMultipleField* indm = (IndexInfoFromMultipleField*)ind;
						FieldRefArray* indexfields = indm->GetFields();
						sLONG fieldpos = 0;
						for (FieldRefArray::Iterator cur = indexfields->First(), end = indexfields->End(); cur != end && match; cur++, fieldpos++)
						{
							Field* cri = cur->crit;
							bool found = false;
							vector<sLONG>::iterator curOrder = fieldOrder.begin();
							for (vector<Field*>::iterator curx = fFields1.begin(), endx = fFields1.end(); curx != endx && !found; curx++, curOrder++)
							{
								if (*curx == cri)
								{
									*curOrder = fieldpos;
									found = true;
								}
							}

							if (!found)
								match = false;
						}

						if (match)
						{
							for (vector<sLONG>::iterator curOrder = fieldOrder.begin(), endOrder = fieldOrder.end(); curOrder != endOrder && match; curOrder++)
							{
								if (*curOrder == -1)
									match = false;
							}
						}

						if (match)
						{
							for (IndexArray::Iterator cur2 = indexes2.First(), end2 = indexes2.End(); cur2 != end2 && fInd2 == nil; cur2++)
							{
								IndexInfo* ind2 = *cur2;
								if (ind2 != nil)
								{
									if (ind2->MatchType(DB4D_Index_OnMultipleFields))
									{
										if (ind2->AskForValid(context))
										{
											vector<sLONG> fieldOrder2;
											fieldOrder2.resize(fFields2.size(), -1);

											bool match2 = true;
											IndexInfoFromMultipleField* indm2 = (IndexInfoFromMultipleField*)ind2;
											FieldRefArray* indexfields2 = indm2->GetFields();
											sLONG fieldpos2 = 0;
											for (FieldRefArray::Iterator cur2 = indexfields2->First(), end2 = indexfields2->End(); cur2 != end2 && match2; cur2++, fieldpos2++)
											{
												Field* cri = cur2->crit;
												bool found2 = false;
												vector<sLONG>::iterator curOrder2 = fieldOrder2.begin();
												for (vector<Field*>::iterator curx = fFields2.begin(), endx = fFields2.end(); curx != endx && !found2; curx++, curOrder2++)
												{
													if (*curx == cri)
													{
														*curOrder2 = fieldpos2;
														found2 = true;
													}
												}
												if (!found2)
													match2 = false;
											}
											if (match2)
											{
												vector<sLONG>::iterator xcurOrder = fieldOrder.begin();
												for (vector<sLONG>::iterator curOrder2 = fieldOrder2.begin(), endOrder2 = fieldOrder2.end(); curOrder2 != endOrder2 && match2; curOrder2++, xcurOrder++)
												{
													if (*curOrder2 != *xcurOrder)
														match2 = false;
												}
											}

											if (match2)
											{
												fInd2 = ind2;
												fInd2->Retain();
											}
											else
												ind2->ReleaseValid();
										}
									}
								}
							}

							if (fInd2 == nil)
							{
								match = false;
							}
						}

						if (match)
						{
							fInd1 = ind;
							fInd1->Retain();
						}
						else
							ind->ReleaseValid();
					}
				}
			}
		}

		isindex = fInd1 != nil && fInd2 != nil;
		if (!isindex)
			ForgetIndexes();
		recalcIndex = false;
	}
	return isindex;
}


uBOOL QueryNodeCompositeJoin::IsAllIndexe(BaseTaskInfo* context)
{
	return IsIndexe(context);
}


VError QueryNodeCompositeJoin::AddJoinPart(Field* cri1, Field* cri2, bool leftjoin, bool rightjoin, const VCompareOptions& inOptions)
{
	if (leftjoin)
		fLeftJoin = true;
	if (rightjoin)
		fRightJoin = true;
	fOptions = inOptions;
	fFields1.push_back(cri1);
	fFields2.push_back(cri2);
	return VE_OK;
}


CompareResult CompareRawRecords(FicheOnDisk* rec1, FicheOnDisk* rec2, vector<Field*>& fields1, vector<Field*>& fields2, vector<const VValueInfo*>& criInfos, const VCompareOptions& inOptions, BaseTaskInfo* context)
{
	CompareResult comp = CR_EQUAL;

	vector<const VValueInfo*>::iterator curinfo = criInfos.begin();
	for (vector<Field*>::iterator cur1 = fields1.begin(), cur2 = fields2.begin(), end1 = fields1.end(), end2 = fields2.end(); cur1 != end1 && cur2 != end2 && comp == CR_EQUAL; cur1++, cur2++, curinfo++)
	{
		sLONG typondisk1 = 0, typondisk2 = 0;
		Field* cri1 = *cur1;
		Field* cri2 = * cur2;
		const VValueInfo* cri1Info = *curinfo;

		void* data1 = (void*)(rec1->GetDataPtrForQuery(cri1->GetPosInRec(), &typondisk1, false));
		void* data2 = (void*)(rec2->GetDataPtrForQuery(cri2->GetPosInRec(), &typondisk2, false));
		if (data1 != nil && data2 != nil)
		{
			if (typondisk1 == typondisk2)
			{
				comp = cri1Info->CompareTwoPtrToDataWithOptions(data1, data2, inOptions);
			}
			else
			{
				ValPtr cv = NewValPtr(typondisk2, data1, typondisk1, rec1->GetOwner(), context);
				comp = cv->CompareToSameKindPtrWithOptions(data2, inOptions);
				delete cv;
			}
		}
		else
		{
			if (data2 == nil)
			{
				comp = CR_BIGGER;
			}
			else
				comp = CR_SMALLER;
		}
	}
	return comp;
}


ComplexSelection* QueryNodeCompositeJoin::PerformComplexJoin(Bittab* filtre1, Bittab* filtre2, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock, VValueBag* curdesc)
{
	err = VE_OK;
	ComplexSelection* result = nil;

	uLONG start = fOwner->GetStartingTime();

	DataTable* df1 = fTarget1.GetTable()->GetDF();
	DataTable* df2 = fTarget2.GetTable()->GetDF();

	sLONG nbrec1, nbrec2;

	if (filtre1 == nil)
		nbrec1 = df1->GetNbRecords(context);
	else
		nbrec1 = filtre1->Compte();

	if (filtre2 == nil)
		nbrec2 = df2->GetNbRecords(context);
	else
		nbrec2 = filtre2->Compte();

	IndexInfo* ind1 = fInd1;
	IndexInfo* ind2 = fInd2;

	Boolean useindex1 = false;
	Boolean useindex2 = false;

	if (ind1 != nil)
	{
		if (ind1->GetNBDiskAccessForAScan(true) < ((sLONG8)nbrec1 * df1->GetSeqRatioCorrector()))
			useindex1 = true;
	}

	if (ind2 != nil)
	{
		if (ind2->GetNBDiskAccessForAScan(true) < ((sLONG8)nbrec2 * df2->GetSeqRatioCorrector()))
			useindex2 = true;
	}

	if (useindex1 && useindex2)
	{
		CQTableDefVector targets;
		try
		{
			targets.push_back(fTarget1);
			targets.push_back(fTarget2);
		}
		catch (...) 
		{
			err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
		}

		if (err == VE_OK)
		{
			result = new ComplexSelection(targets, fLeftJoin, fRightJoin);
			if (result != nil)
			{
				result->PutInCache();
				err = ind1->JoinWithOtherIndex(ind2, filtre1, filtre2, result, context, fOptions, InProgress, fLeftJoin, fRightJoin);
			}
			else
				err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
		}
	}
	else
	{
		SortTab sort1(fTarget1.GetTable()->GetOwner());
		for (vector<Field*>::iterator cur = fFields1.begin(), end = fFields1.end(); cur != end; cur++)
			sort1.AddTriLineField(*cur);
		Selection* sel1 = nil;

		SortTab sort2(fTarget2.GetTable()->GetOwner());
		for (vector<Field*>::iterator cur = fFields2.begin(), end = fFields2.end(); cur != end; cur++)
			sort2.AddTriLineField(*cur);
		Selection* sel2 = nil;

		Selection* TrueSel1 = nil;
		Selection* TrueSel2 = nil;

		fOptions.SetLike(false);
		fOptions.SetBeginsWith(false);

		fOptions.SetIntlManager(GetContextIntl(context));

		if (filtre1 != nil)
		{
			TrueSel1 = new BitSel(df1, filtre1, false);
			if (TrueSel1 == nil)
				err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
		}
		else
			TrueSel1 = df1->AllRecords(context, err);

		if (err == VE_OK)
		{
			if (filtre2 != nil)
			{
				TrueSel2 = new BitSel(df2, filtre2, false);
				if (TrueSel2 == nil)
					err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
			}
			else
				TrueSel2 = df2->AllRecords(context, err);
		}

		if (err == VE_OK)
		{
			sel1 = df1->SortSel(sort1, TrueSel1, context, InProgress, err);
			if (err == VE_OK)
			{
				sel2 = df2->SortSel(sort2, TrueSel2, context, InProgress, err);
			}
		}

		if (TrueSel1 != nil)
			TrueSel1->Release();
		if (TrueSel2 != nil)
			TrueSel2->Release();

		if (err == VE_OK)
		{
			CQTableDefVector targets;
			try
			{
				targets.push_back(fTarget1);
				targets.push_back(fTarget2);
			}
			catch (...) 
			{
				err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
			}

			if (err == VE_OK)
			{
				result = new ComplexSelection(targets, fLeftJoin, fRightJoin);
				if (result != nil)
				{
					result->PutInCache();
					Bittab kept1, kept2;
					Transaction* trans = GetCurrentTransaction(context);

					if (HowToLock == DB4D_Keep_Lock_With_Transaction && trans != nil)
					{
						Bittab* xkept = trans->GetKeptLocks(df1, err, false);
						if (xkept != nil)
							err = kept1.Or(xkept);
						if (err == VE_OK)
						{
							xkept = trans->GetKeptLocks(df2, err, false);
							if (xkept != nil)
								err = kept2.Or(xkept);
						}

					}

					if (err == VE_OK)
					{
						vector<const VValueInfo*> cr1info;
						cr1info.resize(fFields1.size(), nil);
						vector<const VValueInfo*>::iterator curinfo = cr1info.begin();
						for (vector<Field*>::iterator cur = fFields1.begin(), end = fFields1.end(); cur != end; cur++, curinfo++)
							*curinfo = VValue::ValueInfoFromValueKind((ValueKind)(*cur)->GetTyp());

						//SelectionIterator itersel1(sel1);
						//SelectionIterator itersel2(sel2);
						sLONG row1 = 0; //itersel1.FirstRecord();
						sLONG row2 = 0; //itersel2.FirstRecord();
						sLONG count1 = sel1->GetQTfic();
						sLONG count2 = sel2->GetQTfic();

						sLONG oldrow1 = -2, oldrow2 = -2;
						FicheInMem* curFiche1 = nil;
						FicheInMem* curFiche2 = nil;

						ValPtr cv = nil, cvJoin = nil;

						ComplexSelRow FinalRow; 
						FinalRow.resize(2, -1);

						FicheOnDisk* px1 = nil;
						FicheOnDisk* px2 = nil;
						while (row1 < count1 && row2 < count2 && err == VE_OK)
						{
							Boolean couldlock1 = true, couldlock2 = true;
							if (oldrow1 != row1)
							{
								oldrow1 = row1;
								if (px1 != nil)
								{
									px1->Release();
									//px1->FreeAfterUse();
									if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept1.isOn(px1->getnumfic()))
										df1->UnlockRecord(px1->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);
								}
								px1 = df1->LoadNotFullRecord(sel1->GetFic(row1), err, HowToLock, context, true, nil, &couldlock1);
							}

							if (err != VE_OK || px1 == nil)
							{
								row1++; //row1 = itersel1.NextRecord();
							}
							else
							{
								if (oldrow2 != row2)
								{
									oldrow2 = row2;
									if (px2 != nil)
									{
										px2->Release();
										//px2->FreeAfterUse();
										if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept2.isOn(px2->getnumfic()))
											df2->UnlockRecord(px2->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);
									}
									px2 = df2->LoadNotFullRecord(sel2->GetFic(row2), err, HowToLock, context, true, nil, &couldlock2);
								}
								if (err != VE_OK || px2 == nil)
								{
									row2++; //row2 = itersel2.NextRecord();
								}
								else
								{
									if (err == VE_OK)
									{
										CompareResult comp = CompareRawRecords(px1, px2, fFields1, fFields2, cr1info, fOptions, context);

										if (comp == CR_EQUAL)
										{
											//SelectionIterator itersel1x(itersel1);
											sLONG row1x = row1;
											Boolean cont1 = true;
											sLONG lastrow2x;
											do
											{
												if (row1x >= count1)
												{
													cont1 = false;
												}
												else
												{
													CompareResult comp;
													FicheOnDisk* px1x = df1->LoadNotFullRecord(sel1->GetFic(row1x), err, HowToLock, context, true, nil, &couldlock1);
													if (err == VE_OK)
													{
														comp = CompareRawRecords(px1x, px2, fFields1, fFields2, cr1info, fOptions, context);
													}
													else
													{
														err = VE_OK;
														comp = CR_SMALLER;
													}

													if (comp == CR_EQUAL)
													{							
														//SelectionIterator itersel2x(itersel2);
														sLONG row2x = row2;
														Boolean cont2 = true;
														do
														{
															if (row2x >= count2)
															{
																lastrow2x = row2x;
																cont2 = false;
															}
															else
															{
																FicheOnDisk* px2x = df2->LoadNotFullRecord(sel2->GetFic(row2x), err, HowToLock, context, true, nil, &couldlock2);
																CompareResult comp = CompareRawRecords(px1x, px2x, fFields1, fFields2, cr1info, fOptions, context);
																if (comp == CR_EQUAL)
																{
																	if (HowToLock == DB4D_Do_Not_Lock || (couldlock1 && couldlock2))
																	{
																		if (HowToLock != DB4D_Do_Not_Lock)
																		{
																			err = kept1.Set(px1->getnumfic(), true);
																			if (err == VE_OK)
																				err = kept2.Set(px2->getnumfic(), true);
																		}
																		if (err == VE_OK)
																		{
																			FinalRow[0] = px1x->getnumfic(); // sel1->GetFic(row1x);
																			FinalRow[1] = px2x->getnumfic(); // sel2->GetFic(row2x);
																			err = result->AddRow(FinalRow);
																			row2x++; //row2x = itersel2x.NextRecord();
																		}
																	}
																	else
																	{
																		if (!couldlock1)
																		{
																			err = df1->ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_ExecutingComplexQuery);
																		}
																		else
																		{
																			err = df2->ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_ExecutingComplexQuery);
																		}
																	}
																}
																else
																{
																	lastrow2x = row2x;
																	cont2 = false;
																}
																if (px2x != nil)
																{
																	px2x->Release();
																	//px2x->FreeAfterUse();
																	if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept2.isOn(px2->getnumfic()))
																		df2->UnlockRecord(px2->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);
																}
															}
														} while (cont2 && err == VE_OK);
														row1x++; //row1x = itersel1x.NextRecord();
													}
													else
													{
														cont1 = false;
													}
													if (px1x != nil)
													{
														//px1x->FreeAfterUse();
														px1x->Release();
														if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept1.isOn(px1->getnumfic()))
															df1->UnlockRecord(px1->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);
													}
												}

											} while (cont1 && err == VE_OK);

											row1 = row1x;
											row2 = lastrow2x;
										}
										else
										{
											if (comp == CR_SMALLER)
											{
												if (fLeftJoin)
												{
													if (filtre2 == nil)
													{
														FinalRow[0] = px1->getnumfic();
														FinalRow[1] = -2;
														err = result->AddRow(FinalRow);
													}
												}
												row1++; //row1 = itersel1.NextRecord();
											}
											else
											{
												if (fRightJoin)
												{
													if (filtre1 == nil)
													{
														FinalRow[0] = -2;
														FinalRow[1] = px2->getnumfic();
														err = result->AddRow(FinalRow);
													}
												}
												row2++; //row2 = itersel2.NextRecord();
											}
										}
									}
								}
							}
						}

						if (err == VE_OK)
						{
							if (fLeftJoin && filtre2 == nil)
							{
								while (row1 < count1 && err == VE_OK)
								{
									Boolean couldlock1 = true;
									if (px1 != nil)
									{
										px1->Release();
										//px1->FreeAfterUse();
										if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept1.isOn(px1->getnumfic()))
											df1->UnlockRecord(px1->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);
									}
									px1 = df1->LoadNotFullRecord(sel1->GetFic(row1), err, HowToLock, context, true, nil, &couldlock1);
									if (HowToLock == DB4D_Do_Not_Lock || couldlock1)
									{
										if (px1 != nil)
										{
											FinalRow[0] = px1->getnumfic();
											FinalRow[1] = -2;
											err = result->AddRow(FinalRow);
										}
									}
									else
										err = df1->ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_ExecutingComplexQuery);

									row1++;
								}
							}

							if (fRightJoin && filtre1 == nil)
							{
								while (row2 < count2 && err == VE_OK)
								{
									Boolean couldlock2 = true;
									if (px2 != nil)
									{
										px2->Release();
										if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept2.isOn(px2->getnumfic()))
											df2->UnlockRecord(px2->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);
									}
									px2 = df2->LoadNotFullRecord(sel2->GetFic(row2), err, HowToLock, context, true, nil, &couldlock2);
									if (HowToLock == DB4D_Do_Not_Lock || couldlock2)
									{
										if (px2 != nil)
										{
											FinalRow[0] = -2;
											FinalRow[1] = px2->getnumfic();
											err = result->AddRow(FinalRow);
										}
									}
									else
										err = df2->ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_ExecutingComplexQuery);

									row2++;
								}
							}

						}

						if (px1 != nil)
						{
							px1->Release();
							if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept1.isOn(px1->getnumfic()))
								df1->UnlockRecord(px1->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);

						}
						px1 = nil;

						if (px2 != nil)
						{
							px2->Release();
							if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept2.isOn(px2->getnumfic()))
								df2->UnlockRecord(px2->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);
						}
						px2 = nil;
					}
				}
				else
					err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
			}
		}

		if (sel1 != nil)
			sel1->Release();
		if (sel2 != nil)
			sel2->Release();
	}

	if (err != VE_OK && result != nil)
	{
		result->Release();
		result = nil;
	}
	else
	{
		if (result != nil)
		{
			if (fOwner->IsQueryDescribed())
			{
				VString s;
				FullyDescribe(s);
				fOwner->AddToDescription(curdesc, s, start, *result);
			}
		}
	}

	return result;
}




				/* ---------------------------- */


VError QueryNodeJoin::AddToTargets(SetOfTargets& fullTarget)
{
	try
	{
		fullTarget.insert(fTarget1);
		fullTarget.insert(fTarget2);
		return VE_OK;
	}
	catch (...)
	{
		return ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
	}
}


CQTargetsDef* QueryNodeJoin::GenerateTargets(VError& err)
{
	err = VE_OK;
	CQTargetsDef* result = new CQTargetsDef;
	if (result == nil)
		err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
	else
	{
		if (!result->Add(fTarget1) || !result->Add(fTarget2))
		{
			err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
			result->Release();
			result = nil;
		}
	}
	return result;
}


SimpleQueryNode* QueryNodeJoin::ConvertToSimple(const CQTableDef& target, VError& err)
{
	err = VE_OK;
	SimpleQueryNodeJoin* jn = new SimpleQueryNodeJoin(fOwner, target, rt);
	if (jn == nil)
		err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);

	return jn;
}


VError QueryNodeJoin::Describe(VString& result)
{
	VString s = L"[Join] : ";
	VString s2;
	s += BuildStringFromCrit(rt->cri1, rt->fNumInstance1);

	switch (rt->comparaison)
	{
	case DB4D_BeginsWith:
		s2 = L" Begin with ";
		break;

	case DB4D_Equal:
		s2 = L" = ";
		break;

	case DB4D_Like:
		s2 = L" LIKE ";
		break;

	case DB4D_NotEqual:
		s2 = L" # ";
		break;

	case DB4D_NotLike:
		s2 = L" NOT LIKE ";
		break;

	case DB4D_Greater:
	case DB4D_Greater_Like:
		s2 = L" > ";
		break;

	case DB4D_GreaterOrEqual:
	case DB4D_GreaterOrEqual_Like:
		s2 = L" >= ";
		break;

	case DB4D_Lower:
	case DB4D_Lower_Like:
		s2 = L" < ";
		break;

	case DB4D_LowerOrEqual:
	case DB4D_LowerOrEqual_Like:
		s2 = L" <= ";
		break;

	case DB4D_Contains_KeyWord:
		s2 = L" contains ";
		break;

	default:
		s2 = L" <Nop> ";
		break;
	}

	s += s2;

	s += BuildStringFromCrit(rt->cri2, rt->fNumInstance2);

	result = s;

	return VE_OK;
}


VError QueryNodeJoin::FullyDescribe(VString& result)
{
	return Describe(result);
}


VError QueryNodeJoin::AddToQueryDescriptor(VString& descriptor, sLONG level)
{
	VString s;

	QueryNode::AddToQueryDescriptor(descriptor, level);

	Describe(s);

	s += L"\n";

	descriptor += s;

	return VE_OK;
}



uBOOL QueryNodeJoin::IsIndexe(BaseTaskInfo* context)
{
	if (recalcIndex)
	{
		isindex = rt->cri1->IsIndexed() || rt->cri2->IsIndexed();
		recalcIndex = false;
	}
	return isindex;
}


uBOOL QueryNodeJoin::IsAllIndexe(BaseTaskInfo* context)
{
	return IsIndexe(context);
}


CompareResult CompareRawRecords(FicheOnDisk* rec1, FicheOnDisk* rec2, Field* cri1, Field* cri2, const VValueInfo* cri1Info, const VCompareOptions& inOptions, BaseTaskInfo* context)
{
	CompareResult comp = CR_SMALLER;
	sLONG typondisk1 = 0, typondisk2 = 0;
	void* data1 = (void*)(rec1->GetDataPtrForQuery(cri1->GetPosInRec(), &typondisk1, false));
	void* data2 = (void*)(rec2->GetDataPtrForQuery(cri2->GetPosInRec(), &typondisk2, false));
	if (data1 != nil && data2 != nil)
	{
		if (typondisk1 == typondisk2)
		{
			comp = cri1Info->CompareTwoPtrToDataWithOptions(data1, data2, inOptions);
		}
		else
		{
			ValPtr cv = NewValPtr(typondisk2, data1, typondisk1, rec1->GetOwner(), context);
			comp = cv->CompareToSameKindPtrWithOptions(data2, inOptions);
			delete cv;
		}
	}
	else
	{
		if (data2 == nil)
		{
			comp = CR_BIGGER;
		}
	}
	return comp;
}


ComplexSelection* QueryNodeJoin::PerformComplexJoin(Bittab* filtre1, Bittab* filtre2, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock, VValueBag* curdesc)
{
	err = VE_OK;
	ComplexSelection* result = nil;

	uLONG start = fOwner->GetStartingTime();

	DataTable* df1 = rt->cri1->GetOwner()->GetDF();
	DataTable* df2 = rt->cri2->GetOwner()->GetDF();

	sLONG nbrec1, nbrec2;

	if (filtre1 == nil)
		nbrec1 = df1->GetNbRecords(context);
	else
		nbrec1 = filtre1->Compte();

	if (filtre2 == nil)
		nbrec2 = df2->GetNbRecords(context);
	else
		nbrec2 = filtre2->Compte();

	IndexInfo* ind1 = rt->cri1->FindAndRetainIndexSimple(true, true, context);
	IndexInfo* ind2 = rt->cri2->FindAndRetainIndexSimple(true, true, context);

	Boolean useindex1 = false;
	Boolean useindex2 = false;

	if (ind1 != nil)
	{
		if (ind1->GetNBDiskAccessForAScan(true) < ((sLONG8)nbrec1 * df1->GetSeqRatioCorrector()))
			useindex1 = true;
	}

	if (ind2 != nil)
	{
		if ((ind2->GetNBDiskAccessForAScan(true) < ((sLONG8)nbrec2 * df2->GetSeqRatioCorrector())) || ind1 != 0 )
			useindex2 = true;
	}

	if (useindex1 && useindex2 && ind1->MatchType(ind2) && ind1->MatchOtherDataKind(ind2) )
	{
		CQTableDefVector targets;
		try
		{
			targets.push_back(fTarget1);
			targets.push_back(fTarget2);
		}
		catch (...) 
		{
			err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
		}

		if (err == VE_OK)
		{
			result = new ComplexSelection(targets, rt->fLeftJoin, rt->fRightJoin);
			if (result != nil)
			{
				result->PutInCache();
				err = ind1->JoinWithOtherIndex(ind2, filtre1, filtre2, result, context, rt->fOptions, InProgress, rt->fLeftJoin, rt->fRightJoin);
			}
			else
				err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
		}

		if (ind1 != nil)
			ind1->ReleaseValid();

		if (ind2 != nil)
			ind2->ReleaseValid();
	}
	else
	{
		if (ind1 != nil)
			ind1->ReleaseValid();

		if (ind2 != nil)
			ind2->ReleaseValid();

		SortTab sort1(rt->cri1->GetOwner()->GetOwner());
		sort1.AddTriLineField(rt->cri1);
		Selection* sel1 = nil;

		SortTab sort2(rt->cri2->GetOwner()->GetOwner());
		sort2.AddTriLineField(rt->cri2);
		Selection* sel2 = nil;

		Selection* TrueSel1 = nil;
		Selection* TrueSel2 = nil;

		rt->fOptions.SetLike(false);
		rt->fOptions.SetBeginsWith(false);

		if (rt->comparaison == DB4D_Like)
			rt->fOptions.SetLike(true);
		else
		{
			if (rt->comparaison == DB4D_BeginsWith)
			{
				rt->fOptions.SetBeginsWith(true);
			}
		}
		rt->fOptions.SetIntlManager(GetContextIntl(context));

		if (filtre1 != nil)
		{
			TrueSel1 = new BitSel(df1, filtre1, false);
			if (TrueSel1 == nil)
				err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
		}
		else
			TrueSel1 = df1->AllRecords(context, err);

		if (err == VE_OK)
		{
			if (filtre2 != nil)
			{
				TrueSel2 = new BitSel(df2, filtre2, false);
				if (TrueSel2 == nil)
					err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
			}
			else
				TrueSel2 = df2->AllRecords(context, err);
		}

		if (err == VE_OK)
		{
			sel1 = df1->SortSel(sort1, TrueSel1, context, InProgress, err);
			if (err == VE_OK)
			{
				sel2 = df2->SortSel(sort2, TrueSel2, context, InProgress, err);
			}
		}

		if (TrueSel1 != nil)
			TrueSel1->Release();
		if (TrueSel2 != nil)
			TrueSel2->Release();

		if (err == VE_OK)
		{
			CQTableDefVector targets;
			try
			{
				targets.push_back(fTarget1);
				targets.push_back(fTarget2);
			}
			catch (...) 
			{
				err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
			}

			if (err == VE_OK)
			{
				result = new ComplexSelection(targets, rt->fLeftJoin, rt->fRightJoin);
				if (result != nil)
				{
					result->PutInCache();
					Bittab kept1, kept2;
					Transaction* trans = GetCurrentTransaction(context);

					if (HowToLock == DB4D_Keep_Lock_With_Transaction && trans != nil)
					{
						Bittab* xkept = trans->GetKeptLocks(df1, err, false);
						if (xkept != nil)
							err = kept1.Or(xkept);
						if (err == VE_OK)
						{
							xkept = trans->GetKeptLocks(df2, err, false);
							if (xkept != nil)
								err = kept2.Or(xkept);
						}

					}

					if (err == VE_OK)
					{
						const VValueInfo* cr1info = VValue::ValueInfoFromValueKind((ValueKind)rt->cri1->GetTyp());

						//SelectionIterator itersel1(sel1);
						//SelectionIterator itersel2(sel2);
						sLONG row1 = 0; //itersel1.FirstRecord();
						sLONG row2 = 0; //itersel2.FirstRecord();
						sLONG count1 = sel1->GetQTfic();
						sLONG count2 = sel2->GetQTfic();

						sLONG oldrow1 = -2, oldrow2 = -2;
						FicheInMem* curFiche1 = nil;
						FicheInMem* curFiche2 = nil;

						ValPtr cv = nil, cvJoin = nil;

						ComplexSelRow FinalRow; 
						FinalRow.resize(2, -1);

						FicheOnDisk* px1 = nil;
						FicheOnDisk* px2 = nil;
						while (row1 < count1 && row2 < count2 && err == VE_OK)
						{
							Boolean couldlock1 = true, couldlock2 = true;
							if (oldrow1 != row1)
							{
								oldrow1 = row1;
								if (px1 != nil)
								{
									px1->Release();
									//px1->FreeAfterUse();
									if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept1.isOn(px1->getnumfic()))
										df1->UnlockRecord(px1->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);
								}
								px1 = df1->LoadNotFullRecord(sel1->GetFic(row1), err, HowToLock, context, true, nil, &couldlock1);
							}

							if (err != VE_OK || px1 == nil)
							{
								row1++; //row1 = itersel1.NextRecord();
							}
							else
							{
								if (oldrow2 != row2)
								{
									oldrow2 = row2;
									if (px2 != nil)
									{
										px2->Release();
										//px2->FreeAfterUse();
										if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept2.isOn(px2->getnumfic()))
											df2->UnlockRecord(px2->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);
									}
									px2 = df2->LoadNotFullRecord(sel2->GetFic(row2), err, HowToLock, context, true, nil, &couldlock2);
								}
								if (err != VE_OK || px2 == nil)
								{
									row2++; //row2 = itersel2.NextRecord();
								}
								else
								{
									if (err == VE_OK)
									{
										CompareResult comp = CompareRawRecords(px1, px2, rt->cri1, rt->cri2, cr1info, rt->fOptions, context);

										if (comp == CR_EQUAL)
										{
											//SelectionIterator itersel1x(itersel1);
											sLONG row1x = row1;
											Boolean cont1 = true;
											sLONG lastrow2x;
											do
											{
												if (row1x >= count1)
												{
													cont1 = false;
												}
												else
												{
													CompareResult comp;
													FicheOnDisk* px1x = df1->LoadNotFullRecord(sel1->GetFic(row1x), err, HowToLock, context, true, nil, &couldlock1);
													if (err == VE_OK)
													{
														comp = CompareRawRecords(px1x, px2, rt->cri1, rt->cri2, cr1info, rt->fOptions, context);
													}
													else
													{
														err = VE_OK;
														comp = CR_SMALLER;
													}

													if (comp == CR_EQUAL)
													{							
														//SelectionIterator itersel2x(itersel2);
														sLONG row2x = row2;
														Boolean cont2 = true;
														do
														{
															if (row2x >= count2)
															{
																lastrow2x = row2x;
																cont2 = false;
															}
															else
															{
																FicheOnDisk* px2x = df2->LoadNotFullRecord(sel2->GetFic(row2x), err, HowToLock, context, true, nil, &couldlock2);
																CompareResult comp = CompareRawRecords(px1x, px2x, rt->cri1, rt->cri2, cr1info, rt->fOptions, context);
																if (comp == CR_EQUAL)
																{
																	if (HowToLock == DB4D_Do_Not_Lock || (couldlock1 && couldlock2))
																	{
																		if (HowToLock != DB4D_Do_Not_Lock)
																		{
																			err = kept1.Set(px1->getnumfic(), true);
																			if (err == VE_OK)
																				err = kept2.Set(px2->getnumfic(), true);
																		}
																		if (err == VE_OK)
																		{
																			FinalRow[0] = px1x->getnumfic(); // sel1->GetFic(row1x);
																			FinalRow[1] = px2x->getnumfic(); // sel2->GetFic(row2x);
																			err = result->AddRow(FinalRow);
																			row2x++; //row2x = itersel2x.NextRecord();
																		}
																	}
																	else
																	{
																		if (!couldlock1)
																		{
																			err = df1->ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_ExecutingComplexQuery);
																		}
																		else
																		{
																			err = df2->ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_ExecutingComplexQuery);
																		}
																	}
																}
																else
																{
																	lastrow2x = row2x;
																	cont2 = false;
																}
																if (px2x != nil)
																{
																	px2x->Release();
																	//px2x->FreeAfterUse();
																	if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept2.isOn(px2->getnumfic()))
																		df2->UnlockRecord(px2->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);
																}
															}
														} while (cont2 && err == VE_OK);
														row1x++; //row1x = itersel1x.NextRecord();
													}
													else
													{
														cont1 = false;
													}
													if (px1x != nil)
													{
														//px1x->FreeAfterUse();
														px1x->Release();
														if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept1.isOn(px1->getnumfic()))
															df1->UnlockRecord(px1->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);
													}
												}

											} while (cont1 && err == VE_OK);

											row1 = row1x;
											row2 = lastrow2x;
										}
										else
										{
											if (comp == CR_SMALLER)
											{
												if (rt->fLeftJoin)
												{
													FinalRow[0] = px1->getnumfic();
													FinalRow[1] = -2;
													err = result->AddRow(FinalRow);
												}
												row1++; //row1 = itersel1.NextRecord();
											}
											else
											{
												if (rt->fRightJoin)
												{
													FinalRow[0] = -2;
													FinalRow[1] = px2->getnumfic();
													err = result->AddRow(FinalRow);
												}
												row2++; //row2 = itersel2.NextRecord();
											}
										}
									}
								}
							}
						}

						if (err == VE_OK)
						{
							if (rt->fLeftJoin)
							{
								while (row1 < count1 && err == VE_OK)
								{
									Boolean couldlock1 = true;
									if (px1 != nil)
									{
										px1->Release();
										//px1->FreeAfterUse();
										if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept1.isOn(px1->getnumfic()))
											df1->UnlockRecord(px1->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);
									}
									px1 = df1->LoadNotFullRecord(sel1->GetFic(row1), err, HowToLock, context, true, nil, &couldlock1);
									if (HowToLock == DB4D_Do_Not_Lock || couldlock1)
									{
										if (px1 != nil)
										{
											FinalRow[0] = px1->getnumfic();
											FinalRow[1] = -2;
											err = result->AddRow(FinalRow);
										}
									}
									else
										err = df1->ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_ExecutingComplexQuery);

									row1++;
								}
							}

							if (rt->fRightJoin)
							{
								while (row2 < count2 && err == VE_OK)
								{
									Boolean couldlock2 = true;
									if (px2 != nil)
									{
										px2->Release();
										if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept2.isOn(px2->getnumfic()))
											df2->UnlockRecord(px2->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);
									}
									px2 = df2->LoadNotFullRecord(sel2->GetFic(row2), err, HowToLock, context, true, nil, &couldlock2);
									if (HowToLock == DB4D_Do_Not_Lock || couldlock2)
									{
										if (px2 != nil)
										{
											FinalRow[0] = -2;
											FinalRow[1] = px2->getnumfic();
											err = result->AddRow(FinalRow);
										}
									}
									else
										err = df2->ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_ExecutingComplexQuery);

									row2++;
								}
							}

						}

						if (px1 != nil)
						{
							px1->Release();
							if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept1.isOn(px1->getnumfic()))
								df1->UnlockRecord(px1->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);

						}
						px1 = nil;

						if (px2 != nil)
						{
							px2->Release();
							if (HowToLock == DB4D_Keep_Lock_With_Transaction && !kept2.isOn(px2->getnumfic()))
								df2->UnlockRecord(px2->getnumfic(), context, DB4D_Keep_Lock_With_Transaction);
						}
						px2 = nil;
					}
				}
				else
					err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
			}
		}

		if (sel1 != nil)
			sel1->Release();
		if (sel2 != nil)
			sel2->Release();
	}

	if (ind1 != nil)
		ind1->Release();

	if (ind2 != nil)
		ind2->Release();

	if (err != VE_OK && result != nil)
	{
		result->Release();
		result = nil;
	}
	else
	{
		if (result != nil)
		{
			if (fOwner->IsQueryDescribed())
			{
				VString s;
				FullyDescribe(s);
				fOwner->AddToDescription(curdesc, s, start, *result);
			}
		}
	}

	return result;
}



			/* ---------------------------- */


uBOOL QueryNodeBaseJoin::IsIndexe(BaseTaskInfo* context)
{
	uBOOL result = false;

	if (fRoot1 != nil)
		result = result || fRoot1->IsIndexe(context);

	if (fRoot2 != nil)
		result = result || fRoot2->IsIndexe(context);

	if (fJoinRoot != nil)
		result = result || fJoinRoot->IsIndexe(context);

	return result;
}


uBOOL QueryNodeBaseJoin::IsAllIndexe(BaseTaskInfo* context)
{
	uBOOL result = true;

	if (fRoot1 != nil)
		result = result && fRoot1->IsAllIndexe(context);

	if (fRoot2 != nil)
		result = result && fRoot2->IsAllIndexe(context);

	if (fJoinRoot != nil)
		result = result && fJoinRoot->IsAllIndexe(context);
	else
		result = false;

	return result;
}


VError QueryNodeBaseJoin::Describe(VString& result)
{
	if (fJoinRoot != nil)
		result = L"[Merge] : "+BuildStringFromTarget(fTarget1) + L" with " + BuildStringFromTarget(fTarget2);
		
	else
	{
		result = L"[cartesian product] : "+BuildStringFromTarget(fTarget1) + L" with " + BuildStringFromTarget(fTarget2);
	}
	return VE_OK;
}


VError QueryNodeBaseJoin::FullyDescribe(VString& result)
{
	VString s;
	if (fJoinRoot != nil)
		fJoinRoot->FullyDescribe(s);
	else
	{
		s = L"[cartesian product] : "+BuildStringFromTarget(fTarget1) + L" with " + BuildStringFromTarget(fTarget2);
	}
	result += s;
	if (fRoot1 != nil)
	{
		fRoot1->FullyDescribe(s);
		result += L" with filter {";
		result += s;
		result += L"}";
	}
	if (fRoot2 != nil)
	{
		fRoot2->FullyDescribe(s);
		if (fRoot1 == nil)
			result += L" with filter {";
		else
			result  += L" and with filter {";
		result += s;
		result += L"}";
	}
	return VE_OK;
}


VError QueryNodeBaseJoin::AddToQueryDescriptor(VString& descriptor, sLONG level)
{

	VString s2;
	VString s;

	if (fJoinRoot != nil)
	{
		QueryNode::AddToQueryDescriptor(descriptor, level);
		s = L"[Base join] : \n";
		descriptor += s;
		//fJoinRoot->Describe(s);
		fJoinRoot->AddToQueryDescriptor(descriptor, level+1);
	}
	else
	{
		QueryNode::AddToQueryDescriptor(descriptor, level);
		s = L"[cartesian product] : "+BuildStringFromTarget(fTarget1) + L" with " + BuildStringFromTarget(fTarget2);
		s += L"\n";
		descriptor += s;
	}

	if (fRoot1 != nil)
	{
		QueryNode::AddToQueryDescriptor(descriptor, level);
		descriptor += L"With filter :\n";
		fRoot1->AddToQueryDescriptor(descriptor, level+1);
	}
	if (fRoot2 != nil)
	{
		QueryNode::AddToQueryDescriptor(descriptor, level);
		if (fRoot1 == nil)
			descriptor += L"With filter :\n";
		else
			descriptor += L"And with filter :\n";
		fRoot2->AddToQueryDescriptor(descriptor, level+1);
	}

	return VE_OK;
}



VError QueryNodeBaseJoin::AddToTargets(SetOfTargets& fullTarget)
{
	try
	{
		fullTarget.insert(fTarget1);
		fullTarget.insert(fTarget2);
		return VE_OK;
	}
	catch (...)
	{
		return ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
	}
}


CQTargetsDef* QueryNodeBaseJoin::GenerateTargets(VError& err)
{
	err = VE_OK;
	CQTargetsDef* result = new CQTargetsDef;
	if (result == nil)
		err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
	else
	{
		if (!result->Add(fTarget1) || !result->Add(fTarget2))
		{
			err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
			result->Release();
			result = nil;
		}
	}
	return result;
}


sLONG QueryNodeBaseJoin::GetSubPoids(BaseTaskInfo* context)
{
	sLONG res = 0;
	if (fRoot1 != nil)
		res += fRoot1->GetPoids(context);
	if (fRoot2 != nil)
		res += fRoot2->GetPoids(context);
	return res;
}



sLONG QueryNodeBaseJoin::GetJoinPoids(BaseTaskInfo* context)
{
	if (fJoinRoot->IsIndexe(context))
		return 1000;
	else
		return 10000;
}


VError QueryNodeBaseJoin::PerformComplex(ComplexSelection* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, ComplexSelection* &outSel, DB4D_Way_of_Locking HowToLock, sLONG limit, VValueBag* curdesc)
{
	VError err = VE_OK;
	outSel = nil;
	Bittab *filtre1 = nil, *filtre2 = nil;
	Bittab* exceptions = nil;
	Bittab* xfiltre1 = nil, *xfiltre2 = nil;

	uLONG start = fOwner->GetStartingTime();
	VValueBag* subline = nil;
	if (fOwner->IsQueryDescribed())
	{
		VString s;
		Describe(s);
		subline = fOwner->AddToDescription(curdesc, s, start, nil);
	}

	if (fRoot1 != nil)
	{
		if (testAssert(fRoot1->isSimple()))
		{
			SimpleQueryNode* sqn = (SimpleQueryNode*)fRoot1.Get();
			if (filtre != nil)
			{
				sLONG column = filtre->FindTarget(fTarget1);
				if (column != -1)
					xfiltre1 = filtre->GenereBittab(column, context, err);
			}
			if (err == VE_OK)
			{
				if (sqn->IsConst())
				{
					filtre1 = new Bittab;
					if (filtre1 == nil)
						err = ThrowBaseError(memfull, noaction);
				}
				else
				{
					Bittab nulls;
					err = sqn->Perform(&filtre1, xfiltre1, InProgress, context, HowToLock, exceptions, limit, nulls, subline);
				}
			}
			ReleaseRefCountable(&xfiltre1);
		}
	}
	
	if (fRoot2 != nil && err == VE_OK)
	{
		if (testAssert(fRoot2->isSimple()))
		{
			SimpleQueryNode* sqn = (SimpleQueryNode*)fRoot2.Get();
			if (filtre != nil)
			{
				sLONG column = filtre->FindTarget(fTarget2);
				if (column != -1)
					xfiltre2 = filtre->GenereBittab(column, context, err);
			}
			if (err == VE_OK)
			{
				if (sqn->IsConst())
				{
					filtre2 = new Bittab;
					if (filtre2 == nil)
						err = ThrowBaseError(memfull, noaction);
				}
				else
				{
					Bittab nulls;
					err = sqn->Perform(&filtre2, xfiltre2, InProgress, context, HowToLock, exceptions, limit, nulls, subline);
				}
			}
			ReleaseRefCountable(&xfiltre2);
		}
	}

	if (err == VE_OK)
	{
		outSel = fJoinRoot->PerformComplexJoin(filtre1, filtre2, InProgress, context, err, HowToLock, subline);
	}

	ReleaseRefCountable(&filtre1);
	ReleaseRefCountable(&filtre2);

	if (outSel != nil && err != VE_OK)
	{
		outSel->Release();
		outSel = nil;
	}

	if (outSel != nil && fOwner->IsQueryDescribed())
	{
		subline->SetLong(QueryKeys::time, VSystem::GetCurrentTime() - start);
		subline->SetLong(QueryKeys::recordsfounds, outSel->GetNbRows());
	}

	return err;
}





			/* ----------------------------------------------------------- */


QueryNodeAggregate::~QueryNodeAggregate()
{
	/*
	for (QueryNodeMap::iterator cur = fNodes.begin(), end = fNodes.end(); cur != end; cur++)
	{
		CQTargetsDef* x = (CQTargetsDef*)(cur->first);
		if (x != nil)
			delete x;
		QueryNodeVector* nodes = cur->second;

		for (QueryNodeVector::iterator curn = nodes->begin(), endn = nodes->end(); curn != endn; curn++)
		{
			if (*curn != nil)
				delete *curn;
		}
		delete nodes;
	}
	*/
}


Boolean QueryNodeAggregate::AddNode(CQTargetsDef* cle, QueryNode* Node)
{
	Boolean ok = false;
	try
	{
		QueryNodeMap::iterator found = fNodes.find(cle);
		if (found == fNodes.end())
		{
			VRefPtr<CQNodesContent> nodes(new CQNodesContent, false);
			if (nodes != nil)
			{
				if (nodes->Add(Node))
				{
					fNodes.insert(make_pair(cle, nodes));
					ok = true;
				}
			}
		}
		else
		{
			CQNodesContent* nodes = found->second;
			if (nodes->Add(Node))
				ok = true;
		}
	}
	catch (...)
	{
	}
	return ok;
}


Boolean QueryNodeAggregate::AddNode(const CQTableDef& cle, QueryNode* Node)
{
	VRefPtr<CQTargetsDef> targets(new CQTargetsDef, false);
	if (targets == nil)
		return false;
	if (targets->Add(cle))
		return AddNode(targets, Node);
	else
		return false;
}



			/* ---------------------------- */



bool CompareLessCQTableDefVector(CQTargetsDef* x1, CQTargetsDef* x2)
{
	if (x1 == nil || x2 == nil)
	{
		return x1 < x2;
	}
	else
	{
		sLONG x1len = (sLONG)x1->fTargets.size();
		sLONG x2len = (sLONG)x2->fTargets.size();
		sLONG len = x1len;
		if (x1len > x2len)
			len = x2len;

		CQTableDefVector::const_iterator cur1 = x1->fTargets.begin(), cur2 = x2->fTargets.begin();

		for (sLONG i = 0; i < len; i++, cur1++, cur2++)
		{
			if (*cur1 == *cur2)
			{
			}
			else
			{
				return (*cur1 < *cur2);
			}
		}

		return x1len < x2len;
	}
}


			/* ----------------------------------- */


Boolean CQNodesContent::Add(QueryNode* node)
{
	try
	{
		fnodes.push_back(node);
		return true;
	}
	catch (...)
	{
		return false;
	}
}


			/* ----------------------------------- */

CQTableDef::CQTableDef(const QueryTarget& other)
{
	fTable = dynamic_cast<VDB4DTable*>(other.first)->GetTable();
	if (fTable != nil)
		fTable->Retain();
	fNumInstance = other.second;
}


CQTableDef& CQTableDef::operator = ( const QueryTarget& inOther)
{
	CopyRefCountable( &fTable, dynamic_cast<VDB4DTable*>(inOther.first)->GetTable());
	fNumInstance = inOther.second;
	return *this;
}


Boolean CQTargetsDef::Add(const CQTableDef& target)
{
	try
	{
		fTargets.push_back(target);
		return true;
	}
	catch (...)
	{
		return false;
	}
}


bool CQTargetsDef::AlreadyHas(const CQTableDef& target) const
{
	bool res = false;

	for (CQTableDefVector::const_iterator cur = fTargets.begin(), end = fTargets.end(); cur != end && !res; cur++)
	{
		if (*cur == target)
			res = true;
	}

	return res;
}


bool CQTargetsDef::MatchAtLeastOne(const CQTargetsDef& other)
{
	bool res = false;

	for (CQTableDefVector::const_iterator cur = other.fTargets.begin(), end = other.fTargets.end(); cur != end && !res; cur++)
	{
		if (AlreadyHas(*cur))
			res = true;
	}

	return res;
}


void CQTargetsDef::AddTargets(const CQTargetsDef& other)
{
	for (CQTableDefVector::const_iterator cur = other.fTargets.begin(), end = other.fTargets.end(); cur != end; cur++)
	{
		if (!AlreadyHas(*cur))
			Add(*cur);
	}

}


void CQTargetsDef::ReOrder()
{
	sort(fTargets.begin(), fTargets.end());
}




QueryNodeAggregate* ComplexOptimizedQuery::xBuildNodes(ComplexRech *model, VError& err, Boolean OnlyOneToken)
{
	QueryNodeAggregate* curmap = new QueryNodeAggregate;
	sWORD curoper = DB4D_NOTCONJ;
	err = VE_OK;
	Boolean waitforoper = false;
	Boolean waitforstatement = true;


	ComplexRechToken* rt = model->GetNextToken();
	
	while (rt != nil && rt->GetTyp() != rc_ParentD && err == VE_OK)
	{
		switch (rt->GetTyp())
		{
			case rc_ParentG:
				{
					if (waitforstatement)
					{
						QueryNodeAggregate* res = xBuildNodes(model, err, false);
						if (err == VE_OK)
						{
							if (curoper == DB4D_NOTCONJ)
							{
								xbox_assert(curmap->fNodes.size() == 0);
								delete curmap;
								curmap = res;
								if (curmap != nil)
									curoper = curmap->GetOper();
							}
							else
							{
								if (curoper == res->GetOper() || res->GetOper() == DB4D_NOTCONJ)
								{
									for (QueryNodeMap::iterator cur = res->fNodes.begin(), end = res->fNodes.end(); cur != end; cur++)
									{
										QueryNodeMap::iterator found = curmap->fNodes.find(cur->first);
										if (found == curmap->fNodes.end())
										{
											curmap->fNodes.insert(make_pair(cur->first, cur->second));
										}
										else
										{
											QueryNodeVector* nodes = &(found->second->fnodes);
											QueryNodeVector* nodesToAdd = &(cur->second->fnodes);
											nodes->insert(nodes->end(), nodesToAdd->begin(), nodesToAdd->end());
										}
									}
									delete res;
								}
								else
								{
									VRefPtr<QueryNode> node(xMakeOperNode(res, err), false);
									VRefPtr<CQTargetsDef> ref = nil;
									if (err == VE_OK)
									{
										ref.Adopt(node->GenerateTargets(err));
										if (err != VE_OK)
										{
											node = nil;
										}
									}

									if (node != nil && ref != nil)
									{
										curmap->AddNode(ref, node);
									}

									delete res;
								}
							}
						}
						waitforoper = true;
						waitforstatement = false;
					}
					else
						err = VE_DB4D_INVALID_QUERY;
				}
				break;

			case rc_Line:
				{
					if (waitforstatement)
					{
						CQTableDef target(((ComplexRechTokenSimpleComp*)rt)->cri->GetOwner(), ((ComplexRechTokenSimpleComp*)rt)->fNumInstance);
						VRefPtr<SimpleQueryNodeSeq> node(new SimpleQueryNodeSeq(this, target, (ComplexRechTokenSimpleComp*)rt), false);
						if (node == nil)
							err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
						else
						{
							if (! curmap->AddNode(target, node))
								err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
						}
						waitforoper = true;
						waitforstatement = false;
					}
					else
						err = VE_DB4D_INVALID_QUERY;
				}
				break;

			case rc_Script:
				{
					if (waitforstatement)
					{
						CQTableDef target(((ComplexRechTokenScriptComp*)rt)->fTable, ((ComplexRechTokenScriptComp*)rt)->fNumInstance);
						VRefPtr<SimpleQueryNodeLangExpression> node(new SimpleQueryNodeLangExpression(this, target, (ComplexRechTokenScriptComp*)rt), false);
						if (node == nil)
							err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
						else
						{
							if (!curmap->AddNode(target, node))
								err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
						}
						waitforoper = true;
						waitforstatement = false;
					}
					else
						err = VE_DB4D_INVALID_QUERY;
				}
				break;

			case rc_SQLScript:
				{
					if (waitforstatement)
					{
						CQTableDef target(((ComplexRechTokenSQLScriptComp*)rt)->fTable, ((ComplexRechTokenSQLScriptComp*)rt)->fNumInstance);
						VRefPtr<SimpleQueryNodeSQLExpression> node(new SimpleQueryNodeSQLExpression(this, target, (ComplexRechTokenSQLScriptComp*)rt), false);
						if (node == nil)
							err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
						else
						{
							if (!curmap->AddNode(target, node))
								err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
						}
						waitforoper = true;
						waitforstatement = false;
					}
					else
						err = VE_DB4D_INVALID_QUERY;
				}
				break;

			case rc_LineJoin:
				{
					if (waitforstatement)
					{
						CQTableDef target1(((ComplexRechTokenJoin*)rt)->cri1->GetOwner(), ((ComplexRechTokenJoin*)rt)->fNumInstance1);
						CQTableDef target2(((ComplexRechTokenJoin*)rt)->cri2->GetOwner(), ((ComplexRechTokenJoin*)rt)->fNumInstance2);
						VRefPtr<CQTargetsDef> targets(new CQTargetsDef, false);
						if (targets != nil)
						{
							if (target1 < target2)
							{
								targets->Add(target1);
								targets->Add(target2);
							}
							else
							{
								targets->Add(target2);
								targets->Add(target1);
							}

							VRefPtr<QueryNodeJoin> node(new QueryNodeJoin(this, target1, target2, (ComplexRechTokenJoin*)rt), false);
							if (node == nil)
								err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
							else
							{
								if (!curmap->AddNode(targets, node))
									err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
							}
						}
						else
							err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
						waitforoper = true;
						waitforstatement = false;
					}
					else
						err = VE_DB4D_INVALID_QUERY;
				}
				break;

			case rc_LineArray:
				{
					if (waitforstatement)
					{
						CQTableDef target(((ComplexRechTokenArrayComp*)rt)->cri->GetOwner(), ((ComplexRechTokenArrayComp*)rt)->fNumInstance);
						VRefPtr<SimpleQueryNodeArraySeq> node(new SimpleQueryNodeArraySeq(this, target, (ComplexRechTokenArrayComp*)rt), false);
						if (node == nil)
							err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
						else
						{
							if (!curmap->AddNode(target, node))
								err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
						}
						waitforoper = true;
						waitforstatement = false;
					}
					else
						err = VE_DB4D_INVALID_QUERY;
				}
				break;

			case rc_Operator:
				{
					if (waitforoper)
					{
						sWORD oper = ((ComplexRechTokenOper*)rt)->GetOper();

						if (curoper == DB4D_NOTCONJ || curoper == oper)
						{
							curoper = oper;
							curmap->SetOper(curoper);
						}
						else
						{
							VRefPtr<QueryNode> node(xMakeOperNode(curmap, err), false);
							VRefPtr<CQTargetsDef> ref = nil;
							if (err == VE_OK)
							{
								ref.Adopt(node->GenerateTargets(err));
								if (err != VE_OK)
								{
									node = nil;
								}
							}

							if (node != nil && ref != nil)
							{
								delete curmap;
								curmap = new QueryNodeAggregate;
								curoper = oper;
								curmap->SetOper(oper);

								if (!curmap->AddNode(ref, node))
									err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
							}
						}
						waitforoper = false;
						waitforstatement = true;
					}
					else
						err = VE_DB4D_INVALID_QUERY;
				}
				break;

			case rc_Not:
				{
					/*
					rt = model->GetNextToken();
					Boolean onlyone = false;
					if (rt->GetTyp() != rc_ParentG)
						onlyone = true;
					*/

					QueryNodeAggregate* res = xBuildNodes(model, err, true);
					if (err == VE_OK)
					{
						VRefPtr<QueryNode> node(xMakeOperNode(res, err), false);
						if (err == VE_OK)
						{
							VRefPtr<CQTargetsDef> ref = nil;
							ref.Adopt(node->GenerateTargets(err));
							if (err == VE_OK)
							{
								if (ref->fTargets.size() == 1 )
								{
									VRefPtr<SimpleQueryNodeNot> qnnot(new SimpleQueryNodeNot(this, *(ref->fTargets.begin())), false);
									if (qnnot == nil)
										err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
									else
									{
										qnnot->AdoptRoot(node->ConvertToSimple(*(ref->fTargets.begin()),err));
										if (err != VE_OK)
										{
											qnnot = nil;
										}
										if (qnnot != nil)
											curmap->AddNode(ref, qnnot);
										
									}
								}
								else
								{
									VRefPtr<QueryNodeNot> qnnot(new QueryNodeNot(this), false);
									if (qnnot == nil)
										err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
									else
									{
										qnnot->SetRoot(node);
										if (err != VE_OK)
										{
											qnnot = nil;
										}

										if (qnnot != nil && ref != nil)
										{
											curmap->AddNode(ref, qnnot);
										}

									}
								}
								waitforoper = true;
								waitforstatement = false;
							}
						}
					}

					if (res != nil)
						delete res;
					
				}
				break;

			default:
				xbox_assert (false);
				err = VE_DB4D_NOTIMPLEMENTED; /* Sergiy - 10 July 2007 */
				break;

		}

		if (OnlyOneToken)
			rt = nil;
		else
			rt = model->GetNextToken();
	}

	return curmap;
}


			/* -------------------------------- */


void NodeUsage::SetRoot(SimpleQueryNode* root)
{
	fRoot = root;
	if (fSelCacheNode != nil)
		fSelCacheNode = root;
	if (fBelongsToBaseJoin != nil)
	{
		if (fRootNum == 1)
			fBelongsToBaseJoin->SetRoot1(root);
		else
			fBelongsToBaseJoin->SetRoot2(root);
	}
	if (fBelongsToSimpleBaseJoin != nil)
	{
		fBelongsToSimpleBaseJoin->SetRoot(root);
	}
}


VError NodeUsage::CacheRoot(ComplexOptimizedQuery* query)
{
	VError err = VE_OK;
	if ( fCountUsed > 1)
	{
		if (fSelCacheNode == nil)
		{
			if (fRoot != nil)
			{
				fSelCacheNode.Adopt(new QuerySimpleNodeCacheSel(query, fRoot->GetTarget(), fRoot));
				if (fSelCacheNode == nil)
					err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
			}
		}
	}

	return err;
}


SimpleQueryNode* NodeUsage::GetCachedRoot() const
{
	if (fSelCacheNode != nil)
		return fSelCacheNode;
	else
		return fRoot;
}

			/* -------------------------------- */


QueryNode* ComplexOptimizedQuery::xMakeOperNode(QueryNodeAggregate* onemap, VError& err)
{
	err = VE_OK;
	QueryNode* result = nil;
	if (onemap->GetOper() == DB4D_NOTCONJ)
		onemap->SetOper(DB4D_And);
	/*
	if (onemap->GetOper() == DB4D_NOTCONJ)
	{
		if (onemap->fNodes.size() > 0)
		{
			xbox_assert(onemap->fNodes.size() == 1);
			CQNodesContent* nodes = onemap->fNodes.begin()->second;
			xbox_assert(nodes->fnodes.size() == 1);
			result = *(nodes->fnodes.begin());
		}

	}
	else
	*/
	{
		if (onemap->fNodes.size() == 1)
		{
			CQTargetsDef* targets = onemap->fNodes.begin()->first;
			if (targets->fTargets.size() == 1)
			{
				VRefPtr<SimpleQueryNodeBoolOper> rn(new SimpleQueryNodeBoolOper(this, *(targets->fTargets.begin()), onemap->GetOper()), false);
				if (rn == nil)
					err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
				else
				{
					CQNodesContent* content = onemap->fNodes.begin()->second;
					for (QueryNodeVector::iterator cur = content->fnodes.begin(), end = content->fnodes.end(); cur != end && err == VE_OK; cur++)
					{
						QueryNode* node = *cur;
						if (testAssert(node->isSimple()))
						{
							if (!rn->AddNode((SimpleQueryNode*)node))
								err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
						}

					}

					if (err != VE_OK)
						rn = nil;
				}
				if (rn != nil)
					result = rn.Forget();
			}
			else
			{
				VRefPtr<QueryNodeBoolOper> rn(new QueryNodeBoolOper(this, onemap->GetOper()), false);
				if (rn == nil)
					err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
				else
				{
					rn->SetAllSameTarget(true);
					CQNodesContent* content = onemap->fNodes.begin()->second;
					for (QueryNodeVector::iterator cur = content->fnodes.begin(), end = content->fnodes.end(); cur != end && err == VE_OK; cur++)
					{
						QueryNode* node = *cur;
						if (!rn->AddNode(node))
							err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);

					}
					if (err != VE_OK)
						rn = nil;
				}
				if (rn != nil)
					result = rn.Forget();
			}
		}
		else
		{
			VRefPtr<QueryNodeBoolOper> rnroot(new QueryNodeBoolOper(this, onemap->GetOper()), false);
			if (rnroot == nil)
				err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
			else
			{
				for (QueryNodeMap::iterator cur = onemap->fNodes.begin(), end = onemap->fNodes.end(); cur != end && err == VE_OK; cur++)
				{
					CQTargetsDef* targets = cur->first;
					CQNodesContent* content = cur->second;

					if (targets->fTargets.size() == 1)
					{
						if (content->fnodes.size() == 1)
						{
							if (!rnroot->AddNode(content->fnodes[0]))
								err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
						}
						else
						{
							VRefPtr<SimpleQueryNodeBoolOper> rn(new SimpleQueryNodeBoolOper(this, *(targets->fTargets.begin()), onemap->GetOper()), false);
							if (rn == nil)
								err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
							else
							{
								for (QueryNodeVector::iterator cur = content->fnodes.begin(), end = content->fnodes.end(); cur != end && err == VE_OK; cur++)
								{
									QueryNode* node = *cur;
									if (testAssert(node->isSimple()))
									{
										if (!rn->AddNode((SimpleQueryNode*)node))
											err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
									}

								}

								if (err != VE_OK)
									rn = nil;
							}
							if (rn != nil)
							{
								if (!rnroot->AddNode(rn))
									err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
							}
						}
					}
					else
					{
						if (content->fnodes.size() == 1)
						{
							if (!rnroot->AddNode(content->fnodes[0]))
								err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
						}
						else
						{
							VRefPtr<QueryNodeBoolOper> rn(new QueryNodeBoolOper(this, onemap->GetOper()), false);
							if (rn == nil)
								err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
							else
							{
								rn->SetAllSameTarget(true);
								for (QueryNodeVector::iterator cur = content->fnodes.begin(), end = content->fnodes.end(); cur != end && err == VE_OK; cur++)
								{
									QueryNode* node = *cur;
									if (!rn->AddNode(node))
										err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);

								}
								if (err != VE_OK)
									rn = nil;
							}
							if (rn != nil)
							{
								if (!rnroot->AddNode(rn))
									err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
							}
						}
					}

				}
			}
			if (rnroot != nil)
				result = rnroot.Forget();
		}
	}

	return result;
}


VError ComplexOptimizedQuery::BuildTargetSet()
{
	try
	{
		if (fModel->isSimpleTarget())
		{
			fTargetSet.insert(CQTableDef(fModel->GetSimpleTarget(), 0));
		}
		else
		{
			CQTargetsDef* targets = fModel->GetComplexTarget();
			copy(targets->fTargets.begin(), targets->fTargets.end(), insert_iterator<SetOfTargets>(fTargetSet, fTargetSet.begin()) ); 
			/*
			for (CQTableDefVector::iterator cur = targets->fTargets.begin(), end = targets->fTargets.end(); cur != end; cur++)
			{
				fTargetSet.insert(*cur);
			}
			*/
		}
		return VE_OK;
	}

	catch (...)
	{
		return ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
	}
}


Boolean ComplexOptimizedQuery::MatchTarget(const CQTableDef& target)
{
	return IsInCollection(fTargetSet, target);
}


Boolean ComplexOptimizedQuery::IsInFinalTarget(const CQTableDef& target, SetOfTargets& deja, JoinDependenceMap joindeps)
{
	if (!IsInCollection(deja, target))
	{
		if (MatchTarget(target))
			return true;
		else
		{
			deja.insert(target);
			CQTableDefVector* targets = &(joindeps[target].others);
			for (CQTableDefVector::iterator cur = targets->begin(), end = targets->end(); cur != end; cur++)
			{
				if (IsInFinalTarget(*cur, deja, joindeps))
				{
					return true;
				}
			}
			deja.erase(target);
		}
	}
	return false;
}


SimpleQueryNodeBaseJoin* ComplexOptimizedQuery::ConvertToSimpleBaseJoin(QueryNode* joinroot, const CQTableDef& target, const CQTableDef& targetjoin, SimpleQueryNode* root, VError& err)
{
	err = VE_OK;
	SimpleQueryNodeBaseJoin* qn = new SimpleQueryNodeBaseJoin(this, target, targetjoin);
	if (qn == nil)
		err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
	else
	{
		qn->SetRoot(root);
		VRefPtr<SimpleQueryNode> simplejoinroot(nil);
		if (joinroot != nil)
			simplejoinroot.Adopt(joinroot->ConvertToSimple(target, err));
		if (err == VE_OK)
			qn->SetJoinRoot(simplejoinroot);

		if (err != VE_OK)
		{
			qn->Release();
			qn = nil;
		}
	}

	return qn;
}


QueryNode* ComplexOptimizedQuery::xRegroupCompositeJoins(QueryNode* root, VError& err, BaseTaskInfo* context)
{
	QueryNode* result = nil;
	if (root != nil)
	{
		if (root->GetType() == QueryNode_BaseJoin)
		{
			QueryNodeBaseJoin* bjn = (QueryNodeBaseJoin*)root;
			if (bjn->GetJoinRoot() != nil && bjn->GetJoinRoot()->GetType() == QueryNode_BoolOper)
			{
				bool okcomposite = true;
				QueryNodeBoolOper* qnoper = (QueryNodeBoolOper*)bjn->GetJoinRoot();
				if (qnoper->GetOper() == DB4D_And)
				{
					VRefPtr<CQTargetsDef> targets(bjn->GenerateTargets(err), false);
					xbox_assert(targets->fTargets.size() == 2);
					CQTableDefVector::iterator cur = targets->fTargets.begin();
					CQTableDef t1 = *cur;
					cur++;
					CQTableDef t2 = *cur;
					VRefPtr<QueryNodeCompositeJoin> compjoin(new QueryNodeCompositeJoin(this, t1, t2), false);
					QueryNodeVector& nodes = qnoper->GetArrayNodes().fnodes;
					for (QueryNodeVector::iterator cur = nodes.begin(), end = nodes.end(); cur != end && okcomposite && err == VE_OK; cur++)
					{
						QueryNode* qn = *cur;
						if (qn != nil)
						{
							if (qn->GetType() == QueryNode_BaseJoin)
							{
								qn = ((QueryNodeBaseJoin*)qn)->GetJoinRoot();
							}
							if (qn->GetType() == QueryNode_Join)
							{
								QueryNodeJoin* qnjoin = (QueryNodeJoin*)qn;
								if (*(qnjoin->GetTarget1()) == t1)
								{
									err = compjoin->AddJoinPart(qnjoin->rt->cri1, qnjoin->rt->cri2, qnjoin->rt->fLeftJoin, qnjoin->rt->fRightJoin, qnjoin->rt->fOptions);
								}
								else
								{
									xbox_assert(*(qnjoin->GetTarget2()) == t1);
									err = compjoin->AddJoinPart(qnjoin->rt->cri2, qnjoin->rt->cri1, qnjoin->rt->fRightJoin, qnjoin->rt->fLeftJoin, qnjoin->rt->fOptions);
								}
								
							}
							else
								okcomposite = false;
						}
					}

					if (okcomposite && err == VE_OK)
					{
						bjn->SetJoinRoot(compjoin);
						compjoin->IsIndexe(context);
					}
				}
				
			}
		}
		else if (root->GetType() == QueryNode_BoolOper)
		{
			QueryNodeBoolOper* qnoper = (QueryNodeBoolOper*)root;
			QueryNodeVector& nodes = qnoper->GetArrayNodes().fnodes;
			for (QueryNodeVector::iterator cur = nodes.begin(), end = nodes.end(); cur != end && err == VE_OK; cur++)
			{
				QueryNode* qn = *cur;
				if (qn != nil)
				{
					(*cur).Adopt(xRegroupCompositeJoins(qn, err, context));
				}
			}

		}
		else if (root->GetType() == QueryNode_Not)
		{
			QueryNodeNot* qnnot = (QueryNodeNot*)root;
			if (qnnot->GetRoot() != nil)
			{
				VRefPtr<QueryNode> rqn(xRegroupCompositeJoins(qnnot->GetRoot(), err, context), false);
				qnnot->SetRoot(rqn);
			}
		}	
	}

	if (result == nil)
	{
		result = RetainRefCountable(root);
	}
	return result;
}


QueryNode* ComplexOptimizedQuery::xBuildJoins(QueryNode* root, VError& err, QueryNode* &LeftOver)
{
	err = VE_OK;
	QueryNode* result = nil;
	LeftOver = nil;

	if (root != nil)
	{
		if (root->GetType() == QueryNode_BoolOper)
		{
			QueryNodeVector newones;
			QueryNodeBoolOper* rn = (QueryNodeBoolOper*)root;
			QueryNodeVector* nodes = &(rn->GetArrayNodes().fnodes);
			for (QueryNodeVector::iterator cur = nodes->begin(), end = nodes->end(); cur != end && err == VE_OK; cur++)
			{
				QueryNode* onemore = nil;
				VRefPtr<QueryNode> node(xBuildJoins(*cur, err, onemore), false);
				VRefPtr<QueryNode> NodeToAdd(onemore, false);
				if (err == VE_OK)
				{
					*cur = node;
					if (NodeToAdd != nil)
					{
						newones.push_back(NodeToAdd);
					}
				}
			}

			for (QueryNodeVector::iterator cur = newones.begin(), end = newones.end(); cur != end && err == VE_OK; cur++)
			{
				if (!rn->AddNode(*cur))
					err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
			}

			rn->ResetCacheFlags();

			Boolean atLeastOneJoin = false;
			Boolean allJoin = true;

			if (err == VE_OK )
			{
				Boolean mustpass2 = false;

				if (rn->AllSameTarget())
				{
					for (QueryNodeVector::iterator cur = nodes->begin(), end = nodes->end(); cur != end && err == VE_OK; cur++)
					{
						QueryNode* node = *cur;
						if (node->isAllJoin())
							atLeastOneJoin = true;
						else
							allJoin = false;
					}

					if (atLeastOneJoin)
						mustpass2 = true;

					if (atLeastOneJoin && !allJoin)
					{
						LeftOver = new QueryNodeBoolOper(this, rn->GetOper());
						if (LeftOver == nil)
							err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
						else
						{
							for (QueryNodeVector::iterator cur = nodes->begin(), end = nodes->end(); cur != end && err == VE_OK; cur++)
							{
								QueryNode* node = *cur;
								if (!node->isAllJoin())
								{
									if (((QueryNodeBoolOper*)LeftOver)->AddNode(node))
									{
										*cur = nil;
									}
									else
										err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
								}
							}
							if (err == VE_OK)
							{
								QueryNodeVector::iterator newend = remove(nodes->begin(), nodes->end(), VRefPtr<QueryNode>((QueryNode*)nil));
								nodes->erase(newend, nodes->end());

							}
						}
					}
					rn->ResetCacheFlags();

					result = rn;
					result->Retain();
				}
				else
					mustpass2 = true;

				if (mustpass2)  // si pas tous de la meme target ou alors il y a des joins
				{
					if (rn->GetOper() == DB4D_And)
					{
						NodeUsageMap UsedRef;
						JoinDependenceMap joindeps;

						try
						{
							for (QueryNodeVector::iterator cur = nodes->begin(), end = nodes->end(); cur != end && err == VE_OK; cur++)
							{
								QueryNode* node = *cur;
								CQTableDef* target = node->GetSimpleTarget();
								if (target != nil)
								{
									UsedRef[*target].Set((SimpleQueryNode*)node, (sLONG)(cur-nodes->begin()));
								}
								else
								{
									if (node->isAllJoin())
									{
										VRefPtr<CQTargetsDef> targets(node->GenerateTargets(err), false);
										xbox_assert(targets->fTargets.size() == 2);
										CQTableDefVector::iterator cur = targets->fTargets.begin();
										CQTableDef t1 = *cur;
										cur++;
										CQTableDef t2 = *cur;
										joindeps[t1].others.push_back(t2);
										joindeps[t2].others.push_back(t1);
										if (MatchTarget(t1) && MatchTarget(t2))
										{
											UsedRef[t1].fCountUsed++;
											UsedRef[t2].fCountUsed++;
										}
									}
								}
							}

							for (QueryNodeVector::iterator cur = nodes->begin(), end = nodes->end(); cur != end && err == VE_OK; cur++)
							{
								QueryNode* node = *cur;
								if (node != nil)
								{
									if (node->isAllJoin())
									{
										VRefPtr<CQTargetsDef> targets(node->GenerateTargets(err), false);
										xbox_assert(targets->fTargets.size() == 2);
										CQTableDefVector::iterator curtarget = targets->fTargets.begin();
										CQTableDef t1 = *curtarget;
										curtarget++;
										CQTableDef t2 = *curtarget;

										if (!(MatchTarget(t1) && MatchTarget(t2)))
										{
											CQTableDef goodone;
											CQTableDef joinone;
											SetOfTargets deja;
											deja.insert(t2);
											if (IsInFinalTarget(t1, deja, joindeps))
											{
												goodone = t1;
												joinone = t2;
											}
											else
											{
												goodone = t2;
												joinone = t1;
											}

											NodeUsage* xx = &UsedRef[joinone];
											SimpleQueryNode* joinone_root =xx->fRoot;
											if (xx->posinvector != -1)
											{
												(*nodes)[xx->posinvector] = nil;
												xx->posinvector = -1;
											}

											VRefPtr<SimpleQueryNodeBaseJoin> basejoin(ConvertToSimpleBaseJoin(node, goodone, joinone, joinone_root, err), false);
											if (err == VE_OK)
											{
												UsedRef[joinone].fBelongsToSimpleBaseJoin = basejoin;

												*cur = (QueryNode*)nil;
												SimpleQueryNode* root = UsedRef[goodone].GetCachedRoot();
												//UsedRef[goodone].posinvector = -1;	
												if (root->GetType() == QuerySimpleNode_BoolOper && ((SimpleQueryNodeBoolOper*)root)->GetOper() == DB4D_And)
												{
													if (!((SimpleQueryNodeBoolOper*)root)->AddNode(basejoin))
														err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
												}
												else
												{
													VRefPtr<SimpleQueryNodeBoolOper> newand(new SimpleQueryNodeBoolOper(this, goodone, DB4D_And), false);
													if (newand == nil)
														err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
													else
													{
														if (newand->AddNode(root) && newand->AddNode(basejoin))
														{
															sLONG pos = UsedRef[goodone].posinvector;
															if (pos != -1)
															{
																(*nodes)[pos] = (SimpleQueryNode*)newand;
															}
															UsedRef[goodone].SetRoot(newand);
														}
														else
															err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
													}
												}
											}
										}
									}
								}
							}

							for (QueryNodeVector::iterator cur = nodes->begin(), end = nodes->end(); cur != end && err == VE_OK; cur++)
							{
								QueryNode* node = *cur;
								if (node != nil)
								{
									if (node->isAllJoin())
									{
										VRefPtr<CQTargetsDef> targets(node->GenerateTargets(err), false);
										xbox_assert(targets->fTargets.size() == 2);
										CQTableDefVector::iterator curtarget = targets->fTargets.begin();
										CQTableDef t1 = *curtarget;
										curtarget++;
										CQTableDef t2 = *curtarget;

										if (MatchTarget(t1) && MatchTarget(t2))
										{
											err = UsedRef[t1].CacheRoot(this);
											if (err == VE_OK)
												UsedRef[t2].CacheRoot(this);
											if (err == VE_OK)
											{
												VRefPtr<QueryNodeBaseJoin> bjn(new QueryNodeBaseJoin(this, t1, t2), false);
												//QueryNodeBaseJoin* bjn = new QueryNodeBaseJoin(this, t1, t2);
												if (bjn == nil)
													err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
												else
												{
													bjn->SetRoot1(UsedRef[t1].GetCachedRoot());
													UsedRef[t1].SetBelongsToBaseJoin(bjn, 1);
													bjn->SetRoot2(UsedRef[t2].GetCachedRoot());
													UsedRef[t2].SetBelongsToBaseJoin(bjn, 2);
													bjn->SetJoinRoot(node);
													*cur = (QueryNode*)bjn;
												}
											}
										}
									}
								}
							}

							if (err == VE_OK)
							{
								for (NodeUsageMap::iterator cur = UsedRef.begin(), end = UsedRef.end(); cur != end; cur++)
								{
									if (cur->second.fCountUsed > 0)
									{
										if (cur->second.posinvector != -1)
											(*nodes)[cur->second.posinvector] = (QueryNode*)nil;
									}
									else
									{
										/* 
										if (cur->second.posinvector != -1)
											(*nodes)[cur->second.posinvector] = (QueryNode*)nil;
										*/
									}
								}

								rn->ResetCacheFlags();
								QueryNodeVector::iterator newend = remove(nodes->begin(), nodes->end(), VRefPtr<QueryNode>((QueryNode*)nil));
								nodes->erase(newend, nodes->end());
							}

							if (err == VE_OK)
							{
								VRefPtr<CQTargetsDef> rntargets(rn->GenerateTargets(err), false);
								if (err == VE_OK)
								{
									if (rntargets->fTargets.size() == 1)
									{
										QueryNodeVector* rnnodes = &(rn->GetArrayNodes().fnodes);
										if (rnnodes->size() == 1)
										{
											result = (*rnnodes)[0];
											if (testAssert(result != nil))
												result->Retain();
										}
										else
										{
											CQTableDef rntarget = rntargets->fTargets[0];
											SimpleQueryNode* sbn = rn->ConvertToSimple(rntarget, err);
											if (err == VE_OK)
											{
												result = sbn;
											}
										}
									}
									else
									{
										
										QueryNodeVector* rnnodes = &(rn->GetArrayNodes().fnodes);
										if (rnnodes->size() == 1)
										{
											result = (*rnnodes)[0];
											if (testAssert(result != nil))
												result->Retain();
										}
										else
										{
											result = rn;
											result->Retain();
										}
									}
								}
							}

						}

						catch (...)
						{
							err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
						}

					}
					else // si pas un DB4D_And
					{
						result = rn;
						result->Retain();
					}

				}
			}



		}
		else
		{
			result = root;
			result->Retain();
		}
	}

	if (err != VE_OK && LeftOver != nil)
	{
		LeftOver->Release();
		LeftOver = nil;
	}
	return result;
}


//typedef map<sLONG, IndexInfoFromMultipleField*, less<sLONG> , cache_allocator<pair<const sLONG, IndexInfoFromMultipleField*> > > indexmap;

typedef multimap<sLONG, IndexInfoFromMultipleField*> indexmap;


QueryNode* ComplexOptimizedQuery::xTransformSeq(QueryNode* root, Boolean seq, VError& err, BaseTaskInfo* context)
{
	QueryNode* result = nil;
	err = VE_OK;

	if (root != nil)
	{
		switch (root->GetType())
		{
		case QuerySimpleNode_LangExpression:
		case QuerySimpleNode_SQLExpression:
		case QuerySimpleNode_Join:
		case QueryNode_Join:
		case QuerySimpleNode_Seq:
		case QuerySimpleNode_ArraySeq:
			{
				// pas de modif du noeud
			}
			break;


		case QuerySimpleNode_Index:
			{
				if (seq)
				{
					SimpleQueryNodeIndex* qnind = (SimpleQueryNodeIndex*)root;
					if (qnind->fNodeSeq == nil) // index composite
					{
						SimpleQueryNodeBoolOper* newand = new SimpleQueryNodeBoolOper(this, qnind->GetTarget(), DB4D_And);
						if (newand == nil)
							err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
						else
						{
							for (SimpleQueryNodeSeqVector::iterator cur = qnind->fNodes.begin(), end = qnind->fNodes.end(); cur != end; cur++)
							{
								newand->AddNode(*cur);
							}
							if (qnind->fIncludingFork != nil)
								newand->AddNode(qnind->fIncludingFork);
							result = newand;
						}						
					}
					else
					{
						if (qnind->fIncludingFork == nil)
						{
							result = qnind->fNodeSeq;
							result->Retain();
						}
						else
						{
							SimpleQueryNodeBoolOper* newand = new SimpleQueryNodeBoolOper(this, qnind->GetTarget(), DB4D_And);
							if (newand == nil)
								err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
							else
							{
								newand->AddNode(qnind->fNodeSeq);
								newand->AddNode(qnind->fIncludingFork);
								result = newand;
							}
						}
					}


				}
				
			}
			break;


		case QuerySimpleNode_ArrayIndex:
			{
				SimpleQueryNodeArrayIndex* qnind = (SimpleQueryNodeArrayIndex*)root;
				result = qnind->fNodeSeq;
				result->Retain();
			}
			break;

		case QuerySimpleNode_Not:
			{
				SimpleQueryNodeNot* qnnot = (SimpleQueryNodeNot*)root;
				VRefPtr<QueryNode> res(xTransformSeq(qnnot->GetRoot(), seq, err, context), false);
				qnnot->SetRoot((SimpleQueryNode*)res.Get());
			}
			break;

		case QuerySimpleNode_BaseJoin:
			{
				SimpleQueryNodeBaseJoin* qnbasejoin = (SimpleQueryNodeBaseJoin*)root;
				VRefPtr<QueryNode> res(xTransformSeq(qnbasejoin->GetRoot(), seq, err, context), false);
				qnbasejoin->SetRoot((SimpleQueryNode*)res.Get());
				VRefPtr<QueryNode> resjoin(xTransformSeq(qnbasejoin->GetJoinRoot(), seq, err, context), false);
				qnbasejoin->SetJoinRoot((SimpleQueryNode*)resjoin.Get());
				qnbasejoin->ResetCacheFlags();
			}
			break;

		case QuerySimpleNode_CacheSel:
			{
				QuerySimpleNodeCacheSel* qncache = (QuerySimpleNodeCacheSel*)root;
				VRefPtr<QueryNode> res(xTransformSeq(qncache->GetRoot(), seq, err, context), false);
				qncache->SetRoot((SimpleQueryNode*)res.Get());
				qncache->ResetCacheFlags();
			}
			break;

		case QueryNode_BaseJoin:
			{
				QueryNodeBaseJoin* qnbasejoin = (QueryNodeBaseJoin*)root;
				VRefPtr<QueryNode> res1(xTransformSeq(qnbasejoin->GetRoot1(), seq, err, context), false);
				qnbasejoin->SetRoot1(res1);
				VRefPtr<QueryNode> res2(xTransformSeq(qnbasejoin->GetRoot2(), seq, err, context), false);
				qnbasejoin->SetRoot2(res2);
				VRefPtr<QueryNode> resjoin(xTransformSeq(qnbasejoin->GetJoinRoot(), seq, err, context), false);
				qnbasejoin->SetJoinRoot(resjoin);
				qnbasejoin->ResetCacheFlags();
			}
			break;

		case QueryNode_BoolOper:
			{
				QueryNodeBoolOper* qnop = (QueryNodeBoolOper*)root;
				if (qnop->GetOper() == DB4D_OR) 
				{
					if (!qnop->IsIndexe(context))
						seq = true;
				}

				for (QueryNodeVector::iterator cur = qnop->GetArrayNodes().fnodes.begin(), end = qnop->GetArrayNodes().fnodes.end(); cur != end && err == VE_OK; cur++)
				{
					VRefPtr<QueryNode> res(xTransformSeq(*cur, seq, err, context), false);
					*cur = res;
				}
				qnop->ResetCacheFlags();
			}
			break;

		case QuerySimpleNode_BoolOper:
			{
				SimpleQueryNodeBoolOper* qnop = (SimpleQueryNodeBoolOper*)root;
				if (qnop->GetOper() == DB4D_OR) 
				{
					if (!qnop->IsIndexe(context))
						seq = true;
				}

				SimpleQueryNodeVector& Nodes = qnop->GetArrayNodes();
				for (SimpleQueryNodeVector::iterator curnode = Nodes.begin(), endnode = Nodes.end(); curnode != endnode; curnode++)
				{
					VRefPtr<SimpleQueryNode> res((SimpleQueryNode*)xTransformSeq(*curnode, seq, err, context), false);
					*curnode = res;
				}

				qnop->ResetCacheFlags();
			}

			break;
		} // du switch
	}

	if (err == VE_OK && result == nil)
	{
		result = root;
		if (result != nil)
			result->Retain();
	}

	return result;
}


QueryNode* ComplexOptimizedQuery::xTransformIndex(QueryNode* root, VError& err, BaseTaskInfo* context)
{
	QueryNode* result = nil;
	err = VE_OK;

	if (root != nil)
	{
		switch (root->GetType())
		{
			case QuerySimpleNode_LangExpression:
			case QuerySimpleNode_SQLExpression:
			case QuerySimpleNode_ArrayIndex:
			case QuerySimpleNode_Index:
			case QuerySimpleNode_Join:
			case QueryNode_Join:
				{
					// pas de modif du noeud
				}
				break;


			case QuerySimpleNode_Seq:
				{
					SimpleQueryNodeSeq* qnseq = (SimpleQueryNodeSeq*)root;
					ComplexRechTokenSimpleComp* rt = qnseq->rt;
					Boolean canbeindexed = true;
					Boolean ParseIndexSeq = false;

					ValueKind ktyp = VK_UNDEFINED;
					if (rt->ch != nil)
						ktyp = rt->ch->GetValueKind();

					if (rt->ch == nil || rt->ch->IsNull())
					{
						result = new SimpleQueryNodeConst(this, qnseq->GetTarget(), 2);
					}
					else
					{
						if (ktyp == VK_STRING || ktyp == VK_TEXT || ktyp == VK_STRING_UTF8 || ktyp == VK_TEXT_UTF8)
						{
							if (InLikeRange(rt->comparaison))
							{
								VString* s = (VString*)rt->ch;
								sLONG p = s->FindUniChar(GetWildChar(context));

								/*
									for japanese hiragana: NOSA < NOZA < NOSAKI < NOZAKI
									This breaks the binary search when there's a wildchar at the end of the pattern.
									remember to also change OptimizedQuery::xTransformIndex.
								*/
								bool compatible = GetContextIntl(context)->GetCollator()->IsPatternCompatibleWithDichotomyAndDiacritics( s->GetCPointer(), s->GetLength());

								if (p == 0 || p == -1 || (p == s->GetLength() && compatible))
									ParseIndexSeq = false;
								else
									ParseIndexSeq = true;
								if (p == 0)
								{
									rt->comparaison = RemoveTheLike(rt->comparaison);
									SetCompOptionWithOperator(rt->fOptions, rt->comparaison);
								}
							}
						}

						if (canbeindexed)
						{
							Boolean tryindexregular;
							IndexInfo* ind = nil;
							if (rt->comparaison == DB4D_Contains_KeyWord || rt->comparaison == DB4D_DoesntContain_KeyWord
								|| rt->comparaison == DB4D_Contains_KeyWord_Like || rt->comparaison == DB4D_Doesnt_Contain_KeyWord_Like
								|| ParseIndexSeq)
							{
								tryindexregular = ParseIndexSeq;
								ind = rt->cri->FindAndRetainIndexLexico(true, context);
								if (ind != nil)
								{
									if (ParseIndexSeq)
									{
										DB4DKeyWordList words;
										rt->fOptions.SetIntlManager(GetContextIntl(context));
										BuildKeyWords(*((VString*)rt->ch), words, rt->fOptions);
										if (!( words.GetCount() == 1 && (*(words[0]) == *((VString*)rt->ch))))
										{
											ind->ReleaseValid();
											ind->Release();
											ind = nil;
											tryindexregular = true;
										}
									}
									else
										tryindexregular = false;
								}
							}
							else
								tryindexregular = true;

							if (tryindexregular)
							{
								ind = rt->cri->FindAndRetainIndexSimple(true, true, context);
							}

							if (ind != nil)
							{
								SimpleQueryNodeIndex* qnind = new SimpleQueryNodeIndex(this, qnseq->GetTarget(), rt, ind, qnseq, context);
								if (qnind == nil)
									err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
								else
								{
									err = qnind->BuildFrom(rt);
									qnind->SetParseAllIndex(ParseIndexSeq);
									if (err == VE_OK)
									{
										result = qnind;
									}
									else
									{
										qnind->Release();
										qnind = nil;
									}
								}
								ind->Release();
							}
						}
					}
				}
				break;

				
			case QuerySimpleNode_ArraySeq:
				{
					SimpleQueryNodeArraySeq* qnarrayseq = (SimpleQueryNodeArraySeq*)root;
					ComplexRechTokenArrayComp* rta = qnarrayseq->rt;
					if (rta->values->Count() == 0 || rta->values->IsAllNull())
					{
						result = new SimpleQueryNodeConst(this, qnarrayseq->GetTarget(), 2);
					}
					else
					{
						if (rta->values->CanBeUsedWithIndex(GetContextIntl(context)))
						{
							IndexInfo* ind = rta->cri->FindAndRetainIndexSimple(true, true, context);
							if (ind != nil)
							{
								SimpleQueryNodeArrayIndex* qnarrayindex = new SimpleQueryNodeArrayIndex(this, qnarrayseq->GetTarget(), rta, ind, qnarrayseq);
								if (qnarrayindex != nil)
								{
									err = qnarrayindex->BuildFrom(rta);
									if (err != VE_OK)
									{
										qnarrayindex->Release();
										qnarrayindex = nil;
									}
									else
										result = qnarrayindex;
								}
								else
									err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
								ind->Release();
							}
						}
					}
				}
				break;

			case QuerySimpleNode_Not:
				{
					SimpleQueryNodeNot* qnnot = (SimpleQueryNodeNot*)root;
					VRefPtr<QueryNode> res(xTransformIndex(qnnot->GetRoot(), err, context), false);
					qnnot->SetRoot((SimpleQueryNode*)res.Get());
				}
				break;

			case QuerySimpleNode_BaseJoin:
				{
					SimpleQueryNodeBaseJoin* qnbasejoin = (SimpleQueryNodeBaseJoin*)root;
					VRefPtr<QueryNode> res(xTransformIndex(qnbasejoin->GetRoot(), err, context), false);
					qnbasejoin->SetRoot((SimpleQueryNode*)res.Get());
					VRefPtr<QueryNode> resjoin(xTransformIndex(qnbasejoin->GetJoinRoot(), err, context), false);
					qnbasejoin->SetJoinRoot((SimpleQueryNode*)resjoin.Get());
					qnbasejoin->ResetCacheFlags();
				}
				break;

			case QuerySimpleNode_CacheSel:
				{
					QuerySimpleNodeCacheSel* qncache = (QuerySimpleNodeCacheSel*)root;
					VRefPtr<QueryNode> res(xTransformIndex(qncache->GetRoot(), err, context), false);
					qncache->SetRoot((SimpleQueryNode*)res.Get());
					qncache->ResetCacheFlags();
				}
				break;

			case QueryNode_BaseJoin:
				{
					QueryNodeBaseJoin* qnbasejoin = (QueryNodeBaseJoin*)root;
					VRefPtr<QueryNode> res1(xTransformIndex(qnbasejoin->GetRoot1(), err, context), false);
					qnbasejoin->SetRoot1(res1);
					VRefPtr<QueryNode> res2(xTransformIndex(qnbasejoin->GetRoot2(), err, context), false);
					qnbasejoin->SetRoot2(res2);
					VRefPtr<QueryNode> resjoin(xTransformIndex(qnbasejoin->GetJoinRoot(), err, context), false);
					qnbasejoin->SetJoinRoot(resjoin);
					qnbasejoin->ResetCacheFlags();
				}
				break;

			case QueryNode_BoolOper:
				{
					QueryNodeBoolOper* qnop = (QueryNodeBoolOper*)root;

					for (QueryNodeVector::iterator cur = qnop->GetArrayNodes().fnodes.begin(), end = qnop->GetArrayNodes().fnodes.end(); cur != end && err == VE_OK; cur++)
					{
						VRefPtr<QueryNode> res(xTransformIndex(*cur, err, context), false);
						*cur = res;
					}
					qnop->ResetCacheFlags();
				/*
					result = root;
					result->Retain();
					*/
				}
				break;
			
			case QuerySimpleNode_BoolOper:
				{
					SimpleQueryNodeBoolOper* qnop = (SimpleQueryNodeBoolOper*)root;
					if (qnop->GetOper() == DB4D_And)  // on va d'abord chercher tous les index composites sur une serie de AND
					{
						Table* curtarget = qnop->GetTarget().GetTable();
						curtarget->occupe();
						IndexArray indexes;
						curtarget->CopyIndexDep(indexes);
						curtarget->libere();

						indexmap multiindexes;
						IndexArray::Iterator cur = indexes.First(), end = indexes.End();

						for (; cur != end; cur++)
						{
							IndexInfo* ind = *cur;
							if (ind != nil)
							{
								if (ind->MatchType(DB4D_Index_OnMultipleFields))
								{
									if (ind->AskForValid(context))
									{
										IndexInfoFromMultipleField* indm = (IndexInfoFromMultipleField*)ind;
										multiindexes.insert( make_pair(indm->GetFieldNums()->GetNbField(), indm));
									}
								}
							}
						}
						// on a trie les index composites concernant cette table par ordre du nombre de champs concernes

						indexmap::reverse_iterator curm = multiindexes.rbegin(), endm = multiindexes.rend();
						for (; curm != endm; curm++)  // puis pour chque index composite on recherche si l'on trouve une combinasion des champs correspondant dans les AND
						{
							IndexInfoFromMultipleField* indm = curm->second;
							const FieldNuplet* indexfields = indm->GetFieldNums();

							Boolean found = false;
							sLONG goodpos[128];
							sLONG dejapos[128];
							sLONG posinindex[128];
							sLONG poslevel = 0, pos = 0;
							sLONG maxlevel = indexfields->GetNbField();
							xbox_assert(maxlevel<=128);
							sLONG lastfield, lastcomp;

							SimpleQueryNodeVector& Nodes = qnop->GetArrayNodes();
							for (SimpleQueryNodeVector::iterator curnode = Nodes.begin(), endnode = Nodes.end(); curnode != endnode && !found; curnode++, pos++)  // donc on bloucle sur l'ensemble des AND
							{
								SimpleQueryNode* nod = *curnode;
								if (nod != nil)
								{
									if (nod->GetType() == QuerySimpleNode_Seq)
									{
										ComplexRechTokenSimpleComp* rt = ((SimpleQueryNodeSeq*)nod)->rt;
										Boolean okcomp;
#if 0 // ce n'est pas completement vrai LR le 5 fev 2008
										if (poslevel==maxlevel-1)  // pour le dernier niveau de champ on peut comparer sur autre chose que =
										{
											okcomp = ((rt->comparaison >= DB4D_Equal) && (rt->comparaison <= DB4D_LowerOrEqual))
												|| ((rt->comparaison >= DB4D_Like) && (rt->comparaison <= DB4D_LowerOrEqual_Like));
										}
										else
#endif
										{
											okcomp = (rt->comparaison == DB4D_Equal) || (rt->comparaison == DB4D_Like);
										}
										if (okcomp)
										{
											Boolean okfield = false;
											sLONG i;
											for (i = 0; i< maxlevel; i++)
											{
												sLONG numfield = indexfields->GetFieldNum(i);
												if (numfield == rt->cri->GetPosInRec())
												{
													sLONG* enddeja = &dejapos[poslevel];
													if (std::find(&dejapos[0], enddeja, numfield) == enddeja)  // si le champ n'est pas deja selection pour tenter l'index
													{
														okfield = true;
														dejapos[poslevel] = numfield;
														posinindex[poslevel] = i;
														break;
													}
												}
											}
											if (okfield)
											{
												goodpos[poslevel] = pos;
												poslevel++;
												if (poslevel==maxlevel)  // quand on arrive au nombre de champs de l'index alors la combinaison reference par goodpos est la bonne
												{
													lastfield = rt->cri->GetPosInRec();
													lastcomp = rt->comparaison;
													if (indm->AskForValid(context))
														found = true;
												}
											}
										}
									}
								}
							}


							sLONG forkpos = 0;
							sLONG otherPartofFork1, otherPartofFork2;

							if (found)
							{
								switch(lastcomp) 
								{
								case DB4D_LowerOrEqual:
								case DB4D_Lower:
									otherPartofFork1 = DB4D_GreaterOrEqual;
									otherPartofFork2 = DB4D_Greater;
									break;

								case DB4D_GreaterOrEqual:
								case DB4D_Greater:
									otherPartofFork1 = DB4D_LowerOrEqual;
									otherPartofFork2 = DB4D_Lower;
									break;

								case DB4D_LowerOrEqual_Like:
								case DB4D_Lower_Like:
									otherPartofFork1 = DB4D_GreaterOrEqual_Like;
									otherPartofFork2 = DB4D_Greater_Like;
									break;

								case DB4D_GreaterOrEqual_Like:
								case DB4D_Greater_Like:
									otherPartofFork1 = DB4D_LowerOrEqual_Like;
									otherPartofFork2 = DB4D_Lower_Like;
									break;

								default:
									otherPartofFork1 = 0;
									otherPartofFork2 = 0;
									break;
								}

								if (otherPartofFork1 != 0) // on reboucle pour voir s'il est possible de faire une fouchette avec le dernier champ de l'index composite
								{
									pos = 0;
									for (SimpleQueryNodeVector::iterator curnode = Nodes.begin(), endnode = Nodes.end(); curnode != endnode && !found; curnode++, pos++)
									{
										SimpleQueryNode* nod = *curnode;
										if (nod != nil)
										{
											if (nod->GetType() == QuerySimpleNode_Seq)
											{
												ComplexRechTokenSimpleComp* rt = ((SimpleQueryNodeSeq*)nod)->rt;
												if (rt->cri->GetPosInRec() == lastfield)
												{
													if (rt->comparaison == otherPartofFork1 || rt->comparaison == otherPartofFork2)
													{
														forkpos = pos;
													}
												}
											}
										}
									}
								}

								SimpleQueryNodeIndex* qnind = new SimpleQueryNodeIndex(this, qnop->GetTarget(), nil, indm, nil, context);

								//typedef map<sLONG, SimpleQueryNode*, less<sLONG> , cache_allocator<pair<const sLONG, SimpleQueryNode*> > > type_mapnode;
								typedef map<sLONG, SimpleQueryNode*> type_mapnode;
								type_mapnode mapnodes;  // on va trier les AND concernes par place du champ dans l'index composite
								for (sLONG i=0; i<maxlevel; i++)
								{
									mapnodes.insert(make_pair(posinindex[i], Nodes[goodpos[i]]));
									Nodes[goodpos[i]] = nil;
								}
								type_mapnode::iterator curinmap = mapnodes.begin();
								for (; curinmap != mapnodes.end(); curinmap++)
								{
									qnind->Add((SimpleQueryNodeSeq*)curinmap->second);
								}

								SimpleQueryNode* forkother;
								if (forkpos != 0)
								{
									forkother = Nodes[forkpos];
									Nodes[forkpos] = nil;
								}
								else
									forkother = nil;

								qnind->BuildFromMultiple((SimpleQueryNodeSeq*)forkother, context);

								SimpleQueryNodeVector::iterator newend = std::remove(Nodes.begin(), Nodes.end(), VRefPtr<SimpleQueryNode>(nil));
								Nodes.erase(newend, Nodes.end());

								qnop->AddNode(qnind);
								qnind->Release();
							}

						}

						for (indexmap::iterator curm1 = multiindexes.begin(), endm1 = multiindexes.end(); curm1 != endm1; curm1++)
						{
							curm1->second->ReleaseValid();
						}
					}

					 // puis on transforme les AND qui restent en index simple si possible
					SimpleQueryNodeVector& Nodes = qnop->GetArrayNodes();
					for (SimpleQueryNodeVector::iterator curnode = Nodes.begin(), endnode = Nodes.end(); curnode != endnode; curnode++)
					{
						VRefPtr<SimpleQueryNode> res((SimpleQueryNode*)xTransformIndex(*curnode, err, context), false);
						*curnode = res;
					}

					if (qnop->GetOper() == DB4D_And) // et on va rechercher les fourchettes sur les index simples
					{
						for (SimpleQueryNodeVector::iterator curnode = Nodes.begin(), endnode = Nodes.end(); curnode != endnode; curnode++)
						{
							SimpleQueryNode* nod = *curnode;
							if (nod != nil)
							{
								if (nod->GetType() == QuerySimpleNode_Index)
								{
									SimpleQueryNodeIndex* qnind = (SimpleQueryNodeIndex*)nod;

									if (qnind->PeutFourche())
									{
										for (SimpleQueryNodeVector::iterator curnode2 = curnode+1; curnode2 != endnode; curnode2++)
										{
											SimpleQueryNode* nod2 = *curnode2;
											if (nod2 != nil && nod2->GetType() == QuerySimpleNode_Index)
											{
												SimpleQueryNodeIndex* qnind2 = (SimpleQueryNodeIndex*)(*curnode2).Get();
												if (qnind2->PeutFourche(qnind))
												{
													*curnode = nil;
												}
											}
										}
									}
								}
							}
						}

						SimpleQueryNodeVector::iterator newend = std::remove(Nodes.begin(), Nodes.end(), VRefPtr<SimpleQueryNode>(nil));
						Nodes.erase(newend, Nodes.end());
					}

					if (Nodes.size() == 1)
					{
						result = Nodes[0];
						if (result != nil)
							result->Retain();
					}

					qnop->ResetCacheFlags();
				}

				break;
		} // du switch
	}

	if (err == VE_OK && result == nil)
	{
		result = root;
		if (result != nil)
			result->Retain();
	}

	return result;
}


void ComplexOptimizedQuery::FillLevelString(VString &outString, sLONG level)
{
	UniChar buff[512];
	UniChar* p = &buff[0];
	sLONG len = level*2;

	for (sLONG i = 0; i < len; i++)
	{
		*p = ' ';
		p++;
	}
	outString.FromBlock(&buff[0], len*2, VTC_UTF_16);
}


VError ComplexOptimizedQuery::BuildQueryDescriptor(VString& outDescription)
{
	outDescription.Clear();
	if (fRoot != nil)
		return fRoot->AddToQueryDescriptor(outDescription, 0);
	else
		return VE_OK;
}


VValueBag* ComplexOptimizedQuery::AddToDescription(VValueBag* root, const VString s, uLONG startingMillisecs, Bittab* b)
{
	uLONG curtime = VSystem::GetCurrentTime();

	VValueBag* sub = new VValueBag;
	sub->SetString(QueryKeys::description, s);
	sub->SetLong(QueryKeys::time, curtime-startingMillisecs);
	sub->SetLong(QueryKeys::recordsfounds, b == nil ? 0 : b->Compte());
	root->AddElement(QueryKeys::steps, sub);
	sub->Release(); // sub est retained par root
	return sub;
}


VValueBag* ComplexOptimizedQuery::AddToDescription(VValueBag* root, const VString s, uLONG startingMillisecs, ComplexSelection& sel)
{
	uLONG curtime = VSystem::GetCurrentTime();

	VValueBag* sub = new VValueBag;
	sub->SetString(QueryKeys::description, s);
	sub->SetLong(QueryKeys::time, curtime-startingMillisecs);
	sub->SetLong(QueryKeys::recordsfounds, sel.GetNbRows());
	root->AddElement(QueryKeys::steps, sub);
	sub->Release(); // sub est retained par root
	return sub;
}


VError ComplexOptimizedQuery::AnalyseSearch(ComplexRech *model, BaseTaskInfo* context)
{
	VError err = VE_OK;

	fModel = model;

	err = BuildTargetSet();

	fRoot = nil;
	model->PosToFirstToken();

	if (err == VE_OK)
	{
		QueryNodeAggregate* curmap = xBuildNodes(model, err, false); // construit un arbre a partir de la formule lineaire (avec parentheses)

		if (err == VE_OK)
		{
			fRoot.Adopt(xMakeOperNode(curmap, err));

#if debuglr
			if (fRoot != nil)
			{
				VString smess;
				fRoot->AddToQueryDescriptor(smess,0);
				DebugMsg(smess);
				DebugMsg(L"\n\n");
			}
#endif
		}

		if (err == VE_OK)
		{
			QueryNode* leftover = nil;
			fRoot.Adopt(xBuildJoins(fRoot, err, leftover));
			if (leftover != nil)
			{
				leftover->Release();
				err = VE_DB4D_NOTIMPLEMENTED;
			}
			else
			{
				if (fRoot != nil && fRoot->isAllJoin() && fRoot->GetType() != QueryNode_BaseJoin)
				{
					VRefPtr<CQTargetsDef> targets(fRoot->GenerateTargets(err), false);
					//xbox_assert(targets->fTargets.size() == 2);
					if (targets->fTargets.size() == 2)
					{
						CQTableDefVector::iterator curtarget = targets->fTargets.begin();
						CQTableDef t1 = *curtarget;
						curtarget++;
						CQTableDef t2 = *curtarget;

						if (MatchTarget(t1) && MatchTarget(t2))
						{
							VRefPtr<QueryNodeBaseJoin> bjn(new QueryNodeBaseJoin(this, t1, t2), false);
							if (bjn == nil)
								err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
							else
							{
								bjn->SetJoinRoot(fRoot);
								fRoot = (QueryNode*)bjn;
							}
						}
					}
				}
			}

#if debuglr
			if (fRoot != nil)
			{
				VString smess;
				fRoot->AddToQueryDescriptor(smess,0);
				DebugMsg(smess);
				DebugMsg(L"\n\n");
			}
#endif
		}

		if (err == VE_OK)
		{
			fRoot.Adopt(xRegroupCompositeJoins(fRoot, err, context));
#if debuglr
			if (fRoot != nil)
			{
				VString smess;
				fRoot->AddToQueryDescriptor(smess,0);
				DebugMsg(smess);
				DebugMsg(L"\n\n");
			}
#endif
		}


		if (err == VE_OK)
		{
			fRoot.Adopt(xTransformIndex(fRoot, err, context));

#if debuglr
			if (fRoot != nil)
			{
				VString smess;
				fRoot->AddToQueryDescriptor(smess,0);
				DebugMsg(smess);
				DebugMsg(L"\n\n");
			}
#endif
		}

		if (err == VE_OK)
		{
			fRoot.Adopt(xTransformSeq(fRoot, false, err, context));

#if debuglr
			if (fRoot != nil)
			{
				VString smess;
				fRoot->AddToQueryDescriptor(smess,0);
				DebugMsg(smess);
				DebugMsg(L"\n\n");
			}
#endif
		}

		if (err == VE_OK)
		{
			if (fRoot != nil)
				fRoot->PermuteAnds(context);
#if debuglr
			if (fRoot != nil)
			{
				VString smess;
				fRoot->AddToQueryDescriptor(smess,0);
				DebugMsg(smess);
				DebugMsg(L"\n\n");
			}
#endif
		}


		if (curmap != nil)
			delete curmap;
	}

	if (err == VE_OK)
	{
		if (context != nil && context->ShouldDescribeQuery())
		{
			VString smess;
			fRoot->AddToQueryDescriptor(smess,0);
			context->SetQueryDescription(smess);
		}
	}


	if (err != VE_OK && err != VE_DB4D_NOTIMPLEMENTED) /* Sergiy - 10 July 2007 */
	{
		err = ThrowBaseError(VE_DB4D_CANNOT_ANALYZE_COMPLEXQUERY, DBaction_ExecutingComplexQuery);
	}

	return err;
}

typedef map<DataTable*, Bittab> MapOfKeptLocks;


VError ComplexOptimizedQuery::PerformComplex(ComplexSelection* dejasel, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, ComplexSelection* &outSel, DB4D_Way_of_Locking HowToLock, sLONG limit)
{
	VError err = VE_OK;
	outSel = nil;
	MapOfKeptLocks dejaLocked;
	Transaction* trans = GetCurrentTransaction(context);

	if (context != nil && context->ShouldDescribeQuery())
	{
		DescribeQuery(true);
	}

	if (fExecutionDescription != nil)
		fExecutionDescription->Release();

	if (IsQueryDescribed())
	{
		fExecutionDescription = new VValueBag();
	}
	else
		fExecutionDescription = nil;

	if (fRoot != nil)
	{
		fRoot->AdjusteIntl(GetContextIntl(context));
		if (HowToLock == DB4D_Keep_Lock_With_Transaction)
		{
			if (trans != nil)
			{
				for (SetOfTargets::iterator cur = fTargetSet.begin(), end = fTargetSet.end(); cur != end && err == VE_OK; cur++)
				{
					DataTable* df = cur->GetTable()->GetDF();
					if (df != nil)
					{
						MapOfKeptLocks::iterator found = dejaLocked.find(df);
						if (found == dejaLocked.end())
						{
							try
							{
								Bittab* b = &(dejaLocked[df]);
								Bittab* locks = trans->GetKeptLocks(df, err, false);
								if (locks != nil)
									err = b->Or(locks);
							}
							catch (...)
							{
								err = ThrowBaseError(memfull, DBaction_ExecutingComplexQuery);
							}
						}
					}
				}
			}
		}

		if (err == VE_OK)
			err = fRoot->PerformComplex(dejasel, InProgress, context, outSel, HowToLock, limit, fExecutionDescription);

		if (err == VE_OK)
		{
			if (HowToLock == DB4D_Keep_Lock_With_Transaction && trans != nil)
			{
				for (MapOfKeptLocks::iterator cur = dejaLocked.begin(), end = dejaLocked.end(); cur != end && err == VE_OK; cur++)
				{
					DataTable* df = cur->first;

					Bittab* locks = trans->GetKeptLocks(df, err, false);
					if (locks != nil)
					{
						Bittab xlocks;
						err = xlocks.Or(locks);
						if (err == VE_OK)
							err = xlocks.moins(&(cur->second));
						if (err == VE_OK && outSel != nil)
						{
							const CQTableDefVector* targets = outSel->GetTargets();
							for (CQTableDefVector::const_iterator curx = targets->begin(), endx = targets->end(); curx != endx && err == VE_OK; curx++)
							{
								if (curx->GetTable()->GetDF() == df)
								{
									Bittab* b = outSel->GenereBittab(curx - targets->begin(), context, err);
									if (err == VE_OK)
										err = xlocks.moins(b);
									ReleaseRefCountable(&b);
								}
							}
						}
						if (err == VE_OK)
							trans->UnlockSel(df, &xlocks, false);
					}
				}
			}

			if (err == VE_OK && outSel != nil)
			{
				if (fModel->isSimpleTarget())
				{
					const CQTableDefVector* resTargets = outSel->GetTargets();
					xbox_assert(resTargets->size() == 1);
				}
				else
				{
					const CQTargetsDef* targets = fModel->GetComplexTarget();
					err = outSel->ReOrderColumns(targets->fTargets);

					if ( err != VE_OK ) /* Sergiy - 11 July 2007 - If query fails, then SQL engine will try to execute it on its own. */
						err = VE_DB4D_NOTIMPLEMENTED;
				}

			}
		}
	}

	if (err != VE_OK)
	{
		if (HowToLock == DB4D_Keep_Lock_With_Transaction && trans != nil)
		{
			StErrorContextInstaller errs(false);

			for (MapOfKeptLocks::iterator cur = dejaLocked.begin(), end = dejaLocked.end(); cur != end; cur++)
			{
				VError err2 = VE_OK;

				DataTable* df = cur->first;

				Bittab* locks = trans->GetKeptLocks(df, err2, false);
				if (locks != nil)
				{
					Bittab xlocks;
					err2 = xlocks.Or(locks);
					if (err2 == VE_OK)
						err2 = xlocks.moins(&(cur->second));
					if (err2 == VE_OK)
						trans->UnlockSel(df, &xlocks, false);
				}
			}
		}

		if ( err != VE_DB4D_NOTIMPLEMENTED ) /* Sergiy - 11 July 2007 - No error throwing on NOTIMPLEMENTED - SQL engine will try to execute the query on its own. */
			err = ThrowBaseError(VE_DB4D_CANNOT_COMPLETE_COMPLEXQUERY, DBaction_ExecutingComplexQuery);
		if (outSel != nil)
		{
			outSel->Release();
			outSel = nil;
		}
	}

	if (context != nil && IsQueryDescribed())
	{
		VString xml;
		if (fExecutionDescription != nil)
		{
			fExecutionDescription->DumpXML(xml, L"QueryExecution", true);
			context->SetQueryExecutionXML(xml);

#if debuglr
			DebugMsg(L"\n");
			DebugMsg(xml);
			DebugMsg(L"\n\n");
#endif
			VString fulltext;
			OptimizedQuery::BuildQueryFulltext(fExecutionDescription, fulltext, 0);
			context->SetQueryExecution(fulltext);
#if debuglr
			DebugMsg(fulltext);
			DebugMsg(L"\n\n");
#endif

		}
	}

	return err;
}












