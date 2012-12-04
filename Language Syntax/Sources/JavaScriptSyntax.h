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
#ifndef __JAVASCRIPTSYNTAX__
#define __JAVASCRIPTSYNTAX__

#include "JavaScriptLexer.h"
#include "JavaScriptParser.h"

#if !VERSIONWIN
	#define __cdecl
#endif

typedef struct {
	uLONG fLineKindStartBlocks	: 8;
	uLONG fLineKindEndsBlocks	: 8;
	uLONG fEndsWithOpenComment	: 1;
	uLONG fOpenStringType		: 2;
	uLONG fUnused				: 13;
	
} JavaScriptSyntaxLineKind;

typedef enum {
	eText = 1,
	eDisplayText = 2
};

typedef sLONG EJSCompletionTestingMode;

struct Completion {
	XBOX::VString fText;
	XBOX::VString fDisplayText;
	sBYTE fStyle;
	Completion( VString text, sBYTE style ) : fDisplayText(text), fStyle( style ) { fText  = fDisplayText; }
	Completion( VString displayText, VString text, sBYTE style ) : fDisplayText(displayText), fText( text ), fStyle( style ) {}
};

class VJSSyntaxParams : public VObject, public ISyntaxParams
{
public:
	VJSSyntaxParams () { }
	virtual ~VJSSyntaxParams () { }

private:
	void SelfDelete() {delete this;}
};

class VJavaScriptSyntax : public VObject, public ISyntaxInterface, public JavaScriptParserDelegate
{
private:
	ISymbolTable *fSymTable;
	Symbols::ISymbol *fDeclarationParsingContextSymbol;
	Symbols::IFile *fDocumentFile;
	IBreakPointManager *fBreakPointManager;
	bool fAutoInsertTabs;
	bool fAutoInsertClosingChar;
	bool fAutoInsertBlock;
	bool fInsertSpaces;
	sLONG fTabWidth;

	void CleanTokens( std::vector< class ILexerToken * > &inTokens );

	uLONG8 GetFileModificationStamp( ICodeEditorDocument *inDocument );

	Symbols::ISymbol *LocateSymbol( Symbols::ISymbol *inContext, const VString &inText );
	Symbols::ISymbol *FindNamedDescendant( Symbols::ISymbol* inOwner,  Symbols::IFile *inFile, XBOX::VectorOfVString& inSymbolLineage, sLONG inLevel, bool inKeepUNdefined = true);
	Symbols::IFile *FileFromDocument( ICodeEditorDocument *inDocument );

	void _GetSuggestions( ISymbolTable *inSymTable, Symbols::IFile *inFile, sLONG inLineIndex, XBOX::VString &ioXstr, std::vector< Completion >& inCompletions, XBOX::VString& outOriginal);
	void _SuggestLocalLikeNamedSubSymbols(ISymbolTable *inSymTable, Symbols::ISymbol *inContextSymbol, XBOX::VString inName, std::vector< Completion >& inCompletions);
	void _SuggestGlobalLikeNamedSubSymbols(ISymbolTable *inSymTable, Symbols::IFile *inFile, XBOX::VString inName, std::vector< Completion >& inCompletions);
	void _SuggestSymbol(ISymbolTable *inSymTable, Symbols::ISymbol* inSymbol, const XBOX::VString& inContextName, std::vector< Completion >& inCompletions);
	bool _AreExecutionContextCompatible(Symbols::IFile* inFile1, Symbols::IFile* inFile2);

	#if _DEBUG
		void RunUnitTests();
		static void Test();
	#endif

	virtual void HandleCompileError( XBOX::VFilePath inFile, sLONG inLineNumber, sLONG inStatus, IDocumentParserManager::TaskCookie inCookie );
	virtual void ParsingComplete( XBOX::VFilePath inFile, sLONG inStatus, IDocumentParserManager::TaskCookie inCookie );

public:
	VJavaScriptSyntax();
	virtual ~VJavaScriptSyntax();

	virtual void SetAutoInsertParameters( bool inTabs, bool inClosingChar, bool inBlock, bool inInsertSpaces ) { fAutoInsertTabs = inTabs; fAutoInsertClosingChar = inClosingChar; fAutoInsertBlock = inBlock; fInsertSpaces = inInsertSpaces; }
	virtual void GetAutoInsertParameters( bool& outTabs, bool& outClosingChar, bool& outBlock, bool& outInsertSpaces ) { outTabs = fAutoInsertTabs; outClosingChar = fAutoInsertClosingChar; outBlock = fAutoInsertBlock; outInsertSpaces = fInsertSpaces; }
	virtual bool UseInsertSpacesForTabs() { return fInsertSpaces; }		// insert tab or spaces when user hits tab key or when auto inserting tabs
	virtual void SetTabWidth( sLONG inTabWidth ) {fTabWidth = inTabWidth;}
	virtual sLONG GetTabWidth() const {return fTabWidth;}

	virtual void Error( JavaScriptError::Code code, sLONG line, sLONG offset, void *cookie ) {}	// Unused now that we're using the parser manager
	virtual void ParserOffsetChanged( sLONG offset, void *cookie ) { } 	// Unused now that we're using the parser manager
	virtual void BlockOpener( sLONG line, sLONG offset, void *cookie ) {} 	// Unused now that we're using the parser manager
	virtual void BlockCloser( sLONG line, sLONG offset, void *cookie ){}	// Unused now that we're using the parser manager
	virtual Symbols::ISymbol *GetDeclarationContext( void *cookie );

	virtual void SetSymbolTable( ISymbolTable *inTable ) { if (inTable) {inTable->Retain();} if (fSymTable) { fSymTable->Release(); } fSymTable = inTable; }

	void SetBreakPointManager( IBreakPointManager* inBreakPointManager ) { fBreakPointManager = inBreakPointManager; }

