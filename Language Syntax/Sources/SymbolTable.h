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
	const XBOX::VString kSymbolFileExtension( "waSym");

	const XBOX::VString kSymbolDataFileExtension( "waSymData");

	// File kind constant strings
	const XBOX::VString kSymbolFileKind( "com.4d.wakanda.symbol");

	const XBOX::VString kSymbolDataFileKind( "com.4d.wakanda.symbol-data");
}

class ISymbolTable *NewSymbolTable();

#if _DEBUG
void IncSym();
void DecSym();
#endif

class VSymbol : public Symbols::ISymbol, public XBOX::VObject
{
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

	mutable VCriticalSection fModificationLock;

	virtual bool ContainsPrototype( ISymbol *inSym ) {
		VTaskLock lock( &fModificationLock );
		if (inSym == NULL || fPrototypes.empty())	return false;
		if (HasID() && inSym->HasID() &&
			GetID() == inSym->GetID())				return true;
		for (std::vector< Symbols::ISymbol * >::iterator iter = fPrototypes.begin(); iter != fPrototypes.end(); ++iter) {
			if (testAssert( *iter )) {
				if ((*iter) == inSym)					return true;
				if ((*iter)->HasID() && inSym->HasID() &&
					(*iter)->GetID() == inSym->GetID())	return true;
				if (static_cast< VSymbol * >( *iter )->ContainsPrototype( inSym ))	return true;
			}
		}
		return false;
	}

	bool VerifyNoRecusivePrototypeChain( ISymbol *inPossiblePrototype ) {
		// Quick tests!
		if (NULL == inPossiblePrototype)				return true;
		if (this == inPossiblePrototype)				return false;
		if (HasID() && inPossiblePrototype->HasID() &&
			GetID() == inPossiblePrototype->GetID())	return false;

		// And make sure our symbol doesn't appear anywhere in the possible prototype's chain.  We do this
		// because that is to be our new prototype.
		if (static_cast< VSymbol * >( inPossiblePrototype )->ContainsPrototype( this ))	return false;
		return true;
	}

public:
	VSymbol() : fFile( NULL ), fOwner( NULL ), fReference( NULL ), fKind( 0 ), fWAFKind( CVSTR("") ),fID( 0 ), fLineNumber( -1 ), fLineCompletion( -1 ) { 
		#if _DEBUG
			IncSym(); 
		#endif
	}
	virtual ~VSymbol() {

		for (std::vector< Symbols::ISymbol * >::iterator iter = fReturnTypes.begin(); iter != fReturnTypes.end(); ++iter) {
			if ( (*iter) != this && (*iter)->GetOwner() != this )
					(*iter)->Release();
		}

		for (std::vector< Symbols::ISymbol * >::iterator iter = fPrototypes.begin(); iter != fPrototypes.end(); ++iter) {
			if ( (*iter) != this )
				(*iter)->Release();
		}

		if (fFile)			fFile->Release();
		if (fOwner)			fOwner->Release();
		if (fReference)		fReference->Release();
		#if _DEBUG
			DecSym();
		#endif
	}

	virtual sLONG GetLineNumber() const { return fLineNumber; }
	virtual sLONG GetLineCompletionNumber() const { return fLineCompletion; }
	virtual void SetLineNumber( sLONG inLineNumber ) { fLineNumber = inLineNumber; }
	virtual void SetLineCompletionNumber( sLONG inLineNumber ) { fLineCompletion = inLineNumber; }

	virtual bool HasID() const { return fID != 0; }
	virtual sLONG GetID() const { return fID; }
	virtual void SetID( sLONG inID ) { fID = inID; }

	virtual VString GetName() const { return fName; }
	virtual void SetName( const VString &inName ) { fName = inName; }

