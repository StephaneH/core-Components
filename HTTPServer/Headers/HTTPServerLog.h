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
#ifndef __HTTP_SERVER_LOG_INCLUDED__
#define __HTTP_SERVER_LOG_INCLUDED__


extern const sLONG			HTTP_SERVER_LOG_MIN_SIZE;
extern const sLONG			HTTP_SERVER_LOG_DEFAULT_SIZE;
extern const XBOX::VString	HTTP_SERVER_LOG_FILE_PATH;
extern const XBOX::VString	HTTP_SERVER_LOG_FILE_EXTENSION;
extern const XBOX::VString	HTTP_SERVER_LOG_FILE_NAME;


class IHTTPResponse;


class VHTTPServerLogSettings : public XBOX::VObject
{
public:
										VHTTPServerLogSettings();
	virtual								~VHTTPServerLogSettings();

	void								LoadDefaultTokens() const;

	EHTTPServerLogFormat				GetLogFormat() const { return fLogFormat; }
	const VectorOfLogToken&				GetLogTokens() const;
	const XBOX::VFilePath&				GetLogPath() const { return fLogFolderPath; }
	const XBOX::VString&				GetLogFileName() const { return fLogFileName; }
	const XBOX::VString&				GetLogFileNameExtension() const { return fLogFileNameExtension; }
	const XBOX::VString&				GetArchivesFolderName() const { return fArchivesFolderName; }

	void								SetLogFormat (EHTTPServerLogFormat inLogFormat, VectorOfLogToken *inLogTokens = NULL);
	void								SetLogTokens (const VectorOfLogToken& inTokens);
	void								SetLogFolderPath (const XBOX::VFilePath& inFolderPath);
	void								SetLogFileName (const XBOX::VString& inValue) { fLogFileName.FromString (inValue); }
	void								SetLogFileNameExtension (const XBOX::VString& inValue) { fLogFileNameExtension.FromString (inValue); }
	void								SetArchivesFolderName (const XBOX::VString& inValue) { fArchivesFolderName.FromString (inValue); }

private:
	EHTTPServerLogFormat				fLogFormat;
	mutable VectorOfLogToken			fLogTokens;
	XBOX::VCriticalSection				fLogTokensVectorLock;
	XBOX::VFilePath						fLogFolderPath;
	XBOX::VString						fLogFileName;
	XBOX::VString						fLogFileNameExtension;
	XBOX::VString						fArchivesFolderName;
};


class VHTTPServerLogBackupSettings : public XBOX::VObject
{
public:
										VHTTPServerLogBackupSettings();
	virtual								~VHTTPServerLogBackupSettings();

	void								ResetToFactorySettings();
	VHTTPServerLogBackupSettings&		operator = (const VHTTPServerLogBackupSettings& inLogBackupSettings);

	bool								RotateOnSchedule() const;

	ELogRotationMode					GetLogRotationMode() const { return fLogRotationMode; }
	sLONG								GetLogMaxSize() const { return fLogMaxSize; }
	sLONG								GetFrequency() const { return fFrequency; }				// Weekly & Monthly backups
	sLONG								GetStartingTime() const { return fStartingTime; }
	const std::map<sLONG, sLONG>&		GetDaysHoursMap () const { return fDaysHours; }

	void								SetLogRotationMode (ELogRotationMode inValue) { fLogRotationMode = inValue; }
	void								SetLogMaxSize (const sLONG inValue);
	void								SetFrequency (sLONG inValue) { fFrequency = inValue; }	// Weekly & Monthly backups
	void								SetStartingTime (sLONG inValue) { fStartingTime = inValue; }
	void								SetDaysHoursMap (const std::map<sLONG, sLONG>& inValue);

	typedef enum
	{
		eFirstDay,
		eSunday = eFirstDay,
		eMonday,
		eTuesday,
		eWednesday,
		eThursday,
		eFriday,
		eSaturday,
		eLastDay = eSaturday
	} EWeekDay;

