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


RootDataBaseObjectHeader::RootDataBaseObjectHeader(const void* data)
{
	fData = const_cast<void*>(data);
	fDataNeedSwap = false;
	fNeedSwapWhileSaving = false;
}


uLONG RootDataBaseObjectHeader::CalcCheckSum(sLONG len)
{
	uLONG res = 0;
	if (fData != nil)
	{
		if (fDataNeedSwap)
		{
			len = len >> 2;
			uLONG* p = (uLONG*)fData;
			for (; len>0; len--)
			{
				res = res ^ SwapLong(*p);
				p++;
			}
		}
		else
		{
			len = len >> 2;
			uLONG* p = (uLONG*)fData;
			for (; len>0; len--)
			{
				res = res ^ *p;
				p++;
			}
		}
	}
	return res;
}


/* --------------------------------- */


DataBaseObjectHeader::DataBaseObjectHeader(const void* data, sLONG len, DataBaseObjectType WhatType, sLONG PosInParent, sLONG Parent):
RootDataBaseObjectHeader(data)
{
	fDBOH.len = len;
	fDBOH.parent = Parent;
	fDBOH.pos = PosInParent;
	fDBOH.type = WhatType;
}


VError DataBaseObjectHeader::PutInto(VStream& outStream)
{
	VTime aTime;
	VError err;

	aTime.FromSystemTime();

	fDBOH.checksum = CalcCheckSum(fDBOH.len);
	fDBOH.timestamp = aTime.GetStamp();
	err = outStream.PutData(fData, sizeof(fDBOH));

	return err;
}


VError DataBaseObjectHeader::GetFrom(const VStream& inStream)
{
	VError err;

	err = const_cast<VStream&>(inStream).GetData(fData, sizeof(fDBOH));
	return err;
}


VError DataBaseObjectHeader::WriteInto(Base4D* bd, DataAddr4D offset, VString* WhereFrom, sLONG truelen)
{
	VTime aTime;
	VError err;

	aTime.FromSystemTime();

	if (fNeedSwapWhileSaving)
		fDataNeedSwap = false;
	fDBOH.checksum = CalcCheckSum(fDBOH.len);
	fDBOH.timestamp = aTime.GetStamp();
	if (fNeedSwapWhileSaving)
	{
		DataBaseObjectHeaderOnDisk xDBOH = fDBOH;
		ByteSwap(&xDBOH.type);
		ByteSwap(&xDBOH.len);
		ByteSwap(&xDBOH.timestamp);
		//ByteSwap(&xDBOH.checksum);
		ByteSwap(&xDBOH.pos);
		ByteSwap(&xDBOH.parent);
		err = bd->writelong(&xDBOH, sizeof(xDBOH), offset, 0, WhereFrom, truelen);
	}
	else
	{
		err = bd->writelong(&fDBOH, sizeof(fDBOH), offset, 0, WhereFrom, truelen);
	}

	return err;

}


VError DataBaseObjectHeader::WriteInto(Base4D_NotOpened* bd, DataAddr4D offset, VString* WhereFrom)
{
	VTime aTime;
	VError err;

	aTime.FromSystemTime();

	if (fNeedSwapWhileSaving)
		fDataNeedSwap = false;
	fDBOH.checksum = CalcCheckSum(fDBOH.len);
	fDBOH.timestamp = aTime.GetStamp();
	if (fNeedSwapWhileSaving)
	{
		DataBaseObjectHeaderOnDisk xDBOH = fDBOH;
		ByteSwap(&xDBOH.type);
		ByteSwap(&xDBOH.len);
		ByteSwap(&xDBOH.timestamp);
		//ByteSwap(&xDBOH.checksum);
		ByteSwap(&xDBOH.pos);
		ByteSwap(&xDBOH.parent);
		err = bd->writelong(&xDBOH, sizeof(xDBOH), offset, 0, WhereFrom);
	}
	else
		err = bd->writelong(&fDBOH, sizeof(fDBOH), offset, 0, WhereFrom);

	return err;

}


VError DataBaseObjectHeader::WriteInto(SegData* seg, DataAddr4D offset, VString* WhereFrom)
{
	VTime aTime;
	VError err;

	aTime.FromSystemTime();

	if (fNeedSwapWhileSaving)
		fDataNeedSwap = false;
	fDBOH.checksum = CalcCheckSum(fDBOH.len);
	fDBOH.timestamp = aTime.GetStamp();
	if (fNeedSwapWhileSaving)
	{
		DataBaseObjectHeaderOnDisk xDBOH = fDBOH;
		ByteSwap(&xDBOH.type);
		ByteSwap(&xDBOH.len);
		ByteSwap(&xDBOH.timestamp);
		//ByteSwap(&xDBOH.checksum);
		ByteSwap(&xDBOH.pos);
		ByteSwap(&xDBOH.parent);
		err = seg->writeat(&xDBOH, sizeof(xDBOH), offset);
	}
	else
		err = seg->writeat(&fDBOH, sizeof(fDBOH), offset);

	return err;

}


VError DataBaseObjectHeader::WriteInto(SegData_NotOpened* seg, DataAddr4D offset, VString* WhereFrom)
{
	VTime aTime;
	VError err;

	aTime.FromSystemTime();

	if (fNeedSwapWhileSaving)
		fDataNeedSwap = false;
	fDBOH.checksum = CalcCheckSum(fDBOH.len);
	fDBOH.timestamp = aTime.GetStamp();
	if (fNeedSwapWhileSaving)
	{
		DataBaseObjectHeaderOnDisk xDBOH = fDBOH;
		ByteSwap(&xDBOH.type);
		ByteSwap(&xDBOH.len);
		ByteSwap(&xDBOH.timestamp);
		//ByteSwap(&xDBOH.checksum);
		ByteSwap(&xDBOH.pos);
		ByteSwap(&xDBOH.parent);
		err = seg->writeat(&xDBOH, sizeof(xDBOH), offset);
	}
	else
		err = seg->writeat(&fDBOH, sizeof(fDBOH), offset);

	return err;

}



VError DataBaseObjectHeader::ReadFrom(VFileDesc* f, DataAddr4D offset)
{
	VError err;
	err = f->GetData(&fDBOH, sizeof(fDBOH), offset);
	return err;
}



VError DataBaseObjectHeader::ReadFrom(Base4D* bd, DataAddr4D offset, ReadAheadBuffer* buffer)
{
	VError err;
	err = bd->readlong(&fDBOH, sizeof(fDBOH), offset, 0, buffer);
	return err;
}


VError DataBaseObjectHeader::ReadFrom(SegData_NotOpened* seg, DataAddr4D offset)
{
	VError err;
	err = seg->readat(&fDBOH, sizeof(fDBOH), offset);
	return err;
}


VError DataBaseObjectHeader::ReadFrom(Base4D_NotOpened* bd, DataAddr4D offset)
{
	VError err;
	err = bd->readlong(&fDBOH, sizeof(fDBOH), offset, 0);
	return err;
}



Boolean DataBaseObjectHeader::Match(DataBaseObjectType signature)
{
	if (fDBOH.type == signature)
		return true;
	else
	{
		sLONG newtype = SwapLong(fDBOH.type);
		if (newtype == signature)
		{
			fDataNeedSwap = true;
			fDBOH.type = newtype;
			ByteSwap(&fDBOH.len);
			ByteSwap(&fDBOH.timestamp);
			ByteSwap(&fDBOH.checksum);
			ByteSwap(&fDBOH.pos);
			ByteSwap(&fDBOH.parent);
			return true;
		}
	}
	return false;
}


VError DataBaseObjectHeader::ValidateTag(DataBaseObjectType WhatType, sLONG PosInParent, sLONG Parent)
{
	VError err = VE_OK;
	Boolean res = true;

	if (fDBOH.type != WhatType)
	{
		sLONG newtype = SwapLong(fDBOH.type);
		if (newtype == WhatType)
		{
			fDataNeedSwap = true;
			fDBOH.type = newtype;
			ByteSwap(&fDBOH.len);
			ByteSwap(&fDBOH.timestamp);
			ByteSwap(&fDBOH.checksum);
			ByteSwap(&fDBOH.pos);
			ByteSwap(&fDBOH.parent);
		}
	}
	res = res && (WhatType == fDBOH.type);
	res = res && (PosInParent == fDBOH.pos);
	res = res && (Parent == fDBOH.parent);

	if (!res)
		err = VE_DB4D_WRONG_TAG_HEADER;
	//err = ThrowBaseError(VE_DB4D_WRONG_TAG_HEADER, DBactionFinale);
	return err;
}

VError DataBaseObjectHeader::ValidateCheckSum(const void* data, sLONG len)
{
	VError err = VE_OK;

	if (len != fDBOH.len)
		err = ThrowBaseError(VE_DB4D_WRONG_CHECKSUM, DBactionFinale);
	else
	{
		if (len != 0 && data != nil)
		{
			fData = const_cast<void*>(data);
			sLONG xcheck = CalcCheckSum(len);
			if (xcheck != fDBOH.checksum)
				err = ThrowBaseError(VE_DB4D_WRONG_CHECKSUM, DBactionFinale);
		}
	}

	return err;
}



/* --------------------------------- */


RecordHeader::RecordHeader(const void* data, sLONG len, DataBaseObjectType WhatType, sLONG PosInParent, sLONG Parent, sLONG nbfields):
RootDataBaseObjectHeader(data)
{
	fDBOH.len = len;
	fDBOH.parent = Parent;
	fDBOH.pos = PosInParent;
	fDBOH.type = WhatType;
	fDBOH.nbfields = nbfields;
}



VError RecordHeader::PutInto(VStream& outStream)
{
	VTime aTime;
	VError err;

	aTime.FromSystemTime();

	fDBOH.checksum = CalcCheckSum(fDBOH.len);
	fDBOH.timestamp = aTime.GetStamp();
	err = outStream.PutData(fData, sizeof(fDBOH));

	return err;
}


VError RecordHeader::GetFrom(const VStream& inStream)
{
	VError err;

	err = const_cast<VStream&>(inStream).GetData(fData, sizeof(fDBOH));
	return err;
}


VError RecordHeader::WriteInto(SegData* seg, DataAddr4D offset, VString* WhereFrom)
{
	VTime aTime;
	VError err;

	aTime.FromSystemTime();

	if (fNeedSwapWhileSaving)
		fDataNeedSwap = false;
	fDBOH.checksum = CalcCheckSum(fDBOH.len);
	fDBOH.timestamp = aTime.GetStamp();
	if (fNeedSwapWhileSaving)
	{
		RecordHeaderOnDisk xDBOH = fDBOH;
		ByteSwap(&xDBOH.type);
		ByteSwap(&xDBOH.len);
		ByteSwap(&xDBOH.timestamp);
		//ByteSwap(&xDBOH.checksum);
		ByteSwap(&xDBOH.pos);
		ByteSwap(&xDBOH.parent);
		ByteSwap(&xDBOH.nbfields);
		err = seg->writeat(&xDBOH, sizeof(xDBOH), offset);
	}
	else
		err = seg->writeat(&fDBOH, sizeof(fDBOH), offset);

	return err;

}