	virtual VString GetTypeName() const {
		VTaskLock lock( &fModificationLock );

		if (fReference)
			return fReference->GetTypeName();
		else if ( IsFunctionKind() )
		{
			if ( GetKind() == Symbols::ISymbol::kKindClass )
				return CVSTR("");
			else if ( GetKind() == Symbols::ISymbol::kKindClassConstructor )
				return GetName();
			else 
			{
				if (!fReturnTypes.empty())
				{
					VString			ret, tmp;
					VectorOfVString	parts;

					for (size_t i = 0; i < fReturnTypes.size(); i++)
					{
						tmp = fReturnTypes[ i ]->GetOwner() ? fReturnTypes[ i ]->GetOwner()->GetName() : fReturnTypes[ i ]->GetName();
						if( find(parts.begin(), parts.end(), tmp) == parts.end() )
						{
							parts.push_back(tmp);
							if (parts.size() > 1 && tmp.GetLength())
								ret += " | ";
							ret += tmp;
						}
					}
					return ret;
				}
				return CVSTR("void");
			}
		}
		else if (!fPrototypes.empty() )
		{
			VString			ret, tmp;
			VectorOfVString	parts;

			for (size_t i = 0; i < fPrototypes.size(); i++)
			{
				tmp = fPrototypes[ i ]->GetOwner() ? fPrototypes[ i ]->GetOwner()->GetName() : fPrototypes[ i ]->GetName();

				// Skip "Function" prototype as we don't ever want to display it!
				if ( tmp == CVSTR("Function"))
					continue;

				// Skip "Object" prototype if object is an instance
				// of several types
				if ( tmp == CVSTR("Object") && fPrototypes.size() > 1)
					continue;

				if( find(parts.begin(), parts.end(), tmp) == parts.end() )
				{
					parts.push_back(tmp);
					if (parts.size() > 1 && tmp.GetLength())
						ret += " | ";
					ret += tmp;
					
				}
			}
			return ret;
		}
		return CVSTR( "" );
	}

	virtual VString GetScriptDocComment() const { return fScriptDocComment; }
	virtual void SetScriptDocComment( const VString &inCommentText ) { fScriptDocComment = inCommentText; }

	virtual int GetFullKindInformation() const { return fKind; }
	virtual int GetAuxillaryKindInformation() const { return (fKind & 0xFFFFFF00); }
	virtual int GetKind() const { return (fKind & 0xFF); }
	virtual void SetKind( int inKind ) { 
		int oldAux = GetAuxillaryKindInformation();
		fKind = inKind | oldAux;
	}
	virtual void AddAuxillaryKindInformation( int inKind ) { fKind |= inKind; }
	virtual void RemoveAuxillaryKindInformation( int inKind ) { fKind &= ~inKind; }

	virtual bool	IsWAFKind() const{ return fWAFKind.GetLength() > 0; }
	virtual VString	GetWAFKind() const { return fWAFKind; }
	virtual void	SetWAFKind( const VString &inWAFKind )
	{ 
		VIndex	index = inWAFKind.Find("@");
		if ( index )
		{
			fWAFKind = inWAFKind;
			fWAFKind.SubString(index, fWAFKind.GetLength() - ( index - 1) );
		}
	}

	virtual VString GetKindString() const
	{
		Symbols::ISymbol *refSym = const_cast< Symbols::ISymbol * >( Dereference() );

		VString kindStr;

		switch ( refSym->GetKind() ) 
		{
		case kKindLocalVariableDeclaration:
			kindStr = CVSTR("Local Variable");
			break;

		case kKindFunctionDeclaration:
			kindStr = CVSTR("Function");
			break;

		case kKindFunctionParameterDeclaration:
			kindStr = CVSTR("Function Parameter");
			break;

		case kKindCatchBlock:
			kindStr = CVSTR("Catch Block");
			break;
			
		case kKindPublicProperty:
			kindStr = CVSTR("Public Property");
			break;

		case kKindPrivateProperty:
			kindStr = CVSTR("Private Property");
			break;

		case kKindStaticProperty:
			kindStr = CVSTR("Static Property");
			break;

		case kKindObjectLiteral:
			kindStr = CVSTR("Object");
			break;

		case kKindTable:
			kindStr = CVSTR("Table");
			break;

		case kKindTableField:
			kindStr = CVSTR("Table Field");
			break;

		case kKindClass:
			kindStr = CVSTR("Class");
			break;

		case kKindClassConstructor:
			kindStr = CVSTR("Constructor");
			break;

		case kKindClassPublicMethod:
			kindStr = CVSTR("Public Method");
			break;

		case kKindClassPrivateMethod:
			kindStr = CVSTR("Private Method");
			break;

		case kKindClassPrivilegedMethod:
			kindStr = CVSTR("Privileged Method");
			break;

		case kKindClassStaticMethod:
			kindStr = CVSTR("Static Method");
			break;

		default:
			kindStr = CVSTR("Unknown Type");
			break;
		}

		refSym->Release();

		return kindStr;
	}

	virtual const std::vector< ISymbol * >& GetPrototypes() const {
		VTaskLock lock( &fModificationLock );
		return fPrototypes;
	}

