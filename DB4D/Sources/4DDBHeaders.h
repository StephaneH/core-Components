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
#ifndef __4DDBHEADERS__
#define __4DDBHEADERS__

#define oldtempmem 0
#define AllowDefaultEMBasedOnTables 0
#define AllowSyncOnRecID 0
#define BuildEmFromTable 1


#define DB4D_COMPILE 1

#if COMPIL_GCC
#define DB4D_API EXPORT_API
#else
#define DB4D_API
#endif


#if GF
#include "VToolbox.h"
#else
#include "KernelIPC/VKernelIPC.h"
#include "ServerNet/VServerNet.h"
#include "../Headers/DB4D.h"
#include "XML/VXML.h"
#endif

#if OLDNAMESTODELETE
#define VStackArrayOf VxArrayOf
#define VStackArrayPtrOf VxArrayPtrOf
#define VStackArrayRetainedPtrOf VxArrayRetainedPtrOf

#endif


USING_TOOLBOX_NAMESPACE

using namespace std;

typedef sLONG8 DataAddr4D;
const sLONG kMaxSegData = 128;
const sLONG kMaxSegDataNormaux = 64;
const sLONG kMaxSegDataSpeciaux = 64;


const sLONG kMaxNbExtraElements = 1;
const sLONG kExtraElement_FieldsIDInRec = 0;
const sLONG kExtraElement_DataStore_StaticData = 1;

 
#include <map>
#include <set>
#include <vector>
#include <deque>
#include <algorithm>

#include "XML/VXML.h"

//#include "Language Kernel/CLanguage.h"
//#include "DB4DLang/CDB4DLang.h"
//#include "DB4DLang/DB4DLang_ClassDef.h"
//#include "vtuneapi.h"
#include "JavaScript/VJavaScript.h"


#if VERSIONWIN
#pragma warning (disable: 4800) // disable : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning (disable: 4355) // disable : 'this' : used in base member initializer list
#endif


#define WithSelectIO 0
#define FreeMemWaitOnLockWrite 0  // les big objets qui sont locke en Write ne peuvent pas etre libere par un autre process quand ce flag est a 1

#if VERSIONDEBUG
	#define debuglrWithTypObj 1
	#define debug 1
	#define debuglr 1
	#define debugblobs 0
	#define journal 0
	#define debuglog 0
	#define debuglogwrite 0
	#define debuglogplace 0
	#define debugoccupe 0
	#define debugindex_strong 0
	#define debugflush 0
	#define debugblob_strong 0
	#define debugfiche_strong 0
	#define debugoccupe_with_signature 0
	#define debugtrans_temp 0
	#define debug_BTItemIndex 0 // pas threadsafe
	#define debug_BlobsRefsWithTools 0
	#define debugOverlaps_strong 0
	#define debugServer_Streams 0
	#define debugObjInTree_Strong 0
	#define debugLeakCheck_Strong 0
	#define debugLeakCheck_BitSel 0
	#define debugLeakCheck_PetiteSel 0
	#define debugLeakCheck_TreeMem 0
	#define debugLeakCheck_Bittab 0
	#define debugLeakCheck_IndexValid 0
	#define debugLeakCheck_NbLoadedRecords 0
	#define debugLeakCheck_KeptSelections 0
	
	#define debugLeaksAll 1

	#define debugrws 0
	#define debugrws2 0
	#define debugTreeMem_Strong 0
	#define debugFindPlace_strong 0
	#define debugOverWrite_strong 0
	#define debugFicheOnDisk 0
	#define debugFindPlace_inMap 0
	#define debugTabAddr 0
	#define debugCheckIndexPageOnDiskInDestructor 0
	#define debugLogEventTabAddr 0
	#define debugLogEventFindFree 0
	#define debugLogEventPageIndex 0
	#define debugsyncevent 0
	#define debugIndexOverlap_strong 0

	#define debug_more_granularity 1
	#define debug_Addr_Overlap 0
	#define trackClose 0
	#define trackModif 0
	#define trackTruncate 0
	#define trackIndex 0

	#define debug_checkRelaseIndexPage 0

	#define trackRemoteModel 0

	#define debug_checkIndexFourche 1

#else
	#ifdef debug
	#undef debug
	#endif
	#define debugblobs 0
	#define debuglr 0
	#define journal 0
	#define debuglog 0
	#define debuglogwrite 0
	#define debuglogplace 0
	#define debugoccupe 0
	#define debugindex_strong 0
	#define debugflush 0
	#define debugblob_strong 0
	#define debugfiche_strong 0
	#define debugoccupe_with_signature 0
	#define debugtrans_temp 0
	#define debug_BTItemIndex 0
	#define debug_BlobsRefsWithTools 0
	#define debugOverlaps_strong 0
	#define debugServer_Streams 0
	#define debugObjInTree_Strong 0
	#define debugLeakCheck_Strong 0
