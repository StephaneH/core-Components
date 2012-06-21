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

USING_TOOLBOX_NAMESPACE

/* 2010-01-11 - formats of the log

	******************************* IMPORTANT ********************************
	*																		 *
	*	The text format is "compact" and is prioritary (only used by us).	 *
	*	It is imported by a 4D database, whose code expects this format		 *
	*	to be strictly used.												 *
	*																		 *
	*	=>	If you change the text format, change the code of the db !		 *
	*	=>	A version number of the format is used, so the 4D code will²	 *
	*		be able to import differen logs of different versions			 *
	*	=>	So, if you change the text format, think about incrementing		 *
	*		the version number												 *
	*																		 *
	***************************************************************************

	Text (tab-tab-return)
		Look at/use/dump in log _ExplainTTRFormat()

	XML
		(not implemented)
*/
static const sLONG		kTTR_FORMAT_VERSION = 1;

VCacheLog *				VCacheLog::s_cacheLog = NULL;
VCriticalSection		VCacheLog::s_mutex;
bool					VCacheLog::s_isStarted = false;

//	==================================================
//	VCacheLog / TTR format
//	==================================================
// This routine is supposed to be run in VERSIONDEBUG, with debug to check the writting of the file
#if VERSIONDEBUG
static void _ExplainTTRFormat_V1()
{
	VString		doc;

	char		theTime[512] = {0};
	time_t now = ::time( NULL);
	::strftime( theTime, sizeof( theTime),"%Y-%m-%dT%H:%M:%S", localtime( &now));

	doc = "Text (Tab-Tab-Return) format version ";
	doc.AppendLong(kTTR_FORMAT_VERSION);
	doc.AppendCString("\r\r");

	// ===============================================
	doc += "--------------------------------------------------\r";
	doc += "General\r";
	doc += "--------------------------------------------------\r";
	doc.AppendPrintf("Every entry is quoted by square brackets. For example: [%d] for \"FlushFromLanguage\".\r", eCLEntryKind_FlushFromLanguage);
	doc.AppendPrintf("First entry in the log is the \"start\" entry (%d), it gives the current date-time.\r", eCLEntryKind_Start);
	doc += "Then, every other entry is followed by [tab] ellapsed second since log started [tab] Task ID {and optionnaly: [tab] other infos...}\r\r";
	doc += "For example, when the log starts, first line is something like:\r";
	doc.AppendPrintf("[%d]\t%s\t10\r", eCLEntryKind_Start, theTime);
	doc += "Then you may have (FlushFromLanguage, executed 5 seconds since the log started, for task ID 10):\r";
	doc.AppendPrintf("[%d]\t5\t10\r", eCLEntryKind_FlushFromLanguage);

	doc += "\rEntries may be quoted with start/end. In this case, the format is [entry num]s and [entry num]e.";
	doc += " Between both tags, several other entries may be logged, from the same task ID or from others.\r";
	doc += "In all cases, the 'end' tag is formatted: [entry num]e [tab] ellapsed second since log started [tab] taskID [tab] count of milliseconds since [entry num]start\r";
	doc += "For example, with the FlushFromLanguage kind launched from task ID 10, the log could be:\r";
	doc.AppendPrintf("[%d]s\t123\t10\t(...other infos - see format)\r", eCLEntryKind_FlushFromLanguage);
	doc += ". . .\r. . .\r. . .\r";
	doc.AppendPrintf("[%d]s\t126\t10\t(...other infos - see format)\r", eCLEntryKind_FlushFromLanguage);

	// ===============================================
	doc += "--------------------------------------------------\r";
	doc += "List of all kinds of entry (name [tab] value)\r";
	doc += "--------------------------------------------------\r";
	doc.AppendPrintf("Unknown\t%d\r", eCLEntryKind_Unknown);
	doc.AppendPrintf("Start\t%d\r", eCLEntryKind_Start);
	doc.AppendPrintf("Stop\t%d\r", eCLEntryKind_Stop);
	doc.AppendPrintf("Comment\t%d\r", eCLEntryKind_Comment);
	doc.AppendPrintf("NeedsBytes\t%d\r", eCLEntryKind_NeedsBytes);
	doc.AppendPrintf("CallNeedsBytes\t%d\r", eCLEntryKind_CallNeedsBytes);
	doc.AppendPrintf("FlushFromLanguage\t%d\r", eCLEntryKind_FlushFromLanguage);
	doc.AppendPrintf("FlushFromMenuCommand\t%d\r", eCLEntryKind_FlushFromMenuCommand);
	doc.AppendPrintf("FlushFromScheduler\t%d\r", eCLEntryKind_FlushFromScheduler);
	doc.AppendPrintf("FlushFromBackup\t%d\r", eCLEntryKind_FlushFromBackup);
	doc.AppendPrintf("FlushFromNeedsBytes\t%d\r", eCLEntryKind_FlushFromNeedsBytes);
	doc.AppendPrintf("FlushFromRemote\t%d\r", eCLEntryKind_FlushFromRemote);
	doc.AppendPrintf("FlushFromUnknown\t%d\r", eCLEntryKind_FlushFromUnknown);
	doc.AppendPrintf("Flush\t%d\r", eCLEntryKind_Flush);
	doc.AppendPrintf("MemStats\t%d\r", eCLEntryKind_MemStats);

	doc.AppendCString("\r\r");

	// ===============================================
	doc += "--------------------------------------------------\r";
	doc += "Format and meaning of each kind\r";
	doc += "--------------------------------------------------\r";

	doc.AppendPrintf("[%d]\r", eCLEntryKind_Unknown);
	doc += "Unknown entry kind\r";
	doc.AppendPrintf("[%d] [tab] ellapsed second since log started [tab] taskID\r\r", eCLEntryKind_Unknown);

	doc.AppendPrintf("[%d]\r", eCLEntryKind_Start);
	doc += "The log starts. Quoted (start/end) entry, with misc. infos between the start and the end.\r";
	doc.AppendPrintf("[%d]s [tab] current time [tab] taskID [tab] version [cr]\r", eCLEntryKind_Start);
	doc.AppendPrintf("struct [tab] path to host database structure file [cr]\r");
	doc.AppendPrintf("data [tab] path to host database data file [cr]\r");
	doc.AppendPrintf("[%d]e\r\r", eCLEntryKind_Start);

	doc.AppendPrintf("[%d]\r", eCLEntryKind_Stop);
	doc += "The log ends\r";
	doc.AppendPrintf("[%d] [tab] ellapsed second since log started [tab] taskID\r\r", eCLEntryKind_Stop);

	doc.AppendPrintf("[%d]\r", eCLEntryKind_Comment);
	doc += "A comment added by the developer. Comments are always quoted with start and end, a \\r is added at beginning and end.\r";
	doc.AppendPrintf("[%d]start [tab] ellapsed second since log started [tab] taskID [cr] the comment [cr] [%d]end [tab] time [tab] taskID [tab] 0 (the milliseconds)\r\r", eCLEntryKind_Comment);

	doc.AppendPrintf("[%d]\r", eCLEntryKind_NeedsBytes);
	doc += "Any source asks memory to the cache manager\r";
	doc.AppendPrintf("[%d] [tab] ellapsed second since log started [tab] taskID [tab] task name [tab] process 4D num [tab] needed bytes (very large int.)\r", eCLEntryKind_NeedsBytes);
	doc.AppendPrintf("Note: may be followed by [%d]\r\r", eCLEntryKind_MemStats);

	doc.AppendPrintf("[%d]\r", eCLEntryKind_CallNeedsBytes);
	doc += "Memory manager asks memory to the cache manager: not enough space in the cache to allocate memory. The cache manager will try to free unused objects, to flush, etc...\r";
	doc.AppendPrintf("[%d] [tab] ellapsed second since log started [tab] taskID [tab] task name [tab] process 4D num [tab] needed bytes (very large int.)\r", eCLEntryKind_CallNeedsBytes);
	doc.AppendPrintf("Note: may be followed by [%d]\r\r", eCLEntryKind_MemStats);

	doc.AppendPrintf("[%d], [%d], [%d], [%d], [%d], [%d], [%d]\r", eCLEntryKind_FlushFromLanguage,
																	eCLEntryKind_FlushFromMenuCommand,
																	eCLEntryKind_FlushFromScheduler,
																	eCLEntryKind_FlushFromBackup,
																	eCLEntryKind_FlushFromNeedsBytes,
																	eCLEntryKind_FlushFromRemote,
																	eCLEntryKind_FlushFromUnknown);
	doc += "Action at the origin of a flush. All the 'FlushFrom...' share the same format.\r";
	doc.AppendPrintf("[%d] [tab] ellapsed second since log started [tab] taskID [tab] task name [tab] process 4D num [tab] isWaitUntilDone (1 = yes, 0 = no, -1 = unknown) [tab] isEmptyCache (1 = yes, 0 = no, -1 = unknown)\r", eCLEntryKind_CallNeedsBytes);
	doc.AppendPrintf("Note: may be followed by [%d]\r", eCLEntryKind_MemStats);
	doc.AppendPrintf("Note: [%d] means a flush was requested from the remote, but we don't have the exact origin (does a client called FLUSH BUFFERS explicitely? ...)\r\r", eCLEntryKind_FlushFromRemote);

	doc.AppendPrintf("[%d]\r", eCLEntryKind_Flush);
	doc += "Flush (we are inside the Flush Manager)\r";
	doc.AppendPrintf("[%d] [tab] ellapsed second since log started [tab] taskID\r", eCLEntryKind_Flush);
	doc.AppendPrintf("Note: may be followed by [%d]\r\r", eCLEntryKind_MemStats);

	doc.AppendPrintf("[%d]\r", eCLEntryKind_MemStats);
	doc += "The memory statistics. Level of stats is a bit field, to get specific stats, add the following values:\r";
	doc.AppendPrintf("Mini stats (default value): %d\r", eCLDumpStats_Mini);
	doc.AppendPrintf("Objects: %d\r", eCLDumpStats_Objects);
	doc.AppendPrintf("Blocks: %d\r", eCLDumpStats_Blocks);
	doc.AppendPrintf("SmallBlocks: %d\r", eCLDumpStats_SmallBlocks);
	doc.AppendPrintf("BigBlocks: %d\r", eCLDumpStats_BigBlocks);
	doc.AppendPrintf("OtherBlocks: %d\r", eCLDumpStats_OtherBlocks);
	doc.AppendPrintf("All: %d\r", eCLDumpStats_All);

	doc.AppendPrintf("[%d] [tab] ellapsed second since log started [tab] taskID [tab] origin of the stats (a log entry number) [cr] the stats\r", eCLEntryKind_MemStats);
	doc += "the stats themselves are in xml:\r";
	doc += "Main tag: <mem_stats level=\"stat level\" size=\"total amount of memory\" used=\"amount ofused memory\">\r";
	doc += "Then come the details:\r";
	doc += "<system phys=\"physical memory size\" free=\"free memory size\" app_used_phys=\"physical used by app\" used_virtual=\"virtual meory used\" />\r";
	doc += "Allocations blocks: <alloc count=\"number of virtual allocations\" tot=\"total allocated\" />\r";
	doc += "Then come the detailed stats, depending on the level. Note: infos are the same as the one found in GET CACHE STATISTIC.";
	doc += ", the xml uses some abbreviations to reduce a bit the size: bb = \"big blocks\", sb = \"small blocks\", ob = \"other blocks\"\r";
	doc += "Example of log:\r";

	doc += "<mem_stats level=\"1\" tid=\"13\" tname=\"P_1\" pnum=\"5\" size=\"10704570434977792\" used=\"2306663092733982407\">\r";
	doc += "<system phys=\"2143993856\" free=\"1\" app_used_phys=\"2882306048\" used_virtual=\"0\"/>";
	doc += "<alloc count=\"2\" tot=\"104857600\"/>";
	doc += "<stats free=\"102365184\" nb_bb=\"165\" used_bb=\"163\" free_bb=\"2\" nb_pages=\"104\" nb_sb=\"4411\" used_sb=\"4401\" free_sb=\"10\" biggest_block=\"70124512\" biggest_free_block=\"70124512\" nb_obj=\"3852\"/>";
	doc += "</mem_stats>\r\r";

	VFolder *userDocsFolder = VFolder::RetainSystemFolder(eFK_UserDocuments, false);
	if(userDocsFolder != NULL)
	{
		VFile	logDoc(*userDocsFolder, CVSTR("cache_log_doc_v1.txt"));

		VError err = VE_OK;
		if(logDoc.Exists())
			err = logDoc.Delete();

		err = logDoc.Create();

		if(err == VE_OK)
		{
			VFileStream logDocDump(&logDoc);

			err = logDocDump.OpenWriting();

			if(err == VE_OK)
			{
				logDocDump.PutText(doc);
			}
			
			
			if(err == VE_OK)
				err = logDocDump.CloseWriting();
		}
		userDocsFolder->Release();
	}

}
#endif