	virtual void AddPrototype( ISymbol *inPrototype ) {
		VTaskLock lock( &fModificationLock );
		
		if (inPrototype && VerifyNoRecusivePrototypeChain( inPrototype ) && ! ContainsPrototype( inPrototype) )
		{
			if (inPrototype != this)
				inPrototype->Retain();
			fPrototypes.push_back( inPrototype );
		}
	}

	virtual void AddPrototypes( const std::vector< Symbols::ISymbol * > &inSyms ) {
		VTaskLock lock( &fModificationLock );
		for (std::vector< Symbols::ISymbol * >::const_iterator iter = inSyms.begin(); iter != inSyms.end(); ++iter)
		{
			if (*iter && VerifyNoRecusivePrototypeChain( *iter ) && ! ContainsPrototype( *iter ) )
			{
				if (*iter != this)
					(*iter)->Retain();
				fPrototypes.push_back( *iter );
			}
		}
	}

	virtual Symbols::ISymbol *GetOwner() const { return fOwner; }
	virtual void SetOwner( Symbols::ISymbol *inOwner ) {
		if (inOwner)
			inOwner->Retain();
		if (fOwner)	
			fOwner->Release();
		fOwner = inOwner;
	}

	virtual const std::vector< Symbols::ISymbol * >& GetReturnTypes() const {
		VTaskLock lock( &fModificationLock );
		return fReturnTypes;
	}

	virtual void AddReturnType( Symbols::ISymbol *inType) {
		VTaskLock lock( &fModificationLock );

		if (inType) {
			if ( inType != this && inType->GetOwner() != this)
			{
				inType->Retain();
				fReturnTypes.push_back( inType );
			}
		}
	}
	
	virtual void AddReturnTypes( const std::vector< Symbols::ISymbol * > &inSyms ) {
		VTaskLock lock( &fModificationLock );
		for (std::vector< Symbols::ISymbol * >::const_iterator iter = inSyms.begin(); iter != inSyms.end(); ++iter)
		{
			if ( (*iter) != this && (*iter)->GetOwner() != this)
			{
				(*iter)->Retain();
				fReturnTypes.push_back( *iter );
			}
		}
	}

	virtual class Symbols::IFile *GetFile() const { return fFile; }
	virtual void SetFile( class Symbols::IFile *inFile ) {
		if (inFile)
			inFile->Retain();
		if (fFile)	fFile->Release();
		fFile = inFile;
	}

	// Sometimes a symbol is a reference to another symbol.  In that case you can think of the current
	// symbol as more of an opaque placeholder for the reference.  Almost all of the operations which
	// would normally happen on this symbol should actually happen on the referenced symbol.  So we
	// provide a helper function to dereference a symbol -- if the symbol isn't actually a reference, then
	// the deref will just return "this", so it's safe to call on any symbol.
	virtual const Symbols::ISymbol *Dereference() const {
		if ((GetAuxillaryKindInformation() & kReferenceValue) && fReference) {
			// Continue reference chaining
			return fReference->Dereference();
		} else {
			this->Retain();
			// This isn't a reference
			return this;
		}
	}
	virtual void SetReferenceSymbol( Symbols::ISymbol *inReferences ) {
		// We don't want a symbol that can reference itself, so we'll test to see whether that's the case,
		// and break the chain if it is.
		if (inReferences) {
			bool bSafeToAdd = true;
			if (inReferences->HasID() && HasID()) {
				bSafeToAdd = (inReferences->GetID() != GetID());
			} else {
				bSafeToAdd = (inReferences != this);
			}
			if (!bSafeToAdd)	return;
		}

		if (inReferences)
			inReferences->Retain();
		if (fReference)
			fReference->Release();
		fReference = inReferences;

		if (inReferences) {
			fKind |= Symbols::ISymbol::kReferenceValue;
		} else {
			fKind &= ~Symbols::ISymbol::kReferenceValue;
		}
	}

	virtual bool IsFunctionKind() const
	{
		bool ret = false;

		Symbols::ISymbol *refSym = const_cast< Symbols::ISymbol * >( Dereference() );
		if ( refSym->GetKind() == Symbols::ISymbol::kKindFunctionDeclaration ||
			 refSym->GetKind() == Symbols::ISymbol::kKindClass               ||
			 refSym->GetKind() == Symbols::ISymbol::kKindClassConstructor    ||
			 refSym->GetKind() == Symbols::ISymbol::kKindClassPublicMethod   ||
			 refSym->GetKind() == Symbols::ISymbol::kKindClassPrivateMethod  ||
			 refSym->GetKind() == Symbols::ISymbol::kKindClassStaticMethod   ||
			 refSym->GetKind() == Symbols::ISymbol::kKindClassPrivilegedMethod )
			ret = true;
		
		refSym->Release();

		return ret;
	}

