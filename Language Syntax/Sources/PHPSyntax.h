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
#ifndef __PHPSYNTAX__
#define __PHPSYNTAX__

class VPHPSyntax : public VObject, public ISyntaxInterface
{
public:
	VPHPSyntax();
	virtual ~VPHPSyntax();

	// ISyntaxInterface implementation 
	virtual void Init( ICodeEditorDocument* inDocument );
	virtual void Load( ICodeEditorDocument* inDocument, VString& ioContent );
	virtual void Save( ICodeEditorDocument* inDocument, VString& ioContent );
	virtual void SaveLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, VString& ioContent );
	virtual void Close( ICodeEditorDocument* inDocument );
	void SetLine( ICodeEditorDocument* inDocument, sLONG inLineNumber, bool inLoading );
	sLONG GetIndentWidth();
	bool CheckFolding( ICodeEditorDocument* inDocument, sLONG inLineNumber );
	void ComputeFolding( ICodeEditorDocument* inDocument );
	bool CheckOutline( ICodeEditorDocument* inInterface, sLONG inLineNumber );
	void ComputeOutline( ICodeEditorDocument* inInterface );
	void TokenizeLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, std::vector<XBOX::VString>& ioLines );
	void GetTip( ICodeEditorDocument* inDocument, sLONG inLine, sLONG inPos, VString& outText, Boolean& outRtfText );
	void GetSuggestions( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inPos, ITipInfoArray *outSuggestions, sLONG& outStartOffset, bool inAll );
	virtual bool DetermineMorphemeBoundary( ICodeEditorDocument *inDocument, sLONG inLineIndex, sLONG inOffset, sLONG &outLeftBoundary, sLONG &outLength, bool inAlternateKey ) {return false;}
	virtual bool DoubleClick( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, bool inAlternateKey );
	bool UseTab() {return true;};
	int findStopLoc(ICodeEditorDocument* inDocument,int inLineNumber);

	int getLineKind_Comment(ICodeEditorDocument* inDocument,int inLineNumber);
	void setLineKind_Comment(ICodeEditorDocument * inDocument, int inLineNumber,int comment_kind);

	int getLineKind_Lex(ICodeEditorDocument* inDocument,int inLineNumber);
	void setLineKind_Lex(ICodeEditorDocument * inDocument, int inLineNumber,int lexlinekind);

	int getLineKind_InfoLoc(ICodeEditorDocument* inDocument,int inLineNumber);
	void setLineKind_InfoLoc(ICodeEditorDocument * inDocument, int inLineNumber,int extraInfoLoc);

	void parseOneLineByLex(ICodeEditorDocument* inDocument,int inLineNumber,struct PHPStyleInfo * inputstruct);
	virtual void InsertChar( ICodeEditorDocument* inDocument, UniChar inUnichar, sLONG inLineIndex, sLONG inPosition, ITipInfoArray *outSuggestions, sLONG& outStartOffset );
	virtual void UpdateBreakPointsLineNumbers( ICodeEditorDocument* inDocument, const std::vector<sWORD>& inBreakIDs, const std::vector<sLONG>& inLineNumbers );
	virtual void AddBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber, sWORD& outID, bool& outDisabled );
	virtual bool EditBreakPoint( ICodeEditorDocument* inDocument, sWORD inBreakID, bool& outDisabled );
	virtual void RemoveBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber );
	virtual bool GetBreakPoints( ICodeEditorDocument* inDocument, std::vector<sLONG>& outLineNumbers, std::vector<sWORD>& outIDs, std::vector<bool>& outDisabled );
	virtual void UpdateBreakPointContent( ICodeEditorDocument* inDocument, sWORD inBreakID, const XBOX::VString& inNewLineContent ) {}
	virtual void UpdateCompilerErrors( ICodeEditorDocument* inDocument );
	virtual void GotoNextMethodError( ICodeEditorDocument* inDocument, bool inNext );
	virtual void SwapAssignments( ICodeEditorDocument* inDocument, XBOX::VString& ioString );
	virtual bool IsComment( ICodeEditorDocument* inDocument, const XBOX::VString& inString );
	virtual void SwapComment( ICodeEditorDocument* inDocument, XBOX::VString& ioString, bool inComment );
	virtual void CallHelp( ICodeEditorDocument* inDocument );
	virtual bool ShouldValidateTipWindow( UniChar inChar );
	virtual bool IsMatchingCharacter( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, UniChar inChar );
	virtual bool AllowsThreading() { return true; }
	
	virtual void GetTokensForTesting( XBOX::VString& inSourceCode, std::vector<XBOX::VString>& outTokens );
	virtual void GetDefinitionsForTesting( XBOX::VString& inSelection, class ISymbolTable *inSymTable, const XBOX::VString &inFilePathStr, sLONG inLineNumber, std::vector<IDefinition>& outDefinitions ) {}

	virtual bool TokenizeOnKeyEnter() { return false; }

	virtual void PerformIdleProcessing( ICodeEditorDocument *inDocument ) {}

	virtual void SetTabWidth( sLONG inTabWidth ) {fTabWidth = inTabWidth;}
	virtual sLONG GetTabWidth() const {return fTabWidth;}

	void PHPQuickSort( VArrayOf<VString> & thearray, int low, int high );
	void binarySearch( ICodeEditorDocument *inDocument, VArrayOf<VString> & source, ITipInfoArray *outSuggestions, VString userinput, int maxsuggestionNum, int color );
	void removeDuplicate( VArrayOf<VString> & source );

	int VSTRNCMP(const char *s1, const char *s2,int maxlen); 
	int VSTRNICMP(const char *s1, const char *s2,int maxlen); 

protected:
	sLONG fTabWidth;
};

#endif


















