VError RecordHeader::WriteInto(SegData_NotOpened* seg, DataAddr4D offset, VString* WhereFrom)
{
	VTime aTime;
	VError err;

	aTime.FromSystemTime();

	if (fNeedSwapWhileSaving)
		fDataNeedSwap = false;
	fDBOH.checksum = CalcCheckSum(fDBOH.len);
	fDBOH.timestamp = aTime.GetStamp();
	if (fNeedSwapWhileSaving)
	{
		RecordHeaderOnDisk xDBOH = fDBOH;
		ByteSwap(&xDBOH.type);
		ByteSwap(&xDBOH.len);
		ByteSwap(&xDBOH.timestamp);
		//ByteSwap(&xDBOH.checksum);
		ByteSwap(&xDBOH.pos);
		ByteSwap(&xDBOH.parent);
		ByteSwap(&xDBOH.nbfields);
		err = seg->writeat(&xDBOH, sizeof(xDBOH), offset);
	}
	else
		err = seg->writeat(&fDBOH, sizeof(fDBOH), offset);

	return err;

}


VError RecordHeader::WriteInto(Base4D* bd, DataAddr4D offset, VString* WhereFrom, sLONG truelen)
{
	VTime aTime;
	VError err;

	//aTime.FromSystemTime();

	if (fNeedSwapWhileSaving)
		fDataNeedSwap = false;
	fDBOH.checksum = CalcCheckSum(fDBOH.len);
	//fDBOH.timestamp = aTime.GetStamp();
	if (fNeedSwapWhileSaving)
	{
		RecordHeaderOnDisk xDBOH = fDBOH;
		ByteSwap(&xDBOH.type);
		ByteSwap(&xDBOH.len);
		ByteSwap(&xDBOH.timestamp);
		//ByteSwap(&xDBOH.checksum);
		ByteSwap(&xDBOH.pos);
		ByteSwap(&xDBOH.parent);
		ByteSwap(&xDBOH.nbfields);
		err = bd->writelong(&xDBOH, sizeof(xDBOH), offset, 0, WhereFrom, truelen);
	}
	else
		err = bd->writelong(&fDBOH, sizeof(fDBOH), offset, 0, WhereFrom, truelen);

	return err;

}


VError RecordHeader::WriteInto(Base4D_NotOpened* bd, DataAddr4D offset, VString* WhereFrom)
{
	VTime aTime;
	VError err;

	aTime.FromSystemTime();

	if (fNeedSwapWhileSaving)
		fDataNeedSwap = false;
	fDBOH.checksum = CalcCheckSum(fDBOH.len);
	fDBOH.timestamp = aTime.GetStamp();
	if (fNeedSwapWhileSaving)
	{
		RecordHeaderOnDisk xDBOH = fDBOH;
		ByteSwap(&xDBOH.type);
		ByteSwap(&xDBOH.len);
		ByteSwap(&xDBOH.timestamp);
		//ByteSwap(&xDBOH.checksum);
		ByteSwap(&xDBOH.pos);
		ByteSwap(&xDBOH.parent);
		ByteSwap(&xDBOH.nbfields);
		err = bd->writelong(&xDBOH, sizeof(xDBOH), offset, 0, WhereFrom);
	}
	else
		err = bd->writelong(&fDBOH, sizeof(fDBOH), offset, 0, WhereFrom);

	return err;

}


VError RecordHeader::WriteToStream(VStream* log)
{
	VTime aTime;
	VError err;

	aTime.FromSystemTime();

	fDBOH.checksum = CalcCheckSum(fDBOH.len);
	fDBOH.timestamp = aTime.GetStamp();
	err = log->PutData(&fDBOH, sizeof(fDBOH));

	return err;

}


VError RecordHeader::ReadFrom(Base4D* bd, DataAddr4D offset, ReadAheadBuffer* buffer)
{
	VError err;

	err = bd->readlong(&fDBOH, sizeof(fDBOH), offset, 0, buffer);
	return err;
}


VError RecordHeader::ReadFrom(Base4D_NotOpened* bd, DataAddr4D offset)
{
	VError err;

	err = bd->readlong(&fDBOH, sizeof(fDBOH), offset, 0);
	return err;
}


VError RecordHeader::ReadFrom(SegData_NotOpened* seg, DataAddr4D offset)
{
	VError err;

	err = seg->readat(&fDBOH, sizeof(fDBOH), offset);
	return err;
}


VError RecordHeader::ReadFrom(VFileDesc* f, DataAddr4D offset)
{
	VError err;
	err = f->GetData(&fDBOH, sizeof(fDBOH), offset);
	return err;
}


VError RecordHeader::ReadFromStream(VStream* log)
{
	VError err;

	err = log->GetData(&fDBOH, sizeof(fDBOH));
	return err;
}


Boolean RecordHeader::Match(DataBaseObjectType signature)
{
	if (fDBOH.type == signature)
		return true;
	else
	{
		sLONG newtype = SwapLong(fDBOH.type);
		if (newtype == signature)
		{
			fDataNeedSwap = true;
			fDBOH.type = newtype;
			ByteSwap(&fDBOH.len);
			ByteSwap(&fDBOH.timestamp);
			ByteSwap(&fDBOH.checksum);
			ByteSwap(&fDBOH.pos);
			ByteSwap(&fDBOH.parent);
			ByteSwap(&fDBOH.nbfields);
			return true;
		}
	}
	return false;
}


VError RecordHeader::ValidateTag(DataBaseObjectType WhatType, sLONG PosInParent, sLONG Parent)
{
	VError err = VE_OK;
	Boolean res = true;

	if (fDBOH.type != WhatType)
	{
		sLONG newtype = SwapLong(fDBOH.type);
		if (newtype == WhatType)
		{
			fDataNeedSwap = true;
			fDBOH.type = newtype;
			ByteSwap(&fDBOH.len);
			ByteSwap(&fDBOH.timestamp);
			ByteSwap(&fDBOH.checksum);
			ByteSwap(&fDBOH.pos);
			ByteSwap(&fDBOH.parent);
			ByteSwap(&fDBOH.nbfields);
		}
	}

	res = res && (WhatType == fDBOH.type);
	res = res && (PosInParent == fDBOH.pos);
	res = res && (Parent == fDBOH.parent);

	if (!res)
		err = VE_DB4D_WRONGRECORDHEADER;
	return err;
}


VError RecordHeader::ValidateCheckSum(const void* data, sLONG len)
{
	VError err = VE_OK;

	if (len != fDBOH.len)
		err = VE_DB4D_WRONGRECORDHEADER;
	else
	{
		if (len != 0 && data != nil)
		{
			fData = const_cast<void*>(data);
			sLONG xcheck = CalcCheckSum(len);
			if (xcheck != fDBOH.checksum)
				err = VE_DB4D_WRONGRECORDHEADER;
		}
	}

	return err;
}



/* *********************************** */


void SegDataDisk2::SwapBytes()
{
	ByteSwap(&finfic);
	ByteSwap(&limitseg);
	ByteSwap(&AddPagePrim);
	ByteSwap(&sizeblockseg);
	ByteSwap(&ratio);
}


/* *********************************** */


#if debugLogEventFindFree

sLONG gDebugCountFindFree = 0;
sLONG gDebugCountTryToFreeDataSeg = 0;

class dbgFindFree
{
	public:
		inline dbgFindFree()
		{
			fNbCalcPage = 0;
			fNbPageEncours = 0;
			fSegSize = 0;
		}

		inline dbgFindFree(sLONG inNbCalcPage, sLONG inNbPageEncours, DataAddr4D inSegSize)
		{
			fNbCalcPage = inNbCalcPage;
			fNbPageEncours = inNbPageEncours;
			fSegSize = inSegSize;
		}

		sLONG fNbCalcPage;
		sLONG fNbPageEncours;
		DataAddr4D fSegSize;
};

EventLogger<dbgFindFree, 1000> gDebugEventFindFree;

#endif



FreebitsSec::FreebitsSec(SegData* segowner, sLONG NumInTable, DataAddr4D addr, uBOOL init)
{
	if (NumInTable == 0)
		SetInProtectedArea();
	seg = segowner;
	No = NumInTable;
	fNbPagesEncours = -1;
	if (addr!=0) 
		setaddr(StripDataAddr(addr)+segowner->GetNum()-1);
	savetfb = false;
	savefbmaxlibre = false;
	if (init)
	{
		_raz(&tfb, sizeof(tfb));
		_rau(&fbmaxlibre, sizeof(fbmaxlibre));
		_raz(&tfbm, sizeof(tfbm));
	}
}


FreebitsSec::~FreebitsSec()
{
	bittableptr fb;
	bittableptr* p2 = tfbm;
	sLONG nbp,j;

#if debuglr
	if (modifie())
	{
		xbox_assert(!modifie());
	}
#endif
	nbp = calcNBpages();

	for (j=0; (j<nbp); j++)
	{
		fb = *p2;
		++p2;

		if (fb != nil)
		{
			fb->Release();
		}
	}
}


sLONG FreebitsSec::calcNBpages(void)
{
	sLONG nbp, nb;
	SegDataDisk2 *sd = seg->GetSDD();

	nb = (sLONG) ((sd->finfic+taillegroupesec-1) >> ratiogroupebitsec);
	if (No==(nb-1))
	{
		nbp = (sLONG) ( ( (sd->finfic & (taillegroupesec-1)) + taillegroupebit-1 ) >> ratiogroupebit);
		if ( nbp == 0 )
			nbp = NBpagedebit;
	}
	else
		nbp = NBpagedebit;

	return(nbp);
}


VError FreebitsSec::chargetfb(void)
{
	sLONG nbp = calcNBpages();
	VError err;
	DataAddr4D ou = StripDataAddr(getaddr());

#if debugLogEventFindFree
	gDebugEventFindFree.AddMessage(L"Avant chargetfb", dbgFindFree(nbp, fNbPagesEncours, seg->Getfinfic()));
#endif

	_raz(&tfb, sizeof(tfb));
	_raz(&tfbm, sizeof(tfbm));
	_rau(&fbmaxlibre, sizeof(fbmaxlibre));

	err = seg->readat( &tfb, nbp<<3, ou);
#if BIGENDIAN
	ByteSwap(&tfb[0], nbp);
#endif
	if (err == VE_OK)
		err = seg->readat( &fbmaxlibre, nbp<<1, ou+(DataAddr4D)sizetfb );
#if BIGENDIAN
	ByteSwap(&fbmaxlibre[0], nbp);
#endif
	savefbmaxlibre = false;
	savetfb = false;

	if (err != VE_OK)
		err = seg->ThrowError(VE_DB4D_CANNOTLOADFREEBITTABLE, DBaction_LoadingFreeBitTable);

#if debugLogEventFindFree
	gDebugEventFindFree.AddMessage(L"Apres chargetfb", dbgFindFree(nbp, fNbPagesEncours, seg->Getfinfic()));
#endif

	return err;
}


bittableptr FreebitsSec::chargefb(sLONG i, VError& err, OccupableStack* curstack)
{
	bittableptr fbt;
	FreeBitsptr fb;
	err = VE_OK;

	fbt = tfbm[i];
	if (fbt == nil)
	{
		fbt=new bittable(seg->GetDB(),NBlongbit);
		if (fbt!=nil)
		{
			if (fbt->okcree())
			{
				fb=fbt->getfbb();
				DataBaseObjectHeader tag;
				DataAddr4D ou = tfb[i];
				err = tag.ReadFrom(seg->GetDB(), ou, nil);
				if (err == VE_OK)
					err = tag.ValidateTag(DBOH_Bittab, -1, -1);
				if (err == VE_OK)
					err = seg->readat( fb, sizepagebit, StripDataAddr(ou) + kSizeDataBaseObjectHeader);
				if (err == VE_OK)
					err = tag.ValidateCheckSum(fb, sizepagebit);
				fbt->setaddr(ou);
				if (err!=VE_OK)
				{
					fbt->Release();
					fbt=nil;
				}
				else
				{
					if (tag.NeedSwap())
						ByteSwap((sLONG*)fb, NBlongbit);
					fbt->unlock();
					fbt->Occupy(curstack, true);
					tfbm[i]=fbt;
				}
			}
			else
			{
				err = seg->ThrowError(memfull, DBaction_LoadingFreeBit);
				fbt->Release();
				fbt=nil;
			}
		}
		else
		{
			err = seg->ThrowError(memfull, DBaction_LoadingFreeBit);
		}
	}
	else
		fbt->Occupy(curstack, true);

	if (err != VE_OK)
		err = seg->ThrowError(VE_DB4D_CANNOTLOADFREEBIT, DBaction_LoadingFreeBit);

	return(fbt);
}


