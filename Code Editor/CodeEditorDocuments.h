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
// CodeEditorDocuments.h

#ifndef __CodeEditorDocuments__
#define __CodeEditorDocuments__

#include "KernelIPC/VKernelIPC.h"

#if _WIN32
	#pragma pack( push, 8 )
#else
	#pragma options align = power
#endif

class ISyntaxInterface;
class ISymbolTable;
class ITipInfo;
class IDocumentParserManager;
class ICodeEditorDocument;
class IDefinition;

typedef XBOX::VSignalT_0													VSignal_UpdateParsingComplete;
typedef XBOX::VSignalT_1< XBOX::VRefPtr<ICodeEditorDocument> >				VSignal_UpdateDirtyBit;


class ITipInfo : public XBOX::IRefCountable
{
public:
	virtual ITipInfo *Clone() const = 0;
	
	virtual void GetStyle( bool& outBold, bool& outItalic, bool& outUnderline, uBYTE &outRed, uBYTE &outGreen, uBYTE &outBlue ) const = 0;
	virtual void GetDisplayText( XBOX::VString &outText ) const = 0;
	virtual void GetContentText( XBOX::VString &outText ) const = 0;
	virtual void SetContentText( XBOX::VString inText ) = 0;
	virtual void SetDisplayText( XBOX::VString inText ) = 0;
	virtual bool IsSeparator(void) const = 0;
	virtual bool IsMacro(void) const = 0;
	virtual sLONG MacroID(void) const = 0;
	virtual short StyleID() const = 0;

	virtual void SelfDelete() = 0;

	virtual void SetUIView( XBOX::VObject *inView ) = 0;
	virtual XBOX::VObject *GetUIView() const = 0;
};

class ITipInfoArray : public XBOX::IRefCountable
{
public:
	virtual ITipInfoArray *Clone() = 0;
	virtual void AddTip( ITipInfo *inTip ) = 0;
	virtual void RemoveTip( sLONG inIndex ) = 0;
	virtual ITipInfo *operator[]( sLONG inIndex ) = 0;
	virtual void Clear() = 0;
	virtual sLONG GetCount() const = 0;
	virtual bool Contains( const XBOX::VString &inText, bool inCaseSensitive = false ) = 0;
	virtual void Sort() = 0;
};


// each Syntax can derivate from this class and set those params for every document
// use ICodeEditorDocument::SetPrivateParams and ICodeEditorDocument::GetPrivateParams to set the params
class ISyntaxParams
{
public:
	virtual void SelfDelete() = 0;
};

class IDebuggerClient
{
public:
	virtual void Evaluate(XBOX::VString& inExpression, XBOX::VString& outValue) = 0;
};

// interface
class ICodeEditorDocument : public XBOX::IRefCountable
{
public:
	virtual ISymbolTable*		GetSymbolTable() const = 0;
	virtual void				SetSyntax( ISyntaxInterface* inInterface ) = 0;
	virtual ISyntaxInterface*	GetSyntax() const = 0;
//	virtual void				SetValid( bool inValid ) = 0;
	virtual void				SetPath( const XBOX::VFilePath& inPath ) = 0;
	virtual void				GetPath( XBOX::VFilePath& outPath ) = 0;
	virtual void				SetName( const XBOX::VString& inName ) = 0;
	virtual void				GetName( XBOX::VString& outName ) = 0;
	virtual sLONG				GetBaseFolder() = 0;
	virtual void				SetBaseFolder( const sLONG inBaseFolder ) = 0;
	virtual sLONG				GetExecutionContext() = 0;
	virtual void				SetExecutionContext( const sLONG inExecContext ) = 0;

	virtual void SetLineEnding( XBOX::ECarriageReturnMode inCarriageReturnMode ) = 0;
	virtual XBOX::ECarriageReturnMode GetLineEnding() const = 0;

	virtual void SetCharSet( XBOX::CharSet inCharSet ) = 0;
	virtual XBOX::CharSet GetCharSet() const = 0;

	virtual void DisableProtection() = 0;
	virtual void EnableProtection() = 0;

