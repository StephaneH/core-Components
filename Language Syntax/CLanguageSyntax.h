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
// CLanguageSyntax.h

#ifndef __LanguageSyntax__
#define __LanguageSyntax__

#include "Kernel/VKernel.h"
#include "DB4D/Headers/DB4D.h"

#if _WIN32
	#pragma pack( push, 8 )
#else
	#pragma options align = power
#endif

#define PHP_EXTENSION			"php"
#define JAVASCRIPT_EXTENSION	"js"
#define HTML_EXTENSION			"html"
#define HTM_EXTENSION			"htm"
#define XML_EXTENSION			"xml"
#define CSS_EXTENSION			"css"
#define SQL_EXTENSION			"sql"





// DON'T PLAY WITH THE FOLLOWINGS DEFINITIONS
// IT IS USED TO TRACES
#if VERSIONDEBUG
	#define ACTIVATE_JSLOG	0 // O=>No DEBUG TRACE, 1=>DEBUG TRACES
#else
	#define ACTIVATE_JSLOG	0 // O=>No RELEASE TRACE, 1=>RELEASE TRACES
	#include <iostream>
#endif
// DON'T PLAY WITH THE FOLLOWINGS DEFINITIONS
// IT IS USED TO ENABLE / DISABLE IN PROGRESS FEATURES
#define ACTIVATE_GLOBAL_DEFINITION_OUTLINE 1
#define ACTIVATE_SUGGESTION_ON_WIDGETS_AND_DATASOURCES 1

#define ACTIVATE_NEXT_JS_AST_WALKER_TRACES 0
#define ACTIVATE_SYMBOL_TABLE_TRACES 0
#define ACTIVATE_PARSING_SCHEDULER_TRACES 0
#define ACTIVATE_CORE_PARSING_TRACES 0
#define ACTIVATE_CORE_AST_BROWING_TRACES 0
#define ACTIVATE_PARSING_ERROR_TRACES 0
#define ACTIVATE_SUGGESTING_TRACES 0
#define ACTIVATE_OUTLINE_TRACES 0

#define ACTIVATE_GLOBAL_PARSING_TRACES 0
#define ACTIVATE_NEXT_GEN_AST_PREPARING_TRACES 0
#define ACTIVATE_NEXT_GEN_AST_UPDATING_TRACES 0
#define ACTIVATE_NEXT_GEN_AST_FINALIZING_TRACES 0

#define ACTIVATE_NEXT_GEN_AST_MODEL_PARSING_TRACES 0

#define ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST 0

#define ACTIVATE_SERVER_MODULES_AUTOCOMPLETE_DOC_PARSING 0

enum
{
	/*	0	*/eSymbolDeclarationTypeNone=0,
	/*	1	*/eSymbolDeclarationTypeObject,
	/*	2	*/eSymbolDeclarationTypeObjectField,
	/*	3	*/eSymbolDeclarationTypeFunctionCall,
	/*	4	*/eSymbolDeclarationTypeNew,
	/*	5	*/eSymbolDeclarationTypeFunctionExpression,
	/*	6	*/eSymbolDeclarationTypeFunctionArguments,
	/*	7	*/eSymbolDeclarationTypeReturns,
	/*	8	*/eSymbolDeclarationTypeVariable,
	/*	9	*/eSymbolDeclarationTypeIdentifiers,
	/*	10	*/eSymbolDeclarationTypeAssignment,
	/*	11	*/eSymbolDeclarationTypeStringLiteral,
	/*	12	*/eSymbolDeclarationTypeNumberLiteral,
	/*	13	*/eSymbolDeclarationTypeBooleanLiteral,
	/*	14	*/eSymbolDeclarationTypeRegExpLiteral,
	/*	15	*/eSymbolDeclarationTypeArrayLiteral,
	/*	16	*/eSymbolDeclarationTypeNullLiteral,
	/*	17	*/eSymbolDeclarationTypeTypes,
	/*	18	*/eSymbolDeclarationTypeProperty,
	/*	19	*/eSymbolDeclarationTypeUndefined
};
typedef sLONG ESymbolDeclarationType;

enum
{
	/*	0	*/	eSymbolScopeTypeAll=0,
	/*	1	*/	eSymbolScopeTypeUndefined,
	/*	2	*/	eSymbolScopeTypeGlobal,
	/*	3	*/	eSymbolScopeTypeFunctionDeclaration,
	/*	4	*/	eSymbolScopeTypeFunctionExpression,
	/*	5	*/	eSymbolScopeTypeCatch,
	/*	6	*/	eSymbolScopeTypeWith,
	/*	7	*/	eSymbolScopeTypeObject
};
typedef sLONG ESymbolScopeType;

enum
{
	/*	0	*/	eSymbolStatusUpToDate = 0,
	/*	1	*/	eSymbolStatusDeletable
};
typedef sLONG ESymbolStatus;

enum
{
	/*	0	*/	eSymbolDefinitionKindAll=0,
	/*	1	*/	eSymbolDefinitionKindKeyword,
	/*	2	*/	eSymbolDefinitionKindFunctionDeclaration,
	/*	3	*/	eSymbolDefinitionKindFunctionExpression,
	/*	4	*/	eSymbolDefinitionKindCatch,
	/*	5	*/	eSymbolDefinitionKindWith,
	/*	6	*/	eSymbolDefinitionKindUndefined,
	/*	7	*/	eSymbolDefinitionKindObject,
	/*	8	*/	eSymbolDefinitionKindProperty,
	/*	9	*/	eSymbolDefinitionKindInstanceProperty,
	/*	10	*/	eSymbolDefinitionKindVariable,
	/*	11	*/	eSymbolDefinitionKindLocalVariable
};
typedef sLONG ESymbolDefinitionKind;

enum
{
	/*	0	*/	eSymbolReferenceKindAll=0,
	/*	1	*/	eSymbolReferenceKindVariableDeclaration,
	/*	2	*/	eSymbolReferenceKindAssignmentDeclaration
};
typedef sLONG ESymbolReferenceKind;

enum EStyleValues {
	eColumnName = 1,
	eTableName = 2,
	eComment = 9,
	eKeyword = 14,		// We're matching this up with the SQL highlighting instead of keyword
	eNormal = 50,
	eNumber,
	eString,
	eName, 
	eComparison,
	eFunctionKeyword,
	eDebug,
	eRegExp,
	eSelector,			// CSS
	eWidget,
	eValue,
	eSeparator,
	eProperty,
	eIdentifier,
	eTag				// HTML
};

enum
{
	eSymbolFileBaseFolderStudio = 1,
	eSymbolFileBaseFolderStudioJSResources,
	eSymbolFileBaseFolderProject,
	eSymbolFileBaseFolderServerModules
};
typedef sLONG ESymbolFileBaseFolder;

enum
{
	eSymbolFileExecContextClient = 1,
	eSymbolFileExecContextServer,
	eSymbolFileExecContextClientServer
};
typedef sLONG ESymbolFileExecContext;

namespace xbox
{
	class ILexerToken;
	class VLocalizationManager;
};
typedef XBOX::VError ( *SQLTokenizeFuncPtr ) ( XBOX::VString& inSQLStatement, std::vector<XBOX::ILexerToken*>& outTokens, bool inContinuationOfComment );


class ISyntaxInterface;
class ICodeEditorDocument;
class VSymbolFileInfos;

class IEntityModelCatalogMethod : public XBOX::IRefCountable
{
public:

	virtual void	GetName(XBOX::VString& outName) = 0;
	virtual void	GetApplyTo(XBOX::VString& outApplyTo) = 0;
};

class IEntityModelCatalogAttribute : public XBOX::IRefCountable
{
public:

	virtual void	GetName(XBOX::VString& outName) = 0;
	virtual void	GetType(XBOX::VString& outApplyTo) = 0;
};


class IEntityModelCatalog : public XBOX::IRefCountable {
public:
	virtual void					GetCatalogPath( XBOX::VFilePath &outPath ) const = 0;

	/*
		Preferred way to get full access to entity model definitions.
		Use d4 namespace defined in db4d.h to access bag elements.

		The returned bag contains d4:entityModel elements.
	*/
	virtual	const XBOX::VValueBag*	RetainCatalogBag() const = 0;