sWORD FreebitsSec::CalcMaxFree(void)
{
	sLONG nbp = calcNBpages();
	sLONG i;
	sWORD res = -1, z;
	sWORD* p = fbmaxlibre;

	for (i = 0; i<nbp; i++)
	{
		z = *p; ++p;
		if (z>res) res = z;
	}

	return res;
}


bool FreebitsSec::SaveObj(VSize& outSizeSaved)
{
	sLONG tot,nb,offset;
	DataAddr4D ou;
	uBOOL oneinuse;
	VError err = VE_OK;

	tot=0;
	oneinuse=false;
#if debugLogEventFindFree
	sLONG nbcalcpage = calcNBpages();
	gDebugEventFindFree.AddMessage(L"Avant SaveObj", dbgFindFree(nbcalcpage, fNbPagesEncours, seg->Getfinfic()));
#endif

	if (fNbPagesEncours == -1)
		nb = calcNBpages();
	else
	{
		nb = fNbPagesEncours;
#if debugLogEventFindFree
		if (nb < nbcalcpage)
		{
			nb = nb; // break here
			xbox_assert(nb >= nbcalcpage);
		}
#endif
	}
	ou = StripDataAddr(getaddr());
	// if (savetfb)  // on sauve toujours c'est plus sure
	{
#if BIGENDIAN
		ByteSwap(&tfb[0], nb);
#endif
		err = seg->writeat( tfb, nb<<3, ou);
#if BIGENDIAN
		ByteSwap(&tfb[0], nb);
#endif
		savetfb=false;
		tot = tot + (nb<<3);
	}

	// if (savefbmaxlibre)  // on sauve toujours c'est plus sure
	{
#if BIGENDIAN
		ByteSwap(&fbmaxlibre[0], nb);
#endif
		err = seg->writeat( fbmaxlibre, nb<<1, StripDataAddr(ou) + (DataAddr4D)sizetfb );
#if BIGENDIAN
		ByteSwap(&fbmaxlibre[0], nb);
#endif
		savefbmaxlibre = false;
		tot = tot + (nb<<1);
	}


	if (err != VE_OK)
		err = seg->ThrowError(VE_DB4D_CANNOTSAVEFREEBIT, DBaction_SavingFreeBit);

	if (oneinuse)
		tot=0;
	outSizeSaved = tot;

#if debugLogEventFindFree
	nbcalcpage = calcNBpages();
	gDebugEventFindFree.AddMessage(L"Apres SaveObj", dbgFindFree(nbcalcpage, fNbPagesEncours, seg->Getfinfic()));
#endif

	return true;
}


void FreebitsSec::setfbt(sLONG n, bittable *fbt)
{
	xbox_assert(n < NBpagedebit);
	tfbm[n]=(bittableptr)fbt;
}


bool FreebitsSec::TryToFree(sLONG allocationBlockNumber, VSize inNeedToFree, VSize& outFreed, bool ForceAll)
{
	bool allfreed = true;
	if (!IsOccupied())
	{
#if debugLogEventFindFree
		gDebugCountTryToFreeDataSeg++;
#endif
		sLONG nbp = calcNBpages();
#if debugLogEventFindFree
		gDebugEventFindFree.AddMessage(L"Avant TryToFree", dbgFindFree(nbp, fNbPagesEncours, seg->Getfinfic()));
#endif
		if (fNbPagesEncours != -1)
		{
#if debugLogEventFindFree
			if (fNbPagesEncours < nbp)
			{
				nbp = nbp; // break here
				xbox_assert(fNbPagesEncours >= nbp);
			}
#endif
			nbp = fNbPagesEncours;
		}
		bittableptr* pfbt = tfbm;

		outFreed = 0;

		sLONG i;
		for (i = 0; i < nbp; i++, pfbt++)
		{
			bittableptr fbt = *pfbt;
			if (fbt != nil)
			{
				if (! fbt->IsOccupied() && !fbt->modifie())
				{
					if (allocationBlockNumber == -1 || ForceAll || GetAllocationNumber(fbt) == allocationBlockNumber)
					{
						outFreed = outFreed + fbt->getsize();
#if VERSIONDEBUG
						if (fbt->GetRefCount() != 1)
						{
							xbox_assert(fbt->GetRefCount() == 1);
						}
#endif
						fbt->Release();
						*pfbt = nil;
					}
					else
						allfreed = false;
				}
				else
					allfreed = false;
			}
		}
		if (i < nbp)
			allfreed = false;
#if debugLogEventFindFree
		gDebugCountTryToFreeDataSeg--;
		gDebugEventFindFree.AddMessage(L"Apres TryToFree", dbgFindFree(calcNBpages(), fNbPagesEncours, seg->Getfinfic()));
#endif
	}
	else
		allfreed = false;

	if (modifie())
		allfreed = false;

	return allfreed;
}






									// -----------------------------------------------------------------



SegData::SegData( Base4D *inBase, const BaseHeader& inSegInfo, sLONG inNumSeg, VFile *inFile, VFileDesc* inDesc, SegDataDisk2 *realSDD)
{
	xbox_assert( inFile != nil);
	db = inBase;
	if (realSDD == nil)
	{	
		SDDInside = inSegInfo.seginfo;
		SDDx = &SDDInside;
	}
	else
	{
		_raz(&SDDInside, sizeof(SDDInside));
		SDDx = realSDD;
	}

	isopen=false;
	savetfb=false;
	savefbmaxlibre = false;
	ecrisfinfic=false;
	fFile = inFile;
	fFileDesc = inDesc;
	tfbprim = nil;
	tfbdiskprim = nil;
	fbmaxlibreprim = nil;
	WriteProtected = false;
	numseg = inNumSeg;
	fLastPageSec = 0;
	fLastPage = 0;
	fLastBloc = 0;
	fOnePassOnly = true;
	fCountLibereBit = 0;
	fWriteBuffer = nil;
	fWriteBufferLen = 0;
	fWriteBufferOffset = 0;
	fWriteBufferStart = -1;
	SetInProtectedArea();
}

SegData::~SegData()
{
	sLONG i,j;
	sLONG nb;

	FlushBuffer();

	if (fWriteBuffer != nil && fWriteBuffer != (void*)-1)
		VObject::GetMainMemMgr()->Free(fWriteBuffer);
	CloseSeg();

	if (tfbprim != nil)
	{
		nb = tfbprim->GetCount();
		for (i = 0; i<nb; i++)
		{
			FreebitsSecPtr p = (*tfbprim)[i];
			if (p != nil)
			{
				p->Release();
			}
		}
		delete tfbprim;
	}

	if (tfbdiskprim != nil) 
		delete tfbdiskprim;

	if (fbmaxlibreprim != nil) 
		delete fbmaxlibreprim;

	if (fFile != nil)
		fFile->Release();

	if ( fFileDesc )
	{
		delete fFileDesc;
		fFileDesc = nil;
	}

}


VError SegData::CloseSeg()
{
	VError err = VE_OK;
	RemoveFromCache();

	fAccess.LockWrite();
	//trytofree(0,0, forceall);

	if (fFileDesc != nil)
	{
		delete fFileDesc;
		fFileDesc = NULL;
	}

	if (fFile != nil) {
		fFile->Release();
		fFile = nil;
	}

	fAccess.Unlock();

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTCLOSESEG, DBaction_ClosingSeg);
	return err;
}


VError SegData::DeleteSeg()
{
	VError err = VE_OK;
	err = CloseSeg();
	if (fFile != nil) {
		err = fFile->Delete();
	}
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTDELETESEG, DBaction_DeletingSeg);
	return err;
}




VError SegData::OpenSeg(FileAccess InAccess)
{
	VError err = VE_OK;
	if (!fFileDesc)
		err = fFile->Open( InAccess, &fFileDesc );

	if (err == VE_OK)
		WriteProtected = fFileDesc->GetMode() <= FA_READ;

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTOPENSEG, DBaction_OpeningSeg);
	return err;
}