#if WITH_ASSERT
	#define debugLeakCheck_BitSel 0
	#define debugLeakCheck_PetiteSel 0
	#define debugLeakCheck_Bittab 0
	#define debugLeakCheck_TreeMem 0
	#define debugLeakCheck_IndexValid 0
	#define debugLeakCheck_NbLoadedRecords 0
	#define debugLeakCheck_KeptSelections 0
#else
	#define debugLeakCheck_BitSel 0
	#define debugLeakCheck_PetiteSel 0
	#define debugLeakCheck_Bittab 0
	#define debugLeakCheck_TreeMem 0
	#define debugLeakCheck_IndexValid 0
	#define debugLeakCheck_NbLoadedRecords 0
	#define debugLeakCheck_KeptSelections 0
#endif

	#define debugLeaksAll 0

	#define debugrws 0
	#define debugrws2 0
	#define debugTreeMem_Strong 0
	#define debugFindPlace_strong 0
	#define debugFindPlace_log 0
	#define debugOverWrite_strong 0
	#define debugFindPlace_inMap 0
	#define debugTabAddr 0
	#define debugCheckIndexPageOnDiskInDestructor 0
	#define debugLogEventTabAddr 0
	#define debugLogEventFindFree 0
	#define debugLogEventPageIndex 0
	#define debugsyncevent 0
	#define debugIndexOverlap_strong 0

	#define debug_more_granularity 0
	#define debug_Addr_Overlap 0
	#define trackClose 0
	#define trackModif 0
	#define trackTruncate 0
	#define trackIndex 0

	#define debug_checkRelaseIndexPage 0

	#define trackRemoteModel 0

	#define debug_checkIndexFourche 0


#endif

#if debugLeakCheck_BitSel || debugLeakCheck_PetiteSel || debugLeakCheck_IndexValid || debugLeakCheck_NbLoadedRecords || debugLeakCheck_KeptSelections || debugLeakCheck_TreeMem || debugLeakCheck_Bittab
#undef  debugLeakCheck_Strong
#define debugLeakCheck_Strong 1
#endif

#ifndef nil
#define nil NULL
#endif

// L.E. 06/12/99 remapping sur des types VIT
typedef char **tHandle;
typedef VPtr tPtr;
typedef sLONG *sLONGPTR;
typedef uLONG *uLONGPTR;
typedef sWORD *sWORDPTR;
typedef uWORD *uWORDPTR;
typedef uBYTE *uBYTEPTR;
typedef sBYTE *sBYTEPTR;

typedef OsType tResType;
typedef UniPtr *tUniHdl;

typedef Real freal;

#define reflimit 31

// L.E. 09/03/09 major 1 -> 2 for stamps in adress tables in v12
// L.E. 28/03/11 major 2 -> 3 for blobs outside data in v13
// L.R 14 mars 2013 : major 3 -> 4 Fields ID in Rec in Wak 5, and more info filler

const uLONG8 kVersionDB4DMoins1 = XBOX_LONG8(0x0000000300000001);
const uLONG8 kVersionDB4D = XBOX_LONG8(0x0000000300000001);

const sLONG kIndexSegNum = 65;

const sLONG kRetourInvalidClusterIndex = 1;
const sLONG kRetourMustRebuildAllIndex = 2;
const sLONG kRetourMustRebuildDataTableHeader = 4;
const sLONG kRetourMustLoadOldRelations = 8;
const sLONG kRetourMustConvertIndex = 16;
const sLONG kRetourMustSaveInfo = 32;

#define BYTEORDER SMALLENDIAN
#define HighLow		0							// formac
#define LowHigh		0xFFFFFFFF					// forpc
#if WINVER
	#define ByteSens	LowHigh						// for pc
	#define OtherSens	HighLow						// Force la conversion
	#define Like68 0
#else
	#define ByteSens	HighLow						// formac
	#define OtherSens	LowHigh						// Force la conversion
	#define Like68 1
#endif

#if VERSIONMAC || VERSION_LINUX
#define FORMAC 1
#elif VERSIONWIN
#define FORWIN 1
#endif

const sLONG kMaxPositif = 0x7FFFFFFF;

#define KernRange -310000