//	==================================================
//	VCacheLog / Get-destroy the singleton (static accessors)
//	==================================================
/*static*/
VCacheLog * VCacheLog::Get()
{
	if(s_cacheLog == NULL)
	{
		VTaskLock		theLock(&s_mutex);
		if(s_cacheLog == NULL)
		{
			s_cacheLog = new VCacheLog;
		}
	}

	return s_cacheLog;
}

/*static*/
void VCacheLog::Destroy()
{
	if(s_cacheLog != NULL)
	{
		VTaskLock		theLock(&s_mutex);
		
		delete s_cacheLog;
		s_cacheLog = NULL;
	}
}

#pragma mark-
//	==================================================
//	VCacheLog / Start-stop
//	==================================================
bool VCacheLog::Start(const XBOX::VFolder& inFolder, const XBOX::VString& inLogFileName, const XBOX::VValueBag *inParams)
{
#if VERSIONDEBUG && 0
	_ExplainTTRFormat_V1();
#endif

	VTaskLock lock(&s_mutex);

	_StopWithoutLocking();

	fLogFile = new VSplitableLogFile( inFolder, inLogFileName, 1);
	if (fLogFile != NULL)
	{
		if (fLogFile->Open(true))
		{
			s_isStarted = true;
			fStarTimeMS = XBOX::VSystem::GetCurrentTime();

			fMemStatsLevel = eCLDumpStats_Mini;

			VString		appVersion, structPath, dataPath, cacheSettings;
			if(inParams != NULL)
			{
				sLONG	level = 0;
				
				// If stats level not passed or equal to zero => stays to eCLDumpStats_Mini.
				if( inParams->GetLong("mem_stats_level", level) && level != 0)
					fMemStatsLevel = (ECacheLogMemStatsLevel)	level;

				inParams->GetString("application_version", appVersion);
				inParams->GetString("structure_path", structPath);
				inParams->GetString("data_path", dataPath);
				inParams->GetString("cache_settings", cacheSettings);
			}

			char		theTime[512] = {0};
			time_t now = ::time( NULL);
			::strftime( theTime, sizeof( theTime),"%Y-%m-%dT%H:%M:%S", localtime( &now));
			
			// Start of "begin log"
			fLogFile->AppendFormattedString("[%d]s\t%s\t%d\t%d\r", (sLONG) eCLEntryKind_Start, theTime, XBOX::VTask::GetCurrent()->GetID(), kTTR_FORMAT_VERSION);

			// Add the misc. infos
			char	cStrBuffer[2048];

			VString		sysVersStr;
			XBOX::VSystem::GetOSVersionString(sysVersStr);
			cStrBuffer[0] = 0;
			sysVersStr.ToCString(cStrBuffer, 2048);
			fLogFile->AppendFormattedString("%s\r", cStrBuffer);

			cStrBuffer[0] = 0;
			appVersion.ToCString(cStrBuffer, 2048);
			fLogFile->AppendFormattedString("Application version\t%s\r", cStrBuffer);

			cStrBuffer[0] = 0;
			structPath.ToCString(cStrBuffer, 2048);
			fLogFile->AppendFormattedString("struct\t%s\r", cStrBuffer);
			
			cStrBuffer[0] = 0;
			dataPath.ToCString(cStrBuffer, 2048);
			fLogFile->AppendFormattedString("data\t%s\r", cStrBuffer);
			
			cStrBuffer[0] = 0;
			cacheSettings.ToCString(cStrBuffer, 2048);
			fLogFile->AppendFormattedString("Cache settings:\r%s\r", cStrBuffer);

			// End of "begin log"
			fLogFile->AppendFormattedString("[%d]e\r", (sLONG) eCLEntryKind_Start);
		}
		else
		{
			_StopWithoutLocking();
		}
	}

	return fLogFile != NULL;
}

