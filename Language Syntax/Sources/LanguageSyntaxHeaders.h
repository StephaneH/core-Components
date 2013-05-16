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

// Decomment the following line to active specific traces in DEBUG
//#define JS_DBG_TRACES

#ifdef JS_DBG_TRACES
	#define TRACE_FUNCTION_CALL_NAME	DebugMsg(__FUNCTION__); \
										DebugMsg("\n");

	#define TRACE_DUMP_SYMBOL(symbol)	if(NULL != symbol ) { \
											VString msg("[SymbolTable Dump] "); \
											msg.AppendString("Name(");		msg.AppendString(symbol->GetName());					msg.AppendString(") "); \
											msg.AppendString("Id(");		msg.AppendLong(symbol->GetID());						msg.AppendString(") "); \
											msg.AppendString("Line(");		msg.AppendLong(symbol->GetLineNumber());				msg.AppendString(") "); \
											msg.AppendString("Kind(");		msg.AppendLong(symbol->GetKind()); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindLocalVariableDeclaration )		msg.AppendString(",LocalVariableDeclaration"); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindFunctionDeclaration )			msg.AppendString(",FunctionDeclaration"); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindFunctionParameterDeclaration )	msg.AppendString(",FunctionParameterDeclaration"); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindCatchBlock )					msg.AppendString(",CatchBlock"); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindPublicProperty )				msg.AppendString(",PublicProperty"); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindObjectLiteral )					msg.AppendString(",ObjectLiteral"); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindTable )							msg.AppendString(",Table"); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindTableField )					msg.AppendString(",TableField"); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindClass )							msg.AppendString(",Class"); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindClassConstructor )				msg.AppendString(",ClassConstructor"); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindClassPublicMethod )				msg.AppendString(",ClassPublicMethod"); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindClassPrivateMethod )			msg.AppendString(",ClassPrivateMethod"); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindClassStaticMethod )				msg.AppendString(",ClassStaticMethod"); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindClassPrivilegedMethod )			msg.AppendString(",ClassPrivilegedMethod"); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindPrivateProperty )				msg.AppendString(",PrivateProperty"); \
											if( symbol->GetKind() == Symbols::ISymbol::kKindStaticProperty )				msg.AppendString(",StaticProperty"); \
											msg.AppendString(") ");	\
											if( symbol->GetAuxillaryKindInformation() != 0 ) { \
												msg.AppendString("AuxKind(");	msg.AppendLong(symbol->GetAuxillaryKindInformation()); \
												if( symbol->GetAuxillaryKindInformation() & Symbols::ISymbol::kInstanceValue )							msg.AppendString(",InstanceValue"); \
												if( symbol->GetAuxillaryKindInformation() & Symbols::ISymbol::kReferenceValue )							msg.AppendString(",ReferenceValue"); \
												if( symbol->GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModel )						msg.AppendString(",EntityModel"); \
												if( symbol->GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModelMethodEntity )			msg.AppendString(",EntityModelMethodEntity"); \
												if( symbol->GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModelMethodEntityCollection )	msg.AppendString(",EntityModelMethodEntityCollection"); \
												if( symbol->GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModelMethodDataClass )			msg.AppendString(",EntityModelMethodDataClass"); \
												if( symbol->GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModelAttribute )				msg.AppendString(",EntityModelAttribute"); \
												if( symbol->GetAuxillaryKindInformation() & Symbols::ISymbol::kKindUndefined )							msg.AppendString(",Undefined"); \
												if( symbol->GetAuxillaryKindInformation() & Symbols::ISymbol::kKindFunctionExpression )					msg.AppendString(",FunctionExpression"); \
												msg.AppendString(")");	\
											} \
											msg.AppendString("\n");	\
											DebugMsg(msg); \
										}

	#define TRACE_DUMP_SYMBOLS(symbols)	std::vector<Symbols::ISymbol*>::iterator it; \
										for(it=symbols.begin(); it!=symbols.end(); ++it) { \
											TRACE_DUMP_SYMBOL((*it)) \
										}
#else
	#define TRACE_FUNCTION_CALL_NAME
	#define TRACE_DUMP_SYMBOL(symbol)
	#define TRACE_DUMP_SYMBOLS(symbols)
#endif
