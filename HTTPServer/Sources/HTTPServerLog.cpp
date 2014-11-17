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
#include "HTTPServer.h"
#include "HTTPServerLog.h"


//--------------------------------------------------------------------------------------------------


static const char *	CONST_ABBREVIATED_ENGLISH_MONTH_NAMES[] = {
	"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};


const sLONG			HTTP_SERVER_LOG_MIN_SIZE = 100;			//	in kb
const sLONG			HTTP_SERVER_LOG_DEFAULT_SIZE = 1024;	//	in kb
const sLONG			DEFAULT_MAX_REQUEST_IN_BUFFER = 10;


//--------------------------------------------------------------------------------------------------


static
void GetFormatedDateTimeString (const XBOX::VTime& inTime, XBOX::VString &outDateTimeString)
{
	sLONG8 ms = inTime.GetMilliseconds() - (inTime.GetJulianDay() * 24 * 60 * 60 * 1000);

	outDateTimeString.Clear();
	outDateTimeString.Printf ("D%04d%02d%02dT%08d",
		inTime.GetLocalYear(),
		inTime.GetLocalMonth(),
		inTime.GetLocalDay(),
		ms);
}


/*
 *	Set the Time part of a XBOX::VTime (does NOT touch Date)
 */
static
void SetTimeValue (XBOX::VTime& ioTime, sLONG inDayHour)
{
	sLONG8 timeMilliseconds = 0;

	ioTime.SetLocalHour (0);
	ioTime.SetLocalMinute (0);
	ioTime.SetLocalSecond (0);
	ioTime.SetLocalMillisecond (0);

	timeMilliseconds = ioTime.GetMilliseconds();
	timeMilliseconds += (inDayHour * 1000);
	ioTime.FromMilliseconds (timeMilliseconds);
}


static
XBOX::VError GetFileCreationTime (const XBOX::VFilePath& inFilePath, XBOX::VTime *outCreationTime)
{
	if (NULL == outCreationTime)
		return VE_INVALID_PARAMETER;

	XBOX::VError error = XBOX::VE_MEMORY_FULL;
	XBOX::VFile *file = new XBOX::VFile (inFilePath);
	
	if (NULL != file)
	{
		if (file->Exists())
			error = file->GetTimeAttributes (NULL, outCreationTime);
		else
			error = XBOX::VE_FILE_NOT_FOUND;
		
		XBOX::QuickReleaseRefCountable (file);
	}
	
	return error;
}


//--------------------------------------------------------------------------------------------------


VHTTPServerLogSettings::VHTTPServerLogSettings()
: fLogFormat (LOG_FORMAT_CLF)
, fLogTokens()
, fLogFolderPath()
, fLogFileName (HTTP_SERVER_LOG_FILE_NAME)
, fLogFileNameExtension (HTTP_SERVER_LOG_FILE_EXTENSION)
, fArchivesFolderName()
{
}


VHTTPServerLogSettings::~VHTTPServerLogSettings()
{
	fLogTokens.clear();
}


const VectorOfLogToken& VHTTPServerLogSettings::GetLogTokens() const
{
	if (fLogTokens.empty())
		LoadDefaultTokens();

	return fLogTokens;
}


void VHTTPServerLogSettings::SetLogFormat (EHTTPServerLogFormat inLogFormat, VectorOfLogToken *inTokens)
{
	if (inLogFormat != fLogFormat)
	{
		fLogFormat = inLogFormat;

		if ((inLogFormat == LOG_FORMAT_ELF) || (inLogFormat == LOG_FORMAT_WLF))
		{
			if (NULL != inTokens)
			{
				SetLogTokens (*inTokens);
			}
			else
			{
				LoadDefaultTokens();
			}
		}
	}
}


void VHTTPServerLogSettings::SetLogTokens (const VectorOfLogToken& inTokens)
{
	XBOX::VTaskLock lock (&fLogTokensVectorLock);
	fLogTokens.clear();
	fLogTokens.assign (inTokens.begin(), inTokens.end());
}


void VHTTPServerLogSettings::SetLogFolderPath (const XBOX::VFilePath& inFolderPath)
{
	fLogFolderPath.FromFilePath (inFolderPath);
}


void VHTTPServerLogSettings::LoadDefaultTokens() const
{
	HTTPServerTools::GetDefaultLogTokenList (fLogFormat, fLogTokens);
}


//--------------------------------------------------------------------------------------------------


XBOX::VSignalT_0	VHTTPServerLogBackupSettings::fSignal_LogSettingsChanged (XBOX::VSignal::ESM_Asynchonous);


VHTTPServerLogBackupSettings::VHTTPServerLogBackupSettings()
: fLogRotationMode (LRC_ROTATE_ON_FILE_SIZE)
, fLogMaxSize (HTTP_SERVER_LOG_DEFAULT_SIZE)
, fFrequency (0)
, fStartingTime (0)
, fDaysHours()
{
}


VHTTPServerLogBackupSettings::~VHTTPServerLogBackupSettings()
{
	fDaysHours.clear();
}


void VHTTPServerLogBackupSettings::ResetToFactorySettings()
{
	SetLogRotationMode (LRC_ROTATE_ON_FILE_SIZE);
	SetLogMaxSize (HTTP_SERVER_LOG_DEFAULT_SIZE);
	SetFrequency (0);
	SetStartingTime (0);
	fDaysHours.clear();
}


VHTTPServerLogBackupSettings& VHTTPServerLogBackupSettings::operator = (const VHTTPServerLogBackupSettings& inValue)
{
	if (&inValue != this)
	{
		SetLogRotationMode (inValue.fLogRotationMode);
		SetLogMaxSize (inValue.fLogMaxSize);
		SetFrequency (inValue.fFrequency);
		SetStartingTime (inValue.fStartingTime);
		SetDaysHoursMap (inValue.fDaysHours);
	}

	return *this;
}


bool VHTTPServerLogBackupSettings::RotateOnSchedule() const
{
	if ((fLogRotationMode >= LRC_ROTATE_EVERY_HOUR) && (fLogRotationMode <= LRC_ROTATE_EVERY_MONTH))
		return true;

	return false;
}


void VHTTPServerLogBackupSettings::SetLogMaxSize (const sLONG inValue)
{
	if (inValue <= 0)
		fLogMaxSize = 0;
	else
		fLogMaxSize = ((inValue > 0) && (inValue < HTTP_SERVER_LOG_MIN_SIZE)) ? HTTP_SERVER_LOG_MIN_SIZE : inValue;
}


void VHTTPServerLogBackupSettings::SetDaysHoursMap (const std::map<sLONG, sLONG>& inValue)
{
	fDaysHours.insert (inValue.begin(), inValue.end());
}


bool VHTTPServerLogBackupSettings::GetSaveOnWeekDay (EWeekDay inWeekDay) const
{
	if ((inWeekDay >= eFirstDay) && (inWeekDay <= eLastDay))
		return fDaysHours.find ((sLONG)inWeekDay) != fDaysHours.end();

	return false;
}


sLONG VHTTPServerLogBackupSettings::GetWeekDayBackupHour (EWeekDay inWeekDay) const
{
	if ((inWeekDay >= eFirstDay) && (inWeekDay <= eLastDay))
	{
		std::map<sLONG, sLONG>::const_iterator it = fDaysHours.find ((sLONG)inWeekDay);

		if (it != fDaysHours.end())
			return (*it).second;
	}

	return 0;
}


sLONG VHTTPServerLogBackupSettings::GetMonthDay() const
{
	if (fDaysHours.size() == 1)
		return (*fDaysHours.begin()).first;

	return 0;
}


sLONG VHTTPServerLogBackupSettings::GetMonthDayHour() const
{
	if (fDaysHours.size() == 1)
		return (*fDaysHours.begin()).second;

	return 0;
}


/* static */
XBOX::VSignalT_0 *VHTTPServerLogBackupSettings::GetSignal_LogSettingsChanged()
{
	return &fSignal_LogSettingsChanged;
}


/* static */
void VHTTPServerLogBackupSettings::Tell_LogSettingsChanged()
{
	fSignal_LogSettingsChanged();
}


//--------------------------------------------------------------------------------------------------

sLONG VHTTPServerLog::fMaxRequestsInBuffer = DEFAULT_MAX_REQUEST_IN_BUFFER;


VHTTPServerLog::VHTTPServerLog ()
: fSettings()
, fNeedHeader (true)
, fNeedSplit (false)
, fRequestsInBuffer (0)
, fRequestsBuffer ()
, fLogFile (NULL)
, fLogBufferAccessLock()
, fLogFileAccessLock()
, fLogFileSize (0)
, fBackgroundFlusherAccessLock()
, fBackgroundFlusherTask (NULL)
, fBackgroundFlusherSyncEvent ()
, fNextFileRotationTime()
, fLastFileRotationTime()
{
}


VHTTPServerLog::~VHTTPServerLog()
{
	_StopBackgroundLogFlusher();
	xbox_assert (NULL == fBackgroundFlusherTask);

	if (!fRequestsBuffer.IsEmpty())
		_Flush();

	XBOX::ReleaseRefCountable (&fLogFile);
}


bool VHTTPServerLog::_NeedFileRotation()
{
	if (fBackupSettings.GetLogRotationMode() == LRC_NO_ROTATION)
		return false;

	if (fNeedSplit)
		return true;

	if (fBackupSettings.RotateOnSchedule() && (fNextFileRotationTime.GetStamp() > 0))
	{
		XBOX::VTime currentTime;
		currentTime.FromSystemTime();

		if (fNextFileRotationTime.GetStamp() < currentTime.GetStamp())
			return true;
	}

	return false;
}


/* static */
sLONG VHTTPServerLog::_BackgroundFlushLog (XBOX::VTask *inTask)
{
	if (NULL == inTask)
		return -1;

	VHTTPServerLog *log = (VHTTPServerLog *)(inTask->GetKindData());

	if (NULL != log)
	{
#if LOG_IN_CONSOLE
		VDebugTimer timer;
#endif

		while ((XBOX::TS_DYING != inTask->GetState()) && (XBOX::TS_DEAD != inTask->GetState()))
		{
			if ((log->fRequestsInBuffer >= VHTTPServerLog::GetMaxRequestsInBuffer()) || log->_NeedFileRotation())
			{
				if (log->_NeedFileRotation())
					log->_RotateFile();

				if (log->fRequestsInBuffer >= VHTTPServerLog::GetMaxRequestsInBuffer())
					log->_Flush();

			}

#if LOG_IN_CONSOLE
			timer.DebugMsg ("\tElapsed time since last flush: ");
			timer.Reset();
#endif

			if (XBOX::TS_RUNNING == inTask->GetState())
				log->_WaitForBackgroundLogFlusher();
		}
	}

	return 0;
}


void VHTTPServerLog::_StartBackgroundLogFlusher()
{
	if (fBackgroundFlusherAccessLock.Lock())
	{
		if (NULL == fBackgroundFlusherTask)
		{
			fBackgroundFlusherTask = new XBOX::VTask (this, 0, XBOX::eTaskStylePreemptive, &_BackgroundFlushLog);
			fBackgroundFlusherTask->SetName (CVSTR ("HTTP Server Log Flush"));
			fBackgroundFlusherTask->SetKindData ((sLONG_PTR)this);
			fBackgroundFlusherTask->Run();

		}
		fBackgroundFlusherAccessLock.Unlock();
	}
}


void VHTTPServerLog::_StopBackgroundLogFlusher()
{
	if (NULL == fBackgroundFlusherTask)
		return;

	if (fBackgroundFlusherAccessLock.Lock())
	{
		if (NULL != fBackgroundFlusherTask)
		{
			if (fBackgroundFlusherTask->GetState() >= XBOX::TS_RUNNING)
			{
				fBackgroundFlusherTask->Kill();
				fBackgroundFlusherTask->StopMessaging();
			}

			_WakeUpBackgroundLogFlusher();

			fBackgroundFlusherTask->Release();
			XBOX::VInterlocked::ExchangePtr<XBOX::VTask> (&fBackgroundFlusherTask, NULL);
		}

		fBackgroundFlusherAccessLock.Unlock();
	}
}


bool VHTTPServerLog::_WakeUpBackgroundLogFlusher()
{
	return fBackgroundFlusherSyncEvent.Unlock();
}


bool VHTTPServerLog::_WaitForBackgroundLogFlusher()
{
	if (fBackgroundFlusherSyncEvent.Reset())
		return fBackgroundFlusherSyncEvent.Lock();

	return false;
}


void VHTTPServerLog::SetLogFormat (EHTTPServerLogFormat inValue)
{
	fNeedHeader = false;

	if ((inValue >= LOG_FORMAT_CLF) && (inValue <= LOG_FORMAT_WLF))
	{
		if (fSettings.GetLogFormat() != inValue)
		{
			fNeedHeader = true;
			fSettings.SetLogFormat (inValue);
		}
	}
	else
	{
		fSettings.SetLogFormat (LOG_FORMAT_NO_LOG);
	}
}


XBOX::VError VHTTPServerLog::LogRequest (const IHTTPResponse& inHTTPResponse)
{
	XBOX::VError error = XBOX::VE_OK;

	if (fSettings.GetLogFormat() != LOG_FORMAT_NO_LOG)
	{
		if (NULL == fBackgroundFlusherTask)
			_StartBackgroundLogFlusher();

		sLONG			bufferSize = 0;

		if (fLogBufferAccessLock.Lock())
		{
			if ((0 == fRequestsInBuffer) && fNeedHeader)
				_WriteFileHeader (fRequestsBuffer);

			if ((fSettings.GetLogFormat() == LOG_FORMAT_CLF) || (fSettings.GetLogFormat() == LOG_FORMAT_DLF))
			{
				error = _WriteCLF_DLF (inHTTPResponse);
			}
			else 
			{
				error = _WriteWLF_ELF (inHTTPResponse);
			}

			bufferSize = fRequestsBuffer.GetLength();

			fLogBufferAccessLock.Unlock();
		}

		if (fBackupSettings.GetLogRotationMode() == LRC_ROTATE_ON_FILE_SIZE)
			fNeedSplit = ((fBackupSettings.GetLogMaxSize() > 0) && (sLONG8(fLogFileSize + bufferSize) > (sLONG8)(fBackupSettings.GetLogMaxSize() * 1024)));

		if ((++fRequestsInBuffer >= VHTTPServerLog::GetMaxRequestsInBuffer()) || fNeedSplit)
			_WakeUpBackgroundLogFlusher();
	}

	return error;
}


/* static */
void VHTTPServerLog::SetMaxRequestsInBuffer(sLONG inValue)
{
	fMaxRequestsInBuffer = (inValue > 0) ? inValue : DEFAULT_MAX_REQUEST_IN_BUFFER;
}


XBOX::VError VHTTPServerLog::_Flush()
{
	XBOX::VFilePath	logFilePath;
	XBOX::VError	error = XBOX::VE_OK;

	_GetLogFilePath (logFilePath);

	if (!logFilePath.IsValid())
		return XBOX::VE_FILE_NOT_FOUND;

	if (fLogFileAccessLock.Lock())
	{
		if (NULL == fLogFile)
			fLogFile = new XBOX::VFile (logFilePath);

		if (testAssert (NULL != fLogFile))
		{
			XBOX::VFileDesc *fileDesc = NULL;

			error = fLogFile->Open (XBOX::FA_READ_WRITE, &fileDesc, XBOX::FO_CreateIfNotFound);

			if ((XBOX::VE_OK == error) && (NULL != fileDesc))
			{
				sLONG8 logFileSize = fileDesc->GetSize();

				if ((CHAR_NUMBER_SIGN != fRequestsBuffer.GetUniChar (1)) && (0 == logFileSize))
				{
					XBOX::VString fileHeader;
					_WriteFileHeader (fileHeader);

					XBOX::StStringConverter<char> buffer (fileHeader, XBOX::VTC_DefaultTextExport);
					fileDesc->SetPos (fileDesc->GetSize());
					error = fileDesc->PutDataAtPos (buffer.GetCPointer(), buffer.GetSize());
					fileDesc->Flush();
				}

				if (fLogBufferAccessLock.Lock())
				{
					// Flushing the buffer
					XBOX::StStringConverter<char> buffer (fRequestsBuffer, XBOX::VTC_DefaultTextExport);
					fileDesc->SetPos (fileDesc->GetSize());
					error = fileDesc->PutDataAtPos (buffer.GetCPointer(), buffer.GetSize());

					fRequestsBuffer.Clear();
					fRequestsInBuffer = 0;

					fLogBufferAccessLock.Unlock();
				}

				error = fileDesc->Flush();
				fLogFileSize = fileDesc->GetSize();
				delete fileDesc;
				fileDesc = NULL;
			}
			else
			{
				error = VE_CANNOT_OPEN_LOG_FILE;
			}
		}
		else
		{
			error = VE_CANNOT_CREATE_LOG_FILE;
		}

		fLogFileAccessLock.Unlock();

		_CalculateNextFileRotationTime();
	}

	return error;
}


XBOX::VError VHTTPServerLog::_RotateFile()
{
	XBOX::VError	error = XBOX::VE_OK;
	XBOX::VFilePath	logFilePath;
	XBOX::VTime		curTime;

	curTime.FromSystemTime();

	_GetLogFilePath (logFilePath);

	if (fLogFileAccessLock.Lock())
	{
		if (NULL == fLogFile)
			fLogFile = new XBOX::VFile (logFilePath);

		if (fLogFile->Exists())
		{
			// the backup fileName will look like "HTTPServer_DAAAAMMJJT00000000.waLog" => "HTTPServer_D20100122T113307.waLog"
			// I used this format to have the files sorted by time thanks to their name

			XBOX::VString	newFileName;
			XBOX::VString	dateTimeString;

			GetFormatedDateTimeString (curTime, dateTimeString);

			fLogFile->GetNameWithoutExtension (newFileName);
			newFileName.AppendUniChar (CHAR_LOW_LINE);
			newFileName.AppendString (dateTimeString);
			newFileName.AppendUniChar (CHAR_FULL_STOP);
			newFileName.AppendString (fSettings.GetLogFileNameExtension());

			if (fSettings.GetArchivesFolderName().IsEmpty())
			{
				error = fLogFile->Rename (newFileName);
			}
			else
			{
				XBOX::VFilePath	destinationFilePath;

				fLogFile->GetPath (destinationFilePath);
				destinationFilePath.ToParent().ToSubFolder (fSettings.GetArchivesFolderName());

				if (destinationFilePath.IsFolder())
				{
					XBOX::VFolder *	destinationFolder = new XBOX::VFolder (destinationFilePath);
					if (NULL != destinationFolder)
					{
						if (!destinationFolder->Exists())
							error = destinationFolder->CreateRecursive();
						XBOX::ReleaseRefCountable (&destinationFolder);

						if (XBOX::VE_OK == error)
						{
							destinationFilePath.SetFileName (newFileName);

							error = fLogFile->Move (destinationFilePath, NULL, XBOX::FCP_Overwrite);
						}
					}
					else
					{
						error = XBOX::VE_MEMORY_FULL;
					}
				}
			}

			XBOX::ReleaseRefCountable (&fLogFile);
		}

		//Creating the log file
		if ((XBOX::VE_OK == error) && (NULL == fLogFile))
		{
			fLogFile = new XBOX::VFile (logFilePath);
			if (NULL != fLogFile)
			{
				error = fLogFile->Create();

				// Set correct dates & times (used later to determine the next backup time)
				if (XBOX::VE_OK == error)
					error = fLogFile->SetTimeAttributes (&curTime, &curTime, &curTime);
				fLogFileSize = 0;
				
				fLastFileRotationTime.FromTime (curTime);
				_CalculateNextFileRotationTime();
			}
			else
			{
				error = VE_CANNOT_CREATE_LOG_FILE;
			}
		}
		else
		{
			error = VE_CANNOT_OPEN_LOG_FILE;
		}

		fNeedSplit = false;
		fLogFileAccessLock.Unlock();
	}

	return error;
}


void VHTTPServerLog::_GetLogFilePath (XBOX::VFilePath& outFilepath)
{
	XBOX::VFilePath logFolderPath (fSettings.GetLogPath());

	if (logFolderPath.IsValid() && logFolderPath.IsFolder())
	{
		XBOX::VFolder folder (fSettings.GetLogPath());

		if (!folder.Exists())
			folder.CreateRecursive();
	}

	outFilepath.FromFilePath (fSettings.GetLogPath());
	outFilepath.SetFileName (fSettings.GetLogFileName());
}


XBOX::VError VHTTPServerLog::_WriteFileHeader (XBOX::VString& ioBuffer)
{
	/*
		see: http://www.w3.org/TR/WD-logfile.html
	*/
	XBOX::VString	serverName;
	XBOX::VString	dateString;
	XBOX::VString	formatName;
	XBOX::VString	tokenNames;

	fNeedHeader = false;
	/*
		#Version: 1.0
		#Date: 12-Jan-1996 00:00:00
	*/
	HTTPProtocol::MakeServerString (serverName);
	_GetCurrentFormatedDate (dateString);
	HTTPServerTools::GetLogFormatName (fSettings.GetLogFormat(), formatName);
	HTTPServerTools::GetLogTokenNamesList (fSettings.GetLogTokens(), tokenNames);

	ioBuffer.AppendCString ("#Version: 1.0\n#Software: ");
	ioBuffer.AppendString (serverName);
	ioBuffer.AppendCString ("\n#Date: ");
	ioBuffer.AppendString (dateString);

	ioBuffer.AppendCString ("\n#Remark: format ");
	ioBuffer.AppendString (formatName);

	ioBuffer.AppendCString ("\n#Fields: ");
	ioBuffer.AppendString (tokenNames);

	ioBuffer.AppendUniChar (HTTP_LF);

	return XBOX::VE_OK;
}


void VHTTPServerLog::_MakeDateString (const XBOX::VTime& inTime, XBOX::VString& outDateString)
{
	/*	See description: http://httpd.apache.org/docs/2.2/logs.html#common

	[day/month/year:hour:minute:second zone]
	day = 2*digit
	month = 3*letter
	year = 4*digit
	hour = 2*digit
	minute = 2*digit
	second = 2*digit
	zone = (`+' | `-') 4*digit
	*/

	sWORD			year = 0, month = 0, day = 0, hour = 0, minute = 0, seconds = 0, milliseconds = 0;
	static sLONG	sGMTOffSet = 0;
	static bool		sGMTOffSetUndefined = true;

	inTime.GetUTCTime (year, month, day, hour, minute, seconds, milliseconds);

	if (sGMTOffSetUndefined)
	{
		sGMTOffSet = (XBOX::VSystem::GetGMTOffset (true) / (3600));
		sGMTOffSetUndefined = false;
	}

	outDateString.Clear();
	outDateString.Printf (	"%02d/%s/%04d:%02d:%02d:%02d %c%02ld00",
		day,
		CONST_ABBREVIATED_ENGLISH_MONTH_NAMES[month],
		year,
		hour,
		minute,
		seconds,
		(sGMTOffSet >= 0) ? '+' : '-',
		sGMTOffSet);
}


XBOX::VError VHTTPServerLog::_WriteCLF_DLF (const IHTTPResponse& inHTTPResponse)
{
	XBOX::VString	string;
	XBOX::VTime		time;

	// GMT time
	time.FromSystemTime();

	// Client IP address
	string.FromString (inHTTPResponse.GetRequest().GetPeerIP());
	
	fRequestsBuffer.AppendString (string);
	fRequestsBuffer.AppendUniChar (CHAR_SPACE);
	
	// RFC931
	fRequestsBuffer.AppendUniChar (CHAR_HYPHEN_MINUS);
	fRequestsBuffer.AppendUniChar (CHAR_SPACE);
	
	// AuthUser
	XBOX::VString userName;
	inHTTPResponse.GetRequest().GetAuthenticationInfos()->GetUserName (userName);
	
	if (userName.IsEmpty())
		fRequestsBuffer.AppendUniChar (CHAR_HYPHEN_MINUS);
	else
		fRequestsBuffer.AppendString (userName);
	fRequestsBuffer.AppendUniChar (CHAR_SPACE);

	// Date & Time
	string.Clear();
	_MakeDateString (time, string);

	fRequestsBuffer.AppendUniChar (CHAR_LEFT_SQUARE_BRACKET);
	fRequestsBuffer.AppendString (string);
	fRequestsBuffer.AppendUniChar (CHAR_RIGHT_SQUARE_BRACKET);
	fRequestsBuffer.AppendUniChar (CHAR_SPACE);

	// HTTP Request
	fRequestsBuffer.AppendUniChar (CHAR_QUOTATION_MARK);
	fRequestsBuffer.AppendString (inHTTPResponse.GetRequest().GetRequestLine());
	fRequestsBuffer.AppendUniChar (CHAR_QUOTATION_MARK);
	fRequestsBuffer.AppendUniChar (CHAR_SPACE);
	
	// HTTP Status
	string.Clear();
	string.FromLong ((sLONG)inHTTPResponse.GetResponseStatusCode());
	fRequestsBuffer.AppendString (string);
	fRequestsBuffer.AppendUniChar (CHAR_SPACE);

	// HTTP Content Length
	string.Clear();
	if (!inHTTPResponse.GetResponseHeader (STRING_HEADER_CONTENT_LENGTH, string) || string.IsEmpty())
		string.FromLong(0);
	fRequestsBuffer.AppendString (string);
	
	// We add the last fields for the DLF log format
	if (fSettings.GetLogFormat() == LOG_FORMAT_DLF)
	{	
		// HTTP Referrer
		string.Clear();
		fRequestsBuffer.AppendUniChar (CHAR_SPACE);
		inHTTPResponse.GetRequest().GetHTTPHeaders().GetHeaderValue (STRING_HEADER_REFERER, string);
		fRequestsBuffer.AppendUniChar (CHAR_QUOTATION_MARK);
		fRequestsBuffer.AppendString (string);
		fRequestsBuffer.AppendUniChar (CHAR_QUOTATION_MARK);
		fRequestsBuffer.AppendUniChar (CHAR_SPACE);
		
		// HTTP User-Agent
		string.Clear();
		inHTTPResponse.GetRequest().GetHTTPHeaders().GetHeaderValue (STRING_HEADER_USER_AGENT, string);
		fRequestsBuffer.AppendUniChar (CHAR_QUOTATION_MARK);
		fRequestsBuffer.AppendString (string);
		fRequestsBuffer.AppendUniChar (CHAR_QUOTATION_MARK);
	}

	fRequestsBuffer.AppendUniChar (HTTP_LF);

	return XBOX::VE_OK;
}


XBOX::VError VHTTPServerLog::_WriteWLF_ELF (const IHTTPResponse& inHTTPResponse)
{
	XBOX::VString				string;
	XBOX::VString				ipAddress;
	const VectorOfLogToken		tokens = fSettings.GetLogTokens();
	const EHTTPServerLogFormat	format = fSettings.GetLogFormat();

	for (VectorOfLogToken::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
	{
		switch (*it)
		{
			case LOG_TOKEN_DATE:
				string.Clear();
				if (format == LOG_FORMAT_ELF)
				{
					_GetCurrentFormatedDate (string, false);
				}
				else
				{
					_GetCurrentFormatedDate (string, true, HTTP_SOLIDUS);
				}

				if (!string.IsEmpty())
				{
					fRequestsBuffer.AppendString (string);					
				}
				else
				{
					fRequestsBuffer.AppendUniChar (CHAR_HYPHEN_MINUS);
				}
				break;

			case LOG_TOKEN_TIME:
				string.Clear();
				if (format == LOG_FORMAT_WLF)
					_GetCurrentFormatedTime (string, true);				
				else
					_GetCurrentFormatedTime (string, false);
				fRequestsBuffer.AppendString (string);	
				break;

			case LOG_TOKEN_STATUS:
				string.FromLong ((sLONG)inHTTPResponse.GetResponseStatusCode());
				fRequestsBuffer.AppendString (string);
				break;

			case LOG_TOKEN_ELF_S_IP:
				string.FromString (inHTTPResponse.GetRequest().GetPeerIP());
				fRequestsBuffer.AppendString (string);
				break;

			case LOG_TOKEN_HOST_NAME:	//	= C_DNS .....
			case LOG_TOKEN_ELF_C_DNS:	//	DNS lookup : tres couteux en perf : remplacé par l'IP du client (les analyseurs de log font le DNS lookup)...
			case LOG_TOKEN_ELF_C_IP:	//	Client IP Address 192.0.1.3
				string.FromString (inHTTPResponse.GetRequest().GetPeerIP());
				fRequestsBuffer.AppendString (string);
				break;

			case LOG_TOKEN_METHOD:	// The HTTP method : GET HEAD POST. If Unknown, we just copy it
				string.Clear();
				HTTPProtocol::MakeHTTPMethodString (inHTTPResponse.GetRequest().GetRequestMethod(), string);
				fRequestsBuffer.AppendString (string);
				break;

			case LOG_TOKEN_BYTES_SENT:	//WLF : Bytes sent to the client : = HTTP Content Length
				string.Clear();
				if (inHTTPResponse.GetResponseHeader (STRING_HEADER_CONTENT_LENGTH, string) && !string.IsEmpty())
					fRequestsBuffer.AppendString (string);
				else
					fRequestsBuffer.AppendUniChar (CHAR_DIGIT_ZERO);
				break;

			case LOG_TOKEN_AGENT:	// The identity of the browser software or other client. Mozilla/4.04_(Macintosh;_U;_PPC)
				string.Clear();
				if (inHTTPResponse.GetRequest().GetHTTPHeaders().GetHeaderValue (STRING_HEADER_USER_AGENT, string) && !string.IsEmpty())
				{
					string.Exchange (CHAR_SPACE, CHAR_LOW_LINE);
					fRequestsBuffer.AppendString (string);
				}
				else
				{
					fRequestsBuffer.AppendUniChar (CHAR_HYPHEN_MINUS);
				}
				break;

			case LOG_TOKEN_CS_USER_AGENT:	// HTTP request's "User-Agent" header field. "Mozilla/4.04 (Macintosh; U; PPC)"
				string.Clear();
				inHTTPResponse.GetRequest().GetHTTPHeaders().GetHeaderValue (STRING_HEADER_USER_AGENT, string);
				fRequestsBuffer.AppendUniChar (CHAR_QUOTATION_MARK);
				fRequestsBuffer.AppendString (string);
				fRequestsBuffer.AppendUniChar (CHAR_QUOTATION_MARK);
				break;

			case LOG_TOKEN_USER:	//The User Name if there was a Web User entry for a realm.
				string.Clear();
				inHTTPResponse.GetRequest().GetAuthenticationInfos()->GetUserName (string);
				_WriteUsername (string, fRequestsBuffer);
				break;

			case LOG_TOKEN_REFERER: //HTTP request's "Referer" header field, sending  the URL that referred to the current page. www.google.com
				string.Clear();
				inHTTPResponse.GetRequest().GetHTTPHeaders().GetHeaderValue (STRING_HEADER_REFERER, string);
				if (!string.IsEmpty())
					fRequestsBuffer.AppendString (string);
				else
					fRequestsBuffer.AppendUniChar (CHAR_HYPHEN_MINUS);
				break;

			case LOG_TOKEN_CS_REFERER:	//HTTP request's "Referer" header field, sending  the URL that referred to the current page.  www.google.com
				string.Clear();
				inHTTPResponse.GetRequest().GetHTTPHeaders().GetHeaderValue (STRING_HEADER_REFERER, string);
				fRequestsBuffer.AppendUniChar (CHAR_QUOTATION_MARK);
				fRequestsBuffer.AppendString (string);
				fRequestsBuffer.AppendUniChar (CHAR_QUOTATION_MARK);
				break;

			case LOG_TOKEN_ELF_CS_HOST: // = LOG_TOKEN_HOSTFIELD. The "HOST" field of the HTTP request
				fRequestsBuffer.AppendUniChar (CHAR_QUOTATION_MARK);
				fRequestsBuffer.AppendString (inHTTPResponse.GetRequest().GetHost());
				fRequestsBuffer.AppendUniChar (CHAR_QUOTATION_MARK);
				break;

			case LOG_TOKEN_ELF_URI:
				string.FromString (inHTTPResponse.GetRequest().GetURL());
				if (!string.IsEmpty())
					fRequestsBuffer.AppendString (string);
				else
					fRequestsBuffer.AppendUniChar (CHAR_HYPHEN_MINUS);
				break;

			case LOG_TOKEN_URL:
			case LOG_TOKEN_ELF_CS_URI_STEM:	//	Path portion of the HTTP request. "/status/stat.html"
				string.FromString (inHTTPResponse.GetRequest().GetURLPath());
				if (!string.IsEmpty())
					fRequestsBuffer.AppendString (string);
				else
					fRequestsBuffer.AppendUniChar (CHAR_HYPHEN_MINUS);
				break;

			case LOG_TOKEN_SEARCH_ARGS:	//	The search arguments to the URL (text after a question  mark)
				string.FromString (inHTTPResponse.GetRequest().GetURLQuery());
				fRequestsBuffer.AppendString (string);
				break;

			case LOG_TOKEN_ELF_CS_URI_QUERY:	// 	Search argument portion of the HTTP request. "first=last&last=first"
				string.FromString (inHTTPResponse.GetRequest().GetURLQuery());
				if (!string.IsEmpty())
					fRequestsBuffer.AppendString (string);
				else
					fRequestsBuffer.AppendUniChar (CHAR_HYPHEN_MINUS);
				break;

			case LOG_TOKEN_CONNECTION_ID:	// A number that is unique for each connection for this invocation of the server. Typically socket number.
			{
				sLONG rawSocket = inHTTPResponse.GetRawSocket();
				if (rawSocket > 0)
				{
					fRequestsBuffer.AppendLong (rawSocket);
				}
				else
				{
					fRequestsBuffer.AppendUniChar (CHAR_HYPHEN_MINUS);
				}
				break;
			}
			case LOG_TOKEN_ELF_CS_COOKIE:	// The "cookie" information sent in this request "Set-Cookie: CUSTOMER=WILE_E_COYOTE; path=/; expires=Wednesday, 20-Jan-05 23:12:40 GMT"
				string.Clear();
				inHTTPResponse.GetRequest().GetHTTPHeaders().GetHeaderValue (STRING_HEADER_COOKIE, string);
				fRequestsBuffer.AppendUniChar (CHAR_QUOTATION_MARK);
				fRequestsBuffer.AppendString (string);
				fRequestsBuffer.AppendUniChar (CHAR_QUOTATION_MARK);
				break;

			case LOG_TOKEN_TRANSFER_TIME:	// Time-Taken in millisecond like IIS
			{
				uLONG timeTaken = (XBOX::VSystem::GetCurrentTime() - inHTTPResponse.GetStartRequestTime());
				fRequestsBuffer.AppendLong (timeTaken);
				break;
			}
			case LOG_TOKEN_WLF_BYTES_RECEIVED:
			{
				sLONG8 bytesReceived = inHTTPResponse.GetRequest().GetRequestBody().GetDataSize();
				if (bytesReceived > 0)
				{
					fRequestsBuffer.AppendLong8 (bytesReceived);
				}
				else
				{
					fRequestsBuffer.AppendUniChar (CHAR_DIGIT_ZERO);
				}
				break;
			}

			case LOG_TOKEN_PATH_ARGS: //The path arguments to the URL for a CGI (the text after a dollar sign)
			{
				sLONG			posChar = 0;
				XBOX::VString	pathArgString;
				
				string.FromString (inHTTPResponse.GetRequest().GetURL());

				if (!string.IsEmpty() && (posChar = string.FindUniChar (CHAR_DOLLAR_SIGN)) > 0)
				{
					// Let's delete all the stuff before '$'
					string.GetSubString (posChar + 1, string.GetLength() - posChar, pathArgString);

					// We delete the query arguments after the ? : we only want the string after the '$'
					if ((posChar = pathArgString.FindUniChar (CHAR_QUESTION_MARK)) > 0)
						pathArgString.SubString (1, posChar - 1);
				}

				if (!pathArgString.IsEmpty())
				{
					fRequestsBuffer.AppendString (pathArgString);
				}
				else
				{
					fRequestsBuffer.AppendUniChar (CHAR_HYPHEN_MINUS);
				}
				break;
			}

			default:	// this should never happen.
				assert (false);
				fRequestsBuffer.AppendCString ("UNKNOWN_FIELD"); 
				break;
		}

		VectorOfLogToken::const_iterator nextToken = it;
		if (++nextToken != tokens.end())
			fRequestsBuffer.AppendUniChar (CHAR_SPACE);
	}

	fRequestsBuffer.AppendUniChar (HTTP_LF);

	return XBOX::VE_OK;
}


void VHTTPServerLog::_GetCurrentFormatedDate (XBOX::VString& outDateString, bool inLocalTime, const char inDateSeparator)
{
	sWORD		year, month, day, hour, minute, seconds, milliseconds;
	XBOX::VTime	time;
	
	time.FromSystemTime();	// GMT time
	if (inLocalTime)
		time.GetLocalTime (year, month, day, hour, minute, seconds, milliseconds);
	else
		time.GetUTCTime (year, month, day, hour, minute, seconds, milliseconds);
	
	outDateString.Clear();
	outDateString.Printf ("%04d%c%02d%c%02d %02d:%02d:%02d",	year, inDateSeparator,
																month, inDateSeparator, day,
																hour, minute, seconds);
}


void VHTTPServerLog::_GetCurrentFormatedTime (XBOX::VString& outTimeString, bool inLocalTime)
{
	sWORD		year, month, day, hour, minute, seconds, milliseconds;
	XBOX::VTime	time;
	
	time.FromSystemTime();	// GMT time
	if (inLocalTime)
		time.GetLocalTime (year, month, day, hour, minute, seconds, milliseconds);
	else
		time.GetUTCTime (year, month, day, hour, minute, seconds, milliseconds);
	
	outTimeString.Clear();
	outTimeString.Printf ("%02d:%02d:%02d", hour, minute, seconds);
}


void VHTTPServerLog::_WriteUsername (const XBOX::VString& inUserName, XBOX::VString& ioStream)
{
	if (!inUserName.IsEmpty())
	{
		bool			wasCtrl = false;
		XBOX::VString	userName;

		for (XBOX::VIndex pos = 0; pos < inUserName.GetLength(); ++pos)
		{
			// When user name contains CHAR_SPACE, replace it by CHAR_LOW_LINE '_'
			if ((inUserName[pos] <= CHAR_SPACE) && (inUserName[pos] > 0))
			{
				if (!wasCtrl)
					userName.AppendUniChar (CHAR_LOW_LINE);
				wasCtrl = (inUserName[pos] != CHAR_SPACE);
			}
			else
			{
				userName.AppendUniChar (inUserName[pos]);
				wasCtrl = false;
			}
		}

		ioStream.AppendString (userName);
	}
	else
	{
		ioStream.AppendUniChar (CHAR_HYPHEN_MINUS);
	}
}


void VHTTPServerLog::_CalculateNextFileRotationTime()
{
	fNextFileRotationTime.Clear();

	if (fBackupSettings.RotateOnSchedule())
	{
		XBOX::VTime		curTime;
		XBOX::VDuration tempDuration;
		XBOX::VFilePath logPath;

		_GetLogFilePath (logPath);
		fNextFileRotationTime.FromSystemTime();
		curTime.FromSystemTime();

		if (GetFileCreationTime (logPath, &fLastFileRotationTime) != XBOX::VE_OK)
		{
			fLastFileRotationTime.FromSystemTime();
			fLastFileRotationTime.AddDays (-1);
		}

		switch (fBackupSettings.GetLogRotationMode())
		{
			case LRC_NO_ROTATION:
			case LRC_ROTATE_ON_FILE_SIZE:
				fNextFileRotationTime.Clear();
				break;
				
			case LRC_ROTATE_EVERY_HOUR:
			{				
				// I will first get the date of the last backup, or the creation date
				fNextFileRotationTime = fLastFileRotationTime;	
				
				
				// if the date (dd/mm/yy) is different from the curent date, we have to backup to the current date
				if ((fNextFileRotationTime.GetLocalDay()!=curTime.GetLocalDay())||(fNextFileRotationTime.GetLocalMonth()!=curTime.GetLocalMonth())||(fNextFileRotationTime.GetLocalYear()!=curTime.GetLocalYear()))
				{
					// we use the date of today 
					fNextFileRotationTime.FromSystemTime();	
				}
				
				// set the time to the "starting at" value
				SetTimeValue (fNextFileRotationTime, fBackupSettings.GetStartingTime());

				while(fNextFileRotationTime <= fLastFileRotationTime)
				{
					fNextFileRotationTime.AddHours( fBackupSettings.GetFrequency());
				}
				
				if ((fNextFileRotationTime.GetLocalDay()!=curTime.GetLocalDay())||(fNextFileRotationTime.GetLocalMonth()!=curTime.GetLocalMonth())||(fNextFileRotationTime.GetLocalYear()!=curTime.GetLocalYear()))
				{
					// set the time to the "starting at" value
					SetTimeValue (fNextFileRotationTime, fBackupSettings.GetStartingTime());
				}
				break;	// LRC_ROTATE_EVERY_HOUR
			}

			case LRC_ROTATE_EVERY_DAY:
			{
				// I will first get the date of the last backup, or the creation date
				fNextFileRotationTime = fLastFileRotationTime;
				fNextFileRotationTime.AddDays (fBackupSettings.GetFrequency());

				// set the time to the "starting at" value
				SetTimeValue (fNextFileRotationTime, fBackupSettings.GetStartingTime());

				break;	// LRC_ROTATE_EVERY_DAY
			}
				
			case LRC_ROTATE_EVERY_WEEK:
			{
				sLONG dayHour = 0;
				//	First, we check if there is a backup today
				//	Then if there is one this week				
				
				// I will first get the date of the last backup, or the creation date
				fNextFileRotationTime = fLastFileRotationTime;	
				// if the date is the date of today, we just update the hour
				if ((fNextFileRotationTime.GetLocalDay()==curTime.GetLocalDay())&&(fNextFileRotationTime.GetLocalMonth()==curTime.GetLocalMonth())&&(fNextFileRotationTime.GetLocalYear()==curTime.GetLocalYear()))
				{
					VHTTPServerLogBackupSettings::EWeekDay weekDay = (VHTTPServerLogBackupSettings::EWeekDay)curTime.GetWeekDay();

					if (fBackupSettings.GetSaveOnWeekDay (weekDay))
					{	
						dayHour = fBackupSettings.GetWeekDayBackupHour (weekDay);
						
						SetTimeValue (fNextFileRotationTime, dayHour);
					}

					if (fLastFileRotationTime < fNextFileRotationTime)
					{
						break;	// we will have to backup later this day
					}
				}
				// no backup time has been determined
				// let's search the next day which has a backup planned
				
				bool flagIsOk = false;
				fNextFileRotationTime = fLastFileRotationTime;	
				//	This week a backup is planned... because a backup has been done already
				//	Let's check if another backup is planned this week
				//	note : like in the database backup, if we missed one backup, it will be done ASAP
				if (fNextFileRotationTime.GetWeekDay() != VHTTPServerLogBackupSettings::eSunday)
				{
					while (fNextFileRotationTime.GetWeekDay() != VHTTPServerLogBackupSettings::eSunday  && !flagIsOk)
					{
						VHTTPServerLogBackupSettings::EWeekDay weekDay = (VHTTPServerLogBackupSettings::EWeekDay)fNextFileRotationTime.GetWeekDay();

						fNextFileRotationTime.AddDays (1);		// check the next day

						if (fBackupSettings.GetSaveOnWeekDay (weekDay))
						{
							dayHour = fBackupSettings.GetWeekDayBackupHour (weekDay);
							
							SetTimeValue (fNextFileRotationTime, dayHour);
							flagIsOk = true;
						}
					}
				}

				if (flagIsOk)	// another backup has been planned
					break;
				
				//	There is no backup to do this week
				//	Let's look gEveryWeek after ...
				fNextFileRotationTime = fLastFileRotationTime;	
				fNextFileRotationTime.AddDays (fBackupSettings.GetFrequency() * 7);

				//	We "rewind" the week until the first day of the week : Monday !
				while (fNextFileRotationTime.GetWeekDay() != VHTTPServerLogBackupSettings::eMonday)
					fNextFileRotationTime.AddDays(-1);

				//	We look each day of the week
				if (fBackupSettings.GetSaveOnWeekDay (VHTTPServerLogBackupSettings::eMonday))
				{
					dayHour = fBackupSettings.GetWeekDayBackupHour (VHTTPServerLogBackupSettings::eMonday);
					
					SetTimeValue (fNextFileRotationTime, dayHour);
				}
				else if (fBackupSettings.GetSaveOnWeekDay (VHTTPServerLogBackupSettings::eTuesday))
				{
					dayHour = fBackupSettings.GetWeekDayBackupHour (VHTTPServerLogBackupSettings::eTuesday);
					
					SetTimeValue (fNextFileRotationTime, dayHour);
					
					fNextFileRotationTime.AddDays(1);
				}
				else if (fBackupSettings.GetSaveOnWeekDay (VHTTPServerLogBackupSettings::eWednesday))
				{
					dayHour = fBackupSettings.GetWeekDayBackupHour (VHTTPServerLogBackupSettings::eWednesday);
					
					SetTimeValue (fNextFileRotationTime, dayHour);
					
					fNextFileRotationTime.AddDays(2);
				}
				else if (fBackupSettings.GetSaveOnWeekDay (VHTTPServerLogBackupSettings::eThursday))
				{
					dayHour = fBackupSettings.GetWeekDayBackupHour (VHTTPServerLogBackupSettings::eThursday);
					
					SetTimeValue (fNextFileRotationTime, dayHour);
					
					fNextFileRotationTime.AddDays(3);
					
				}
				else if (fBackupSettings.GetSaveOnWeekDay (VHTTPServerLogBackupSettings::eFriday))
				{
					dayHour = fBackupSettings.GetWeekDayBackupHour (VHTTPServerLogBackupSettings::eFriday);
					
					SetTimeValue (fNextFileRotationTime, dayHour);
					
					fNextFileRotationTime.AddDays(4);
				}
				else if (fBackupSettings.GetSaveOnWeekDay (VHTTPServerLogBackupSettings::eSaturday))
				{
					dayHour = fBackupSettings.GetWeekDayBackupHour (VHTTPServerLogBackupSettings::eSaturday);
					
					SetTimeValue (fNextFileRotationTime, dayHour);
					
					fNextFileRotationTime.AddDays(5);
				}
				else if (fBackupSettings.GetSaveOnWeekDay (VHTTPServerLogBackupSettings::eSunday))
				{
					dayHour = fBackupSettings.GetWeekDayBackupHour (VHTTPServerLogBackupSettings::eSunday);
					
					SetTimeValue (fNextFileRotationTime, dayHour);
					
					fNextFileRotationTime.AddDays(6);
				}

				break; // LRC_ROTATE_EVERY_WEEK
			}

			case LRC_ROTATE_EVERY_MONTH:
			{	
				fNextFileRotationTime = fLastFileRotationTime;
				fNextFileRotationTime.AddMonths( fBackupSettings.GetFrequency());
				
				if (fBackupSettings.GetMonthDay() == 29)	// the preference selected is "Last"
				{
					fNextFileRotationTime.AddMonths(1);
					fNextFileRotationTime.SetLocalDay(1);	// sets the time to the first day of he month
					fNextFileRotationTime.AddDays(-1);		// removes one day to get the last day of the previous month
				}
				else
				{
					fNextFileRotationTime.SetLocalDay(fBackupSettings.GetMonthDay());
				}
				// set the time to the "starting at" value
				SetTimeValue (fNextFileRotationTime, fBackupSettings.GetMonthDayHour());

				break;	// LRC_ROTATE_EVERY_MONTH
			}
				
			default : // Trying to backup on an unknown schedule
				assert (false);
				break;
		}
	}
}