void VCacheLog::Stop()
{
	VTaskLock lock(&s_mutex);
	_StopWithoutLocking();
}

void VCacheLog::_StopWithoutLocking()
{
	if (fLogFile != NULL)
	{
		Log( eCLEntryKind_Stop );

		fLogFile->Close();
		delete fLogFile;
		fLogFile = NULL;

		fStarTimeMS = 0;
	}
	s_isStarted = false;
}


#pragma mark-
//	==================================================
//	VCacheLog / Append to the log
//	==================================================
void VCacheLog::Log(ECacheLogEntryKind inLogWhat, bool inWithStartFlag)
{
	if (fLogFile != NULL)
	{
		VTaskLock lock(&s_mutex);
		if (fLogFile != NULL)
		{
			_UpdateStartFlag(inWithStartFlag);

			switch(inLogWhat)
			{
			case eCLEntryKind_MemStats:
				_LogMemStatsWithoutLocking();
				break;

			default:
				if(_IsFlushFromActionEntrykind(inLogWhat))
				{
					_LogFlushFromActionWithoutLocking(inLogWhat, -1, -1);
				}
				else
				{
					_AddEntry(inLogWhat, true);
				}
				break;
			}
			_UpdateStartFlag(false);
		}
	}
}

void VCacheLog::LogComment( const XBOX::VString& inComment )
{
	if(!inComment.IsEmpty())
	{
		if (fLogFile != NULL)
		{
			VTaskLock lock(&s_mutex);
			if (fLogFile != NULL)
			{
				_UpdateStartFlag(true);
				_AddEntry(eCLEntryKind_Comment);
				_UpdateStartFlag(false);

				fLogFile->AppendFormattedString("\r%s\r", fConverter.ConvertString(inComment));

				_AddEntryEnd(eCLEntryKind_Comment, true);
			}
		}
	}
}