	virtual	void					GetEntityModelNames( XBOX::VectorOfVString& outEntityNames) const = 0;
	virtual	void					GetEntityModelAttributes(const XBOX::VString& inEntityName, std::vector<IEntityModelCatalogAttribute* >& outAttributes) const = 0;
	virtual	void					GetEntityModelMethods(const XBOX::VString& inEntityName, std::vector< IEntityModelCatalogMethod* >& outMethods) const = 0;
};

class IDocumentParserManager : public XBOX::IRefCountable {
public:
	typedef XBOX::VSignalT_1< XBOX::VFilePath >									ParsingStartSignal;				// VFilePath inFile
	typedef XBOX::VSignalT_2< XBOX::VFilePath, sLONG >							ParsingCompleteSignal;			// VFilePath inFile, sLONG inStatus
	typedef XBOX::VSignalT_3< XBOX::VFilePath, sLONG, XBOX::VString >			CompileErrorSignal;				// VFilePath inFile, sLONG inLineNumber,VString inErrorMessage
	typedef XBOX::VSignalT_1< sLONG >											PendingParsingRequestSignal;	// sLONG inPendindRequestCount

	typedef enum Priority {
		kPriorityLow,
		kPriorityNormal,
		kPriorityAboveNormal,	// Yields the same amount as normal, but ensures that the item is processed before other "normal" items
		kPriorityHigh,
	} Priority;

	class IJob : public XBOX::IRefCountable {
	public:
		virtual void ScheduleTask( const VSymbolFileInfos inFileInfos ) = 0;
		virtual sLONG GetTaskCount() = 0;
	};

	IDocumentParserManager() { fCurrentSize = 0; fTotalSize = 0; } 

	// Creates a new job object for you to schedule tasks against
	virtual IJob *CreateJob() = 0;

	// When scheduling tasks or jobs, you may choose to pass in a symbol table to have the declarations added to it.  You may also decide not
	// to pass in a symbol table (for instance, if you wish to have information about compile errors only). One piece of information which is 
	// never optional is the "inCaller" parameter, which uniquely identifies who schedules what task.  This is needed so that it is possible 
	// to *unschedule* tasks later.  This is only used as a unique identifier, so typical usage will be to pass the "this" pointer.
	//
	// There are two ways to schedule an individual parsing task.  Both ways require you to pass in the location of a file on disk so that we can
	// uniquely identify the parsing pass (for instance, modifying the symbol table).  However, the alternative ScheduleTask call allows
	// you to pass in the contents of the file you wish to have parsed.  You might wish to do this if you are processing a file that exists
	// on disk, but before it has been saved.  The file path identifies which file to associate symbols with, but it doesn't require the data
	// to live on disk.
	virtual bool ScheduleTask( const void *inCaller, const VSymbolFileInfos &inFileInfos, const XBOX::VJSONValue &inValue, class ISymbolTable *inTable, Priority inPriority = kPriorityNormal, const bool& inParsingMandatory = false ) = 0;
	virtual bool ScheduleTask( const void *inCaller, const VSymbolFileInfos &inFileInfos, const XBOX::VString &inContents, class ISymbolTable *inTable, Priority inPriority = kPriorityNormal, const bool& inParsingMandatory = false ) = 0;
	virtual bool ScheduleTask( const void *inCaller, const VSymbolFileInfos &inFileInfos, class ISymbolTable *inTable, Priority inPriority = kPriorityNormal, const bool& inParsingMandatory = false ) = 0;
	virtual bool ScheduleTask( const void *inCaller, IEntityModelCatalog *inCatalog, class ISymbolTable *inTable, Priority inPriority = kPriorityNormal, const bool& inParsingMandatory = false ) = 0;
	// You can also schedule a batch of tasks to be formed in the form of a job.  This allows you to get a notification once the entire batch
	// has been processed, as well as individual items
	virtual bool ScheduleTask( const void *inCaller, IJob *inJob, class ISymbolTable *inTable, Priority inPriority = kPriorityNormal, const bool& inParsingMandatory = false ) = 0;
	// This also removes tasks associated with a job
	virtual void UnscheduleTasksForHandler( const void *inCaller ) = 0;

	virtual XBOX::VCriticalSection* GetLockForParseOrOutline(const XBOX::VFilePath&) = 0;

	virtual ParsingStartSignal &GetParsingStartSignal() = 0;
	virtual ParsingCompleteSignal &GetParsingCompleteSignal() = 0;
	virtual CompileErrorSignal &GetCompileErrorSignal() = 0;
	virtual PendingParsingRequestSignal &GetPendingParsingRequestSignal() = 0;

	virtual sLONG8	GetCurrentSize() { return fCurrentSize; }
	virtual void	SetCurrentSize(sLONG inCurrentSize) { fCurrentSize = inCurrentSize; }
	virtual sLONG8	GetTotalSize()   { return fTotalSize; }
	virtual void	SetTotalSize(sLONG inTotalSize) { fTotalSize = inTotalSize; }
	virtual sLONG	GetComputedPercentDone() { SmallReal result = fTotalSize ? (SmallReal) fCurrentSize / (SmallReal) fTotalSize * 100 : 100; return result; }
	
	virtual sLONG	GetFilesToParseCount() const = 0;

protected:
	sLONG8 fTotalSize;
	sLONG8 fCurrentSize;
};

class IBreakPointManager
{
public:
	virtual void UpdateBreakPointsLineNumbers( ICodeEditorDocument* inDocument, const std::vector<sWORD>& inBreakIDs, const std::vector<sLONG>& inLineNumbers ) = 0;
	virtual void AddBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber, sWORD& outID, bool& outDisabled ) = 0;
	virtual bool EditBreakPoint( ICodeEditorDocument* inDocument, sWORD inBreakID, bool& outDisabled ) = 0;
	virtual void UpdateBreakPointContent( ICodeEditorDocument* inDocument, sWORD inBreakID, const XBOX::VString& inLineContent ) = 0;
	virtual void RemoveBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber ) = 0;
	virtual bool GetBreakPoints( ICodeEditorDocument* inDocument, std::vector<sLONG>& outLineNumbers, std::vector<sWORD>& outIDs, std::vector<bool>& outDisabled ) = 0;
};

class CLanguageSyntaxComponent : public XBOX::CComponent
{
public:
	enum { Component_Type = 'lscm' };

	virtual ISyntaxInterface* GetSyntaxByExtension(const XBOX::VString &inextension) = 0;	
	virtual XBOX::VLocalizationManager* GetLocalizationMgr() = 0;

	virtual class ISymbolTable *CreateSymbolTable() = 0;
	virtual IDocumentParserManager *CreateDocumentParserManager() = 0;
	virtual bool ParseScriptDocComment( const XBOX::VString &inComment, std::vector< class IScriptDocCommentField * > &outFields ) = 0;

	virtual XBOX::VJSObject CreateJavaScriptTestObject( XBOX::VJSContext inJSContext ) = 0;

	virtual void SetBreakPointManager( ISyntaxInterface* inSyntax, IBreakPointManager* inBreakPointManager ) = 0;

	virtual void SetSQLTokenizer ( SQLTokenizeFuncPtr inPtr, const std::vector< XBOX::VString * >& vctrSQLKeywords, const std::vector< XBOX::VString * >& vctrSQLFunctions ) = 0;

	virtual void Stop() = 0;

	virtual void UseNextGen( bool inUseNextGen ) = 0;

	typedef XBOX::VSignalT_1< XBOX::VString >	DisplayDocumentationSignal;
	virtual DisplayDocumentationSignal& GetDisplayDocumentationSignal() = 0;
};

namespace ScriptDocKeys {
	// String, Exposed for: ParamElement
	EXTERN_BAGKEY( Name );
	// String, Exposed for: CommentElement, ParamElement, ReturnElement
	EXTERN_BAGKEY( Comment );
	// String, Exposed for: UnknownElement
	EXTERN_BAGKEY( Tag );
	// String, Exposed for: UnknownElement
	EXTERN_BAGKEY( Data );
	// String, Exposed for: TypeElement, ParamElement, ReturnElement
	EXTERN_BAGKEY( Types );
	// String, Exposed for: ParamValueElement
	EXTERN_BAGKEY( Values );
	// Boolean, Exposed for: ParamElement
	EXTERN_BAGKEY( IsOptional );
}

class IScriptDocCommentField : public XBOX::IRefCountable
{
public:
	enum ElementType {
		kUnknown,
		kClass,
		kComment,
		kConstructor,
		kDeprecated,
		kExtends,
		kInherits,
		kParam,
		kParamValue,
		kPrivate,
		kReturn,
		kType
	};