VError SegData::CreateSeg(const VUUID* StructID, FileAccess inAccess)
{
	VError err = VE_OK;
	VError err2;

	if (!fFileDesc)
		err = fFile->Open(inAccess,&fFileDesc,FO_CreateIfNotFound);

	/*

	BaseHeader (multiple de 128 octets)
	Zone Extra optionnelle (taille precisee dans BaseHeader)
	Table des freebits (sizetfb = 16ko)
	Premiere bittable (sizepagebit = 512 bytes)

	*/

	if (err == VE_OK) {
		DataAddr4D newSize = db->GetBaseHeaderSize() + sizetfb + sizepagebit + ktaillebloc /* pour les headers de la page de bit */ + sizefbmaxlibre;
		newSize = (newSize+blockEOF-1) & (-blockEOF);
		err = fFileDesc->SetSize( newSize + 512L);


		sLONG len = db->GetBaseHeaderSize();
		VPtr p = (VPtr) vCalloc( 1, len, 'seg ');
		_raz(p, len);
		BaseHeader* p2 = (BaseHeader*)p;
		if (StructID != nil)
		{
			StructID->ToBuffer(p2->GetID());
			p2->VersionNumber = db->GetVersionNumber() /*kVersionDB4D*/;
		}
		p2->tag = tagHeader4D;
		/*
		*((uLONG*)p) = tagHeader4D;
		*/
		err = fFileDesc->PutData( p, len, 0);
		vFree( p);

		if (err == VE_OK) {
			p = (VPtr) vCalloc( 1, sizetfb, 'seg ');
			_raz(p,sizetfb);
			*((DataAddr4D*)p) = db->GetBaseHeaderSize() + sizetfb + sizefbmaxlibre + numseg - 1; // adr de la 1ere bittable
#if BIGENDIAN
			ByteSwap((DataAddr4D*)p);
#endif
			err = fFileDesc->PutData( p, sizetfb, db->GetBaseHeaderSize());
			vFree( p);

			SDDx->finfic = db->GetBaseHeaderSize() + sizetfb + sizefbmaxlibre + sizepagebit + ktaillebloc;
			FreeBitsptr fb = (FreeBitsptr) vCalloc( 1, sizepagebit, 'seg ');
			_raz(fb, sizepagebit);
			PrendsBits(fb,0,SDDx->finfic);

			p = (VPtr) vCalloc( 1, sizefbmaxlibre, 'seg ');
			_rau(p,sizefbmaxlibre);
			*((sWORD*)p) = maxbitfree(fb);
#if BIGENDIAN
			ByteSwap((sWORD*)p);
#endif
			if (err == VE_OK)
				err = fFileDesc->PutData( p, sizefbmaxlibre, db->GetBaseHeaderSize() + sizetfb);
			vFree( p);

			if (err == VE_OK) {

				newSize = (SDDx->finfic+blockEOF-1) & (-blockEOF);
				err = fFileDesc->SetSize( newSize + 512);

				if (err == VE_OK)
				{
					DataBaseObjectHeader tag(fb, sizepagebit, DBOH_Bittab, -1, -1);
					tag.WriteInto(this, db->GetBaseHeaderSize() + sizetfb + sizefbmaxlibre);
					err = fFileDesc->PutData( fb, sizepagebit, db->GetBaseHeaderSize() + sizetfb + sizefbmaxlibre + kSizeDataBaseObjectHeader);
				}
			}
			vFree( fb);
		}


		if (err != VE_OK) {
			delete fFileDesc;
			fFileDesc = NULL;
			PullLastError();

			err2 = fFile->Delete();
			if (err2 != VE_OK)
				PullLastError();
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTCREATESEG, DBaction_CreatingSeg);
	return err;
}

void SegData::PrendsBits(FreeBitsptr fb, sLONG start, sLONG end)
{
	sLONG i;
	uLONG nbit;
	uLONG *p;

	start=(start>>SDDx->ratio) & (NBbitparpage-1);
	end=((end-1)>>SDDx->ratio) & (NBbitparpage-1);
	// end=( (end+SDDx->sizeblockseg-1)>>SDDx->ratio ) & (NBbitparpage-1);

	nbit=start & 31;
	p=((uLONG*)fb) + (start>>5);
	for (i=start; i<=end; i++)
	{
		*p=*p | (1<<nbit);
		nbit++;
		if (nbit>31)
		{
			nbit=0;
			p++;
		}
	}
}


void SegData::ClearBits(FreeBitsptr fb, sLONG start, sLONG end)
{
	sLONG i;
	uLONG nbit;
	uLONG *p;

	start=(start>>SDDx->ratio) & (NBbitparpage-1);
	end=((end-1)>>SDDx->ratio) & (NBbitparpage-1);
	// end=( (end+SDDx->sizeblockseg-1)>>SDDx->ratio ) & (NBbitparpage-1);

	nbit=start & 31;
	p=((uLONG*)fb) + (start>>5);
	for (i=start; i<=end; i++)
	{
		*p=*p & (ALLFF ^ (1<<nbit));
		nbit++;
		if (nbit>31)
		{
			nbit=0;
			p++;
		}
	}
}

// vérifie que tous les bits sont à la même valeur que test
Boolean SegData::TestBits(DataAddr4D ou, sLONG len, Boolean test) // J.A. 10/2004
{
	sLONG i;
	DataAddr4D start,end,substart,subend;
	FreeBitsptr fb;
	bittableptr fbt;
	VError err;
	sWORD maxbit;
	Boolean result = true;

	fAccess.LockWrite();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	// IncNbAcces();
	err = chargetfb();

	if (err==VE_OK)
	{

		DataAddr4D xx = ou;

		sLONG LastPageSec = xx >> ratiogroupebitsec;
		xx = xx & (taillegroupesec - 1);
		sLONG LastPage = xx >> ratiogroupebit;
		xx = xx & (taillegroupebit - 1);
		sLONG LastBloc = xx >> kratio;

		if ( (LastPageSec<=fLastPageSec) && (LastPage<=fLastPage) && (LastBloc<fLastBloc))
		{
			fLastPageSec = LastPageSec;
			fLastPage = LastPage;
			fLastBloc = LastBloc;
		}


		start=ou>>ratiogroupebit;
		end=(ou+len-1)>>ratiogroupebit;
		for (i=start; (i<=end) && (err==VE_OK) && (result); i++)
		{
			fbt=chargefb(i, err, curstack);
			if (fbt!=nil)
			{
				fb=fbt->getfbb();

				if (i==start)
				{
					substart=ou & (taillegroupebit-1);
				}
				else
				{
					substart=0;
				}

				if (i==end)
				{
					subend=(ou+len) & (taillegroupebit-1);
				}
				else
				{
					subend=taillegroupebit-1;
				}

				result = result && TestBits(fb,substart,subend,test);

				fbt->unlock();
				fbt->Free(curstack, true);
			}
		}
	}

	CheckForMemRequest();
	fAccess.Unlock();

	return result;
}

Boolean SegData::TestBits(FreeBitsptr fb, sLONG start, sLONG end,Boolean test)
{
	sLONG i;
	uLONG nbit;
	uLONG *p;
	Boolean result = true;
	start=(start>>SDDx->ratio) & (NBbitparpage-1);
	end=((end-1)>>SDDx->ratio) & (NBbitparpage-1);
	// end=( (end+SDDx->sizeblockseg-1)>>SDDx->ratio ) & (NBbitparpage-1);

	nbit=start & 31;
	p=((uLONG*)fb) + (start>>5);
	for (i=start; i<=end && result; i++)
	{
		result = (((*p & (1<<nbit))!=0) == test);
		nbit++;
		if (nbit>31)
		{
			nbit=0;
			p++;
		}
	}
	return result;
}

VError SegData::chargetfb(void)
{
	sWORD *p;
	sLONG i;
	VError err;
	StAllocateInCache alloc;

	sLONG nbbitpagesec=CalcNbpageSec();

	err=VE_OK;

	if (fbmaxlibreprim==nil)
	{
		fbmaxlibreprim=new V0ArrayOf<sWORD>();
		if (fbmaxlibreprim==nil)
		{
			err=ThrowError(memfull, DBaction_LoadingFreeBitTable);
		}
		else
		{
			if (!fbmaxlibreprim->AddNSpaces(nbbitpagesec, false))
			{
				err=ThrowError(memfull, DBaction_LoadingFreeBitTable);
			}
			else
			{
				if (nbbitpagesec>0)
				{
					p=fbmaxlibreprim->First();
					for (i=0;i<nbbitpagesec;i++)
					{
						*p++=-1;
					}
				}
			}
		}
	}

	if ((tfbprim==nil) && (err==VE_OK))
	{
		savefbmaxlibre = false;
		savetfb=false;
		tfbprim=new FreebitsSecArray();
		if (tfbprim==nil)
		{
			err=ThrowError(memfull, DBaction_LoadingFreeBitTable);
		}
		else
		{
			if (!tfbprim->AddNSpaces(nbbitpagesec, false))
			{
				err=ThrowError(memfull, DBaction_LoadingFreeBitTable);
			}
			else
			{
				_raz(tfbprim->First(), tfbprim->GetCount() * sizeof(FreebitsSecPtr));

				if (SDDx->AddPagePrim == 0) // si data inferieur a 17G alors pas de table primaire
				{
					// rien a faire ici
				}
				else
				{
					tfbdiskprim = new DiskAddrArray();
					if (tfbdiskprim == nil)
					{
						err=ThrowError(memfull, DBaction_LoadingFreeBitTable);
					}
					else
					{
						if (!tfbdiskprim->AddNSpaces(nbbitpagesec, false))
						{
							err=ThrowError(memfull, DBaction_LoadingFreeBitTable);
						}
						else
						{
							if (nbbitpagesec>0)
							{
								err = readat(tfbdiskprim->First(), nbbitpagesec<<3, StripDataAddr(SDDx->AddPagePrim));
#if BIGENDIAN
								ByteSwap(tfbdiskprim->First(), nbbitpagesec);
#endif

								if (err == VE_OK)
								{
									err = readat(fbmaxlibreprim->First(), nbbitpagesec<<1, StripDataAddr(SDDx->AddPagePrim)+sizetfbprim);
#if BIGENDIAN
									ByteSwap(fbmaxlibreprim->First(), nbbitpagesec);
#endif
								}

							}
						}
					}
					if (err != VE_OK)
					{
						delete tfbdiskprim;
						tfbdiskprim=nil;
					}
				}

				if (err!=VE_OK)
				{
					delete tfbprim;
					tfbprim=nil;
				}
			}
		}
	}

	if (err != VE_OK)	
		err=ThrowError(VE_DB4D_CANNOTLOADFREEBITTABLE, DBaction_LoadingFreeBitTable);
	return(err);
}


bool SegData::FreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed)
{
	bool okfree = false;

	if (fAccess.TryToLockWrite())
	{
		sLONG nb;
		VSize tot;
		bool okdel = true;

		if (combien==0)
			combien=kMaxPositif;

		tot=0;

		if (tfbprim != nil)
		{
			for (FreebitsSecArray::Iterator cur = tfbprim->First(), end = tfbprim->End(); cur != end /*&& tot < combien*/; cur++)
			{
				FreebitsSecPtr sec = *cur;
				if (sec != nil)
				{
					VSize subFreed;
					if (sec->TryToFree(allocationBlockNumber, combien, subFreed))
					{
						*cur = nil;
						//sec->RemoveFromFlush();
						sec->Release();

					}
					else
						okdel = false;
					outSizeFreed += subFreed;
				}
			}

			if (okdel && combien == -5)
			{
				tot=tot+tfbprim->GetAllocatedSize();
				delete tfbprim;
				tfbprim=nil;
			}
		}

		if ((tfbdiskprim!=nil) && (!savetfb) && (combien == -5) )
		{
			tot=tot+tfbdiskprim->GetAllocatedSize();
			delete tfbdiskprim;
			tfbdiskprim=nil;
			savetfb=false;
		}

		if ((fbmaxlibreprim!=nil) && (!savefbmaxlibre) && (combien == -5) )
		{
			tot=tot+fbmaxlibreprim->GetAllocatedSize();
			delete fbmaxlibreprim;
			fbmaxlibreprim=nil;
		}

		ClearMemRequest();
		outSizeFreed = tot;
		fAccess.Unlock();
		okfree = true;
	}

	return(okfree);
}


bittableptr SegData::chargefb(sLONG i, VError& err, OccupableStack* curstack)
{
	sLONG n,n2;
	bittableptr fbt = nil;
	FreebitsSecPtr fbsec;

	n = i >> kratiosec;
	n2 = i & (NBpagedebit - 1);

	fbsec = chargefbsec(n, err, curstack);

	if (fbsec != nil)
	{
		fbt = fbsec->chargefb(n2, err, curstack);
		fbsec->Free(curstack, true);
	}

	return(fbt);
}


FreebitsSecPtr SegData::chargefbsec(sLONG i, VError& err, OccupableStack* curstack)
{
	FreebitsSecPtr fbsec;
	err = VE_OK;

	fbsec = (*tfbprim)[i];
	if (fbsec == nil)
	{
		if (SDDx->AddPagePrim == 0)
		{
			xbox_assert(i == 0);
			fbsec = new FreebitsSec(this, i, db->GetBaseHeaderSize()+numseg - 1);
		}
		else
		{
			fbsec = new FreebitsSec(this, i, (*tfbdiskprim)[i]);
		}

		if (fbsec == nil)
		{
			err=ThrowError(memfull, DBaction_LoadingFreeBitTable);
		}
		else
		{
			err = fbsec->chargetfb();
			if (err == 0)
			{
				fbsec->Occupy(curstack, true);
				(*tfbprim)[i] = fbsec;
			}

			if (err != 0)
			{ 
				fbsec->Release();
				fbsec = nil;
			}
		}
	}
	else 
		fbsec->Occupy(curstack, true);

	return fbsec;
}


#define debuglogFindPlace 0

