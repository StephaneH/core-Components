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
#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

namespace RIAFileKind
{
	// Extension constant strings
	const XBOX::VString kSymbolFileExtension( "waSym" );

	const XBOX::VString kSymbolDataFileExtension( "waSymData" );

	// File kind constant strings
	const XBOX::VString kSymbolFileKind( "com.4d.wakanda.symbol" );

	const XBOX::VString kSymbolDataFileKind( "com.4d.wakanda.symbol-data" );
}

class ISymbolTable *NewSymbolTable();

#if _DEBUG
void IncSym();
void DecSym();
#endif

class VSymbol : public Symbols::ISymbol, public XBOX::VObject
{
public:
	VSymbol();
	virtual ~VSymbol();

	virtual sLONG GetLineNumber() const;
	virtual void SetLineNumber( sLONG inLineNumber );

	virtual sLONG GetLineCompletionNumber() const;
	virtual void SetLineCompletionNumber( sLONG inLineNumber );

	virtual bool HasID() const;
	virtual sLONG GetID() const;
	virtual void SetID( sLONG inID );

	virtual VString GetName() const;
	virtual void SetName( const VString &inName );

	virtual VString GetTypeName() const;

	virtual VString GetScriptDocComment() const;
	virtual void SetScriptDocComment( const VString &inCommentText );

	virtual int GetFullKindInformation() const;
	virtual int GetAuxillaryKindInformation() const;
	virtual int GetKind() const;
	virtual void SetKind( int inKind );
	virtual void AddAuxillaryKindInformation( int inKind );
	virtual void RemoveAuxillaryKindInformation( int inKind );

	virtual bool	IsWAFKind() const;
	virtual VString	GetWAFKind() const;
	virtual void	SetWAFKind( const VString &inWAFKind );

	virtual VString GetKindString() const;
	virtual VString	GetKindString	(const int& inKind)		const;
	virtual VString	GetAuxKindString(const int& inAuxKind)	const;

	virtual const std::vector<ISymbol*>& GetPrototypes() const;

	virtual void AddPrototype( ISymbol *inPrototype );

	virtual void AddPrototypes( const std::vector< Symbols::ISymbol * > &inSyms );

	virtual Symbols::ISymbol* GetOwner() const;
	virtual void SetOwner( Symbols::ISymbol *inOwner );

	virtual const std::vector<Symbols::ISymbol*>& GetReturnTypes() const;

	virtual void AddReturnType( Symbols::ISymbol *inType);
	
	virtual void AddReturnTypes( const std::vector< Symbols::ISymbol * > &inSyms );

	virtual class Symbols::IFile* GetFile() const;
	virtual void SetFile( class Symbols::IFile *inFile );

	// Sometimes a symbol is a reference to another symbol.  In that case you can think of the current
	// symbol as more of an opaque placeholder for the reference.  Almost all of the operations which
	// would normally happen on this symbol should actually happen on the referenced symbol.  So we
	// provide a helper function to dereference a symbol -- if the symbol isn't actually a reference, then
	// the deref will just return "this", so it's safe to call on any symbol.
	virtual const Symbols::ISymbol* RetainReferencedSymbol() const;
	virtual void SetReferenceSymbol( Symbols::ISymbol *inReferences );

	virtual bool IsFunctionKind() const;
	virtual bool IsFunctionExpressionKind() const;
	virtual bool IsFunctionParameterKind() const;
	virtual bool IsPublicMethodKind() const;
	virtual bool IsPublicPropertyKind() const;
	virtual bool IsPublicKind() const;
	virtual bool IsStaticKind() const;
	virtual bool IsObjectLiteral() const;
	virtual bool IsEntityModelKind() const;
	virtual bool IsEntityModelMethodKind() const;
	virtual bool IsDeprecated() const;
	virtual bool IsPrivate() const;

	bool HasJSDocElement(IScriptDocCommentField::ElementType inType) const;

	virtual bool GetParamValue(sLONG inIndex, std::vector<VString>& outValues) const;

	virtual bool IsaClass() const;
	virtual VString GetClass() const;
	
	virtual void SetUndefinedState(bool inState);
	virtual bool GetUndefinedState() const;
	
	virtual void SetInstanceState(bool inState);
	virtual bool GetInstanceState() const;
	
	virtual void SetReferenceState(bool inState);
	virtual bool GetReferenceState() const;

	virtual void SetEditionState(bool inState);
	virtual bool GetEditionState() const;

	virtual void SetFullName(const XBOX::VString& inName);
	virtual void ComputeFullName();
	virtual XBOX::VString GetFullName() const;

	
private:
	VString				fName;
	VString				fScriptDocComment;
	int					fKind;
	VString				fWAFKind;
	Symbols::IFile*		fFile;
	Symbols::ISymbol*	fOwner;
	Symbols::ISymbol*	fReference;
	sLONG				fID;
	sLONG				fLineNumber;
	sLONG				fLineCompletion;
	
	std::vector< Symbols::ISymbol * > fReturnTypes;
	std::vector< Symbols::ISymbol * > fPrototypes;
	
	virtual bool ContainsPrototype( ISymbol *inSym );
	bool VerifyNoRecusivePrototypeChain( ISymbol *inPossiblePrototype );
	
	bool fUndefinedState;
	bool fInstanceState;
	bool fReferenceState;
	bool fEditionState;

	VString fFullName;
};






class VSymbolFile : public Symbols::IFile, public XBOX::VObject
{
public:
	VSymbolFile();
	virtual ~VSymbolFile();

	virtual sLONG GetID() const;
	virtual void SetID( sLONG inID );

	virtual VString GetPath() const;
	virtual void SetPath( const VString &inPath );

	virtual ESymbolFileBaseFolder GetBaseFolder() const;
	void SetBaseFolder( const ESymbolFileBaseFolder inBaseFolder );

	virtual ESymbolFileExecContext GetExecutionContext() const;
	virtual void SetExecutionContext( const ESymbolFileExecContext inExecContext );

	virtual uLONG8 GetModificationTimestamp() const;
	virtual void SetModificationTimestamp( uLONG8 inTimestamp );
	
	virtual void SetResolveState(bool inState);
	virtual bool GetResolveState() const;
	
private:
	VString					fFile;
	ESymbolFileBaseFolder	fBaseFolder;
	ESymbolFileExecContext	fExecContext;
	uLONG8					fTimestamp;
	sLONG					fID;
	bool					fResolved;
};






class VSymbolExtraInfo : public Symbols::IExtraInfo
{
public:
	VSymbolExtraInfo();
	virtual ~VSymbolExtraInfo();
	
	virtual VString GetStringData() const;
	virtual void SetStringData( const XBOX::VString &inData );

	virtual uLONG GetIntegerData() const;
	virtual void SetIntegerData( uLONG inData );

	virtual const void *GetNonPersistentCookie() const;
	virtual void SetNonPersistentCookie( const void *inData );

	virtual Kind GetKind() const;
	virtual void SetKind( Kind inKind );

private:
	VString		fStringData;
	uLONG		fIntegerData;
	const void*	fPtrData;
	Kind		fKind;
};
#endif // _SYMBOL_TABLE_H_