	virtual int GetKind() = 0;
	virtual XBOX::VString GetContent() = 0;
	virtual XBOX::VValueBag *GetContents() = 0;
};

namespace Symbols {
	// Since symbol tables can become incredibly large in a hurry, we do not want the symbol
	// to always store a reference to its child symbols.  If we were to do that, it would
	// mean that loading up a single symbol could load an entire tree, which could be extensive.
	// Instead, when we want to find information out about symbols, we ask the symbol table for
	// further information.  However, we will keep prototype and ownership information with 
	// the symbol.  It is generally a small amount of information, but can save a lot of time
	// cutting down how often we have to request information from the database.
	class ISymbol : public XBOX::IRefCountable
	{
	public:
		enum {
			// These are all of the private property types and cannot be found
			// using qualified lookup operations
			kKindLocalVariableDeclaration,
			kKindFunctionDeclaration,
			kKindFunctionParameterDeclaration,
			kKindCatchBlock,
			
			// This is the public property type, which is used for qualified 
			// lookup operations
			kKindPublicProperty,

			// These types are used to give extra information about the symbol
			kKindObjectLiteral,
			kKindClass,
			kKindClassConstructor,
			kKindClassPublicMethod,
			kKindClassPrivateMethod,
			kKindClassStaticMethod,
			kKindClassPrivilegedMethod,
			kKindPrivateProperty,
			kKindStaticProperty,

			// These values are masked in to provide auxillary information about
			// the symbol
			kKindEntityModel = 1 << 10,
			kKindEntityModelMethodEntity = 1 << 11,
			kKindEntityModelMethodEntityCollection = 1 << 12,
			kKindEntityModelMethodDataClass = 1 << 13,
			kKindEntityModelAttribute = 1 << 14,

			kKindFunctionExpression = 1 << 16
		};

		virtual XBOX::VString GetName() const = 0;
		virtual void SetName( const XBOX::VString &inName ) = 0;

		virtual XBOX::VString GetTypeName() const = 0;

		virtual XBOX::VString GetScriptDocComment() const = 0;
		virtual void SetScriptDocComment( const XBOX::VString &inCommentText ) = 0;

		virtual int GetAuxillaryKindInformation() const = 0;
		virtual void AddAuxillaryKindInformation( int inKind ) = 0;
		virtual void RemoveAuxillaryKindInformation( int inKind ) = 0;
		virtual int GetKind() const = 0;
		virtual void SetKind( int inKind ) = 0;
		virtual int GetFullKindInformation() const = 0;

		virtual bool			IsWAFKind() const = 0;
		virtual XBOX::VString	GetWAFKind() const = 0;
		virtual void			SetWAFKind( const XBOX::VString &inKind ) = 0;

		virtual const std::vector< ISymbol * >& GetPrototypes() const = 0;
		virtual void AddPrototype( ISymbol *inPrototype ) = 0;
		virtual void AddPrototypes( const std::vector< ISymbol * > &inSyms ) = 0;

		virtual ISymbol *GetOwner() const = 0;
		virtual void SetOwner( ISymbol *inOwner ) = 0;

		virtual const std::vector< ISymbol * >& GetReturnTypes() const = 0;
		virtual void AddReturnType( ISymbol *inType ) = 0;
		virtual void AddReturnTypes( const std::vector< ISymbol * > &inSyms ) = 0;

		virtual class IFile *GetFile() const = 0;
		virtual void SetFile( class IFile *inFile ) = 0;

		virtual bool HasID() const = 0;
		virtual sLONG GetID() const = 0;
		virtual void SetID( sLONG inID ) = 0;

		virtual sLONG GetLineNumber() const = 0;
		virtual sLONG GetLineCompletionNumber() const = 0;
		virtual void SetLineNumber( sLONG inLineNumber ) = 0;
		virtual void SetLineCompletionNumber( sLONG inLineNumber ) = 0;

		// Sometimes a symbol is a reference to another symbol.  In that case you can think of the current
		// symbol as more of an opaque placeholder for the reference.  Almost all of the operations which
		// would normally happen on this symbol should actually happen on the referenced symbol.  So we
		// provide a helper function to dereference a symbol -- if the symbol isn't actually a reference, then
		// the deref will just return "this", so it's safe to call on any symbol.  However, because this call
		// can return an unrelated symbol, it is up to you to call Release on the returned value!
		virtual const Symbols::ISymbol *RetainReferencedSymbol() const = 0;
		virtual void SetReferenceSymbol( Symbols::ISymbol *inReferences ) = 0;
		
		virtual bool IsFunctionKind() const = 0;
		virtual bool IsFunctionExpressionKind() const = 0;
		virtual bool IsFunctionParameterKind() const = 0;
		virtual bool IsPublicMethodKind() const = 0;
		virtual bool IsPublicPropertyKind() const = 0;
		virtual bool IsPublicKind() const = 0;
		virtual bool IsStaticKind() const = 0;
		virtual bool IsObjectLiteral() const = 0;
		virtual bool IsEntityModelKind() const = 0;

		virtual XBOX::VString	GetKindString() const = 0;
		virtual XBOX::VString	GetClass() const = 0;
		virtual bool			IsaClass() const = 0;
		
		virtual bool IsDeprecated() const = 0;
		virtual bool IsPrivate() const = 0;

		virtual bool GetParamValue(sLONG inIndex, std::vector<XBOX::VString>& outValues) const = 0;

		virtual XBOX::VString	GetKindString	(const int& inKind)		const = 0;
		virtual XBOX::VString	GetAuxKindString(const int& inAuxKind)	const = 0;
		
		virtual void SetUndefinedState(bool inState) = 0;
		virtual bool GetUndefinedState() const = 0;

		virtual void SetInstanceState(bool inState) = 0;
		virtual bool GetInstanceState() const = 0;

		virtual void SetReferenceState(bool inState) = 0;
		virtual bool GetReferenceState() const = 0;

		virtual void SetEditionState(bool inState) = 0;
		virtual bool GetEditionState() const = 0;

		virtual void SetFullName(const XBOX::VString& inName) = 0;
		virtual void ComputeFullName() = 0;
		virtual XBOX::VString GetFullName() const = 0;
	};

	class IFile : public XBOX::IRefCountable
	{
	public:
		// These paths should be relative to the symbol table instead of absolute paths, and
		// they should use POSIX separators
		virtual XBOX::VString GetPath() const = 0;
		virtual void SetPath( const XBOX::VString &inPath ) = 0;
	
		virtual ESymbolFileBaseFolder GetBaseFolder() const = 0;
		virtual void SetBaseFolder( const ESymbolFileBaseFolder inBaseFolder ) = 0;

		virtual ESymbolFileExecContext GetExecutionContext() const = 0;
		virtual void SetExecutionContext( const ESymbolFileExecContext inExecContext ) = 0;

		virtual uLONG8 GetModificationTimestamp() const = 0;
		virtual void SetModificationTimestamp( uLONG8 inTimestamp ) = 0;

		virtual sLONG GetID() const = 0;
		virtual void SetID( sLONG inID ) = 0;

		virtual void SetResolveState(bool inState) = 0;
		virtual bool GetResolveState() const = 0;
	};

	class IExtraInfo : public XBOX::IRefCountable
	{
	public:
		virtual XBOX::VString GetStringData() const = 0;
		virtual void SetStringData( const XBOX::VString &inData ) = 0;

		virtual uLONG GetIntegerData() const = 0;
		virtual void SetIntegerData( uLONG inData ) = 0;

		// This is a useful field for passing internal information along with the extra info.  
		// Because this is meant to use pointers, but not be stored information, the symbol 
		// table *DOES NOT* store the actual data into the database.  Hence, it is non-persistent.
		// This information is typically something that allows you to post-process symbol information
		// once the symbols have already been added to the symbol table.  So, for instance, you can
		// store a Symbol pointer into the non-persistent data section, and translate it into the 
		// Symbol's unique ID to set the Integer data.
		virtual const void *GetNonPersistentCookie() const = 0;
		virtual void SetNonPersistentCookie( const void * inData ) = 0;

		typedef enum Kind {
			kKindUnused,
			kKindFunctionSignature,		// Uses the String data
			kKindPropertyAssignedFunctionExpression,	// Uses Integer data to give symbol ID for the linked function expression
		} Kind;

		virtual Kind GetKind() const = 0;
		virtual void SetKind( Kind inKind ) = 0;
	};
}

