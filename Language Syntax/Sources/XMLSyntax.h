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
#ifndef __XMLSYNTAX__
#define __XMLSYNTAX__

class VXMLSyntax : public VObject, public ISyntaxInterface
{
public:
	VXMLSyntax();
	virtual ~VXMLSyntax();

	// la ligne 
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
	virtual bool DetermineMorphemeBoundary( ICodeEditorDocument *inDocument, sLONG inLineIndex, sLONG inOffset, sLONG &outLeftBoundary, sLONG &outLength, bool inAlternateKey );
	virtual bool DoubleClick( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, bool inAlternateKey );
	virtual bool UseTab();
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
	virtual void GetDefinitionsForTesting( XBOX::VString& inSelection, class ISymbolTable *inSymTable, const XBOX::VString &inFilePathStr, sLONG inLineNumber, std::vector<IDefinition>& outDefinitions ) { }

	virtual bool TokenizeOnKeyEnter() { return false; }

	virtual void PerformIdleProcessing( ICodeEditorDocument *inDocument ) {}

	int findStopLoc(ICodeEditorDocument* inDocument,int inLineNumber,int commenttype);

	int getLineKind_Lex(ICodeEditorDocument* inDocument,int inLineNumber);
	void setLineKind_Lex(ICodeEditorDocument* inDocument, int inLineNumber,int lex_kind);
	


	int getLineKind_Comment(ICodeEditorDocument* inDocument,int inLineNumber,int commenttype);
	void setLineKind_Comment(ICodeEditorDocument * inDocument, int inLineNumber,int comment_value, int commenttype);
	int getFullLindKind_Comment(ICodeEditorDocument* inDocument,int inLineNumber);
	
	
	int getLindKind_Tagnameloc(ICodeEditorDocument* inDocument,int inLineNumber);
	void setLindKind_Tagnameloc(ICodeEditorDocument* inDocument, int inLineNumber,int tagname_location);

	void setIsInpair(ICodeEditorDocument* inDocument, int inLineNumber,bool isinpair);
	int  getIsInpair(ICodeEditorDocument* inDocument, int inLineNumber);

	void parseOneLineByLex(ICodeEditorDocument *&inDocument,sLONG inLineNumber, struct xmlLexStyleInfo *&inputstruct);
	int VSTRNICMP(const char *s1, const char *s2,int maxlen);

	virtual void SetAutoInsertParameters( bool inTabs, bool inClosingChar, bool inBlock, bool inInsertSpaces ) { fAutoInsertTabs = inTabs; fAutoInsertClosingChar = inClosingChar; fAutoInsertBlock = inBlock; fInsertSpaces = inInsertSpaces; }
	virtual void GetAutoInsertParameters( bool& outTabs, bool& outClosingChar, bool& outBlock, bool& outInsertSpaces ) { outTabs = fAutoInsertTabs; outClosingChar = fAutoInsertClosingChar; outBlock = fAutoInsertBlock; outInsertSpaces = fInsertSpaces; }
	virtual bool UseInsertSpacesForTabs() { return fInsertSpaces; }		// insert tab or spaces when user hits tab key or when auto inserting tabs
	virtual void SetTabWidth( sLONG inTabWidth ) {fTabWidth = inTabWidth;}
	virtual sLONG GetTabWidth() const {return fTabWidth;}

private:
	bool fAutoInsertTabs;
	bool fAutoInsertClosingChar;
	bool fAutoInsertBlock;
	bool fInsertSpaces;
	sLONG fTabWidth;

};

#endif