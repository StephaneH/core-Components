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
#ifndef __LanguageSyntaxHeaders__
#define __LanguageSyntaxHeaders__

#include "Kernel/VKernel.h"
#include "KernelIPC/VKernelIPC.h"
#include "Code Editor/CodeEditorDocuments.h"
#include "CLanguageSyntax.h"
#include "DB4D/Headers/DB4D.h"

USING_TOOLBOX_NAMESPACE
#include "Kernel/Sources/ILexer.h"
#include "XML/Sources/VLocalizationManager.h"
#include "LanguageSyntax.h"
#include "LanguageSyntaxComponent.h"
#include "JavaScriptSyntax.h"
#include "SQLSyntax.h"
#include "PHPSyntax.h"     
#include "XMLSyntax.h"
#include "HTMLSyntax.h"
#include "CSSSyntax.h"
#include "ScriptDoc.h"
#include "SymbolTable.h"


// extract from icu
#define U_MASK(x) ((uint32_t)1<<(x))
enum
{
    U_UPPERCASE_LETTER        = 1,
    U_LOWERCASE_LETTER        = 2,
    U_TITLECASE_LETTER        = 3,
    U_MODIFIER_LETTER         = 4,
    U_OTHER_LETTER            = 5,
    U_NON_SPACING_MARK        = 6,
    U_ENCLOSING_MARK          = 7,
    U_COMBINING_SPACING_MARK  = 8,
    U_DECIMAL_DIGIT_NUMBER    = 9,
    U_LETTER_NUMBER           = 10,
    U_SPACE_SEPARATOR         = 12,
    U_LINE_SEPARATOR          = 13,
    U_CONNECTOR_PUNCTUATION   = 22,
};

#define U_GC_ZS_MASK    U_MASK(U_SPACE_SEPARATOR)
#define U_GC_LU_MASK    U_MASK(U_UPPERCASE_LETTER)
#define U_GC_LL_MASK    U_MASK(U_LOWERCASE_LETTER)
#define U_GC_LT_MASK    U_MASK(U_TITLECASE_LETTER)
#define U_GC_LM_MASK    U_MASK(U_MODIFIER_LETTER)
#define U_GC_LO_MASK    U_MASK(U_OTHER_LETTER)
#define U_GC_NL_MASK    U_MASK(U_LETTER_NUMBER)
#define U_GC_MC_MASK    U_MASK(U_COMBINING_SPACING_MARK)
#define U_GC_MN_MASK    U_MASK(U_NON_SPACING_MARK)
#define U_GC_ME_MASK    U_MASK(U_ENCLOSING_MARK)
#define U_GC_M_MASK		(U_GC_MN_MASK|U_GC_ME_MASK|U_GC_MC_MASK)
#define U_GC_ND_MASK    U_MASK(U_DECIMAL_DIGIT_NUMBER)
#define U_GC_PC_MASK    U_MASK(U_CONNECTOR_PUNCTUATION)
#define U_GC_N_MASK		(U_GC_ND_MASK|U_GC_NL_MASK|U_GC_NO_MASK)
#define U_GC_ZL_MASK    U_MASK(U_LINE_SEPARATOR)

#define U_GET_GC_MASK(c) U_MASK( XBOX::VIntlMgr::CharType(c))

#endif