	virtual bool IsFunctionExpressionKind() const
	{
		bool ret = false;

		if ( GetAuxillaryKindInformation() & Symbols::ISymbol::kKindFunctionExpression )
			ret = true;

		return ret;
	}

	virtual bool IsFunctionParameterKind() const
	{
		return GetKind() == Symbols::ISymbol::kKindFunctionParameterDeclaration;
	}

	virtual bool IsPublicMethodKind() const
	{
		if ( GetKind() == Symbols::ISymbol::kKindFunctionDeclaration ||
			 GetKind() == Symbols::ISymbol::kKindClassPublicMethod   ||
			 GetKind() == Symbols::ISymbol::kKindClassStaticMethod   ||
			 GetKind() == Symbols::ISymbol::kKindClassPrivilegedMethod )
			 return true;
		return false;
	}

	virtual bool IsPublicPropertyKind() const
	{
		if ( GetKind() == Symbols::ISymbol::kKindPublicProperty ||
			 GetKind() == Symbols::ISymbol::kKindStaticProperty )
			return true;
		return false;
	}

	virtual bool IsPublicKind() const
	{
		return ( IsPublicMethodKind() || IsPublicPropertyKind() );
	}

	virtual bool IsStaticKind() const
	{
		if ( GetKind() == Symbols::ISymbol::kKindStaticProperty ||
			 GetKind() == Symbols::ISymbol::kKindClassStaticMethod )
			 return true;
		return false;
	}

	virtual bool IsPropertyKind() const
	{
		if ( GetKind() == Symbols::ISymbol::kKindPublicProperty ||
			 GetKind() == Symbols::ISymbol::kKindStaticProperty ||
			 GetKind() == Symbols::ISymbol::kKindPrivateProperty )
			return true;
		return false;
	}

	virtual bool IsPrototype() const
	{
		if ( GetKind() == Symbols::ISymbol::kKindPublicProperty && GetName() == CVSTR("prototype") )
			return true;
		return false;
	}

	virtual bool IsObjectLiteral() const
	{
		if ( GetKind() == Symbols::ISymbol::kKindObjectLiteral)
			return true;
		return false;
	}

	virtual bool IsEntityModelKind() const
	{
		if ( GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModel )
			return true;
		return false;
	}

	virtual bool IsEntityModelPropertyOrMethodKind() const
	{
		if ( IsEntityModelMethodKind() || GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModelAttribute )
			return true;
		return false;
	}

	virtual bool IsEntityModelMethodKind() const
	{
		if ( GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModelMethodEntity           ||
			 GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModelMethodEntityCollection ||
			 GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModelMethodDataClass )
			return true;
		return false;
	}

	virtual bool IsDeprecated() const
	{
		bool deprecated = false;

		VString jsDoc = GetScriptDocComment();
		if (! jsDoc.IsEmpty())
		{
			ScriptDocComment *comment = ScriptDocComment::Create( jsDoc );
			if (comment)
			{
				ScriptDocLexer::Element *element = comment->GetElement( IScriptDocCommentField::kDeprecated );
				if (NULL != element)
					deprecated = true;	
			}
			delete comment;
		}

		return deprecated;
	}

	virtual bool IsPrivate() const
	{
		return HasJSDocElement( IScriptDocCommentField::kPrivate );
	}

	bool HasJSDocElement(IScriptDocCommentField::ElementType inType) const
	{
		bool hasElement = false;

		VString jsDoc = GetScriptDocComment();
		if (! jsDoc.IsEmpty())
		{
			ScriptDocComment *comment = ScriptDocComment::Create( jsDoc );
			if (comment)
			{
				ScriptDocLexer::Element *element = comment->GetElement( inType );
				if (NULL != element)
					hasElement = true;	
			}
			delete comment;
		}

		return hasElement;
	}