DataAddr4D SegData::FindFree(sLONG len, void *obj, bool cangrow, DataAddr4D excludeAddr, sLONG excludeLen)
{
	sLONG nbits,i,j,k,oldi,jj,insidelong,i0,taillesup, realtaillesup;
	DataAddr4D alloue,oldfinfic,newfinfic,place,plus,start,end,substart,subend;
	sLONG newnbbitpage,nbbitpage,newnbpagesec;
	uBOOL trouve,okici,empty,dernierePageSec;
	FreeBitsptr fb;
	bittableptr fbt;
	sWORD maxbit;
	sLONG *p;
	uLONG *p2;
	VError err = VE_OK;
	FreebitsSecPtr fbsec = nil;
	sLONG nbbitpagesec;
	StAllocateInCache alloc;

	bool okmem = CheckForMemRequest(150000);

	fAccess.LockWrite();
#if debugLogEventFindFree
	gDebugCountFindFree++;
	gDebugEventFindFree.AddMessage(L"Avant FindFree", dbgFindFree(-1, -1, SDDx->finfic));
#endif
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	nbbitpagesec=CalcNbpageSec();
	alloue=-1;
	taillesup=0;
	realtaillesup = 0;
	bool someholes = false;

	if (!WriteProtected)
	{
		err = chargetfb();
		if (err == VE_OK)
		{
			nbits=(SDDx->sizeblockseg+len-1)>>SDDx->ratio;
			trouve=false;

			sLONG nbpass;
			if ( (fLastPage == 0 && fLastPageSec == 0) || fOnePassOnly )
				nbpass = 1;
			else
				nbpass = 2;

			for (sLONG curpass = 1; curpass <= nbpass; curpass++)
			{
				for (i0 = fLastPageSec; (i0 < nbbitpagesec) && (!trouve) && (err == VE_OK); i0++)
				{
					okici=false;
					maxbit=(*fbmaxlibreprim)[i0];
					if (nbits>NBbitparpagesec-6)
					{
						if ((maxbit>0) || (maxbit==-1)) okici=true;
					}
					else
					{
						if (maxbit==-1)
						{
							okici=true;
						}
						else
						{
							if (nbits<=maxbit) 
								okici=true;
							else
							{
								if (nbits != 0)
									someholes = true;
							}
						}
					}

					if (okici)
					{
						bool reachedEndOfPageSec = false;
						dernierePageSec = (i0 == (nbbitpagesec-1));
						fbsec = chargefbsec(i0, err, curstack);
						if (fbsec != nil)
						{
							nbbitpage = fbsec->calcNBpages();

							sLONG starti = 0;
							if (i0 == fLastPageSec) starti = fLastPage;

							for (i=starti; (i<nbbitpage) && (!trouve) && (err == VE_OK); i++)
							{
								okici=false;

								maxbit=fbsec->GetMaxLibre(i);
								if (nbits>NBbitparpage-6)
								{
									if ((maxbit>0) || (maxbit==-1)) okici=true;
								}
								else
								{
									if (maxbit==-1)
									{
										okici=true;
									}
									else
									{
										if (nbits<=maxbit) 
											okici=true;
										else
										{
											if (nbits != 0)
												someholes = true;
										}
									}
								}

								if (okici)
								{
									sLONG iSaved = i;
									fbt = fbsec->chargefb(i, err, curstack);

									if (fbt!=nil)
									{
										fb=fbt->getfbb();

										if (i0 == fLastPageSec && i == fLastPage)
										{
											j = fLastBloc;
										}
										else
										{
											p=(sLONG*)fb;
											j=-2;
											for (jj=0;(jj<NBlongbit) && (j==-2);jj++)
											{
												if (*p++!=-1)
												{
													someholes = true;
													j=(jj-1)<<5;
													if (j<0) 
														j=-1;
													break;
												}
											}
											if ((jj>=NBlongbit) && (j==-2)) 
												j=NBbitparpage-1;
										}

										insidelong=j & 31;
										if (j==-1)
										{
											p2=(uLONG*)fb-1;
										}
										else
										{
											p2=((uLONG*)fb)+(j>>5);
										}
										while ((j<NBbitparpage-1) && (!trouve) && (err == VE_OK) && !reachedEndOfPageSec)
										{
											j++;
											insidelong++;
											if (insidelong>31)
											{
												// on peut encore optimiser ici en recherchant a nouveau les long qui sont plein ( a -1)
												insidelong=0;
												p2++;
											}
											if ( ((*p2) & (1<<insidelong)) == 0 )  // block libre
											{
												k=0;
												empty=true;
												oldi=i;
												while (empty && (k<nbits-1))
												{
													insidelong++;
													if (insidelong>31)
													{
														// on peut encore optimiser ici en recherchant a nouveau les long qui sont vide ( a 0)
														insidelong=0;
														p2++;
													}
													k++;
													if ( ((k+j) & (NBbitparpage-1)) == 0)
													{
														if (i>=(nbbitpage-1))		// on atteint la fin de la page sec
														{
															empty=false;
															if (dernierePageSec)
															{
																trouve=true;
																alloue=(taillegroupesec * (DataAddr4D)i0)+(DataAddr4D)((DataAddr4D)(j+(oldi<<ratiopagebit))<<SDDx->ratio);
																fLastPageSec = i0;
																fLastPage = oldi;
																fLastBloc = j;			
															}
															else
																reachedEndOfPageSec = true;
														}
														else	// on passe a la page de bit suivante
														{
															fbt->unlock();
															fbt->Free(curstack, true);
															i++;

															fbt=fbsec->chargefb(i, err, curstack);
															if (fbt!=nil)
															{
																// fbt->occupe();
																fb=fbt->getfbb();

																insidelong=0;
																p2=(uLONG*)fb;

																if ( ((*p2) & (1<<insidelong)) != 0 ) empty=false;
															}
															else
															{
																empty=false;
															}
														}
													}
													else
													{
														if ( ((*p2) & (1<<insidelong)) != 0 ) empty=false;
													}
												} // fin du while

												if (empty)
												{
													trouve=true;
													alloue=(taillegroupesec * (DataAddr4D)i0)+(DataAddr4D)((DataAddr4D)(j+(oldi<<ratiopagebit))<<SDDx->ratio);
													fLastPageSec = i0;
													fLastPage = oldi;
													fLastBloc = j;
												}
												else
												{
													j=(j+k) & (NBbitparpage-1);
												}
											}
										} 

										if (fbt != nil)
										{
											fbt->unlock();
											fbt->Free(curstack, true);
										}

										if (!trouve && maxbit==-1)
										{
											maxbit=maxbitfree(fb);
											fbsec->SetMaxLibre(i,maxbit);
											if (i > iSaved)
											{
												for (sLONG ii = iSaved; ii < i; ii++)
												{
													bittableptr fbtx = fbsec->chargefb(ii, err, curstack);
													if (fbtx!=nil)
													{
														FreeBitsptr fbx = fbtx->getfbb();
														maxbit=maxbitfree(fbx);
														fbsec->SetMaxLibre(ii,maxbit);
														fbtx->unlock();
														fbtx->Free(curstack, true);
													}
												}
											}
										}

									} // du if fbt!=nil
								} // du if okici
							} // end du for i

							fbsec->Free(curstack, true);
							if (trouve && (alloue == -1)) trouve = false;
						}
					} // du if okici
				} // end du for i0

				if (fbsec != nil)
					i0 = fbsec->GetNum();

#if 0
				sLONG reallen = (len+ SDDx->sizeblockseg-1) & (-SDDx->sizeblockseg);
				excludeLen = (excludeLen+ SDDx->sizeblockseg-1) & (-SDDx->sizeblockseg);
				DataAddr4D excludeAddr2 = excludeAddr + excludeLen - 1;

				if ( (alloue<=excludeAddr && excludeAddr2 < alloue+reallen) || (alloue<=excludeAddr2 && excludeAddr < alloue+reallen) )
				{
					alloue = -1;
				}
#else

				if ((!cangrow) && alloue >= excludeAddr)
					alloue = -1;
#endif

				if (alloue == -1)
				{
					fLastPageSec = 0;
					fLastPage = 0;
					fLastBloc = 0;
				}
				else
				{
					if (curpass == nbpass)
						break;
					else
					{
						if (alloue+len>SDDx->finfic)
						{
							trouve = false;
							alloue = -1;
							fLastPageSec = 0;
							fLastPage = 0;
							fLastBloc = 0;
						}
						else
							break;
					}
				}

				if (alloue == -1)
					trouve = false;

			}

			if ((!cangrow) && alloue+len>SDDx->finfic)
			{
				alloue = -1;
			}

			plus=100*1024;

			if (cangrow)
			{

				if (alloue==-1 && err == VE_OK)
				{
					DataAddr4D xx = SDDx->finfic+len;

					fLastPageSec = xx >> ratiogroupebitsec;
					i0 = fLastPageSec;
					xx = xx & (taillegroupesec - 1);
					fLastPage = xx >> ratiogroupebit;
					xx = xx & (taillegroupebit - 1);
					fLastBloc = xx >> kratio;

					fOnePassOnly = true;
					if (someholes)
						fOnePassOnly = false;

					if (SDDx->finfic+len<taillemaxsegdata)
					{
						alloue=SDDx->finfic;
					}
					else // on depasse la taille maxi d'un segment de donnes
					{
						alloue=-1;
					}
				} // fin de la premiere passe de test sur alloue quand on avait rien trouve dans les bittables
				else
					fOnePassOnly = false;

				//err=VE_OK;
				if (alloue!=-1 && err == VE_OK)
				{
					if (alloue+len>SDDx->finfic)
					{
						// on calcul ici le nombre de nouvelles bittable necessaires
						newnbbitpage = ((alloue+len+taillegroupebit-1) >> ratiogroupebit) - ((SDDx->finfic+taillegroupebit-1) >> ratiogroupebit);
						taillesup = newnbbitpage * ( sizepagebit + ktaillebloc ); // ktaillebloc pour la taille du header tag
						/*
						newnbbitpage=(alloue+len-1+taillegroupebit)>>ratiogroupebit;
						taillesup=(newnbbitpage-nbbitpage)*sizepagebit;
						*/

						// on calcul ici le nombre de nouvelle spagesec necessaires
						if ( ((alloue+len+taillesup+taillegroupesec-1)>>ratiogroupebitsec) > ((SDDx->finfic+taillegroupesec-1)>>ratiogroupebitsec) )
						{
							// on vient de change de page secondaire
							newnbpagesec = 1;
							taillesup = taillesup + sizetfb + sizefbmaxlibre; // on rajoute la place pour une page secondaire
							if (nbbitpagesec == 1) 
								taillesup = taillesup + sizetfbprim + sizefbmaxlibreprim; // on cree la table primaire des qu'il y a une deuxieme page secondaire

							// le deuxieme recalcul de taillesup est necessaire car la taille ajoute peut elle meme creer une noubelle bittable
							newnbbitpage = ((alloue+len+taillesup+taillegroupebit-1) >> ratiogroupebit) - ((SDDx->finfic+taillegroupebit-1) >> ratiogroupebit);
							taillesup = newnbbitpage * ( sizepagebit + ktaillebloc ); // ktaillebloc pour la taille du header tag
							taillesup = taillesup + sizetfb + sizefbmaxlibre; // on rajoute la place pour une page secondaire
							if (nbbitpagesec == 1) 
								taillesup = taillesup + sizetfbprim + sizefbmaxlibreprim; // on cree la table primaire des qu'il y a une deuxieme page secondaire	
						}
						else
						{
							// le deuxieme recalcul de taillesup est necessaire car la taille ajoute peut elle meme creer une noubelle bittable
							newnbbitpage = ((alloue+len+taillesup+taillegroupebit-1) >> ratiogroupebit) - ((SDDx->finfic+taillegroupebit-1) >> ratiogroupebit);
							taillesup = newnbbitpage * ( sizepagebit + ktaillebloc ); // ktaillebloc pour la taille du header tag
							newnbpagesec = 0;
						}

						if ( (SDDx->limitseg!=0) && (alloue+len+taillesup>SDDx->limitseg) ) // and (prefseg<>-1)

						{
							alloue=-1; // depasse limite imposee
							trouve=false;
						}
						else
						{
							sLONG8 nbfreebytes;

							err = fFile->GetVolumeFreeSpace( &nbfreebytes, NULL, true, 5000); // on interroge toutes les 5 sec
							if (err == VE_OK)
							{
								place = nbfreebytes;
							}
							else
							{
								place=taillemaxsegdata;
								err = 0;
							}

							if (place<(DataAddr4D)len+(DataAddr4D)plus+(DataAddr4D)taillesup || err != VE_OK)  // and (prefseg<>-1)
							{
								alloue=-1; // plus de place
								trouve=false;
							}
							else
							{
								DataAddr4D realoldfinfic = SDDx->finfic;
								oldfinfic=(SDDx->finfic+blockEOF-1) & (-blockEOF);

								if (taillesup>0) // il faut alloue des page de bits supplementaires
								{
									if (i0 >= tfbprim->GetCount())
										fbsec = nil;
									else
										fbsec = chargefbsec(i0, err, curstack);

									sLONG totlen = len+taillesup;
									sLONG newnbpage1,newnbpage2;
									FreebitsSecPtr newfbsec = nil;
									DataAddr4D offset = (alloue+len+SDDx->sizeblockseg-1) & (-SDDx->sizeblockseg);

									if (newnbpagesec>0) // si besoin de nouvelle pagesec
									{
										DataAddr4D subplace;

										if (nbbitpagesec == 1)  // il faut creer la table primaire
										{
											subplace = FindFree(sizetfbprim + sizefbmaxlibreprim, nil, false, alloue, totlen);
											if (subplace == -1)
											{
												subplace = offset;
												offset = offset + sizetfbprim + sizefbmaxlibreprim;
												realtaillesup = realtaillesup + sizetfbprim + sizefbmaxlibreprim;

											}
											tfbdiskprim = new DiskAddrArray();
											DataAddr4D xx = (fbsec == nil) ? db->GetBaseHeaderSize()+numseg - 1 : fbsec->getaddr();
											tfbdiskprim->Add(xx);
											SDDx->AddPagePrim = subplace + numseg - 1;
										}

										subplace = FindFree(sizetfbprim + sizefbmaxlibreprim, nil, false, alloue, totlen);
										if (subplace == -1)
										{
											subplace = offset;
											offset = offset + sizetfb + sizefbmaxlibre;
											realtaillesup = realtaillesup + sizetfb + sizefbmaxlibre;
										}

										newfbsec = new FreebitsSec(this, nbbitpagesec, subplace + numseg - 1, true);
										newfbsec->Occupy(curstack, true);

										maxbit=-1;
										if (fbmaxlibreprim->Add(maxbit) && tfbdiskprim->Add(subplace+numseg - 1) && tfbprim->Add(newfbsec))
										{
											// tout est bon
										}
										else
										{
											err=ThrowError(memfull, DBaction_AllocatingSpaceInDataSeg);
										}

										savetfb = true;
										savefbmaxlibre = true;
										ecrisfinfic = true;
										setmodif(true, db, nil);
										nbbitpage = (fbsec == nil) ? NBpagedebit : fbsec->calcNBpages();
										newnbpage1 = NBpagedebit;
										newnbpage2 = newnbbitpage - (NBpagedebit - nbbitpage);
										xbox_assert(newnbpage2 > 0);
									}
									else
									{
										newnbpage1 = nbbitpage + newnbbitpage;
										newnbpage2 = 0;
									}

									sLONG startpagesec = 0;
									if (fbsec == nil)
									{
										xbox_assert(newnbpagesec>0);
										startpagesec = 1;
									}

									for (j = startpagesec; j<=newnbpagesec && (err == VE_OK); j++)
									{
										DataAddr4D subplace;
										FreebitsSecPtr xfbsec = nil;
										sLONG x1,x2;

										if (j == 0)
										{
											x1 = nbbitpage;
											x2 = newnbpage1;
											xfbsec = fbsec;
											xbox_assert(fbsec != nil);
										}
										else
										{
											x1 = 0;
											x2 = newnbpage2;
											xfbsec = newfbsec;
										}
										for (i=x1;(i<x2) && (err==0);i++)
										{
											subplace = FindFree(sizepagebit + ktaillebloc, nil, false, alloue, totlen);
											if (subplace == -1)
											{
												subplace = offset;
												offset = offset + sizepagebit + ktaillebloc; // ktaillebloc pour la taille du header tag
												realtaillesup = realtaillesup + sizepagebit + ktaillebloc;
											}

											fbt=new bittable(db,NBlongbit);
											if ((fbt==nil) || (!fbt->okcree()))
											{
												err=ThrowError(memfull, DBaction_AllocatingSpaceInDataSeg);
												if (fbt!=nil) 
													fbt->Release();
											}
											else
											{
												_raz(fbt->getfbb(), sizepagebit);
												fbt->setaddr(subplace + numseg - 1);
												fbt->setmodif(true, db, nil);
											}
											//xfbsec->Occupy(curstack, true);
											xfbsec->setfb(i, fbt, subplace+numseg - 1);
											xfbsec->SetAtLeastCurrentNbPages(i);
											xfbsec->setmodif(true, db, nil);
											//xfbsec->Free(curstack, true);
										}
									}

									if (err == VE_OK)
									{
										SDDx->finfic=(alloue+len+SDDx->sizeblockseg-1+taillesup) & (-SDDx->sizeblockseg);
									}

									if (newfbsec != nil) 
									{
										newfbsec->SetCurrentNbPages(-1);
										newfbsec->Free(curstack, true);
									}

									if (fbsec != nil)
									{
										fbsec->Free(curstack, true);
										fbsec->SetCurrentNbPages(-1);
									}
								}

								if (err == VE_OK)
								{
									SDDx->finfic=(alloue+len+SDDx->sizeblockseg-1+taillesup) & (-SDDx->sizeblockseg);
									newfinfic=(SDDx->finfic+blockEOF-1) & (-blockEOF);
									if (oldfinfic<newfinfic)
									{
										err = fFileDesc->SetSize(newfinfic+512);
										if (err != VE_OK)
											SDDx->finfic = realoldfinfic;
									}
									if (err == VE_OK)
									{
										ecrisfinfic=true;
										setmodif(true, db, nil);
									}
								}

							}
						}
					}
				}
			}

			if ((alloue!=-1) && (err==VE_OK)) // maintenant on peut mettre a jour les bits
			{
				start=alloue>>ratiogroupebit;
				end=(alloue+len+realtaillesup-1)>>ratiogroupebit;
				for (i=start; (i<=end) && (err==VE_OK); i++)
				{
					fbt=chargefb(i, err, curstack);
					if (fbt != nil)
					{
						// fbt->occupe();
						fb=fbt->getfbb();

						if (i==start)
						{
							substart=alloue & (taillegroupebit-1);
						}
						else
						{
							substart=0;
						}

						if (i==end)
						{
							subend=(alloue+len+realtaillesup) & (taillegroupebit-1);
						}
						else
						{
							subend=taillegroupebit-1;
						}

						PrendsBits(fb,substart,subend);

						ResetMaxLibre(i, curstack);

						fbt->setmodif(true, db, nil);
						fbt->unlock();
						fbt->Free(curstack, true);
					}
					else
						err = ThrowError(memfull, DBaction_AllocatingSpaceInDataSeg);
				}

			} // fin de la mise a jour des bits

		} // fin du test d'erreur sur le chargetfb
	}
	else
	{
		err = ThrowError(VE_DB4D_FILEISWRITEPROTECTED, DBaction_AllocatingSpaceInDataSeg);
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOTALLOCATESPACEINDATASEG, DBaction_AllocatingSpaceInDataSeg);
		alloue = -1;
	}