void VCacheLog::LogFlushFromAction( ECacheLogEntryKind inOriginOfTheFlush, sLONG inIsWaitUntilDone, sLONG inIsEmptyCache, bool inWithMemStats, bool inWithStartFlag)
{
	if (fLogFile != NULL)
	{
		VTaskLock lock(&s_mutex);
		if (fLogFile != NULL)
		{
			_UpdateStartFlag(inWithStartFlag);
			_LogFlushFromActionWithoutLocking(inOriginOfTheFlush, inIsWaitUntilDone, inIsEmptyCache);
			_UpdateStartFlag(false);

			if(inWithMemStats)
				_LogMemStatsWithoutLocking(inOriginOfTheFlush);
		}
	}
}

void VCacheLog::LogNeedsBytes(ECacheLogEntryKind inKind, VSize inNeededBytes, bool inWithMemStats, bool inWithStartFlag)
{
	if (fLogFile != NULL)
	{
		VTaskLock lock(&s_mutex);
		if (fLogFile != NULL)
		{
			_FormatCurrentVTaskInfos();

			_UpdateStartFlag(inWithStartFlag);
			_AddEntry(inKind);
			fLogFile->AppendFormattedString("\t%s\t%llu\r", fVTaskInfos, (uLONG8) inNeededBytes);
			_UpdateStartFlag(false);

			if(inWithMemStats)
				_LogMemStatsWithoutLocking(inKind);
		}
	}
}