class ISymbolUpdateParameters
{
public:
	virtual ~ISymbolUpdateParameters() {}
	
	virtual bool MustSetStatus() const = 0;
	virtual void MustSetStatus(const bool& inMust) = 0;

	virtual bool MustGetStatus() const = 0;
	virtual void MustGetStatus(const bool& inMust) = 0;
	
	virtual void SetStatus(const ESymbolStatus& inStatus) = 0;
	virtual void GetStatus(ESymbolStatus& outStatus) const = 0;
	
	virtual bool MustSetBoundaries() const = 0;
	virtual void MustSetBoundaries(const bool& inMust) = 0;

	virtual bool MustGetBoundaries() const = 0;
	virtual void MustGetBoundaries(const bool& inMust) = 0;

	virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset) = 0;
	virtual void GetBoundaries(sLONG& outFromLine, sLONG& outFromOffset, sLONG& outToLine, sLONG& outToOffset) const = 0;

	virtual bool MustSetConstructorFlag() const = 0;
	virtual void MustSetConstructorFlag(const bool& inMust) = 0;
	
	virtual bool MustGetConstructorFlag() const = 0;
	virtual void MustGetConstructorFlag(const bool& inMust) = 0;
	
	virtual void SetConstructorFlag(const bool& inFlag) = 0;
	virtual void GetConstructorFlag(bool& outFlag) const = 0;
};

class ISymbolTable : public XBOX::IRefCountable
{
public:
	/*
	 *	NEXT GEN MANAGEMENT
	 */
	virtual bool UseNextGen() const = 0;

	/*
	 *	SCOPE MANAGEMENT
	 */
	virtual ISymbolUpdateParameters* RetainSymbolUpdateParameters() const = 0;
	virtual void ReleaseSymbolUpdateParameters(ISymbolUpdateParameters* inUpdateParameters) const = 0;
	
	virtual bool GetGlobalScope(const ESymbolFileExecContext& inExecutionContext, ISymbolUpdateParameters* inUpdateParameters, sLONG& outScopeId) const = 0;
	virtual void SetGlobalScope(const ESymbolFileExecContext& inExecutionContext, sLONG& outScopeId) = 0;

	virtual bool GetScope(const XBOX::VString& inScopeName, const ESymbolScopeType& inScopeType, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, ISymbolUpdateParameters* inUpdateParameters, sLONG& outScopeId) const = 0;
	virtual bool GetScope(const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, const sLONG& inLineNumber, sLONG& outScopeId) const = 0;
	virtual bool GetScope(const XBOX::VString& inScopeName, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromScopeId, const sLONG& inToScopeId, sLONG& outScopeId) const = 0;
	virtual bool GetScopes(const XBOX::VString& inScopeName, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromScopeId, const sLONG& inToScopeId, std::vector<sLONG>& outScopeIds) const = 0;
	virtual bool GetScopes(const XBOX::VString& inScopeName, const std::vector<sLONG>& inFromScopeIds, std::vector<sLONG>& outScopeIds) const = 0;
	
	virtual void SetScopeFullName(const sLONG inScopeId, const XBOX::VString& inScopeFullName) = 0;
	virtual void SetScopeType(const sLONG inScopeId, const ESymbolScopeType& inScopeType) = 0;
	virtual void SetScope(const XBOX::VString& inScopeName, const ESymbolScopeType& inScopeType, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset, const bool& inConstructorFlag, sLONG& outScopeId) = 0;
	
	virtual void PrepareUpdates(const sLONG& inSourceId, const bool& inOutlineOnly) = 0;
	virtual void FinalizeUpdates(const sLONG& inSourceId, const bool& inOutlineOnly) = 0;
	
	virtual bool GetScopeBoundaries(const sLONG& inScopeId, sLONG& outFromLine, sLONG& outFromOffset, sLONG& outToLine, sLONG& outToOffset) const = 0;
	virtual bool GetScopeSourceId(const sLONG& inScopeId, sLONG& outSourceId) const = 0;
	virtual bool GetScopeName(const sLONG& inScopeId, XBOX::VString& outName) const = 0;
	virtual bool GetScopeFullName(const sLONG& inScopeId, XBOX::VString& outName) const = 0;
	virtual bool GetScopeType(const sLONG& inScopeId, ESymbolScopeType& outType) const = 0;
	virtual bool GetScopeParentId(const sLONG& inScopeId, sLONG& outParentId) const = 0;
	virtual bool GetScopeChildren(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const bool& inUseSourceId, const sLONG& inSourceId, std::vector<sLONG>& outScopeChildrenIds) const = 0;
	virtual bool GetScopeChildren(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inChildName, std::vector<sLONG>& outScopeChildrenIds) const = 0;
	virtual bool GetScopeChild(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inChildName, sLONG& outScopeChildId) const = 0;
	virtual bool GetScopePrototypeId(const XBOX::VString& inScopeName, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromScopeId, const sLONG& inToScopeId, sLONG& outScopeId) const = 0;

	/*
	 *	SOURCE MANAGEMENT
	 */
	virtual void DeleteSource(const sLONG& inSourceId) = 0;
	virtual bool GetSourcePath(const sLONG& inSourceId, XBOX::VString& outSourcePath) const = 0;
	virtual bool GetSourceBaseFolder(const sLONG& inSourceId, ESymbolFileBaseFolder& outSourceBaseFolder) const = 0;
	virtual bool GetSourceId(const XBOX::VString& inSourcePath, const ESymbolFileExecContext& inExecutionContext, const ESymbolFileBaseFolder& inBaseFolder, sLONG& outFileId) const = 0;
	virtual bool GetHtmlSourceIdLinkedToJsSourceId(const sLONG& inJsSourceId, sLONG& outHtmlSourceId) const = 0;
	
	/*
	 *	SYMBOL'S NAME MANAGEMENT
	 */
	virtual bool GetName(const XBOX::VString& inSymbolName, const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, ISymbolUpdateParameters* inUpdateParameters, sLONG& outSymbolNameId) const = 0;
	virtual bool GetNames(const XBOX::VString& inSymbolName, const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, ISymbolUpdateParameters* inUpdateParameters, std::vector<sLONG>& outSymbolNameIds) const = 0;
	virtual bool GetName(const sLONG& inNameId, XBOX::VString& outName) const = 0;
	virtual bool GetName(const sLONG& inNameId, const XBOX::VString& inSymbolStartsWith, XBOX::VString& outName) const = 0;
	virtual bool GetNameScopeId(const sLONG& inNameId, sLONG& outScopeId) const = 0;
	virtual bool GetNameExecutionContext(const sLONG& inNameId, ESymbolFileExecContext& outExecutionContext) const = 0;
	virtual bool GetNameFromScope(const XBOX::VString& inSymbolName, const sLONG& inFromScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, sLONG& outNameId) const = 0;
	virtual bool GetNames(const XBOX::VString& inStartWith, const sLONG& inScopeId, std::vector<sLONG>& outNameIds) const = 0;
	
	virtual void SetName(const XBOX::VString& inSymbolName, const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, sLONG& outSymbolNameId) = 0;

	/*
	 *	SYMBOL'S DEFINITION MANAGEMENT
	 */
	virtual bool GetDefinition(const sLONG& inScopeId, const sLONG& inNameId, const sLONG& inSourceId, const ESymbolDefinitionKind& inKind, const sLONG& inLineNumber, ISymbolUpdateParameters* inUpdateParameters, sLONG& outDefinitionId) const = 0;
	virtual bool GetDefinitionId(const sLONG& inScopeId, const sLONG& inNameId, const sLONG& inSourceId, sLONG& outDefinitionId) const = 0;
	virtual bool GetDefinitionName(const sLONG& inDefinitionId, XBOX::VString& outName) const = 0;
	virtual bool GetDefinitionNameId(const sLONG& inDefinitionId, sLONG& outNameId) const = 0;
	virtual bool GetDefinitionKind(const sLONG& inDefinitionId, ESymbolDefinitionKind& outKind) const = 0;
	virtual bool GetDefinitionParentScopeId(const sLONG& inDefinitionId, sLONG& outParentScopeId) const = 0;
	virtual bool GetDefinitionStatus(const sLONG& inDefinitionId, ESymbolStatus& outStatus) const = 0;
	virtual bool GetDefinitionSourceId(const sLONG& inDefinitionId, sLONG& outSourceId) const = 0;
	virtual bool GetDefinitionLine(const sLONG& inDefinitionId, sLONG& outLine) const = 0;
	