inline void xSetPurgeable(tHandle pHandle,uBOOL pState) {;}
inline long minl(const long a, const long b) {return (a>b) ? b:a;}
inline long maxl(const long a, const long b) {return (a>b) ? a:b;}
inline long absl(const long a) {return (a>0) ? a:-a;}

class LockEntity;

inline LockEntity *vGetLockEntity() {return nil /*Cur4DThread()->GetLockEntity();*/ ;}

typedef struct tRect {
	short top, left, bottom, right;
} tRect;

#define blobx blob

/*
typedef enum {
    wrien,
    wegal,
    wdif,
    wsup,
    wsupegal,
    winf,
    winfegal,
    wcontient,
    wnoncontient,
    wfourche,
	wbeginwith,
	wendwith
}__rechop;
typedef	uCHAR	rechop; // enum

typedef enum {
    lrien,
    land,
    lor,
    DB4D_Except
}__oplogic;
typedef	uCHAR	oplogic; // enum
*/

enum {
    trig_rien,
    trig_ajout,
    trig_modif,
	trig_sup,
	trig_lecture
};
typedef	uCHAR	trigaction; // enum


class Table;
class FicheInMem;
class Fichier;


// L.E. 15/12/00 too big for visual...
//#include "kernel/UniCodeTable.h"

#if VERSIONMAC || VERSION_LINUX
//const sLONG kReadPerm=fsRdPerm;
//const sLONG kWritePerm=fsWrPerm;
//const sLONG kReadWritePerm=fsRdWrPerm;
//const sLONG kReadWriteSharePerm=fsRdWrShPerm;
const UniChar xkVolSep 	= CHAR_COLON;				// :
const UniChar kDirSep 	= CHAR_COLON;				// :
const UniChar kExtSep	= CHAR_FULL_STOP;			// .
#elif VERSIONWIN
const UniChar xkVolSep 		= CHAR_COLON;				// :
const UniChar kDirSep 			= CHAR_REVERSE_SOLIDUS;
const UniChar kExtSep			= CHAR_FULL_STOP;				// .
const UniChar xkWildCard		= CHAR_ASTERISK;			// *
#endif
const UniChar kVitDirSep		= CHAR_SOLIDUS;

const uCHAR FakeUUID_FieldInRecDef = 1;
const uCHAR FakeUUID_FieldInTableDef = 2;
const uCHAR FakeUUID_TableDef = 3;
const uCHAR FakeUUID_RecordDef = 4;
const uCHAR FakeUUID_TableVarInBaseDef = 5;
const uCHAR FakeUUID_TableVarInNameSpaceDef = 6;



/**
* Functor to retrieve the value of a key-value pair in a map, used in STL
* iteration algorithms.
*/
template <class PairType>
struct select2nd : public unary_function<PairType, typename PairType::second_type>
{
	typedef unary_function<PairType, typename PairType::second_type> BaseClassType;

	typedef typename BaseClassType::result_type     result_type;
	typedef typename BaseClassType::argument_type   argument_type;

	typedef PairType                                value_type;

	/**
	* Retrieve the value of a key-value pair.
	*
	* @param thePair key-value pair
	* @return value
	*/
	result_type
		operator()(const argument_type&     thePair) const
	{
		return thePair.second;
	}
};


#include "Err4D.h"

#include "VTools.h"


#include "restypes.h"
#include "journaling.h"
#include "util4D.h"
#include "CodeReg.h"
//#include "BufDisk.h"
#include "DB4DMgr.h"
#include "Cache.h"
#include "BitTable.h"
#include "Sels.h"
#include "Fichier.h"
#include "datasource.h"
#include "Base4D.h"
#include "Blob4D.h"
#include "DataSeg.h"
#include "Fiche.h"
#include "Flush.h"
#include "Cron.h"
#include "IndClust.h"
#include "IndexTemplate.h"
#include "index4D.h"
#include "EntityModel.h"
#include "LocalModel.h"
#include "InfoTask.h"
#include "Rech4D.h"
#include "RechComplex.h"
#include "Relation.h"
#include "Sort4D.h"
#include "TabAddr.h"
#include "Transac.h"
#include "Sql.h"
#include "ImpExp.h"
#include "CheckAndRepair.h"
#include "Server.h"
#include "DB4DComponent.h"
#include "Scan.h"
#include "journal_parser.h"
#include "Export.h"
#include "Web.h"
#include "Rest.h"
#include "IndexTemplatePart2.h"
#include "javascript_db4d.h"
#include "VCacheLog.h"

#endif
