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
#ifndef __VLANGUAGESYNTAX__
#define __VLANGUAGESYNTAX__

#include "CLanguageSyntax.h"
#include "KernelIPC/Sources/VComponentLibrary.h"
#include "JavaScript/VJavaScript.h"

class VLanguageSyntaxComponent;
class VLanguageSyntaxPlugin;

extern VComponentLibrary *gLanguageSyntaxCompLib;

class LanguageSyntaxComponentLibrary : public VComponentLibrary
{
public:
			LanguageSyntaxComponentLibrary (const CImpDescriptor* inTypeList, sLONG inTypeCount);
	virtual	~LanguageSyntaxComponentLibrary ();
	
protected:
	virtual	void	DoRegister ();
	virtual	void	DoUnregister ();
};


// Registration/creation mecanism as required by VComponentLibrary
const sLONG				kCOMPONENT_TYPE_COUNT	= 1;
const CImpDescriptor	kCOMPONENT_TYPE_LIST[]	= { { CLanguageSyntaxComponent::Component_Type, VImpCreator<VLanguageSyntaxComponent>::CreateImp } };


class VCodeEditorTipInfo : public XBOX::VObject, public ITipInfo
{
public:
	VCodeEditorTipInfo( ICodeEditorDocument *inDocument, XBOX::VString inText, sBYTE inKind ) : fDisplayText( inText ), fCompletionText( inText ), fKind( inKind ), fMacroID( -1 ), fDocument( inDocument ), fView( NULL ) {}
	VCodeEditorTipInfo( ICodeEditorDocument *inDocument, XBOX::VString inDisplayText, XBOX::VString inCompletionText, sBYTE inKind ) : fDisplayText( inDisplayText ), fCompletionText( inCompletionText ), fKind( inKind ), fMacroID( -1 ), fDocument( inDocument ), fView( NULL ) {}
	VCodeEditorTipInfo( ICodeEditorDocument *inDocument, XBOX::VString inDisplayText, sBYTE inKind, sBYTE inMacroID ) : fDisplayText( inDisplayText ), fCompletionText( inDisplayText ), fKind( inKind ), fMacroID( inMacroID ), fDocument( inDocument ), fView( NULL ) {}

	virtual ITipInfo *Clone() const { VCodeEditorTipInfo *tip = new VCodeEditorTipInfo( fDocument, fDisplayText, fCompletionText, fKind ); tip->fMacroID = fMacroID; tip->fView = fView; return tip; }

	virtual void GetStyle( bool& outBold, bool& outItalic, bool& outUnderline, uBYTE &outRed, uBYTE &outGreen, uBYTE &outBlue ) const 
	{	
		fDocument->GetColor( fKind, outRed, outGreen, outBlue );
		fDocument->GetFace( fKind, outBold, outItalic, outUnderline );
	}

	virtual void GetDisplayText( XBOX::VString &outText ) const { outText = fDisplayText; }
	virtual void GetContentText( XBOX::VString &outText ) const { outText = fCompletionText; }
	virtual void SetContentText( XBOX::VString inText ) { fCompletionText = inText; }
	virtual void SetDisplayText( XBOX::VString inText ) { fDisplayText = inText; }
	virtual bool IsSeparator(void) const { return fDisplayText.GetLength() > 0 && fDisplayText[0] == '-'; }
	virtual bool IsMacro(void) const { return fMacroID >= 0; }
	virtual sLONG MacroID(void) const { return fMacroID; }
	virtual short StyleID(void) const { return fKind; }
	virtual void SelfDelete() { this->Release(); }

	virtual void SetUIView( XBOX::VObject* inView ) { fView = inView; }
	virtual XBOX::VObject* GetUIView() const { return fView; }

protected:
	XBOX::VString fDisplayText;
	XBOX::VString fCompletionText;
	short fKind;
	sLONG fMacroID;
	ICodeEditorDocument* fDocument;
	XBOX::VObject* fView;
};