	// to be used when loading or saving document
	typedef enum { None, ProtectedLine, startOfProtectedBlock, endOfProtectedBlock } LockStatus;
	virtual void SetProtectedStatus( sLONG inLineIndex, LockStatus inStatus ) = 0;
	virtual LockStatus GetProtectedStatus( sLONG inLineIndex ) const = 0;

	// to be used when editing documents
	virtual void SetLineProtected( sLONG inLineIndex, bool inProtected ) = 0;
	virtual bool IsLineProtected( sLONG inLineIndex ) const = 0;

	virtual void SetStyle( sBYTE inStyle, bool inBold, bool inItalic, bool inUnderline, uBYTE inRed, uBYTE inGreen, uBYTE inBlue ) = 0;
	virtual void GetFace( sBYTE inStyle, bool& outBold, bool& outItalic, bool& outUnderline ) = 0;
	virtual void GetColor( sBYTE inStyle, uBYTE& outRed, uBYTE& outGreen, uBYTE& outBlue ) = 0;
	virtual UniChar GetChar( sLONG inIndex ) const = 0;
	virtual void SetLineStyle( sLONG inLineIndex, sLONG inStart, sLONG inEnd, sBYTE inStyle ) = 0;
	virtual void GetLine( sLONG inLineIndex, XBOX::VString& outString ) const = 0;
	virtual void GetStyle( sLONG inLineIndex, const XBOX::VArrayByte*& outStyle ) const = 0;
	virtual void SetFoldable( sLONG inLineIndex, bool inFoldable, bool inMiddleOfBlock = false, bool inExpandOnCR = false ) = 0;
	virtual bool IsFolded( sLONG inLineIndex ) const = 0;
	virtual void SetFolded( sLONG inLineIndex, bool inFolded ) = 0;	// just change line status without changing undo
	virtual void SetEndFoldingString( sLONG inLineIndex, XBOX::VString& inString ) = 0;
	virtual void GetEndFoldingString( sLONG inLineIndex, XBOX::VString& outString ) const = 0;
	virtual	void Fold( bool inFolded ) = 0;	// to be called as a separate action (records undo)
	virtual sLONG GetNbLines() const = 0;
	virtual void SetLineIndent( sLONG inLineIndex, sLONG inIndent ) = 0;
	virtual sLONG GetLineIndent( sLONG inLineIndex ) const = 0;
	virtual void SetLineKind( sLONG inLineIndex, sLONG inKind ) = 0;
	virtual sLONG GetLineKind( sLONG inLineIndex ) const = 0;
	virtual void GetLineSymbols( sLONG inLineIndex, std::vector<sLONG>*& outSymbols ) const = 0;
	// no count : pas de numerotation pour cette ligne, et le goto line l'ignore aussi. Utilise pour les lignes de suite
	virtual void SetLineNoCount( sLONG inLineIndex, bool inNoCount ) = 0;
	virtual bool GetLineNoCount( sLONG inLineIndex ) const = 0;
	virtual sLONG GetNbFoldedLines( sLONG inLineIndex ) const = 0;
	virtual void SetNbFoldedLines( sLONG inLineIndex, sLONG inNbLines ) = 0;
	virtual void GetSelection( sLONG& outStartSelection, sLONG& outEndSelection, sLONG& outFirstVisibleLine, sLONG& outLastVisibleLine ) = 0;
	virtual void Select( sLONG inStartSelection, sLONG inEndSelection, sLONG inFirstVisibleLine, sLONG inLastVisibleLine ) = 0;
	virtual void SelectByTextOffset( sLONG inStartTextOffset, sLONG inEndTextOffset, bool isLeftToRight = true ) = 0;
	virtual void GotoLineIndex( sLONG inLineIndex ) = 0;	// will expand the line to make it visible
	virtual void SelectAll() = 0;
	virtual Boolean IsSelectionEmpty() const = 0;
	virtual bool FindString( const XBOX::VString& inToFind, bool inUp, bool inCaseSensitive, bool inWholeWord, sLONG inStartLineIndex, sLONG inStartOffset, bool inFindProtectedOnly, bool inWrap, bool inDoSelect = true ) = 0;
	virtual void ReplaceAllText( const XBOX::VString& inToFind, const XBOX::VString& inToReplace ) = 0;
	virtual void InsertText( const XBOX::VString& inString, bool inWithTokenization = false ) = 0;
	virtual void SetLineText( sLONG inLineIndex, const XBOX::VString& inText ) = 0;
	virtual void GetSelectedText( XBOX::VString& outString, bool inGetWholeWord = false ) = 0;
	virtual void GetCodeText( XBOX::VString& outText, bool inForSave = true ) = 0;
	virtual void ValidateSave() = 0;
	virtual void SaveDocument( XBOX::VString& outString, XBOX::ECarriageReturnMode& outLineEnding, XBOX::CharSet& outCharset ) = 0;
	virtual void OpenDocument( const XBOX::VString& inStream, ISymbolTable *inSymTable, ISyntaxInterface* inSyntax, XBOX::ECarriageReturnMode inLineEnding, XBOX::CharSet inCharset, bool inUseSuggestions = true ) = 0;
	virtual bool CheckEncoding( XBOX::CharSet inCharSet ) = 0;
	virtual void ResetOutline() = 0;
	virtual sLONG AddToOutline( sLONG inRef, XBOX::VString& inText, XBOX::VArrayByte& inStyle ) = 0;
	virtual sLONG GetLineIndex( sLONG inVisibleLine ) const = 0;
	virtual sLONG LineNumberToLineIndex( sLONG inLineNumber ) const = 0;
	virtual sLONG LineIndexToLineNumber( sLONG inLineIndex ) const = 0;
	virtual sLONG GetVisibleLine( sLONG inLineIndex ) const = 0;
	virtual void SetSyntaxParams( ISyntaxParams* inSyntaxParams, sWORD inIndex = 0 ) = 0;
	virtual ISyntaxParams* GetSyntaxParams( sWORD inIndex = 0 ) const = 0;
	virtual void SetExecutable( sLONG inLineIndex, bool inExecutable ) = 0;
	virtual void SetExecutionPointer( sLONG inLineIndex, bool inTerminalCall, bool inMovable = true ) = 0;
	virtual sLONG GetExecutionPointer( bool& outTerminalCall ) const = 0;
	virtual Boolean IsInSelection( sLONG inVisibleLine, sLONG inPos, bool inAfter ) const = 0;