void VCacheLog::LogEndOfOperation(ECacheLogEntryKind inKind, sLONG8 inDurationMilliSeconds)
{
	if (fLogFile != NULL)
	{
		VTaskLock lock(&s_mutex);
		if (fLogFile != NULL)
		{
			_AddEntryEnd(inKind);
			fLogFile->AppendFormattedString("\t%lu\r", (uLONG) inDurationMilliSeconds);
		// Flush the log now depending on the entry
			switch(inKind)
			{
			case eCLEntryKind_Flush:
				fLogFile->Flush();
				break;

			default:
				break;
			}
		}
	}
}

void VCacheLog::LogMemStats(ECacheLogEntryKind inOrigin)
{
	if (fLogFile != NULL)
	{
		VTaskLock lock(&s_mutex);
		if (fLogFile != NULL)
		{
			_LogMemStatsWithoutLocking(inOrigin);
		}
	}
}

void VCacheLog::_LogMemStatsWithoutLocking(ECacheLogEntryKind inOrigin)
{
	if(fLogFile == NULL)
		return;
	
	XBOX::VCppMemMgr *memMgr = VDBMgr::GetCacheManager()->GetMemoryManager();

	_AddEntry(eCLEntryKind_MemStats);
	fLogFile->AppendFormattedString("\t%d\r", (sLONG) inOrigin);

// "Header", with main infos
	VSize theSize, usedMem;
	memMgr->GetMemUsageInfo(theSize, usedMem);
	_FormatCurrentVTaskInfos(true);
	fLogFile->AppendFormattedString("<mem_stats level=\"%d\" %s size=\"%llu\" used=\"%llu\">\r", fMemStatsLevel, fVTaskInfos, (uLONG8) theSize, (uLONG8) usedMem);

// Physical/virtual sizes
	fLogFile->AppendFormattedString("<system phys=\"%llu\" free=\"%llu\" app_used_phys=\"%llu\" used_virtual=\"%llu\"/>\r",
										(uLONG8) VSystem::GetPhysicalMemSize(),
										(uLONG8) VSystem::GetPhysicalFreeMemSize(),
										(uLONG8) VSystem::GetApplicationPhysicalMemSize(),
										(uLONG8) VSystem::VirtualMemoryUsedSize() );

// Allocation blocks
	// Voir avec LE/LR si c'est utile. Le code qui parcourt les infos des blocs d'allocation
	// ("DumpAllocationToLog") est plus loin, commenté.
	// En attendant, on Log le minimum
	VIndex	countAlloc = memMgr->CountVirtualAllocations();
	VSize	totalAlloc = memMgr->GetAllocatedMem();
	fLogFile->AppendFormattedString("<alloc count=\"%d\" tot=\"%llu\"/>\r", countAlloc, (uLONG8) totalAlloc);


// Mem. stats
	VMemStats stats;
	memMgr->GetStatistics(stats);
	_DumpMemStatsWithoutLocking(stats);


// "Footer"
	fLogFile->AppendFormattedString("</mem_stats>\r");
}