#define LS_TESTER_CATALOG_DATACLASSES	CVSTR("dataClasses")
#define LS_TESTER_CATALOG_NAME			CVSTR("name")
#define LS_TESTER_CATALOG_ATTRIBUTES	CVSTR("attributes")
#define LS_TESTER_CATALOG_METHODS		CVSTR("methods")
#define LS_TESTER_CATALOG_APPLYTO		CVSTR("applyTo")
#define LS_TESTER_CATALOG_TYPE			CVSTR("type")

class VLanguageSyntaxTesterCatalog : public XBOX::VObject, public IEntityModelCatalog
{
public:
	VLanguageSyntaxTesterCatalog(XBOX::VFilePath& inFilePath);
	~VLanguageSyntaxTesterCatalog();

	virtual void					GetCatalogPath( XBOX::VFilePath &outPath ) const { outPath = fFilePath; }

	// from IEntityModelCatalog
	virtual	const XBOX::VValueBag*	RetainCatalogBag() const;
	virtual	void					GetEntityModelNames( XBOX::VectorOfVString& outEntityNames) const;
	virtual	void					GetEntityModelAttributes(const XBOX::VString& inEntityName, std::vector<IEntityModelCatalogAttribute* >& outAttributes) const;
	virtual	void					GetEntityModelMethods(const XBOX::VString& inEntityName, std::vector< IEntityModelCatalogMethod* >& outMethods) const;


private:
	XBOX::VFilePath		fFilePath;
	XBOX::VValueBag*	fCatalogBag;
};

class VLanguageSyntaxTesterCatalogMethod : public XBOX::VObject, public IEntityModelCatalogMethod
{
public:
	VLanguageSyntaxTesterCatalogMethod(XBOX::VString& inName, XBOX::VString& inApplyTo) : fName(inName), fApplyTo(inApplyTo) { }
	~VLanguageSyntaxTesterCatalogMethod() { }

	// from IEntityModelCatalogMethod
	virtual void	GetName(XBOX::VString& outName) { outName = fName; }
	virtual void	GetApplyTo(XBOX::VString& outApplyTo) { outApplyTo = fApplyTo; }

private:
	XBOX::VString fName;
	XBOX::VString fApplyTo;
};

class VLanguageSyntaxTesterCatalogAttribute : public XBOX::VObject, public IEntityModelCatalogAttribute
{
public:
	VLanguageSyntaxTesterCatalogAttribute(XBOX::VString& inName, XBOX::VString& inType) : fName(inName), fType(inType) { }
	~VLanguageSyntaxTesterCatalogAttribute() { }

	// from IEntityModelCatalogAttribute
	virtual void	GetName(XBOX::VString& outName) { outName = fName; }
	virtual void	GetType(XBOX::VString& outType) { outType = fType; }

private:
	XBOX::VString fName;
	XBOX::VString fType;
};



class VJSLanguageSyntaxTester : public XBOX::VJSClass< VJSLanguageSyntaxTester, void >
{
public:
	typedef XBOX::VJSClass< VJSLanguageSyntaxTester, void > inherited;

	static void Initialize( const XBOX::VJSParms_initialize &inParams, void * );
	static void Finalize( const XBOX::VJSParms_finalize &inParams, void * );
	static void GetDefinition ( ClassDefinition &outDefinition );

	static void _GetCompletions( XBOX::VJSParms_callStaticFunction &ioParams, void * );
	static void _GetSymbol( XBOX::VJSParms_callStaticFunction &ioParams, void * );
	static void _GetSyntaxColoring( XBOX::VJSParms_callStaticFunction &ioParams, void * );
	static void _ParseFile( XBOX::VJSParms_callStaticFunction &ioParams, void * );
	static void _Tokenize( XBOX::VJSParms_callStaticFunction &ioParams, void * );
	static void _GotoDefinition( XBOX::VJSParms_callStaticFunction &ioParams, void * );
};