	virtual ISyntaxParams *GetLineSyntaxParams( sLONG inLineIndex ) = 0;
	// NOTE: the code line assumes ownership of the params at this point, and will release it as needed
	virtual void AssignLineSyntaxParams( sLONG inLineIndex, ISyntaxParams *inParams ) = 0;

	virtual void GetSearchString( XBOX::VString& outString, bool& outCaseSensitive, bool& outFullWord ) = 0;
	virtual bool ShowSearchString() const = 0;

	virtual bool HasBookmark( sLONG inLineIndex ) const = 0;
	virtual void SetBookmark( sLONG inLineIndex ) = 0;

	virtual void GetLineParsingError( sLONG inLineIndex, XBOX::VString& outComment ) const = 0;
	virtual void GetLineCompilerError( sLONG inLineIndex, XBOX::VString& outComment ) const = 0;

	virtual void SetLineParsingError( sLONG inLineIndex, const XBOX::VString& inComment ) = 0;
	virtual void SetLineCompilerError( sLONG inLineIndex, const XBOX::VString& inComment ) = 0;

	virtual void SetCommentText( const XBOX::VString& inText ) = 0;
	virtual void GetCommentText( XBOX::VString& outText ) = 0;

	virtual void ViewInvisibles( bool inView ) = 0;
	virtual bool ShowInvisibles() const = 0;

	virtual bool IsShowLineNumbers() const = 0;

	virtual sLONG GetNextError() = 0;
	virtual sLONG GetPreviousError() = 0;

	virtual void StartMultipleAction() = 0;
	virtual void EndMultipleAction() = 0;