void VCacheLog::_DumpMemStatsWithoutLocking(const VMemStats& inStats)
{
	if(fLogFile == NULL)
		return;

	// Dump values that are always dumped
	fLogFile->AppendFormattedString("<stats free=\"%llu\" nb_bb=\"%d\" used_bb=\"%d\" free_bb=\"%d\" nb_pages=\"%d\" nb_sb=\"%d\" used_sb=\"%d\" free_sb=\"%d\" biggest_block=\"%llu\" biggest_free_block=\"%llu\" nb_obj=\"%d\"/>\r",
										(uLONG8) inStats.fFreeMem,
										inStats.fNbBigBlocks,
										inStats.fNbBigBlocksUsed,
										inStats.fNbBigBlocksFree,
										inStats.fNbPages,
										inStats.fNbSmallBlocks,
										inStats.fNbSmallBlocksUsed,
										inStats.fNbSmallBlocksFree,
										(uLONG8) inStats.fBiggestBlock,
										(uLONG8) inStats.fBiggestBlockFree,
										inStats.fNbObjects);

	if(fMemStatsLevel & eCLDumpStats_Objects)
	{
		fLogFile->AppendFormattedString("<objects>\r");
		for( XBOX::VMapOfObjectInfo::const_iterator i = inStats.fObjectInfo.begin() ; i != inStats.fObjectInfo.end() ; ++i)
		{
			const std::type_info* typobj = i->first;
			VStr255 s;
			XBOX::VSystem::DemangleSymbol( typobj->name(), s);
			char	name255[256] = {0};
			if(!s.IsEmpty())
				s.ToCString(name255, 255);

			fLogFile->AppendFormattedString("<obj name=\"%s\" count=\"%u\" tot_size=\"%llu\"/>\r", name255, i->second.fCount, (uLONG8) i->second.fTotalSize);
		}
		fLogFile->AppendFormattedString("<objects/>\r");
	}

	if(fMemStatsLevel & eCLDumpStats_Blocks)
	{
		fLogFile->AppendFormattedString("<blocks>\r");
		for( XBOX::VMapOfBlockInfo::const_iterator i = inStats.fBlockInfo.begin() ; i != inStats.fBlockInfo.end() ; ++i)
		{
			sLONG tag = i->first;
#if SMALLENDIAN
			ByteSwap(&tag);
#endif
			char	nameHex[20] = {0};
			char	nameOSType[20] = {0};
			::sprintf(nameHex, "0x%x", tag);
			::memcpy(nameOSType, &tag, sizeof(tag));
			nameOSType[ sizeof(tag) ] = 0;

			fLogFile->AppendFormattedString("<block tag=\"%s\" tag_hex=\"%s\" count=\"%u\" tot_size=\"%llu\"/>\r", nameOSType, nameHex, i->second.fCount, (uLONG8) i->second.fTotalSize);
		}
		fLogFile->AppendFormattedString("<blocks/>\r");
	}

	if(fMemStatsLevel & eCLDumpStats_SmallBlocks)
	{
		fLogFile->AppendFormattedString("<mem_sb>\r");
		for( XBOX::VMapOfMemBlockInfo::const_iterator i = inStats.fSmallBlockInfo.begin() ; i != inStats.fSmallBlockInfo.end() ; ++i)
		{
			fLogFile->AppendFormattedString("<sb name=\"%u\" used=\"%d\" free=\"%d\"/>\r", i->first, i->second.fUsedCount, i->second.fFreeCount);
		}
		fLogFile->AppendFormattedString("<mem_b/>\r");
	}

	if(fMemStatsLevel & eCLDumpStats_BigBlocks)
	{
		fLogFile->AppendFormattedString("<mem_bb>\r");
		for( XBOX::VMapOfMemBlockInfo::const_iterator i = inStats.fBigBlockInfo.begin() ; i != inStats.fBigBlockInfo.end() ; ++i)
		{
			fLogFile->AppendFormattedString("<bb name=\"%u\" used=\"%d\" free=\"%d\"/>\r", i->first, i->second.fUsedCount, i->second.fFreeCount);
		}
		fLogFile->AppendFormattedString("<mem_b/>\r");
	}

	if(fMemStatsLevel & eCLDumpStats_OtherBlocks)
	{
		fLogFile->AppendFormattedString("<mem_ob>\r");
		for( XBOX::VMapOfMemBlockInfo::const_iterator i = inStats.fOtherBlockInfo.begin() ; i != inStats.fOtherBlockInfo.end() ; ++i)
		{
			fLogFile->AppendFormattedString("<ob name=\"%u\" used=\"%d\" free=\"%d\"/>\r", i->first, i->second.fUsedCount, i->second.fFreeCount);
		}
		fLogFile->AppendFormattedString("<mem_ob/>\r");
	}
}