#if debuglr == 126
	if (alloue != -1)
	{
		debug_AddrMap::iterator found = fDebugAddrMap.find(alloue);

		if (testAssert(found == fDebugAddrMap.end()))
		{
			fDebugAddrMap.insert(make_pair(alloue, debug_addrRange(obj, len)));
			found = fDebugAddrMap.find(alloue);
			xbox_assert(found != fDebugAddrMap.end());
			if (found != fDebugAddrMap.begin())
			{
				debug_AddrMap::iterator prev = found;
				prev--;
				if (prev->first + prev->second.fLen > alloue)
				{
					xbox_assert(false);
					alloue = alloue; // put break here
				}
			}
			debug_AddrMap::iterator next = found;
			next++;
			if (next != fDebugAddrMap.end())
			{
				if (alloue + len > next->first)
				{
					xbox_assert(false);
					alloue = alloue; // put break here
				}

			}
		}
		else
		{
			alloue = alloue; // put break here
		}
	}
#endif

#if debuglogFindPlace
	DebugMsg(L"Find Place : "+ToString(alloue/64)+L"  ,  len = "+ToString((len+63) / 64)+L"\n\n");
#endif

#if debugFindPlace_inMap
	if (alloue == 84352)
	{
		alloue = alloue;
	}
	if (alloue != -1)
	{
		fDebugAddrs.insert(make_pair(alloue, (len+ktaillebloc-1) & (-ktaillebloc)));
	}
#endif

	CheckForMemRequest();
#if debugLogEventFindFree
	gDebugCountFindFree--;
	gDebugEventFindFree.AddMessage(L"Apres FindFree", dbgFindFree(-1, -1, SDDx->finfic));
#endif
	fAccess.Unlock();

	if (alloue != -1)
	{
		VTaskLock lock(&fFullyDeletedMutex);
		DataAddr4D allouefin = alloue + (DataAddr4D)len;
		vector<DataAddr4D> toRemove;
		Boolean lowerbound = true;
		DataAddrSet::iterator cur = fFullyDeletedAddr.lower_bound(alloue), end = fFullyDeletedAddr.end();
		while (cur != end && lowerbound)
		{
			if ( (*cur) >= allouefin )
				lowerbound = false;
			else
			{
				if ( (*cur) >= alloue )
				{
					toRemove.push_back(*cur);
				}
			}
			cur++;
		}

		for (vector<DataAddr4D>::iterator curx = toRemove.begin(), endx = toRemove.end(); curx != endx; curx++)
		{
			fFullyDeletedAddr.erase(*curx);
		}

	}


#if debugFindPlace_strong
	if (alloue != -1)
	{
		VTaskLock lock(&debug_FindFreeMutex);
		debug_FindFreeInfo* xFind = &(debug_FindFrees[alloue]);
		if (xFind->obj != nil)
		{
			xbox_assert(xFind->obj == nil);
			VStr<2048> ss;
			xFind->fCrawl.Dump(ss);
			DebugMsg(ss);
			DebugMsg(L"\n\n\n\n");
		}
		xFind->obj = obj;
		xFind->fCrawl.LoadFrames(0, kMaxScrawlFrames);
		
	}
#endif

	if (!AllowedAddress(alloue))
	{
		ThrowBaseError(VE_DB4D_DATABASEISWRITEPROTECTED, noaction); // break here
		alloue = -1;
	}
	return(alloue);
}


#if debugFindPlace_strong

void SegData::Debug_CheckAddrOverWrite(IObjToFlush* obj, DataAddr4D ou)
{
	VTaskLock lock(&debug_FindFreeMutex);
	debug_MapOfFindFree::iterator found = debug_FindFrees.find(ou);
	if (found != debug_FindFrees.end())
	{
		IObjToFlush* xobj = nil;
		debug_FindFreeInfo* xFind = &(found->second);
		try
		{
			xobj = dynamic_cast<IObjToFlush*>((VObject*)xFind->obj);
		}
		catch (...)
		{
			xobj = nil;
		}
		if (xobj != nil && xobj != obj)
		{
			xbox_assert(false);
			VStr<2048> ss;
			xFind->fCrawl.Dump(ss);
			DebugMsg(ss);
			DebugMsg(L"\n\n\n\n");
		}
	}
}