	virtual void SetDefinition(const sLONG& inSourceId, const sLONG& inScopeId, const sLONG& inNameId, const ESymbolFileExecContext& inExecutionContext, const ESymbolDefinitionKind& inKind, const sLONG& inLineNumber, sLONG& outDefinitionId) = 0;
	
	/*
	 *	SYMBOL'S SUGGESTION MANAGEMENT
	 */
	virtual void GetConstructorNames(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inStartWith, const sLONG& inFromScope, const sLONG& inToScope, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds) const = 0;
	virtual void GetWidgetNames(const sLONG& inHtmlSourceId, const XBOX::VString& inStartWith, std::vector<XBOX::VString>& outNames) const = 0;
	virtual void GetModelEntityPropertyNames(const XBOX::VString& inModelName, const XBOX::VString& inModelEntityName, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds) const = 0;
	virtual void GetNames(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inStartWith, const sLONG& inFromScope, const sLONG& inToScope, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds) const = 0;
	virtual void GetNames(const XBOX::VString& inStartWith, const std::vector<sLONG>& inScopeIds, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds) const = 0;
	
	/*
	 *	SYMBOL'S PROTOTYPE CHAIN MANAGEMENT
	 */
	virtual bool GetPrototypeChain(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inFromName, const XBOX::VString& inToName, ISymbolUpdateParameters* inUpdateParameters, sLONG& outPrototypeId) const = 0;
	virtual bool GetPrototypeChain(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inFromName, std::vector<XBOX::VString>& outPrototypeNames) const = 0;
	virtual bool GetPrototypeChainParentName(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inFromName, XBOX::VString& outParentPrototypeName) const = 0;
	
	virtual void SetPrototypeChain(const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, const XBOX::VString& inFromName, const XBOX::VString& inToName) = 0;
	
	/*
	 *	SYMBOL'S RETURNS CHAIN MANAGEMENT
	 */
	virtual bool GetReturnType(const sLONG& inScopeId, const XBOX::VString& inReturnTypeName, ISymbolUpdateParameters* inUpdateParameters, sLONG& outReturnTypeId) const = 0;
	virtual bool GetReturnTypeNames(const sLONG& inScopeId, std::vector<XBOX::VString>& outNames) const = 0;
	
	virtual void SetReturnType(const sLONG& inSourceId, const sLONG& inScopeId, const XBOX::VString& inTypeName) = 0;

	/*
	 *	SYMBOL'S TYPES MANAGEMENT
	 */
	virtual bool GetType(const sLONG& inDefinitionId, const XBOX::VString& inTypeName, ISymbolUpdateParameters* inUpdateParameters, sLONG& outTypeId) const = 0;
	virtual bool GetTypeNames(const sLONG& inDefinitionId, std::vector<XBOX::VString>& outNames) const = 0;
	
	virtual void SetType(const sLONG& inDefinitionId, const sLONG& inSourceId, const XBOX::VString& inTypeName) = 0;

	/*
	 *	SYMBOL'S REFERENCES MANAGEMENT
	 */
	virtual bool GetReference(const ESymbolReferenceKind& inReferenceKind, const sLONG& inFromId, const sLONG& inToId, ISymbolUpdateParameters* inUpdateParameters, sLONG& outReferenceId) const = 0;
	virtual bool GetReference(const ESymbolReferenceKind& inReferenceKind, const sLONG& inFromId, ISymbolUpdateParameters* inUpdateParameters, std::vector<sLONG>& outReferenceIds) const = 0;
	virtual bool GetReferenceToId(const sLONG& inReferenceId, sLONG& outToId) const = 0;
	virtual bool GetReferenceKind(const sLONG& inReferenceId, ESymbolReferenceKind& outReferenceKind) const = 0;
	virtual bool IsReferenced(const sLONG& inScopeId) const = 0;
	
	virtual void SetReference(const ESymbolReferenceKind& inReferenceKind, const sLONG& inFromId, const sLONG& inToId, const sLONG& inSourceId) = 0;

	/*
	 *	SYMBOL'S OUTLINE MANAGEMENT
	 */
	virtual bool GetScopeContentDefinitions(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, std::vector<sLONG>& outIds) const = 0;
	virtual bool GetScopeContentDefinitions(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, std::vector<sLONG>& outIds) const = 0;

	/*
	 *	OUTLINE ITEM DATAS MANAGEMENT
	 */
	virtual bool GetOutlineItem(const sLONG& inSourceId, const XBOX::VString& inName, const sLONG& inLine, ISymbolUpdateParameters* inUpdateParameters, sLONG& outItemId) const = 0;
	virtual bool GetOutlineItemSourceId(const sLONG& inItemId, sLONG& outSourceId) const = 0;
	virtual bool GetOutlineItemName(const sLONG& inItemId, XBOX::VString& outName) const = 0;
	virtual bool GetOutlineItemLine(const sLONG& inItemId, sLONG& outLine) const = 0;
	
	virtual void SetOutlineItem(const sLONG& inSourceId, const XBOX::VString& inName, const sLONG& inLine, sLONG& outItemId) = 0;

	typedef enum SortingOptions {
		kDefault,
		kByName,
		kByLineNumber,
		kByKind,
	} SortingOptions;
	
	// This function assigns a new unique ID to a symbol that is to be added to this symbol
	// table.  We do this instead of allowing the database to do it for us as an optimization.
	// Any time you create a new ISymbol that you intend to add to the symbol table, you should
	// call this function to assign it a unique ID.
	virtual uLONG GetNextUniqueID() = 0;

	// Converts a file path into one that is relative to the symbol table itself.  This is useful
	// when putting data into an IFile our pulling data out from an IFile.
	virtual void GetRelativePath( const XBOX::VFilePath &inFilePath, const ESymbolFileBaseFolder inBaseFolder, XBOX::VString &outFilePath ) const = 0;

	// The symbol table is thread safe -- however, if you are going to be handing a symbol table
	// over to a new thread, you need to clone it for new thread usage.  This way, Thread A and
	// Thread B can continue to operate on the same symbol table object in a thread-safe manner.
	// Failing to adhere to this can lead to database corruption, crashes and other bad things.
	virtual ISymbolTable *CloneForNewThread() = 0;

	// This call will open an existing symbol table database, and if one doesn't exist, it 
	// will create a new symbol table database from scratch at the location provided.  If the
	// database cannot be opened or created, this call will return false.
	virtual bool OpenSymbolDatabase( const XBOX::VFile &inDatabaseFile ) = 0;

	// These calls are used to add rows to the database.  Any relationships between the files and 
	// symbols must be preserved when adding them to the database, which means that this call must
	// receive sets of data if they are related.  That is because it needs to add all of the base 
	// data to the table first, and then set up all of the relationships.  Failing to do so would 
	// mean that ownership and prototypes may not be hooked up properly.  The only caveat to this 
	// rule has to do with files and symbols.  The file a symbol belongs to must be added to the
	// database before the symbols contained within the file.
	virtual bool AddFile( Symbols::IFile* inFile ) = 0;
	virtual bool AddSymbols( std::vector< Symbols::ISymbol * > inSymbols ) = 0;

	virtual bool UpdateFile( Symbols::IFile *inFile ) = 0;

	// These calls are used to query data from the database and will form the bulk of the symbol
	// table usage.  The caller is responsible for calling Release on the symbols that are returned
	// when they are done using them.
	virtual std::vector< Symbols::ISymbol * > RetainSymbolsByName( const XBOX::VString &inName ) = 0;
	virtual std::vector< Symbols::ISymbol * > RetainSymbolsByName( Symbols::ISymbol *inOwner, const XBOX::VString &inName, Symbols::IFile *inOwnerFile = NULL ) = 0;
	virtual std::vector< Symbols::ISymbol * > RetainClassSymbols( Symbols::ISymbol *inOwner, const XBOX::VString &inName, Symbols::IFile *inOwnerFile = NULL ) = 0;
	virtual void ReleaseSymbols(std::vector<Symbols::ISymbol*>& inSymbols) = 0;
	