	virtual void SelfDelete() = 0;
	virtual bool IsDirty() const = 0;
	virtual void SetDirty( bool inDirty = true ) = 0;
	virtual Boolean IsEmpty() = 0;
	virtual void SetReadOnly( bool inReadOnly = true ) = 0;
	virtual bool IsReadOnly() const = 0;
	virtual void GetCursorPosition( XBOX::VString& outString ) = 0;
	virtual sLONG GetCursorTextOffset() const = 0;

	virtual VSignal_UpdateParsingComplete&	GetSignalParsingComplete() = 0;
	virtual VSignal_UpdateDirtyBit&			GetSignalUpdateDirtyBit() = 0;

	virtual IDocumentParserManager *GetParserManager() = 0;

	virtual void EmptyUndo() = 0;
	virtual void New() = 0;

	virtual void SetDebuggerClient(IDebuggerClient* inDebuggerClient) { };
	virtual IDebuggerClient* GetDebuggerClient() { return 0; };

	virtual void GetDefinitions(XBOX::VString& outSymbolName, std::vector<IDefinition>& outDefinitions) { }
	virtual void GetDefinitionsSelection(XBOX::VString& outSymbol, std::vector<XBOX::VString>& outParts) { }

	virtual void ParsingComplete( ) { }

	virtual void GetLinePluginParsingError( sLONG inLineIndex, XBOX::VString& outComment ) { }
	virtual void SetLinePluginParsingError( sLONG inLineIndex, const XBOX::VString& inComment ) { }
	virtual void SafeSetLinePluginParsingError( sLONG inLineIndex, const XBOX::VString& inComment ) { }

	virtual XBOX::VIndex FindLineNamedToken( sLONG inLineIndex, const XBOX::VString& inText ) { return 0; }

	virtual bool IsRightToLeftSelection() const =0;
	virtual sLONG LinePositionToTextOffset( sLONG inVisibleLine, sLONG inPosition ) const = 0;
	virtual sLONG MakeLineVisible( sLONG inLineIndex ) = 0;

	virtual sLONG GetFirstSelectedLine() const = 0;
	virtual sLONG GetLastSelectedLine() const = 0;
	virtual sLONG GetStartSelection() const = 0;
	virtual sLONG GetEndSelection() const = 0;
};