class IJSLanguageSyntaxTesterResults : public XBOX::IRefCountable
{
public:
	virtual size_t GetResultCount() const = 0;
	virtual VString GetResult( size_t inIndex ) = 0;
};

class VJSLanguageSyntaxTesterResults : public XBOX::VJSClass< VJSLanguageSyntaxTesterResults, IJSLanguageSyntaxTesterResults >
{
public:
	typedef XBOX::VJSClass< VJSLanguageSyntaxTesterResults, IJSLanguageSyntaxTesterResults > inherited;

	static void Initialize( const XBOX::VJSParms_initialize &inParams, IJSLanguageSyntaxTesterResults *inResults );
	static void Finalize( const XBOX::VJSParms_finalize &inParams, IJSLanguageSyntaxTesterResults *inResults );
	static void GetDefinition ( ClassDefinition &outDefinition );

	static void _GetResultCount( XBOX::VJSParms_callStaticFunction &ioParams, IJSLanguageSyntaxTesterResults *inResults );
	static void _GetResult( XBOX::VJSParms_callStaticFunction &ioParams, IJSLanguageSyntaxTesterResults *inResults );
};

class IJSLanguageSyntaxTesterColorResults : public XBOX::IRefCountable
{
public:
	virtual size_t GetOffset() const = 0;
	virtual size_t GetLength() const = 0;
	virtual sLONG GetStyle() const = 0;
};

class VJSLanguageSyntaxTesterColorResults : public XBOX::VJSClass< VJSLanguageSyntaxTesterColorResults, IJSLanguageSyntaxTesterColorResults >
{
public:
	typedef XBOX::VJSClass< VJSLanguageSyntaxTesterColorResults, IJSLanguageSyntaxTesterColorResults > inherited;

	static void Initialize( const XBOX::VJSParms_initialize &inParams, IJSLanguageSyntaxTesterColorResults *inResults );
	static void Finalize( const XBOX::VJSParms_finalize &inParams, IJSLanguageSyntaxTesterColorResults *inResults );
	static void GetDefinition ( ClassDefinition &outDefinition );

	static void _GetOffset( XBOX::VJSParms_callStaticFunction &ioParams, IJSLanguageSyntaxTesterColorResults *inResults );
	static void _GetLength( XBOX::VJSParms_callStaticFunction &ioParams, IJSLanguageSyntaxTesterColorResults *inResults );
	static void _GetStyle( XBOX::VJSParms_callStaticFunction &ioParams, IJSLanguageSyntaxTesterColorResults *inResults );
	static void _GetProperty( XBOX::VJSParms_getProperty &ioParams, IJSLanguageSyntaxTesterColorResults *inResults );
};

class IJSLanguageSyntaxTesterDefinitionResults : public XBOX::IRefCountable
{
public:
	virtual size_t GetResultCount() const = 0;
	virtual IDefinition GetResult( size_t inIndex ) = 0;
};

class VJSLanguageSyntaxTesterDefinitionResults : public XBOX::VJSClass< VJSLanguageSyntaxTesterDefinitionResults, IJSLanguageSyntaxTesterDefinitionResults >
{
public:
	typedef XBOX::VJSClass< VJSLanguageSyntaxTesterDefinitionResults, IJSLanguageSyntaxTesterDefinitionResults > inherited;

	static void Initialize( const XBOX::VJSParms_initialize &inParams, IJSLanguageSyntaxTesterDefinitionResults *inResults );
	static void Finalize( const XBOX::VJSParms_finalize &inParams, IJSLanguageSyntaxTesterDefinitionResults *inResults );
	static void GetDefinition ( ClassDefinition &outDefinition );

	static void _GetResultCount( XBOX::VJSParms_callStaticFunction &ioParams, IJSLanguageSyntaxTesterDefinitionResults *inResults );
	static void _GetResult( XBOX::VJSParms_callStaticFunction &ioParams, IJSLanguageSyntaxTesterDefinitionResults *inResults );
};

#endif