	// The owner file allows you to filter so that you only retrieve symbols for a given file.  If you
	// don't supply an owner file, this API will get you all of symbols in the table, regardless of 
	// which file the symbol belongs to.
	virtual std::vector< Symbols::ISymbol * > GetSymbolsForOutline( Symbols::ISymbol *inOwner, Symbols::IFile *inOwnerFile, SortingOptions inOptions ) = 0;
	virtual std::vector< Symbols::ISymbol * > RetainNamedSubSymbols( Symbols::ISymbol *inOwner, Symbols::IFile *inOwnerFile = NULL, SortingOptions inOptions = kDefault ) = 0;
	virtual std::vector< Symbols::IFile * > RetainFilesByPathAndBaseFolder( const XBOX::VFilePath &inPath, const ESymbolFileBaseFolder inBaseFolder) = 0;
	virtual std::vector< Symbols::IFile * > RetainFilesByName( const XBOX::VString &inName ) = 0;
	virtual void ReleaseFiles(std::vector<Symbols::IFile*>& inFiles) = 0;

	// This function should typically only be used when trying to get suggestions for the auto complete
	// engine.  The symbols that are returned are not to be used for navigating the symbol hierarchy
	// because they've not been fully loaded.  For instance, the owner, return type, etc will always be
	// set to NULL to speed up the loading of symbols.  This is required because large projects can have
	// a large amount of symbols at the root level.  Also note that if you pass in an empty string, or
	// the match string ("@"), then this function will not return any results.
	virtual std::vector< Symbols::ISymbol * > RetainLikeNamedSubSymbols( Symbols::ISymbol *inOwner, const XBOX::VString &inLikeName, const bool inFullyLoad = false) = 0;

	// Use this function to perform a narrowest-fit search on all of the symbols in a given file.  This
	// is useful to find the context object for a given line of source code.  This function will return 
	// NULL if the line number resides in the global namespace.
	virtual Symbols::ISymbol *GetSymbolOwnerByLine( Symbols::IFile *inOwnerFile, int inLineNumber ) = 0;

	// If you have a symbol's unique ID, you can locate the exact symbol with this API
	virtual Symbols::ISymbol *GetSymbolByID( sLONG inID ) = 0;

	virtual bool UpdateSymbol( Symbols::ISymbol * inSymbol ) = 0;

	// This function will remove the symbol and any associated children from the table
	virtual bool DeleteSymbolsForFile( Symbols::IFile *inFile, bool inOnlyEditedSymbols ) = 0;

	virtual std::vector< Symbols::IExtraInfo * > GetExtraInformation( const Symbols::ISymbol *inOwnerSymbol, Symbols::IExtraInfo::Kind inKind ) = 0;
	virtual bool AddExtraInformation( const Symbols::ISymbol *inOwnerSymbol, const Symbols::IExtraInfo *inInfo ) = 0;

	virtual void LockUpdates() = 0;
	virtual void UnlockUpdates() = 0;

	virtual void GetSymbolSignature(Symbols::ISymbol* inSym, XBOX::VString& outSignature)
	{
		if (NULL != inSym)
		{
			if ( outSignature.IsEmpty() )
				outSignature = inSym->GetName();

			Symbols::ISymbol *refSym = const_cast< Symbols::ISymbol * >( inSym->RetainReferencedSymbol() );

			// Function symbols will generally have some extra information associated with them to give
			// us the function's signature.  We will try to grab that now, and change our item's text.
			if ( refSym->IsFunctionKind() )
			{
				if (refSym->GetKind() != Symbols::ISymbol::kKindClass)
				{
					XBOX::VString extra;
					std::vector< Symbols::IExtraInfo * > extras = GetExtraInformation( refSym, Symbols::IExtraInfo::kKindFunctionSignature );
					if (!extras.empty())
						extra = extras.front()->GetStringData();

					XBOX::VIndex found = extra.Find("(");
					if (found > 1)
						extra.SubString(found , extra.GetLength() + 1 - found);
					if ( extra != CVSTR("()") )
						outSignature += CVSTR (" ") + extra;
					for (std::vector< Symbols::IExtraInfo * >::iterator iter = extras.begin(); iter != extras.end(); ++iter)
						(*iter)->Release();

					outSignature +=  " : "  + refSym->GetTypeName();
				}
			}
			else
			{
				XBOX::VString symTypeName = refSym->GetTypeName();
				if (symTypeName.GetLength() && symTypeName != CVSTR("Function"))
					outSignature += " : " + symTypeName;
			}

			refSym->Release();
		}
	}

	virtual Symbols::ISymbol* GetAssignedFunction(Symbols::ISymbol* inSym)
	{
		Symbols::ISymbol* functionSym = NULL;

		std::vector< Symbols::IExtraInfo * > extras = GetExtraInformation( inSym, Symbols::IExtraInfo::kKindPropertyAssignedFunctionExpression );
		if ( ! extras.empty() )
		{
			functionSym = GetSymbolByID( extras.front()->GetIntegerData() );
			for ( std::vector< Symbols::IExtraInfo * >::iterator it = extras.begin(); it != extras.end(); ++it)
				(*it)->Release();
		}

		return functionSym;
	}

	virtual void SetBaseFolderPathStr(const ESymbolFileBaseFolder& inType, const XBOX::VString& inPathStr, const bool& inPosixConvert) = 0;
	virtual void GetBaseFolderPathStr(const ESymbolFileBaseFolder& inType, XBOX::VString& outPathStr) const = 0;
	
	virtual void SetSymbolsEditionState(const Symbols::IFile* inFile, bool inEditionState, std::vector<sLONG>& outIds) = 0;

	virtual void GetFileIdsReferencingDeadSymbols(const std::vector<sLONG>& inSymbolIds, std::vector<sLONG>& outFileIds) = 0;
	
	virtual Symbols::IFile* RetainFileByID(sLONG inID) = 0;
	
	virtual void GetSymbolIdsByFileAndEditionState(sLONG inFileId, bool inEditionState, std::vector<sLONG>& outSymbolIds) = 0;
	
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	virtual void Dump(const XBOX::VString& inRequest) = 0;
#endif
};

// This is meant as a helper class for the auto-complete engine
// to make use of.  It's fully self-contained, and never expected
// to be subclassed.
class IAutoCompleteSymbolTable : public XBOX::IRefCountable
{
public:
	// This is a helper class for the symbol table to use
	class IAutoCompleteSymbol {
	public:
		enum SymbolType {
			eTable,
			eView,
			e4DMethod,
			eColumn,
			eSchema,
			eIndex,
			eGroup,
			e4DLocalVariable,
			e4DProcessVariable,
			e4DInterProcessVariable,
			eJavaScriptVariable,
			eJavaScriptFunction,
			eJavaScriptObjectLiteral,
			eJavaScriptObjectLiteralField,
			eJavaScriptGlobalContext,
		};

		virtual ~IAutoCompleteSymbol() {}

		virtual void GetCompletionText( XBOX::VString &outName ) = 0;
		virtual SymbolType GetType() = 0;
		virtual void GetSubSymbols( std::vector< IAutoCompleteSymbol * > &outSyms ) = 0;
		
		void SelfDelete() { delete this; }

		virtual IAutoCompleteSymbol *GetSubSymbol( XBOX::VString inName ) {
			std::vector< IAutoCompleteSymbol * > syms;
			GetSubSymbols( syms );
			for (std::vector< IAutoCompleteSymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter) {
				XBOX::VString text;
				(*iter)->GetCompletionText( text );
				if (text == inName) return *iter;
			}
			return NULL;
		}

		virtual void GetSubSymbols( SymbolType inFilter, std::vector< IAutoCompleteSymbol * > &outSyms ) {
			std::vector< IAutoCompleteSymbol * > syms;
			GetSubSymbols( syms );
			for (std::vector< IAutoCompleteSymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter) {
				if ((*iter)->GetType() == inFilter) outSyms.push_back( *iter );
			}
		}

		void SetTemporary( bool set ) { fIsTemporary = set; }
		bool IsTemporary() { return fIsTemporary; }
		void SetSemiHidden( bool set ) { fIsSemiHidden = set; }
		bool IsSemiHidden() { return fIsSemiHidden; }

	protected:
		// We want the user to call the constructor which assigns values
		explicit IAutoCompleteSymbol() : fIsTemporary( false ), fIsSemiHidden( false ) {}

		bool fIsTemporary;
		bool fIsSemiHidden;
	};

	class VColumnSymbol : public IAutoCompleteSymbol {
	private:
		XBOX::VString fName;

	protected:
		// So we can create a clone
		VColumnSymbol() : IAutoCompleteSymbol() {}

