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

#if journal

Boolean fJournal = 0;
Boolean _okjournal = 0;
VFileStream *journal4D = nil;
VFile *journal4DFile = nil;
sLONG _JNUM = 0;
sLONG _JLINENUM = 0;
sLONG _JMAXLINE = 800000;
VCriticalSection _JMUTEX;


void __InitLog()
{
	fJournal = true;
	_okjournal = true;
}

Boolean OpenJournal4D(void)
{
	VFile *vf;
	VName vs,vs2;
	VError err;
	Boolean ok;
	
	if (_okjournal && fJournal)
	{
		if (journal4D != nil)
		{
			if (_JLINENUM>_JMAXLINE)
			{
				CloseJournal4D();
			}
			else
			{
				_JLINENUM++;
			}
		}
		if (journal4D == nil)
		{
			_okjournal = false;
			_JNUM++;
			_JLINENUM = 0;

			/*
			if (_JNUM>20)
			{
				vs.FromCString("JDebug");
				vs2.FromLong(_JNUM-20);
				vs += vs2;
				vs2.FromCString(".txt");
				vs += vs2;
				VFilePath oldpath;
				oldpath.FromFullPath(vs);
				VFile oldvf(oldpath);
				err = vf->Delete();
			}
			*/

			vs.FromCString("c:\\JDebug");
			vs2.FromLong(_JNUM);
			vs += vs2;
			vs2.FromCString(".txt");
			vs += vs2;
			VFilePath path;
			path.FromFullPath(vs);
			vf = new VFile(path);
			err = vf->Delete();
			
			err = vf->Create(true);
			if (err == VE_OK)
			{
				journal4D = new VFileStream(vf);
			//	journal4D->AllocateBuffer(64000);
				journal4D->OpenWriting();
				journal4DFile = vf;
			}
			_okjournal = true;
		}
		
		ok = journal4D != nil;
	}
	else ok = false;
	
	return(ok);
}

void CloseJournal4D(void)
{
	if (journal4D != nil)
	{
		journal4D->CloseWriting();
		Boolean oldj = _okjournal;
		_okjournal = false;
		delete journal4D;
		journal4D = nil;
		journal4DFile->Close();
		delete journal4DFile;
		journal4DFile = nil;
		_okjournal = oldj;
	}
}


void __WriteLogLong(const sLONG x)
{
	VStr63 vs;
	
	vs.FromLong(x);
	__WriteLogVStr(vs);
}


void __WriteLogHexa(const sLONG x)
{
	VStr63 vs;
	
	vs.FromLong(x);
	__WriteLogVStr(vs);
}


void __WriteLogStr(const uBYTE *cStr)
{
	VString vs;
	VTaskLock lock(&_JMUTEX);
	
	if (OpenJournal4D())
	{
		vs.FromCString((sBYTE*)cStr); 
		journal4D->PutText(vs);
	}
}

void __WriteLogVStr(const VString& vs)
{
	VTaskLock lock(&_JMUTEX);
	if (OpenJournal4D())
	{
		journal4D->PutText(vs);
	}
}

void __WriteLnLogStr(const uBYTE *cStr)
{
	VStr<80> vs;
	VTaskLock lock(&_JMUTEX);

	if (OpenJournal4D())
	{
		if (cStr == nil)
			vs.Clear();
		else
			vs.FromCString((sBYTE*)cStr); 
		vs.AppendChar(13);
#if WINVER
		vs.AppendChar(10);
#endif
		journal4D->PutText(vs);
	}
}

void __WriteLnLogVStr(const VString& vs)
{
	VStr<10> cr;
	VTaskLock lock(&_JMUTEX);
	cr.AppendChar(13);
#if WINVER
	cr.AppendChar(10);
#endif
	if (OpenJournal4D())
	{
		journal4D->PutText(vs);
		journal4D->PutText(cr);
	}
}


void __WriteLogAction(const VString& vs, sLONG val)
{
	VStr<120> s;
	s = vs;
	VStr<50> s2;
	s2.FromLong(val);
	s.AppendString(L" : ");
	s.AppendString(s2);
	VTask* curtask = VTask::GetCurrentTask();
	if (curtask != nil)
	{
		s.AppendString(L"  , processid : ");
		s2.FromLong(curtask->GetID());
		s.AppendString(s2);
	}
	__WriteLnLogVStr(s);
}



#endif