class ISyntaxInterface
{
public: 
	// It's time to create the styles or to do any particular init. No text is loaded yet. Called once for all
	virtual void Init( ICodeEditorDocument* inInterface ) = 0;
	// Document is loaded, it's time to parse the content and to make any change in it before editing
	virtual void Load( ICodeEditorDocument* inInterface, XBOX::VString& ioContent ) = 0;
	// Document is saved, it's time to parse the whole content and to make any change in it before saving
	virtual void Save( ICodeEditorDocument* inInterface, XBOX::VString& ioContent ) = 0;
	// Called for each line of document during save, each line can be individually modified during save
	virtual void SaveLine( ICodeEditorDocument* inInterface, sLONG inLineIndex, XBOX::VString& ioContent ) = 0;
	// Document is about to be closed, it's time to do any particular cleanup
	virtual void Close( ICodeEditorDocument* inInterface ) = 0;
	// called each time the content of this particular line has changed
	virtual void SetLine( ICodeEditorDocument* inInterfaces, sLONG inLineIndex, bool inLoading ) = 0;
	virtual sLONG GetIndentWidth() = 0;
	// tell if this particular line has something to deal with collapse/expand
	virtual bool CheckFolding( ICodeEditorDocument* inInterface, sLONG inLineIndex ) = 0;
	// something has changed in collapse/expand, check the whole document to compute if lines are foldables and the number of folded lines
	virtual void ComputeFolding( ICodeEditorDocument* inInterface ) = 0;
	// tell if this particular line has something to deal with outlining
	virtual bool CheckOutline( ICodeEditorDocument* inInterface, sLONG inLineIndex ) = 0;
	// something has changed in outlined lines, check the whole document to compute new outline
	virtual void ComputeOutline( ICodeEditorDocument* inInterface ) = 0;
	// the line content should be retokenized, a vector of string is used to give once a splitted line
	virtual void TokenizeLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, std::vector<XBOX::VString>& ioLines ) = 0;
	// get the text for a particular text offset in a given line. Will be called when the user let the mouse over the text for a while
	virtual void GetTip( ICodeEditorDocument* inInterface, sLONG inLineIndex, sLONG inPos, XBOX::VString& outText, Boolean& outRtfText ) = 0;
	virtual void GetSuggestions( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inPos, ITipInfoArray *outSuggestions, sLONG& outStartOffset, bool inAll ) = 0;
	virtual XBOX::VString GetSuggestionsSeparator() { return CVSTR(""); }
	virtual sLONG GetSuggestionsNumberOfColumns() { return 1; }
	virtual bool HasToSortSuggestions() { return true; }
	
	// Tells the auto-complete engine information about external symbols, aside from
	// just ones that exist within the language itself
	virtual void SetSymbolTable( class IAutoCompleteSymbolTable *inTable ) {}	// This is not pure virtual because it's not required by all implementors
	virtual void SetSymbolTable( ISymbolTable *inTable ) {}						// This is not pure virtual because it's not required by all implementors
	virtual bool TokenizeOnKeyEnter() { return true; }						// This is not pure virtual because it's not required by all implementors
	
	virtual bool UsesSymbolTableForOutlining() const { return false; }

	// tell if the editor allows tabulations
	virtual bool UseTab() = 0;
	virtual sLONG GetTabWidth() const { return 4; }		// tab width is exprimed in characters
	virtual void SetTabWidth( sLONG inTabWidth ) {}		

	// Given a position within a line, determine the boundaries of a word, as well as its length.  This is used to handle situations
	// like double-clicks, Ctrl+Left Arrow, etc.  The behavior depends on the language being dealt with as to what is considered a morpheme.
	// to get standard behaviour return false
	virtual bool DetermineMorphemeBoundary( ICodeEditorDocument *inDocument, sLONG inLineIndex, sLONG inOffset, sLONG &outLeftBoundary, sLONG &outLength, bool inAlternateKey ) = 0;

	// function will most of the time use DetermineMorphemeBoundary but it can also do any other action
	virtual bool DoubleClick( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, bool inAlternateKey ) = 0;

	// trigger one char
	virtual void InsertChar( ICodeEditorDocument* inDocument, UniChar inUnichar, sLONG inLineIndex, sLONG inPosition, ITipInfoArray *outSuggestions, sLONG& outStartOffset ) = 0;
	// on insere des lignes ou l'on en supprime dans l'editeur, il faut mettre a jour les numeros de ligne des breakpoints qui ont ete modifies
	virtual void UpdateBreakPointsLineNumbers( ICodeEditorDocument* inDocument, const std::vector<sWORD>& inBreakIDs, const std::vector<sLONG>& inLineNumbers ) = 0;
	// ajout dans l'editeur d'un breakpoint "simple". En retour il faut donner l'ID identifiant le breakpoint
	virtual void AddBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber, sWORD& outID, bool& outDisabled ) = 0;
	// edition d'un breakpoint par un 'alt-clic' dans l'editeur sur le break point
	virtual bool EditBreakPoint( ICodeEditorDocument* inDocument, sWORD inBreakID, bool& outDisabled ) = 0;
	// suppression d'un break point par un clic sur un breakpoint existant dans l'editeur
	virtual void RemoveBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber ) = 0;
	// les breakpoints ont ete modifies a l'exterieur de l'editeur, l'editeur a ete notifie et demande la nouvelle liste des breakpoints
	virtual bool GetBreakPoints( ICodeEditorDocument* inDocument, std::vector<sLONG>& outLineNumbers, std::vector<sWORD>& outIDs, std::vector<bool>& outDisabled ) = 0;
	// appele chaque fois qu'une ligne contenant un breakpoint est modifie
	virtual void UpdateBreakPointContent( ICodeEditorDocument* inDocument, sWORD inBreakID, const XBOX::VString& inNewLineContent ) = 0;
	// pour les langages compiles, permet de mettre a jour les erreurs de compilation
	virtual void UpdateCompilerErrors( ICodeEditorDocument* inDocument ) = 0;
	// pour passer a l'erreur suivante dans le meme langage mais dans une fenetre differente
	// inNext = true -> erreur suivante, inNext = false -> erreur precedente
	virtual void GotoNextMethodError( ICodeEditorDocument* inDocument, bool inNext ) = 0;
	// called to swap lines of text a=b; become b=a;
	virtual void SwapAssignments( ICodeEditorDocument* inDocument, XBOX::VString& ioString ) = 0;
	// ask if the following line starts with a comment
	virtual bool IsComment( ICodeEditorDocument* inDocument, const XBOX::VString& inString ) = 0;
	// called to swap the following line of code
	virtual void SwapComment( ICodeEditorDocument* inDocument, XBOX::VString& ioString, bool inComment ) = 0;
	// virtual void CallHelp
	virtual void CallHelp( ICodeEditorDocument* inDocument ) = 0;
	// ask if this character should close tip window (and enter its content) in most of the case it will answer true for ( < [ { and so on
	virtual bool ShouldValidateTipWindow( UniChar inChar ) = 0;
	// tells on which characters the matching parenthesis should work. Will ask for <>(){}[]
	// usually programming languages will answer true for (){}[] and XML and HTML will also return true for <>
	virtual bool IsMatchingCharacter( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, UniChar inChar ) = 0;
	virtual bool AllowsThreading() = 0;

	// Some languages should do background processing when "idle" so that they can update possible parsing errors, check
	// for new declarations, etc.
	virtual void PerformIdleProcessing( ICodeEditorDocument *inDocument ) = 0;

	// for all languages that can automatically do some insertion. Maybe we'll have more specific cases to add later...
	virtual void SetAutoInsertParameters( bool inTabs, bool inClosingChar, bool inBlock ) {}

	virtual void GetDefinitions( ICodeEditorDocument* inInterface, XBOX::VString& outSelection, std::vector<IDefinition>& outDefinitions ) {}
	virtual void GetDefinitions(XBOX::VectorOfVString& inParts, std::vector<IDefinition> & outDefinitions) { }
	virtual void GetDefinitionsSelection(ICodeEditorDocument* inInterface, XBOX::VString& outSymbol, std::vector<XBOX::VString>& outParts) { }
	virtual XBOX::VIndex FindLineNamedToken( ICodeEditorDocument* inInterface, sLONG inLineIndex, const XBOX::VString& inText ) { return 0; }

	// These are testing methods which can be implemented by any of the syntax engines
	virtual void GetSuggestionsForTesting( const XBOX::VString &inSourceLine, class ISymbolTable *inSymTable, const XBOX::VString &inFilePathStr, sLONG inLineIndex, sLONG inSuggestionMode, std::vector< XBOX::VString > &outSuggestions ) {}
	virtual void GetColoringForTesting( const XBOX::VString &inSourceLine, std::vector< size_t > &outOffsets, std::vector< size_t > &outLengths, std::vector< sLONG > &outStyles ) {}
	virtual void GetTokensForTesting( XBOX::VString& inSourceCode, std::vector<XBOX::VString>& outTokens ) = 0;
	virtual void GetDefinitionsForTesting( XBOX::VString& inSelection, class ISymbolTable *inSymTable, const XBOX::VString &inFilePathStr, sLONG inLineNumber, std::vector<IDefinition>& outDefinitions ) = 0;
};

class ISourceControlInterface
{
	virtual bool CanEdit( const XBOX::VFilePath& inPath ) = 0;
};


class CCodeEditorComponent : public XBOX::CComponent
{
public:
	enum { Component_Type = 'cecm' };
	//cree un nouveau document, qui peut rester en memoire ou etre affecte a une zone
	virtual ICodeEditorDocument* NewDocument( ISourceControlInterface* inSourceControl = NULL, IDocumentParserManager *inManager = NULL ) = 0;
	virtual bool RunColorPicker( XBOX::VObject* inParentView, uBYTE& outRed, uBYTE& outGreen, uBYTE& outBlue ) = 0;
	virtual bool RunGetFileDialog( XBOX::VObject* inParentView, const XBOX::VFilePath& inDefaultFilePath, XBOX::VString& outRelativePath ) = 0;
};

#endif