	/* Used for Weekly Backup */
	bool								GetSaveOnWeekDay( EWeekDay inWeekDay) const;
	sLONG								GetWeekDayBackupHour( EWeekDay inWeekDay) const;

	/* Used for Monthly Backup */
	sLONG								GetMonthDay() const;
	sLONG								GetMonthDayHour() const;

	/* Signals */
	static XBOX::VSignalT_0 *			GetSignal_LogSettingsChanged();
	static void							Tell_LogSettingsChanged();

private:
	ELogRotationMode					fLogRotationMode;
	sLONG								fLogMaxSize;
	sLONG								fFrequency;	// Weekly & Monthly backups
	sLONG								fStartingTime;
	std::map<sLONG, sLONG>				fDaysHours;	// Map of couples of Day/Hours (weekly bakcup) or DayNo/Hours (Monthly Backup)

	/* Signals */
	static XBOX::VSignalT_0				fSignal_LogSettingsChanged;
};


class VHTTPServerLog : public XBOX::VObject, public XBOX::IRefCountable
{
public:
										VHTTPServerLog ();
	virtual 							~VHTTPServerLog();

	VHTTPServerLogSettings&				GetSettings() { return fSettings; }
	VHTTPServerLogBackupSettings&		GetBackupSettings() { return fBackupSettings; }
	void								SetLogFormat (EHTTPServerLogFormat inValue);

	XBOX::VError						LogRequest (const IHTTPResponse& inHTTPResponse);

	const XBOX::VTime&					GetNextFileRotationTime() const { return fNextFileRotationTime; }
	const XBOX::VTime&					GetLastFileRotationTime() const { return fLastFileRotationTime; }

	void								SetBackupSettings (const VHTTPServerLogBackupSettings& inValue) { fBackupSettings = inValue; }

protected:
	XBOX::VError						_WriteFileHeader (XBOX::VString& ioBuffer);
	void								_MakeDateString (const XBOX::VTime& inTime, XBOX::VString& outDateString);
	XBOX::VError						_WriteCLF_DLF (const IHTTPResponse& inHTTPResponse);
	XBOX::VError						_WriteWLF_ELF (const IHTTPResponse& inHTTPResponse);

	void								_GetCurrentFormatedDate (XBOX::VString& outDateString, bool inLocalTime = true, const char inDateSeparator = HTTP_MINUS);
	void								_GetCurrentFormatedTime (XBOX::VString& outTimeString, bool inLocalTime = true);
	void								_WriteUsername (const XBOX::VString& inUserName, XBOX::VString& ioStream);

	XBOX::VError						_Flush();
	XBOX::VError						_RotateFile();
	bool								_NeedFileRotation();
	void								_CalculateNextFileRotationTime();

	void								_GetLogFilePath (XBOX::VFilePath& outFilepath);

	static sLONG						_BackgroundFlushLog (XBOX::VTask *inTask);
	void								_StartBackgroundLogFlusher();
	void								_StopBackgroundLogFlusher();
	bool								_WakeUpBackgroundLogFlusher();
	bool								_WaitForBackgroundLogFlusher();

private:
	VHTTPServerLogSettings				fSettings;
	VHTTPServerLogBackupSettings		fBackupSettings;
	bool								fNeedHeader;
	bool								fNeedSplit;
	sLONG								fRequestsInBuffer;
	XBOX::VString						fRequestsBuffer;
	XBOX::VFile *						fLogFile;
	XBOX::VCriticalSection				fLogFileAccessLock;
	XBOX::VCriticalSection				fLogBufferAccessLock;
	sLONG8								fLogFileSize;
	sLONG8								fLogHeaderSize;
	XBOX::VTask *						fBackgroundFlusherTask;
	XBOX::VCriticalSection				fBackgroundFlusherAccessLock;
	XBOX::VSyncEvent					fBackgroundFlusherSyncEvent;
	
	XBOX::VTime							fNextFileRotationTime;
	XBOX::VTime							fLastFileRotationTime;
};


#endif	// __HTTP_SERVER_LOG_INCLUDED__