/*****************************************

	Initialement mis dans VMemoryImpl.cpp, parce que pour accéder à ces infos,
	il faut aussi accéder à des chamsp private de VMemCppImpl (par exemple fFirstAllocation)
	
void VMemCppImpl::DumpAllocationToLog(VLog *inLog)
{
	VLog*	retainedLog = RetainRefCountable(inLog);
	if(retainedLog != NULL)
	{
		VKernelTaskLock lock(&fGlobalMemMutext);

		inLog->Append("<mem_allocations>\r");
		for( VMemImplAllocation *allocation = fFirstAllocation ; allocation != NULL ; allocation = allocation->GetNext())
		{
			inLog->Append("<mem_alloc size=\"%u\" start=\"%u\" end=\"%u\"/>\r",
							allocation->GetSystemSize(),
							(uLONG) (sLONG_PTR) allocation->GetDataStart(),
							(uLONG) (sLONG_PTR) allocation->GetDataEnd() );
		}
		inLog->Append("</mem_allocations>\r");

		retainedLog->Release();
	}
}
*****************************************/

#pragma mark -
//	==================================================
//	VCacheLog / Utilities
//	==================================================
void VCacheLog::_AddEntry(ECacheLogEntryKind inKind, bool inAddReturn)
{
	_FormatEllapsedSeconds();

	if(inAddReturn)
		fLogFile->AppendFormattedString("[%d]%s\t%s\t%d\r", (sLONG) inKind, fStart, fEllapsedSeconds, XBOX::VTask::GetCurrent()->GetID());
	else
		fLogFile->AppendFormattedString("[%d]%s\t%s\t%d", (sLONG) inKind, fStart, fEllapsedSeconds, XBOX::VTask::GetCurrent()->GetID());

}

void VCacheLog::_AddEntryEnd(ECacheLogEntryKind inKind, bool inAddReturn)
{
	_FormatEllapsedSeconds();

	if(inAddReturn)
		fLogFile->AppendFormattedString("[%d]e\t%s\t%d\r", (sLONG) inKind, fEllapsedSeconds, XBOX::VTask::GetCurrent()->GetID());
	else
		fLogFile->AppendFormattedString("[%d]e\t%s\t%d", (sLONG) inKind, fEllapsedSeconds, XBOX::VTask::GetCurrent()->GetID());
}

void VCacheLog::_UpdateStartFlag(bool inWithFlag)
{
	if(inWithFlag)
		::strcpy(fStart, "s");
	else
		fStart[0] = 0;
}

void VCacheLog::_FormatEllapsedSeconds()
{
	uLONG	ellapsedMS = XBOX::VSystem::GetCurrentTime() - fStarTimeMS;
	::sprintf( fEllapsedSeconds, "%d", (sLONG) (ellapsedMS / 1000) );
}

void VCacheLog::_FormatCurrentVTaskInfos(bool inForTags)
{
	XBOX::VString	taskName;
	char			nameCStr[256] = {0};

	VTask *currentTask = VTask::GetCurrent();
	currentTask->GetName(taskName);
	taskName.ToCString(nameCStr, 256);

	sLONG	pNum = 0;
	if(currentTask->GetKind() == '4dut')
		pNum = (sLONG) currentTask->GetKindData();

	/********* WARNING **********
	fVTaskInfos is char[512]. If you add more infos, think about increasing its size
	*/
	fVTaskInfos[0] = 0;
	if(inForTags)
	{
		sprintf(fVTaskInfos, "tname=\"%s\" pnum=\"%d\"", nameCStr, pNum);
	}
	else
	{
		sprintf(fVTaskInfos, "%s\t%d", nameCStr, pNum);
	}
}

bool VCacheLog::_IsFlushFromActionEntrykind(ECacheLogEntryKind inKind) const
{
	switch(inKind)
	{
		case eCLEntryKind_FlushFromLanguage:
		case eCLEntryKind_FlushFromMenuCommand:
		case eCLEntryKind_FlushFromScheduler:
		case eCLEntryKind_FlushFromBackup:
		case eCLEntryKind_FlushFromNeedsBytes:
		case eCLEntryKind_FlushFromRemote:
		case eCLEntryKind_FlushFromUnknown:
			return true;
			break;
	}

	return false;
}