	// ISyntaxInterface implementation
	virtual void Init( ICodeEditorDocument* inDocument );
	virtual void Load( ICodeEditorDocument* inDocument, VString& ioContent );
	virtual void Save( ICodeEditorDocument* inDocument, VString& ioContent );
	virtual void SaveLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, VString& ioContent );
	virtual void Close( ICodeEditorDocument* inDocument );
	virtual void SetLine( ICodeEditorDocument* inDocument, sLONG inLineNumber, bool inLoading );
	virtual sLONG GetIndentWidth();
	virtual bool CheckFolding( ICodeEditorDocument* inDocument, sLONG inLineNumber );
	virtual void ComputeFolding( ICodeEditorDocument* inDocument );
	virtual bool CheckOutline( ICodeEditorDocument* inInterface, sLONG inLineNumber );
	virtual void ComputeOutline( ICodeEditorDocument* inInterface );
	virtual void TokenizeLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, std::vector<XBOX::VString>& ioLines );
	virtual void GetTip( ICodeEditorDocument* inDocument, sLONG inLine, sLONG inPos, VString& outText, Boolean& outRtfText );	
	virtual void GetSuggestions( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inPos, ITipInfoArray *outSuggestions, sLONG& outStartOffset, bool inAll );
	virtual XBOX::VString GetSuggestionsSeparator() { return JavaScriptParser::GetSuggestionsSeparator(); }
	virtual sLONG GetSuggestionsNumberOfColumns() { return JavaScriptParser::GetSuggestionsNumberOfColumns(); }
	virtual bool HasToSortSuggestions() { return false; }
	virtual bool DetermineMorphemeBoundary( ICodeEditorDocument *inDocument, sLONG inLineIndex, sLONG inOffset, sLONG &outLeftBoundary, sLONG &outLength, bool inAlternateKey );
	virtual bool DoubleClick( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, bool inAlternateKey );
	virtual bool UseTab() {return true;};
	virtual void InsertChar( ICodeEditorDocument* inDocument, UniChar inUnichar, sLONG inLineIndex, sLONG inPosition, ITipInfoArray *outSuggestions, sLONG& outStartOffset );
	virtual void UpdateBreakPointsLineNumbers( ICodeEditorDocument* inDocument, const std::vector<sWORD>& inBreakIDs, const std::vector<sLONG>& inLineNumbers );
	virtual void AddBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber, sWORD& outID, bool& outDisabled );
	virtual bool EditBreakPoint( ICodeEditorDocument* inDocument, sWORD inBreakID, bool& outDisabled );
	virtual void RemoveBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber );
	virtual bool GetBreakPoints( ICodeEditorDocument* inDocument, std::vector<sLONG>& outLineNumbers, std::vector<sWORD>& outIDs, std::vector<bool>& outDisabled );
	virtual void UpdateBreakPointContent( ICodeEditorDocument* inDocument, sWORD inBreakID, const XBOX::VString& inNewLineContent );
	virtual void UpdateCompilerErrors( ICodeEditorDocument* inDocument );
	virtual void GotoNextMethodError( ICodeEditorDocument* inDocument, bool inNext );
	virtual void SwapAssignments( ICodeEditorDocument* inDocument, XBOX::VString& ioString );
	virtual bool IsComment( ICodeEditorDocument* inDocument, const XBOX::VString& inString );
	virtual void SwapComment( ICodeEditorDocument* inDocument, XBOX::VString& ioString, bool inComment );
	virtual void CallHelp( ICodeEditorDocument* inDocument );
	virtual bool ShouldValidateTipWindow( UniChar inChar );
	virtual bool IsMatchingCharacter( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, UniChar inChar );
	virtual bool AllowsThreading() { return true; }

	virtual void PerformIdleProcessing( ICodeEditorDocument *inDocument );

	virtual bool UsesSymbolTableForOutlining() const { return true; }

	virtual void GetSuggestionsForTesting( const XBOX::VString &inSourceLine, ISymbolTable *inSymTable, const XBOX::VString &inFilePath, sLONG inLineIndex, EJSCompletionTestingMode inSuggestionMode, std::vector< XBOX::VString > &outSuggestions );
	virtual void GetColoringForTesting( const XBOX::VString &inSourceLine, std::vector< size_t > &outOffsets, std::vector< size_t > &outLengths, std::vector< sLONG > &outStyles );
	virtual void GetTokensForTesting( XBOX::VString& inSourceCode, std::vector<XBOX::VString>& outTokens );
	virtual void GetDefinitionsForTesting( XBOX::VString& inSelection, class ISymbolTable *inSymTable, const XBOX::VString &inFilePathStr, sLONG inLineNumber, std::vector<IDefinition>& outDefinitions );

	virtual bool TokenizeOnKeyEnter() { return false; }

	virtual void GetDefinitions( ICodeEditorDocument* inDocument, XBOX::VString& outSymbolName, std::vector<IDefinition> & outDefinitions);
	virtual void GetDefinitions(XBOX::VectorOfVString& inParts, std::vector<IDefinition> & outDefinitions);
	
	void GetDefinitions(XBOX::VectorOfVString& inParts, Symbols::ISymbol *contextSymbol, Symbols::IFile *inFile, std::vector<IDefinition> & outDefinitions);
	void GetDefinitionsSelection(ICodeEditorDocument* inDocument, XBOX::VString& outSymbol, std::vector<XBOX::VString>& outParts);

	XBOX::VIndex FindLineNamedToken( ICodeEditorDocument* inInterface, sLONG inLineIndex, const XBOX::VString& inText );	
	bool HasToSuggestConstructor(std::vector< ILexerToken * >& inTokens, XBOX::VString& ioXstr);
};

#endif
