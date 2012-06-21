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
#include "KernelIPC/VKernelIPC.h"
#include "Code Editor/CodeEditorDocuments.h"
#include "SQL/Interfaces/CSQLServer.h"
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