void VCacheLog::_LogFlushFromActionWithoutLocking( ECacheLogEntryKind inKind, sLONG inIsWaitUntilDone, sLONG inIsEmptyCache)
{
	if (fLogFile != NULL)
	{
		VTaskLock lock(&s_mutex);
		if (fLogFile != NULL)
		{
			_FormatCurrentVTaskInfos();

			_AddEntry(inKind);
			fLogFile->AppendFormattedString("\t%s\t%d\t%d\r", fVTaskInfos, inIsWaitUntilDone, inIsEmptyCache);
		}
	}
}

#pragma mark -
//	==================================================
//	VCacheLog / Contruct-destruct (privates)
//	==================================================
VCacheLog::VCacheLog() :
fLogFile(NULL),
fConverter(XBOX::VTC_StdLib_char),
fMemStatsLevel(eCLDumpStats_Mini),
fStarTimeMS(0)
{
	fEllapsedSeconds[0] = 0;
	fVTaskInfos[0] = 0;
	fStart[0] = 0;
}

VCacheLog::~VCacheLog()
{
	Stop();
}



#pragma mark -
//	==================================================
//	VCacheLogQuotedEntry
//	==================================================
void VCacheLogQuotedEntry::_Reset()
{
	fLogWasStarted = false;
	fLogKind = eCLEntryKind_Unknown;
	fQuoteWithMemStats = false;
	fWithCounter = false;
}

VCacheLogQuotedEntry::VCacheLogQuotedEntry(ECacheLogEntryKind inKind, bool inQuoteWithMemStats, bool inWithCounter)
{
	_Reset();

	fLogKind = inKind;
	fQuoteWithMemStats = inQuoteWithMemStats;
	fWithCounter = inWithCounter;

	if(VCacheLog::IsStarted())
	{
		fLogWasStarted = true;

		VCacheLog::Get()->Log(fLogKind, true);

		if(fQuoteWithMemStats)
			VCacheLog::Get()->LogMemStats(fLogKind);

		if(fWithCounter)
			_StartCounter();
	}
}

VCacheLogQuotedEntry::VCacheLogQuotedEntry(ECacheLogEntryKind inKind, const XBOX::VSize& inNeededBytes, bool inQuoteWithMemStats, bool inWithCounter)
{
	_Reset();
	
	fLogKind = inKind;
	fQuoteWithMemStats = inQuoteWithMemStats;
	fWithCounter = inWithCounter;

#if VERSIONDEBUG
	switch(inKind)
	{
	case eCLEntryKind_NeedsBytes:
	case eCLEntryKind_CallNeedsBytes:
		break;

	default:
		assert(false);
		break;
	}
#endif

	if(VCacheLog::IsStarted())
	{
		fLogWasStarted = true;

		VCacheLog::Get()->LogNeedsBytes(inKind, inNeededBytes, fQuoteWithMemStats, true);

		if(fWithCounter)
			_StartCounter();
	}
}

VCacheLogQuotedEntry::VCacheLogQuotedEntry()
{
	_Reset();

	fLogWasStarted = VCacheLog::IsStarted();
}


VCacheLogQuotedEntry::~VCacheLogQuotedEntry()
{
	if(fLogWasStarted && VCacheLog::IsStarted())
	{
		if(fQuoteWithMemStats)
			VCacheLog::Get()->LogMemStats(fLogKind);

		if(fWithCounter)
		{
			VCacheLog::Get()->LogEndOfOperation(fLogKind, _StopCounter() / 1000);
		}
		else
		{
			VCacheLog::Get()->LogEndOfOperation(fLogKind);
		}
	}
}

void VCacheLogQuotedEntry::_StartCounter()
{
	fWithCounter = true;
	fCounter.Start();
}

sLONG VCacheLogQuotedEntry::_StopCounter()
{
	if(fWithCounter)
	{
		fWithCounter = false;
		return fCounter.Stop();
	}
	return 0;
}


#pragma mark -
//	==================================================
//	VCacheLogEntryFlush
//	==================================================
VCacheLogEntryFlushFromAction::VCacheLogEntryFlushFromAction(ECacheLogEntryKind inOrigin, bool inIsWaitUntilDone, bool isEmptyMem, bool inQuoteWithMemStats) :
VCacheLogQuotedEntry()
{
	fLogKind = inOrigin;
	fQuoteWithMemStats = inQuoteWithMemStats;

	if(fLogWasStarted)
	{
		VCacheLog::Get()->LogFlushFromAction(inOrigin, inIsWaitUntilDone ? 1 : 0, isEmptyMem ? 1 : 0, fQuoteWithMemStats, true);

		_StartCounter(); // Always a counter for Flush log
	}
}


VCacheLogEntryFlushFromAction::~VCacheLogEntryFlushFromAction()
{
	// . . .let's parent to do everything (log mem stats if needed, end of log entry, ...)
}

