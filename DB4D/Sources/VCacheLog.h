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
#ifndef __CacheLog__
#define __CacheLog__


/** @Class	VCacheLog

			Log infos about flush and cache. This log:
				- Is thread safe
				- Each entry of the log is atomic: if an entry writes several lines (typically memory stats),
				  no other log entry will be inserted between the lines.
				- A doc of the text format can be generated using the static _ExplainTTRFormat_V1() routine.
			
			There is only one VCacheLog, you access it using VCacheLog::Get().

			Typical use:
				(1) Start the log:
						VCacheLog::get()->Start(. . .);

				(2) Use utility objects to add entries:
						VCacheLogQuotedEntry	logEntry(eCLEntryKind_Flush, false, true);
						. . .
						VCacheLogQuotedEntry	logEntry(eCLEntryKind_NeedsBytes, inNeededBytes);

				(3) Stop the log:
					VCacheLog::get()->Stop();

			* * * * * * * * * * * * * * * * * IMPORTANT * * * * * * * * * * * * * * * * *
			*																			*
			*	Logging is time consuming and can dramatically reduce performances,		*
			*	remember you're inside DB4D, which is at the heart of about				*
			*	everything ;->. So, before adding calls to the log, carefuly think		*
			*	about it, talk with other developpers, check the real needs, etc.		*
			*																			*
			* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
class VCacheLog : public XBOX::VObject
{
public:
	/** @name Static accessors */
	//@{
	/** @brief	Get() returns the singleton */
	static  VCacheLog *			Get();
	/** @brief	Free memory used by the singleton */
	static	void				Destroy();
	/**@brief	IsStarted() is a quick accessor not thread safe. Its purpose is to let the caller to test if a log
				has been started or not without locking the mutex.
				2 possibles issues are:
				1/	IsStarted() returns false, while at the same time Start() is called. In this situation, the
					caller will miss writting the log.

				2/	IsStarted() returns true while at the same time, Stop() is called. Here, next call to Log(...)
					will just do nothing.
	*/
	static	bool				IsStarted()	{ return VCacheLog::s_isStarted; }
	//@}


	/*@brief	inParams is used as "multipurpose-evolutive-parameters", so we don't need to change DB4D.h,
				DB4DComponent.h, DB4DComponent.cpp, etc... when we want to add/modify start parameters. */
			bool				Start(const XBOX::VFolder& inFolder, const XBOX::VString& inLogFileName, const XBOX::VValueBag *inParams = NULL);
			void				Stop();

			void				Log( ECacheLogEntryKind inLogWhat, bool inWithStartFlag = false );
			void				LogComment( const XBOX::VString& inComment );
	/**@brief	inIsWaitUntilDone and inIsEmptyCache: 0 == false, 1 == true, -1 == unknown. It can be unknown
				in some recursive entries */
			void				LogFlushFromAction( ECacheLogEntryKind inOriginOfTheFlush, sLONG inIsWaitUntilDone, sLONG inIsEmptyCache, bool inWithMemStats = false, bool inWithStartFlag = false);
			void				LogNeedsBytes(ECacheLogEntryKind inKind, VSize inNeededBytes, bool inWithMemStats = false, bool inWithStartFlag = false);
			void				LogMemStats( ECacheLogEntryKind inOrigin = eCLEntryKind_Unknown );

	/** @name Quoted entries */
	//@{
	/** @brief	Quoted entries are log entries where:
					-	ECacheLogEntryKind is first inserted in the log as "[log kind]start"
						instead of just "[log kind]"
					-	(...here goes probably other log entries...)
					-	Then the entry is inserted as "[log kind]end" (instead of nothing)

				Usage :
					1/ Call a Log... routine that has a inWithStartFlag param and set it to true
					   NOTE: not all Log...routine have the "inWithStartFlag" parameter. only the one we need at this time.
					2/ Add any other entry to the log
					3/ Call LogEndOfOperation

				Those accessors are mainly used by VCacheLogQuotedEntry class.				
	*/
	/** @brief	For Quoted Entries, the line will starts with "[logkind]end" */
			void				LogEndOfOperation( ECacheLogEntryKind inKind, sLONG8 inDurationMilliSeconds = 0);
	//@}