	public:
		static VColumnSymbol *TemporaryFrom( XBOX::VString renameTo ) {
			VColumnSymbol *ret = new VColumnSymbol();
			ret->SetTemporary( true );
			ret->fName = renameTo;
			return ret;
		}

		VColumnSymbol( class CDB4DField *field, bool inIsSemiHidden = false ) : IAutoCompleteSymbol() { 
			xbox_assert( field );
			SetSemiHidden( inIsSemiHidden ); 
			field->GetName( fName );
		}

		virtual void GetCompletionText( XBOX::VString &outName ) { outName = fName; }
		virtual SymbolType GetType() { return eColumn; }
		virtual void GetSubSymbols( std::vector< IAutoCompleteSymbol * > & /*outSyms*/) {}
	};

	class VTableSymbol : public IAutoCompleteSymbol {
	private:
		XBOX::VString fName;
		std::vector< class VColumnSymbol * > fColumns;

	protected:
		// So we can create a clone
		VTableSymbol() : IAutoCompleteSymbol() {}

	public:
		static VTableSymbol *TemporaryFrom( IAutoCompleteSymbol *other, XBOX::VString renameTo = "" ) {
			xbox_assert( other );
			if (other->GetType() != eTable)		return NULL;

			VTableSymbol *otherTable = static_cast< VTableSymbol * >( other );
			VTableSymbol *ret = new VTableSymbol();
			ret->SetTemporary( true );
			ret->fColumns.assign( otherTable->fColumns.begin(), otherTable->fColumns.end() );
			if (renameTo == "") {
				ret->fName = otherTable->fName;
			} else {
				ret->fName = renameTo;
			}
			return ret;
		}

		VTableSymbol( class CDB4DTable *table, bool inIsSemiHidden = false ) : IAutoCompleteSymbol() { 
			xbox_assert( table );
			SetSemiHidden( inIsSemiHidden ); 
			table->GetName( fName );
			sLONG fieldCount = table->CountFields();
			for (sLONG j = 1; j <= fieldCount; j++) {
				CDB4DField *field = table->RetainNthField( j );
				if (field) {
					fColumns.push_back( new VColumnSymbol( field, inIsSemiHidden ) );
					field->Release();
				}
			}
		}

		virtual void GetCompletionText( XBOX::VString &outName ) { outName = fName; }
		virtual SymbolType GetType() { return eTable; }
		virtual void GetSubSymbols( std::vector< IAutoCompleteSymbol * > &outSyms ) { 
			// Yay for no covariance in C++'s STL.
			for (std::vector< VColumnSymbol * >::iterator iter = fColumns.begin(); iter != fColumns.end(); ++iter) {
				outSyms.push_back( (*iter) );
			}
		}
	};

	class VViewSymbol : public IAutoCompleteSymbol {
	private:
		XBOX::VString fName;

	protected:
		// So we can create a clone
		VViewSymbol() : IAutoCompleteSymbol() {}

	public:
		static VViewSymbol *TemporaryFrom( IAutoCompleteSymbol *other, XBOX::VString renameTo = "" ) {
			xbox_assert( other );
			if (other->GetType() != eView)		return NULL;

			VViewSymbol *otherView = static_cast< VViewSymbol * >( other );
			VViewSymbol *ret = new VViewSymbol();
			ret->SetTemporary( true );
			if (renameTo == "") {
				ret->fName = otherView->fName;
			} else {
				ret->fName = renameTo;
			}
			return ret;
		}

		VViewSymbol( XBOX::VString& inName, bool inIsSemiHidden = false ) : IAutoCompleteSymbol() { 
			SetSemiHidden( inIsSemiHidden ); 
			fName = inName;
		}

		virtual void GetCompletionText( XBOX::VString &outName ) { outName = fName; }
		virtual SymbolType GetType() { return eView; }
		virtual void GetSubSymbols( std::vector< IAutoCompleteSymbol*>& /*outSyms*/) {
		}
	};

	class VIndexSymbol : public IAutoCompleteSymbol {
	private:
		XBOX::VString fName;

	public:
		VIndexSymbol( class CDB4DIndex *index ) : IAutoCompleteSymbol() {
			xbox_assert( index );
			fName = index->GetName();
		}

		virtual void GetCompletionText( XBOX::VString &outName ) { outName = fName; }
		virtual SymbolType GetType() { return eIndex; }
		virtual void GetSubSymbols(std::vector< IAutoCompleteSymbol*>& /*outSyms*/) {}
	};

	class VSchemaSymbol : public IAutoCompleteSymbol {
	private:
		XBOX::VString fName;

	public:
		VSchemaSymbol( class CDB4DSchema *schema ) : IAutoCompleteSymbol() {
			xbox_assert( schema ); 
			fName = schema->GetName();
		}

		virtual void GetCompletionText( XBOX::VString &outName ) { outName = fName; }
		virtual SymbolType GetType() { return eSchema; }
		virtual void GetSubSymbols( std::vector<IAutoCompleteSymbol*>& /*outSyms*/) {}
	};

	class VGroupSymbol : public IAutoCompleteSymbol {
	private:
		XBOX::VString fName;

	public:
		VGroupSymbol( XBOX::VString name ) : IAutoCompleteSymbol() {
			fName = name;
		}

		virtual void GetCompletionText( XBOX::VString &outName ) { outName = fName; }
		virtual SymbolType GetType() { return eGroup; }
		virtual void GetSubSymbols( std::vector< IAutoCompleteSymbol*>& /*outSyms*/) {}
	};

	virtual ~IAutoCompleteSymbolTable() {
		while (!fTable.empty()) {
			fTable.back()->SelfDelete();
			fTable.pop_back();
		}
	}

#if _DEBUG
	// This function is only used to assist with unit testing, and so it only exists in debug builds
	void EmptyTable( void ) {
		while (!fTable.empty()) {
			fTable.back()->SelfDelete();
			fTable.pop_back();
		}
	}
#endif

	void AddSymbolToTable( IAutoCompleteSymbol *inSym ) { fTable.push_back( inSym ); }
	//void AddSymbolToTable( XBOX::VString inName, IAutoCompleteSymbol::SymbolType inType ) { fTable.push_back( new IAutoCompleteSymbol( inName, inType ) ); }
	size_t GetSymbolCount() { return fTable.size(); }
	IAutoCompleteSymbol *GetSymbol( size_t inIndex ) { return (inIndex < fTable.size()) ? (fTable[ inIndex ]) : (NULL); }
	IAutoCompleteSymbol *GetSymbolByName( XBOX::VString name ) {
		for (std::vector< IAutoCompleteSymbol * >::iterator iter = fTable.begin(); iter != fTable.end(); ++iter) {
			XBOX::VString text;
			(*iter)->GetCompletionText( text );
			if (name.EqualToString( text, true ))	return *iter;
		}
		return NULL;
	}

	void ReplaceSymbolInTable( IAutoCompleteSymbol *inSym, IAutoCompleteSymbol *inReplacement ) {
		for (size_t i = 0; i < fTable.size(); ++i) {
			if (fTable[ i ] == inSym) {
				fTable[ i ] = inReplacement;
				inSym->SelfDelete();
			}
		}
	}

	void RemoveTemporaries() {
		for (std::vector< IAutoCompleteSymbol * >::reverse_iterator iter = fTable.rbegin(); iter != fTable.rend(); ++iter) {
			IAutoCompleteSymbol *sym = *iter;
			if (sym->IsTemporary()) {
				std::vector< IAutoCompleteSymbol * >::reverse_iterator temp = iter;
				fTable.erase( (++temp).base() );				
			}
		}
	}

private:
	std::vector< IAutoCompleteSymbol * > fTable;
};



typedef struct SuggestionInfo
{
	XBOX::VString fDisplayText;
	XBOX::VString fText;

	enum Type {
		eKeyword,
		eTable,
		eView,
		eColumn,
		eSchema,
		e4DMethod,
		eScalarMethod,
		eName,
		eIndex,
		eGroup,
		e4DLocalVariable,
		e4DProcessVariable,
		e4DInterProcessVariable,
		eJavaScriptIdentifier,
		eWidget
	};
	Type fType;

	SuggestionInfo( const XBOX::VString& text, Type type ):
	fDisplayText(text),
	fText(text),
	fType(type)
	{
	}
	
	SuggestionInfo( const XBOX::VString& displayText, const XBOX::VString& contentText, Type type ):
	fDisplayText(displayText),
	fText(contentText),
	fType(type)
	{
	}

} SuggestionInfo;