#endif


void SegData::ResetMaxLibre(sLONG i, OccupableStack* curstack)
{
	sLONG n,n2,oldmax;
	FreebitsSecPtr fbsec;
	VError err;

	n = i >> kratiosec;
	n2 = i & (NBpagedebit - 1);

	fbsec = chargefbsec(n, err, curstack);

	if (fbsec != nil)
	{
		oldmax = fbsec->GetMaxLibre(n2);
		if (n2>=(*fbmaxlibreprim)[n]) 
		{
			(*fbmaxlibreprim)[n] = -1;
			savefbmaxlibre = true;
			setmodif(true, db, nil);
		}
		fbsec->SetMaxLibre(n2, -1);
		fbsec->setmodif(true, db, nil);
	}

	fbsec->Free(curstack, true);
}


VError SegData::LibereBits(DataAddr4D ou, sLONG len, void *obj)
{
	sLONG i;
	DataAddr4D start,end,substart,subend;
	FreeBitsptr fb;
	bittableptr fbt;
	VError err;
	sWORD maxbit;


	if (!AllowedAddress(ou))
	{
		return ThrowBaseError(VE_DB4D_DATABASEISWRITEPROTECTED, noaction); // break here
	}

	fAccess.LockWrite();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	// IncNbAcces();

#if debugFindPlace_inMap
	sLONG len2 = (len+ktaillebloc-1) & (-ktaillebloc);
	MapOfAddrs::iterator found = fDebugAddrs.find(ou);
	if (found != fDebugAddrs.end())
	{
		if (len2 != found->second)
		{
			xbox_assert(false);
		}
		fDebugAddrs.erase(found);
	}
#endif

#if debuglogFindPlace
	DebugMsg(L"libere Place : "+ToString(ou/64)+L"  ,  len = "+ToString((len+63) / 64)+L"\n\n");
#endif

#if debugFindPlace_strong
	if (ou != -1)
	{
		VTaskLock lock(&debug_FindFreeMutex);
		debug_FindFrees.erase(ou);
	}
#endif

#if debuglr == 126

	debug_AddrMap::iterator found = fDebugAddrMap.find(ou);
	if (testAssert(found != fDebugAddrMap.end()))
	{
		if (len != found->second.fLen)
		{
			ou = ou; // put a break here
		}
		fDebugAddrMap.erase(found);
	}
	else
	{
		ou = ou; // put a break here
	}

#endif

	err = chargetfb();

	if (err==VE_OK)
	{

		DataAddr4D xx = ou;

		fOnePassOnly = false;
		fLastPage = 0;
		fLastBloc = 0;
		fLastPageSec = 0;
		fCountLibereBit++;

#if 0
		sLONG LastPageSec = xx >> ratiogroupebitsec;
		xx = xx & (taillegroupesec - 1);
		sLONG LastPage = xx >> ratiogroupebit;
		xx = xx & (taillegroupebit - 1);
		sLONG LastBloc = xx >> kratio;


		if (fCountLibereBit > 2048)
		{
			fCountLibereBit = 0;
			fLastPage = 0;
			fLastBloc = 0;
		}
		else
		{
			Boolean isinf = false;

			if (LastPageSec<fLastPageSec)
			{
				isinf = true;
			}
			else
			{
				if (LastPageSec == fLastPageSec)
				{
					if (LastPage<fLastPage)
					{
						isinf = true;
					}
					else
					{
						if (LastPage == fLastPage)
						{
							if (LastBloc<=fLastBloc)
								isinf = true;
						}
					}
				}
			}

			if ( isinf )
			{
				fLastPageSec = LastPageSec;
				fLastPage = LastPage;
				fLastBloc = LastBloc-1;
				if (fLastBloc == -1)
				{
					fLastPage--;
					fLastBloc = 0;
					if (fLastPage == -1)
					{
						fLastPageSec--;
						fLastPage = 0;
						if (fLastPageSec == -1)
							fLastPageSec = 0;
					}
				}
				else
				{
					fLastBloc = 0;
					if ((fCountLibereBit & 128) == 0)
					{
						if (fLastPage > 0)
							fLastPage--;
					}
				}
			}
		}
#endif

		start=ou>>ratiogroupebit;
		end=(ou+len-1)>>ratiogroupebit;
		for (i=start; (i<=end) && (err==VE_OK); i++)
		{
			fbt=chargefb(i, err, curstack);
			if (fbt!=nil)
			{
				fb=fbt->getfbb();

				if (i==start)
				{
					substart=ou & (taillegroupebit-1);
				}
				else
				{
					substart=0;
				}

				if (i==end)
				{
					subend=(ou+len) & (taillegroupebit-1);
				}
				else
				{
					subend=taillegroupebit-1;
				}

				ClearBits(fb,substart,subend);

				ResetMaxLibre(i, curstack);
				fbt->setmodif(true, db, nil);
				fbt->unlock();
				fbt->Free(curstack, true);
			}
		}
	}

	CheckForMemRequest();
	fAccess.Unlock();

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTFREESPACEINDATASEG, DBaction_FreeingSpaceInDataSeg);
	return err;
}

sLONG SegData::maxbitfree(FreeBitsptr fb)
{
	sLONG insidelong,i,max,tot;
	uLONG *p;

	p=(uLONG*)fb;
	max=0;
	tot=0;

	i=0;
	while (i<NBbitparpage)
	{
		if ((*((sLONG*)p))==-1) // tous les 32 blocks occupes
		{
			tot=0;
			p++;
			i=i+32;
		}
		else
		{
			if (*p==0)	// tous 32 les blocks libres
			{
				tot=tot+32;
				if (tot>max) max=tot;
				i=i+32;
				p++;
			}
			else
			{
				insidelong=0;
				do
				{
					if ( (*p & (1<<insidelong)) != 0 )  // block occupe
					{
						tot=0;
					}
					else
					{
						tot=tot+1;
						if (tot>max) max=tot;
					}
					insidelong++;
					i++;
					if (insidelong>31)
					{
						insidelong=0;
						p++;
					};
				} while (insidelong!=0);
			}
		}
	}

	return(max);
}


bool SegData::SaveObj(VSize& outSizeSaved)
{
	sLONG nb,offset;
	VSize tot;
	DataAddr4D ou;
	uBOOL oneinuse;
	VError err = VE_OK, err2 = VE_OK, err3 = VE_OK;

	tot=0;

	if ((fFileDesc != nil) && (fFileDesc->GetMode() >= FA_READ)) {
		if ((tfbdiskprim!=nil) && (SDDx->AddPagePrim != 0))	// save table primaire des table secondaire des bittable
		{
			ou = StripDataAddr(SDDx->AddPagePrim);
			nb = CalcNbpageSec();
			// if (savetfb) // on sauve toujours c'est plus sure
			{
#if BIGENDIAN
				ByteSwap(tfbdiskprim->First(), nb);
#endif
				err = writeat( tfbdiskprim->First(), nb<<3, ou);
#if BIGENDIAN
				ByteSwap(tfbdiskprim->First(), nb);
#endif
				savetfb=false;
				tot = tot + (nb<<3);
			}

			// if (savefbmaxlibre) // on sauve toujours c'est plus sure
			{
#if BIGENDIAN
				ByteSwap(fbmaxlibreprim->First(), nb);
#endif
				err2 = writeat(fbmaxlibreprim->First(), nb<<1, ou+sizetfbprim);
#if BIGENDIAN
				ByteSwap(fbmaxlibreprim->First(), nb);
#endif
				savefbmaxlibre = false;
				tot = tot + (nb<<1);
			}

		}

		if (ecrisfinfic)
		{
			uLONG entete = tagHeader4D;
			err3 = writeat(&entete, sizeof(entete), 0);

			offset=db->getSDDoffset();
			err3 = writeat(SDDx, sizeof(SegDataDisk2), offset);
			//err3 = db->writelong(&SDD,sizeof(SDD),db->GetMultiSegHeaderAddress(),offset + kSizeDataBaseObjectHeader);
			ecrisfinfic=false;
			tot = tot + sizeof(SegDataDisk2);
		}
	}

	if (err != VE_OK || err2 != VE_OK || err3 != VE_OK)
		ThrowError(VE_DB4D_CANNOTSAVEDATASEG, DBaction_SavingDataSeg);

	outSizeSaved = tot;
	return true;
}


VError SegData::ThrowError( VError inErrCode, ActionDB4D inAction)
{
	VUUID nullid;

	VErrorDB4D_OnBase *err = new VErrorDB4D_OnBase(inErrCode, inAction, db);
	VTask::GetCurrent()->PushRetainedError( err);

	return inErrCode;
}


VError SegData::readat(void* p, sLONG len, DataAddr4D ou)
{
	VError err;
	{
		VTaskLock lock(&fMutex);
		if ((fWriteBufferOffset > 0) && (fWriteBufferStart <= ou) && (ou <= fWriteBufferStart+fWriteBufferOffset))
			FlushBuffer();
		err = fFileDesc->GetData( p, len, ou);
	}

	if (err != VE_OK)
		ThrowError(err, DBaction_ReadingData);

	xbox_assert(err == VE_OK);
	return(err);
}

uBOOL SegData::IsAddrValid(DataAddr4D ou)
{
	return( ou >= 0 && ou <= Getfinfic() );
}

VError SegData::writeat(void* p, sLONG len, DataAddr4D ou, sLONG TrueLen)
{
	VError err = VE_OK;
	if (TrueLen == -1 || fWriteBuffer == (void*)-1)
	{
		if (fWriteBuffer != (void*)-1)
		{
			FlushBuffer();
		}
		err = fFileDesc->PutData( p, len, ou);
	}
	else
	{
		if (TrueLen == 0)
			TrueLen = len;
		VTaskLock lock(&fMutex);
		if (fWriteBuffer == nil)
		{
			fWriteBufferLen = 65536;
			fWriteBuffer = VObject::GetMainMemMgr()->Malloc(fWriteBufferLen + 4, false, 'wBuf');
			if (fWriteBuffer == nil)
			{
				fWriteBuffer = (void*)-1;
				fWriteBufferLen = 0;
			}
			else
			{
				fWriteBufferStart = ou;
				fWriteBufferOffset = 0;
			}
		}

		if (fWriteBuffer == nil)
		{
			err = fFileDesc->PutData( p, len, ou);
		}
		else
		{
			if (fWriteBufferStart == -1)
				fWriteBufferStart = ou;
			else
			{
				if ( ((ou+TrueLen) > (fWriteBufferStart+fWriteBufferLen)) || (ou < fWriteBufferStart) )
				{
					FlushBuffer();
					fWriteBufferStart = ou;
				}
			}

			DataAddr4D xnewoffset = ou - fWriteBufferStart;
			sLONG newoffset = (sLONG)xnewoffset;

			if ((newoffset - fWriteBufferOffset) >= ktaillebloc)
			{
				FlushBuffer();
				fWriteBufferStart = ou;
				newoffset = 0;
			}

			if (TrueLen > fWriteBufferLen)
			{
				fWriteBufferStart = -1;
				err = fFileDesc->PutData( p, len, ou);
			}
			else
			{
				char* pbuf = (char*)fWriteBuffer;
				if (newoffset > fWriteBufferOffset)
				{
					std::fill(&pbuf[fWriteBufferOffset], &pbuf[newoffset], 0);
				}
				vBlockMove(p, pbuf+newoffset, len);
				std::fill(&pbuf[newoffset+len], &pbuf[newoffset+TrueLen], 0);
				if ( (newoffset + TrueLen) > fWriteBufferOffset)
					fWriteBufferOffset = newoffset + TrueLen;
			}

		}

	}

	if (err != VE_OK)
		ThrowError(err, DBaction_WritingData);

	xbox_assert(err == VE_OK);
	return err;
}


