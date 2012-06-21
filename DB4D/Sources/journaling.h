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
#ifndef __JOURNALING__
#define __JOURNALING__

#if journal

void __WriteLogHexa(sLONG x);
void __WriteLogLong(sLONG x);
void __WriteLogStr(const uBYTE *cStr);
void __WriteLogVStr(const VString& vs);
void __WriteLnLogStr(const uBYTE *cStr = nil);
void __WriteLnLogVStr(const VString& vs);
void __WriteLogAction(const VString& vs, sLONG val);
void CloseJournal4D(void);
void __InitLog();

#define WriteLogLong(x) __WriteLogLong(x)
#define WriteLogHexa(x) __WriteLogHexa(x)
#define WriteLogStr(cStr) __WriteLogStr((uBYTE*)(cStr))
#define WriteLogVStr(s)  __WriteLogVStr(s)
#define WriteLnLogStr(cStr)  __WriteLnLogStr((uBYTE*)(cStr))
#define WriteLnLogVStr(s)  __WriteLnLogVStr(s)
#define WriteLogAction(s, x) __WriteLogAction(s, x)
#define CLOSELOG CloseJournal4D()
#define ACCEPTJOURNAL __InitLog()

#else

#define WriteLogHexa(x)
#define WriteLogLong(x)
#define WriteLogStr(s)
#define WriteLogVStr(s)
#define WriteLnLogStr(s)
#define WriteLnLogVStr(s)
#define WriteLogAction(s, x)
#define CLOSELOG
#define ACCEPTJOURNAL

#endif


#endif
