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
#ifndef __LanguageSyntaxComponent__
#define __LanguageSyntaxComponent__

extern VLanguageSyntaxComponent* gLanguageSyntax;
class VSQLSyntax;
class VPHPSyntax;
class VJavaScriptSyntax;
class VXMLSyntax;
class VHTMLSyntax;
class VCSSSyntax;			

class VLanguageSyntaxComponent : public VComponentImp<CLanguageSyntaxComponent>
{
public:
	VLanguageSyntaxComponent();
	virtual ~VLanguageSyntaxComponent();

	//initialize localization manager
	virtual void InitLocalizationManager();

	//clear localization manager
	virtual void ClearLocalizationManager();

	//localization manager accessor
	virtual VLocalizationManager* GetLocalizationMgr();

	virtual ISyntaxInterface * GetSyntaxByExtension(const VString &inextension);

	virtual IDocumentParserManager *CreateDocumentParserManager();
	virtual class ISymbolTable *CreateSymbolTable();
	virtual bool ParseScriptDocComment( const XBOX::VString &inComment, std::vector< class IScriptDocCommentField * > &outFields );

	virtual XBOX::VJSObject CreateJavaScriptTestObject( XBOX::VJSContext inJSContext );

	void SetBreakPointManager( ISyntaxInterface* inSyntax, IBreakPointManager* inBreakPointManager );

	virtual void Stop();

protected:
	VLocalizationManager* fDefaultLocalization;
	VFolder* fComponentFolder;
private:
	VSQLSyntax*			fSQLSyntax;
	VPHPSyntax*			fPHPSyntax;
	VJavaScriptSyntax*	fJavaScriptSyntax;
	VXMLSyntax*			fXMLSyntax;
	VHTMLSyntax*		fHTMLSyntax;
	VCSSSyntax*			fCSSSyntax;
};

#endif