class SuggestionList : public std::vector<SuggestionInfo*>
{
public:
	virtual ~SuggestionList()
	{
		Reset();
	}
	
	void Suggest( const XBOX::VString& inText, SuggestionInfo::Type inType )
	{
		push_back( new SuggestionInfo( inText, inType ) );
	}

	void Suggest( const XBOX::VString& inDisplayText, const XBOX::VString& inContentText, SuggestionInfo::Type inType )
	{
		push_back( new SuggestionInfo( inDisplayText, inContentText, inType ) );
	}

	void SuggestJavaScriptIdentifiers( void )
	{
		SuggestByCategory( SuggestionInfo::eJavaScriptIdentifier );
	}

	void SuggestTables( void )
	{
		SuggestByCategory( SuggestionInfo::eTable );
	}

	void SuggestViews( void )
	{
		SuggestByCategory( SuggestionInfo::eView );
	}

	void SuggestColumns( void )
	{
		SuggestByCategory( SuggestionInfo::eColumn );
	}

	void SuggestSchemas( void )
	{
		SuggestByCategory( SuggestionInfo::eSchema );
	}

	void Suggest4DMethods( void )
	{
		SuggestByCategory( SuggestionInfo::e4DMethod );
	}

	void Suggest4DLocalVariables( void )
	{
		SuggestByCategory( SuggestionInfo::e4DLocalVariable );
	}

	void Suggest4DProcessVariables( void )
	{
		SuggestByCategory( SuggestionInfo::e4DProcessVariable );
	}

	void Suggest4DInterProcessVariables( void )
	{
		SuggestByCategory( SuggestionInfo::e4DInterProcessVariable );
	}

	void SuggestIndexes( void )
	{
		SuggestByCategory( SuggestionInfo::eIndex );
	}

	void SuggestGroups( void )
	{
		SuggestByCategory( SuggestionInfo::eGroup );
	}

	void UnsuggestByType(SuggestionInfo::Type inType)
	{
		int index = size()-1;
		while(index >= 0)
		{
			SuggestionInfo* currentSuggestion = at(index);
			if( currentSuggestion->fType == inType )
				erase(begin()+index);
			
			index--;
		}
	}
	
	void Reset()
	{
		while( !empty() )
		{
			delete back();
			pop_back();
		}
	}
	
private:
	void SuggestByCategory( SuggestionInfo::Type inType )
	{
		push_back( new SuggestionInfo( CVSTR(""), inType ) );
	}
};

class IDefinition
{
public:
	IDefinition() { }
	IDefinition( Symbols::ISymbol* inSymbol)
	{ 
		if (NULL != inSymbol)
		{
			fName = inSymbol->GetName();
			fLineNumber = inSymbol->GetLineNumber();
			fSymbolID = inSymbol->GetID();

			if ( NULL != inSymbol->GetFile() )
			{
				fFilePath = inSymbol->GetFile()->GetPath();
				fBaseFolder = inSymbol->GetFile()->GetBaseFolder();
			}
		}
	}
	
	IDefinition(const XBOX::VString& inFilePath, const ESymbolFileBaseFolder& inBaseFolder, const sLONG& inLineNumber, const XBOX::VString& inName, const sLONG& inSymbolID) {
		fFilePath	=	inFilePath;
		fBaseFolder =	inBaseFolder;
		fLineNumber =	inLineNumber;
		fName		=	inName;
		fSymbolID	=	inSymbolID;
	}

	void operator = (const IDefinition& inDefinition) {
		fName		=	inDefinition.fName;
		fFilePath	=	inDefinition.fFilePath;
		fBaseFolder =	inDefinition.fBaseFolder;
		fLineNumber =	inDefinition.fLineNumber;
		fSymbolID	=	inDefinition.fSymbolID;
	}

	XBOX::VString			GetName()		{ return fName; }
	XBOX::VString			GetFilePath()	{ return fFilePath; }
	ESymbolFileBaseFolder	GetBaseFolder() { return fBaseFolder; }
	sLONG					GetLineNumber() { return fLineNumber; }
	sLONG					GetSymbolID()	{ return fSymbolID; }

private:
	XBOX::VString			fName;
	XBOX::VString			fFilePath;
	ESymbolFileBaseFolder	fBaseFolder;
	sLONG					fLineNumber;
	sLONG					fSymbolID;
};

class VSymbolFileInfos : public XBOX::VObject
{

public:

	VSymbolFileInfos(const XBOX::VFilePath& inFilePath, const ESymbolFileBaseFolder inBaseFolder, const ESymbolFileExecContext inExecContext)
	{
		fFilePath = inFilePath;
		fBaseFolder = inBaseFolder;
		fExecContext = inExecContext;
	}

	~VSymbolFileInfos() {}

	const XBOX::VFilePath&	GetFilePath() const { return fFilePath; }
	void					SetFilePath( const XBOX::VFilePath &inFilePath ) { fFilePath = inFilePath; }
	ESymbolFileBaseFolder	GetBaseFolder() const { return fBaseFolder; }
	void					SetBaseFolder( const ESymbolFileBaseFolder inBaseFolder ) { fBaseFolder = inBaseFolder; }
	ESymbolFileExecContext	GetExecutionContext() const { return fExecContext; }
	void					SetExecutionContext( const ESymbolFileExecContext inExecContext ) { fExecContext = inExecContext;}


private:
	XBOX::VFilePath			fFilePath;
	ESymbolFileBaseFolder	fBaseFolder;
	ESymbolFileExecContext	fExecContext;

};




class JSLog
{
public:
	JSLog(const bool& inUseTimer=false)
	{
#if ACTIVATE_JSLOG
		fPrinted	=	false;
		
		fTimeStart	=	0;
		
		fUseTimer	=	inUseTimer;
		if( fUseTimer )
			fTimeStart = XBOX::VSystem::GetCurrentTime();
#else
		(void) inUseTimer;
#endif
	}
	
	~JSLog()
	{
#if ACTIVATE_JSLOG
		if( !fPrinted )
			Print();
#endif
	}
	
	void SetTitle(const XBOX::VString& inTitle)
	{
#if ACTIVATE_JSLOG
		fLogMessage.AppendString( CVSTR("[") );
		fLogMessage.AppendString(inTitle);
		fLogMessage.AppendString( CVSTR("]") );
#else
		(void) inTitle;
#endif
	}
	
	void Append(const XBOX::VString& inKey, const sLONG& inValue)
	{
#if ACTIVATE_JSLOG
		_KeyHeader(inKey);
		fLogMessage.AppendLong(inValue);
		_KeyFooter();
#else
		(void) inKey;
		(void) inValue;
#endif
	}
	
	void Append(const XBOX::VString& inKey, const XBOX::VString& inValue)
	{
#if ACTIVATE_JSLOG
		_KeyHeader(inKey);
		fLogMessage.AppendString(inValue);
		_KeyFooter();
#else
		(void) inKey;
		(void) inValue;
#endif
	}
	
	void Print()
	{
#if ACTIVATE_JSLOG
		if( fUseTimer )
		{
			sLONG duration = XBOX::VSystem::GetCurrentTime() - fTimeStart;
			
			_KeyHeader( CVSTR("Duration") );
			fLogMessage.AppendLong(duration);
			fLogMessage.AppendString( CVSTR("ms") );
			_KeyFooter();
		}
		
#if VERSIONDEBUG
		fLogMessage.AppendString( CVSTR("\n") );
		DebugMsg(fLogMessage);
#else
		char buffer[512];
		fLogMessage.ToCString(buffer, 512);
		std::cout << buffer << std::endl;
#endif
		
		fPrinted = true;
		
#endif
	}
	
private:
	void _KeyHeader(const XBOX::VString& inKey)
	{
#if ACTIVATE_JSLOG
		fLogMessage.AppendString( CVSTR(" ") );
		fLogMessage.AppendString(inKey);
		fLogMessage.AppendString( CVSTR("(") );
#else
		(void) inKey;
#endif
	}
	
	void _KeyFooter()
	{
#if ACTIVATE_JSLOG
		fLogMessage.AppendString( CVSTR(")") );
#endif
	}
	
	JSLog(const JSLog& inLog);
	JSLog& operator=(const JSLog& inLog);
	
	bool			fPrinted;
	XBOX::VString	fLogMessage;
	bool			fUseTimer;
	sLONG			fTimeStart;
};


#if _WIN32
	#pragma pack( pop )
#else
	#pragma options align = reset
#endif

#endif
