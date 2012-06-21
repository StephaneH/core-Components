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
#ifndef __ERR4D__
#define __ERR4D__


#define memfull VE_MEMORY_FULL


#define kErr4DNoComms 				-2L
#define kErr4DUnimpReq 				-9800L
#define kErr4DIncompWS 				-9956L
#define kErr4DClosedCon				-10001L
#define kErr4DClosedCon2			-10002L
#define kErr4DMemFull				-108L

#define kErr4DBadLen 				-3101L
#define kErr4DBadSeg				-3102L
#define kErr4DDepassePos			-3103L
#define kErr4DDiskFull				-3104L
#define kErr4DWrongBaseRef			-3105L
#define kErr4DBadTag				-3106L
#define kErr4DBadCheckSum			-3107L
#define KErr4DBackupExist			-3108L
#define KErr4DBackupNonExist		-3109L
#define kErr4DWrongFileRef			-3110L
#define kErr4DWrongRecLen			-3111L
#define kErr4DEOF					-3112L
#define kActionCanceled				-3113L
#define kErrStrToShort				-3114L
#define kErrRecordLocked			-3115L
#define kErrMissingCode				-3116L
#define kErrCanNotLoadBlob			-3117L
#define kErrUnimplemented			-3118L
#define kErr4DWrongFieldRef			-3119L
#define kErr4DWrongFormRef			-3120L
#define kErr4DResourceLocked		-3121L
#define kErr4DPrivilege				-3122L
#define kErr4DBaseInUse				-3123L	// tentative de fermeture d'une base en cours d'utilisation
#define kErr4DBaseAlreadyOpen		-3124L	// tentative d'ouverture d'une base deja ouverte

#define kErr4DQueryMissingArgument	-3150L
#define kErr4DQueryMissingOperator	-3151L
#define kErr4DQueryIsEmpty			-3152L

#define kErr4DTransactionConflict	-3153L
#define kErr4DEndOfCollection		-3154L
#define kErr4DNotFoundInCollection	-3155L

#define kErr4DSQLTokenNotFound	-3156L
#define kErr4DSQLSyntaxError	-3157L

#define kErr4DInvalidTableName	-3158L
#define kErr4DInvalidFieldName	-3159L

#define kErr4DInvalidIndex	-3160L
#define kErr4DInvalidIndexType	-3161L


           /* -------------------------------------- */


#define kMsgIdle4D							0x4444
#define kMsgClose4D							0x4445

#endif