private:
	/* static singletons */
	static	VCacheLog *				s_cacheLog;
	static	XBOX::VCriticalSection	s_mutex;
	static bool						s_isStarted;

	/* Constructor-destructor are private */
								VCacheLog();
								~VCacheLog();

	/** @name Utilities */
	//@{
	// @brief	Do operation (stop, flush, append) without locking the mutex (ie: we know it's already locked)
	//			Goal is to speed up a bit the things.
	inline	void				_StopWithoutLocking();
	inline	void				_FormatCurrentVTaskInfos(bool inForTags = false);
	/** @brief	Dump the full memory stats, does'nt lock the mutex (called in several public routines) */
			void				_LogMemStatsWithoutLocking(ECacheLogEntryKind inOrigin = eCLEntryKind_Unknown);
	/** @brief	Dump the memory stats, does'nt lock the mutex (usually called by public accessors) */
			void				_DumpMemStatsWithoutLocking(const VMemStats& inStats);
	/** @brief	Log infos for a flush generated from an action (4D language, menu, backup, ...),
				does'nt lock the mutex (usually called by public accessors)
				inKind should be a eCLEntryKind_FlushFrom... constant. */
			void				_LogFlushFromActionWithoutLocking( ECacheLogEntryKind inKind, sLONG inIsWaitUntilDone, sLONG inIsEmptyCache);
	/* @brief	Format the count of seconds since Start() was called */
	inline	void				_FormatEllapsedSeconds();
	/** @brief	Format the infos about the VTask (name, ID, 4D process num) */
	/** @brief	Check the kind of the log entry */
	inline	bool				_IsFlushFromActionEntrykind(ECacheLogEntryKind inKind) const;
	/** @brief	Prepare to insert "start" after the kind of log */
	inline	void				_UpdateStartFlag(bool inWithFlag);
	/** @brief	Format and write the entry header */
			void				_AddEntry(ECacheLogEntryKind inKind, bool inAddReturn = false);
			void				_AddEntryEnd(ECacheLogEntryKind inKind, bool inAddReturn = false);
	//@}
			

	/* Class member variables */
	XBOX::VSplitableLogFile		*fLogFile;
	uLONG						fStarTimeMS;

	XBOX::StStringConverter<char>	fConverter;//<<< uses VTC_StdLib_char
	char						fEllapsedSeconds[512];
	char						fVTaskInfos[512];
	char						fStart[10];

	sLONG						fMemStatsLevel;
};

/** @Class VCacheLogQuotedEntry

	Helper to use VCacheLog in one shot :
		-> If CacheLog is not started, nothing is done
		-> If it is started, the object:
				- Logs the info in its constructor
				- Logs the "end of info" in its destructor
				- with ot without memstats (at begin and end)
				- with ot without timer
*/
class VCacheLogQuotedEntry : public VObject
{
public:
	/**@brief	Common constructor. Calls VCacheLog::Get()->Log(inKind) */
						VCacheLogQuotedEntry(ECacheLogEntryKind inKind, bool inQuoteWithMemStats = false, bool inDoCounter = false);
	/**@brief	Constructor that should be used only by CacheManager / NeedsBytes
				Using const XBOX::VSize& to avoid any ambiguity at runtime with other constructor(s)
	*/
						VCacheLogQuotedEntry(ECacheLogEntryKind inKind, const XBOX::VSize& inNeededBytes, bool inQuoteWithMemStats = false, bool inWithCounter = false);

						~VCacheLogQuotedEntry();

protected:
	// Protected constructor used by childs which don't want the entry to be written immediately,
	// but will write it later
						VCacheLogQuotedEntry();

	// Utilities
		void			_StartCounter();
		sLONG			_StopCounter();


		bool					fLogWasStarted;
		ECacheLogEntryKind		fLogKind;
		bool					fWithCounter;
		XBOX::VMicrosecondsCounter	fCounter;
		bool					fQuoteWithMemStats;

private:
		void			_Reset();
};

/** @Class	VCacheLogEntryFlushFromAction

			To be used when the flush is required from an action (should be a a eCLEntryKind_FlushFrom... constant).
			The class logs the infos with originn of the flush, and open/close tags.

			If inWithmemStats is true in the constructor, mem stats are dumped in constructor and in destructor
			(which means before and after the flush)
*/
class VCacheLogEntryFlushFromAction : public VCacheLogQuotedEntry
{
public:
	/**@brief	Constructor used at "high" level, by part of the code that explicitely ask to flush */
				VCacheLogEntryFlushFromAction(ECacheLogEntryKind inOrigin, bool inIsWaitUntilDone, bool isEmptyMem, bool inQuoteWithMemStats = false);

				~VCacheLogEntryFlushFromAction();
};

#endif	// __CacheLog__