VError SegData::FlushBuffer()
{
	VError err;
	VTaskLock lock(&fMutex);
	if (fWriteBuffer != nil &&  fWriteBuffer != (void*)-1 && fWriteBufferOffset > 0)
	{
		err = fFileDesc->PutData( fWriteBuffer, fWriteBufferOffset, fWriteBufferStart);
		fWriteBufferOffset = 0;
	}
	else
		err = VE_OK;
	return err;
}


void SegData::GetFullPath( VFilePath& outPath) const
{
	if (fFile != nil)
		outPath = fFile->GetPath();
	else {
		VFilePath empty;
		outPath = empty;
	}
}


void SegData::MarkBlockToFullyDeleted(DataAddr4D ou, sLONG len)
{
	VTaskLock lock(&fFullyDeletedMutex);
	fFullyDeletedAddr.insert(ou);
}




SegData::SegData( Base4D *inBase, sLONG inNumSeg, VFile *inFile, SegDataDisk2 *realSDD)
{
	xbox_assert( inFile != nil);

	db = inBase;

	if (realSDD == nil)
		SDDx = &SDDInside;
	else
		SDDx = realSDD;

	SDDx->finfic = 0;
	SDDx->limitseg = 0;
	SDDx->sizeblockseg = ktaillebloc;
	SDDx->ratio=kratio;
	SDDx->AddPagePrim = 0; // si 0 alors un seul niveau d'acces aux bittable 

	isopen = false;
	savetfb = false;
	savefbmaxlibre = false;
	ecrisfinfic = false;
	fFile = inFile;
	fFileDesc = nil;
	tfbprim = nil;
	tfbdiskprim = nil;
	fbmaxlibreprim = nil;
	WriteProtected = false;
	numseg = inNumSeg;
	fLastPageSec = 0;
	fLastPage = 0;
	fLastBloc = 0;
	fWriteBuffer = nil;
	fWriteBufferLen = 0;
	fWriteBufferOffset = 0;
	fWriteBufferStart = -1;
	SetInProtectedArea();
}



/* ----------------------------- */

SegData_NotOpened::SegData_NotOpened(Base4D_NotOpened *xdb)
{
	db = xdb;
	fFile = nil;
	fFileDesc = nil;
	fHeaderIsValid = false;
	finficIsValid = false;
	fIsAttached = true;
}


SegData_NotOpened::~SegData_NotOpened()
{
	Close();
	if (fFile != nil)
		fFile->Release();
}


void SegData_NotOpened::Init(VFile* inFile, sLONG xsegnum)
{
	fFile = inFile;
	if (fFile != nil)
		fFile->Retain();
	fSegNum = xsegnum;
	fIsAttached = false;
}


void SegData_NotOpened::Init(VFileDesc* inFileDesc, sLONG xsegnum)
{
	fIsAttached = true;
	fFile = nil;
	fSegNum = xsegnum;
	fFileDesc = inFileDesc;
}


VError SegData_NotOpened::Open(sLONG numseg, ToolLog *log, FileAccess access)
{
	VError errexec = VE_OK;
	fSegNum = numseg;
	VError err = VE_OK;

	if (!fIsAttached)
	{
		if (fFile->Exists())
			err = fFile->Open(access, &fFileDesc);
		else
			err = VE_FILE_NOT_FOUND;
	}

	if (err == VE_OK)
	{
		BaseHeader bh;
		err = readat(&bh, sizeof(bh), 0);
		if (err == VE_OK)
		{
			if (bh.tag != tagHeader4D)
			{
				if (bh.tag == SwapLong(tagHeader4D))
				{
					bh.SwapBytes();
				}
			}
			if (bh.tag != tagHeader4D) 
			{
				DataSegProblem pb(TOP_WrongHeader, fSegNum);
				errexec = log->Add(pb);
			}
			else
			{
				fID.FromBuffer(bh.GetID());
				SDDInside = bh.seginfo;
				if ((1<<SDDInside.ratio) != SDDInside.sizeblockseg || SDDInside.sizeblockseg != ktaillebloc)
				{
					DataSegProblem pb(TOP_WrongHeader, fSegNum);
					errexec = log->Add(pb);
				}
				else
				{
					finfic = fFileDesc->GetSize();
					if (finfic >= SDDInside.finfic /* && ((SDDInside.finfic+blockEOF-1) & (-blockEOF)) + 512 == finfic*/)
					{
						finficIsValid = true;
						finfic = SDDInside.finfic;
					}
					else
					{
						DataSegProblem pb(TOP_EndOfFileNotMatching, fSegNum);
						errexec = log->Add(pb);
					}

					fHeaderIsValid = true;
				}
			}

		}
		else
		{
			DataSegProblem pb(TOP_WrongHeader, fSegNum);
			errexec = log->Add(pb);
		}
	}
	else
	{
		errexec = err;
	}

	return errexec;
}


void SegData_NotOpened::Close()
{
	if (!fIsAttached)
	{
		if (fFileDesc != nil)
			delete fFileDesc;
	}
	fFileDesc = nil;
	fIsAttached = false;
}


VError SegData_NotOpened::readat(void* p, sLONG len, DataAddr4D ou)
{
	//ou = StripDataAddr(ou);
	VError err = fFileDesc->GetData( p, len, ou);

	return(err);
}

uBOOL SegData_NotOpened::IsAddrValid(DataAddr4D ou, Boolean TestAppartenance)
{
	if (TestAppartenance)
	{
		sLONG xnumseg = ou & (kMaxSegData-1);
		if (xnumseg != (fSegNum - 1))
			return false;
	}
	return( StripDataAddr(ou) > 0 && StripDataAddr(ou) < finfic );
}

VError SegData_NotOpened::writeat(void* p, sLONG len, DataAddr4D ou)
{
	ou = StripDataAddr(ou);
	VError err = fFileDesc->PutData( p, len, ou);

	return err;
}


VError SegData_NotOpened::CheckBittables(ToolLog* log)
{
	VString unformatted_str;
	VString formatted_str;
	log->GetVerifyOrCompactString(18,unformatted_str);	// Check Bittables
	XBOX::VValueBag *bag = new XBOX::VValueBag();
	if (bag != NULL)
	{
		bag->SetLong("SegNum", fSegNum);
	}
	formatted_str = unformatted_str.Format(bag);

	log->OpenProgressSession(formatted_str, (finfic+(sLONG8)(ktaillebloc*NBbitparpage)-1) / (sLONG8)(ktaillebloc*NBbitparpage));
	VError errexec = VE_OK;
	sLONG nbprim = CalcNbpageSec();

	log->MarkAddr_SegHeader(fSegNum-1, db->GetBaseHeaderSize(), fSegNum);

	if (SDDInside.AddPagePrim == 0)
	{
		if (nbprim > 1)
		{
			DataSegProblem pb(TOP_AddrOfPrimaryTableOfBittableIsInvalid, fSegNum);
			errexec = log->Add(pb);
		}
		else
			errexec = CheckBitTableSec(log, db->GetBaseHeaderSize()+fSegNum-1, 0);
	}
	else
	{
		if (IsAddrValid(SDDInside.AddPagePrim, true))
		{
			log->MarkAddr_PrimBittab(SDDInside.AddPagePrim, nbprim*sizeof(DataAddr4D), fSegNum);
			DataAddr4D* primaddr = (DataAddr4D*)malloc(nbprim*sizeof(DataAddr4D));
			if (primaddr == nil)
				errexec = memfull;
			else
			{
				VError err = readat(primaddr, nbprim*sizeof(DataAddr4D), StripDataAddr(SDDInside.AddPagePrim));
				if (err == VE_OK)
				{
					sLONG i;
					DataAddr4D* cur;
					for (i=0, cur = primaddr; i<nbprim && errexec == VE_OK; i++, cur++)
					{
						errexec = CheckBitTableSec(log, *cur, i);
					}
				}
				else
				{
					DataSegProblem pb(TOP_PrimaryTableOfBittableIsInvalid, fSegNum);
					errexec = log->Add(pb);
				}
				free(primaddr);
			}
		}
		else
		{
			DataSegProblem pb(TOP_AddrOfPrimaryTableOfBittableIsInvalid, fSegNum);
			errexec = log->Add(pb);
		}
	}

	log->CloseProgressSession();
	return errexec;
}



VError SegData_NotOpened::CheckBitTableSec(ToolLog* log, DataAddr4D addrfbsec, sLONG No)
{
	VError errexec = VE_OK;

	if (IsAddrValid(addrfbsec, true))
	{
		log->MarkAddr_SecondaryBittab(addrfbsec, sizetfb + sizefbmaxlibre, fSegNum, No);
		sLONG nbp;
		sLONG nb = (sLONG) ((finfic+taillegroupesec-1) >> ratiogroupebitsec);
		if (No==(nb-1)) nbp = (sLONG) ( ( (finfic & (taillegroupesec-1)) + taillegroupebit-1 ) >> ratiogroupebit);
		else nbp = NBpagedebit;

		FreeBitsPageOnDisk tfb;
		DataAddr4D ou = StripDataAddr(addrfbsec);

		_raz(&tfb, sizeof(tfb));

		VError err = readat( &tfb, nbp<<3, ou);
		if (err != VE_OK)
		{
			BittableSecTableProblem pb(TOP_SecondaryTableOfBittableIsInvalid, fSegNum, No);
			errexec = log->Add(pb);
		}
		else
		{
#if BIGENDIAN
			ByteSwap(&tfb[0], nbp);
#endif

			sLONG i;
			DataAddr4D* cur;
			for (i=0, cur = &tfb[0]; i<nbp && errexec == VE_OK; i++, cur++)
			{
				errexec = CheckBitTable(log, *cur, No, i);
			}
		}

	}
	else
	{
		BittableSecTableProblem pb(TOP_AddrOfSecondaryTableOfBittableIsInvalid, fSegNum, No);
		errexec = log->Add(pb);
	}

	return errexec;
}


VError SegData_NotOpened::CheckBitTable(ToolLog* log, DataAddr4D addrfb, sLONG numsec, sLONG No)
{
	VError errexec = VE_OK;

	errexec	= log->Progress((sLONG8)numsec*(sLONG8)NBpagedebit+(sLONG8)No);

	if (errexec == VE_OK)
	{
		if (IsAddrValid(addrfb, true))
		{
			log->MarkAddr_Bittab(addrfb, sizepagebit + kSizeDataBaseObjectHeader, fSegNum, numsec, No);

			DataBaseObjectHeader tag;
			DataAddr4D ou = StripDataAddr(addrfb);
			FreeBits fbb;

			VError err = tag.ReadFrom(this, ou);
			if (err == VE_OK)
				err = tag.ValidateTag(DBOH_Bittab, -1, -1);
			if (err == VE_OK)
				err = readat( &fbb, sizepagebit,ou + kSizeDataBaseObjectHeader);
			if (err == VE_OK)
				err = tag.ValidateCheckSum(&fbb, sizepagebit);

			if (err != VE_OK)
			{
				BittableProblem pb(TOP_BittableIsInvalid, fSegNum, numsec, No);
				errexec = log->Add(pb);
			}

		}
		else
		{
			BittableProblem pb(TOP_AddrOfBittableIsInvalid, fSegNum, numsec, No);
			errexec = log->Add(pb);
		}
	}

	return errexec;
}


/* ----------------------------- */