	virtual bool GetParamValue(sLONG inIndex, std::vector<VString>& outValues) const
	{
		VString jsDoc = GetScriptDocComment();
		if (! jsDoc.IsEmpty())
		{
			ScriptDocComment *comment = ScriptDocComment::Create( jsDoc );
			if (comment)
			{
				std::vector<ScriptDocLexer::Element*> params, paramValues;

				comment->GetElements( IScriptDocCommentField::kParam, params );
				if ( params.size() >= inIndex )
				{
					ScriptDocLexer::ParamElement* param = dynamic_cast< ScriptDocLexer::ParamElement* >( params[ inIndex - 1] );

					comment->GetElements( IScriptDocCommentField::kParamValue, paramValues );
					for (std::vector<ScriptDocLexer::Element*>::iterator it = paramValues.begin(); it != paramValues.end(); ++it)
					{
						if ( param->fName == dynamic_cast< ScriptDocLexer::ParamValueElement* >( *it)->fName )
						{
							VectorOfVString results;
							VString values = dynamic_cast< ScriptDocLexer::ParamValueElement* >( *it )->fValues;

							values.GetSubStrings( CHAR_VERTICAL_LINE, results, false, true);
							if ( results.size() )
								outValues = results;
							break;
						}
					}
				}	
				comment->Release();
			}
		}

		return true;
	}


	virtual bool IsaClass() const
	{
		if ( GetKind() == Symbols::ISymbol::kKindClass || IsEntityModelKind() )
			return true;
		return false;
	}


	virtual VString GetClass() const
	{
		const Symbols::ISymbol* classSym = this;
		while ( NULL != classSym )
		{
			// We've got a class: we can stop moving up the owner chain
			if ( classSym->IsFunctionKind() || IsEntityModelKind() )
				break;
			
			// If current owner has an owner we move up 
			// the owner chain. Else lookup is finished
			if (classSym->GetOwner() && classSym->GetOwner()->GetName().GetLength() )
			{
				classSym = classSym->GetOwner();
				if (classSym->GetName() != CVSTR("prototype") )
					break;
			}
			else
				break;
	
		}

		return ( NULL != classSym ? classSym->GetName() : GetName() );
	}
	
};

class VSymbolFile : public Symbols::IFile, public XBOX::VObject
{
private:
	VString					fFile;
	ESymbolFileBaseFolder	fBaseFolder;
	ESymbolFileExecContext	fExecContext;
	uLONG8					fTimestamp;
	sLONG					fID;

public:
	VSymbolFile() : fTimestamp( 0 ), fID( 0 ) {}
	virtual ~VSymbolFile() {}

	virtual sLONG GetID() const { xbox_assert( fID ); return fID; }
	virtual void SetID( sLONG inID ) { fID = inID; }

	virtual VString GetPath() const {
		return fFile;
	}

	virtual void SetPath( const VString &inPath ) {
		fFile = inPath;
	}

	virtual ESymbolFileBaseFolder GetBaseFolder() const {
		return fBaseFolder;
	}

	void SetBaseFolder( const ESymbolFileBaseFolder inBaseFolder ) {
		fBaseFolder = inBaseFolder;
	}

	virtual ESymbolFileExecContext GetExecutionContext() const {
		return fExecContext;
	}

	virtual void SetExecutionContext( const ESymbolFileExecContext inExecContext ) {
		fExecContext = inExecContext;
	}

	virtual uLONG8 GetModificationTimestamp() const {
		return fTimestamp;
	}

	virtual void SetModificationTimestamp( uLONG8 inTimestamp ) {
		fTimestamp = inTimestamp;
	}
};

class VSymbolExtraInfo : public Symbols::IExtraInfo {
private:
	VString fStringData;
	uLONG fIntegerData;
	const void *fPtrData;
	Kind fKind;

public:
	VSymbolExtraInfo() : fPtrData( NULL ), fIntegerData( 0 ), fKind( kKindUnused ) {}
	
	virtual VString GetStringData() const { return fStringData; }
	virtual void SetStringData( const XBOX::VString &inData ) { fStringData = inData; }

	virtual uLONG GetIntegerData() const { return fIntegerData; }
	virtual void SetIntegerData( uLONG inData ) { fIntegerData = inData; }

	virtual const void *GetNonPersistentCookie() const { return fPtrData; }
	virtual void SetNonPersistentCookie( const void *inData ) { fPtrData = inData; }

	virtual Kind GetKind() const { return fKind; }
	virtual void SetKind( Kind inKind ) { fKind = inKind; }
};
#endif // _SYMBOL_TABLE_H_