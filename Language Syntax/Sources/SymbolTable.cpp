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
#include "LanguageSyntaxHeaders.h"
#include "SymbolTable.h"
#include "DB4D/Headers/DB4D.h"

#if _DEBUG
static int symCount = 0;
void IncSym()
{
	++symCount;
}

void DecSym()
{
	--symCount;
}

#endif



bool VSymbol::ContainsPrototype( ISymbol *inSym )
{
	if (inSym == NULL || fPrototypes.empty())
		return false;
	
	if (HasID() && inSym->HasID() && GetID() == inSym->GetID())
		return true;
	
	for (std::vector< Symbols::ISymbol * >::iterator iter = fPrototypes.begin(); iter != fPrototypes.end(); ++iter)
	{
		if (testAssert( *iter ))
		{
			if ((*iter) == inSym)
				return true;
				
			if ((*iter)->HasID() && inSym->HasID() && (*iter)->GetID() == inSym->GetID())
				return true;
				
			if (static_cast< VSymbol * >( *iter )->ContainsPrototype( inSym ))
				return true;
		}
	}
	return false;
}



bool VSymbol::VerifyNoRecusivePrototypeChain( ISymbol *inPossiblePrototype )
{
	// Quick tests!
	if (NULL == inPossiblePrototype)
		return true;
	
	if (this == inPossiblePrototype)
		return false;
	
	if (HasID() && inPossiblePrototype->HasID() && GetID() == inPossiblePrototype->GetID())
		return false;
		
	// And make sure our symbol doesn't appear anywhere in the possible prototype's chain.  We do this
	// because that is to be our new prototype.
	if (static_cast< VSymbol * >( inPossiblePrototype )->ContainsPrototype( this ))
		return false;
	
	return true;
}



VSymbol::VSymbol():
fFile( NULL ),
fOwner( NULL ),
fReference( NULL ),
fKind( 0 ),
fWAFKind( CVSTR("") ),
fID( 0 ),
fLineNumber( -1 ),
fLineCompletion( -1 ),
fUndefinedState( false ),
fInstanceState( false ),
fReferenceState( false ),
fEditionState( false ),
fFullName( "" )
{
#if _DEBUG
		IncSym();
#endif
}



VSymbol::~VSymbol()
{
	for (std::vector< Symbols::ISymbol * >::iterator iter = fReturnTypes.begin(); iter != fReturnTypes.end(); ++iter)
	{
		if ( (*iter) != this && (*iter)->GetOwner() != this )
			(*iter)->Release();
	}
	
	for (std::vector< Symbols::ISymbol * >::iterator iter = fPrototypes.begin(); iter != fPrototypes.end(); ++iter)
	{
		if ( (*iter) != this )
			(*iter)->Release();
	}
	
	if (fFile)
		fFile->Release();
	
	if (fOwner)
		fOwner->Release();
	
	if (fReference)
		fReference->Release();
	
#if _DEBUG
		DecSym();
#endif
}



sLONG VSymbol::GetLineNumber() const
{
	return fLineNumber;
}



sLONG VSymbol::GetLineCompletionNumber() const
{
	return fLineCompletion;
}



void VSymbol::SetLineNumber( sLONG inLineNumber )
{
	fLineNumber = inLineNumber;
}



void VSymbol::SetLineCompletionNumber( sLONG inLineNumber )
{
	fLineCompletion = inLineNumber;
}



bool VSymbol::HasID() const
{
	return fID != 0;
}



sLONG VSymbol::GetID() const
{
	return fID;
}



void VSymbol::SetID( sLONG inID )
{
	fID = inID;
}



VString VSymbol::GetName() const
{
	return fName;
}



void VSymbol::SetName( const VString &inName )
{
	fName = inName;
}



VString VSymbol::GetTypeName() const
{
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



VString VSymbol::GetScriptDocComment() const
{
	return fScriptDocComment;
}



void VSymbol::SetScriptDocComment( const VString &inCommentText )
{
	fScriptDocComment = inCommentText;
}



int VSymbol::GetFullKindInformation() const
{
	return fKind;
}



int VSymbol::GetAuxillaryKindInformation() const
{
	return (fKind & 0xFFFFFF00);
}



int VSymbol::GetKind() const
{
	return (fKind & 0xFF);
}



void VSymbol::SetKind( int inKind )
{
	int oldAux = GetAuxillaryKindInformation();
	fKind = inKind | oldAux;
}



void VSymbol::AddAuxillaryKindInformation( int inKind )
{
	fKind |= inKind;
}



void VSymbol::RemoveAuxillaryKindInformation( int inKind )
{
	fKind &= ~inKind;
}



bool VSymbol::IsWAFKind() const
{
	return fWAFKind.GetLength() > 0;
}



VString	VSymbol::GetWAFKind() const
{
	return fWAFKind;
}



void VSymbol::SetWAFKind( const VString &inWAFKind )
{
	VIndex	index = inWAFKind.Find( "@" );
	if ( index )
	{
		fWAFKind = inWAFKind;
		fWAFKind.SubString(index, fWAFKind.GetLength() - ( index - 1) );
	}
}



VString VSymbol::GetKindString() const
{
	Symbols::ISymbol *refSym = const_cast< Symbols::ISymbol * >( RetainReferencedSymbol() );
		
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

VString	VSymbol::GetKindString(const int& inKind) const
{
	VString msg;
	
	switch ( inKind )
	{
		case kKindLocalVariableDeclaration:
			msg = CVSTR("Local Variable");
			break;
			
		case kKindFunctionDeclaration:
			msg = CVSTR("Function");
			break;
			
		case kKindFunctionParameterDeclaration:
			msg = CVSTR("Function Parameter");
			break;
			
		case kKindCatchBlock:
			msg = CVSTR("Catch Block");
			break;
			
		case kKindPublicProperty:
			msg = CVSTR("Public Property");
			break;
			
		case kKindPrivateProperty:
			msg = CVSTR("Private Property");
			break;
			
		case kKindStaticProperty:
			msg = CVSTR("Static Property");
			break;
			
		case kKindObjectLiteral:
			msg = CVSTR("Object");
			break;
			
		case kKindClass:
			msg = CVSTR("Class");
			break;
			
		case kKindClassConstructor:
			msg = CVSTR("Constructor");
			break;
			
		case kKindClassPublicMethod:
			msg = CVSTR("Public Method");
			break;
			
		case kKindClassPrivateMethod:
			msg = CVSTR("Private Method");
			break;
			
		case kKindClassPrivilegedMethod:
			msg = CVSTR("Privileged Method");
			break;
			
		case kKindClassStaticMethod:
			msg = CVSTR("Static Method");
			break;
			
		default:
			msg = CVSTR("Unknown Type");
			break;
	}
	
	return msg;
}

VString	VSymbol::GetAuxKindString(const int& inAuxKind) const
{
	VString msg;
	
	bool atLeastOneAuxKindMatch = false;
	
	if( fInstanceState )
	{
		msg.AppendString( CVSTR("Instance") );
		atLeastOneAuxKindMatch = true;
	}
	
	if( fReferenceState )
	{
		if( atLeastOneAuxKindMatch )	msg.AppendString( CVSTR(",") );
		msg.AppendString( CVSTR("Reference") );
		atLeastOneAuxKindMatch = true;
	}
	
	if( inAuxKind & Symbols::ISymbol::kKindEntityModel )
	{
		if( atLeastOneAuxKindMatch )	msg.AppendString( CVSTR(",") );
		msg.AppendString( CVSTR("EntityModel") );
		atLeastOneAuxKindMatch = true;
	}
	
	if( inAuxKind & Symbols::ISymbol::kKindEntityModelMethodEntity )
	{
		if( atLeastOneAuxKindMatch )	msg.AppendString( CVSTR(",") );
		msg.AppendString( CVSTR("EntityModelMethodEntity") );
		atLeastOneAuxKindMatch = true;
	}
	
	if( inAuxKind & Symbols::ISymbol::kKindEntityModelMethodEntityCollection )
	{
		if( atLeastOneAuxKindMatch )	msg.AppendString( CVSTR(",") );
		msg.AppendString( CVSTR("EntityModelMethodEntityCollection") );
		atLeastOneAuxKindMatch = true;
	}
	
	if( inAuxKind & Symbols::ISymbol::kKindEntityModelMethodDataClass )
	{
		if( atLeastOneAuxKindMatch )	msg.AppendString( CVSTR(",") );
		msg.AppendString( CVSTR("EntityModelMethodDataClass") );
		atLeastOneAuxKindMatch = true;
	}
	
	if( inAuxKind & Symbols::ISymbol::kKindEntityModelAttribute )
	{
		if( atLeastOneAuxKindMatch )	msg.AppendString( CVSTR(",") );
		msg.AppendString( CVSTR("EntityModelAttribute") );
		atLeastOneAuxKindMatch = true;
	}
	
	if( fUndefinedState )
	{
		if( atLeastOneAuxKindMatch )	msg.AppendString( CVSTR(",") );
		msg.AppendString( CVSTR("Undefined") );
		atLeastOneAuxKindMatch = true;
	}
	
	if( inAuxKind & Symbols::ISymbol::kKindFunctionExpression )
	{
		if( atLeastOneAuxKindMatch )	msg.AppendString( CVSTR(",") );
		msg.AppendString( CVSTR("FunctionExpression") );
	}
	
	return msg;
}

const std::vector<Symbols::ISymbol*>& VSymbol::GetPrototypes() const
{
	return fPrototypes;
}



void VSymbol::AddPrototype( ISymbol *inPrototype )
{
	if (inPrototype && VerifyNoRecusivePrototypeChain( inPrototype ) && ! ContainsPrototype( inPrototype) )
	{
		if (inPrototype != this)
			inPrototype->Retain();
		
		fPrototypes.push_back( inPrototype );
	}
}



void VSymbol::AddPrototypes( const std::vector< Symbols::ISymbol * > &inSyms )
{
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



Symbols::ISymbol* VSymbol::GetOwner() const
{
	return fOwner;
}



void VSymbol::SetOwner( Symbols::ISymbol *inOwner )
{

	if (inOwner)
		inOwner->Retain();
	
	if (fOwner)
		fOwner->Release();
	
	fOwner = inOwner;
}



const std::vector<Symbols::ISymbol*>& VSymbol::GetReturnTypes() const
{
	return fReturnTypes;
}



void VSymbol::AddReturnType( Symbols::ISymbol *inType)
{
	if (inType)
	{
		if ( inType != this && inType->GetOwner() != this)
		{
			inType->Retain();
			fReturnTypes.push_back( inType );
		}
	}
}



void VSymbol::AddReturnTypes( const std::vector< Symbols::ISymbol * > &inSyms )
{
	for (std::vector< Symbols::ISymbol * >::const_iterator iter = inSyms.begin(); iter != inSyms.end(); ++iter)
	{
		if ( (*iter) != this && (*iter)->GetOwner() != this)
		{
			(*iter)->Retain();
			fReturnTypes.push_back( *iter );
		}
	}
}



class Symbols::IFile* VSymbol::GetFile() const
{
	return fFile;
}



void VSymbol::SetFile( class Symbols::IFile *inFile )
{
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
const Symbols::ISymbol* VSymbol::RetainReferencedSymbol() const
{
	if( fReferenceState && fReference)
	{
		// Continue reference chaining
		return fReference->RetainReferencedSymbol();
	}
	else
	{
		this->Retain();
		// This isn't a reference
		return this;
	}
}



void VSymbol::SetReferenceSymbol( Symbols::ISymbol *inReferences )
{
	// We don't want a symbol that can reference itself, so we'll test to see whether that's the case,
	// and break the chain if it is.
	if (inReferences)
	{
		bool bSafeToAdd = true;
		if (inReferences->HasID() && HasID())
		{
			bSafeToAdd = (inReferences->GetID() != GetID());
		}
		else
		{
			bSafeToAdd = (inReferences != this);
		}
		if (!bSafeToAdd)
			return;
	}
		
	if (inReferences)
		inReferences->Retain();
	
	if (fReference)
		fReference->Release();
	
	fReference = inReferences;
		
	if (inReferences)
	{
		SetReferenceState(true);
	}
	else
	{
		SetReferenceState(false);
	}
}



bool VSymbol::IsFunctionKind() const
{
	bool ret = false;
		
	Symbols::ISymbol *refSym = const_cast< Symbols::ISymbol * >( RetainReferencedSymbol() );
	if (	refSym->GetKind() == Symbols::ISymbol::kKindFunctionDeclaration ||
			refSym->GetKind() == Symbols::ISymbol::kKindClass               ||
			refSym->GetKind() == Symbols::ISymbol::kKindClassConstructor    ||
			refSym->GetKind() == Symbols::ISymbol::kKindClassPublicMethod   ||
			refSym->GetKind() == Symbols::ISymbol::kKindClassPrivateMethod  ||
			refSym->GetKind() == Symbols::ISymbol::kKindClassStaticMethod   ||
			refSym->GetKind() == Symbols::ISymbol::kKindClassPrivilegedMethod )
	{
		ret = true;
	}
	
	refSym->Release();
		
	return ret;
}



bool VSymbol::IsFunctionExpressionKind() const
{
	bool ret = false;
		
	if ( GetAuxillaryKindInformation() & Symbols::ISymbol::kKindFunctionExpression )
		ret = true;
		
	return ret;
}



bool VSymbol::IsFunctionParameterKind() const
{
	return GetKind() == Symbols::ISymbol::kKindFunctionParameterDeclaration;
}



bool VSymbol::IsPublicMethodKind() const
{
	if (	GetKind() == Symbols::ISymbol::kKindFunctionDeclaration ||
			GetKind() == Symbols::ISymbol::kKindClassPublicMethod   ||
			GetKind() == Symbols::ISymbol::kKindClassStaticMethod   ||
			GetKind() == Symbols::ISymbol::kKindClassPrivilegedMethod )
	{
		return true;
	}
	
	return false;
}



bool VSymbol::IsPublicPropertyKind() const
{
	if ( GetKind() == Symbols::ISymbol::kKindPublicProperty || 	GetKind() == Symbols::ISymbol::kKindStaticProperty )
		return true;
	
	return false;
}



bool VSymbol::IsPublicKind() const
{
	return ( IsPublicMethodKind() || IsPublicPropertyKind() );
}



bool VSymbol::IsStaticKind() const
{
	if ( GetKind() == Symbols::ISymbol::kKindStaticProperty || 	GetKind() == Symbols::ISymbol::kKindClassStaticMethod )
		return true;

	return false;
}



bool VSymbol::IsObjectLiteral() const
{
	if ( GetKind() == Symbols::ISymbol::kKindObjectLiteral)
		return true;

	return false;
}



bool VSymbol::IsEntityModelKind() const
{
	if ( GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModel )
		return true;
	
	return false;
}



bool VSymbol::IsEntityModelMethodKind() const
{
	if (	GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModelMethodEntity           ||
			GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModelMethodEntityCollection ||
			GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModelMethodDataClass )
	{
		return true;
	}
	
	return false;
}



bool VSymbol::IsDeprecated() const
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



bool VSymbol::IsPrivate() const
{
	return HasJSDocElement( IScriptDocCommentField::kPrivate );
}



bool VSymbol::HasJSDocElement(IScriptDocCommentField::ElementType inType) const
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



bool VSymbol::GetParamValue(sLONG inIndex, std::vector<VString>& outValues) const
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



bool VSymbol::IsaClass() const
{
	if ( GetKind() == Symbols::ISymbol::kKindClass || IsEntityModelKind() )
		return true;

	return false;
}


	
VString VSymbol::GetClass() const
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



void VSymbol::SetUndefinedState(bool inState)
{
	fUndefinedState = inState;
}



bool VSymbol::GetUndefinedState() const
{
	return fUndefinedState;
}



void VSymbol::SetInstanceState(bool inState)
{
	fInstanceState = inState;
}



bool VSymbol::GetInstanceState() const
{
	return fInstanceState;
}



void VSymbol::SetReferenceState(bool inState)
{
	fReferenceState = inState;
}



bool VSymbol::GetReferenceState() const
{
	return fReferenceState;
}



void VSymbol::SetEditionState(bool inState)
{
	fEditionState = inState;
}



bool VSymbol::GetEditionState() const
{
	return fEditionState;
}



void VSymbol::SetFullName(const XBOX::VString& inName)
{
	fFullName = inName;
}



void VSymbol::ComputeFullName()
{
	Symbols::ISymbol* current = const_cast<VSymbol*>(this);

	// Get all owners
	std::vector<Symbols::ISymbol*> chain;
	while( current )
	{
		chain.push_back(current);
		current = current->GetOwner();
	}

	// Construct the name chain
	VString name;
	for(VIndex index=chain.size()-1; index>=0; index--)
	{
		name += "\'";
		name += chain[index]->GetName();
		name += "\'";
		if( index > 0 )
			name += ".";
	}

	this->SetFullName(name);
}



XBOX::VString VSymbol::GetFullName() const
{
	return fFullName;
}










VSymbolFile::VSymbolFile():
fTimestamp( 0 ),
fID( 0 ),
fResolved( true )
{
}



VSymbolFile::~VSymbolFile()
{
}



sLONG VSymbolFile::GetID() const
{
	xbox_assert( fID );
	return fID;
}



void VSymbolFile::SetID( sLONG inID )
{
	fID = inID;
}



VString VSymbolFile::GetPath() const
{
	return fFile;
}



void VSymbolFile::SetPath( const VString &inPath )
{
	fFile = inPath;
}



ESymbolFileBaseFolder VSymbolFile::GetBaseFolder() const
{
	return fBaseFolder;
}



void VSymbolFile::SetBaseFolder( const ESymbolFileBaseFolder inBaseFolder )
{
	fBaseFolder = inBaseFolder;
}



ESymbolFileExecContext VSymbolFile::GetExecutionContext() const
{
	return fExecContext;
}



void VSymbolFile::SetExecutionContext( const ESymbolFileExecContext inExecContext )
{
	fExecContext = inExecContext;
}



uLONG8 VSymbolFile::GetModificationTimestamp() const
{
	return fTimestamp;
}



void VSymbolFile::SetModificationTimestamp( uLONG8 inTimestamp )
{
	fTimestamp = inTimestamp;
}



void VSymbolFile::SetResolveState(bool inState)
{
	fResolved = inState;
}



bool VSymbolFile::GetResolveState() const
{
	return fResolved;
}






VSymbolExtraInfo::VSymbolExtraInfo():
fPtrData( NULL ),
fIntegerData( 0 ),
fKind( kKindUnused )
{
}



VSymbolExtraInfo::~VSymbolExtraInfo()
{
}



VString VSymbolExtraInfo::GetStringData() const
{
	return fStringData;
}



void VSymbolExtraInfo::SetStringData( const XBOX::VString &inData )
{
	fStringData = inData;
}



uLONG VSymbolExtraInfo::GetIntegerData() const
{
	return fIntegerData;
}



void VSymbolExtraInfo::SetIntegerData( uLONG inData )
{
	fIntegerData = inData;
}



const void* VSymbolExtraInfo::GetNonPersistentCookie() const
{
	return fPtrData;
}



void VSymbolExtraInfo::SetNonPersistentCookie( const void *inData )
{
	fPtrData = inData;
}



VSymbolExtraInfo::Kind VSymbolExtraInfo::GetKind() const
{
	return fKind;
}



void VSymbolExtraInfo::SetKind( VSymbolExtraInfo::Kind inKind )
{
	fKind = inKind;
}










// The VSymbolTable class is actually an adapter that wraps around an InnerSymbolTable class.  The
// inner class is what actually performs all of the work.  The VSymbolTable class merely wraps an inner
// inner class and a context object.  We do this so that access to the symbol table can be thread-safe
// in an efficient manner -- the inner class is always handed a context object that is only used for a
// single thread, even if the actual underlying database is shared between threads.  If any APIs are added
// to the VSymbolTable interface, they should also be added to the InnerSymbolTable interface and actually
// implemented there; taking their context object as a parameter.
class VSymbolTable : public ISymbolTable, public VObject
{

	
	friend class SymbolCollectionManager;
	
	
	
public:
	/*
	 *	NEXT GEN MANAGEMENT
	 */
	virtual bool UseNextGen() const;

	/*
	 *	SCOPE MANAGEMENT
	 */
	virtual ISymbolUpdateParameters* RetainSymbolUpdateParameters() const;
	virtual void ReleaseSymbolUpdateParameters(ISymbolUpdateParameters* inUpdateParameters) const;

	virtual bool GetGlobalScope(const ESymbolFileExecContext& inExecutionContext, ISymbolUpdateParameters* inUpdateParameters, sLONG& outScopeId) const;
	virtual void SetGlobalScope(const ESymbolFileExecContext& inExecutionContext, sLONG& outScopeId);

	virtual bool GetScope(const XBOX::VString& inScopeName, const ESymbolScopeType& inScopeType, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, ISymbolUpdateParameters* inUpdateParameters, sLONG& outScopeId) const;
	virtual bool GetScope(const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, const sLONG& inLineNumber, sLONG& outScopeId) const;
	virtual bool GetScope(const XBOX::VString& inScopeName, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromScopeId, const sLONG& inToScopeId, sLONG& outScopeId) const;
	virtual bool GetScopes(const XBOX::VString& inScopeName, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromScopeId, const sLONG& inToScopeId, std::vector<sLONG>& outScopeIds) const;
	virtual bool GetScopes(const XBOX::VString& inScopeName, const std::vector<sLONG>& inFromScopeIds, std::vector<sLONG>& outScopeIds) const;

	virtual void SetScopeFullName(const sLONG inScopeId, const XBOX::VString& inScopeFullName);
	virtual void SetScopeType(const sLONG inScopeId, const ESymbolScopeType& inScopeType);
	virtual void SetScope(const XBOX::VString& inScopeName, const ESymbolScopeType& inScopeType, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset, const bool& inConstructorFlag, sLONG& outScopeId);

	virtual void PrepareUpdates(const sLONG& inSourceId, const bool& inOutlineOnly);
	virtual void FinalizeUpdates(const sLONG& inSourceId, const bool& inOutlineOnly);

	virtual bool GetScopeBoundaries(const sLONG& inScopeId, sLONG& outFromLine, sLONG& outFromOffset, sLONG& outToLine, sLONG& outToOffset) const;
	virtual bool GetScopeSourceId(const sLONG& inScopeId, sLONG& outSourceId) const;
	virtual bool GetScopeName(const sLONG& inScopeId, XBOX::VString& outName) const;
	virtual bool GetScopeFullName(const sLONG& inScopeId, XBOX::VString& outName) const;
	virtual bool GetScopeType(const sLONG& inScopeId, ESymbolScopeType& outType) const;
	virtual bool GetScopeParentId(const sLONG& inScopeId, sLONG& outParentId) const;
	virtual bool GetScopeChildren(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const bool& inUseSourceId, const sLONG& inSourceId, std::vector<sLONG>& outScopeChildrenIds) const;
	virtual bool GetScopeChildren(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inChildName, std::vector<sLONG>& outScopeChildrenIds) const;
	virtual bool GetScopeChild(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inChildName, sLONG& outScopeChildId) const;
	virtual bool GetScopePrototypeId(const XBOX::VString& inScopeName, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromScopeId, const sLONG& inToScopeId, sLONG& outScopeId) const;

	/*
	 *	SOURCE MANAGEMENT
	 */
	virtual void DeleteSource(const sLONG& inSourceId);
	virtual bool GetSourcePath(const sLONG& inSourceId, XBOX::VString& outSourcePath) const;
	virtual bool GetSourceBaseFolder(const sLONG& inSourceId, ESymbolFileBaseFolder& outSourceBaseFolder) const;
	virtual bool GetSourceId(const XBOX::VString& inSourcePath, const ESymbolFileExecContext& inExecutionContext, const ESymbolFileBaseFolder& inBaseFolder, sLONG& outFileId) const;
	virtual bool GetHtmlSourceIdLinkedToJsSourceId(const sLONG& inJsSourceId, sLONG& outHtmlSourceId) const;
	
	/*
	 *	SYMBOL'S NAME MANAGEMENT
	 */
	virtual bool GetName(const XBOX::VString& inSymbolName, const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, ISymbolUpdateParameters* inUpdateParameters, sLONG& outSymbolNameId) const;
	virtual bool GetNames(const XBOX::VString& inSymbolName, const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, ISymbolUpdateParameters* inUpdateParameters, std::vector<sLONG>& outSymbolNameIds) const;
	virtual bool GetName(const sLONG& inNameId, XBOX::VString& outName) const;
	virtual bool GetName(const sLONG& inNameId, const XBOX::VString& inSymbolStartsWith, XBOX::VString& outName) const;
	virtual bool GetNameScopeId(const sLONG& inNameId, sLONG& outScopeId) const;
	virtual bool GetNameExecutionContext(const sLONG& inNameId, ESymbolFileExecContext& outExecutionContext) const;
	virtual bool GetNameFromScope(const XBOX::VString& inSymbolName, const sLONG& inFromScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, sLONG& outNameId) const;
	virtual bool GetNames(const XBOX::VString& inStartWith, const sLONG& inScopeId, std::vector<sLONG>& outNameIds) const;
	
	virtual void SetName(const XBOX::VString& inSymbolName, const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, sLONG& outSymbolNameId);
	/*
	 *	SYMBOL'S DEFINITION MANAGEMENT
	 */
	virtual bool GetDefinition(const sLONG& inScopeId, const sLONG& inNameId, const sLONG& inSourceId, const ESymbolDefinitionKind& inKind, const sLONG& inLineNumber, ISymbolUpdateParameters* inUpdateParameters, sLONG& outDefinitionId) const;
	virtual bool GetDefinitionId(const sLONG& inScopeId, const sLONG& inNameId, const sLONG& inSourceId, sLONG& outDefinitionId) const;
	virtual bool GetDefinitionName(const sLONG& inDefinitionId, XBOX::VString& outName) const;
	virtual bool GetDefinitionNameId(const sLONG& inDefinitionId, sLONG& outNameId) const;
	virtual bool GetDefinitionKind(const sLONG& inDefinitionId, ESymbolDefinitionKind& outKind) const;
	virtual bool GetDefinitionParentScopeId(const sLONG& inDefinitionId, sLONG& outParentScopeId) const;
	virtual bool GetDefinitionStatus(const sLONG& inDefinitionId, ESymbolStatus& outStatus) const;
	virtual bool GetDefinitionSourceId(const sLONG& inDefinitionId, sLONG& outSourceId) const;
	virtual bool GetDefinitionLine(const sLONG& inDefinitionId, sLONG& outLine) const;

	virtual void SetDefinition(const sLONG& inSourceId, const sLONG& inScopeId, const sLONG& inNameId, const ESymbolFileExecContext& inExecutionContext, const ESymbolDefinitionKind& inKind, const sLONG& inLineNumber, sLONG& outDefinitionId);

	/*
	 *	SYMBOL'S SUGGESTION MANAGEMENT
	 */
	virtual void GetConstructorNames(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inStartWith, const sLONG& inFromScope, const sLONG& inToScope, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds) const;
	virtual void GetWidgetNames(const sLONG& inHtmlSourceId, const XBOX::VString& inStartWith, std::vector<XBOX::VString>& outNames) const;
	virtual void GetModelEntityPropertyNames(const XBOX::VString& inModelName, const XBOX::VString& inModelEntityName, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds) const;
	virtual void GetNames(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inStartWith, const sLONG& inFromScope, const sLONG& inToScope, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds) const;
	virtual void GetNames(const XBOX::VString& inStartWith, const std::vector<sLONG>& inScopeIds, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds) const;
	
	/*
	 *	SYMBOL'S PROTOTYPE CHAIN MANAGEMENT
	 */
	virtual bool GetPrototypeChain(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inFromName, const XBOX::VString& inToName, ISymbolUpdateParameters* inUpdateParameters, sLONG& outPrototypeId) const;
	virtual bool GetPrototypeChain(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inFromName, std::vector<XBOX::VString>& outPrototypeNames) const;
	virtual bool GetPrototypeChainParentName(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inFromName, XBOX::VString& outParentPrototypeName) const;
	
	virtual void SetPrototypeChain(const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, const XBOX::VString& inFromName, const XBOX::VString& inToName);
	
	/*
	 *	SYMBOL'S RETURNS CHAIN MANAGEMENT
	 */
	virtual bool GetReturnType(const sLONG& inScopeId, const XBOX::VString& inReturnTypeName, ISymbolUpdateParameters* inUpdateParameters, sLONG& outReturnTypeId) const;
	virtual bool GetReturnTypeNames(const sLONG& inScopeId, std::vector<XBOX::VString>& outNames) const;
	
	virtual void SetReturnType(const sLONG& inSourceId, const sLONG& inScopeId, const XBOX::VString& inTypeName);
	
	/*
	 *	SYMBOL'S TYPES MANAGEMENT
	 */
	virtual bool GetType(const sLONG& inDefinitionId, const XBOX::VString& inTypeName, ISymbolUpdateParameters* inUpdateParameters, sLONG& outTypeId) const;
	virtual bool GetTypeNames(const sLONG& inDefinitionId, std::vector<XBOX::VString>& outNames) const;

	virtual void SetType(const sLONG& inDefinitionId, const sLONG& inSourceId, const XBOX::VString& inTypeName);
	
	/*
	 *	SYMBOL'S REFERENCES MANAGEMENT
	 */
	virtual bool GetReference(const ESymbolReferenceKind& inReferenceKind, const sLONG& inFromId, const sLONG& inToId, ISymbolUpdateParameters* inUpdateParameters, sLONG& outReferenceId) const;
	virtual bool GetReference(const ESymbolReferenceKind& inReferenceKind, const sLONG& inFromId, ISymbolUpdateParameters* inUpdateParameters, std::vector<sLONG>& outReferenceIds) const;
	virtual bool GetReferenceToId(const sLONG& inReferenceId, sLONG& outToId) const;
	virtual bool GetReferenceKind(const sLONG& inReferenceId, ESymbolReferenceKind& outReferenceKind) const;
	virtual bool IsReferenced(const sLONG& inScopeId) const;
	
	virtual void SetReference(const ESymbolReferenceKind& inReferenceKind, const sLONG& inFromId, const sLONG& inToId, const sLONG& inSourceId);

	/*
	 *	SYMBOL'S OUTLINE MANAGEMENT
	 */
	virtual bool GetScopeContentDefinitions(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, std::vector<sLONG>& outIds) const;
	virtual bool GetScopeContentDefinitions(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, std::vector<sLONG>& outIds) const;
	
	/*
	 *	OUTLINE ITEM DATAS MANAGEMENT
	 */
	virtual bool GetOutlineItem(const sLONG& inSourceId, const XBOX::VString& inName, const sLONG& inLine, ISymbolUpdateParameters* inUpdateParameters, sLONG& outItemId) const;
	virtual bool GetOutlineItemSourceId(const sLONG& inItemId, sLONG& outSourceId) const;
	virtual bool GetOutlineItemName(const sLONG& inItemId, XBOX::VString& outName) const;
	virtual bool GetOutlineItemLine(const sLONG& inItemId, sLONG& outLine) const;
	
	virtual void SetOutlineItem(const sLONG& inSourceId, const XBOX::VString& inName, const sLONG& inLine, sLONG& outItemId);

	
	
	VSymbolTable(const bool& inUseNextGen);
	virtual ~VSymbolTable();
	
	virtual void LockUpdates();
	virtual void UnlockUpdates();
	
	virtual void GetRelativePath( const XBOX::VFilePath &inFilePath, const ESymbolFileBaseFolder inBaseFolder, XBOX::VString &outFilePath ) const;
	
	virtual ISymbolTable* CloneForNewThread();
	
	virtual uLONG GetNextUniqueID();
	
	virtual bool OpenSymbolDatabase( const VFile &inDatabaseFile );
	
	virtual bool AddFile( Symbols::IFile* inFile );
	virtual bool AddSymbols( std::vector< Symbols::ISymbol * > inSymbols );
	virtual bool UpdateFile( Symbols::IFile *inFile );
	virtual bool UpdateSymbol( Symbols::ISymbol * inSymbol );
	virtual std::vector< Symbols::ISymbol * > RetainSymbolsByName( const VString &inName );
	virtual std::vector< Symbols::ISymbol * > RetainSymbolsByName( Symbols::ISymbol *inOwner, const VString &inName, Symbols::IFile *inOwnerFile = NULL );
	virtual std::vector< Symbols::ISymbol * > RetainClassSymbols( Symbols::ISymbol *inOwner, const VString &inName, Symbols::IFile *inOwnerFile = NULL );
	virtual std::vector< Symbols::ISymbol * > RetainNamedSubSymbols( Symbols::ISymbol *inOwner, Symbols::IFile *inOwnerFile = NULL, SortingOptions inOptions = kDefault );
	virtual std::vector< Symbols::ISymbol * > GetSymbolsForOutline( Symbols::ISymbol *inOwner, Symbols::IFile *inOwnerFile, SortingOptions inOptions );
	virtual std::vector< Symbols::ISymbol * > RetainLikeNamedSubSymbols( Symbols::ISymbol *inOwner, const VString &inLikeName, const bool inFullyLoad );
	virtual void ReleaseSymbols(std::vector<Symbols::ISymbol*>& inSymbols);
	virtual std::vector< Symbols::IFile * > RetainFilesByPathAndBaseFolder( const VFilePath &inPath, const ESymbolFileBaseFolder inBaseFolder );
	virtual std::vector< Symbols::IFile * > RetainFilesByName( const VString &inName );
	virtual void ReleaseFiles(std::vector<Symbols::IFile*>& inFiles);
	virtual Symbols::ISymbol *GetSymbolOwnerByLine( Symbols::IFile *inOwnerFile, int inLineNumber );
	virtual Symbols::ISymbol *GetSymbolByID( sLONG inID );
	virtual bool DeleteSymbolsForFile( Symbols::IFile *inFile, bool inOnlyEditedSymbols );
	virtual std::vector<Symbols::IExtraInfo*> GetExtraInformation( const Symbols::ISymbol *inOwnerSymbol, Symbols::IExtraInfo::Kind inKind );
	virtual bool AddExtraInformation( const Symbols::ISymbol *inOwnerSymbol, const Symbols::IExtraInfo *inInfo );
	virtual void SetBaseFolderPathStr(const ESymbolFileBaseFolder& inType, const XBOX::VString& inPathStr, const bool& inPosixConvert);
	virtual void GetBaseFolderPathStr(const ESymbolFileBaseFolder& inType, XBOX::VString& outPathStr) const;

	virtual void SetSymbolsEditionState(const Symbols::IFile* inFile, bool inEditionState, std::vector<sLONG>& outIds);
	
	virtual void GetFileIdsReferencingDeadSymbols(const std::vector<sLONG>& inSymbolIds, std::vector<sLONG>& outFileIds);
	
	virtual Symbols::IFile* RetainFileByID(sLONG inID);
	
	virtual void GetSymbolIdsByFileAndEditionState(sLONG inFileId, bool inEditionState, std::vector<sLONG>& outSymbolIds);
	
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	virtual void Dump(const XBOX::VString& inRequest);
#endif
	
private:
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	void _MagicDump(const XBOX::VString& inTraceHeader) const;
#endif
	bool fUseNextGen;

	class SymbolUpdateParameter : public ISymbolUpdateParameters
	{
	public:
		SymbolUpdateParameter();
		virtual ~SymbolUpdateParameter();
		
		virtual bool MustSetStatus() const;
		virtual void MustSetStatus(const bool& inMust);
		
		virtual bool MustGetStatus() const;
		virtual void MustGetStatus(const bool& inMust);
		
		virtual void SetStatus(const ESymbolStatus& inStatus);
		virtual void GetStatus(ESymbolStatus& outStatus) const;
		
		virtual bool MustSetBoundaries() const;
		virtual void MustSetBoundaries(const bool& inMust);
		
		virtual bool MustGetBoundaries() const;
		virtual void MustGetBoundaries(const bool& inMust);
		
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		virtual void GetBoundaries(sLONG& outFromLine, sLONG& outFromOffset, sLONG& outToLine, sLONG& outToOffset) const;
		
		virtual bool MustSetConstructorFlag() const;
		virtual void MustSetConstructorFlag(const bool& inMust);
		
		virtual bool MustGetConstructorFlag() const;
		virtual void MustGetConstructorFlag(const bool& inMust);
		
		virtual void SetConstructorFlag(const bool& inFlag);
		virtual void GetConstructorFlag(bool& outFlag) const;

	private:
		bool			fMustSetStatus;
		bool			fMustGetStatus;
		ESymbolStatus	fStatus;
		
		bool	fMustSetBoundaries;
		bool	fMustGetBoundaries;
		sLONG	fFromLine;
		sLONG	fFromOffset;
		sLONG	fToLine;
		sLONG	fToOffset;

		bool	fMustSetConstructorFlag;
		bool	fMustGetConstructorFlag;
		bool	fConstructorFlag;
	};

	class InnerSymbolTable : public IRefCountable
	{
		
		friend class SymbolCollectionManager;
		
	public:
		/*
		 *	SCOPE MANAGEMENT
		 */
		virtual bool GetGlobalScope(const ESymbolFileExecContext& inExecutionContext, ISymbolUpdateParameters* inUpdateParameters, sLONG& outScopeId, CDB4DBaseContext* inContext) const;
		virtual void SetGlobalScope(const ESymbolFileExecContext& inExecutionContext, sLONG& outScopeId, CDB4DBaseContext* inContext);

		virtual bool GetScope(const XBOX::VString& inScopeName, const ESymbolScopeType& inScopeType, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, ISymbolUpdateParameters* inUpdateParameters, sLONG& outScopeId, CDB4DBaseContext* inContext) const;
		virtual bool GetScope(const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, const sLONG& inLineNumber, sLONG& outScopeId, CDB4DBaseContext* inContext) const;
		virtual bool GetScope(const XBOX::VString& inScopeName, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromScopeId, const sLONG& inToScopeId, sLONG& outScopeId, CDB4DBaseContext* inContext) const;
		virtual bool GetScopes(const XBOX::VString& inScopeName, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromScopeId, const sLONG& inToScopeId, std::vector<sLONG>& outScopeIds, CDB4DBaseContext* inContext) const;
		virtual bool GetScopes(const XBOX::VString& inScopeName, const std::vector<sLONG>& inFromScopeIds, std::vector<sLONG>& outScopeIds, CDB4DBaseContext* inContext) const;

		virtual void SetScopeFullName(const sLONG inScopeId, const XBOX::VString& inScopeFullName, CDB4DBaseContext* inContext);
		virtual void SetScopeType(const sLONG inScopeId, const ESymbolScopeType& inScopeType, CDB4DBaseContext* inContext);
		virtual void SetScope(const XBOX::VString& inScopeName, const ESymbolScopeType& inScopeType, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset, const bool& inConstructorFlag, sLONG& outScopeId, CDB4DBaseContext* inContext);

		virtual void PrepareUpdates(const sLONG& inSourceId, const bool& inOutlineOnly, CDB4DBaseContext* inContext);
		virtual void FinalizeUpdates(const sLONG& inSourceId, const bool& inOutlineOnly, CDB4DBaseContext* inContext);
		
		virtual bool GetScopeBoundaries(const sLONG& inScopeId, sLONG& outFromLine, sLONG& outFromOffset, sLONG& outToLine, sLONG& outToOffset, CDB4DBaseContext* inContext) const;
		virtual bool GetScopeSourceId(const sLONG& inScopeId, sLONG& outSourceId, CDB4DBaseContext* inContext) const;
		virtual bool GetScopeName(const sLONG& inScopeId, XBOX::VString& outName, CDB4DBaseContext* inContext) const;
		virtual bool GetScopeFullName(const sLONG& inScopeId, XBOX::VString& outName, CDB4DBaseContext* inContext) const;
		virtual bool GetScopeType(const sLONG& inScopeId, ESymbolScopeType& outType, CDB4DBaseContext* inContext) const;
		virtual bool GetScopeParentId(const sLONG& inScopeId, sLONG& outParentId, CDB4DBaseContext* inContext) const;
		virtual bool GetScopeChildren(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const bool& inUseSourceId, const sLONG& inSourceId, std::vector<sLONG>& outScopeChildrenIds, CDB4DBaseContext* inContext) const;
		virtual bool GetScopeChildren(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inChildName, std::vector<sLONG>& outScopeChildrenIds, CDB4DBaseContext* inContext) const;
		virtual bool GetScopeChild(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inChildName, sLONG& outScopeChildId, CDB4DBaseContext* inContext) const;

		/*
		 *	SOURCE MANAGEMENT
		 */
		virtual void DeleteSource(const sLONG& inSourceId, CDB4DBaseContext* inContext);
		virtual bool GetSourcePath(const sLONG& inSourceId, XBOX::VString& outSourcePath, CDB4DBaseContext* inContext) const;
		virtual bool GetSourceBaseFolder(const sLONG& inSourceId, ESymbolFileBaseFolder& outSourceBaseFolder, CDB4DBaseContext* inContext) const;
		virtual bool GetSourceId(const XBOX::VString& inSourcePath, const ESymbolFileExecContext& inExecutionContext, const ESymbolFileBaseFolder& inBaseFolder, sLONG& outFileId, CDB4DBaseContext* inContext) const;

	private:
		void _PrepareUpdates(const sLONG& inSourceId, CDB4DTable* inTable, CDB4DField* inSourceField, CDB4DField* inStatusField, CDB4DBaseContext* inContext);
		void _FinalizeUpdates(const sLONG& inSourceId, CDB4DTable* inTable, CDB4DField* inSourceField, CDB4DField* inStatusField, CDB4DBaseContext* inContext);
		void _DropSymbols(const sLONG& inSourceId, CDB4DTable* inTable, CDB4DField* inSourceField, CDB4DBaseContext* inContext);

	public:
		/*
		 *	SYMBOL'S NAME MANAGEMENT
		 */
		virtual bool GetName(const XBOX::VString& inSymbolName, const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, ISymbolUpdateParameters* inUpdateParameters, sLONG& outSymbolNameId, CDB4DBaseContext* inContext) const;
		virtual bool GetNames(const XBOX::VString& inSymbolName, const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, ISymbolUpdateParameters* inUpdateParameters, std::vector<sLONG>& outSymbolNameIds, CDB4DBaseContext* inContext) const;
		virtual bool GetName(const sLONG& inNameId, XBOX::VString& outName, CDB4DBaseContext* inContext) const;
		virtual bool GetName(const sLONG& inNameId, const XBOX::VString& inSymbolStartsWith, XBOX::VString& outName, CDB4DBaseContext* inContext) const;
		virtual bool GetNameScopeId(const sLONG& inNameId, sLONG& outScopeId, CDB4DBaseContext* inContext) const;
		virtual bool GetNameExecutionContext(const sLONG& inNameId, ESymbolFileExecContext& outExecutionContext, CDB4DBaseContext* inContext) const;
		virtual bool GetNameFromScope(const XBOX::VString& inSymbolName, const sLONG& inFromScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, sLONG& outNameId, CDB4DBaseContext* inContext) const;
		virtual bool GetNames(const XBOX::VString& inStartWith, const sLONG& inScopeId, std::vector<sLONG>& outNameIds, CDB4DBaseContext* inContext) const;

		virtual void SetName(const XBOX::VString& inSymbolName, const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, sLONG& outSymbolNameId, CDB4DBaseContext* inContext);

		/*
		 *	SYMBOL'S DEFINITION MANAGEMENT
		 */
		virtual bool GetDefinition(const sLONG& inScopeId, const sLONG& inNameId, const sLONG& inSourceId, const ESymbolDefinitionKind& inKind, const sLONG& inLineNumber, ISymbolUpdateParameters* inUpdateParameters, sLONG& outDefinitionId, CDB4DBaseContext* inContext) const;
		virtual bool GetDefinitionId(const sLONG& inScopeId, const sLONG& inNameId, const sLONG& inSourceId, sLONG& outDefinitionId, CDB4DBaseContext* inContext) const;
		virtual bool GetDefinitionName(const sLONG& inDefinitionId, XBOX::VString& outName, CDB4DBaseContext* inContext) const;
		virtual bool GetDefinitionNameId(const sLONG& inDefinitionId, sLONG& outNameId, CDB4DBaseContext* inContext) const;
		virtual bool GetDefinitionKind(const sLONG& inDefinitionId, ESymbolDefinitionKind& outKind, CDB4DBaseContext* inContext) const;
		virtual bool GetDefinitionParentScopeId(const sLONG& inDefinitionId, sLONG& outParentScopeId, CDB4DBaseContext* inContext) const;
		virtual bool GetDefinitionStatus(const sLONG& inDefinitionId, ESymbolStatus& outStatus, CDB4DBaseContext* inContext) const;
		virtual bool GetDefinitionSourceId(const sLONG& inDefinitionId, sLONG& outSourceId, CDB4DBaseContext* inContext) const;
		virtual bool GetDefinitionLine(const sLONG& inDefinitionId, sLONG& outLine, CDB4DBaseContext* inContext) const;

		virtual void SetDefinition(const sLONG& inSourceId, const sLONG& inScopeId, const sLONG& inNameId, const ESymbolFileExecContext& inExecutionContext, const ESymbolDefinitionKind& inKind, const sLONG& inLineNumber, sLONG& outDefinitionId, CDB4DBaseContext* inContext);

		/*
		 *	SYMBOL'S SUGGESTION MANAGEMENT
		 */
		virtual void GetConstructorNames(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inStartWith, const sLONG& inFromScope, const sLONG& inToScope, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds, CDB4DBaseContext* inContext) const;
		virtual void GetWidgetNames(const sLONG& inHtmlSourceId, const XBOX::VString& inStartWith, std::vector<XBOX::VString>& outNames, CDB4DBaseContext* inContext) const;
		virtual void GetModelEntityPropertyNames(const XBOX::VString& inModelName, const XBOX::VString& inModelEntityName, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds, CDB4DBaseContext* inContext) const;
		virtual void GetNames(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inStartWith, const sLONG& inFromScope, const sLONG& inToScope, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds, CDB4DBaseContext* inContext) const;
		virtual void GetNames(const XBOX::VString& inStartWith, const std::vector<sLONG>& inScopeIds, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds, CDB4DBaseContext* inContext) const;

		/*
		 *	SYMBOL'S PROTOTYPE CHAIN MANAGEMENT
		 */
		virtual bool GetPrototypeChain(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inFromName, const XBOX::VString& inToName, ISymbolUpdateParameters* inUpdateParameters, sLONG& outPrototypeId, CDB4DBaseContext* inContext) const;
		virtual bool GetPrototypeChain(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inFromName, std::vector<XBOX::VString>& outPrototypeNames, CDB4DBaseContext* inContext) const;
		virtual bool GetPrototypeChainParentName(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inFromName, XBOX::VString& outParentPrototypeName, CDB4DBaseContext* inContext) const;
		
		virtual void SetPrototypeChain(const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, const XBOX::VString& inFromName, const XBOX::VString& inToName, CDB4DBaseContext* inContext);
		
		/*
		 *	SYMBOL'S RETURNS CHAIN MANAGEMENT
		 */
		virtual bool GetReturnType(const sLONG& inScopeId, const XBOX::VString& inReturnTypeName, ISymbolUpdateParameters* inUpdateParameters, sLONG& outReturnTypeId, CDB4DBaseContext* inContext) const;
		virtual bool GetReturnTypeNames(const sLONG& inScopeId, std::vector<XBOX::VString>& outNames, CDB4DBaseContext* inContext) const;
		
		virtual void SetReturnType(const sLONG& inSourceId, const sLONG& inScopeId, const XBOX::VString& inTypeName, CDB4DBaseContext* inContext);
		
		/*
		 *	SYMBOL'S TYPES MANAGEMENT
		 */
		virtual bool GetType(const sLONG& inDefinitionId, const XBOX::VString& inTypeName, ISymbolUpdateParameters* inUpdateParameters, sLONG& outTypeId, CDB4DBaseContext* inContext) const;
		virtual bool GetTypeNames(const sLONG& inDefinitionId, std::vector<XBOX::VString>& outNames, CDB4DBaseContext* inContext) const;
		
		virtual bool HasUpToDateDefinedType(const sLONG& inDefinitionId, CDB4DBaseContext* inContext);
		
		virtual void SetType(const sLONG& inDefinitionId, const sLONG& inSourceId, const XBOX::VString& inTypeName, CDB4DBaseContext* inContext);

		/*
		 *	SYMBOL'S REFERENCES MANAGEMENT
		 */
		virtual bool GetReference(const ESymbolReferenceKind& inReferenceKind, const sLONG& inFromId, const sLONG& inToId, ISymbolUpdateParameters* inUpdateParameters, sLONG& outReferenceId, CDB4DBaseContext* inContext) const;
		virtual bool GetReference(const ESymbolReferenceKind& inReferenceKind, const sLONG& inFromId, ISymbolUpdateParameters* inUpdateParameters, std::vector<sLONG>& outReferenceIds, CDB4DBaseContext* inContext) const;
		virtual bool GetReferenceToId(const sLONG& inReferenceId, sLONG& outToId, CDB4DBaseContext* inContext) const;
		virtual bool GetReferenceKind(const sLONG& inReferenceId, ESymbolReferenceKind& outReferenceKind, CDB4DBaseContext* inContext) const;
		virtual bool IsReferenced(const sLONG& inScopeId, CDB4DBaseContext* inContext) const;

		virtual void SetReference(const ESymbolReferenceKind& inReferenceKind, const sLONG& inFromId, const sLONG& inToId, const sLONG& inSourceId, CDB4DBaseContext* inContext);

		/*
		 *	SYMBOL'S OUTLINE MANAGEMENT
		 */
		virtual bool GetScopeContentDefinitions(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, std::vector<sLONG>& outIds, CDB4DBaseContext* inContext) const;
		virtual bool GetScopeContentDefinitions(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, std::vector<sLONG>& outIds, CDB4DBaseContext* inContext) const;
		
		/*
		 *	OUTLINE ITEM DATAS MANAGEMENT
		 */
		virtual bool GetOutlineItem(const sLONG& inSourceId, const XBOX::VString& inName, const sLONG& inLine, ISymbolUpdateParameters* inUpdateParameters, sLONG& outItemId, CDB4DBaseContext* inContext) const;
		virtual bool GetOutlineItemSourceId(const sLONG& inItemId, sLONG& outSourceId, CDB4DBaseContext* inContext) const;
		virtual bool GetOutlineItemName(const sLONG& inItemId, XBOX::VString& outName, CDB4DBaseContext* inContext) const;
		virtual bool GetOutlineItemLine(const sLONG& inItemId, sLONG& outLine, CDB4DBaseContext* inContext) const;
		
		virtual void SetOutlineItem(const sLONG& inSourceId, const XBOX::VString& inName, const sLONG& inLine, sLONG& outItemId, CDB4DBaseContext* inContext);

	private:
		/*
		 *	SYMBOL'S CORE MANAGEMENT
		 */
		void _InsertECMA262CoreSymbols(const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsScope							(const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext, sLONG& outGlobalScopeId);
		void _ECMA262_InsertGlobalObjectSymbolsReservedKeywords					(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsReservedLiterals					(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsValueProperties					(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsFunctionProperties				(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsURIHandlingFunctionProperties	(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsObjectProperties					(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsConstructorObject				(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsConstructorFunction				(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsConstructorArray					(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsConstructorString				(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsConstructorBoolean				(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsConstructorNumber				(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsConstructorDate					(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsConstructorRegExp				(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsConstructorError					(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsConstructorEvalError				(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsConstructorRangeError			(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsConstructorReferenceError		(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsConstructorSyntaxError			(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsConstructorTypeError				(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);
		void _ECMA262_InsertGlobalObjectSymbolsConstructorURIError				(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext);

		bool fUseNextGen;
		
	public:
		// FM : try to optimize access to symbol table by using a sync event
		// so it will lock all updates to database by all the threads parsing files
		// when a user action requires some suggestions.
		virtual void LockUpdates()
		{
			fAllowedToAddSymbolsEvent.Reset();
		}
		
		virtual void UnlockUpdates()
		{
			fAllowedToAddSymbolsEvent.Unlock();
		}
		
		InnerSymbolTable(const bool& inUseNextGen);
		virtual ~InnerSymbolTable();
		
		CDB4DBaseContext *GetContextForNewThread();
		uLONG GetNextUniqueID() { return (uLONG)VInterlocked::Increment( (sLONG *)&fNextUniqueSymbolID ); }
		
		virtual bool OpenSymbolDatabase( const VFile &inDatabaseFile, CDB4DBaseContext **outContext );
		virtual bool AddFile( Symbols::IFile* inFile, CDB4DBaseContext *inContext );
		virtual bool AddSymbols( std::vector< Symbols::ISymbol * > inSymbols, CDB4DBaseContext *inContext );
		
		virtual bool UpdateFile( Symbols::IFile *inFile, CDB4DBaseContext *inContext );
		virtual bool UpdateSymbol( Symbols::ISymbol *inSym, CDB4DBaseContext *inContext );
		
		virtual std::vector< Symbols::ISymbol * > RetainSymbolsByName( const VString &inName, CDB4DBaseContext *inContext );
		virtual std::vector< Symbols::ISymbol * > RetainSymbolsByName( Symbols::ISymbol *inOwner, const VString &inName, Symbols::IFile *inOwnerFile, CDB4DBaseContext *inContext );
		virtual std::vector< Symbols::ISymbol * > RetainNamedSubSymbols( Symbols::ISymbol *inOwner, Symbols::IFile *inOwnerFile, SortingOptions inOptions, CDB4DBaseContext *inContext );
		virtual std::vector< Symbols::ISymbol * > GetSymbolsForOutline( Symbols::ISymbol *inOwner, Symbols::IFile *inOwnerFile, SortingOptions inOptions, CDB4DBaseContext *inContext );
		virtual std::vector< Symbols::ISymbol * > RetainLikeNamedSubSymbols( Symbols::ISymbol *inOwner, const VString &inLikeName, CDB4DBaseContext *inContext, const bool inFullyLoad = false );
		virtual std::vector< Symbols::ISymbol * > RetainClassSymbols( Symbols::ISymbol *inOwner, const VString &inName, Symbols::IFile *inOwnerFile, CDB4DBaseContext *inContext );
		virtual void ReleaseSymbols(std::vector<Symbols::ISymbol*>& inSymbols);
		
		virtual std::vector< Symbols::IFile * > RetainFilesByPathAndBaseFolder( const VString &inName, const ESymbolFileBaseFolder inBaseFolder, CDB4DBaseContext *inContext );
		virtual std::vector< Symbols::IFile * > RetainFilesByName( const VString &inName, CDB4DBaseContext *inContext );
		virtual void ReleaseFiles(std::vector<Symbols::IFile*>& inFiles);
		
		virtual Symbols::ISymbol *GetSymbolOwnerByLine( Symbols::IFile *inOwnerFile, int inLineNumber, CDB4DBaseContext *inContext );
		virtual Symbols::ISymbol *GetSymbolByID( sLONG inID, CDB4DBaseContext *inContext );
		
		virtual bool DeleteSymbolsForFile( Symbols::IFile *inFile, bool inOnlyEditedSymbols, CDB4DBaseContext *inContext );
		
		virtual std::vector< Symbols::IExtraInfo * > GetExtraInformation( const Symbols::ISymbol *inOwnerSymbol, Symbols::IExtraInfo::Kind inKind, CDB4DBaseContext *inContext );
		virtual bool AddExtraInformation( const Symbols::ISymbol *inOwnerSymbol, const Symbols::IExtraInfo *inInfo, CDB4DBaseContext *inContext );
		
		
		virtual void SetSymbolsEditionState(const Symbols::IFile* inFile, bool inEditionState, std::vector<sLONG>& outIds, CDB4DBaseContext* inContext);
		
		virtual void GetFileIdsReferencingDeadSymbols(const std::vector<sLONG>& inSymbolIds, std::vector<sLONG>& outFileIds, CDB4DBaseContext* inContext);
		
		virtual Symbols::IFile* RetainFileByID(sLONG inID, CDB4DBaseContext* inContext);
		
		virtual void GetSymbolIdsByFileAndEditionState(sLONG inFileId, bool inEditionState, std::vector<sLONG>& outSymbolIds, CDB4DBaseContext* inContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		virtual void Dump(const XBOX::VString& inRequest, CDB4DBaseContext* inContext);
#endif

	private:
		void DumpSymbols(sLONG inFileId, CDB4DBaseContext* inContext);
		void RetainSymbolsFromFullName(const VString& inFullName, std::vector<Symbols::ISymbol*>& outSymbols, CDB4DBaseContext* inContext);
		CDB4DTable* fScopesTable;
		CDB4DField* fScopeFieldId;
		CDB4DField* fScopeFieldName;
		CDB4DField* fScopeFieldFullName;
		CDB4DField* fScopeFieldType;
		CDB4DField*	fScopeFieldExecutionContext;
		CDB4DField* fScopeFieldParentId;
		CDB4DField* fScopeFieldFromLine;
		CDB4DField* fScopeFieldToLine;
		CDB4DField* fScopeFieldFromOffset;
		CDB4DField* fScopeFieldToOffset;
		CDB4DField*	fScopeFieldStatus;
		CDB4DField*	fScopeFieldSourceId;
		CDB4DField*	fScopeFieldConstructorFlag;

		CDB4DTable* fSymbolNamesTable;
		CDB4DField* fSymbolNameFieldId;
		CDB4DField* fSymbolNameFieldValue;
		CDB4DField*	fSymbolNameFieldScopeId;
		CDB4DField*	fSymbolNameFieldExecutionContext;
		CDB4DField*	fSymbolNameFieldStatus;
		CDB4DField*	fSymbolNameFieldSourceId;
		
		CDB4DTable* fSymbolDefinitionsTable;
		CDB4DField* fSymbolDefinitionFieldId;
		CDB4DField* fSymbolDefinitionFieldScopeId;
		CDB4DField* fSymbolDefinitionFieldNameId;
		CDB4DField* fSymbolDefinitionFieldKind;
		CDB4DField*	fSymbolDefinitionFieldExecutionContext;
		CDB4DField*	fSymbolDefinitionFieldStatus;
		CDB4DField*	fSymbolDefinitionFieldSourceId;
		CDB4DField*	fSymbolDefinitionFieldLineNumber;

		CDB4DTable* fSymbolPrototypesTable;
		CDB4DField* fSymbolPrototypeFieldId;
		CDB4DField* fSymbolPrototypeFieldFromName;
		CDB4DField*	fSymbolPrototypeFieldToName;
		CDB4DField*	fSymbolPrototypeFieldExecutionContext;
		CDB4DField* fSymbolPrototypeFieldStatus;
		CDB4DField*	fSymbolPrototypeFieldSourceId;
		
		CDB4DTable* fSymbolReturnsTable;
		CDB4DField* fSymbolReturnFieldId;
		CDB4DField* fSymbolReturnFieldScopeId;
		CDB4DField*	fSymbolReturnFieldName;
		CDB4DField*	fSymbolReturnFieldStatus;
		CDB4DField*	fSymbolReturnFieldSourceId;

		CDB4DTable* fSymbolTypesTable;
		CDB4DField* fSymbolTypeFieldId;
		CDB4DField* fSymbolTypeFieldDefinitionId;
		CDB4DField*	fSymbolTypeFieldName;
		CDB4DField*	fSymbolTypeFieldStatus;
		CDB4DField*	fSymbolTypeFieldSourceId;

		CDB4DTable* fSymbolReferencesTable;
		CDB4DField* fSymbolReferenceFieldId;
		CDB4DField* fSymbolReferenceFieldType;
		CDB4DField* fSymbolReferenceFieldFromId;
		CDB4DField*	fSymbolReferenceFieldToId;
		CDB4DField* fSymbolReferenceFieldStatus;
		CDB4DField*	fSymbolReferenceFieldSourceId;

		CDB4DTable* fOutlineItemsTable;
		CDB4DField* fOutlineItemFieldId;
		CDB4DField*	fOutlineItemFieldSourceId;
		CDB4DField*	fOutlineItemFieldName;
		CDB4DField*	fOutlineItemFieldLine;
		CDB4DField* fOutlineItemFieldStatus;

			CDB4DTable* fSymbolsTable;
			CDB4DField* fSymbolNameField;
			CDB4DField* fSymbolIDField;
			CDB4DField* fSymbolFileIDField;
			CDB4DField* fSymbolScriptDocTextField;
			CDB4DField* fSymbolKindField;
			CDB4DField* fSymbolWAFKindField;
			CDB4DField* fSymbolPrototypeIDField;
			CDB4DField* fSymbolOwnerIDField;
			CDB4DField* fSymbolLineNumberField;
			CDB4DField* fSymbolLineCompletionNumberField;
			CDB4DField* fSymbolReturnTypeIDField;
			CDB4DField* fSymbolReferenceIDField;
			CDB4DField* fSymbolUndefinedStateField;
			CDB4DField* fSymbolInstanceStateField;
			CDB4DField* fSymbolReferenceStateField;
			CDB4DField* fSymbolEditionStateField;
			CDB4DField* fSymbolFullNameField;

			CDB4DTable* fFilesTable;
			CDB4DField* fFileIDField;
			CDB4DField* fFilePathField;
			CDB4DField* fFileBaseFolderField;
			CDB4DField* fFileExecContextField;
			CDB4DField* fFileModificationTimeField;
			CDB4DField* fFileResolvedStateField;

			CDB4DTable* fExtrasTable;
			CDB4DField* fExtraIDField;
			CDB4DField* fExtrasOwnerSymbolIDField;
			CDB4DField* fExtrasKindField;
			CDB4DField* fExtrasStringDataField;
			CDB4DField* fExtrasIntegerDataField;
		
			CDB4DTable* fVersionTable;
			CDB4DField* fVersionNumberField;

			const sLONG kCurrentVersion;
		
			// This field is retrieved from the management database, and updated before we close
			// the symbol table down.  We use it to service requests for symbol IDs.
			uLONG fNextUniqueSymbolID;
		
			CDB4DManager*	fDatabaseManager;
			CDB4DBase*		fDatabase;
			VSyncEvent		fAllowedToAddSymbolsEvent;
		
			std::map< sLONG, Symbols::ISymbol * > fRecursionSet;
			VCriticalSection *fRecursionTaskLock;
		
			bool InitializeNewDatabase( CDB4DBaseContext *inContext );
			bool RetainTables( CDB4DBaseContext *inContext );
			void ReleaseTables();
		
			void RetainFields( CDB4DBaseContext *inContext );
			void ReleaseFields();
		
			Symbols::ISymbol* SymbolFromRecord( CDB4DTable *inTable, CDB4DRecord *inRecord, CDB4DBaseContext *inContext, bool inFullyLoad = true );
		
			Symbols::IExtraInfo* ExtraInfoFromRecord( CDB4DTable *inTable, CDB4DRecord *inRecord, CDB4DBaseContext *inContext );
			bool RecordFromExtraInfo( const Symbols::ISymbol *inOwner, const Symbols::IExtraInfo *inExtraInfo, CDB4DRecord *ioRecord, CDB4DBaseContext *inContext );
		
			bool RecordFromFile( Symbols::IFile *inFile, CDB4DRecord *ioRecord, CDB4DBaseContext *inContext );
			Symbols::IFile* FileFromRecord( CDB4DTable *inTable, CDB4DRecord *inRecord, CDB4DBaseContext *inContext );
			Symbols::IFile* GetFileByID( sLONG inID, CDB4DBaseContext *inContext );
		
			std::vector< Symbols::ISymbol * > RetainSymbolsByNameHelper( sLONG inOwnerID, const VString &inName, sLONG inOwnerFileID, CDB4DBaseContext *inContext );
		
			VString	GetStringField( CDB4DRecord *inRecord, CDB4DField *inField ) const;
			sLONG	GetIntegerField( CDB4DRecord *inRecord, CDB4DField *inField, bool *isNull ) const;
			uLONG8	GetLongIntegerField( CDB4DRecord *inRecord, CDB4DField *inField ) const;
			bool	GetBooleanField( CDB4DRecord *inRecord, CDB4DField *inField ) const;
		
			bool OpenDataFile( const VFile &inDatabaseFile, CDB4DBaseContext *inContext );
		
			bool DeleteDatabaseOnDisk( const VFile &inDatabaseFile );
		
			bool CheckRequiredStructure( CDB4DBaseContext *inContext );
		
			virtual void CheckUpdatesAllowed()
			{
				fAllowedToAddSymbolsEvent.Lock();
			}
		
			UniChar fWildChar;
		};
	
	
	
	
		InnerSymbolTable*								fTable;
		CDB4DBaseContext*								fContext;
		VString											fTableFolder;
		std::map<ESymbolFileBaseFolder, XBOX::VString>	fBaseFolderPosixPathStrings;
};





/*
 *	NEXT GEN MANAGEMENT
 */
bool VSymbolTable::UseNextGen() const
{
	return fUseNextGen;
}

/*
 *	SCOPE MANAGEMENT
 */
ISymbolUpdateParameters* VSymbolTable::RetainSymbolUpdateParameters() const
{
	return new SymbolUpdateParameter();
}

void VSymbolTable::ReleaseSymbolUpdateParameters(ISymbolUpdateParameters* inUpdateParameters) const
{
	if( inUpdateParameters != NULL )
	{
		delete inUpdateParameters;
		inUpdateParameters = NULL;
	}
}

bool VSymbolTable::GetGlobalScope(const ESymbolFileExecContext& inExecutionContext, ISymbolUpdateParameters* inUpdateParameters, sLONG& outScopeId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetGlobalScope(inExecutionContext, inUpdateParameters, outScopeId, fContext);

#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetGlobalScope : ");
		headerTrace.AppendString("inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", outScopeId=");
		headerTrace.AppendLong(outScopeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

void VSymbolTable::SetGlobalScope(const ESymbolFileExecContext& inExecutionContext, sLONG& outScopeId)
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("SetGlobalScope : ");
		headerTrace.AppendString("inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);

		VString before("Before ");	before.AppendString(headerTrace);	_MagicDump(before);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		fTable->SetGlobalScope(inExecutionContext, outScopeId, fContext);

#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		headerTrace.AppendString(", outScopeId=");
		headerTrace.AppendLong(outScopeId);
		VString after("After ");	after.AppendString(headerTrace);	_MagicDump(after);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}

bool VSymbolTable::GetScope(const XBOX::VString& inScopeName, const ESymbolScopeType& inScopeType, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, ISymbolUpdateParameters* inUpdateParameters, sLONG& outScopeId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetScope(inScopeName, inScopeType, inParentScopeId, inSourceId, inExecutionContext, inUpdateParameters, outScopeId, fContext);

#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScope : ");
		headerTrace.AppendString("inScopeName=");
		headerTrace.AppendString(inScopeName);
		headerTrace.AppendString(", inScopeType=");
		headerTrace.AppendLong(inScopeType);
		headerTrace.AppendString(", inParentScopeId=");
		headerTrace.AppendLong(inParentScopeId);
		headerTrace.AppendString(", inSourceId=");
		headerTrace.AppendLong(inSourceId);
		headerTrace.AppendString(", inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", outScopeId=");
		headerTrace.AppendLong(outScopeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetScope(const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, const sLONG& inLineNumber, sLONG& outScopeId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetScope(inExecutionContext, inSourceId, inLineNumber, outScopeId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScope : ");
		headerTrace.AppendString("inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inSourceId=");
		headerTrace.AppendLong(inSourceId);
		headerTrace.AppendString(", inLineNumber=");
		headerTrace.AppendLong(inLineNumber);
		headerTrace.AppendString(", outScopeId=");
		headerTrace.AppendLong(outScopeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetScope(const XBOX::VString& inScopeName, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromScopeId, const sLONG& inToScopeId, sLONG& outScopeId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetScope(inScopeName, inExecutionContext, inFromScopeId, inToScopeId, outScopeId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScope : ");
		headerTrace.AppendString("inScopeName=");
		headerTrace.AppendString(inScopeName);
		headerTrace.AppendString(", inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inFromScopeId=");
		headerTrace.AppendLong(inFromScopeId);
		headerTrace.AppendString(", inToScopeId=");
		headerTrace.AppendLong(inToScopeId);
		headerTrace.AppendString(", outScopeId=");
		headerTrace.AppendLong(outScopeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetScopes(const XBOX::VString& inScopeName, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromScopeId, const sLONG& inToScopeId, std::vector<sLONG>& outScopeIds) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetScopes(inScopeName, inExecutionContext, inFromScopeId, inToScopeId, outScopeIds, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScope : ");
		headerTrace.AppendString("inScopeName=");
		headerTrace.AppendString(inScopeName);
		headerTrace.AppendString(", inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inFromScopeId=");
		headerTrace.AppendLong(inFromScopeId);
		headerTrace.AppendString(", inToScopeId=");
		headerTrace.AppendLong(inToScopeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetScopes(const XBOX::VString& inScopeName, const std::vector<sLONG>& inFromScopeIds, std::vector<sLONG>& outScopeIds) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetScopes(inScopeName, inFromScopeIds, outScopeIds, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScope : ");
		headerTrace.AppendString("inScopeName=");
		headerTrace.AppendString(inScopeName);

		XBOX::VString ids;
		VIndex total=inFromScopeIds.size();
		for(VIndex index=0; index<total; ++index)
		{
			ids.AppendLong(inFromScopeIds[index]);
			if( index < (total-1) )
				ids.AppendString(CVSTR("/"));
		}
		headerTrace.AppendString(", inFromScopeIds=");
		headerTrace.AppendString(ids);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

void VSymbolTable::SetScopeFullName(const sLONG inScopeId, const XBOX::VString& inScopeFullName)
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("SetScopeFullName : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		headerTrace.AppendString(", inScopeFullName=");
		headerTrace.AppendString(inScopeFullName);
		
		VString before("Before ");	before.AppendString(headerTrace);	_MagicDump(before);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		fTable->SetScopeFullName(inScopeId, inScopeFullName, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString after("After ");	after.AppendString(headerTrace);	_MagicDump(after);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}

void VSymbolTable::SetScopeType(const sLONG inScopeId, const ESymbolScopeType& inScopeType)
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("SetScopeType : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		headerTrace.AppendString(", inScopeType=");
		headerTrace.AppendLong(inScopeType);
		
		VString before("Before ");	before.AppendString(headerTrace);	_MagicDump(before);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		fTable->SetScopeType(inScopeId, inScopeType, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString after("After ");	after.AppendString(headerTrace);	_MagicDump(after);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}

void VSymbolTable::SetScope(const XBOX::VString& inScopeName, const ESymbolScopeType& inScopeType, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset, const bool& inConstructorFlag, sLONG& outScopeId)
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("SetScope : ");
		headerTrace.AppendString("inScopeName=");
		headerTrace.AppendString(inScopeName);
		headerTrace.AppendString(", inScopeType=");
		headerTrace.AppendLong(inScopeType);
		headerTrace.AppendString(", inParentScopeId=");
		headerTrace.AppendLong(inParentScopeId);
		headerTrace.AppendString(", inSourceId=");
		headerTrace.AppendLong(inSourceId);
		headerTrace.AppendString(", inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inFromLine=");
		headerTrace.AppendLong(inFromLine);
		headerTrace.AppendString(", inFromOffset=");
		headerTrace.AppendLong(inFromOffset);
		headerTrace.AppendString(", inToLine=");
		headerTrace.AppendLong(inToLine);
		headerTrace.AppendString(", inToOffset=");
		headerTrace.AppendLong(inToOffset);
		headerTrace.AppendString(", inConstructorFlag=");
		headerTrace.AppendLong(inConstructorFlag);

		VString before("Before ");	before.AppendString(headerTrace);	_MagicDump(before);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		fTable->SetScope(inScopeName, inScopeType, inParentScopeId, inSourceId, inExecutionContext, inFromLine, inFromOffset, inToLine, inToOffset, inConstructorFlag, outScopeId, fContext);

#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		headerTrace.AppendString(", outScopeId=");
		headerTrace.AppendLong(outScopeId);
		VString after("After ");	after.AppendString(headerTrace);	_MagicDump(after);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}

void VSymbolTable::PrepareUpdates(const sLONG& inSourceId, const bool& inOutlineOnly)
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("PrepareUpdates : ");
		headerTrace.AppendString("inSourceId=");
		headerTrace.AppendLong(inSourceId);
		
		VString before("Before ");	before.AppendString(headerTrace);	_MagicDump(before);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		fTable->PrepareUpdates(inSourceId, inOutlineOnly, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString after("After ");	after.AppendString(headerTrace);	_MagicDump(after);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}

void VSymbolTable::FinalizeUpdates(const sLONG& inSourceId, const bool& inOutlineOnly)
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("FinalizeUpdates : ");
		headerTrace.AppendString("inSourceId=");
		headerTrace.AppendLong(inSourceId);
		
		VString before("Before ");	before.AppendString(headerTrace);	_MagicDump(before);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		fTable->FinalizeUpdates(inSourceId, inOutlineOnly, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString after("After ");	after.AppendString(headerTrace);	_MagicDump(after);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}

bool VSymbolTable::GetScopeBoundaries(const sLONG& inScopeId, sLONG& outFromLine, sLONG& outFromOffset, sLONG& outToLine, sLONG& outToOffset) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScopeBoundaries : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		result = fTable->GetScopeBoundaries(inScopeId, outFromLine, outFromOffset, outToLine, outToOffset, fContext);
	}
	
	return result;
}

bool VSymbolTable::GetScopeSourceId(const sLONG& inScopeId, sLONG& outSourceId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScopeSourceId : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		result = fTable->GetScopeSourceId(inScopeId, outSourceId, fContext);
	}
	
	return result;
}

bool VSymbolTable::GetScopeName(const sLONG& inScopeId, XBOX::VString& outName) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScopeName : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		result = fTable->GetScopeName(inScopeId, outName, fContext);
	}
	
	return result;
}

bool VSymbolTable::GetScopeFullName(const sLONG& inScopeId, XBOX::VString& outName) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScopeFullName : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		result = fTable->GetScopeFullName(inScopeId, outName, fContext);
	}
	
	return result;
}

bool VSymbolTable::GetScopeType(const sLONG& inScopeId, ESymbolScopeType& outType) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScopeType : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		result = fTable->GetScopeType(inScopeId, outType, fContext);
	}
	
	return result;
}

bool VSymbolTable::GetScopeParentId(const sLONG& inScopeId, sLONG& outParentId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScopeParentId : ");
		headerTrace.AppendString("outParentId=");
		headerTrace.AppendLong(outParentId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		result = fTable->GetScopeParentId(inScopeId, outParentId, fContext);
	}
	
	return result;
}

/*
 *	SOURCE MANAGEMENT
 */
void VSymbolTable::DeleteSource(const sLONG& inSourceId)
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		fTable->DeleteSource(inSourceId, fContext);
	}
}

bool VSymbolTable::GetSourcePath(const sLONG& inSourceId, XBOX::VString& outSourcePath) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetSourcePath : ");
		headerTrace.AppendString("inSourceId=");
		headerTrace.AppendLong(inSourceId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		result = fTable->GetSourcePath(inSourceId, outSourcePath, fContext);
	}
	
	return result;
}

bool VSymbolTable::GetSourceBaseFolder(const sLONG& inSourceId, ESymbolFileBaseFolder& outSourceBaseFolder) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetSourceBaseFolder : ");
		headerTrace.AppendString("inSourceId=");
		headerTrace.AppendLong(inSourceId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		result = fTable->GetSourceBaseFolder(inSourceId, outSourceBaseFolder, fContext);
	}
	
	return result;
}

bool VSymbolTable::GetSourceId(const XBOX::VString& inSourcePath, const ESymbolFileExecContext& inExecutionContext, const ESymbolFileBaseFolder& inBaseFolder, sLONG& outFileId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetSourceId : ");
		headerTrace.AppendString("inSourcePath=");
		headerTrace.AppendString(inSourcePath);
		headerTrace.AppendString("inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString("inBaseFolder=");
		headerTrace.AppendLong(inBaseFolder);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		result = fTable->GetSourceId(inSourcePath, inExecutionContext, inBaseFolder, outFileId, fContext);
	}
	
	return result;
}

bool VSymbolTable::GetHtmlSourceIdLinkedToJsSourceId(const sLONG& inJsSourceId, sLONG& outHtmlSourceId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		VString path;
		GetSourcePath(inJsSourceId, path);
		
		ESymbolFileBaseFolder baseFolder;
		GetSourceBaseFolder(inJsSourceId, baseFolder);
		
		// The current file is a WAPAGE'S script file, let's get its HTML file and append the file path
		VString basePathStr;
		GetBaseFolderPathStr(baseFolder, basePathStr);
		basePathStr.AppendString(path);
		
		// Create a valid posix file path
		VFilePath basePath(basePathStr, FPS_POSIX);
		
		// Get the file name without .js extension
		VString jsFileName;
		basePath.GetFileName(jsFileName, false);
		
		if( jsFileName == "index" || jsFileName.BeginsWith("index-") )
		{
			// Go back to the .waPage folder
			VString folderName;
			basePath.GetFolderName(folderName);
			while( !folderName.EndsWith(".waPage") )
			{
				VFilePath tempPath = basePath.ToParent();
				basePath = tempPath;
				
				if( !basePath.IsFolder() )
					break;
				
				basePath.GetFolderName(folderName);
			}
			
			if( basePath.IsFolder() && folderName.EndsWith(".waPage") )
			{
				// Construct the associated HTML file path in which data source symbols are
				VString htmlFileName(jsFileName);
				htmlFileName.AppendString(".html");
				
				VFilePath htmlFilePath = basePath.ToSubFile(htmlFileName);
				
				VString htmlRelativeFilePathString;
				GetRelativePath(htmlFilePath, baseFolder, htmlRelativeFilePathString);
				
				sLONG htmlSourceId;
				result = GetSourceId(htmlRelativeFilePathString, eSymbolFileExecContextClient, baseFolder, outHtmlSourceId);
			}
		}
	}
	
	return result;
}

bool VSymbolTable::GetScopeChildren(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const bool& inUseSourceId, const sLONG& inSourceId, std::vector<sLONG>& outScopeChildrenIds) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScopeChildren : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		headerTrace.AppendString(", inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inUseSourceId=");
		headerTrace.AppendLong(inUseSourceId);
		headerTrace.AppendString(", inSourceId=");
		headerTrace.AppendLong(inSourceId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		result = fTable->GetScopeChildren(inScopeId, inExecutionContext, inUseSourceId, inSourceId, outScopeChildrenIds, fContext);
	}
	
	return result;
}

bool VSymbolTable::GetScopeChildren(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inChildName, std::vector<sLONG>& outScopeChildrenIds) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScopeChildren : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		headerTrace.AppendString(", inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inChildName=");
		headerTrace.AppendString(inChildName);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		result = fTable->GetScopeChildren(inScopeId, inExecutionContext, inChildName, outScopeChildrenIds, fContext);
	}
	
	return result;
}

bool VSymbolTable::GetScopeChild(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inChildName, sLONG& outScopeChildId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScopeChild : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		headerTrace.AppendString(", inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inChildName=");
		headerTrace.AppendString(inChildName);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		result = fTable->GetScopeChild(inScopeId, inExecutionContext, inChildName, outScopeChildId, fContext);
	}
	
	return result;
}

bool VSymbolTable::GetScopePrototypeId(const XBOX::VString& inScopeName, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromScopeId, const sLONG& inToScopeId, sLONG& outScopeId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		sLONG returnScopeId;
		if( fTable->GetScope(inScopeName, inExecutionContext, inFromScopeId, inToScopeId, returnScopeId, fContext) == true )
		{
			if( fTable->GetScopeChild(returnScopeId, inExecutionContext, CVSTR("prototype"), outScopeId, fContext) == true )
			{
				result = true;
			}
		}
	}
	
	return result;
}

/*
 *	SYMBOL'S NAME MANAGEMENT
 */
bool VSymbolTable::GetName(const XBOX::VString& inSymbolName, const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, ISymbolUpdateParameters* inUpdateParameters, sLONG& outSymbolNameId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetName(inSymbolName, inScopeId, inExecutionContext, inSourceId, inUpdateParameters, outSymbolNameId, fContext);

#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetName : ");
		headerTrace.AppendString("inSymbolName=");
		headerTrace.AppendString(inSymbolName);
		headerTrace.AppendString(", inScopeId=");
		headerTrace.AppendLong(inScopeId);
		headerTrace.AppendString(", inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inSourceId=");
		headerTrace.AppendLong(inSourceId);
		headerTrace.AppendString(", outSymbolNameId=");
		headerTrace.AppendLong(outSymbolNameId);
		
		_MagicDump(headerTrace);
#endif
	}
	
	return result;
}

bool VSymbolTable::GetNames(const XBOX::VString& inSymbolName, const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, ISymbolUpdateParameters* inUpdateParameters, std::vector<sLONG>& outSymbolNameIds) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetNames(inSymbolName, inScopeId, inExecutionContext, inSourceId, inUpdateParameters, outSymbolNameIds, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetName : ");
		headerTrace.AppendString("inSymbolName=");
		headerTrace.AppendString(inSymbolName);
		headerTrace.AppendString(", inScopeId=");
		headerTrace.AppendLong(inScopeId);
		headerTrace.AppendString(", inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inSourceId=");
		headerTrace.AppendLong(inSourceId);
		
		_MagicDump(headerTrace);
#endif
	}
	
	return result;
}

bool VSymbolTable::GetName(const sLONG& inNameId, XBOX::VString& outName) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetName(inNameId, outName, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetName : ");
		headerTrace.AppendString("inNameId=");
		headerTrace.AppendLong(inNameId);
		headerTrace.AppendString(", outName=");
		headerTrace.AppendString(outName);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetName(const sLONG& inNameId, const XBOX::VString& inSymbolStartsWith, XBOX::VString& outName) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetName(inNameId, inSymbolStartsWith, outName, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetName : ");
		headerTrace.AppendString("inNameId=");
		headerTrace.AppendLong(inNameId);
		headerTrace.AppendString(", inSymbolStartsWith=");
		headerTrace.AppendString(inSymbolStartsWith);
		headerTrace.AppendString(", outName=");
		headerTrace.AppendString(outName);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetNameScopeId(const sLONG& inNameId, sLONG& outScopeId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetNameScopeId(inNameId, outScopeId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetNameScopeId : ");
		headerTrace.AppendString("inNameId=");
		headerTrace.AppendLong(inNameId);
		headerTrace.AppendString(", outScopeId=");
		headerTrace.AppendLong(outScopeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetNameExecutionContext(const sLONG& inNameId, ESymbolFileExecContext& outExecutionContext) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetNameExecutionContext(inNameId, outExecutionContext, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetNameExecutionContext : ");
		headerTrace.AppendString("inNameId=");
		headerTrace.AppendLong(inNameId);
		headerTrace.AppendString(", outExecutionContext=");
		headerTrace.AppendLong(outExecutionContext);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetNames(const XBOX::VString& inStartWith, const sLONG& inScopeId, std::vector<sLONG>& outNameIds) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetNames(inStartWith, inScopeId, outNameIds, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetNames : ");
		headerTrace.AppendString("inStartWith=");
		headerTrace.AppendString(inStartWith);
		headerTrace.AppendString(", inScopeId=");
		headerTrace.AppendLong(inScopeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetNameFromScope(const XBOX::VString& inSymbolName, const sLONG& inFromScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, sLONG& outNameId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetNameFromScope(inSymbolName, inFromScopeId, inExecutionContext, inSourceId, outNameId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetNameFromScope : ");
		headerTrace.AppendString("inSymbolName=");
		headerTrace.AppendString(inSymbolName);
		headerTrace.AppendString(", inFromScopeId=");
		headerTrace.AppendLong(inFromScopeId);
		headerTrace.AppendString(", inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inSourceId=");
		headerTrace.AppendLong(inSourceId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

void VSymbolTable::SetName(const XBOX::VString& inSymbolName, const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, sLONG& outSymbolNameId)
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("SetName : ");
		headerTrace.AppendString("inSymbolName=");
		headerTrace.AppendString(inSymbolName);
		headerTrace.AppendString(", inScopeId=");
		headerTrace.AppendLong(inScopeId);
		headerTrace.AppendString(", inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inSourceId=");
		headerTrace.AppendLong(inSourceId);

		VString before("Before ");	before.AppendString(headerTrace);	_MagicDump(before);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		fTable->SetName(inSymbolName, inScopeId, inExecutionContext, inSourceId, outSymbolNameId, fContext);

#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		headerTrace.AppendString(", outSymbolNameId=");
		headerTrace.AppendLong(outSymbolNameId);
		VString after("After ");	after.AppendString(headerTrace);	_MagicDump(after);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}

/*
 *	SYMBOL'S DEFINITION MANAGEMENT
 */
bool VSymbolTable::GetDefinition(const sLONG& inScopeId, const sLONG& inNameId, const sLONG& inSourceId, const ESymbolDefinitionKind& inKind, const sLONG& inLineNumber, ISymbolUpdateParameters* inUpdateParameters, sLONG& outDefinitionId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetDefinition(inScopeId, inNameId, inSourceId, inKind, inLineNumber, inUpdateParameters, outDefinitionId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetDefinition : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		headerTrace.AppendString("inNameId=");
		headerTrace.AppendLong(inNameId);
		headerTrace.AppendString(", inSourceId=");
		headerTrace.AppendLong(inSourceId);
		headerTrace.AppendString(", inKind=");
		headerTrace.AppendLong(inKind);
		headerTrace.AppendString(", inLineNumber=");
		headerTrace.AppendLong(inLineNumber);
		headerTrace.AppendString(", outDefinitionId=");
		headerTrace.AppendLong(outDefinitionId);

		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetDefinitionId(const sLONG& inScopeId, const sLONG& inNameId, const sLONG& inSourceId, sLONG& outDefinitionId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetDefinitionId(inScopeId, inNameId, inSourceId, outDefinitionId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetDefinitionId : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		headerTrace.AppendString(", inNameId=");
		headerTrace.AppendLong(inNameId);
		headerTrace.AppendString(", inSourceId=");
		headerTrace.AppendLong(inSourceId);
		headerTrace.AppendString(", outDefinitionId=");
		headerTrace.AppendLong(outDefinitionId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetDefinitionName(const sLONG& inDefinitionId, XBOX::VString& outName) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetDefinitionName(inDefinitionId, outName, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetDefinitionName : ");
		headerTrace.AppendString("inDefinitionId=");
		headerTrace.AppendLong(inDefinitionId);
		headerTrace.AppendString(", outName=");
		headerTrace.AppendString(outName);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetDefinitionNameId(const sLONG& inDefinitionId, sLONG& outNameId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetDefinitionNameId(inDefinitionId, outNameId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetDefinitionNameId : ");
		headerTrace.AppendString("inDefinitionId=");
		headerTrace.AppendLong(inDefinitionId);
		headerTrace.AppendString(", outNameId=");
		headerTrace.AppendLong(outNameId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetDefinitionKind(const sLONG& inDefinitionId, ESymbolDefinitionKind& outKind) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetDefinitionKind(inDefinitionId, outKind, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetDefinitionKind : ");
		headerTrace.AppendString("inDefinitionId=");
		headerTrace.AppendLong(inDefinitionId);
		headerTrace.AppendString(", outKind=");
		headerTrace.AppendLong(outKind);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetDefinitionParentScopeId(const sLONG& inDefinitionId, sLONG& outParentScopeId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetDefinitionParentScopeId(inDefinitionId, outParentScopeId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetDefinitionKind : ");
		headerTrace.AppendString("inDefinitionId=");
		headerTrace.AppendLong(inDefinitionId);
		headerTrace.AppendString(", outParentScopeId=");
		headerTrace.AppendLong(outParentScopeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetDefinitionStatus(const sLONG& inDefinitionId, ESymbolStatus& outStatus) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetDefinitionStatus(inDefinitionId, outStatus, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetDefinitionStatus : ");
		headerTrace.AppendString("inDefinitionId=");
		headerTrace.AppendLong(inDefinitionId);
		headerTrace.AppendString(", outStatus=");
		headerTrace.AppendLong(outStatus);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetDefinitionSourceId(const sLONG& inDefinitionId, sLONG& outSourceId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetDefinitionSourceId(inDefinitionId, outSourceId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetDefinitionSourceId : ");
		headerTrace.AppendString("inDefinitionId=");
		headerTrace.AppendLong(inDefinitionId);
		headerTrace.AppendString(", outSourceId=");
		headerTrace.AppendLong(outSourceId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetDefinitionLine(const sLONG& inDefinitionId, sLONG& outLine) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetDefinitionLine(inDefinitionId, outLine, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetDefinitionLine : ");
		headerTrace.AppendString("inDefinitionId=");
		headerTrace.AppendLong(inDefinitionId);
		headerTrace.AppendString(", outLine=");
		headerTrace.AppendLong(outLine);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

void VSymbolTable::SetDefinition(const sLONG& inSourceId, const sLONG& inScopeId, const sLONG& inNameId, const ESymbolFileExecContext& inExecutionContext, const ESymbolDefinitionKind& inKind, const sLONG& inLineNumber, sLONG& outDefinitionId)
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("SetDefinition : ");
		headerTrace.AppendString("inSourceId=");
		headerTrace.AppendLong(inSourceId);
		headerTrace.AppendString(", inScopeId=");
		headerTrace.AppendLong(inScopeId);
		headerTrace.AppendString(", inNameId=");
		headerTrace.AppendLong(inNameId);
		headerTrace.AppendString(", inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inKind=");
		headerTrace.AppendLong(inKind);
		headerTrace.AppendString(", inLineNumber=");
		headerTrace.AppendLong(inLineNumber);
				
		VString before("Before ");	before.AppendString(headerTrace);	_MagicDump(before);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		fTable->SetDefinition(inSourceId, inScopeId, inNameId, inExecutionContext, inKind, inLineNumber, outDefinitionId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString after("After ");	after.AppendString(headerTrace);	_MagicDump(after);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}


/*
 *	SYMBOL'S SUGGESTION MANAGEMENT
 */
void VSymbolTable::GetConstructorNames(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inStartWith, const sLONG& inFromScope, const sLONG& inToScope, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds) const
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		fTable->GetConstructorNames(inExecutionContext, inStartWith, inFromScope, inToScope, outNames, outNameScopeIds, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetConstructorNames : ");
		headerTrace.AppendString("inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inStartWith=");
		headerTrace.AppendString(inStartWith);
		headerTrace.AppendString(", inFromScope=");
		headerTrace.AppendLong(inFromScope);
		headerTrace.AppendString(", inToScope=");
		headerTrace.AppendLong(inToScope);

		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}

void VSymbolTable::GetWidgetNames(const sLONG& inHtmlSourceId, const XBOX::VString& inStartWith, std::vector<XBOX::VString>& outNames) const
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		fTable->GetWidgetNames(inHtmlSourceId, inStartWith, outNames, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetWidgetNames : ");
		headerTrace.AppendString("inHtmlSourceId=");
		headerTrace.AppendLong(inHtmlSourceId);
		headerTrace.AppendString(", inStartWith=");
		headerTrace.AppendString(inStartWith);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}

void VSymbolTable::GetModelEntityPropertyNames(const XBOX::VString& inModelName, const XBOX::VString& inModelEntityName, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds) const
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		fTable->GetModelEntityPropertyNames(inModelName, inModelEntityName, outNames, outNameScopeIds, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetModelEntityPropertyNames : ");
		headerTrace.AppendString("inModelName=");
		headerTrace.AppendString(inModelName);
		headerTrace.AppendString(", inModelEntityName=");
		headerTrace.AppendString(inModelEntityName);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}

void VSymbolTable::GetNames(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inStartWith, const sLONG& inFromScope, const sLONG& inToScope, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds) const
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		fTable->GetNames(inExecutionContext, inStartWith, inFromScope, inToScope, outNames, outNameScopeIds, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetNames : ");
		headerTrace.AppendString("inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inStartWith=");
		headerTrace.AppendString(inStartWith);
		headerTrace.AppendString(", inFromScope=");
		headerTrace.AppendLong(inFromScope);
		headerTrace.AppendString(", inToScope=");
		headerTrace.AppendLong(inToScope);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}

void VSymbolTable::GetNames(const XBOX::VString& inStartWith, const std::vector<sLONG>& inScopeIds, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds) const
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		fTable->GetNames(inStartWith, inScopeIds, outNames, outNameScopeIds, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetNames : ");
		headerTrace.AppendString("inStartWith=");
		headerTrace.AppendString(inStartWith);
		headerTrace.AppendString(", inScopes=");
		XBOX::VString ids;
		VIndex total=inScopeIds.size();
		for(VIndex index=0; index<total; ++index)
		{
			ids.AppendLong(inScopeIds[index]);
			if( index < (total-1) )
				ids.AppendString(CVSTR("/"));
		}
		headerTrace.AppendString(ids);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}


/*
 *	SYMBOL'S PROTOTYPE CHAIN MANAGEMENT
 */
bool VSymbolTable::GetPrototypeChain(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inFromName, const XBOX::VString& inToName, ISymbolUpdateParameters* inUpdateParameters, sLONG& outPrototypeId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetPrototypeChain(inExecutionContext, inFromName, inToName, inUpdateParameters, outPrototypeId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetPrototypeChain : ");
		headerTrace.AppendString("inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inFromName=");
		headerTrace.AppendString(inFromName);
		headerTrace.AppendString(", inToName=");
		headerTrace.AppendString(inToName);
		headerTrace.AppendString(", outPrototypeId=");
		headerTrace.AppendLong(outPrototypeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetPrototypeChain(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inFromName, std::vector<XBOX::VString>& outPrototypeNames) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetPrototypeChain(inExecutionContext, inFromName, outPrototypeNames, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetPrototypeChain : ");
		headerTrace.AppendString("inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inFromName=");
		headerTrace.AppendString(inFromName);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetPrototypeChainParentName(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inFromName, XBOX::VString& outParentPrototypeName) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetPrototypeChainParentName(inExecutionContext, inFromName, outParentPrototypeName, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetPrototypeChainParentName : ");
		headerTrace.AppendString("inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inFromName=");
		headerTrace.AppendString(inFromName);
		headerTrace.AppendString(", outParentPrototypeName=");
		headerTrace.AppendString(outParentPrototypeName);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

void VSymbolTable::SetPrototypeChain(const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, const XBOX::VString& inFromName, const XBOX::VString& inToName)
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("SetPrototypeChain : ");
		headerTrace.AppendString("inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inSourceId=");
		headerTrace.AppendLong(inSourceId);
		headerTrace.AppendString(", inFromName=");
		headerTrace.AppendString(inFromName);
		headerTrace.AppendString(", inToName=");
		headerTrace.AppendString(inToName);
		
		VString before("Before ");	before.AppendString(headerTrace);	_MagicDump(before);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		fTable->SetPrototypeChain(inExecutionContext, inSourceId, inFromName, inToName, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString after("After ");	after.AppendString(headerTrace);	_MagicDump(after);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}

/*
 *	SYMBOL'S RETURNS CHAIN MANAGEMENT
 */
bool VSymbolTable::GetReturnType(const sLONG& inScopeId, const XBOX::VString& inReturnTypeName, ISymbolUpdateParameters* inUpdateParameters, sLONG& outReturnTypeId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetReturnType(inScopeId, inReturnTypeName, inUpdateParameters, outReturnTypeId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetReturnType : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		headerTrace.AppendString(", inReturnTypeName=");
		headerTrace.AppendString(inReturnTypeName);
		headerTrace.AppendString(", outReturnTypeId=");
		headerTrace.AppendLong(outReturnTypeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetReturnTypeNames(const sLONG& inScopeId, std::vector<XBOX::VString>& outNames) const
{
	bool result = false;

	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetReturnTypeNames(inScopeId, outNames, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetReturnTypeNames : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}

	return result;
}

void VSymbolTable::SetReturnType(const sLONG& inSourceId, const sLONG& inScopeId, const XBOX::VString& inTypeName)
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("SetReturnType : ");
		headerTrace.AppendString("inSourceId=");
		headerTrace.AppendLong(inSourceId);
		headerTrace.AppendString(", inScopeId=");
		headerTrace.AppendLong(inScopeId);
		headerTrace.AppendString(", inTypeName=");
		headerTrace.AppendString(inTypeName);
		
		VString before("Before ");	before.AppendString(headerTrace);	_MagicDump(before);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		fTable->SetReturnType(inSourceId, inScopeId, inTypeName, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString after("After ");	after.AppendString(headerTrace);	_MagicDump(after);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}

/*
 *	SYMBOL'S TYPES MANAGEMENT
 */
bool VSymbolTable::GetType(const sLONG& inDefinitionId, const XBOX::VString& inTypeName, ISymbolUpdateParameters* inUpdateParameters, sLONG& outTypeId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetType(inDefinitionId, inTypeName, inUpdateParameters, outTypeId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetType : ");
		headerTrace.AppendString("inDefinitionId=");
		headerTrace.AppendLong(inDefinitionId);
		headerTrace.AppendString(", inTypeName=");
		headerTrace.AppendString(inTypeName);
		headerTrace.AppendString(", outTypeId=");
		headerTrace.AppendLong(outTypeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetTypeNames(const sLONG& inDefinitionId, std::vector<XBOX::VString>& outNames) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetTypeNames(inDefinitionId, outNames, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetTypeNames : ");
		headerTrace.AppendString("inDefinitionId=");
		headerTrace.AppendLong(inDefinitionId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

void VSymbolTable::SetType(const sLONG& inDefinitionId, const sLONG& inSourceId, const XBOX::VString& inTypeName)
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("SetType : ");
		headerTrace.AppendString("inDefinitionId=");
		headerTrace.AppendLong(inDefinitionId);
		headerTrace.AppendString(", inSourceId=");
		headerTrace.AppendLong(inSourceId);
		headerTrace.AppendString(", inTypeName=");
		headerTrace.AppendString(inTypeName);
		
		VString before("Before ");	before.AppendString(headerTrace);	_MagicDump(before);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		fTable->SetType(inDefinitionId, inSourceId, inTypeName, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString after("After ");	after.AppendString(headerTrace);	_MagicDump(after);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}

/*
 *	SYMBOL'S REFERENCES MANAGEMENT
 */
bool VSymbolTable::GetReference(const ESymbolReferenceKind& inReferenceKind, const sLONG& inFromId, const sLONG& inToId, ISymbolUpdateParameters* inUpdateParameters, sLONG& outReferenceId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetReference(inReferenceKind, inFromId, inToId, inUpdateParameters, outReferenceId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetReference : ");
		headerTrace.AppendString("inReferenceKind=");
		headerTrace.AppendLong(inReferenceKind);
		headerTrace.AppendString(", inFromId=");
		headerTrace.AppendLong(inFromId);
		headerTrace.AppendString(", inToId=");
		headerTrace.AppendLong(inToId);
		headerTrace.AppendString(", outReferenceId=");
		headerTrace.AppendLong(outReferenceId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetReference(const ESymbolReferenceKind& inReferenceKind, const sLONG& inFromId, ISymbolUpdateParameters* inUpdateParameters, std::vector<sLONG>& outReferenceIds) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetReference(inReferenceKind, inFromId, inUpdateParameters, outReferenceIds, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetReference : ");
		headerTrace.AppendString("inReferenceKind=");
		headerTrace.AppendLong(inReferenceKind);
		headerTrace.AppendString(", inFromId=");
		headerTrace.AppendLong(inFromId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetReferenceToId(const sLONG& inReferenceId, sLONG& outToId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetReferenceToId(inReferenceId, outToId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetReferenceToId : ");
		headerTrace.AppendString("inReferenceId=");
		headerTrace.AppendLong(inReferenceId);
		headerTrace.AppendString(", outToId=");
		headerTrace.AppendLong(outToId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetReferenceKind(const sLONG& inReferenceId, ESymbolReferenceKind& outReferenceKind) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetReferenceKind(inReferenceId, outReferenceKind, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetReferenceKind : ");
		headerTrace.AppendString("inReferenceId=");
		headerTrace.AppendLong(outReferenceKind);
		headerTrace.AppendString(", outReferenceKind=");
		headerTrace.AppendLong(outReferenceKind);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::IsReferenced(const sLONG& inScopeId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->IsReferenced(inScopeId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("IsReferenced : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

void VSymbolTable::SetReference(const ESymbolReferenceKind& inReferenceKind, const sLONG& inFromId, const sLONG& inToId, const sLONG& inSourceId)
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("SetReference : ");
		headerTrace.AppendString("inReferenceKind=");
		headerTrace.AppendLong(inReferenceKind);
		headerTrace.AppendString(", inFromId=");
		headerTrace.AppendLong(inFromId);
		headerTrace.AppendString(", inToId=");
		headerTrace.AppendLong(inToId);
		headerTrace.AppendString(", inSourceId=");
		headerTrace.AppendLong(inSourceId);
		
		VString before("Before ");	before.AppendString(headerTrace);	_MagicDump(before);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		fTable->SetReference(inReferenceKind, inFromId, inToId, inSourceId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString after("After ");	after.AppendString(headerTrace);	_MagicDump(after);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}

/*
 *	SYMBOL'S OUTLINE MANAGEMENT
 */
bool VSymbolTable::GetScopeContentDefinitions(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, std::vector<sLONG>& outIds) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetScopeContentDefinitions(inScopeId, inExecutionContext, outIds, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScopeContentDefinitions : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		headerTrace.AppendString(", inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetScopeContentDefinitions(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, std::vector<sLONG>& outIds) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetScopeContentDefinitions(inScopeId, inExecutionContext, inSourceId, outIds, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetScopeContentDefinitions : ");
		headerTrace.AppendString("inScopeId=");
		headerTrace.AppendLong(inScopeId);
		headerTrace.AppendString(", inExecutionContext=");
		headerTrace.AppendLong(inExecutionContext);
		headerTrace.AppendString(", inSourceId=");
		headerTrace.AppendLong(inSourceId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

/*
 *	OUTLINE ITEM DATAS MANAGEMENT
 */
bool VSymbolTable::GetOutlineItem(const sLONG& inSourceId, const XBOX::VString& inName, const sLONG& inLine, ISymbolUpdateParameters* inUpdateParameters, sLONG& outItemId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetOutlineItem(inSourceId, inName, inLine, inUpdateParameters, outItemId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetOutlineItem : ");
		headerTrace.AppendString("inSourceId=");
		headerTrace.AppendLong(inSourceId);
		headerTrace.AppendString(", inName=");
		headerTrace.AppendString(inName);
		headerTrace.AppendString(", inLine=");
		headerTrace.AppendLong(inLine);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetOutlineItemSourceId(const sLONG& inItemId, sLONG& outSourceId) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetOutlineItemSourceId(inItemId, outSourceId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetOutlineItemSourceId : ");
		headerTrace.AppendString("inItemId=");
		headerTrace.AppendLong(inItemId);
		headerTrace.AppendString(", outSourceId=");
		headerTrace.AppendLong(outSourceId);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetOutlineItemName(const sLONG& inItemId, XBOX::VString& outName) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetOutlineItemName(inItemId, outName, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetOutlineItemName : ");
		headerTrace.AppendString("inItemId=");
		headerTrace.AppendLong(inItemId);
		headerTrace.AppendString(", outName=");
		headerTrace.AppendString(outName);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

bool VSymbolTable::GetOutlineItemLine(const sLONG& inItemId, sLONG& outLine) const
{
	bool result = false;
	
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
		result = fTable->GetOutlineItemLine(inItemId, outLine, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("GetOutlineItemLine : ");
		headerTrace.AppendString("inItemId=");
		headerTrace.AppendLong(inItemId);
		headerTrace.AppendString(", outLine=");
		headerTrace.AppendLong(outLine);
		
		_MagicDump(headerTrace);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
	
	return result;
}

void VSymbolTable::SetOutlineItem(const sLONG& inSourceId, const XBOX::VString& inName, const sLONG& inLine, sLONG& outItemId)
{
	xbox_assert( fTable != NULL && fContext != NULL );
	if( fTable != NULL && fContext != NULL )
	{
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString headerTrace("SetOutlineItem : ");
		headerTrace.AppendString("inSourceId=");
		headerTrace.AppendLong(inSourceId);
		headerTrace.AppendString(", inName=");
		headerTrace.AppendString(inName);
		headerTrace.AppendString(", inLine=");
		headerTrace.AppendLong(inLine);
		
		VString before("Before ");	before.AppendString(headerTrace);	_MagicDump(before);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		
		fTable->SetOutlineItem(inSourceId, inName, inLine, outItemId, fContext);
		
#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
		VString after("After ");	after.AppendString(headerTrace);	_MagicDump(after);
#endif//ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
	}
}




VSymbolTable::VSymbolTable(const bool& inUseNextGen):
fTable( NULL ),
fContext( NULL ),
fUseNextGen(inUseNextGen)
{
}



VSymbolTable::~VSymbolTable()
{
	if (fContext)
		fContext->Release();
	
	if (fTable)
		fTable->Release();
}



void VSymbolTable::LockUpdates()
{
	fTable->LockUpdates();
}



void VSymbolTable::UnlockUpdates()
{
	fTable->UnlockUpdates();
}



void VSymbolTable::GetRelativePath( const XBOX::VFilePath &inFilePath, const ESymbolFileBaseFolder inBaseFolder, XBOX::VString &outFilePath ) const
{
	std::map<ESymbolFileBaseFolder, XBOX::VString>::const_iterator it = fBaseFolderPosixPathStrings.find(inBaseFolder);
	if( it != fBaseFolderPosixPathStrings.end() )
	{
		VString inFilePathPosix;
		inFilePath.GetPosixPath(inFilePathPosix);
		
		XBOX::VFilePath baseFolderPath(it->second, FPS_POSIX);
		inFilePath.GetRelativePosixPath(baseFolderPath, outFilePath);
	}
}



ISymbolTable* VSymbolTable::CloneForNewThread()
{
	VSymbolTable *ret = new VSymbolTable(fUseNextGen);
	
	ret->fTableFolder					=	fTableFolder;
	ret->fBaseFolderPosixPathStrings	=	fBaseFolderPosixPathStrings;
	
	ret->fTable					=	fTable;
	if (fTable)
	{
		fTable->Retain();		// Because a new table shares ownership of this
		ret->fContext = fTable->GetContextForNewThread();
	}
	return ret;
}



uLONG VSymbolTable::GetNextUniqueID()
{
	if (fTable)
	{
		return fTable->GetNextUniqueID();
	}
	else
	{
		return 0;
	}
}



bool VSymbolTable::OpenSymbolDatabase( const VFile &inDatabaseFile )
{
	xbox_assert( !fTable );
	
	// Grab the parent folder for the database
	VFolder *parent = inDatabaseFile.RetainParentFolder();
	parent->GetPath().GetPosixPath( fTableFolder );
	parent->Release();
	
	SetBaseFolderPathStr(eSymbolFileBaseFolderProject, fTableFolder, false);
	
	fTable = new InnerSymbolTable(fUseNextGen);
	return fTable->OpenSymbolDatabase( inDatabaseFile, &fContext );
}



bool VSymbolTable::AddFile( Symbols::IFile* inFile )
{
	bool result = false;
	
	if (fTable)
		result = fTable->AddFile( inFile, fContext );

	return result;
}



bool VSymbolTable::AddSymbols( std::vector< Symbols::ISymbol * > inSymbols )
{
	if (fTable)
	{
		return fTable->AddSymbols( inSymbols, fContext );
	}
	return false;
}



bool VSymbolTable::UpdateFile( Symbols::IFile *inFile )
{
	if (fTable)
	{
		return fTable->UpdateFile( inFile, fContext );
	}
	return false;
}



bool VSymbolTable::UpdateSymbol( Symbols::ISymbol * inSymbol )
{
	if (fTable)
	{
		return fTable->UpdateSymbol( inSymbol, fContext );
	}
	return false;
}



std::vector< Symbols::ISymbol * > VSymbolTable::RetainSymbolsByName( const VString &inName )
{
	if (fTable)
	{
		return fTable->RetainSymbolsByName( inName, fContext );
	}
	std::vector< Symbols::ISymbol * > ret;
	return ret;
}



std::vector< Symbols::ISymbol * > VSymbolTable::RetainSymbolsByName( Symbols::ISymbol *inOwner, const VString &inName, Symbols::IFile* inOwnerFile/*=NULL*/ )
{
	if (fTable)
	{
		return fTable->RetainSymbolsByName( inOwner, inName, inOwnerFile, fContext );
	}
	std::vector< Symbols::ISymbol * > ret;
	return ret;
}



std::vector< Symbols::ISymbol * > VSymbolTable::RetainClassSymbols( Symbols::ISymbol *inOwner, const VString &inName, Symbols::IFile* inOwnerFile/*=NULL*/ )
{
	if (fTable)
	{
		return fTable->RetainClassSymbols( inOwner, inName, inOwnerFile, fContext );
	}
	std::vector< Symbols::ISymbol * > ret;
	return ret;
}



std::vector< Symbols::ISymbol * > VSymbolTable::RetainNamedSubSymbols( Symbols::ISymbol *inOwner, Symbols::IFile* inOwnerFile/*=NULL*/, SortingOptions inOptions/*=kDefault*/ )
{
	if (fTable)
	{
		return fTable->RetainNamedSubSymbols( inOwner, inOwnerFile, inOptions, fContext );
	}
	std::vector< Symbols::ISymbol * > ret;
	return ret;
}



std::vector< Symbols::ISymbol * > VSymbolTable::GetSymbolsForOutline( Symbols::ISymbol *inOwner, Symbols::IFile *inOwnerFile, SortingOptions inOptions )
{
	if (fTable)
	{
		return fTable->GetSymbolsForOutline( inOwner, inOwnerFile, inOptions, fContext );
	}
	std::vector< Symbols::ISymbol * > ret;
	return ret;
}



std::vector< Symbols::ISymbol * > VSymbolTable::RetainLikeNamedSubSymbols( Symbols::ISymbol *inOwner, const VString &inLikeName, const bool inFullyLoad )
{
	if (fTable)
	{
		return fTable->RetainLikeNamedSubSymbols( inOwner, inLikeName, fContext, inFullyLoad );
	}
	std::vector< Symbols::ISymbol * > ret;
	return ret;
}



void VSymbolTable::ReleaseSymbols(std::vector<Symbols::ISymbol*>& inSymbols)
{
	if( fTable )
		fTable->ReleaseSymbols(inSymbols);
}



std::vector< Symbols::IFile * > VSymbolTable::RetainFilesByPathAndBaseFolder( const VFilePath &inPath, const ESymbolFileBaseFolder inBaseFolder )
{
	// Convert the input path into a relative file path
	VString path;
	GetRelativePath(inPath, inBaseFolder, path );
	if (fTable)
	{
		return fTable->RetainFilesByPathAndBaseFolder( path, inBaseFolder, fContext );
	}
	std::vector< Symbols::IFile * > ret;
	return ret;
}



std::vector< Symbols::IFile * > VSymbolTable::RetainFilesByName( const VString &inName )
{
	if (fTable)
	{
		return fTable->RetainFilesByName( inName, fContext );
	}
	std::vector< Symbols::IFile * > ret;
	return ret;
}



void VSymbolTable::ReleaseFiles(std::vector<Symbols::IFile*>& inFiles)
{
	if( fTable )
		fTable->ReleaseFiles(inFiles);
}



Symbols::ISymbol* VSymbolTable::GetSymbolOwnerByLine( Symbols::IFile *inOwnerFile, int inLineNumber )
{
	if (fTable)
	{
		return fTable->GetSymbolOwnerByLine( inOwnerFile, inLineNumber, fContext );
	}
	return NULL;
}



Symbols::ISymbol* VSymbolTable::GetSymbolByID( sLONG inID )
{
	if (fTable)
	{
		return fTable->GetSymbolByID( inID, fContext );
	}
	return NULL;
}



bool VSymbolTable::DeleteSymbolsForFile( Symbols::IFile *inFile, bool inOnlyEditedSymbols )
{
	if (fTable)
	{
		return fTable->DeleteSymbolsForFile( inFile, inOnlyEditedSymbols, fContext );
	}
	return false;
}



std::vector< Symbols::IExtraInfo * > VSymbolTable::GetExtraInformation( const Symbols::ISymbol *inOwnerSymbol, Symbols::IExtraInfo::Kind inKind )
{
	if (fTable)
	{
		return fTable->GetExtraInformation( inOwnerSymbol, inKind, fContext );
	}
	std::vector< Symbols::IExtraInfo * > ret;
	return ret;
}



bool VSymbolTable::AddExtraInformation( const Symbols::ISymbol *inOwnerSymbol, const Symbols::IExtraInfo *inInfo )
{
	if (fTable)
	{
		return fTable->AddExtraInformation( inOwnerSymbol, inInfo, fContext );
	}
	return false;
}



void VSymbolTable::SetBaseFolderPathStr(const ESymbolFileBaseFolder& inType, const XBOX::VString& inPathStr, const bool& inPosixConvert)
{
	VFilePath path(inPathStr);
	
	VString pathStr;
	if( inPosixConvert )
	{
		path.GetPosixPath(pathStr);
	}
	else
	{
		path.GetPath(pathStr);
	}
	
	fBaseFolderPosixPathStrings[inType] = pathStr;
}



void VSymbolTable::GetBaseFolderPathStr(const ESymbolFileBaseFolder& inType, XBOX::VString& outPathStr) const
{
	std::map<ESymbolFileBaseFolder, XBOX::VString>::const_iterator it = fBaseFolderPosixPathStrings.find(inType);
	if( it != fBaseFolderPosixPathStrings.end() )
		outPathStr = it->second;
}




void VSymbolTable::SetSymbolsEditionState(const Symbols::IFile* inFile, bool inEditionState, std::vector<sLONG>& outIds)
{
	if (fTable)
		fTable->SetSymbolsEditionState( inFile, inEditionState, outIds, fContext );
}



void VSymbolTable::GetFileIdsReferencingDeadSymbols(const std::vector<sLONG>& inSymbolIds, std::vector<sLONG>& outFileIds)
{
	if( fTable )
		fTable->GetFileIdsReferencingDeadSymbols(inSymbolIds, outFileIds, fContext);
}

Symbols::IFile* VSymbolTable::RetainFileByID(sLONG inID)
{
	Symbols::IFile* ret = NULL;
	
	if( fTable )
		ret = fTable->RetainFileByID(inID, fContext);
	
	return ret;
}

void VSymbolTable::GetSymbolIdsByFileAndEditionState(sLONG inFileId, bool inEditionState, std::vector<sLONG>& outSymbolIds)
{
	if( fTable )
		fTable->GetSymbolIdsByFileAndEditionState(inFileId, inEditionState, outSymbolIds, fContext);
}

#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
void VSymbolTable::Dump(const XBOX::VString& inRequest)
{
	if( fTable )
		fTable->Dump(inRequest, fContext);
}

void VSymbolTable::_MagicDump(const XBOX::VString& inTraceHeader) const
{
	xbox_assert( fContext != NULL && fTable != NULL );
	if( fContext != NULL && fTable != NULL )
	{
#if VERSIONWIN
		VFilePath path("C:/stlr.txt", FPS_POSIX);
#elif VERSIONMAC
		VFilePath path("/stlr.txt", FPS_POSIX);
#endif
		if( path.IsValid() )
		{
			VFile file(path);
			if( file.Exists() )
			{
				VFileStream stream( &file );
				if (VE_OK == stream.OpenReading())
				{
					VString contents;
					stream.GuessCharSetFromLeadingBytes( VTC_DefaultTextExport );
					stream.GetText( contents );
					stream.CloseReading();
					contents.ConvertCarriageReturns( eCRM_CR );
					
					if( contents.IsEmpty() == false )
					{
						// Each request must be writen on a single line
						VectorOfVString requests;
						contents.GetSubStrings("\r", requests);
						
						if( requests.size() > 0 )
						{
#if ACTIVATE_JSLOG
							{ JSLog log; log.SetTitle("******************************************************"); }
#endif //ACTIVATE_JSLOG
							for(VectorOfVString::const_iterator it=requests.begin(); it!=requests.end(); ++it)
							{
								// Each request ending with ! will be exectued on symbol table
								VString curReq = (*it);
								if( !curReq.BeginsWith("//") && curReq.EndsWith("!") )
								{
									// Removing !
									curReq.SubString(1, curReq.GetLength()-1);
									
									VString title("Executing STLR request : ");
									title.AppendString(curReq);
									
									JSLog log(true);
									log.SetTitle(title);
									log.Append("From", inTraceHeader);
									log.Print();
									
									// Execute request
									fTable->Dump(curReq, fContext);
								}
							}
#if ACTIVATE_JSLOG
							{ JSLog log; log.SetTitle("******************************************************"); }
#endif //ACTIVATE_JSLOG
						}
					}
				}
			}
		}
	}
}
#endif




// This section is devoted to named entities in the database.
const char* kScopesTable					=	"Scopes";			// Table
const char* kScopeFieldId					=	"Id";				// Int32, Unique, Non-Null, Primary Key, Auto-Sequence
const char* kScopeFieldName					=	"Name";				// String
const char* kScopeFieldFullName				=	"FullName";			// String
const char* kScopeFieldType					=	"Type";				// Int32, Non-Null
const char* kScopeFieldSourceId				=	"SourceId";			// Int32
const char* kScopeFieldExecutionContext		=	"ExecutionContext";	// Int32, Non-Null
const char* kScopeFieldParentId				=	"ParentId";			// Int32
const char* kScopeFieldStatus				=	"Status";			// Int32, Non-Null
const char* kScopeFieldFromLine				=	"FromLine";			// Int32
const char* kScopeFieldToLine				=	"ToLine";			// Int32
const char* kScopeFieldFromOffset			=	"FromOffset";		// Int32
const char* kScopeFieldToOffset				=	"ToOffset";			// Int32
const char* kScopeFieldConstructorFlag		=	"ConstructorFlag";	// Boolean, Non-Null

const char* kSymbolNamesTable					=	"Names";			// Table
const char* kSymbolNameFieldId					=	"Id";				// Int32, Unique, Non-Null, Primary Key, Auto-Sequence
const char* kSymbolNameFieldSourceId			=	"SourceId";			// Int32, Non-Null
const char* kSymbolNameFieldValue				=	"Value";			// String
const char* kSymbolNameFieldScopeId				=	"ScopeId";			// Int32, Non-Null
const char* kSymbolNameFieldExecutionContext	=	"ExecutionContext";	// Int32, Non-Null
const char* kSymbolNameFieldStatus				=	"Status";			// Int32

const char* kSymbolDefinitionsTable					=	"Definitions";			// Table
const char* kSymbolDefinitionFieldId				=	"Id";					// Int32, Unique, Non-Null, Primary Key, Auto-Sequence
const char* kSymbolDefinitionFieldScopeId			=	"ScopeId";				// Int32, Non-Null
const char* kSymbolDefinitionFieldNameId			=	"NameId";				// Int32, Non-Null
const char* kSymbolDefinitionFieldKind				=	"Kind";					// Int32, Non-Null
const char* kSymbolDefinitionFieldExecutionContext	=	"ExecutionContext";		// Int32, Non-Null
const char* kSymbolDefinitionFieldStatus			=	"Status";				// Int32, Non-Null
const char* kSymbolDefinitionFieldSourceId			=	"SourceId";				// Int32, Non-Null
const char* kSymbolDefinitionFieldLineNumber		=	"LineNumber";			// Int32, Non-Null

const char* kSymbolPrototypesTable					=	"Prototypes";			// Table
const char* kSymbolPrototypeFieldId					=	"Id";					// Int32, Unique, Non-Null, Primary Key, Auto-Sequence
const char* kSymbolPrototypeFieldFromName			=	"FromName";				// String
const char* kSymbolPrototypeFieldToName				=	"ToName";				// String
const char* kSymbolPrototypeFieldExecutionContext	=	"ExecutionContext";		// Int32, Non-Null
const char* kSymbolPrototypeFieldStatus				=	"Status";				// Int32
const char* kSymbolPrototypeFieldSourceId			=	"SourceId";				// Int32, Non-Null

const char* kSymbolReturnsTable						=	"Returns";	// Table
const char* kSymbolReturnFieldId					=	"Id";		// Int32, Unique, Non-Null, Primary Key, Auto-Sequence
const char* kSymbolReturnFieldScopeId				=	"ScopeId";	// Int32, Non-Null
const char* kSymbolReturnFieldName					=	"Name";		// String
const char* kSymbolReturnFieldStatus				=	"Status";	// Int32
const char* kSymbolReturnFieldSourceId				=	"SourceId";	// Int32, Non-Null

const char* kSymbolTypesTable						=	"Types";		// Table
const char* kSymbolTypeFieldId						=	"Id";			// Int32, Unique, Non-Null, Primary Key, Auto-Sequence
const char* kSymbolTypeFieldDefinitionId			=	"DefinitionId";	// Int32, Non-Null
const char*	kSymbolTypeFieldName					=	"Name";			// String
const char* kSymbolTypeFieldStatus					=	"Status";		// Int32, Non-Null
const char* kSymbolTypeFieldSourceId				=	"SourceId";		// Int32, Non-Null

const char* kSymbolReferencesTable			=	"References";	// Table
const char* kSymbolReferenceFieldId			=	"Id";			// Int32, Unique, Non-Null, Primary Key, Auto-Sequence
const char* kSymbolReferenceFieldType		=	"Type";			// Int32, Non-Null
const char* kSymbolReferenceFieldFromId		=	"FromId";		// Int32, Non-Null
const char*	kSymbolReferenceFieldToId		=	"ToId";			// Int32, Non-Null
const char* kSymbolReferenceFieldStatus		=	"Status";		// Int32, Non-Null
const char*	kSymbolReferenceFieldSourceId	=	"SourceId";		// Int32, Non-Null

const char*	kOutlineItemsTable			=	"OutlineItems";	// Table
const char*	kOutlineItemFieldId			=	"Id";			// Int32, Unique, Non-Null, Primary Key, Auto-Sequence
const char*	kOutlineItemFieldSourceId	=	"SourceId";		// Int32, Non-Null
const char*	kOutlineItemFieldName		=	"Name";			// String
const char*	kOutlineItemFieldLine		=	"Line";			// Int32, Non-Null
const char*	kOutlineItemFieldStatus		=	"Status";		// Int32, Non-Null

const char * kSymbolTable			= "Symbols";				// Table
const char * kSymbolID				= "UniqueID";				// Int32, Unique, Auto-Sequence, Non-Null, Primary Key
const char * kSymbolName			= "Name";					// String
const char * kSymbolFileID			= "FileID";					// Int32, Non-Null
const char * kSymbolPrototypeID		= "PrototypeID";			// String, Comma-separated list of Int32 IDs
const char * kSymbolOwnerID			= "OwnerID";				// Int32
const char * kSymbolKind			= "Kind";					// Int16
const char * kSymbolWAFKind			= "WAFKind";				// String
const char * kSymbolScriptDocText	= "ScriptDocText";			// Text
const char * kLineNumber			= "LineNumber";				// Int32
const char * kLineCompletionNumber	= "LineCompletionNumber";	// Int32
const char * kSymbolReturnTypeID	= "ReturnTypeID";			// String, Comma-separated list of Int32 IDs
const char * kSymbolReferenceID		= "ReferenceID";			// Int32
const char * kSymbolUndefinedState	= "UndefinedState";			// Int32
const char * kSymbolInstanceState	= "InstanceState";			// Int32
const char * kSymbolReferenceState	= "ReferenceState";			// Int32
const char * kSymbolEditionState	= "EditionState";			// Int32
const char * kSymbolFullName		= "SymbolFullName";			// String


const char * kFileTable				= "Files";					// Table
const char * kFileID				= "UniqueID";				// Int32, Unique, Auto-Sequence, Non-Null, Primary Key
const char * kFilePath				= "Path";					// String, Non-Null
const char * kFileBaseFolder		= "BaseFolder";				// Int16
const char * kFileExecContext		= "ExecutionContext";		// Int16
const char * kFileModificationTime	= "ModificationTimestamp";	// UInt64, Non-Null
const char * kFileResolvedState		= "ResolvedState";			// Int16, Non Null

const char * kExtrasTable			= "ExtraInfo";				// Table
const char * kExtraID				= "UniqueID";				// Int32, Unique, Auto-Sequence, Non-Null, Primary Key
const char * kExtrasOwnerSymbolID	= "OwnerID";				// Int32, Non-Null, Primary Key
const char * kExtrasKind			= "Kind";					// Int32, Non-Null
const char * kExtrasStringData		= "StringData";				// String
const char * kExtrasIntegerData		= "IntegerData";			// Int32

const char * kVersionTable			= "Version";				// Table
const char * kVersionNumber			= "VersionNumber";			// Int32, Non-Null






class SymbolCollectionManager : public DB4DCollectionManager
{
private:
	const std::vector< Symbols::ISymbol * >&	fSymbolList;
	CDB4DTable*									fTable;
	CDB4DField*									fSymbolIDField;
	CDB4DField*									fSymbolNameField;
	CDB4DField*									fSymbolFileIDField;
	CDB4DField*									fSymbolPrototypeField;
	CDB4DField*									fSymbolOwnerField;
	CDB4DField*									fSymbolKindField;
	CDB4DField*									fSymbolWAFKindField;
	CDB4DField*									fSymbolScriptDocField;
	CDB4DField*									fSymbolLineNumberField;
	CDB4DField*									fSymbolLineCompletionNumberField;
	CDB4DField*									fSymbolReturnTypeField;
	CDB4DField*									fSymbolReferenceIDField;
	CDB4DField*									fSymbolUndefinedStateField;
	CDB4DField*									fSymbolInstanceStateField;
	CDB4DField*									fSymbolReferenceStateField;
	CDB4DField*									fSymbolEditionStateField;
	CDB4DField*									fSymbolFullNameField;
	
public:
	SymbolCollectionManager( const std::vector< Symbols::ISymbol * > &inSymbolList, CDB4DTable *in4DTable, CDB4DBaseContext *inContext ):
		fSymbolList( inSymbolList ),
		fTable( in4DTable )
	{
		fSymbolIDField						=	fTable->FindAndRetainField( kSymbolID,				inContext );
		fSymbolNameField					=	fTable->FindAndRetainField( kSymbolName,			inContext );
		fSymbolFileIDField					=	fTable->FindAndRetainField( kSymbolFileID,			inContext );
		fSymbolPrototypeField				=	fTable->FindAndRetainField( kSymbolPrototypeID,		inContext );
		fSymbolOwnerField					=	fTable->FindAndRetainField( kSymbolOwnerID,			inContext );
		fSymbolKindField					=	fTable->FindAndRetainField( kSymbolKind,			inContext );
		fSymbolWAFKindField					=	fTable->FindAndRetainField( kSymbolWAFKind,			inContext );
		fSymbolScriptDocField				=	fTable->FindAndRetainField( kSymbolScriptDocText,	inContext );
		fSymbolLineNumberField				=	fTable->FindAndRetainField( kLineNumber,			inContext );
		fSymbolLineCompletionNumberField	=	fTable->FindAndRetainField( kLineCompletionNumber,	inContext );
		fSymbolReturnTypeField				=	fTable->FindAndRetainField( kSymbolReturnTypeID,	inContext );
		fSymbolReferenceIDField				=	fTable->FindAndRetainField( kSymbolReferenceID,		inContext );
		fSymbolUndefinedStateField			=	fTable->FindAndRetainField( kSymbolUndefinedState,	inContext );
		fSymbolInstanceStateField			=	fTable->FindAndRetainField( kSymbolInstanceState,	inContext );
		fSymbolReferenceStateField			=	fTable->FindAndRetainField( kSymbolReferenceState,	inContext );
		fSymbolEditionStateField			=	fTable->FindAndRetainField( kSymbolEditionState,	inContext );
		fSymbolFullNameField				=	fTable->FindAndRetainField( kSymbolFullName,		inContext );
	}
	
	
	virtual ~SymbolCollectionManager()
	{
		ReleaseRefCountable(&fSymbolIDField);
		ReleaseRefCountable(&fSymbolNameField);
		ReleaseRefCountable(&fSymbolFileIDField);
		ReleaseRefCountable(&fSymbolPrototypeField);
		ReleaseRefCountable(&fSymbolOwnerField);
		ReleaseRefCountable(&fSymbolKindField);
		ReleaseRefCountable(&fSymbolWAFKindField);
		ReleaseRefCountable(&fSymbolScriptDocField);
		ReleaseRefCountable(&fSymbolLineNumberField);
		ReleaseRefCountable(&fSymbolLineCompletionNumberField);
		ReleaseRefCountable(&fSymbolReturnTypeField);
		ReleaseRefCountable(&fSymbolReferenceIDField);
		ReleaseRefCountable(&fSymbolUndefinedStateField);
		ReleaseRefCountable(&fSymbolInstanceStateField);
		ReleaseRefCountable(&fSymbolReferenceStateField);
		ReleaseRefCountable(&fSymbolEditionStateField);
		ReleaseRefCountable(&fSymbolFullNameField);
	}
	
	virtual VErrorDB4D SetCollectionSize(RecIDType size, Boolean ClearData = true)
	{
		return VE_DB4D_NOTIMPLEMENTED;
	}
	
	virtual RecIDType GetCollectionSize()
	{
		return fSymbolList.size();
	}
	
	virtual sLONG GetNumberOfColumns()
	{
		return 17;	// One for each of the columns on the symbol
	}
	
	virtual	bool AcceptRawData()
	{
		return false;
	}
	
	virtual CDB4DField* GetColumnRef(sLONG ColumnNumber)
	{
		// Column numbering starts from 1
		switch (ColumnNumber)
		{
			case 1:
				return fSymbolIDField;
				
			case 2:
				return fSymbolNameField;
				
			case 3:
				return fSymbolFileIDField;
				
			case 4:
				return fSymbolPrototypeField;
				
			case 5:
				return fSymbolOwnerField;
				
			case 6:
				return fSymbolKindField;
				
			case 7:
				return fSymbolWAFKindField;
				
			case 8:
				return fSymbolScriptDocField;
				
			case 9:
				return fSymbolLineNumberField;
				
			case 10:
				return fSymbolLineCompletionNumberField;
				
			case 11:
				return fSymbolReturnTypeField;
				
			case 12:
				return fSymbolReferenceIDField;
				
			case 13:
				return fSymbolUndefinedStateField;

			case 14:
				return fSymbolInstanceStateField;
				
			case 15:
				return fSymbolReferenceStateField;

			case 16:
				return fSymbolEditionStateField;

			case 17:
				return fSymbolFullNameField;
		}
		return NULL;
	}
	
	virtual VErrorDB4D SetNthElement(RecIDType ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle& inValue)
	{
		return VE_DB4D_NOTIMPLEMENTED;
	}
		
	virtual	XBOX::VSize GetNthElementSpace( RecIDType inIndex, sLONG inColumnNumber, XBOX::ValueKind inRawValueKind, bool *outRejected, XBOX::VError& outError)
	{
		return 0;
	}
	
	virtual	void* WriteNthElementToPtr( RecIDType inIndex, sLONG inColumnNumber, void *outRawValue, XBOX::ValueKind inRawValueKind, bool *outRejected, XBOX::VError& outError)
	{
		return NULL;
	}
	
	virtual VErrorDB4D SetNthElementRawData(RecIDType ElemNumber, sLONG ColumnNumber, const void *inRawValue, XBOX::ValueKind inRawValueKind, bool *outRejected)
	{
		return VE_DB4D_NOTIMPLEMENTED;
	}
	
	virtual VErrorDB4D GetNthElement(RecIDType ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle*& outValue, bool *outDisposeIt)
	{
		if (VTask::GetCurrent()->IsDying())
		{
			// If our thread has been asked to die, we're going to bail out as quickly as possible, so we
			// will just fail at the task of saving out elements.
			return VE_DB4D_CANNOTSAVERECORD;
		}
		
		// Element numbers start at 1, as do column numbers
		Symbols::ISymbol *sym = fSymbolList[ ElemNumber - 1 ];
		outValue = NULL;
		switch (ColumnNumber)
		{
			case 1:
			{
				if (testAssert( sym->HasID() ))
				{
					outValue = new VLong( sym->GetID() );
				}
			}
			break;
				
			case 2:
			{
				outValue = new VString( sym->GetName() );
			}
			break;
				
			case 3:
			{
				Symbols::IFile *file = sym->GetFile();
				if (file)
				{
					outValue = new VLong( file->GetID() );
				}
			}
			break;
				
			case 4:
			{
				std::vector< Symbols::ISymbol * > protos = sym->GetPrototypes();
				if (!protos.empty())
				{
					VectorOfVString parts;
					for (std::vector< Symbols::ISymbol * >::iterator iter = protos.begin(); iter != protos.end(); ++iter)
					{
						VString id;
						id.FromLong( (*iter)->GetID() );
						parts.push_back( id );
					}
					outValue = new VString();
					((VString *)outValue )->Join( parts, ',' );
				}
			}
			break;
				
			case 5:
			{
				Symbols::ISymbol *owner = sym->GetOwner();
				if (owner  && owner->HasID() )
				{
					outValue = new VLong( owner->GetID() );
				}
			}
			break;
				
			case 6:
			{
				outValue = new VLong( sym->GetFullKindInformation() );
			}
			break;
				
			case 7:
			{
				outValue = new VString( sym->GetWAFKind() );
			}
			break;
				
			case 8:
			{
				outValue = new VString( sym->GetScriptDocComment() );
			}
			break;
				
			case 9:
			{
				outValue = new VLong( sym->GetLineNumber() );
			}
			break;
				
			case 10:
			{
				outValue = new VLong( sym->GetLineCompletionNumber() );
			}
			break;
				
			case 11:
			{
				std::vector< Symbols::ISymbol * > protos = sym->GetReturnTypes();
				if (!protos.empty())
				{
					VectorOfVString parts;
					for (std::vector< Symbols::ISymbol * >::iterator iter = protos.begin(); iter != protos.end(); ++iter)
					{
						VString id;
						id.FromLong( (*iter)->GetID() );
						parts.push_back( id );
					}
					outValue = new VString();
					((VString *)outValue )->Join( parts, ',' );
				}
			}
			break;
				
			case 12:
			{
				if( sym->GetReferenceState() )
				{
					const Symbols::ISymbol *ref = sym->RetainReferencedSymbol();
					sLONG id = ref->GetID();
					ref->Release();
					xbox_assert( sym->GetID() != id );
					outValue = new VLong( id );
				}
			}
			break;

			case 13:
			{
				outValue = new VLong( sym->GetUndefinedState() );
			}
			break;
				
			case 14:
			{
				outValue = new VLong( sym->GetInstanceState() );
			}
			break;
				
			case 15:
			{
				outValue = new VLong( sym->GetReferenceState() );
			}
			break;

			case 16:
			{
				outValue = new VLong( sym->GetEditionState() );
			}
			break;

			case 17:
			{
				outValue = new VString( sym->GetFullName() );
			}
			break;
				
			default:
				return VE_DB4D_INVALIDRECORD;
		}
		*outDisposeIt = (outValue != NULL);
		return VE_OK;
	}
	
	virtual VErrorDB4D AddOneElement(sLONG ColumnNumber, const XBOX::VValueSingle& inValue)
	{
		return VE_DB4D_NOTIMPLEMENTED;
	}
	
	virtual sLONG GetSignature()
	{
		return 0;
	}
	
	virtual XBOX::VError PutInto(XBOX::VStream& outStream)
	{
		return VE_DB4D_NOTIMPLEMENTED;
	}
};



/*
 *	SCOPE MANAGEMENT
 */
VSymbolTable::SymbolUpdateParameter::SymbolUpdateParameter():
fMustSetStatus(false),
fMustGetStatus(false),
fStatus(eSymbolStatusUpToDate),
fMustSetBoundaries(false),
fMustGetBoundaries(false),
fFromLine(0),
fFromOffset(0),
fToLine(0),
fToOffset(0),
fMustSetConstructorFlag(false),
fMustGetConstructorFlag(false),
fConstructorFlag(false)
{
	
}

VSymbolTable::SymbolUpdateParameter::~SymbolUpdateParameter()
{
	
}

bool VSymbolTable::SymbolUpdateParameter::MustSetStatus() const
{
	return fMustSetStatus;
}

void VSymbolTable::SymbolUpdateParameter::MustSetStatus(const bool& inMust)
{
	fMustSetStatus = inMust;
}

bool VSymbolTable::SymbolUpdateParameter::MustGetStatus() const
{
	return fMustGetStatus;
}

void VSymbolTable::SymbolUpdateParameter::MustGetStatus(const bool& inMust)
{
	fMustGetStatus = inMust;
}

void VSymbolTable::SymbolUpdateParameter::SetStatus(const ESymbolStatus& inStatus)
{
	fStatus = inStatus;
}

void VSymbolTable::SymbolUpdateParameter::GetStatus(ESymbolStatus& outStatus) const
{
	outStatus = fStatus;
}

bool VSymbolTable::SymbolUpdateParameter::MustSetBoundaries() const
{
	return fMustSetBoundaries;
}

void VSymbolTable::SymbolUpdateParameter::MustSetBoundaries(const bool& inMust)
{
	fMustSetBoundaries = inMust;
}

bool VSymbolTable::SymbolUpdateParameter::MustGetBoundaries() const
{
	return fMustGetBoundaries;
}

void VSymbolTable::SymbolUpdateParameter::MustGetBoundaries(const bool& inMust)
{
	fMustGetBoundaries = inMust;
}

void VSymbolTable::SymbolUpdateParameter::SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset)
{
	fFromLine	= inFromLine;
	fFromOffset = inFromOffset;
	fToLine		= inToLine;
	fToOffset	= inToOffset;
}

void VSymbolTable::SymbolUpdateParameter::GetBoundaries(sLONG& outFromLine, sLONG& outFromOffset, sLONG& outToLine, sLONG& outToOffset) const
{
	outFromLine		= fFromLine;
	outFromOffset	= fFromOffset;
	outToLine		= fToLine;
	outToOffset		= fToOffset;
}

bool VSymbolTable::SymbolUpdateParameter::MustSetConstructorFlag() const
{
	return fMustSetConstructorFlag;
}

void VSymbolTable::SymbolUpdateParameter::MustSetConstructorFlag(const bool& inMust)
{
	fMustSetConstructorFlag = inMust;
}

bool VSymbolTable::SymbolUpdateParameter::MustGetConstructorFlag() const
{
	return fMustGetConstructorFlag;
}

void VSymbolTable::SymbolUpdateParameter::MustGetConstructorFlag(const bool& inMust)
{
	fMustGetConstructorFlag = inMust;
}

void VSymbolTable::SymbolUpdateParameter::SetConstructorFlag(const bool& inFlag)
{
	fConstructorFlag = inFlag;
}

void VSymbolTable::SymbolUpdateParameter::GetConstructorFlag(bool& outFlag) const
{
	outFlag = fConstructorFlag;
}






bool VSymbolTable::InnerSymbolTable::GetGlobalScope(const ESymbolFileExecContext& inExecutionContext, ISymbolUpdateParameters* inUpdateParameters, sLONG& outScopeId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fScopesTable != NULL && fScopeFieldType != NULL && fScopeFieldExecutionContext != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldType != NULL && fScopeFieldExecutionContext != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			VLong type(eSymbolScopeTypeGlobal);
			query->AddCriteria(fScopeFieldType, DB4D_Equal, type);
			
			query->AddLogicalOperator(DB4D_And);
			VLong exeContext(inExecutionContext);
			query->AddCriteria(fScopeFieldExecutionContext, DB4D_Equal, exeContext);
			
			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					if( inUpdateParameters == NULL )
					{
						// As we don't plan to use the record for write operations, we DO NOT LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
						if( record != NULL )
						{
							outScopeId = GetIntegerField(record, fScopeFieldId, NULL);
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
					else
					{
						// As we plan to use the record for write operations, we LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Keep_Lock_With_Record, inContext );
						if( record != NULL )
						{
							outScopeId = GetIntegerField(record, fScopeFieldId, NULL);
							
							if( inUpdateParameters->MustSetStatus() == true )
							{
								ESymbolStatus status;
								inUpdateParameters->GetStatus(status);
								record->SetLong(fScopeFieldStatus,	status);
								
								// Save
								record->Save();
							}
							
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

void VSymbolTable::InnerSymbolTable::SetGlobalScope(const ESymbolFileExecContext& inExecutionContext, sLONG& outScopeId, CDB4DBaseContext* inContext)
{
	xbox_assert( inContext != NULL && fScopesTable != NULL && fScopeFieldId != NULL && fScopeFieldName != NULL && fScopeFieldType != NULL && fScopeFieldSourceId != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldParentId != NULL && fScopeFieldStatus != NULL && fScopeFieldConstructorFlag != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldId != NULL && fScopeFieldName != NULL && fScopeFieldType != NULL && fScopeFieldSourceId != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldParentId != NULL && fScopeFieldStatus != NULL && fScopeFieldConstructorFlag != NULL )
	{
		CDB4DRecord* record = fScopesTable->NewRecord(inContext);
		if( record != NULL )
		{
			// Set field values
			record->SetString(	fScopeFieldName,				CVSTR(""));
			record->SetLong(	fScopeFieldType,				eSymbolScopeTypeGlobal);
			record->SetLong(	fScopeFieldExecutionContext,	inExecutionContext);

			record->SetFieldToNull(fScopeFieldSourceId);
			record->SetFieldToNull(fScopeFieldParentId);

			record->SetLong(	fScopeFieldStatus,	eSymbolStatusUpToDate);

			record->SetLong(	fScopeFieldFromLine,			0);
			record->SetLong(	fScopeFieldToLine,				0);
			record->SetLong(	fScopeFieldFromOffset,			0);
			record->SetLong(	fScopeFieldToOffset,			0);
			record->SetBoolean( fScopeFieldConstructorFlag,		false);

			// Save
			record->Save();
			
			// Get scope id
			outScopeId = GetIntegerField(record, fScopeFieldId, NULL);
			
			ReleaseRefCountable(&record);
		}
	}
}

bool VSymbolTable::InnerSymbolTable::GetScope(const XBOX::VString& inScopeName, const ESymbolScopeType& inScopeType, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, ISymbolUpdateParameters* inUpdateParameters, sLONG& outScopeId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fScopesTable != NULL && fScopeFieldName != NULL && fScopeFieldType != NULL && fScopeFieldSourceId != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldParentId != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldName != NULL && fScopeFieldType != NULL && fScopeFieldSourceId != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldParentId != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			query->AddCriteria(fScopeFieldName, DB4D_Equal, inScopeName);

			if( inScopeType != eSymbolScopeTypeAll )
			{
				query->AddLogicalOperator(DB4D_And);
				VLong type(inScopeType);
				query->AddCriteria(fScopeFieldType, DB4D_Equal, type);
			}

			if( inSourceId != -1 )
			{
				query->AddLogicalOperator(DB4D_And);
				VLong source(inSourceId);
				query->AddCriteria(fScopeFieldSourceId, DB4D_Equal, source);
			}

			query->AddLogicalOperator(DB4D_And);
			VLong exeContext(inExecutionContext);
			query->AddCriteria(fScopeFieldExecutionContext, DB4D_Equal, exeContext);

			if( inParentScopeId != -1 )
			{
				query->AddLogicalOperator(DB4D_And);
				VLong parentId(inParentScopeId);
				query->AddCriteria(fScopeFieldParentId, DB4D_Equal, parentId);
			}
			
			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					if( inUpdateParameters == NULL )
					{
						// As we don't plan to use the record for write operations, we DO NOT LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
						if( record != NULL )
						{
							outScopeId = GetIntegerField(record, fScopeFieldId, NULL);
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
					else
					{
						// As we plan to use the record for write operations, we LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Keep_Lock_With_Record, inContext );
						if( record != NULL )
						{
							outScopeId = GetIntegerField(record, fScopeFieldId, NULL);

							bool anyFieldHasBeenUpdated = false;
							
							if( inUpdateParameters->MustSetStatus() == true )
							{
								ESymbolStatus status;
								inUpdateParameters->GetStatus(status);
								record->SetLong(fScopeFieldStatus,	status);
								anyFieldHasBeenUpdated = true;
							}
							
							if( inUpdateParameters->MustSetBoundaries() == true )
							{
								sLONG fromLine;
								sLONG fromOffset;
								sLONG toLine;
								sLONG toOffset;
								inUpdateParameters->GetBoundaries(fromLine, fromOffset, toLine, toOffset);
								record->SetLong(fScopeFieldFromLine,	fromLine);
								record->SetLong(fScopeFieldToLine,		toLine);
								record->SetLong(fScopeFieldFromOffset,	fromOffset);
								record->SetLong(fScopeFieldToOffset,	toOffset);
								anyFieldHasBeenUpdated = true;
							}
							
							if( inUpdateParameters->MustSetConstructorFlag() == true )
							{
								bool constructorFlag;
								inUpdateParameters->GetConstructorFlag(constructorFlag);
								record->SetBoolean(fScopeFieldConstructorFlag, constructorFlag);
								anyFieldHasBeenUpdated = true;
							}

							if( anyFieldHasBeenUpdated == true )
							{
								// Save
								record->Save();
							}

							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetScope(const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, const sLONG& inLineNumber, sLONG& outScopeId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fScopesTable != NULL && fScopeFieldSourceId != NULL && fScopeFieldFromLine != NULL && fScopeFieldToLine != NULL && fScopeFieldId != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldSourceId != NULL && fScopeFieldFromLine != NULL && fScopeFieldToLine != NULL && fScopeFieldId != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			VLong executionContext(inExecutionContext);
			query->AddCriteria(fScopeFieldExecutionContext, DB4D_Equal, executionContext);
			
			query->AddLogicalOperator(DB4D_And);
			VLong sourceId(inSourceId);
			query->AddCriteria(fScopeFieldSourceId, DB4D_Equal, sourceId);

			VLong line(inLineNumber);

			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fScopeFieldFromLine, DB4D_LowerOrEqual, line);

			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fScopeFieldToLine, DB4D_GreaterOrEqual, line);

			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				if( total > 0 )
				{
					results->SortSelection(fScopeFieldFromLine->GetID(inContext), false, NULL, inContext);

					CDB4DRecord* record = results->LoadSelectedRecord( 1, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outScopeId = GetIntegerField(record, fScopeFieldId, NULL);

						ESymbolScopeType scopeType = GetIntegerField(record, fScopeFieldType, NULL);
						if( scopeType != eSymbolScopeTypeUndefined )
							found = true;
						
						ReleaseRefCountable(&record);
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetScope(const XBOX::VString& inScopeName, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromScopeId, const sLONG& inToScopeId, sLONG& outScopeId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fScopesTable != NULL && fScopeFieldName != NULL && fScopeFieldType != NULL && fScopeFieldSourceId != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldParentId != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldName != NULL && fScopeFieldType != NULL && fScopeFieldSourceId != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldParentId != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			query->AddCriteria(fScopeFieldName, DB4D_Equal, inScopeName);
			
			query->AddLogicalOperator(DB4D_And);
			VLong exeContext(inExecutionContext);
			query->AddCriteria(fScopeFieldExecutionContext, DB4D_Equal, exeContext);
			
			query->AddLogicalOperator(DB4D_And);
			VLong fromScopeId(inFromScopeId);
			query->AddCriteria(fScopeFieldParentId, DB4D_Equal, fromScopeId);
			
			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outScopeId = GetIntegerField(record, fScopeFieldId, NULL);
						ReleaseRefCountable(&record);
							
						found = true;
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	if( found == false )
	{
		if( inFromScopeId != inToScopeId )
		{
			sLONG nextScope;
			if( GetScopeParentId(inFromScopeId, nextScope, inContext) == true )
				found = GetScope(inScopeName, inExecutionContext, nextScope, inToScopeId, outScopeId, inContext);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetScopes(const XBOX::VString& inScopeName, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromScopeId, const sLONG& inToScopeId, std::vector<sLONG>& outScopeIds, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fScopesTable != NULL && fScopeFieldName != NULL && fScopeFieldType != NULL && fScopeFieldSourceId != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldParentId != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldName != NULL && fScopeFieldType != NULL && fScopeFieldSourceId != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldParentId != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			query->AddCriteria(fScopeFieldName, DB4D_Equal, inScopeName);
			
			query->AddLogicalOperator(DB4D_And);
			VLong exeContext(inExecutionContext);
			query->AddCriteria(fScopeFieldExecutionContext, DB4D_Equal, exeContext);
			
			query->AddLogicalOperator(DB4D_And);
			VLong fromScopeId(inFromScopeId);
			query->AddCriteria(fScopeFieldParentId, DB4D_Equal, fromScopeId);
			
			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outScopeIds.push_back(GetIntegerField(record, fScopeFieldId, NULL));
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	if( found == false )
	{
		if( inFromScopeId != inToScopeId )
		{
			sLONG nextScope;
			if( GetScopeParentId(inFromScopeId, nextScope, inContext) == true )
				found = GetScopes(inScopeName, inExecutionContext, nextScope, inToScopeId, outScopeIds, inContext);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetScopes(const XBOX::VString& inScopeName, const std::vector<sLONG>& inFromScopeIds, std::vector<sLONG>& outScopeIds, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fScopesTable != NULL && fScopeFieldParentId != NULL && fScopeFieldName != NULL && fScopeFieldId != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldParentId != NULL && fScopeFieldName != NULL && fScopeFieldId != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			VIndex fromCount = inFromScopeIds.size();
			for(VIndex fromIndex=0; fromIndex<fromCount; ++fromIndex)
			{
				if( fromIndex > 0 )
					query->AddLogicalOperator(DB4D_OR);
				
				VLong fromId(inFromScopeIds[fromIndex]);
				query->AddCriteria(fScopeFieldParentId, DB4D_Equal, fromId);
			}
			
			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						VString scopeName = GetStringField(record, fScopeFieldName);
						if( scopeName == inScopeName )
						{
							outScopeIds.push_back(GetIntegerField(record, fScopeFieldId, NULL));
							found = true;
						}
						ReleaseRefCountable(&record);
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

void VSymbolTable::InnerSymbolTable::SetScopeFullName(const sLONG inScopeId, const XBOX::VString& inScopeFullName, CDB4DBaseContext* inContext)
{
	xbox_assert( inContext != NULL && fScopesTable != NULL && fScopeFieldId != NULL && fScopeFieldFullName != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldId != NULL && fScopeFieldFullName != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			VLong scopeId(inScopeId);
			query->AddCriteria(fScopeFieldId, DB4D_Equal, scopeId);
			
			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we plan to use the record for write operations, we LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Keep_Lock_With_Record, inContext );
					if( record != NULL )
					{
						record->SetString( fScopeFieldFullName, inScopeFullName );
						record->Save();
						
						ReleaseRefCountable(&record);
					}
				}
			}
			ReleaseRefCountable(&results);
		}
		ReleaseRefCountable(&query);
	}
}

void VSymbolTable::InnerSymbolTable::SetScopeType(const sLONG inScopeId, const ESymbolScopeType& inScopeType, CDB4DBaseContext* inContext)
{
	xbox_assert( inContext != NULL && fScopesTable != NULL && fScopeFieldId != NULL && fScopeFieldType != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldId != NULL && fScopeFieldType != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			VLong scopeId(inScopeId);
			query->AddCriteria(fScopeFieldId, DB4D_Equal, scopeId);
			
			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we plan to use the record for write operations, we LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Keep_Lock_With_Record, inContext );
					if( record != NULL )
					{
						record->SetLong( fScopeFieldType, inScopeType);
						record->Save();

						ReleaseRefCountable(&record);
					}
				}
			}
			ReleaseRefCountable(&results);
		}
		ReleaseRefCountable(&query);
	}
}

void VSymbolTable::InnerSymbolTable::SetScope(const XBOX::VString& inScopeName, const ESymbolScopeType& inScopeType, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset, const bool& inConstructorFlag, sLONG& outScopeId, CDB4DBaseContext* inContext)
{
	xbox_assert( inContext != NULL && fScopesTable != NULL && fScopeFieldId != NULL && fScopeFieldName != NULL && fScopeFieldFullName != NULL && fScopeFieldType != NULL && fScopeFieldSourceId != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldParentId != NULL && fScopeFieldStatus != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldId != NULL && fScopeFieldName != NULL && fScopeFieldFullName != NULL && fScopeFieldType != NULL && fScopeFieldSourceId != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldParentId != NULL && fScopeFieldStatus != NULL )
	{
		CDB4DRecord* record = fScopesTable->NewRecord(inContext);
		if( record != NULL )
		{
			// Set field values
			record->SetString(	fScopeFieldName,				inScopeName);
			record->SetString(	fScopeFieldFullName,			inScopeName);
			record->SetLong(	fScopeFieldType,				inScopeType);
			record->SetLong(	fScopeFieldSourceId,			inSourceId);
			record->SetLong(	fScopeFieldExecutionContext,	inExecutionContext);
			record->SetLong(	fScopeFieldParentId,			inParentScopeId);
			record->SetLong(	fScopeFieldStatus,				eSymbolStatusUpToDate);
			record->SetLong(	fScopeFieldFromLine,			inFromLine);
			record->SetLong(	fScopeFieldFromOffset,			inFromOffset);
			record->SetLong(	fScopeFieldToLine,				inToLine);
			record->SetLong(	fScopeFieldToOffset,			inToOffset);
			record->SetBoolean( fScopeFieldConstructorFlag,		inConstructorFlag);
			
			// Save
			record->Save();

			// Get scope id
			outScopeId = GetIntegerField(record, fScopeFieldId, NULL);
			
			ReleaseRefCountable(&record);
		}
	}
}

void VSymbolTable::InnerSymbolTable::PrepareUpdates(const sLONG& inSourceId, const bool& inOutlineOnly, CDB4DBaseContext* inContext)
{
	if( inOutlineOnly == true )
	{
		_PrepareUpdates(inSourceId, fOutlineItemsTable, fOutlineItemFieldSourceId, fOutlineItemFieldStatus, inContext);
	}
	else
	{
		_PrepareUpdates(inSourceId, fScopesTable,				fScopeFieldSourceId,			fScopeFieldStatus,				inContext);
		_PrepareUpdates(inSourceId, fSymbolNamesTable,			fSymbolNameFieldSourceId,		fSymbolNameFieldStatus,			inContext);
		_PrepareUpdates(inSourceId, fSymbolReturnsTable,		fSymbolReturnFieldSourceId,		fSymbolReturnFieldStatus,		inContext);
		_PrepareUpdates(inSourceId, fSymbolPrototypesTable,		fSymbolPrototypeFieldSourceId,	fSymbolPrototypeFieldStatus,	inContext);
		_PrepareUpdates(inSourceId, fSymbolDefinitionsTable,	fSymbolDefinitionFieldSourceId,	fSymbolDefinitionFieldStatus,	inContext);
		_PrepareUpdates(inSourceId, fSymbolTypesTable,			fSymbolTypeFieldSourceId,		fSymbolTypeFieldStatus,			inContext);
		_PrepareUpdates(inSourceId, fSymbolReferencesTable,		fSymbolReferenceFieldSourceId,	fSymbolReferenceFieldStatus,	inContext);
	}
}

void VSymbolTable::InnerSymbolTable::FinalizeUpdates(const sLONG& inSourceId, const bool& inOutlineOnly, CDB4DBaseContext* inContext)
{
	if( inOutlineOnly == true )
	{
		_FinalizeUpdates(inSourceId, fOutlineItemsTable, fOutlineItemFieldSourceId, fOutlineItemFieldStatus, inContext);
	}
	else
	{
		_FinalizeUpdates(inSourceId, fScopesTable,				fScopeFieldSourceId,			fScopeFieldStatus,				inContext);
		_FinalizeUpdates(inSourceId, fSymbolNamesTable,			fSymbolNameFieldSourceId,		fSymbolNameFieldStatus,			inContext);
		_FinalizeUpdates(inSourceId, fSymbolReturnsTable,		fSymbolReturnFieldSourceId,		fSymbolReturnFieldStatus,		inContext);
		_FinalizeUpdates(inSourceId, fSymbolPrototypesTable,	fSymbolPrototypeFieldSourceId,	fSymbolPrototypeFieldStatus,	inContext);
		_FinalizeUpdates(inSourceId, fSymbolDefinitionsTable,	fSymbolDefinitionFieldSourceId, fSymbolDefinitionFieldStatus,	inContext);
		_FinalizeUpdates(inSourceId, fSymbolTypesTable,			fSymbolTypeFieldSourceId,		fSymbolTypeFieldStatus,			inContext);
		_FinalizeUpdates(inSourceId, fSymbolReferencesTable,	fSymbolReferenceFieldSourceId,	fSymbolReferenceFieldStatus,	inContext);
	}
}

bool VSymbolTable::InnerSymbolTable::GetScopeBoundaries(const sLONG& inScopeId, sLONG& outFromLine, sLONG& outFromOffset, sLONG& outToLine, sLONG& outToOffset, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fScopesTable && fScopeFieldId != NULL && fScopeFieldFromLine != NULL && fScopeFieldFromOffset != NULL && fScopeFieldToLine != NULL && fScopeFieldToOffset != NULL );
	if( inContext != NULL && fScopesTable && fScopeFieldId != NULL && fScopeFieldFromLine != NULL && fScopeFieldFromOffset != NULL && fScopeFieldToLine != NULL && fScopeFieldToOffset != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			VLong scopeId(inScopeId);
			query->AddCriteria(fScopeFieldId , DB4D_Equal, scopeId);
			
			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outFromLine		= GetIntegerField(record, fScopeFieldFromLine, NULL);
						outFromOffset	= GetIntegerField(record, fScopeFieldFromOffset, NULL);
						outToLine		= GetIntegerField(record, fScopeFieldToLine, NULL);
						outToOffset		= GetIntegerField(record, fScopeFieldToOffset, NULL);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetScopeSourceId(const sLONG& inScopeId, sLONG& outSourceId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fScopesTable && fScopeFieldId != NULL && fScopeFieldSourceId != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldId != NULL && fScopeFieldSourceId != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			VLong scopeId(inScopeId);
			query->AddCriteria(fScopeFieldId , DB4D_Equal, scopeId);
			
			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outSourceId = GetIntegerField(record, fScopeFieldSourceId, NULL);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetScopeName(const sLONG& inScopeId, XBOX::VString& outName, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fScopesTable && fScopeFieldId != NULL && fScopeFieldName != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldId != NULL && fScopeFieldName != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			VLong scopeId(inScopeId);
			query->AddCriteria(fScopeFieldId , DB4D_Equal, scopeId);

			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						record->GetString(fScopeFieldName, outName);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetScopeFullName(const sLONG& inScopeId, XBOX::VString& outName, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fScopesTable && fScopeFieldId != NULL && fScopeFieldFullName != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldId != NULL && fScopeFieldFullName != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			VLong scopeId(inScopeId);
			query->AddCriteria(fScopeFieldId , DB4D_Equal, scopeId);
			
			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						record->GetString(fScopeFieldFullName, outName);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetScopeType(const sLONG& inScopeId, ESymbolScopeType& outType, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fScopesTable && fScopeFieldId != NULL && fScopeFieldType != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldId != NULL && fScopeFieldType != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			VLong scopeId(inScopeId);
			query->AddCriteria(fScopeFieldId , DB4D_Equal, scopeId);
			
			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outType = GetIntegerField(record, fScopeFieldType, NULL);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetScopeParentId(const sLONG& inScopeId, sLONG& outParentId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fScopesTable && fScopeFieldId != NULL && fScopeFieldParentId != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldId != NULL && fScopeFieldParentId != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			VLong scopeId(inScopeId);
			query->AddCriteria(fScopeFieldId , DB4D_Equal, scopeId);
			
			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outParentId = GetIntegerField(record, fScopeFieldParentId, NULL);
						
						ReleaseRefCountable(&record);
						
						if( outParentId != 0 )
							found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

/*
 *	SOURCE MANAGEMENT
 */
void VSymbolTable::InnerSymbolTable::DeleteSource(const sLONG& inSourceId, CDB4DBaseContext* inContext)
{
	_DropSymbols(inSourceId, fScopesTable,				fScopeFieldSourceId,			inContext);
	_DropSymbols(inSourceId, fSymbolNamesTable,			fSymbolNameFieldSourceId,		inContext);
	_DropSymbols(inSourceId, fSymbolDefinitionsTable,	fSymbolDefinitionFieldSourceId, inContext);
	_DropSymbols(inSourceId, fSymbolPrototypesTable,	fSymbolPrototypeFieldSourceId,	inContext);
	_DropSymbols(inSourceId, fSymbolReturnsTable,		fSymbolReturnFieldSourceId,		inContext);
	_DropSymbols(inSourceId, fSymbolTypesTable,			fSymbolTypeFieldSourceId,		inContext);
	_DropSymbols(inSourceId, fSymbolReferencesTable,	fSymbolReferenceFieldSourceId,	inContext);
	_DropSymbols(inSourceId, fOutlineItemsTable,		fOutlineItemFieldSourceId,		inContext);
	_DropSymbols(inSourceId, fFilesTable,				fFileIDField,					inContext);
}

bool VSymbolTable::InnerSymbolTable::GetSourcePath(const sLONG& inSourceId, XBOX::VString& outSourcePath, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fFilesTable != NULL && fFilePathField != NULL );
	if( inContext != NULL && fFilesTable != NULL && fFilePathField != NULL )
	{
		CDB4DQuery* query = fFilesTable->NewQuery();
		if( query != NULL )
		{
			VLong sourceId(inSourceId);
			query->AddCriteria(fFileIDField , DB4D_Equal, sourceId);
			
			CDB4DSelection* results = fFilesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						record->GetString(fFilePathField, outSourcePath);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetSourceBaseFolder(const sLONG& inSourceId, ESymbolFileBaseFolder& outSourceBaseFolder, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fFilesTable != NULL && fFileBaseFolderField != NULL );
	if( inContext != NULL && fFilesTable != NULL && fFileBaseFolderField != NULL )
	{
		CDB4DQuery* query = fFilesTable->NewQuery();
		if( query != NULL )
		{
			VLong sourceId(inSourceId);
			query->AddCriteria(fFileIDField , DB4D_Equal, sourceId);
			
			CDB4DSelection* results = fFilesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outSourceBaseFolder = GetIntegerField(record, fFileBaseFolderField, NULL);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}


bool VSymbolTable::InnerSymbolTable::GetSourceId(const XBOX::VString& inSourcePath, const ESymbolFileExecContext& inExecutionContext, const ESymbolFileBaseFolder& inBaseFolder, sLONG& outFileId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fFilesTable != NULL && fFilePathField != NULL && fFileExecContextField != NULL && fFileBaseFolderField != NULL && fFileIDField != NULL );
	if( inContext != NULL && fFilesTable != NULL && fFilePathField != NULL && fFileExecContextField != NULL && fFileBaseFolderField != NULL && fFileIDField != NULL )
	{
		CDB4DQuery* query = fFilesTable->NewQuery();
		if( query != NULL )
		{
			query->AddCriteria(fFilePathField , DB4D_Equal, inSourcePath);
			
			query->AddLogicalOperator(DB4D_And);
			VLong executionContext(inExecutionContext);
			query->AddCriteria(fFileExecContextField, DB4D_Equal, executionContext);

			query->AddLogicalOperator(DB4D_And);
			VLong baseFolder(inBaseFolder);
			query->AddCriteria(fFileBaseFolderField, DB4D_Equal, baseFolder);

			CDB4DSelection* results = fFilesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection(inContext);
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outFileId = GetIntegerField(record, fFileIDField, NULL);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}

	return found;
}

bool VSymbolTable::InnerSymbolTable::GetScopeChildren(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const bool& inUseSourceId, const sLONG& inSourceId, std::vector<sLONG>& outScopeChildrenIds, CDB4DBaseContext* inContext) const
{
	bool foundAny = false;
	
	xbox_assert( inContext != NULL && fScopesTable != NULL && fScopeFieldParentId != NULL && fScopeFieldName != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldSourceId != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldParentId != NULL && fScopeFieldName != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldSourceId != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			VLong parentScope(inScopeId);
			query->AddCriteria(fScopeFieldParentId, DB4D_Equal, parentScope);
			
			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fScopeFieldName, DB4D_NotEqual, CVSTR(""));

			query->AddLogicalOperator(DB4D_And);
			VLong executionContext(inExecutionContext);
			query->AddCriteria(fScopeFieldExecutionContext, DB4D_Equal, executionContext);

			if( inUseSourceId == true )
			{
				query->AddLogicalOperator(DB4D_And);
				VLong sourceId(inSourceId);
				query->AddCriteria(fScopeFieldSourceId, DB4D_Equal, sourceId);
			}
			
			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						sLONG childScopeId = GetIntegerField(record, fScopeFieldId, NULL);
						outScopeChildrenIds.push_back(childScopeId);

						foundAny = true;
						
						ReleaseRefCountable(&record);
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return foundAny;
}

bool VSymbolTable::InnerSymbolTable::GetScopeChildren(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inChildName, std::vector<sLONG>& outScopeChildrenIds, CDB4DBaseContext* inContext) const
{
	bool foundAny = false;
	
	xbox_assert( inContext != NULL && fScopesTable != NULL && fScopeFieldParentId != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldName != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldParentId != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldName != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			VLong parentScope(inScopeId);
			query->AddCriteria(fScopeFieldParentId, DB4D_Equal, parentScope);
			
			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fScopeFieldName, DB4D_Equal, inChildName);
			
			query->AddLogicalOperator(DB4D_And);
			VLong executionContext(inExecutionContext);
			query->AddCriteria(fScopeFieldExecutionContext, DB4D_Equal, executionContext);

			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						sLONG childScopeId = GetIntegerField(record, fScopeFieldId, NULL);
						outScopeChildrenIds.push_back(childScopeId);
						
						foundAny = true;
						
						ReleaseRefCountable(&record);
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return foundAny;
}

bool VSymbolTable::InnerSymbolTable::GetScopeChild(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inChildName, sLONG& outScopeChildId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fScopesTable != NULL && fScopeFieldParentId != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldName != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldParentId != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldName != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			VLong parentScope(inScopeId);
			query->AddCriteria(fScopeFieldParentId, DB4D_Equal, parentScope);
			
			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fScopeFieldName, DB4D_Equal, inChildName);
			
			query->AddLogicalOperator(DB4D_And);
			VLong executionContext(inExecutionContext);
			query->AddCriteria(fScopeFieldExecutionContext, DB4D_Equal, executionContext);
			
			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );

				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outScopeChildId = GetIntegerField(record, fScopeFieldId, NULL);
						
						found = true;
						
						ReleaseRefCountable(&record);
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

void VSymbolTable::InnerSymbolTable::_PrepareUpdates(const sLONG& inSourceId, CDB4DTable* inTable, CDB4DField* inSourceField, CDB4DField* inStatusField, CDB4DBaseContext* inContext)
{
	xbox_assert( inContext != NULL && inTable != NULL && inSourceField != NULL && inStatusField != NULL );
	if( inContext != NULL && inTable != NULL && inSourceField != NULL && inStatusField != NULL )
	{
		CDB4DQuery* query = inTable->NewQuery();
		if( query != NULL )
		{
			VLong source(inSourceId);
			query->AddCriteria(inSourceField, DB4D_Equal, source);

			CDB4DSelection* results = inTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Keep_Lock_With_Record, inContext );
					if( record != NULL )
					{
						record->SetLong(inStatusField, eSymbolStatusDeletable);
						record->Save();
						ReleaseRefCountable(&record);
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
}

void VSymbolTable::InnerSymbolTable::_FinalizeUpdates(const sLONG& inSourceId, CDB4DTable* inTable, CDB4DField* inSourceField, CDB4DField* inStatusField, CDB4DBaseContext* inContext)
{
	xbox_assert( inContext != NULL && inTable != NULL && inSourceField != NULL && inStatusField != NULL );
	if( inContext != NULL && inTable != NULL && inSourceField != NULL && inStatusField != NULL )
	{
		CDB4DQuery* query = inTable->NewQuery();
		if( query != NULL )
		{
			VLong source(inSourceId);
			query->AddCriteria(inSourceField, DB4D_Equal, source);
			
			query->AddLogicalOperator(DB4D_And);
			VLong status(eSymbolStatusDeletable);
			query->AddCriteria(inStatusField, DB4D_Equal, status);
			
			CDB4DSelection* results = inTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Keep_Lock_With_Record, inContext );
					if( record != NULL )
					{
						// Delete the current name
						record->Drop();
						ReleaseRefCountable(&record);
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
}

void VSymbolTable::InnerSymbolTable::_DropSymbols(const sLONG& inSourceId, CDB4DTable* inTable, CDB4DField* inSourceField, CDB4DBaseContext* inContext)
{
	xbox_assert( inContext != NULL && inTable != NULL && inSourceField != NULL );
	if( inContext != NULL && inTable != NULL && inSourceField != NULL )
	{
		CDB4DQuery* query = inTable->NewQuery();
		if( query != NULL )
		{
			VLong source(inSourceId);
			query->AddCriteria(inSourceField, DB4D_Equal, source);
			
			CDB4DSelection* results = inTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Keep_Lock_With_Record, inContext );
					if( record != NULL )
					{
						// Delete the current name
						record->Drop();
						ReleaseRefCountable(&record);
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
}

/*
 *	SYMBOL'S NAME MANAGEMENT
 */
bool VSymbolTable::InnerSymbolTable::GetName(const XBOX::VString& inSymbolName, const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, ISymbolUpdateParameters* inUpdateParameters, sLONG& outSymbolNameId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldValue != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldExecutionContext != NULL && fSymbolNameFieldSourceId != NULL );
	if( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldValue != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldExecutionContext != NULL && fSymbolNameFieldId != NULL && fSymbolNameFieldSourceId != NULL )
	{
		CDB4DQuery* query = fSymbolNamesTable->NewQuery();
		if( query != NULL )
		{
			query->AddCriteria(fSymbolNameFieldValue, DB4D_Equal, inSymbolName);
			
			query->AddLogicalOperator(DB4D_And);
			VLong scope(inScopeId);
			query->AddCriteria(fSymbolNameFieldScopeId, DB4D_Equal, scope);
			
			if( inExecutionContext != -1 )
			{
				query->AddLogicalOperator(DB4D_And);
				VLong exeContext(inExecutionContext);
				query->AddCriteria(fSymbolNameFieldExecutionContext, DB4D_Equal, exeContext);
			}

			if( inSourceId != -1 )
			{
				query->AddLogicalOperator(DB4D_And);
				VLong sourceId(inSourceId);
				query->AddCriteria(fSymbolNameFieldSourceId, DB4D_Equal, sourceId);
			}

			CDB4DSelection* results = fSymbolNamesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					if( inUpdateParameters == NULL )
					{
						// As we don't plan to use the record for write operations, we DO NOT LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
						if( record != NULL )
						{
							outSymbolNameId = GetIntegerField(record, fSymbolNameFieldId, NULL);
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
					else
					{
						// As we plan to use the record for write operations, we LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Keep_Lock_With_Record, inContext );
						if( record != NULL )
						{
							outSymbolNameId = GetIntegerField(record, fSymbolNameFieldId, NULL);
							
							if( inUpdateParameters->MustSetStatus() == true )
							{
								ESymbolStatus status;
								inUpdateParameters->GetStatus(status);
								record->SetLong(fSymbolNameFieldStatus,	status);
									
								// Save
								record->Save();
							}
							
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
				}
				
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetNames(const XBOX::VString& inSymbolName, const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, ISymbolUpdateParameters* inUpdateParameters, std::vector<sLONG>& outSymbolNameIds, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldValue != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldExecutionContext != NULL && fSymbolNameFieldSourceId != NULL );
	if( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldValue != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldExecutionContext != NULL && fSymbolNameFieldId != NULL && fSymbolNameFieldSourceId != NULL )
	{
		CDB4DQuery* query = fSymbolNamesTable->NewQuery();
		if( query != NULL )
		{
			query->AddCriteria(fSymbolNameFieldValue, DB4D_Equal, inSymbolName);
			
			query->AddLogicalOperator(DB4D_And);
			VLong scope(inScopeId);
			query->AddCriteria(fSymbolNameFieldScopeId, DB4D_Equal, scope);
			
			if( inExecutionContext != -1 )
			{
				query->AddLogicalOperator(DB4D_And);
				VLong exeContext(inExecutionContext);
				query->AddCriteria(fSymbolNameFieldExecutionContext, DB4D_Equal, exeContext);
			}
			
			if( inSourceId != -1 )
			{
				query->AddLogicalOperator(DB4D_And);
				VLong sourceId(inSourceId);
				query->AddCriteria(fSymbolNameFieldSourceId, DB4D_Equal, sourceId);
			}
			
			CDB4DSelection* results = fSymbolNamesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					if( inUpdateParameters == NULL )
					{
						// As we don't plan to use the record for write operations, we DO NOT LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
						if( record != NULL )
						{
							sLONG nameId = GetIntegerField(record, fSymbolNameFieldId, NULL);
							
							VIndex namesCount = outSymbolNameIds.size();
							if( namesCount == 0 || (namesCount >= 1 && outSymbolNameIds[namesCount-1] != nameId) )
								outSymbolNameIds.push_back(nameId);
							
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
					else
					{
						// As we plan to use the record for write operations, we LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Keep_Lock_With_Record, inContext );
						if( record != NULL )
						{
							sLONG nameId = GetIntegerField(record, fSymbolNameFieldId, NULL);
							
							VIndex namesCount = outSymbolNameIds.size();
							if( namesCount == 0 || (namesCount >= 1 && outSymbolNameIds[namesCount-1] != nameId) )
								outSymbolNameIds.push_back(nameId);
							
							if( inUpdateParameters->MustSetStatus() == true )
							{
								ESymbolStatus status;
								inUpdateParameters->GetStatus(status);
								record->SetLong(fSymbolNameFieldStatus,	status);
								
								// Save
								record->Save();
							}
							
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
				}
				
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetName(const sLONG& inNameId, XBOX::VString& outName, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldId != NULL && fSymbolNameFieldValue != NULL );
	if( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldId != NULL && fSymbolNameFieldValue != NULL )
	{
		CDB4DQuery* query = fSymbolNamesTable->NewQuery();
		if( query != NULL )
		{
			VLong nameId(inNameId);
			query->AddCriteria(fSymbolNameFieldId, DB4D_Equal, nameId);
			
			CDB4DSelection* results = fSymbolNamesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						record->GetString(fSymbolNameFieldValue, outName);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetName(const sLONG& inNameId, const XBOX::VString& inSymbolStartsWith, XBOX::VString& outName, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldId != NULL && fSymbolNameFieldValue != NULL );
	if( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldId != NULL && fSymbolNameFieldValue != NULL )
	{
		CDB4DQuery* query = fSymbolNamesTable->NewQuery();
		if( query != NULL )
		{
			VLong nameId(inNameId);
			query->AddCriteria(fSymbolNameFieldId, DB4D_Equal, nameId);
			
			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fSymbolNameFieldValue, DB4D_BeginsWith, inSymbolStartsWith);
			
			CDB4DSelection* results = fSymbolNamesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						record->GetString(fSymbolNameFieldValue, outName);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetNameScopeId(const sLONG& inNameId, sLONG& outScopeId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldId != NULL && fSymbolNameFieldScopeId != NULL );
	if( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldId != NULL && fSymbolNameFieldScopeId != NULL )
	{
		CDB4DQuery* query = fSymbolNamesTable->NewQuery();
		if( query != NULL )
		{
			VLong nameId(inNameId);
			query->AddCriteria(fSymbolNameFieldId, DB4D_Equal, nameId);
			
			CDB4DSelection* results = fSymbolNamesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outScopeId = GetIntegerField(record, fSymbolNameFieldScopeId, NULL);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetNameExecutionContext(const sLONG& inNameId, ESymbolFileExecContext& outExecutionContext, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldId != NULL && fSymbolNameFieldExecutionContext != NULL );
	if( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldId != NULL && fSymbolNameFieldExecutionContext != NULL )
	{
		CDB4DQuery* query = fSymbolNamesTable->NewQuery();
		if( query != NULL )
		{
			VLong nameId(inNameId);
			query->AddCriteria(fSymbolNameFieldId, DB4D_Equal, nameId);
			
			CDB4DSelection* results = fSymbolNamesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outExecutionContext = GetIntegerField(record, fSymbolNameFieldExecutionContext, NULL);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetNames(const XBOX::VString& inStartWith, const sLONG& inScopeId, std::vector<sLONG>& outNameIds, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldValue != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldId != NULL );
	if( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldValue != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldId != NULL )
	{
		CDB4DQuery* query = fSymbolNamesTable->NewQuery();
		if( query != NULL )
		{
			query->AddCriteria(fSymbolNameFieldValue, DB4D_BeginsWith, inStartWith);

			query->AddLogicalOperator(DB4D_And);
			VLong scope(inScopeId);
			query->AddCriteria(fSymbolNameFieldScopeId, DB4D_Equal, scope);

			CDB4DSelection* results = fSymbolNamesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				results->SortSelection(fSymbolNameFieldValue->GetID(), true);
				
				RecIDType total = results->CountRecordsInSelection( inContext );

				for( RecIDType index=1; index<=total; ++index )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outNameIds.push_back(GetIntegerField(record, fSymbolNameFieldId, NULL));
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetNameFromScope(const XBOX::VString& inSymbolName, const sLONG& inFromScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, sLONG& outNameId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldValue != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldExecutionContext != NULL && fSymbolNameFieldSourceId != NULL );
	if( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldValue != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldExecutionContext != NULL && fSymbolNameFieldId != NULL && fSymbolNameFieldSourceId != NULL )
	{
		CDB4DQuery* query = fSymbolNamesTable->NewQuery();
		if( query != NULL )
		{
			query->AddCriteria(fSymbolNameFieldValue, DB4D_Equal, inSymbolName);
			
			query->AddLogicalOperator(DB4D_And);
			VLong scope(inFromScopeId);
			query->AddCriteria(fSymbolNameFieldScopeId, DB4D_Equal, scope);
			
			query->AddLogicalOperator(DB4D_And);
			VLong exeContext(inExecutionContext);
			query->AddCriteria(fSymbolNameFieldExecutionContext, DB4D_Equal, exeContext);

			query->AddLogicalOperator(DB4D_And);
			VLong sourceId(inSourceId);
			query->AddCriteria(fSymbolNameFieldSourceId, DB4D_Equal, sourceId);
			
			CDB4DSelection* results = fSymbolNamesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outNameId = GetIntegerField(record, fSymbolNameFieldId, NULL);

						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	if( found == false )
	{
		sLONG nextParentScopeId;
		if( GetScopeParentId(inFromScopeId, nextParentScopeId, inContext) == true )
			found = GetNameFromScope(inSymbolName, nextParentScopeId, inExecutionContext, inSourceId, outNameId, inContext);
	}
	
	if( found == true )
	{
		sLONG definitionId;
		
		bool foundLocalVariable = GetDefinition(inFromScopeId, outNameId, inSourceId, eSymbolDefinitionKindLocalVariable, -1, NULL, definitionId, inContext);
		bool foundVariable = GetDefinition(inFromScopeId, outNameId, inSourceId, eSymbolDefinitionKindVariable, -1, NULL, definitionId, inContext);
		
		if( foundLocalVariable == false && foundVariable == false )
		{
			sLONG nextParentScopeId;
			if( GetScopeParentId(inFromScopeId, nextParentScopeId, inContext) == true )
				found = GetNameFromScope(inSymbolName, nextParentScopeId, inExecutionContext, inSourceId, outNameId, inContext);
			else
				found = false;
		}
		else
		{
			ESymbolStatus status;
			if( GetDefinitionStatus(definitionId, status, inContext) == true )
			{
				if( status == eSymbolStatusUpToDate )
					found = true;
				else
				{
					sLONG nextParentScopeId;
					if( GetScopeParentId(inFromScopeId, nextParentScopeId, inContext) == true )
						found = GetNameFromScope(inSymbolName, nextParentScopeId, inExecutionContext, inSourceId, outNameId, inContext);
					else
						found = false;
				}
			}
		}
	}
	
	return found;
}

void VSymbolTable::InnerSymbolTable::SetName(const XBOX::VString& inSymbolName, const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, sLONG& outSymbolNameId, CDB4DBaseContext* inContext)
{
	xbox_assert( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldValue != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldExecutionContext != NULL && fSymbolNameFieldId != NULL && fSymbolNameFieldStatus != NULL && fSymbolNameFieldSourceId != NULL );
	if( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldValue != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldExecutionContext != NULL && fSymbolNameFieldId != NULL && fSymbolNameFieldStatus != NULL && fSymbolNameFieldSourceId != NULL )
	{
		CDB4DRecord* record = fSymbolNamesTable->NewRecord(inContext);
		if( record != NULL )
		{
			// Set field values
			record->SetString(	fSymbolNameFieldValue,				inSymbolName);
			record->SetLong(	fSymbolNameFieldScopeId,			inScopeId);
			record->SetLong(	fSymbolNameFieldExecutionContext,	inExecutionContext);
			record->SetLong(	fSymbolNameFieldStatus,				eSymbolStatusUpToDate);
			record->SetLong(	fSymbolNameFieldSourceId,			inSourceId);
			
			// Save
			record->Save();
			
			// Get scope id
			outSymbolNameId = GetIntegerField(record, fSymbolNameFieldId, NULL);
			
			ReleaseRefCountable(&record);
		}
	}
}

/*
 *	SYMBOL'S DEFINITION MANAGEMENT
 */
bool VSymbolTable::InnerSymbolTable::GetDefinition(const sLONG& inScopeId, const sLONG& inNameId, const sLONG& inSourceId, const ESymbolDefinitionKind& inKind, const sLONG& inLineNumber, ISymbolUpdateParameters* inUpdateParameters, sLONG& outDefinitionId, CDB4DBaseContext* inContext) const
{
	bool found = false;

	xbox_assert( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldScopeId != NULL && fSymbolDefinitionFieldNameId != NULL && fSymbolDefinitionFieldSourceId != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldKind != NULL );
	if( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldScopeId != NULL && fSymbolDefinitionFieldNameId != NULL && fSymbolDefinitionFieldSourceId != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldKind != NULL )
	{
		CDB4DQuery* query = fSymbolDefinitionsTable->NewQuery();
		if( query != NULL )
		{
			VLong scopeId(inScopeId);
			query->AddCriteria(fSymbolDefinitionFieldScopeId, DB4D_Equal, scopeId);
			
			query->AddLogicalOperator(DB4D_And);
			VLong nameId(inNameId);
			query->AddCriteria(fSymbolDefinitionFieldNameId, DB4D_Equal, nameId);
			
			if( inSourceId != -1 )
			{
				query->AddLogicalOperator(DB4D_And);
				VLong sourceId(inSourceId);
				query->AddCriteria(fSymbolDefinitionFieldSourceId, DB4D_Equal, sourceId);
			}

			if( inKind != eSymbolDefinitionKindAll )
			{
				query->AddLogicalOperator(DB4D_And);
				VLong kind(inKind);
				query->AddCriteria(fSymbolDefinitionFieldKind, DB4D_Equal, kind);
			}

			if( inLineNumber != -1 )
			{
				query->AddLogicalOperator(DB4D_And);
				VLong lineNumber(inLineNumber);
				query->AddCriteria(fSymbolDefinitionFieldLineNumber, DB4D_Equal, lineNumber);
			}
			
			CDB4DSelection* results = fSymbolDefinitionsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					if( inUpdateParameters == NULL )
					{
						// As we don't plan to use the record for write operations, we DO NOT LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
						if( record != NULL )
						{
							outDefinitionId = GetIntegerField(record, fSymbolDefinitionFieldId, NULL);
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
					else
					{
						// As we plan to use the record for write operations, we LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Keep_Lock_With_Record, inContext );
						if( record != NULL )
						{
							outDefinitionId = GetIntegerField(record, fSymbolDefinitionFieldId, NULL);
							
							bool anyFieldHasBeenUpdated = false;
							
							if( inUpdateParameters->MustSetStatus() == true )
							{
								ESymbolStatus status;
								inUpdateParameters->GetStatus(status);
								record->SetLong(fSymbolDefinitionFieldStatus,	status);
								
								anyFieldHasBeenUpdated = true;
							}
							
							if( inUpdateParameters->MustSetBoundaries() == true )
							{
								sLONG fromLine, fromOffset, toLine, toOffset;
								inUpdateParameters->GetBoundaries(fromLine, fromOffset, toLine, toOffset);
								
								if( fromLine != -1 )
								{
									record->SetLong(fSymbolDefinitionFieldLineNumber, fromLine);
									anyFieldHasBeenUpdated = true;
								}
							}
							
							if( anyFieldHasBeenUpdated == true )
							{
								// Save
								record->Save();
							}
							
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetDefinitionId(const sLONG& inScopeId, const sLONG& inNameId, const sLONG& inSourceId, sLONG& outDefinitionId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldScopeId != NULL && fSymbolDefinitionFieldNameId != NULL && fSymbolDefinitionFieldSourceId != NULL );
	if( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldScopeId != NULL && fSymbolDefinitionFieldNameId != NULL && fSymbolDefinitionFieldSourceId != NULL )
	{
		CDB4DQuery* query = fSymbolDefinitionsTable->NewQuery();
		if( query != NULL )
		{
			VLong scopeId(inScopeId);
			query->AddCriteria(fSymbolDefinitionFieldScopeId, DB4D_Equal, scopeId);

			query->AddLogicalOperator(DB4D_And);
			VLong nameId(inNameId);
			query->AddCriteria(fSymbolDefinitionFieldNameId, DB4D_Equal, nameId);

			if( inSourceId != -1 )
			{
				query->AddLogicalOperator(DB4D_And);
				VLong sourceId(inSourceId);
				query->AddCriteria(fSymbolDefinitionFieldSourceId, DB4D_Equal, sourceId);
			}

			CDB4DSelection* results = fSymbolDefinitionsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outDefinitionId = GetIntegerField(record, fSymbolDefinitionFieldId, NULL);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetDefinitionName(const sLONG& inDefinitionId, XBOX::VString& outName, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldNameId != NULL );
	if( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldNameId != NULL )
	{
		CDB4DQuery* query = fSymbolDefinitionsTable->NewQuery();
		if( query != NULL )
		{
			VLong definitionId(inDefinitionId);
			query->AddCriteria(fSymbolDefinitionFieldId, DB4D_Equal, definitionId);
			
			CDB4DSelection* results = fSymbolDefinitionsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						sLONG nameId = GetIntegerField(record, fSymbolDefinitionFieldNameId, NULL);
						
						ReleaseRefCountable(&record);
						
						found = GetName(nameId, outName, inContext);
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetDefinitionNameId(const sLONG& inDefinitionId, sLONG& outNameId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldNameId != NULL );
	if( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldNameId != NULL )
	{
		CDB4DQuery* query = fSymbolDefinitionsTable->NewQuery();
		if( query != NULL )
		{
			VLong definitionId(inDefinitionId);
			query->AddCriteria(fSymbolDefinitionFieldId, DB4D_Equal, definitionId);
			
			CDB4DSelection* results = fSymbolDefinitionsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outNameId = GetIntegerField(record, fSymbolDefinitionFieldNameId, NULL);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetDefinitionKind(const sLONG& inDefinitionId, ESymbolDefinitionKind& outKind, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldKind != NULL );
	if( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldKind != NULL )
	{
		CDB4DQuery* query = fSymbolDefinitionsTable->NewQuery();
		if( query != NULL )
		{
			VLong definitionId(inDefinitionId);
			query->AddCriteria(fSymbolDefinitionFieldId, DB4D_Equal, definitionId);
			
			CDB4DSelection* results = fSymbolDefinitionsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outKind = GetIntegerField(record, fSymbolDefinitionFieldKind, NULL);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetDefinitionParentScopeId(const sLONG& inDefinitionId, sLONG& outParentScopeId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldScopeId != NULL );
	if( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldScopeId != NULL )
	{
		CDB4DQuery* query = fSymbolDefinitionsTable->NewQuery();
		if( query != NULL )
		{
			VLong definitionId(inDefinitionId);
			query->AddCriteria(fSymbolDefinitionFieldId, DB4D_Equal, definitionId);
			
			CDB4DSelection* results = fSymbolDefinitionsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outParentScopeId = GetIntegerField(record, fSymbolDefinitionFieldScopeId, NULL);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetDefinitionStatus(const sLONG& inDefinitionId, ESymbolStatus& outStatus, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldStatus != NULL );
	if( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldStatus != NULL )
	{
		CDB4DQuery* query = fSymbolDefinitionsTable->NewQuery();
		if( query != NULL )
		{
			VLong definitionId(inDefinitionId);
			query->AddCriteria(fSymbolDefinitionFieldId, DB4D_Equal, definitionId);
			
			CDB4DSelection* results = fSymbolDefinitionsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outStatus = GetIntegerField(record, fSymbolDefinitionFieldStatus, NULL);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetDefinitionSourceId(const sLONG& inDefinitionId, sLONG& outSourceId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldSourceId != NULL );
	if( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldSourceId != NULL )
	{
		CDB4DQuery* query = fSymbolDefinitionsTable->NewQuery();
		if( query != NULL )
		{
			VLong definitionId(inDefinitionId);
			query->AddCriteria(fSymbolDefinitionFieldId, DB4D_Equal, definitionId);
			
			CDB4DSelection* results = fSymbolDefinitionsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outSourceId = GetIntegerField(record, fSymbolDefinitionFieldSourceId, NULL);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetDefinitionLine(const sLONG& inDefinitionId, sLONG& outLine, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldLineNumber != NULL );
	if( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldLineNumber != NULL )
	{
		CDB4DQuery* query = fSymbolDefinitionsTable->NewQuery();
		if( query != NULL )
		{
			VLong definitionId(inDefinitionId);
			query->AddCriteria(fSymbolDefinitionFieldId, DB4D_Equal, definitionId);
			
			CDB4DSelection* results = fSymbolDefinitionsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outLine = GetIntegerField(record, fSymbolDefinitionFieldLineNumber, NULL);
						
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

void VSymbolTable::InnerSymbolTable::SetDefinition(const sLONG& inSourceId, const sLONG& inScopeId, const sLONG& inNameId, const ESymbolFileExecContext& inExecutionContext, const ESymbolDefinitionKind& inKind, const sLONG& inLineNumber, sLONG& outDefinitionId, CDB4DBaseContext* inContext)
{
	xbox_assert( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldScopeId != NULL && fSymbolDefinitionFieldNameId != NULL && fSymbolDefinitionFieldKind != NULL && fSymbolDefinitionFieldExecutionContext != NULL && fSymbolDefinitionFieldStatus != NULL && fSymbolDefinitionFieldSourceId != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldLineNumber != NULL );
	if( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldScopeId != NULL && fSymbolDefinitionFieldNameId != NULL && fSymbolDefinitionFieldKind != NULL && fSymbolDefinitionFieldExecutionContext != NULL && fSymbolDefinitionFieldStatus != NULL && fSymbolDefinitionFieldSourceId != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldLineNumber != NULL )
	{
		CDB4DRecord* record = fSymbolDefinitionsTable->NewRecord(inContext);
		if( record != NULL )
		{
			// Set field values
			record->SetLong(	fSymbolDefinitionFieldScopeId,				inScopeId);
			record->SetLong(	fSymbolDefinitionFieldNameId,				inNameId);
			record->SetLong(	fSymbolDefinitionFieldKind,					inKind);
			record->SetLong(	fSymbolDefinitionFieldExecutionContext,		inExecutionContext);
			record->SetLong(	fSymbolDefinitionFieldStatus,				eSymbolStatusUpToDate);
			record->SetLong(	fSymbolDefinitionFieldSourceId,				inSourceId);
			record->SetLong(	fSymbolDefinitionFieldLineNumber,			inLineNumber);

			outDefinitionId = GetIntegerField(record, fSymbolDefinitionFieldId, NULL);
			// Save
			record->Save();
			
			ReleaseRefCountable(&record);
		}
	}
}


/*
 *	SYMBOL'S SUGGESTION MANAGEMENT
 */
void VSymbolTable::InnerSymbolTable::GetConstructorNames(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inStartWith, const sLONG& inFromScope, const sLONG& inToScope, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds, CDB4DBaseContext* inContext) const
{
	xbox_assert( inContext != NULL && fScopesTable && fScopeFieldExecutionContext != NULL && fScopeFieldConstructorFlag != NULL && fScopeFieldName != NULL );
	if( inContext != NULL && fScopesTable != NULL && fScopeFieldExecutionContext != NULL && fScopeFieldConstructorFlag != NULL && fScopeFieldName != NULL )
	{
		CDB4DQuery* query = fScopesTable->NewQuery();
		if( query != NULL )
		{
			VLong exeContext(inExecutionContext);
			query->AddCriteria(fScopeFieldExecutionContext, DB4D_Equal, exeContext);

			query->AddLogicalOperator(DB4D_And);
			VLong scopeId(inFromScope);
			query->AddCriteria(fScopeFieldParentId, DB4D_Equal, scopeId);

			query->AddLogicalOperator(DB4D_And);
			VBoolean flag(true);
			query->AddCriteria(fScopeFieldConstructorFlag, DB4D_Equal, flag);

			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fScopeFieldName, DB4D_BeginsWith, inStartWith);

			CDB4DSelection* results = fScopesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				results->SortSelection(fScopeFieldName->GetID(), true);
				
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						VString name = GetStringField(record, fScopeFieldName);
						
						VIndex namesCount = outNames.size();
						if( namesCount == 0 || (namesCount >= 1 && outNames[namesCount-1] != name) )
						{
							outNames.push_back(name);
							outNameScopeIds.push_back(inFromScope);
						}
						
						ReleaseRefCountable(&record);
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	if( inFromScope != inToScope )
	{
		sLONG nextScope;
		if( GetScopeParentId(inFromScope, nextScope, inContext) == true )
			GetConstructorNames(inExecutionContext, inStartWith, nextScope, inToScope, outNames, outNameScopeIds, inContext);
	}
}

void VSymbolTable::InnerSymbolTable::GetWidgetNames(const sLONG& inHtmlSourceId, const XBOX::VString& inStartWith, std::vector<XBOX::VString>& outNames, CDB4DBaseContext* inContext) const
{
	sLONG globalScopeId;
	if( GetGlobalScope(eSymbolFileExecContextClient, NULL, globalScopeId, inContext) == true )
	{
		sLONG wafId;
		if( GetScope(CVSTR("WAF"), eSymbolScopeTypeObject, globalScopeId, inHtmlSourceId, eSymbolFileExecContextClient, NULL, wafId, inContext) == true )
		{
			sLONG widgetsParentId;
			if( GetScope(CVSTR("widgets"), eSymbolScopeTypeObject, wafId, inHtmlSourceId, eSymbolFileExecContextClient, NULL, widgetsParentId, inContext) == true )
			{
				std::vector<sLONG> widgetsIds;
				if( GetScopeChildren(widgetsParentId, eSymbolFileExecContextClient, true, inHtmlSourceId, widgetsIds, inContext) == true )
				{
					VIndex total = widgetsIds.size();
					for(VIndex index=0; index<total; ++index)
					{
						VString name;
						if( GetScopeName(widgetsIds[index], name, inContext) == true )
						{
							bool match = false;
							if( (inStartWith.GetLength() > 0 && name.BeginsWith(inStartWith)) || (inStartWith.GetLength() == 0) )
								match = true;

							if( match == true )
								outNames.push_back(name);
						}
					}
					sort(outNames.begin(), outNames.end());
					unique(outNames.begin(), outNames.end());
				}
			}
		}
	}
}

void VSymbolTable::InnerSymbolTable::GetModelEntityPropertyNames(const XBOX::VString& inModelName, const XBOX::VString& inModelEntityName, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds, CDB4DBaseContext* inContext) const
{
	xbox_assert( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldValue != NULL && fSymbolNameFieldScopeId != NULL );
	if( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldValue != NULL && fSymbolNameFieldScopeId != NULL )
	{
		sLONG globalScopeId;
		if( GetGlobalScope(eSymbolFileExecContextServer, NULL, globalScopeId, inContext) == true )
		{
			// Get all "model" scope definitions
			std::vector<sLONG> modelScopeIds;
			if( GetScopes(inModelName, eSymbolFileExecContextServer, globalScopeId, globalScopeId, modelScopeIds, inContext) == true )
			{
				std::vector<sLONG> entityPropertyScopeIds;
				
				// For each "model" definition, get all "entity" scope definition
				for(std::vector<sLONG>::const_iterator it=modelScopeIds.begin(); it!=modelScopeIds.end(); ++it)
				{
					sLONG modelId = (*it);
					GetScopeChildren(modelId, eSymbolFileExecContextServer, inModelEntityName, entityPropertyScopeIds, inContext);
				}
				
				// For each "entity" definition, get all "entityMethods" scope definition
				VIndex entityPropertyScopeCount = entityPropertyScopeIds.size();
				for(VIndex index=0; index<entityPropertyScopeCount; ++index)
				{
					sLONG entityId = entityPropertyScopeIds[index];
					GetScopeChildren(entityId, eSymbolFileExecContextServer, CVSTR("entityMethods"), entityPropertyScopeIds, inContext);
				}
				
				// Get all names within entityPropertyScopeIds
				CDB4DQuery* query = fSymbolNamesTable->NewQuery();
				if( query != NULL )
				{
					entityPropertyScopeCount = entityPropertyScopeIds.size();
					for(VIndex index=0; index<entityPropertyScopeCount; ++index)
					{
						if( index != 0 )
							query->AddLogicalOperator(DB4D_OR);
						
						VLong entityId(entityPropertyScopeIds[index]);
						query->AddCriteria(fSymbolNameFieldScopeId, DB4D_Equal, entityId);
					}
					
					CDB4DSelection* results = fSymbolNamesTable->ExecuteQuery(query, inContext);
					if( results != NULL )
					{
						results->SortSelection(fSymbolNameFieldValue->GetID(), true);
						
						RecIDType total = results->CountRecordsInSelection( inContext );
						for( RecIDType index=1; index<=total; ++index )
						{
							// As we don't plan to use the record for write operations, we DO NOT LOCK it !
							CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
							if( record != NULL )
							{
								VString name = GetStringField(record, fSymbolNameFieldValue);
								if( name != CVSTR("entityMethods") )
								{
									VIndex namesCount = outNames.size();
									if( namesCount == 0 || (namesCount >= 1 && outNames[namesCount-1] != name) )
									{
										outNames.push_back(name);
										
										sLONG nameScopeId = GetIntegerField(record, fSymbolNameFieldScopeId, NULL);
										VString scopeName;
										if( GetScopeName(nameScopeId, scopeName, inContext) == true )
										{
											if( scopeName == CVSTR("entityMethods") )
												GetScopeParentId(nameScopeId, nameScopeId, inContext);
										}
										outNameScopeIds.push_back(nameScopeId);
									}
								}
								
								ReleaseRefCountable(&record);
							}
						}

						ReleaseRefCountable(&results);
					}
					ReleaseRefCountable(&query);
				}
			}
		}
	}
}

void VSymbolTable::InnerSymbolTable::GetNames(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inStartWith, const sLONG& inFromScope, const sLONG& inToScope, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds, CDB4DBaseContext* inContext) const
{
	xbox_assert( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldExecutionContext != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldValue != NULL );
	if( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldExecutionContext != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldValue != NULL )
	{
		CDB4DQuery* query = fSymbolNamesTable->NewQuery();
		if( query != NULL )
		{
			VLong exeContext(inExecutionContext);
			query->AddCriteria(fSymbolNameFieldExecutionContext, DB4D_Equal, exeContext);

			query->AddLogicalOperator(DB4D_And);
			VLong scopeId(inFromScope);
			query->AddCriteria(fSymbolNameFieldScopeId, DB4D_Equal, scopeId);
			
			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fSymbolNameFieldValue, DB4D_BeginsWith, inStartWith);
			
			CDB4DSelection* results = fSymbolNamesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				results->SortSelection(fSymbolNameFieldValue->GetID(), true);
				
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						VString name = GetStringField(record, fSymbolNameFieldValue);
						VIndex namesCount = outNames.size();
						if( namesCount == 0 || (namesCount >= 1 && outNames[namesCount-1] != name) )
						{
							outNames.push_back(name);
							outNameScopeIds.push_back(inFromScope);
						}
						
						ReleaseRefCountable(&record);
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	if( inFromScope != inToScope )
	{
		sLONG nextScope;
		if( GetScopeParentId(inFromScope, nextScope, inContext) == true )
			GetNames(inExecutionContext, inStartWith, nextScope, inToScope, outNames, outNameScopeIds, inContext);
	}
}

void VSymbolTable::InnerSymbolTable::GetNames(const XBOX::VString& inStartWith, const std::vector<sLONG>& inScopeIds, std::vector<XBOX::VString>& outNames, std::vector<sLONG>& outNameScopeIds, CDB4DBaseContext* inContext) const
{
	xbox_assert( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldValue != NULL );
	if( inContext != NULL && fSymbolNamesTable != NULL && fSymbolNameFieldScopeId != NULL && fSymbolNameFieldValue != NULL )
	{
		CDB4DQuery* query = fSymbolNamesTable->NewQuery();
		if( query != NULL )
		{
			VIndex scopesCount = inScopeIds.size();
			for(VIndex scopeIndex=0; scopeIndex<scopesCount; ++scopeIndex)
			{
				if( scopeIndex > 0 )
					query->AddLogicalOperator(DB4D_OR);
				
				VLong scopeId(inScopeIds[scopeIndex]);
				query->AddCriteria(fSymbolNameFieldScopeId, DB4D_Equal, scopeId);
			}
			
			CDB4DSelection* results = fSymbolNamesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				results->SortSelection(fSymbolNameFieldValue->GetID(), true);
				
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						VString name = GetStringField(record, fSymbolNameFieldValue);
						if( name.BeginsWith(inStartWith) == true )
						{
							VIndex namesCount = outNames.size();
							if( namesCount == 0 || (namesCount >= 1 && outNames[namesCount-1] != name) )
							{
								outNames.push_back(name);
								
								sLONG fromScope = GetIntegerField(record, fSymbolNameFieldScopeId, NULL);
								outNameScopeIds.push_back(fromScope);
							}
						}
						ReleaseRefCountable(&record);
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
}

/*
 *	SYMBOL'S PROTOTYPE CHAIN MANAGEMENT
 */
bool VSymbolTable::InnerSymbolTable::GetPrototypeChain(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inFromName, const XBOX::VString& inToName, ISymbolUpdateParameters* inUpdateParameters, sLONG& outPrototypeId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolPrototypesTable != NULL && fSymbolPrototypeFieldExecutionContext != NULL && fSymbolPrototypeFieldFromName != NULL && fSymbolPrototypeFieldToName != NULL && fSymbolPrototypeFieldId != NULL );
	if( inContext != NULL && fSymbolPrototypesTable != NULL && fSymbolPrototypeFieldExecutionContext != NULL && fSymbolPrototypeFieldFromName != NULL && fSymbolPrototypeFieldToName != NULL && fSymbolPrototypeFieldId != NULL )
	{
		CDB4DQuery* query = fSymbolPrototypesTable->NewQuery();
		if( query != NULL )
		{
			VLong executionContext(inExecutionContext);
			query->AddCriteria(fSymbolPrototypeFieldExecutionContext, DB4D_Equal, executionContext);
			
			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fSymbolPrototypeFieldFromName, DB4D_Equal, inFromName);

			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fSymbolPrototypeFieldToName, DB4D_Equal, inToName);

			CDB4DSelection* results = fSymbolPrototypesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					if( inUpdateParameters == NULL )
					{
						// As we don't plan to use the record for write operations, we DO NOT LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
						if( record != NULL )
						{
							outPrototypeId = GetIntegerField(record, fSymbolPrototypeFieldId, NULL);
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
					else
					{
						// As we plan to use the record for write operations, we LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Keep_Lock_With_Record, inContext );
						if( record != NULL )
						{
							outPrototypeId = GetIntegerField(record, fSymbolPrototypeFieldId, NULL);
							
							if( inUpdateParameters->MustSetStatus() == true )
							{
								ESymbolStatus status;
								inUpdateParameters->GetStatus(status);
								record->SetLong(fSymbolPrototypeFieldStatus,	status);
								
								// Save
								record->Save();
							}
							
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}
bool VSymbolTable::InnerSymbolTable::GetPrototypeChain(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inFromName, std::vector<XBOX::VString>& outPrototypeNames, CDB4DBaseContext* inContext) const
{
	bool found = false;
	VString toName;
	
	xbox_assert( inContext != NULL && fSymbolPrototypesTable != NULL && fSymbolPrototypeFieldExecutionContext != NULL && fSymbolPrototypeFieldFromName != NULL && fSymbolPrototypeFieldToName != NULL );
	if( inContext != NULL && fSymbolPrototypesTable != NULL && fSymbolPrototypeFieldExecutionContext != NULL && fSymbolPrototypeFieldFromName != NULL && fSymbolPrototypeFieldToName != NULL )
	{
		CDB4DQuery* query = fSymbolPrototypesTable->NewQuery();
		if( query != NULL )
		{
			VLong executionContext(inExecutionContext);
			query->AddCriteria(fSymbolPrototypeFieldExecutionContext, DB4D_Equal, executionContext);
			
			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fSymbolPrototypeFieldFromName, DB4D_Equal, inFromName);
			
			CDB4DSelection* results = fSymbolPrototypesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						toName = GetStringField(record, fSymbolPrototypeFieldToName);
						if( toName != CVSTR("") )
						{
							outPrototypeNames.push_back(toName);
							found = true;
						}
						
						ReleaseRefCountable(&record);
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	if( found == true )
		GetPrototypeChain(inExecutionContext, toName, outPrototypeNames, inContext);
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetPrototypeChainParentName(const ESymbolFileExecContext& inExecutionContext, const XBOX::VString& inFromName, XBOX::VString& outParentPrototypeName, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolPrototypesTable != NULL && fSymbolPrototypeFieldExecutionContext != NULL && fSymbolPrototypeFieldFromName != NULL && fSymbolPrototypeFieldToName != NULL );
	if( inContext != NULL && fSymbolPrototypesTable != NULL && fSymbolPrototypeFieldExecutionContext != NULL && fSymbolPrototypeFieldFromName != NULL && fSymbolPrototypeFieldToName != NULL )
	{
		CDB4DQuery* query = fSymbolPrototypesTable->NewQuery();
		if( query != NULL )
		{
			VLong executionContext(inExecutionContext);
			query->AddCriteria(fSymbolPrototypeFieldExecutionContext, DB4D_Equal, executionContext);
			
			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fSymbolPrototypeFieldFromName, DB4D_Equal, inFromName);
			
			CDB4DSelection* results = fSymbolPrototypesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outParentPrototypeName = GetStringField(record, fSymbolPrototypeFieldToName);

						found = true;
						
						ReleaseRefCountable(&record);
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

void VSymbolTable::InnerSymbolTable::SetPrototypeChain(const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, const XBOX::VString& inFromName, const XBOX::VString& inToName, CDB4DBaseContext* inContext)
{
	xbox_assert( inContext != NULL && fSymbolPrototypesTable != NULL && fSymbolPrototypeFieldExecutionContext != NULL && fSymbolPrototypeFieldFromName != NULL && fSymbolPrototypeFieldToName != NULL && fSymbolPrototypeFieldStatus != NULL && fSymbolPrototypeFieldSourceId != NULL );
	if( inContext != NULL && fSymbolPrototypesTable != NULL && fSymbolPrototypeFieldExecutionContext != NULL && fSymbolPrototypeFieldFromName != NULL && fSymbolPrototypeFieldToName != NULL && fSymbolPrototypeFieldStatus != NULL && fSymbolPrototypeFieldSourceId != NULL )
	{
		CDB4DRecord* record = fSymbolPrototypesTable->NewRecord(inContext);
		if( record != NULL )
		{
			// Set field values
			record->SetLong(	fSymbolPrototypeFieldExecutionContext,	inExecutionContext);
			record->SetString(	fSymbolPrototypeFieldFromName,			inFromName);
			record->SetString(	fSymbolPrototypeFieldToName,			inToName);
			record->SetLong(	fSymbolPrototypeFieldStatus,			eSymbolStatusUpToDate);
			record->SetLong(	fSymbolPrototypeFieldSourceId,			inSourceId);
			
			// Save
			record->Save();
			
			ReleaseRefCountable(&record);
		}
	}
}

/*
 *	SYMBOL'S RETURNS CHAIN MANAGEMENT
 */
bool VSymbolTable::InnerSymbolTable::GetReturnType(const sLONG& inScopeId, const XBOX::VString& inReturnTypeName, ISymbolUpdateParameters* inUpdateParameters, sLONG& outReturnTypeId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolReturnsTable != NULL && fSymbolReturnFieldScopeId != NULL && fSymbolReturnFieldName != NULL );
	if( inContext != NULL && fSymbolReturnsTable != NULL && fSymbolReturnFieldScopeId != NULL && fSymbolReturnFieldName != NULL )
	{
		CDB4DQuery* query = fSymbolReturnsTable->NewQuery();
		if( query != NULL )
		{
			VLong scopeId(inScopeId);
			query->AddCriteria(fSymbolReturnFieldScopeId, DB4D_Equal, scopeId);

			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fSymbolReturnFieldName, DB4D_Equal, inReturnTypeName);

			CDB4DSelection* results = fSymbolReturnsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );

				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					if( inUpdateParameters == NULL )
					{
						// As we don't plan to use the record for write operations, we DO NOT LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
						if( record != NULL )
						{
							outReturnTypeId = GetIntegerField(record, fSymbolReturnFieldId, NULL);
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
					else
					{
						// As we plan to use the record for write operations, we LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Keep_Lock_With_Record, inContext );
						if( record != NULL )
						{
							outReturnTypeId = GetIntegerField(record, fSymbolReturnFieldId, NULL);
							
							if( inUpdateParameters->MustSetStatus() == true )
							{
								ESymbolStatus status;
								inUpdateParameters->GetStatus(status);
								record->SetLong(fSymbolReturnFieldStatus,	status);
								
								// Save
								record->Save();
							}
							
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetReturnTypeNames(const sLONG& inScopeId, std::vector<XBOX::VString>& outNames, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolReturnsTable != NULL && fSymbolReturnFieldScopeId != NULL && fSymbolReturnFieldName != NULL );
	if( inContext != NULL && fSymbolReturnsTable != NULL && fSymbolReturnFieldScopeId != NULL && fSymbolReturnFieldName != NULL )
	{
		CDB4DQuery* query = fSymbolReturnsTable->NewQuery();
		if( query != NULL )
		{
			VLong scopeId(inScopeId);
			query->AddCriteria(fSymbolReturnFieldScopeId, DB4D_Equal, scopeId);
			
			CDB4DSelection* results = fSymbolReturnsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						VString name = GetStringField(record, fSymbolReturnFieldName);
						outNames.push_back(name);

						found = true;
						
						ReleaseRefCountable(&record);
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

void VSymbolTable::InnerSymbolTable::SetReturnType(const sLONG& inSourceId, const sLONG& inScopeId, const XBOX::VString& inTypeName, CDB4DBaseContext* inContext)
{
	xbox_assert( inContext != NULL && fSymbolReturnsTable != NULL && fSymbolReturnFieldScopeId != NULL && fSymbolReturnFieldName != NULL && fSymbolReturnFieldStatus != NULL && fSymbolReturnFieldSourceId != NULL );
	if( inContext != NULL && fSymbolReturnsTable != NULL && fSymbolReturnFieldScopeId != NULL && fSymbolReturnFieldName != NULL && fSymbolReturnFieldStatus != NULL && fSymbolReturnFieldSourceId != NULL )
	{
		CDB4DRecord* record = fSymbolReturnsTable->NewRecord(inContext);
		if( record != NULL )
		{
			// Set field values
			record->SetLong(	fSymbolReturnFieldScopeId,	inScopeId);
			record->SetString(	fSymbolReturnFieldName,		inTypeName);
			record->SetLong(	fSymbolReturnFieldStatus,	eSymbolStatusUpToDate);
			record->SetLong(	fSymbolReturnFieldSourceId,	inSourceId);
			
			// Save
			record->Save();
			
			ReleaseRefCountable(&record);
		}
	}
}

/*
 *	SYMBOL'S TYPES MANAGEMENT
 */
bool VSymbolTable::InnerSymbolTable::GetType(const sLONG& inDefinitionId, const XBOX::VString& inTypeName, ISymbolUpdateParameters* inUpdateParameters, sLONG& outTypeId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolTypesTable != NULL && fSymbolTypeFieldDefinitionId != NULL && fSymbolTypeFieldName != NULL && fSymbolTypeFieldStatus != NULL && fSymbolTypeFieldId != NULL );
	if( inContext != NULL && fSymbolTypesTable != NULL && fSymbolTypeFieldDefinitionId != NULL && fSymbolTypeFieldName != NULL && fSymbolTypeFieldStatus != NULL && fSymbolTypeFieldId != NULL )
	{
		CDB4DQuery* query = fSymbolTypesTable->NewQuery();
		if( query != NULL )
		{
			VLong definitionId(inDefinitionId);
			query->AddCriteria(fSymbolTypeFieldDefinitionId, DB4D_Equal, definitionId);
			
			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fSymbolTypeFieldName, DB4D_Equal, inTypeName);

			CDB4DSelection* results = fSymbolTypesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					if( inUpdateParameters == NULL )
					{
						// As we don't plan to use the record for write operations, we DO NOT LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
						if( record != NULL )
						{
							outTypeId = GetIntegerField(record, fSymbolTypeFieldId, NULL);
							
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
					else
					{
						// As we plan to use the record for write operations, we LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Keep_Lock_With_Record, inContext );
						if( record != NULL )
						{
							outTypeId = GetIntegerField(record, fSymbolTypeFieldId, NULL);
							
							if( inUpdateParameters->MustSetStatus() == true )
							{
								ESymbolStatus status;
								inUpdateParameters->GetStatus(status);
								record->SetLong(fSymbolTypeFieldStatus,	status);
								
								// Save
								record->Save();
							}
							
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetTypeNames(const sLONG& inDefinitionId, std::vector<XBOX::VString>& outNames, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolTypesTable != NULL && fSymbolTypeFieldDefinitionId != NULL && fSymbolTypeFieldName != NULL );
	if( inContext != NULL && fSymbolTypesTable != NULL && fSymbolTypeFieldDefinitionId != NULL && fSymbolTypeFieldName != NULL )
	{
		CDB4DQuery* query = fSymbolTypesTable->NewQuery();
		if( query != NULL )
		{
			VLong definitionId(inDefinitionId);
			query->AddCriteria(fSymbolTypeFieldDefinitionId, DB4D_Equal, definitionId);
			
			CDB4DSelection* results = fSymbolTypesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						VString name = GetStringField(record, fSymbolTypeFieldName);
						outNames.push_back(name);
						
						found = true;
						
						ReleaseRefCountable(&record);
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::HasUpToDateDefinedType(const sLONG& inDefinitionId, CDB4DBaseContext* inContext)
{
	bool result = false;

	xbox_assert( inContext != NULL && fSymbolTypesTable != NULL && fSymbolTypeFieldDefinitionId != NULL && fSymbolTypeFieldName != NULL && fSymbolTypeFieldStatus != NULL );
	if( inContext != NULL && fSymbolTypesTable != NULL && fSymbolTypeFieldDefinitionId != NULL && fSymbolTypeFieldName != NULL && fSymbolTypeFieldStatus != NULL )
	{
		CDB4DQuery* query = fSymbolTypesTable->NewQuery();
		if( query != NULL )
		{
			VLong definitionId(inDefinitionId);
			query->AddCriteria(fSymbolTypeFieldDefinitionId, DB4D_Equal, definitionId);
			
			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fSymbolTypeFieldName, DB4D_NotEqual, CVSTR("Undefined"));

			query->AddLogicalOperator(DB4D_And);
			VLong statusValue(eSymbolStatusUpToDate);
			query->AddCriteria(fSymbolTypeFieldStatus, DB4D_Equal, statusValue);
			
			CDB4DSelection* results = fSymbolTypesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				if( results->CountRecordsInSelection(inContext) > 0 )
					result = true;

				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}

	return result;
}

void VSymbolTable::InnerSymbolTable::SetType(const sLONG& inDefinitionId, const sLONG& inSourceId, const XBOX::VString& inTypeName, CDB4DBaseContext* inContext)
{
	if( inTypeName == CVSTR("Undefined") && HasUpToDateDefinedType(inDefinitionId, inContext) == true )
		return;
	
	xbox_assert( inContext != NULL && fSymbolTypesTable != NULL && fSymbolTypeFieldDefinitionId != NULL && fSymbolTypeFieldName != NULL && fSymbolTypeFieldSourceId != NULL && fSymbolTypeFieldStatus != NULL );
	if( inContext != NULL && fSymbolTypesTable != NULL && fSymbolTypeFieldDefinitionId != NULL && fSymbolTypeFieldName != NULL && fSymbolTypeFieldSourceId != NULL && fSymbolTypeFieldStatus != NULL )
	{
		CDB4DRecord* record = fSymbolTypesTable->NewRecord(inContext);
		if( record != NULL )
		{
			// Set field values
			record->SetLong(	fSymbolTypeFieldDefinitionId,	inDefinitionId);
			record->SetString(	fSymbolTypeFieldName,			inTypeName);
			record->SetLong(	fSymbolTypeFieldSourceId,		inSourceId);
			record->SetLong(	fSymbolTypeFieldStatus,			eSymbolStatusUpToDate);
			
			// Save
			record->Save();
			
			ReleaseRefCountable(&record);
		}
		
		// If inTypeName is different from "Undefined", remove any occurences of "Undefined"
		if( inTypeName != CVSTR("Undefined") )
		{
			sLONG undefinedTypeId;
			if( GetType(inDefinitionId, CVSTR("Undefined"), NULL, undefinedTypeId, inContext) == true )
			{
				CDB4DQuery* query = fSymbolTypesTable->NewQuery();
				if( query != NULL )
				{
					VLong id(undefinedTypeId);
					query->AddCriteria(fSymbolTypeFieldId, DB4D_Equal, id);
					
					CDB4DSelection* results = fSymbolTypesTable->ExecuteQuery(query, inContext);
					if( results != NULL )
					{
						CDB4DRecord* record = results->LoadSelectedRecord( 1, DB4D_Keep_Lock_With_Record, inContext );
						if( record != NULL )
						{
							// Delete the current name
							record->Drop();
							ReleaseRefCountable(&record);
						}
					}
					ReleaseRefCountable(&results);
				}
				ReleaseRefCountable(&query);
			}
		}
	}
}


/*
 *	SYMBOL'S REFERENCES MANAGEMENT
 */
bool VSymbolTable::InnerSymbolTable::GetReference(const ESymbolReferenceKind& inReferenceKind, const sLONG& inFromId, const sLONG& inToId, ISymbolUpdateParameters* inUpdateParameters, sLONG& outReferenceId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolReferencesTable != NULL && fSymbolReferenceFieldType != NULL && fSymbolReferenceFieldFromId != NULL && fSymbolReferenceFieldToId != NULL && fSymbolReferenceFieldId != NULL );
	if( inContext != NULL && fSymbolReferencesTable != NULL && fSymbolReferenceFieldType != NULL && fSymbolReferenceFieldFromId != NULL && fSymbolReferenceFieldToId != NULL && fSymbolReferenceFieldId != NULL )
	{
		CDB4DQuery* query = fSymbolReferencesTable->NewQuery();
		if( query != NULL )
		{
			VLong kind(inReferenceKind);
			query->AddCriteria(fSymbolReferenceFieldType, DB4D_Equal, kind);
			
			query->AddLogicalOperator(DB4D_And);
			VLong fromId(inFromId);
			query->AddCriteria(fSymbolReferenceFieldFromId, DB4D_Equal, fromId);
			
			query->AddLogicalOperator(DB4D_And);
			VLong toId(inToId);
			query->AddCriteria(fSymbolReferenceFieldToId, DB4D_Equal, toId);
			
			CDB4DSelection* results = fSymbolReferencesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					if( inUpdateParameters == NULL )
					{
						// As we don't plan to use the record for write operations, we DO NOT LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
						if( record != NULL )
						{
							outReferenceId = GetIntegerField(record, fSymbolReferenceFieldId, NULL);
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
					else
					{
						// As we plan to use the record for write operations, we LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Keep_Lock_With_Record, inContext );
						if( record != NULL )
						{
							outReferenceId = GetIntegerField(record, fSymbolReferenceFieldId, NULL);
							
							if( inUpdateParameters->MustSetStatus() == true )
							{
								ESymbolStatus status;
								inUpdateParameters->GetStatus(status);
								record->SetLong(fSymbolReferenceFieldStatus,	status);
								
								// Save
								record->Save();
							}
							
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetReference(const ESymbolReferenceKind& inReferenceKind, const sLONG& inFromId, ISymbolUpdateParameters* inUpdateParameters, std::vector<sLONG>& outReferenceIds, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolReferencesTable != NULL && fSymbolReferenceFieldType != NULL && fSymbolReferenceFieldFromId != NULL && fSymbolReferenceFieldId != NULL );
	if( inContext != NULL && fSymbolReferencesTable != NULL && fSymbolReferenceFieldType != NULL && fSymbolReferenceFieldFromId != NULL && fSymbolReferenceFieldId != NULL )
	{
		CDB4DQuery* query = fSymbolReferencesTable->NewQuery();
		if( query != NULL )
		{
			VLong fromId(inFromId);
			query->AddCriteria(fSymbolReferenceFieldFromId, DB4D_Equal, fromId);
			
			if( inReferenceKind != eSymbolReferenceKindAll )
			{
				query->AddLogicalOperator(DB4D_And);
				VLong kind(inReferenceKind);
				query->AddCriteria(fSymbolReferenceFieldType, DB4D_Equal, kind);
			}

			CDB4DSelection* results = fSymbolReferencesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					if( inUpdateParameters == NULL )
					{
						// As we don't plan to use the record for write operations, we DO NOT LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
						if( record != NULL )
						{
							outReferenceIds.push_back(GetIntegerField(record, fSymbolReferenceFieldId, NULL));
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
					else
					{
						// As we plan to use the record for write operations, we LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Keep_Lock_With_Record, inContext );
						if( record != NULL )
						{
							outReferenceIds.push_back(GetIntegerField(record, fSymbolReferenceFieldId, NULL));
							
							if( inUpdateParameters->MustSetStatus() == true )
							{
								ESymbolStatus status;
								inUpdateParameters->GetStatus(status);
								record->SetLong(fSymbolReferenceFieldStatus,	status);
								
								// Save
								record->Save();
							}
							
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}


bool VSymbolTable::InnerSymbolTable::GetReferenceToId(const sLONG& inReferenceId, sLONG& outToId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolReferencesTable != NULL && fSymbolReferenceFieldId != NULL && fSymbolReferenceFieldToId != NULL );
	if( inContext != NULL && fSymbolReferencesTable != NULL && fSymbolReferenceFieldId != NULL && fSymbolReferenceFieldToId != NULL )
	{
		CDB4DQuery* query = fSymbolReferencesTable->NewQuery();
		if( query != NULL )
		{
			VLong referenceId(inReferenceId);
			query->AddCriteria(fSymbolReferenceFieldId, DB4D_Equal, referenceId);

			CDB4DSelection* results = fSymbolReferencesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );

				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );

				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outToId = GetIntegerField(record, fSymbolReferenceFieldToId, NULL);
						ReleaseRefCountable(&record);
						found = true;
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}

	return found;
}

bool VSymbolTable::InnerSymbolTable::GetReferenceKind(const sLONG& inReferenceId, ESymbolReferenceKind& outReferenceKind, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolReferencesTable != NULL && fSymbolReferenceFieldId != NULL && fSymbolReferenceFieldType != NULL );
	if( inContext != NULL && fSymbolReferencesTable != NULL && fSymbolReferenceFieldId != NULL && fSymbolReferenceFieldType != NULL )
	{
		CDB4DQuery* query = fSymbolReferencesTable->NewQuery();
		if( query != NULL )
		{
			VLong referenceId(inReferenceId);
			query->AddCriteria(fSymbolReferenceFieldId, DB4D_Equal, referenceId);
			
			CDB4DSelection* results = fSymbolReferencesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				// We expect to have 0 or 1 matching results ONLY
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outReferenceKind = GetIntegerField(record, fSymbolReferenceFieldType, NULL);
						ReleaseRefCountable(&record);
						found = true;
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::IsReferenced(const sLONG& inScopeId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolReferencesTable != NULL && fSymbolReferenceFieldToId != NULL );
	if( inContext != NULL && fSymbolReferencesTable != NULL && fSymbolReferenceFieldToId != NULL )
	{
		CDB4DQuery* query = fSymbolReferencesTable->NewQuery();
		if( query != NULL )
		{
			VLong id(inScopeId);
			query->AddCriteria(fSymbolReferenceFieldToId, DB4D_Equal, id);
			
			CDB4DSelection* results = fSymbolReferencesTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				
				if( total >= 1 )
					found = true;

				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

void VSymbolTable::InnerSymbolTable::SetReference(const ESymbolReferenceKind& inReferenceKind, const sLONG& inFromId, const sLONG& inToId, const sLONG& inSourceId, CDB4DBaseContext* inContext)
{
	xbox_assert( inContext != NULL && fSymbolReferencesTable != NULL && fSymbolReferenceFieldType != NULL && fSymbolReferenceFieldFromId != NULL && fSymbolReferenceFieldToId != NULL && fSymbolReferenceFieldSourceId != NULL && fSymbolReferenceFieldStatus != NULL );
	if( inContext != NULL && fSymbolReferencesTable != NULL && fSymbolReferenceFieldType != NULL && fSymbolReferenceFieldFromId != NULL && fSymbolReferenceFieldToId != NULL && fSymbolReferenceFieldSourceId != NULL && fSymbolReferenceFieldStatus != NULL )
	{
		CDB4DRecord* record = fSymbolReferencesTable->NewRecord(inContext);
		if( record != NULL )
		{
			// Set field values
			record->SetLong(	fSymbolReferenceFieldType,		inReferenceKind);
			record->SetLong(	fSymbolReferenceFieldFromId,	inFromId);
			record->SetLong(	fSymbolReferenceFieldToId,		inToId);
			record->SetLong(	fSymbolReferenceFieldSourceId,	inSourceId);
			record->SetLong(	fSymbolReferenceFieldStatus,	eSymbolStatusUpToDate);
			
			// Save
			record->Save();
			
			ReleaseRefCountable(&record);
		}
	}
}


/*
 *	SYMBOL'S OUTLINE MANAGEMENT
 */
bool VSymbolTable::InnerSymbolTable::GetScopeContentDefinitions(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, std::vector<sLONG>& outIds, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldScopeId != NULL && fSymbolDefinitionFieldExecutionContext != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldKind != NULL );
	if( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldScopeId != NULL && fSymbolDefinitionFieldExecutionContext != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldKind != NULL )
	{
		CDB4DQuery* query = fSymbolDefinitionsTable->NewQuery();
		if( query != NULL )
		{
			VLong scopeId(inScopeId);
			query->AddCriteria(fSymbolDefinitionFieldScopeId, DB4D_Equal, scopeId);
			
			query->AddLogicalOperator(DB4D_And);
			
			VLong executionContext(inExecutionContext);
			query->AddCriteria(fSymbolDefinitionFieldExecutionContext, DB4D_Equal, executionContext);
			
			CDB4DSelection* results = fSymbolDefinitionsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						sLONG id = GetIntegerField(record, fSymbolDefinitionFieldId, NULL);
						outIds.push_back(id);
							
						found = true;
						
						ReleaseRefCountable(&record);
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetScopeContentDefinitions(const sLONG& inScopeId, const ESymbolFileExecContext& inExecutionContext, const sLONG& inSourceId, std::vector<sLONG>& outIds, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldScopeId != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldSourceId != NULL );
	if( inContext != NULL && fSymbolDefinitionsTable != NULL && fSymbolDefinitionFieldScopeId != NULL && fSymbolDefinitionFieldId != NULL && fSymbolDefinitionFieldSourceId != NULL )
	{
		CDB4DQuery* query = fSymbolDefinitionsTable->NewQuery();
		if( query != NULL )
		{
			VLong scopeId(inScopeId);
			query->AddCriteria(fSymbolDefinitionFieldScopeId, DB4D_Equal, scopeId);
			
			query->AddLogicalOperator(DB4D_And);
			VLong sourceId(inSourceId);
			query->AddCriteria(fSymbolDefinitionFieldSourceId, DB4D_Equal, sourceId);
			
			CDB4DSelection* results = fSymbolDefinitionsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; ++index )
				{
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						// Remove children of current scope
						sLONG id = GetIntegerField(record, fSymbolDefinitionFieldId, NULL);
						outIds.push_back(id);
						
						found = true;
						
						ReleaseRefCountable(&record);
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}


/*
 *	OUTLINE ITEM DATAS MANAGEMENT
 */
bool VSymbolTable::InnerSymbolTable::GetOutlineItem(const sLONG& inSourceId, const XBOX::VString& inName, const sLONG& inLine, ISymbolUpdateParameters* inUpdateParameters, sLONG& outItemId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fOutlineItemsTable != NULL && fOutlineItemFieldSourceId != NULL && fOutlineItemFieldName != NULL && fOutlineItemFieldLine != NULL && fOutlineItemFieldId != NULL && fOutlineItemFieldStatus != NULL );
	if( inContext != NULL && fOutlineItemsTable != NULL && fOutlineItemFieldSourceId != NULL && fOutlineItemFieldName != NULL && fOutlineItemFieldLine != NULL && fOutlineItemFieldId != NULL && fOutlineItemFieldStatus != NULL )
	{
		CDB4DQuery* query = fOutlineItemsTable->NewQuery();
		if( query != NULL )
		{
			VLong sourceId(inSourceId);
			query->AddCriteria(fOutlineItemFieldSourceId, DB4D_Equal, sourceId);
			
			query->AddLogicalOperator(DB4D_And);
			query->AddCriteria(fOutlineItemFieldName, DB4D_Equal, inName);

			query->AddLogicalOperator(DB4D_And);
			VLong line(inLine);
			query->AddCriteria(fOutlineItemFieldLine, DB4D_Equal, line);

			CDB4DSelection* results = fOutlineItemsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					if( inUpdateParameters == NULL )
					{
						// As we don't plan to use the record for write operations, we DO NOT LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
						if( record != NULL )
						{
							outItemId = GetIntegerField(record, fOutlineItemFieldId, NULL);
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
					else
					{
						// As we plan to use the record for write operations, we LOCK it !
						CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Keep_Lock_With_Record, inContext );
						if( record != NULL )
						{
							outItemId = GetIntegerField(record, fOutlineItemFieldId, NULL);
							
							if( inUpdateParameters->MustSetStatus() == true )
							{
								ESymbolStatus status;
								inUpdateParameters->GetStatus(status);
								record->SetLong(fOutlineItemFieldStatus, status);
								
								// Save
								record->Save();
							}
							
							ReleaseRefCountable(&record);
							
							found = true;
						}
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetOutlineItemSourceId(const sLONG& inItemId, sLONG& outSourceId, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fOutlineItemsTable != NULL && fOutlineItemFieldSourceId != NULL && fOutlineItemFieldId != NULL );
	if( inContext != NULL && fOutlineItemsTable != NULL && fOutlineItemFieldSourceId != NULL && fOutlineItemFieldId != NULL )
	{
		CDB4DQuery* query = fOutlineItemsTable->NewQuery();
		if( query != NULL )
		{
			VLong itemId(inItemId);
			query->AddCriteria(fOutlineItemFieldId, DB4D_Equal, itemId);
			
			CDB4DSelection* results = fOutlineItemsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outSourceId = GetIntegerField(record, fOutlineItemFieldSourceId, NULL);
						ReleaseRefCountable(&record);
							
						found = true;
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetOutlineItemName(const sLONG& inItemId, XBOX::VString& outName, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fOutlineItemsTable != NULL && fOutlineItemFieldName != NULL && fOutlineItemFieldId != NULL );
	if( inContext != NULL && fOutlineItemsTable != NULL && fOutlineItemFieldName != NULL && fOutlineItemFieldId != NULL )
	{
		CDB4DQuery* query = fOutlineItemsTable->NewQuery();
		if( query != NULL )
		{
			VLong itemId(inItemId);
			query->AddCriteria(fOutlineItemFieldId, DB4D_Equal, itemId);
			
			CDB4DSelection* results = fOutlineItemsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outName = GetStringField(record, fOutlineItemFieldName);
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

bool VSymbolTable::InnerSymbolTable::GetOutlineItemLine(const sLONG& inItemId, sLONG& outLine, CDB4DBaseContext* inContext) const
{
	bool found = false;
	
	xbox_assert( inContext != NULL && fOutlineItemsTable != NULL && fOutlineItemFieldLine != NULL && fOutlineItemFieldId != NULL );
	if( inContext != NULL && fOutlineItemsTable != NULL && fOutlineItemFieldLine != NULL && fOutlineItemFieldId != NULL )
	{
		CDB4DQuery* query = fOutlineItemsTable->NewQuery();
		if( query != NULL )
		{
			VLong itemId(inItemId);
			query->AddCriteria(fOutlineItemFieldId, DB4D_Equal, itemId);
			
			CDB4DSelection* results = fOutlineItemsTable->ExecuteQuery(query, inContext);
			if( results != NULL )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				xbox_assert( total <= 1 );
				
				if( total == 1 )
				{
					// As we don't plan to use the record for write operations, we DO NOT LOCK it !
					CDB4DRecord* record = results->LoadSelectedRecord( total, DB4D_Do_Not_Lock, inContext );
					if( record != NULL )
					{
						outLine = GetIntegerField(record, fOutlineItemFieldLine, NULL);
						ReleaseRefCountable(&record);
						
						found = true;
					}
				}
				ReleaseRefCountable(&results);
			}
			ReleaseRefCountable(&query);
		}
	}
	
	return found;
}

void VSymbolTable::InnerSymbolTable::SetOutlineItem(const sLONG& inSourceId, const XBOX::VString& inName, const sLONG& inLine, sLONG& outItemId, CDB4DBaseContext* inContext)
{
	xbox_assert( inContext != NULL && fOutlineItemsTable != NULL && fOutlineItemFieldSourceId != NULL && fOutlineItemFieldName != NULL && fOutlineItemFieldLine != NULL && fOutlineItemFieldStatus != NULL && fOutlineItemFieldId != NULL );
	if( inContext != NULL && fOutlineItemsTable != NULL && fOutlineItemFieldSourceId != NULL && fOutlineItemFieldName != NULL && fOutlineItemFieldLine != NULL && fOutlineItemFieldStatus != NULL && fOutlineItemFieldId != NULL )
	{
		CDB4DRecord* record = fOutlineItemsTable->NewRecord(inContext);
		if( record != NULL )
		{
			// Set field values
			record->SetLong(	fOutlineItemFieldSourceId,	inSourceId);
			record->SetString(	fOutlineItemFieldName,		inName);
			record->SetLong(	fOutlineItemFieldLine,		inLine);
			record->SetLong(	fOutlineItemFieldStatus,	eSymbolStatusUpToDate);
			
			// Save
			record->Save();
			
			outItemId = GetIntegerField(record, fOutlineItemFieldId, NULL);

			ReleaseRefCountable(&record);
		}
	}
}


/*
 *	SYMBOL'S CORE MANAGEMENT
 */
void VSymbolTable::InnerSymbolTable::_InsertECMA262CoreSymbols(const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG globalScopeId;
	
	_ECMA262_InsertGlobalObjectSymbolsScope							(inExecutionContext, inContext, globalScopeId);
	_ECMA262_InsertGlobalObjectSymbolsReservedKeywords				(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsReservedLiterals				(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsValueProperties				(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsFunctionProperties			(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsURIHandlingFunctionProperties	(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsObjectProperties				(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsConstructorObject				(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsConstructorFunction			(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsConstructorArray				(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsConstructorString				(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsConstructorBoolean			(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsConstructorNumber				(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsConstructorDate				(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsConstructorRegExp				(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsConstructorError				(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsConstructorEvalError			(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsConstructorRangeError			(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsConstructorReferenceError		(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsConstructorSyntaxError		(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsConstructorTypeError			(globalScopeId, inExecutionContext, inContext);
	_ECMA262_InsertGlobalObjectSymbolsConstructorURIError			(globalScopeId, inExecutionContext, inContext);
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsScope(const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext, sLONG& outGlobalScopeId)
{
	SetGlobalScope(inExecutionContext, outGlobalScopeId, inContext);
	
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsScope");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsScope");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsReservedKeywords(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG scopeId;
	sLONG definitionId;
	
	bool constructorFlag = false;
	
	// Those are reserved key words
	// NaN, Infinity and undefined
	SetName(	"break",			inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"case",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"catch",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"continue",			inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"debugger",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"default",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"delete",			inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"do",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"else",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"finally",			inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"for",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"function",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"if",			inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"in",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"instanceof",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"typeof",			inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"new",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"var",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"return",			inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"void",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"switch",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"while",			inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"this",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"with",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"throw",			inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"try",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);

	SetName(	"class",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"const",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"enum",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"export",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"extends",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"import",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
	SetName(	"super",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsReservedKeywords");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsReservedKeywords");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsReservedLiterals(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG scopeId;
	sLONG definitionId;
	
	bool constructorFlag = false;

	// Those are identifiers, not scopes
	// NaN, Infinity and undefined
	SetName(	"null",			inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	SetType(definitionId, 0, "Null", inContext);

	SetName(	"true",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	SetType(definitionId, 0, "Boolean", inContext);

	SetName(	"false",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	SetType(definitionId, 0, "Boolean", inContext);

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsReservedLiterals");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsReservedLiterals");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsValueProperties(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG scopeId;
	sLONG definitionId;

	bool constructorFlag = false;

	// Those are identifiers, not scopes
	// NaN, Infinity and undefined
	SetName(	"NaN",			inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	SetName(	"Infinity",		inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	SetName(	"undefined",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, inGlobalScopeId, nameId, inExecutionContext, eSymbolDefinitionKindKeyword, -1, definitionId, inContext);
	SetType(definitionId, 0, "Undefined", inContext);

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsValueProperties");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsValueProperties");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsFunctionProperties(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG scopeId;
	sLONG definitionId;

	bool constructorFlag = false;

	// Those are scopes
	// eval, parseInt, parseFloat, isNaN and isFinite
	SetScope(	"eval",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, scopeId, inContext);
	SetName(	"eval",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, scopeId, "Object", inContext);

	SetScope(	"parseInt",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, scopeId, inContext);
	SetName(	"parseInt",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, scopeId, "Number", inContext);
	
	SetScope(	"parseFloat",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, scopeId, inContext);
	SetName(	"parseFloat",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, scopeId, "Number", inContext);
	
	SetScope(	"isNaN",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, scopeId, inContext);
	SetName(	"isNaN",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, scopeId, "Boolean", inContext);
	
	SetScope(	"isFinite",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, scopeId, inContext);
	SetName(	"isFinite",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, scopeId, "Boolean", inContext);
	
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsFunctionProperties");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsFunctionProperties");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsURIHandlingFunctionProperties(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG scopeId;
	sLONG definitionId;

	bool constructorFlag = false;

	// Those are scopes
	// decodeURI, decodeURIComponent, encoreURI and encoreURIComponent
	SetScope(	"decodeURI",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, scopeId, inContext);
	SetName(	"decodeURI",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, scopeId, "String", inContext);

	SetScope(	"decodeURIComponent",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, scopeId, inContext);
	SetName(	"decodeURIComponent",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, scopeId, "String", inContext);
	
	SetScope(	"encoreURI",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, scopeId, inContext);
	SetName(	"encoreURI",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, scopeId, "String", inContext);
	
	SetScope(	"encoreURIComponent",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, scopeId, inContext);
	SetName(	"encoreURIComponent",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, scopeId, "String", inContext);
	
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsURIHandlingFunctionProperties");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsURIHandlingFunctionProperties");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsObjectProperties(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = false;

	// Those are objects
	// ... THE MATH
	SetScope(	"Math",	eSymbolScopeTypeObject, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"Math",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "Math", "Object", inContext);
	
	SetName(	"E",		constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);
	
	SetName(	"LN10",		constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	SetName(	"LN2",		constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	SetName(	"LOG2E",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	SetName(	"LOG10E",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	SetName(	"PI",		constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	SetName(	"SQRT1_2",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	SetName(	"SQRT2",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	sLONG propertyScopeId;
	SetScope(	"abs",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"abs",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);

	SetScope(	"acos",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"acos",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	SetScope(	"asin",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"asin",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	SetScope(	"atan",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"atan",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	SetScope(	"atan2",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"atan2",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	SetScope(	"ceil",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"ceil",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	SetScope(	"cos",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"cos",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	SetScope(	"exp",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"exp",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	SetScope(	"floor",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"floor",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	SetScope(	"log",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"log",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	SetScope(	"max",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"max",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	SetScope(	"min",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"min",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	SetScope(	"pow",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"pow",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	SetScope(	"random",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"random",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);

	SetScope(	"round",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"round",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	SetScope(	"sin",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"sin",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	SetScope(	"sqrt",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"sqrt",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	SetScope(	"tan",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"tan",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	
	// ... THE JSON OBJECT
	SetScope(	"JSON",	eSymbolScopeTypeObject, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"JSON",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "JSON", "Object", inContext);

	SetScope(	"parse",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"parse",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);

	SetScope(	"stringify",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"stringify",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsObjectProperties");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsObjectProperties");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsConstructorObject(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = true;

	SetScope(	"Object",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"Object",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "Object", "", inContext);
	SetReturnType(0, constructorScopeId, "Object", inContext);

	constructorFlag = false;

	sLONG propertyScopeId;
	SetScope(	"getPrototypeOf",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getPrototypeOf",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"getOwnPropertyDescriptor",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getOwnPropertyDescriptor",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"getOwnPropertyNames",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getOwnPropertyNames",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Array", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"create",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"create",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"defineProperty",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"defineProperty",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"defineProperties",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"defineProperties",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"seal",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"seal",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"freeze",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"freeze",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"preventExtensions",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"preventExtensions",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"isSealed",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"isSealed",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Boolean", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"isFrozen",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"isFrozen",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Boolean", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"isExtensible",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"isExtensible",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Boolean", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"keys",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"keys",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Array", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	sLONG prototypeScopeId;
	SetScope(	"prototype",	eSymbolScopeTypeObject, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, prototypeScopeId, inContext);
	SetName(	"prototype",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"constructor",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"constructor",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toLocaleString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toLocaleString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"valueOf",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"valueOf",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"hasOwnProperty",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"hasOwnProperty",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Boolean", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"isPrototypeOf",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"isPrototypeOf",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Boolean", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"propertyIsEnumerable",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"propertyIsEnumerable",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Boolean", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorObject");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorObject");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsConstructorFunction(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = true;

	SetScope(	"Function",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"Function",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "Function", "Object", inContext);

	constructorFlag = false;
	
	SetName(	"length",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	sLONG prototypeScopeId;
	SetScope(	"prototype",	eSymbolScopeTypeObject, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, prototypeScopeId, inContext);
	SetName(	"prototype",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	sLONG propertyScopeId;
	SetScope(	"constructor",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"constructor",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"apply",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"apply",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"call",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"call",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"bind",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"bind",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorFunction");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorFunction");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsConstructorArray(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = true;

	SetScope(	"Array",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"Array",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "Array", "Object", inContext);

	constructorFlag = false;
	
	sLONG propertyScopeId;
	SetScope(	"isArray",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"isArray",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Boolean", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);

	SetName(	"length",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);
	
	sLONG prototypeScopeId;
	SetScope(	"prototype",	eSymbolScopeTypeObject, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, prototypeScopeId, inContext);
	SetName(	"prototype",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"concat",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"concat",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Array", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"constructor",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"constructor",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"every",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"every",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Boolean", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"filter",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"filter",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Boolean", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"forEach",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"forEach",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"indexOf",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"indexOf",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"join",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"join",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"lastIndexOf",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"lastIndexOf",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"map",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"map",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"pop",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"pop",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"push",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"push",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"reduce",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"reduce",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"reduceRight",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"reduceRight",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"reverse",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"reverse",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Array", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"shift",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"shift",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Object", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"slice",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"slice",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Array", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"some",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"some",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Boolean", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"sort",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"sort",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Array", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"splice",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"splice",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Array", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toLocaleString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toLocaleString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"unshift",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"unshift",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorArray");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorArray");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsConstructorString(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = true;

	SetScope(	"String",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"String",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "String", "Object", inContext);

	constructorFlag = false;
	
	sLONG propertyScopeId;
	SetScope(	"fromCharCode",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"fromCharCode",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);

	sLONG prototypeScopeId;
	SetScope(	"prototype",	eSymbolScopeTypeObject, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, prototypeScopeId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);

	SetName(	"length",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);
	
	SetScope(	"constructor",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"constructor",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"valueOf",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"valueOf",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"charAt",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"charAt",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"charCodeAt",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"charCodeAt",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"concat",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"concat",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"indexOf",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"indexOf",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"lastIndexOf",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"lastIndexOf",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"localeCompare",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"localeCompare",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"match",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"match",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Array", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"replace",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"replace",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"search",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"search",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"slice",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"slice",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"split",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"split",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"substr",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"substr",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"substring",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"substring",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toLowerCase",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toLowerCase",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toLocaleLowerCase",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toLocaleLowerCase",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toUpperCase",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toUpperCase",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toLocaleUpperCase",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toLocaleUpperCase",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"trim",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"trim",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorString");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorString");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsConstructorBoolean(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = true;
	
	SetScope(	"Boolean",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"Boolean",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "Boolean", "Object", inContext);

	constructorFlag = false;
	
	SetName(	"length",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);
	
	sLONG prototypeScopeId;
	SetScope(	"prototype",	eSymbolScopeTypeObject, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, prototypeScopeId, inContext);
	SetName(	"prototype",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	sLONG propertyScopeId;
	SetScope(	"constructor",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"constructor",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

	SetScope(	"valueOf",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"valueOf",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Boolean", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorBoolean");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorBoolean");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsConstructorNumber(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = true;
	
	SetScope(	"Number",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"Number",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "Number", "Object", inContext);

	constructorFlag = false;
	
	SetName(	"length",				constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	SetName(	"MAX_VALUE",			constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	SetName(	"MIN_VALUE",			constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	SetName(	"NaN",					constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	SetName(	"NEGATIVE_INFINITY",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	SetName(	"POSITIVE_INFINITY",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	sLONG prototypeScopeId;
	SetScope(	"prototype",	eSymbolScopeTypeObject, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, prototypeScopeId, inContext);
	SetName(	"prototype",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	sLONG propertyScopeId;
	SetScope(	"constructor",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"constructor",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

	SetScope(	"toLocaleString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toLocaleString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

	SetScope(	"valueOf",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"valueOf",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

	SetScope(	"toFixed",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toFixed",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toExponential",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toExponential",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toPrecision",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toPrecision",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorNumber");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorNumber");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsConstructorDate(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = true;
	
	SetScope(	"Date",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"Date",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "Date", "Object", inContext);

	constructorFlag = false;
	
	sLONG propertyScopeId;
	SetScope(	"parse",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"parse",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);

	SetScope(	"UTC",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"UTC",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"now",	eSymbolScopeTypeFunctionExpression, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"now",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetName(	"length",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	sLONG prototypeScopeId;
	SetScope(	"prototype",	eSymbolScopeTypeObject, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, prototypeScopeId, inContext);
	SetName(	"prototype",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetScope(	"constructor",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"constructor",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toDateString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toDateString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toTimeString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toTimeString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toLocaleString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toLocaleString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toLocaleDateString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toLocaleDateString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toLocaleTimeString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toLocaleTimeString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"valueOf",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"valueOf",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getTime",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getTime",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getFullYear",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getFullYear",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getUTCFullYear",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getUTCFullYear",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getMonth",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getMonth",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getUTCMonth",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getUTCMonth",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getDate",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getDate",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getUTCDate",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getUTCDate",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getDay",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getDay",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getUTCDay",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getUTCDay",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getHours",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getHours",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getUTCHours",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getUTCHours",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getMinutes",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getMinutes",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getUTCMinutes",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getUTCMinutes",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getSeconds",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getSeconds",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getUTCSeconds",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getUTCSeconds",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getMilliseconds",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getMilliseconds",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getUTCMilliseconds",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getUTCMilliseconds",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"getTimezoneOffset",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"getTimezoneOffset",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Number", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"setTime",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"setTime",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

	SetScope(	"setMilliseconds",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"setMilliseconds",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"setUTCMilliseconds",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"setUTCMilliseconds",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"setSeconds",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"setSeconds",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"setUTCSeconds",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"setUTCSeconds",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"setMinutes",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"setMinutes",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"setUTCMinutes",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"setUTCMinutes",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"setHours",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"setHours",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"setUTCHours",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"setUTCHours",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"setDate",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"setDate",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"setUTCDate",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"setUTCDate",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"setMonth",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"setMonth",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"setUTCMonth",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"setUTCMonth",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"setFullYear",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"setFullYear",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"setUTCFullYear",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"setUTCFullYear",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Date", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toUTCString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toUTCString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toISOString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toISOString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toJSON",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toJSON",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorDate");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorDate");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsConstructorRegExp(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = true;
	
	SetScope(	"RegExp",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"RegExp",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "RegExp", "Object", inContext);

	constructorFlag = false;
	
	SetName(	"length",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	sLONG prototypeScopeId;
	SetScope(	"prototype",	eSymbolScopeTypeObject, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, prototypeScopeId, inContext);
	SetName(	"prototype",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);

	SetName(	"source",		constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "String", inContext);

	SetName(	"global",		constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Boolean", inContext);

	SetName(	"ignoreCase",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Boolean", inContext);

	SetName(	"multiline",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Boolean", inContext);

	SetName(	"lastIndex",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);

	sLONG propertyScopeId;
	SetScope(	"constructor",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"constructor",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

	SetScope(	"exec",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"exec",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Array", inContext);
	SetReturnType(0, propertyScopeId, "Null", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

	SetScope(	"test",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"test",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "Boolean", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

	SetScope(	"toString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorRegExp");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorRegExp");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsConstructorError(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = true;
	
	SetScope(	"Error",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"Error",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "Error", "Object", inContext);

	constructorFlag = false;
	
	SetName(	"length",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);
	
	sLONG prototypeScopeId;
	SetScope(	"prototype",	eSymbolScopeTypeObject, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, prototypeScopeId, inContext);
	SetName(	"prototype",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetName(	"name",		prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

	SetName(	"message",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "String", inContext);

	sLONG propertyScopeId;
	SetScope(	"constructor",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"constructor",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorError");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorError");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsConstructorEvalError(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = true;
	
	SetScope(	"EvalError",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"EvalError",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "EvalError", "Error", inContext);

	constructorFlag = false;
	
	SetName(	"length",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);
	
	sLONG prototypeScopeId;
	SetScope(	"prototype",	eSymbolScopeTypeObject, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, prototypeScopeId, inContext);
	SetName(	"prototype",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetName(	"name",		prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "String", inContext);

	SetName(	"message",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "String", inContext);
	
	sLONG propertyScopeId;
	SetScope(	"constructor",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"constructor",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorEvalError");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorEvalError");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsConstructorRangeError(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = true;
	
	SetScope(	"RangeError",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"RangeError",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "RangeError", "Error", inContext);
	
	constructorFlag = false;
	
	SetName(	"length",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);
	
	sLONG prototypeScopeId;
	SetScope(	"prototype",	eSymbolScopeTypeObject, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, prototypeScopeId, inContext);
	SetName(	"prototype",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetName(	"name",		prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "String", inContext);

	SetName(	"message",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "String", inContext);
	
	sLONG propertyScopeId;
	SetScope(	"constructor",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"constructor",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorRangeError");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorRangeError");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsConstructorReferenceError(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = true;
	
	SetScope(	"ReferenceError",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"ReferenceError",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "ReferenceError", "Error", inContext);

	constructorFlag = false;
	
	SetName(	"length",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);
	
	sLONG prototypeScopeId;
	SetScope(	"prototype",	eSymbolScopeTypeObject, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, prototypeScopeId, inContext);
	SetName(	"prototype",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetName(	"name",		prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "String", inContext);

	SetName(	"message",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "String", inContext);
	
	sLONG propertyScopeId;
	SetScope(	"constructor",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"constructor",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorReferenceError");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorReferenceError");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsConstructorSyntaxError(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = true;
	
	SetScope(	"SyntaxError",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"SyntaxError",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "SyntaxError", "Error", inContext);

	constructorFlag = false;
	
	SetName(	"length",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);
	
	sLONG prototypeScopeId;
	SetScope(	"prototype",	eSymbolScopeTypeObject, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, prototypeScopeId, inContext);
	SetName(	"prototype",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetName(	"name",		prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "String", inContext);

	SetName(	"message",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "String", inContext);
	
	sLONG propertyScopeId;
	SetScope(	"constructor",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"constructor",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorSyntaxError");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorSyntaxError");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsConstructorTypeError(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = true;
	
	SetScope(	"TypeError",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"TypeError",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "TypeError", "Error", inContext);

	constructorFlag = false;
	
	SetName(	"length",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);
	
	sLONG prototypeScopeId;
	SetScope(	"prototype",	eSymbolScopeTypeObject, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, prototypeScopeId, inContext);
	SetName(	"prototype",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetName(	"name",		prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "String", inContext);

	SetName(	"message",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "String", inContext);
	
	sLONG propertyScopeId;
	SetScope(	"constructor",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"constructor",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorTypeError");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorTypeError");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

void VSymbolTable::InnerSymbolTable::_ECMA262_InsertGlobalObjectSymbolsConstructorURIError(const sLONG& inGlobalScopeId, const ESymbolFileExecContext& inExecutionContext, CDB4DBaseContext* inContext)
{
	sLONG nameId;
	sLONG constructorScopeId;
	sLONG definitionId;

	bool constructorFlag = true;
	
	SetScope(	"URIError",	eSymbolScopeTypeFunctionDeclaration, inGlobalScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, constructorScopeId, inContext);
	SetName(	"URIError",	inGlobalScopeId, inExecutionContext, 0, nameId, inContext);
	SetPrototypeChain(inExecutionContext, 0, "URIError", "Error", inContext);

	constructorFlag = false;
	
	SetName(	"length",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "Number", inContext);
	
	sLONG prototypeScopeId;
	SetScope(	"prototype",	eSymbolScopeTypeObject, constructorScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, prototypeScopeId, inContext);
	SetName(	"prototype",	constructorScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, constructorScopeId, nameId, inExecutionContext, eSymbolDefinitionKindProperty, -1, definitionId, inContext);
	
	SetName(	"name",		prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "String", inContext);

	SetName(	"message",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	SetType(definitionId, 0, "String", inContext);
	
	sLONG propertyScopeId;
	SetScope(	"constructor",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"constructor",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);
	
	SetScope(	"toString",	eSymbolScopeTypeFunctionExpression, prototypeScopeId, 0, inExecutionContext, 0, 0, 0, 0, constructorFlag, propertyScopeId, inContext);
	SetName(	"toString",	prototypeScopeId, inExecutionContext, 0, nameId, inContext);
	SetReturnType(0, propertyScopeId, "String", inContext);
	SetDefinition(0, prototypeScopeId, nameId, inExecutionContext, eSymbolDefinitionKindInstanceProperty, -1, definitionId, inContext);

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	{
		VString request("scopes id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorURIError");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
	{
		VString request("names id > 0");
		JSLog log;
		log.SetTitle("_ECMA262_InsertGlobalObjectSymbolsConstructorURIError");
		log.Append("Request", request);
		log.Print();
		Dump(request, inContext);
	}
#endif//ACTIVATE_JSLOG
#endif//ACTIVATE_SYMBOL_TABLE_TRACES
}

VSymbolTable::InnerSymbolTable::InnerSymbolTable(const bool& inUseNextGen):
fDatabaseManager( NULL ),
fDatabase( NULL ),
fNextUniqueSymbolID( 0 ),
kCurrentVersion( 16 ),
fScopesTable(NULL),
fScopeFieldId(NULL),
fScopeFieldName(NULL),
fScopeFieldFullName(NULL),
fScopeFieldType(NULL),
fScopeFieldSourceId(NULL),
fScopeFieldExecutionContext(NULL),
fScopeFieldParentId(NULL),
fScopeFieldStatus(NULL),
fScopeFieldFromLine(NULL),
fScopeFieldToLine(NULL),
fScopeFieldFromOffset(NULL),
fScopeFieldToOffset(NULL),
fScopeFieldConstructorFlag(NULL),
fSymbolNamesTable(NULL),
fSymbolNameFieldId(NULL),
fSymbolNameFieldSourceId(NULL),
fSymbolNameFieldValue(NULL),
fSymbolNameFieldScopeId(NULL),
fSymbolNameFieldExecutionContext(NULL),
fSymbolNameFieldStatus(NULL),
fSymbolDefinitionsTable(NULL),
fSymbolDefinitionFieldId(NULL),
fSymbolDefinitionFieldScopeId(NULL),
fSymbolDefinitionFieldNameId(NULL),
fSymbolDefinitionFieldKind(NULL),
fSymbolDefinitionFieldExecutionContext(NULL),
fSymbolDefinitionFieldStatus(NULL),
fSymbolDefinitionFieldSourceId(NULL),
fSymbolDefinitionFieldLineNumber(NULL),
fSymbolPrototypesTable(NULL),
fSymbolPrototypeFieldId(NULL),
fSymbolPrototypeFieldFromName(NULL),
fSymbolPrototypeFieldToName(NULL),
fSymbolPrototypeFieldExecutionContext(NULL),
fSymbolPrototypeFieldStatus(NULL),
fSymbolPrototypeFieldSourceId(NULL),
fSymbolReturnsTable(NULL),
fSymbolReturnFieldId(NULL),
fSymbolReturnFieldScopeId(NULL),
fSymbolReturnFieldName(NULL),
fSymbolReturnFieldStatus(NULL),
fSymbolReturnFieldSourceId(NULL),
fSymbolTypesTable(NULL),
fSymbolTypeFieldId(NULL),
fSymbolTypeFieldDefinitionId(NULL),
fSymbolTypeFieldName(NULL),
fSymbolTypeFieldStatus(NULL),
fSymbolTypeFieldSourceId(NULL),
fSymbolReferencesTable(NULL),
fSymbolReferenceFieldId(NULL),
fSymbolReferenceFieldType(NULL),
fSymbolReferenceFieldFromId(NULL),
fSymbolReferenceFieldToId(NULL),
fSymbolReferenceFieldStatus(NULL),
fSymbolReferenceFieldSourceId(NULL),
fOutlineItemsTable(NULL),
fOutlineItemFieldId(NULL),
fOutlineItemFieldSourceId(NULL),
fOutlineItemFieldName(NULL),
fOutlineItemFieldLine(NULL),
fOutlineItemFieldStatus(NULL),
fSymbolsTable(NULL),
fSymbolNameField(NULL),
fSymbolIDField(NULL),
fSymbolFileIDField(NULL),
fSymbolScriptDocTextField(NULL),
fSymbolKindField(NULL),
fSymbolWAFKindField(NULL),
fSymbolPrototypeIDField(NULL),
fSymbolOwnerIDField(NULL),
fSymbolLineNumberField(NULL),
fSymbolLineCompletionNumberField(NULL),
fSymbolReturnTypeIDField(NULL),
fSymbolReferenceIDField(NULL),
fSymbolUndefinedStateField(NULL),
fSymbolInstanceStateField(NULL),
fSymbolReferenceStateField(NULL),
fSymbolEditionStateField(NULL),
fSymbolFullNameField(NULL),
fFilesTable(NULL),
fFileIDField(NULL),
fFilePathField(NULL),
fFileBaseFolderField(NULL),
fFileExecContextField(NULL),
fFileModificationTimeField(NULL),
fFileResolvedStateField(NULL),
fExtrasTable(NULL),
fExtraIDField(NULL),
fExtrasOwnerSymbolIDField(NULL),
fExtrasKindField(NULL),
fExtrasStringDataField(NULL),
fExtrasIntegerDataField(NULL),
fVersionTable(NULL),
fVersionNumberField(NULL),
fUseNextGen(inUseNextGen)
{
	fDatabaseManager = CDB4DManager::RetainManager();
	fRecursionTaskLock = new VCriticalSection();
	fAllowedToAddSymbolsEvent.Unlock();	// we are allowed to add symbols again
	
	VIntlMgr* mgr = VIntlMgr::GetDefaultMgr();
	fWildChar = mgr->GetWildChar();
	
#if ACTIVATE_JSLOG
	JSLog log;
	log.SetTitle( CVSTR("WildChar") );
	VString wildCharAsString;
	wildCharAsString.AppendUniChar(fWildChar);
	log.Append(CVSTR("Value"), wildCharAsString);
	log.Print();
#endif //ACTIVATE_JSLOG

}



VSymbolTable::InnerSymbolTable::~InnerSymbolTable()
{
	ReleaseFields();
	ReleaseTables();

	if( fDatabase != NULL )
		fDatabase->CloseAndRelease();
	
	if( fDatabaseManager != NULL )
		fDatabaseManager->Release();
	
	delete fRecursionTaskLock;
}



CDB4DBaseContext *VSymbolTable::InnerSymbolTable::GetContextForNewThread()
{
	if (!fDatabase)
		return NULL;
	
	return fDatabase->NewContext( NULL, NULL );
}



bool VSymbolTable::InnerSymbolTable::CheckRequiredStructure( CDB4DBaseContext *inContext )
{
	// We want to make sure the structure of the database contains all of the tables, fields and whatnot
	// that we require.  If we add any new fields or tables, we should check them here.  Note, we don't
	// check every possible field because we already know everyone's at a particular state and there's
	// no need to double-check what we already know.
	//
	// We can assume the database has already been opened at this point and the context is valid
	
	xbox_assert( fDatabase );
	xbox_assert( inContext );
	
	bool ret = true;
	
	// If the current version of the table is actually less than the version of the database we've opened,
	// we want to fail.  This prevents old versions of the product from opening new versions of the database.
	if (fVersionTable)
	{
		CDB4DRecord *rec = fVersionTable->LoadRecord( 0, DB4D_Do_Not_Lock, inContext );
		if (rec)
		{
			if (fVersionNumberField)
			{
				sLONG version = GetIntegerField( rec, fVersionNumberField, NULL );
				if (version < kCurrentVersion)
					ret = false;
#if ACTIVATE_JSLOG
				JSLog log;
				log.SetTitle( CVSTR("SymbolTable") );
				log.Append( CVSTR("BinaryVersion"),		kCurrentVersion );
				log.Append( CVSTR("SolutionVersion"),	version );
				log.Append( CVSTR("NextGenFlag"),		fUseNextGen);
				log.Print();
#endif
			}
			else
				ret = false;
			
			rec->Release();
		}
		else
			ret = false;
		
	}
	else
		ret = false;
	
	return ret;
}



bool VSymbolTable::InnerSymbolTable::DeleteDatabaseOnDisk( const VFile &inDatabaseFile )
{
	bool ret = true;
	if (inDatabaseFile.Exists())
	{
		ret &= (VE_OK == inDatabaseFile.Delete());
	}
	
	VFilePath path = inDatabaseFile.GetPath();
	path.SetExtension( RIAFileKind::kSymbolDataFileExtension );
	VFile dataFile( path );
	if (dataFile.Exists())
	{
		ret &= (VE_OK == dataFile.Delete());
	}
	
	return ret;
}



bool VSymbolTable::InnerSymbolTable::OpenDataFile( const VFile &inDatabaseFile, CDB4DBaseContext *inContext )
{
	VFilePath path = inDatabaseFile.GetPath();
	path.SetExtension( RIAFileKind::kSymbolDataFileExtension );
	VFile dataFile( path );
	
	bool ret = true;
	if (dataFile.Exists())
	{
		// We want to open the data file for the user
		if (!fDatabase->OpenData( dataFile, DB4D_Open_Convert_To_Higher_Version, inContext ))
			ret = false;
	}
	else
	{
		if (!fDatabase->CreateData( dataFile, 0, NULL, inContext ))
			ret = false;
	}
	
	return ret;
}



bool VSymbolTable::InnerSymbolTable::OpenSymbolDatabase( const VFile &inDatabaseFile, CDB4DBaseContext **outContext )
{
	if (!fDatabaseManager)
		return false;
	
	// We want to keep any errors previously on the stack, but we don't want to add any new ones, since
	// we handle failure cases internally and retry manually.
	StErrorContextInstaller errorFilter( false );
	
	// Check to see if there is a file that already exists at the path given.  If
	// there is one, we assume it is the database the user wants.  If there's not
	// one, then we need to create a new database for the caller.
	if (inDatabaseFile.Exists())
	{
		// The file exists, so let's open it
		fDatabase = fDatabaseManager->OpenBase( inDatabaseFile, DB4D_Open_Convert_To_Higher_Version );
		
		// Let's see if we were able to get a handle to the database
		if (fDatabase == NULL)
		{
			// We have a database, but we can't actually open it.  Let's see if we can delete
			// the database file and try again.
			if (DeleteDatabaseOnDisk( inDatabaseFile ))
			{
				return OpenSymbolDatabase( inDatabaseFile, outContext );
			}
			else
			{
				return false;
			}
		}

		*outContext = fDatabase->NewContext( NULL, NULL );
		if( !RetainTables(*outContext) )
		{
			ReleaseTables();
			(*outContext)->Release();
			fDatabase->CloseAndRelease();
			fDatabase = NULL;
			// We have a database, but we can't actually open it.  Let's see if we can delete
			// the database file and try again.
			if (DeleteDatabaseOnDisk( inDatabaseFile ))
			{
				return OpenSymbolDatabase( inDatabaseFile, outContext );
			}
			else
			{
				return false;
			}
		}
		
		RetainFields(*outContext);
		
		// The datafile needs to be opened before we can perform certain operations
		if (!OpenDataFile( inDatabaseFile, *outContext ) || !CheckRequiredStructure( *outContext ))
		{
			ReleaseFields();
			ReleaseTables();
			(*outContext)->Release();
			fDatabase->CloseAndRelease();
			fDatabase = NULL;
			
			// Let's try to recover by deleting the database and structure file both, and
			// starting over.
			if (DeleteDatabaseOnDisk( inDatabaseFile ))
			{
				return OpenSymbolDatabase( inDatabaseFile, outContext );
			}
			else
			{
				return false;
			}
		}

		while ( fDatabase->StillIndexing() )
		{
			VTask::Sleep( 20 );
		}

	}
	else
	{
		// The file doesn't exist, so let's make a new one
		fDatabase = fDatabaseManager->CreateBase( inDatabaseFile, DB4D_Create_AllDefaultParamaters, VIntlMgr::GetDefaultMgr() );
		
		// Let's see if we were able to get a handle to the database
		if (!fDatabase)
			return false;
		
		// Create a new context for this database
		*outContext = fDatabase->NewContext( NULL, NULL );
		
		// The datafile needs to be opened before we can perform certain operations
		if (!OpenDataFile( inDatabaseFile, *outContext ))
		{
			ReleaseFields();
			ReleaseTables();
			(*outContext)->Release();
			ReleaseRefCountable(&fDatabase);
			return false;
		}
		
		// We need to initialize this database so that it's structure is correct
		if( !InitializeNewDatabase( *outContext) )
		{
			ReleaseFields();
			ReleaseTables();
			(*outContext)->Release();
			ReleaseRefCountable(&fDatabase);
			return false;
		}
		else
		{

			while ( fDatabase->StillIndexing() )
			{
				VTask::Sleep( 20 );
			}

			if( fUseNextGen == true )
			{
#if ACTIVATE_JSLOG
				JSLog log(true);
				log.SetTitle("CoreSymbolsInserting");
#endif//ACTIVATE_JSLOG
				
				_InsertECMA262CoreSymbols(eSymbolFileExecContextClient, *outContext);
				_InsertECMA262CoreSymbols(eSymbolFileExecContextServer, *outContext);
				
#if ACTIVATE_JSLOG
				log.Print();
#endif//ACTIVATE_JSLOG
			}
		}
	}
	
	// Now that we've got a database opened, we're going to set the temp folder to the system's temp directory,
	// so that large databases do not trigger project refreshes constantly.
	fDatabase->SetTemporaryFolderSelector( DB4D_SystemTemporaryFolder );
	
	// We're going to take a look at the symbols table and try to find the greatest symbol
	// id in the table.  That will tell us our next unique symbol ID.  We default to zero in
	// the case where there are no records in the symbols table.
	if (fSymbolsTable)
	{
		fNextUniqueSymbolID = 0;
		CDB4DSelection *allRecords = fSymbolsTable->SelectAllRecords( *outContext );
		if (allRecords)
		{
			// We want the information in descending order so that we can load the first record as it will
			// have the max ID
			if (allRecords->CountRecordsInSelection( *outContext ) > 0)
			{
				if (fSymbolIDField)
				{
					allRecords->SortSelection( fSymbolIDField->GetID( *outContext ), false, NULL, *outContext );
					CDB4DRecord *record = allRecords->LoadSelectedRecord( 1, DB4D_Do_Not_Lock, *outContext );
					if (record)
					{
						fNextUniqueSymbolID = GetIntegerField( record, fSymbolIDField, NULL );
						record->Release();
					}
				}
			}
			allRecords->Release();
		}
	}
	
	return true;
}



bool VSymbolTable::InnerSymbolTable::InitializeNewDatabase( CDB4DBaseContext *inContext )
{
	xbox_assert( fDatabase );
	
	// We need to fill out the database structure since this is a brand-new database.
	// The database will have the following structure when we're done:
	//
	// Symbols
	//		UniqueID as Int32 (non-null, auto-increment, unique, primary key)
	//		Name as String
	//		FileID as Int32 (non-null)
	//		ScriptDocCommentText as Text
	//		SymbolKind as Int32
	//		SymbolWAFKind as String
	//		PrototypeID as String
	//		OwnerID as Int32
	//		LineNumber as Int32
	//		LineCompletionNumber as Int32
	//		ReturnTypeID as String
	//		ReferenceID as Int32
	//
	// Files
	//		UniqueID as Int32 (non-null, auto-increment, unique, primary key)
	//		Path as String (non-null)
	//		ModificationDate as Int64 (non-null)
	//
	// ExtraInfo
	//		OwnerID as Int32 (non-null, primary key)
	//		Kind as Int32 (non-null)
	//		StringData as String
	//		IntegerData as Int32
	//
	//	Version
	//		VersionNumber as Int32 (non-null)
	//
	// There are relations between the fields in the tables as well.  There is a
	// relation going from Files.UniqueID to Symbols.FileID so that the removal of
	// a file from the database will result in its associated symbols being removed
	// as well.  There is also a relation going from Symbols.OwnerID to
	// Symbols.UniqueID so that subsymbols can be removed when their owner symbol is
	// removed.  Note that prototypes do *not* have a relation to the unique ID.  That
	// is because a prototype can be removed without associated symbols being removed.
	// In the case where that happens, we need to manually set the any symbol whose
	// PrototypeID matches back to null (so that it no longer has a prototype instead
	// of referencing a prototype that doesn't exist).
	//
	// There is also a relationship between the extra information and symbol tables.
	// When a symbol is removed, all of the extra information owned by that symbol is
	// also removed at the same time.
	if( fUseNextGen == true )
	{
		fScopesTable = fDatabase->CreateOutsideTable( kScopesTable, inContext);
		if( fScopesTable == NULL )
			return false;
		
		fSymbolNamesTable = fDatabase->CreateOutsideTable( kSymbolNamesTable, inContext);
		if( fSymbolNamesTable == NULL )
			return false;
		
		fSymbolDefinitionsTable = fDatabase->CreateOutsideTable( kSymbolDefinitionsTable, inContext);
		if( fSymbolDefinitionsTable == NULL )
			return false;
		
		fSymbolPrototypesTable = fDatabase->CreateOutsideTable( kSymbolPrototypesTable, inContext);
		if( fSymbolPrototypesTable == NULL )
			return false;
		
		fSymbolReturnsTable = fDatabase->CreateOutsideTable( kSymbolReturnsTable, inContext);
		if( fSymbolReturnsTable == NULL )
			return false;
		
		fSymbolTypesTable = fDatabase->CreateOutsideTable( kSymbolTypesTable, inContext);
		if( fSymbolTypesTable == NULL )
			return false;
		
		fSymbolReferencesTable = fDatabase->CreateOutsideTable( kSymbolReferencesTable, inContext);
		if( fSymbolReferencesTable == NULL )
			return false;
		
		fOutlineItemsTable = fDatabase->CreateOutsideTable( kOutlineItemsTable, inContext);
		if( fOutlineItemsTable == NULL )
			return false;
	}

	fSymbolsTable = fDatabase->CreateOutsideTable( kSymbolTable, inContext );
	if (!fSymbolsTable)
		return false;
	
	fFilesTable = fDatabase->CreateOutsideTable( kFileTable, inContext );
	if (!fFilesTable)
	{
		ReleaseTables();
		return false;
	}
	
	fExtrasTable = fDatabase->CreateOutsideTable( kExtrasTable, inContext );
	if (!fExtrasTable)
	{
		ReleaseTables();
		return false;
	}
	
	fVersionTable = fDatabase->CreateOutsideTable( kVersionTable, inContext );
	if (!fVersionTable)
	{
		ReleaseTables();
		return false;
	}
	
	int status = 0x1;
		
	// We only want to test for uniqueness when in a debug build -- in theory, this is something
	// we can assure without requiring the DB to check for us.  So release builds will turn this off.
#if _DEBUG
	const int kUnique = DB4D_Unique;
#else
	const int kUnique = 0;
#endif
	
	if( fUseNextGen == true )
	{
		status &= (VE_OK == fScopesTable->AddField( kScopeFieldId,					DB4D_Integer32, -1, DB4D_Not_Null | kUnique| DB4D_AutoSeq,	NULL, inContext ));
		status &= (VE_OK == fScopesTable->AddField( kScopeFieldName,				DB4D_StrFix,	-1, 0,										NULL, inContext ));
		status &= (VE_OK == fScopesTable->AddField( kScopeFieldFullName,			DB4D_StrFix,	-1, 0,										NULL, inContext ));
		status &= (VE_OK == fScopesTable->AddField( kScopeFieldType,				DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fScopesTable->AddField( kScopeFieldSourceId,			DB4D_Integer32,	-1, 0,										NULL, inContext ));
		status &= (VE_OK == fScopesTable->AddField( kScopeFieldExecutionContext,	DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fScopesTable->AddField( kScopeFieldParentId,			DB4D_Integer32, -1, 0,										NULL, inContext ));
		status &= (VE_OK == fScopesTable->AddField( kScopeFieldStatus,				DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fScopesTable->AddField( kScopeFieldFromLine,			DB4D_Integer32, -1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fScopesTable->AddField( kScopeFieldToLine,				DB4D_Integer32, -1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fScopesTable->AddField( kScopeFieldFromOffset,			DB4D_Integer32, -1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fScopesTable->AddField( kScopeFieldToOffset,			DB4D_Integer32, -1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fScopesTable->AddField( kScopeFieldConstructorFlag,		DB4D_Boolean,	-1, DB4D_Not_Null,							NULL, inContext ));
		
		status &= (VE_OK == fSymbolNamesTable->AddField( kSymbolNameFieldId,				DB4D_Integer32, -1, DB4D_Not_Null | kUnique| DB4D_AutoSeq,	NULL, inContext ));
		status &= (VE_OK == fSymbolNamesTable->AddField( kSymbolNameFieldSourceId,			DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolNamesTable->AddField( kSymbolNameFieldValue,				DB4D_StrFix,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolNamesTable->AddField( kSymbolNameFieldScopeId,			DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolNamesTable->AddField( kSymbolNameFieldExecutionContext,	DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolNamesTable->AddField( kSymbolNameFieldStatus,			DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		
		status &= (VE_OK == fSymbolDefinitionsTable->AddField( kSymbolDefinitionFieldId,				DB4D_Integer32, -1, DB4D_Not_Null | kUnique| DB4D_AutoSeq,	NULL, inContext ));
		status &= (VE_OK == fSymbolDefinitionsTable->AddField( kSymbolDefinitionFieldScopeId,			DB4D_Integer32, -1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolDefinitionsTable->AddField( kSymbolDefinitionFieldNameId,			DB4D_Integer32, -1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolDefinitionsTable->AddField( kSymbolDefinitionFieldKind,				DB4D_Integer32, -1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolDefinitionsTable->AddField( kSymbolDefinitionFieldExecutionContext,	DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolDefinitionsTable->AddField( kSymbolDefinitionFieldStatus,			DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolDefinitionsTable->AddField( kSymbolDefinitionFieldSourceId,			DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolDefinitionsTable->AddField( kSymbolDefinitionFieldLineNumber,		DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		
		status &= (VE_OK == fSymbolPrototypesTable->AddField( kSymbolPrototypeFieldId,					DB4D_Integer32, -1, DB4D_Not_Null | kUnique| DB4D_AutoSeq,	NULL, inContext ));
		status &= (VE_OK == fSymbolPrototypesTable->AddField( kSymbolPrototypeFieldFromName,			DB4D_StrFix,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolPrototypesTable->AddField( kSymbolPrototypeFieldToName,				DB4D_StrFix,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolPrototypesTable->AddField( kSymbolPrototypeFieldExecutionContext,	DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolPrototypesTable->AddField( kSymbolPrototypeFieldStatus,				DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolPrototypesTable->AddField( kSymbolPrototypeFieldSourceId,			DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		
		status &= (VE_OK == fSymbolReturnsTable->AddField( kSymbolReturnFieldId,		DB4D_Integer32, -1, DB4D_Not_Null | kUnique| DB4D_AutoSeq,	NULL, inContext ));
		status &= (VE_OK == fSymbolReturnsTable->AddField( kSymbolReturnFieldScopeId,	DB4D_Integer32, -1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolReturnsTable->AddField( kSymbolReturnFieldName,		DB4D_StrFix,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolReturnsTable->AddField( kSymbolReturnFieldStatus,	DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolReturnsTable->AddField( kSymbolReturnFieldSourceId,	DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		
		status &= (VE_OK == fSymbolTypesTable->AddField( kSymbolTypeFieldId,			DB4D_Integer32, -1, DB4D_Not_Null | kUnique| DB4D_AutoSeq,	NULL, inContext ));
		status &= (VE_OK == fSymbolTypesTable->AddField( kSymbolTypeFieldDefinitionId,	DB4D_Integer32, -1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolTypesTable->AddField( kSymbolTypeFieldName,			DB4D_StrFix,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolTypesTable->AddField( kSymbolTypeFieldStatus,		DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolTypesTable->AddField( kSymbolTypeFieldSourceId,		DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));

		status &= (VE_OK == fSymbolReferencesTable->AddField( kSymbolReferenceFieldId,			DB4D_Integer32, -1, DB4D_Not_Null | kUnique| DB4D_AutoSeq,	NULL, inContext ));
		status &= (VE_OK == fSymbolReferencesTable->AddField( kSymbolReferenceFieldType,		DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolReferencesTable->AddField( kSymbolReferenceFieldFromId,		DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolReferencesTable->AddField( kSymbolReferenceFieldToId,		DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolReferencesTable->AddField( kSymbolReferenceFieldStatus,		DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fSymbolReferencesTable->AddField( kSymbolReferenceFieldSourceId,	DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));

		status &= (VE_OK == fOutlineItemsTable->AddField( kOutlineItemFieldId,			DB4D_Integer32, -1, DB4D_Not_Null | kUnique| DB4D_AutoSeq,	NULL, inContext ));
		status &= (VE_OK == fOutlineItemsTable->AddField( kOutlineItemFieldSourceId,	DB4D_Integer32, -1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fOutlineItemsTable->AddField( kOutlineItemFieldName,		DB4D_StrFix,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fOutlineItemsTable->AddField( kOutlineItemFieldLine,		DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
		status &= (VE_OK == fOutlineItemsTable->AddField( kOutlineItemFieldStatus,		DB4D_Integer32,	-1, DB4D_Not_Null,							NULL, inContext ));
	}
	
	// Now we can fill out the fields of the symbol table
	status &= (VE_OK == fSymbolsTable->AddField( kSymbolName,			DB4D_StrFix,	-1, 0,							NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kSymbolID,				DB4D_Integer32, -1, DB4D_Not_Null | kUnique,	NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kSymbolFileID,			DB4D_Integer32, -1, DB4D_Not_Null,				NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kSymbolScriptDocText,	DB4D_StrFix,	-1, 0,							NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kSymbolKind,			DB4D_Integer32, -1, 0,							NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kSymbolWAFKind,		DB4D_StrFix,	-1, 0,							NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kSymbolPrototypeID,	DB4D_StrFix,	-1, 0,							NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kSymbolOwnerID,		DB4D_Integer32, -1, 0,							NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kLineNumber,			DB4D_Integer32, -1, 0,							NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kLineCompletionNumber, DB4D_Integer32, -1, 0,							NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kSymbolReturnTypeID,	DB4D_StrFix,	-1, 0,							NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kSymbolReferenceID,	DB4D_Integer32, -1, 0,							NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kSymbolUndefinedState,	DB4D_Boolean,	-1, 0,							NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kSymbolInstanceState,	DB4D_Boolean,	-1, 0,							NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kSymbolReferenceState,	DB4D_Boolean,	-1, 0,							NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kSymbolEditionState,	DB4D_Boolean,	-1, 0,							NULL, inContext ));
	status &= (VE_OK == fSymbolsTable->AddField( kSymbolFullName ,		DB4D_StrFix,	-1, 0,							NULL, inContext ));
		
		
	// We can do the same for the file table
	status &= (VE_OK == fFilesTable->AddField( kFileID,					DB4D_Integer32, -1, DB4D_Not_Null | kUnique | DB4D_AutoSeq, NULL, inContext ));
	status &= (VE_OK == fFilesTable->AddField( kFilePath,				DB4D_StrFix,	-1, DB4D_Not_Null,							NULL, inContext ));
	status &= (VE_OK == fFilesTable->AddField( kFileBaseFolder,			DB4D_Integer16, -1, DB4D_Not_Null,							NULL, inContext ));
	status &= (VE_OK == fFilesTable->AddField( kFileExecContext,		DB4D_Integer16, -1, DB4D_Not_Null,							NULL, inContext ));
	status &= (VE_OK == fFilesTable->AddField( kFileModificationTime,	DB4D_Integer64, -1, DB4D_Not_Null,							NULL, inContext ));
	status &= (VE_OK == fFilesTable->AddField( kFileResolvedState,		DB4D_Boolean,	-1,	DB4D_Not_Null,							NULL, inContext ));
		
	// And the extras table
	status &= (VE_OK == fExtrasTable->AddField( kExtraID,				DB4D_Integer32, -1, DB4D_Not_Null | kUnique | DB4D_AutoSeq, NULL, inContext ));
	status &= (VE_OK == fExtrasTable->AddField( kExtrasOwnerSymbolID,	DB4D_Integer32, -1, DB4D_Not_Null,							NULL, inContext ));
	status &= (VE_OK == fExtrasTable->AddField( kExtrasKind,			DB4D_Integer32, -1, DB4D_Not_Null,							NULL, inContext ));
	status &= (VE_OK == fExtrasTable->AddField( kExtrasStringData,		DB4D_StrFix,	-1, 0,										NULL, inContext ));
	status &= (VE_OK == fExtrasTable->AddField( kExtrasIntegerData,		DB4D_Integer32, -1, 0,										NULL, inContext ));
		
	// And the version table
	status &= (VE_OK == fVersionTable->AddField( kVersionNumber, DB4D_Integer32, -1, DB4D_Not_Null, NULL, inContext ));
		
	RetainFields(inContext);	
		
	if (status)
	{
		// Add the tables to the database now
		if( fUseNextGen == true )
		{
			status &= (VE_OK == fDatabase->AddTable( fScopesTable,				inContext ));
			status &= (VE_OK == fDatabase->AddTable( fSymbolNamesTable,			inContext ));
			status &= (VE_OK == fDatabase->AddTable( fSymbolDefinitionsTable,	inContext ));
			status &= (VE_OK == fDatabase->AddTable( fSymbolPrototypesTable,	inContext ));
			status &= (VE_OK == fDatabase->AddTable( fSymbolReturnsTable,		inContext ));
			status &= (VE_OK == fDatabase->AddTable( fSymbolTypesTable,			inContext ));
			status &= (VE_OK == fDatabase->AddTable( fSymbolReferencesTable,	inContext ));
			status &= (VE_OK == fDatabase->AddTable( fOutlineItemsTable,		inContext ));
		}
		status &= (VE_OK == fDatabase->AddTable( fSymbolsTable, inContext ));
		status &= (VE_OK == fDatabase->AddTable( fFilesTable,	inContext ));
		status &= (VE_OK == fDatabase->AddTable( fExtrasTable,	inContext ));
		status &= (VE_OK == fDatabase->AddTable( fVersionTable, inContext ));
	}
				
	// Set the version number for the table
	CDB4DRecord* rec = fVersionTable->NewRecord( inContext );
	rec->SetLong( fVersionNumberField, kCurrentVersion );
	rec->Save();
	rec->Release();
	
	// Now we want to set up primary keys
	// Because of the way we clean up everything, we need this to use its own
	// block scope.  This is purely to ensure that the primaryKeyArray is
	// initialized and finalized properly.
	CDB4DFieldArray primaryKeyArray;
	
	if( fUseNextGen == true )
	{
		primaryKeyArray.Add( fScopeFieldId );
		status &= (VE_OK == fScopesTable->SetPrimaryKey( primaryKeyArray, NULL, false, inContext ));
		primaryKeyArray.Delete( fScopeFieldId );
		
		primaryKeyArray.Add( fSymbolNameFieldId );
		status &= (VE_OK == fSymbolNamesTable->SetPrimaryKey( primaryKeyArray, NULL, false, inContext ));
		primaryKeyArray.Delete( fSymbolNameFieldId );
		
		primaryKeyArray.Add( fSymbolDefinitionFieldId );
		status &= (VE_OK == fSymbolDefinitionsTable->SetPrimaryKey( primaryKeyArray, NULL, false, inContext ));
		primaryKeyArray.Delete( fSymbolDefinitionFieldId );
		
		primaryKeyArray.Add( fSymbolPrototypeFieldId );
		status &= (VE_OK == fSymbolPrototypesTable->SetPrimaryKey( primaryKeyArray, NULL, false, inContext ));
		primaryKeyArray.Delete( fSymbolPrototypeFieldId );
		
		primaryKeyArray.Add( fSymbolReturnFieldId );
		status &= (VE_OK == fSymbolReturnsTable->SetPrimaryKey( primaryKeyArray, NULL, false, inContext ));
		primaryKeyArray.Delete( fSymbolReturnFieldId );
		
		primaryKeyArray.Add( fSymbolTypeFieldId );
		status &= (VE_OK == fSymbolTypesTable->SetPrimaryKey( primaryKeyArray, NULL, false, inContext ));
		primaryKeyArray.Delete( fSymbolTypeFieldId );
		
		primaryKeyArray.Add( fSymbolReferenceFieldId );
		status &= (VE_OK == fSymbolReferencesTable->SetPrimaryKey( primaryKeyArray, NULL, false, inContext ));
		primaryKeyArray.Delete( fSymbolReferenceFieldId );
		
		primaryKeyArray.Add( fOutlineItemFieldId );
		status &= (VE_OK == fOutlineItemsTable->SetPrimaryKey( primaryKeyArray, NULL, false, inContext ));
		primaryKeyArray.Delete( fOutlineItemFieldId );
	}

	primaryKeyArray.Add( fSymbolIDField );
	status &= (VE_OK == fSymbolsTable->SetPrimaryKey( primaryKeyArray, NULL, false, inContext ));
	primaryKeyArray.Delete( fSymbolIDField );
		
	primaryKeyArray.Add( fFileIDField );
	status &= (VE_OK == fFilesTable->SetPrimaryKey( primaryKeyArray, NULL, false, inContext ));
	primaryKeyArray.Delete( fFileIDField );

	primaryKeyArray.Add( fExtraIDField );
	status &= (VE_OK == fExtrasTable->SetPrimaryKey( primaryKeyArray, NULL, false, inContext ));
	primaryKeyArray.Delete( fExtraIDField );

	primaryKeyArray.Add( fVersionNumberField );
	status &= (VE_OK == fVersionTable->SetPrimaryKey( primaryKeyArray, NULL, false, inContext ));
	primaryKeyArray.Delete( fVersionNumberField );

	// Now that we've set up some primary keys, let's set up some relations!
	CDB4DRelation *fileToSymbolRelation = fDatabase->CreateRelation( CVSTR( "FileToSymbol" ), CVSTR( "FileToSymbolM1R" ), fSymbolFileIDField, fFileIDField, inContext );
	status &= (VE_OK == fileToSymbolRelation->SetForeignKey( true, inContext ));
	status &= (VE_OK == fileToSymbolRelation->SetReferentialIntegrity( true, true, inContext ));
	fileToSymbolRelation->Release();

		
	// We also want a relation going from a symbol's OwnerID to its UniqueID so that the deletion of an
	// owner causes the child to be deleted as well.
	CDB4DRelation *ownerToChildRelation = fDatabase->CreateRelation( CVSTR( "OwnerToChild" ), CVSTR( "OwnerToChildM1R" ), fSymbolOwnerIDField, fSymbolIDField, inContext );
	status &= (VE_OK == ownerToChildRelation->SetForeignKey( true, inContext ));
	status &= (VE_OK == ownerToChildRelation->SetReferentialIntegrity( true, true, inContext ));
	ownerToChildRelation->Release();

		
	// And there's a relationship between a symbol and its extra information
	CDB4DRelation *symbolToExtraInfoRelation = fDatabase->CreateRelation( CVSTR( "SymbolToExtraInfo" ), CVSTR( "SymbolToExtraInfoM1R" ), fExtrasOwnerSymbolIDField, fSymbolIDField, inContext );
	status &= (VE_OK == symbolToExtraInfoRelation->SetForeignKey( true, inContext ));
	status &= (VE_OK == symbolToExtraInfoRelation->SetReferentialIntegrity( true, true, inContext ));
	symbolToExtraInfoRelation->Release();

	
	// Initialize some indexes to make searching go significantly faster
	CDB4DIndex* index	= NULL;
	VString indexName;
		
	indexName = "SymbolIDIndex";
	status &= (VE_OK == fDatabase->CreateIndexOnOneField( fSymbolIDField,			DB4D_Index_DefaultType, true,	NULL, &indexName, &index, true, NULL, inContext ));
	ReleaseRefCountable(&index);
		
	indexName = "FileIDIndex";
	status &= (VE_OK == fDatabase->CreateIndexOnOneField( fFileIDField,				DB4D_Index_DefaultType, true,	NULL, &indexName, &index, true, NULL, inContext ));
	ReleaseRefCountable(&index);

	indexName = "SymbolNameIndex";
	status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolNameField, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
	ReleaseRefCountable(&index);
		
	indexName = "SymbolLineNumberIndex";
	status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolLineNumberField, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
	ReleaseRefCountable(&index);
		
	indexName = "SymbolKindIndex";
	status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolKindField, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
	ReleaseRefCountable(&index);

	indexName = "FilePathIndex";
	status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fFilePathField, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
	ReleaseRefCountable(&index);
		
	indexName = "SymbolOwnerIndex";
	status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolOwnerIDField, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
	ReleaseRefCountable(&index);

	indexName = "SymbolPrototypeIndex";
	status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolPrototypeIDField, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
	ReleaseRefCountable(&index);

	indexName = "SymbolReferenceIndex";
	status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolReferenceIDField, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
	ReleaseRefCountable(&index);
	
	indexName = "SymbolReturnTypeIndex";
	status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolReturnTypeIDField, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
	ReleaseRefCountable(&index);
	
	indexName = "SymbolExtraIDFieldIndex";
	status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fExtraIDField, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
	ReleaseRefCountable(&index);

	if( fUseNextGen == true )
	{
		indexName = "ScopeFieldIdIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fScopeFieldId, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "ScopeFieldNameIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fScopeFieldName, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "ScopeFieldSourceIdIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fScopeFieldSourceId, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "ScopeFieldConstructorFlagIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fScopeFieldConstructorFlag, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "SymbolNameFieldIdIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolNameFieldId, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "SymbolNameFieldValueIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolNameFieldValue, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "SymbolNameFieldSourceIdIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolNameFieldSourceId, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "SymbolDefinitionFieldIdIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolDefinitionFieldId, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "SymbolDefinitionFieldScopeIdIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolDefinitionFieldScopeId, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "SymbolDefinitionFieldSourceIdIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolDefinitionFieldSourceId, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "SymbolPrototypeFieldIdIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolPrototypeFieldId, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "SymbolPrototypeFieldSourceIdIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolPrototypeFieldSourceId, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "SymbolReturnFieldIdIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolReturnFieldId, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "SymbolReturnFieldSourceIdIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolReturnFieldSourceId, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "SymbolTypeFieldIdIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolTypeFieldId, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "SymbolTypeFieldSourceIdIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolTypeFieldSourceId, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable(&index);
		
		indexName = "SymbolReferenceFieldIdIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fSymbolReferenceFieldId, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable( &index );
		
		indexName = "OutlineItemFieldIdIndex";
		status &= ( VE_OK == fDatabase->CreateIndexOnOneField( fOutlineItemFieldId, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ) );
		ReleaseRefCountable( &index );
	}
	
	return status != 0;
}



bool VSymbolTable::InnerSymbolTable::RetainTables( CDB4DBaseContext *inContext )
{
	bool res = false;
	
	if( fUseNextGen == true )
	{
		fScopesTable			=	fDatabase->FindAndRetainTable(kScopesTable);
		fSymbolNamesTable		=	fDatabase->FindAndRetainTable(kSymbolNamesTable);
		fSymbolDefinitionsTable	=	fDatabase->FindAndRetainTable(kSymbolDefinitionsTable);
		fSymbolPrototypesTable	=	fDatabase->FindAndRetainTable(kSymbolPrototypesTable);
		fSymbolReturnsTable		=	fDatabase->FindAndRetainTable(kSymbolReturnsTable);
		fSymbolTypesTable		=	fDatabase->FindAndRetainTable(kSymbolTypesTable);
		fSymbolReferencesTable	=	fDatabase->FindAndRetainTable(kSymbolReferencesTable);
		fOutlineItemsTable		=	fDatabase->FindAndRetainTable(kOutlineItemsTable);
	}

	fSymbolsTable	=	fDatabase->FindAndRetainTable(kSymbolTable);
	fFilesTable		=	fDatabase->FindAndRetainTable(kFileTable);
	fExtrasTable	=	fDatabase->FindAndRetainTable(kExtrasTable);
	fVersionTable	=	fDatabase->FindAndRetainTable(kVersionTable);
	
	if( fUseNextGen == true )
	{
		res = (fScopesTable!= NULL && fSymbolNamesTable != NULL && fSymbolDefinitionsTable != NULL && fSymbolPrototypesTable != NULL && fSymbolReturnsTable != NULL && fSymbolTypesTable != NULL && fSymbolReferencesTable != NULL && fOutlineItemsTable != NULL && fSymbolsTable != NULL && fFilesTable != NULL && fExtrasTable != NULL && fVersionTable != NULL);
	}
	else
	{
		res = (fSymbolsTable != NULL && fFilesTable != NULL && fExtrasTable != NULL && fVersionTable != NULL);
	}

	return res;
}



void VSymbolTable::InnerSymbolTable::ReleaseTables()
{
	if( fUseNextGen == true )
	{
		ReleaseRefCountable(&fScopesTable);
		ReleaseRefCountable(&fSymbolNamesTable);
		ReleaseRefCountable(&fSymbolDefinitionsTable);
		ReleaseRefCountable(&fSymbolPrototypesTable);
		ReleaseRefCountable(&fSymbolReturnsTable);
		ReleaseRefCountable(&fSymbolTypesTable);
		ReleaseRefCountable(&fSymbolReferencesTable);
		ReleaseRefCountable(&fOutlineItemsTable);
	}

	ReleaseRefCountable(&fSymbolsTable);
	ReleaseRefCountable(&fFilesTable);
	ReleaseRefCountable(&fExtrasTable);
	ReleaseRefCountable(&fVersionTable);
}



void VSymbolTable::InnerSymbolTable::RetainFields( CDB4DBaseContext *inContext )
{
	if( fUseNextGen == true )
	{
		fScopeFieldId					=	fScopesTable->FindAndRetainField( kScopeFieldId,				inContext);
		fScopeFieldName					=	fScopesTable->FindAndRetainField( kScopeFieldName,				inContext);
		fScopeFieldFullName				=	fScopesTable->FindAndRetainField( kScopeFieldFullName,			inContext);
		fScopeFieldType					=	fScopesTable->FindAndRetainField( kScopeFieldType,				inContext);
		fScopeFieldSourceId				=	fScopesTable->FindAndRetainField( kScopeFieldSourceId,			inContext);
		fScopeFieldExecutionContext		=	fScopesTable->FindAndRetainField( kScopeFieldExecutionContext,	inContext);
		fScopeFieldParentId				=	fScopesTable->FindAndRetainField( kScopeFieldParentId,			inContext);
		fScopeFieldStatus				=	fScopesTable->FindAndRetainField( kScopeFieldStatus,			inContext);
		fScopeFieldFromLine				=	fScopesTable->FindAndRetainField( kScopeFieldFromLine,			inContext);
		fScopeFieldToLine				=	fScopesTable->FindAndRetainField( kScopeFieldToLine,			inContext);
		fScopeFieldFromOffset			=	fScopesTable->FindAndRetainField( kScopeFieldFromOffset,		inContext);
		fScopeFieldToOffset				=	fScopesTable->FindAndRetainField( kScopeFieldToOffset,			inContext);
		fScopeFieldConstructorFlag		=	fScopesTable->FindAndRetainField( kScopeFieldConstructorFlag,	inContext);
		
		fSymbolNameFieldId					=	fSymbolNamesTable->FindAndRetainField( kSymbolNameFieldId,					inContext);
		fSymbolNameFieldSourceId			=	fSymbolNamesTable->FindAndRetainField( kSymbolNameFieldSourceId,			inContext);
		fSymbolNameFieldValue				=	fSymbolNamesTable->FindAndRetainField( kSymbolNameFieldValue,				inContext);
		fSymbolNameFieldScopeId				=	fSymbolNamesTable->FindAndRetainField( kSymbolNameFieldScopeId,				inContext);
		fSymbolNameFieldExecutionContext	=	fSymbolNamesTable->FindAndRetainField( kSymbolNameFieldExecutionContext,	inContext);
		fSymbolNameFieldStatus				=	fSymbolNamesTable->FindAndRetainField( kSymbolNameFieldStatus,				inContext);
		
		fSymbolDefinitionFieldId				=	fSymbolDefinitionsTable->FindAndRetainField( kSymbolDefinitionFieldId,					inContext);
		fSymbolDefinitionFieldScopeId			=	fSymbolDefinitionsTable->FindAndRetainField( kSymbolDefinitionFieldScopeId,				inContext);
		fSymbolDefinitionFieldNameId			=	fSymbolDefinitionsTable->FindAndRetainField( kSymbolDefinitionFieldNameId,				inContext);
		fSymbolDefinitionFieldKind				=	fSymbolDefinitionsTable->FindAndRetainField( kSymbolDefinitionFieldKind,				inContext);
		fSymbolDefinitionFieldExecutionContext	=	fSymbolDefinitionsTable->FindAndRetainField( kSymbolDefinitionFieldExecutionContext,	inContext);
		fSymbolDefinitionFieldStatus			=	fSymbolDefinitionsTable->FindAndRetainField( kSymbolDefinitionFieldStatus,				inContext);
		fSymbolDefinitionFieldSourceId			=	fSymbolDefinitionsTable->FindAndRetainField( kSymbolDefinitionFieldSourceId,			inContext);
		fSymbolDefinitionFieldLineNumber		=	fSymbolDefinitionsTable->FindAndRetainField( kSymbolDefinitionFieldLineNumber,			inContext);
		
		fSymbolPrototypeFieldId					=	fSymbolPrototypesTable->FindAndRetainField(	kSymbolPrototypeFieldId,				inContext);
		fSymbolPrototypeFieldFromName			=	fSymbolPrototypesTable->FindAndRetainField(	kSymbolPrototypeFieldFromName,			inContext);
		fSymbolPrototypeFieldToName				=	fSymbolPrototypesTable->FindAndRetainField(	kSymbolPrototypeFieldToName,			inContext);
		fSymbolPrototypeFieldExecutionContext	=	fSymbolPrototypesTable->FindAndRetainField( kSymbolPrototypeFieldExecutionContext,	inContext);
		fSymbolPrototypeFieldStatus				=	fSymbolPrototypesTable->FindAndRetainField( kSymbolPrototypeFieldStatus,			inContext);
		fSymbolPrototypeFieldSourceId			=	fSymbolPrototypesTable->FindAndRetainField( kSymbolPrototypeFieldSourceId,			inContext);
		
		fSymbolReturnFieldId		=	fSymbolReturnsTable->FindAndRetainField( kSymbolReturnFieldId,			inContext);
		fSymbolReturnFieldScopeId	=	fSymbolReturnsTable->FindAndRetainField( kSymbolReturnFieldScopeId,		inContext);
		fSymbolReturnFieldName		=	fSymbolReturnsTable->FindAndRetainField( kSymbolReturnFieldName,		inContext);
		fSymbolReturnFieldStatus	=	fSymbolReturnsTable->FindAndRetainField( kSymbolReturnFieldStatus,		inContext);
		fSymbolReturnFieldSourceId	=	fSymbolReturnsTable->FindAndRetainField( kSymbolReturnFieldSourceId,	inContext);
		
		fSymbolTypeFieldId				=	fSymbolTypesTable->FindAndRetainField( kSymbolTypeFieldId,				inContext);
		fSymbolTypeFieldDefinitionId	=	fSymbolTypesTable->FindAndRetainField( kSymbolTypeFieldDefinitionId,	inContext);
		fSymbolTypeFieldName			=	fSymbolTypesTable->FindAndRetainField( kSymbolTypeFieldName,			inContext);
		fSymbolTypeFieldStatus			=	fSymbolTypesTable->FindAndRetainField( kSymbolTypeFieldStatus,			inContext);
		fSymbolTypeFieldSourceId		=	fSymbolTypesTable->FindAndRetainField( kSymbolTypeFieldSourceId,		inContext);

		fSymbolReferenceFieldId			=	fSymbolReferencesTable->FindAndRetainField(	kSymbolReferenceFieldId,		inContext);
		fSymbolReferenceFieldType		=	fSymbolReferencesTable->FindAndRetainField( kSymbolReferenceFieldType,		inContext);
		fSymbolReferenceFieldFromId		=	fSymbolReferencesTable->FindAndRetainField(	kSymbolReferenceFieldFromId,	inContext);
		fSymbolReferenceFieldToId		=	fSymbolReferencesTable->FindAndRetainField(	kSymbolReferenceFieldToId,		inContext);
		fSymbolReferenceFieldStatus		=	fSymbolReferencesTable->FindAndRetainField( kSymbolReferenceFieldStatus,	inContext);
		fSymbolReferenceFieldSourceId	=	fSymbolReferencesTable->FindAndRetainField( kSymbolReferenceFieldSourceId,	inContext);

		fOutlineItemFieldId			=	fOutlineItemsTable->FindAndRetainField(	kOutlineItemFieldId,		inContext);
		fOutlineItemFieldSourceId	=	fOutlineItemsTable->FindAndRetainField( kOutlineItemFieldSourceId,	inContext);
		fOutlineItemFieldName		=	fOutlineItemsTable->FindAndRetainField( kOutlineItemFieldName,		inContext);
		fOutlineItemFieldLine		=	fOutlineItemsTable->FindAndRetainField( kOutlineItemFieldLine,		inContext);
		fOutlineItemFieldStatus		=	fOutlineItemsTable->FindAndRetainField( kOutlineItemFieldStatus,	inContext);
	}

	fSymbolNameField					=	fSymbolsTable->FindAndRetainField( kSymbolName, inContext );
	fSymbolIDField						=	fSymbolsTable->FindAndRetainField( kSymbolID, inContext );
	fSymbolFileIDField					=	fSymbolsTable->FindAndRetainField( kSymbolFileID, inContext );
	fSymbolScriptDocTextField			=	fSymbolsTable->FindAndRetainField( kSymbolScriptDocText, inContext );
	fSymbolKindField					=	fSymbolsTable->FindAndRetainField( kSymbolKind, inContext );
	fSymbolWAFKindField					=	fSymbolsTable->FindAndRetainField( kSymbolWAFKind, inContext );
	fSymbolPrototypeIDField				=	fSymbolsTable->FindAndRetainField( kSymbolPrototypeID, inContext );
	fSymbolOwnerIDField					=	fSymbolsTable->FindAndRetainField( kSymbolOwnerID, inContext );
	fSymbolLineNumberField				=	fSymbolsTable->FindAndRetainField( kLineNumber, inContext );
	fSymbolLineCompletionNumberField	=	fSymbolsTable->FindAndRetainField( kLineCompletionNumber, inContext );
	fSymbolReturnTypeIDField			=	fSymbolsTable->FindAndRetainField( kSymbolReturnTypeID, inContext );
	fSymbolReferenceIDField				=	fSymbolsTable->FindAndRetainField( kSymbolReferenceID, inContext );
	fSymbolUndefinedStateField			=	fSymbolsTable->FindAndRetainField( kSymbolUndefinedState, inContext );
	fSymbolInstanceStateField			=	fSymbolsTable->FindAndRetainField( kSymbolInstanceState, inContext );
	fSymbolReferenceStateField			=	fSymbolsTable->FindAndRetainField( kSymbolReferenceState, inContext );
	fSymbolEditionStateField			=	fSymbolsTable->FindAndRetainField( kSymbolEditionState, inContext );
	fSymbolFullNameField				=	fSymbolsTable->FindAndRetainField( kSymbolFullName , inContext );
	
	fFileIDField				=	fFilesTable->FindAndRetainField( kFileID, inContext );
	fFilePathField				=	fFilesTable->FindAndRetainField( kFilePath, inContext );
	fFileBaseFolderField		=	fFilesTable->FindAndRetainField( kFileBaseFolder, inContext );
	fFileExecContextField		=	fFilesTable->FindAndRetainField( kFileExecContext, inContext );
	fFileModificationTimeField	=	fFilesTable->FindAndRetainField( kFileModificationTime, inContext );
	fFileResolvedStateField		=	fFilesTable->FindAndRetainField( kFileResolvedState, inContext );
	
	fExtrasOwnerSymbolIDField	=	fExtrasTable->FindAndRetainField( kExtrasOwnerSymbolID, inContext );
	fExtraIDField				=	fExtrasTable->FindAndRetainField( kExtraID, inContext );
	fExtrasKindField			=	fExtrasTable->FindAndRetainField( kExtrasKind, inContext );
	fExtrasStringDataField		=	fExtrasTable->FindAndRetainField( kExtrasStringData, inContext );
	fExtrasIntegerDataField		=	fExtrasTable->FindAndRetainField( kExtrasIntegerData, inContext );
	
	fVersionNumberField			=	fVersionTable->FindAndRetainField( kVersionNumber, inContext );
}



void VSymbolTable::InnerSymbolTable::ReleaseFields()
{
	if( fUseNextGen == true )
	{
		ReleaseRefCountable(&fScopeFieldId);
		ReleaseRefCountable(&fScopeFieldName);
		ReleaseRefCountable(&fScopeFieldFullName);
		ReleaseRefCountable(&fScopeFieldType);
		ReleaseRefCountable(&fScopeFieldSourceId);
		ReleaseRefCountable(&fScopeFieldExecutionContext);
		ReleaseRefCountable(&fScopeFieldParentId);
		ReleaseRefCountable(&fScopeFieldStatus);
		ReleaseRefCountable(&fScopeFieldFromLine);
		ReleaseRefCountable(&fScopeFieldToLine);
		ReleaseRefCountable(&fScopeFieldFromOffset);
		ReleaseRefCountable(&fScopeFieldToOffset);
		ReleaseRefCountable(&fScopeFieldConstructorFlag);
		
		ReleaseRefCountable(&fSymbolNameFieldId);
		ReleaseRefCountable(&fSymbolNameFieldSourceId);
		ReleaseRefCountable(&fSymbolNameFieldValue);
		ReleaseRefCountable(&fSymbolNameFieldScopeId);
		ReleaseRefCountable(&fSymbolNameFieldExecutionContext);
		ReleaseRefCountable(&fSymbolNameFieldStatus);
		
		ReleaseRefCountable(&fSymbolDefinitionFieldId);
		ReleaseRefCountable(&fSymbolDefinitionFieldScopeId);
		ReleaseRefCountable(&fSymbolDefinitionFieldNameId);
		ReleaseRefCountable(&fSymbolDefinitionFieldKind);
		ReleaseRefCountable(&fSymbolDefinitionFieldExecutionContext);
		ReleaseRefCountable(&fSymbolDefinitionFieldStatus);
		ReleaseRefCountable(&fSymbolDefinitionFieldSourceId);
		ReleaseRefCountable(&fSymbolDefinitionFieldLineNumber);
		
		ReleaseRefCountable(&fSymbolPrototypeFieldId);
		ReleaseRefCountable(&fSymbolPrototypeFieldFromName);
		ReleaseRefCountable(&fSymbolPrototypeFieldToName);
		ReleaseRefCountable(&fSymbolPrototypeFieldExecutionContext);
		ReleaseRefCountable(&fSymbolPrototypeFieldStatus);
		ReleaseRefCountable(&fSymbolPrototypeFieldSourceId);
		
		ReleaseRefCountable(&fSymbolReturnFieldId);
		ReleaseRefCountable(&fSymbolReturnFieldScopeId);
		ReleaseRefCountable(&fSymbolReturnFieldName);
		ReleaseRefCountable(&fSymbolReturnFieldStatus);
		ReleaseRefCountable(&fSymbolReturnFieldSourceId);
		
		ReleaseRefCountable(&fSymbolTypeFieldId);
		ReleaseRefCountable(&fSymbolTypeFieldDefinitionId);
		ReleaseRefCountable(&fSymbolTypeFieldName);
		ReleaseRefCountable(&fSymbolTypeFieldStatus);
		ReleaseRefCountable(&fSymbolTypeFieldSourceId);

		ReleaseRefCountable(&fSymbolReferenceFieldId);
		ReleaseRefCountable(&fSymbolReferenceFieldType);
		ReleaseRefCountable(&fSymbolReferenceFieldFromId);
		ReleaseRefCountable(&fSymbolReferenceFieldToId);
		ReleaseRefCountable(&fSymbolReferenceFieldStatus);
		ReleaseRefCountable(&fSymbolReferenceFieldSourceId);

		ReleaseRefCountable(&fOutlineItemFieldId);
		ReleaseRefCountable(&fOutlineItemFieldSourceId);
		ReleaseRefCountable(&fOutlineItemFieldName);
		ReleaseRefCountable(&fOutlineItemFieldLine);
		ReleaseRefCountable(&fOutlineItemFieldStatus);
	}

	ReleaseRefCountable(&fSymbolNameField);
	ReleaseRefCountable(&fSymbolIDField);
	ReleaseRefCountable(&fSymbolFileIDField);
	ReleaseRefCountable(&fSymbolScriptDocTextField);
	ReleaseRefCountable(&fSymbolKindField);
	ReleaseRefCountable(&fSymbolWAFKindField);
	ReleaseRefCountable(&fSymbolPrototypeIDField);
	ReleaseRefCountable(&fSymbolOwnerIDField);
	ReleaseRefCountable(&fSymbolLineNumberField);
	ReleaseRefCountable(&fSymbolLineCompletionNumberField);
	ReleaseRefCountable(&fSymbolReturnTypeIDField);
	ReleaseRefCountable(&fSymbolReferenceIDField);
	ReleaseRefCountable(&fSymbolUndefinedStateField);
	ReleaseRefCountable(&fSymbolInstanceStateField);
	ReleaseRefCountable(&fSymbolReferenceStateField);
	ReleaseRefCountable(&fSymbolEditionStateField);
	ReleaseRefCountable(&fSymbolFullNameField);

	ReleaseRefCountable(&fFileIDField);
	ReleaseRefCountable(&fFilePathField);
	ReleaseRefCountable(&fFileBaseFolderField);
	ReleaseRefCountable(&fFileExecContextField);
	ReleaseRefCountable(&fFileModificationTimeField);
	ReleaseRefCountable(&fFileResolvedStateField);
	
	ReleaseRefCountable(&fExtraIDField);
	ReleaseRefCountable(&fExtrasOwnerSymbolIDField);
	ReleaseRefCountable(&fExtrasKindField);
	ReleaseRefCountable(&fExtrasStringDataField);
	ReleaseRefCountable(&fExtrasIntegerDataField);
	
	ReleaseRefCountable(&fVersionNumberField);
}



std::vector< Symbols::ISymbol * > VSymbolTable::InnerSymbolTable::RetainSymbolsByNameHelper( sLONG inOwnerID, const VString &inName, sLONG inOwnerFileID, CDB4DBaseContext *inContext )
{
	std::vector< Symbols::ISymbol * > ret;
	if (!fDatabase)
		return ret;
	
	// We are going to be searching based on the name of the symbol as well as who owns
	// the symbol.  The SQL search we are imitating is:
	// SELECT * FROM Symbols WHERE OwnerID = inOwnerID AND Name = inName
	//
	// Let's get a new query object for us to fill out.  Note that since we are searching
	// text, we are going to do a case-sensitive search.  While this isn't true for all
	// of the languages we support, it's true for the current priority case, which is JavaScript.
	// When we start to add support for other languages, we may have to carry case sensitivity
	// information with the symbol so that we can perform the correct operation.
	CDB4DQuery *query = fSymbolsTable->NewQuery();
	query->AddCriteria( fSymbolNameField, DB4D_Equal, inName );
	
	query->AddLogicalOperator( DB4D_And );
	if (inOwnerID)
	{
		VLong id( inOwnerID );
		query->AddCriteria( fSymbolOwnerIDField, DB4D_Equal, id );
	}
	else
	{
		VLong id( 0 );
		query->AddCriteria( fSymbolOwnerIDField, DB4D_IsNull, id );
	}
	
	if (inOwnerFileID)
	{
		// If we have a file ID, then we want to further filter the results based on the file
		VLong id( inOwnerFileID );
		query->AddLogicalOperator( DB4D_And );
		query->AddCriteria( fSymbolFileIDField, DB4D_Equal, id );
	}
	
	CDB4DSelection *results = fSymbolsTable->ExecuteQuery( query, inContext );
	if (results)
	{
		RecIDType total = results->CountRecordsInSelection( inContext );
		for (RecIDType ctr = 1; ctr <= total; ctr++)
		{
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			if (record)
			{
				ret.push_back( SymbolFromRecord( fSymbolsTable, record, inContext ) );
				record->Release();
			}
		}
		results->Release();
	}
	
	query->Release();
	
	return ret;
}



std::vector< Symbols::ISymbol * > VSymbolTable::InnerSymbolTable::RetainSymbolsByName( const VString &inName, CDB4DBaseContext *inContext )
{
	return RetainSymbolsByNameHelper( 0, inName, 0, inContext );
}



std::vector< Symbols::ISymbol * > VSymbolTable::InnerSymbolTable::RetainSymbolsByName( Symbols::ISymbol *inOwner, const VString &inName, Symbols::IFile *inOwnerFile, CDB4DBaseContext *inContext )
{
	return RetainSymbolsByNameHelper( (inOwner) ? (inOwner->GetID()) : 0, inName, (inOwnerFile) ? (inOwnerFile->GetID()) : 0, inContext );
}



std::vector< Symbols::ISymbol * > VSymbolTable::InnerSymbolTable::RetainClassSymbols( Symbols::ISymbol *inOwner, const VString &inName, Symbols::IFile *inOwnerFile, CDB4DBaseContext *inContext )
{
	std::vector< Symbols::ISymbol * > ret;
	if (!fDatabase)	return
		ret;
	
	CDB4DQuery *query = fSymbolsTable->NewQuery();
	query->AddCriteria( fSymbolNameField, DB4D_Like, inName );
	query->AddLogicalOperator( DB4D_And );
	
	sLONG ownerFileId = inOwnerFile ? inOwnerFile->GetID() : 0;
	
	if (ownerFileId)
	{
		// If we have a file ID, then we want to further filter the results based on the file
		VLong id( ownerFileId );
		query->AddLogicalOperator( DB4D_And );
		query->AddCriteria( fSymbolFileIDField, DB4D_Equal, id );
	}
	
	VLong kind( Symbols::ISymbol::kKindClassConstructor );
	query->AddLogicalOperator( DB4D_And );
	query->AddCriteria( fSymbolKindField, DB4D_Equal, kind );
	
	CDB4DSelection *results = fSymbolsTable->ExecuteQuery( query, inContext );
	if (results)
	{
		RecIDType total = results->CountRecordsInSelection( inContext );
		results->SortSelection(fSymbolNameField->GetID(), true);
		for (RecIDType ctr = 1; ctr <= total; ctr++)
		{
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			if (record)
			{
				ret.push_back( SymbolFromRecord( fSymbolsTable, record, inContext ) );
				record->Release();
			}
		}
		results->Release();
	}
	
	query->Release();
	
	return ret;
}



std::vector< Symbols::ISymbol * > VSymbolTable::InnerSymbolTable::RetainLikeNamedSubSymbols( Symbols::ISymbol *inOwner, const VString &inLikeName, CDB4DBaseContext *inContext, const bool inFullyLoad)
{
	// We may have been given an owner (though it is legal for the owner to be NULL), and
	// we want to locate subsymbols for it.  The basic idea is that we want to perform a
	// SELECT * FROM Symbols WHERE Symbols.OwnerID = inOwner.UniqueID AND Symbols.Name LIKE inLikeName
	std::vector< Symbols::ISymbol * > ret;
	
	// If the user didn't pass us any LIKE text, then we want to bail out early.  Since this is only
	// used to get us an approximate list, we don't want to monkey around with getting the user every
	// single globally accessible symbol in the system (as that is very expensive) -- at least let them
	// filter the list with a single letter!
	if (inLikeName.IsEmpty() || (inLikeName.GetLength() == 1 && inLikeName[0] == inContext->GetIntlMgr()->GetWildChar()))
		return ret;
	
	if (!fDatabase)
		return ret;

	// Let's get a new query object for us to fill out
	CDB4DQuery *query = fSymbolsTable->NewQuery();
	
	if (inOwner)
	{
		VLong id( inOwner->GetID() );
		query->AddCriteria( fSymbolOwnerIDField, DB4D_Equal, id );
	}
	else
	{
		VLong id( 0 );
		query->AddCriteria( fSymbolOwnerIDField, DB4D_IsNull, id );
	}
	
	query->AddLogicalOperator( DB4D_And );
	query->AddCriteria( fSymbolNameField, DB4D_Like, inLikeName );
	
	VString emptyString = "";
	query->AddLogicalOperator( DB4D_And );
	query->AddCriteria( fSymbolNameField, DB4D_NotEqual, emptyString );
	
	CDB4DSelection *results = fSymbolsTable->ExecuteQuery( query, inContext );
	if (results)
	{
		RecIDType total = results->CountRecordsInSelection( inContext );
		for (RecIDType ctr = 1; ctr <= total; ctr++)
		{
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			if (record)
			{
				ret.push_back( SymbolFromRecord( fSymbolsTable, record, inContext, inFullyLoad ) );
				record->Release();
			}
		}
		results->Release();
	}
	query->Release();
	
	return ret;
}



std::vector< Symbols::ISymbol * > VSymbolTable::InnerSymbolTable::GetSymbolsForOutline( Symbols::ISymbol *inOwner, Symbols::IFile *inOwnerFile, SortingOptions inOptions, CDB4DBaseContext *inContext )
{
	// We may have been given an owner (though it is legal for the owner to be NULL), and
	// we want to locate subsymbols for it.  This is used for the outline functionality, and
	// so it is expected to return symbols that are interesting for the outline.  Some of these
	// symbols are named (in fact, most of them should be, since it's for code navigation), but
	// some are not named, such as anonymous function expressions.  We should be picky about what
	// we return to the caller though -- we don't want to return *all* symbols, because some nameless
	// ones are purely around as helpers (such as object literals).  The truly fun part of this is
	// that we don't want to show *all* nameless symbols, only the ones which haven't already been
	// assigned to a named symbol at the same level!
	std::vector< Symbols::ISymbol * > ret;
	
	if (!fDatabase)
		return ret;
	
	// Let's get a new query object for us to fill out
	CDB4DQuery *query = fSymbolsTable->NewQuery();
	
	if (inOwner)
	{
		VLong id( inOwner->GetID() );
		query->AddCriteria( fSymbolOwnerIDField, DB4D_Equal, id );
	}
	else
	{
		VLong id( 0 );
		query->AddCriteria( fSymbolOwnerIDField, DB4D_IsNull, id );
	}
	
	// If the user passed in a file to filter by, we want to add another criteria that
	// specifies Symbols.FileID = inOwnerFile.UniqueID
	if (inOwnerFile)
	{
		query->AddLogicalOperator( DB4D_And );
		VLong id( inOwnerFile->GetID() );
		query->AddCriteria( fSymbolFileIDField, DB4D_Equal, id );
	}
	
	CDB4DSelection *results = fSymbolsTable->ExecuteQuery( query, inContext );
	if (results)
	{
		// If the user passed in some sorting options, we want to pass those along to DB4D
		DB4D_FieldID sortingField = -1;
		switch (inOptions)
		{
			case kByName:
				sortingField = fSymbolNameField->GetID( inContext );
			break;
				
			case kByLineNumber:
				sortingField = fSymbolLineNumberField->GetID( inContext );
			break;
				
			case kByKind:
				sortingField = fSymbolKindField->GetID( inContext );
			break;
				
			case kDefault:
				break;
		}
		
		if (-1 != sortingField)
			results->SortSelection( sortingField, true, NULL, inContext );
		
		RecIDType total = results->CountRecordsInSelection( inContext );
		for (RecIDType ctr = 1; ctr <= total; ctr++)
		{
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			if (record)
			{
				ret.push_back( SymbolFromRecord( fSymbolsTable, record, inContext ) );
				record->Release();
			}
		}
		results->Release();
	}
	query->Release();
	
	return ret;
}



std::vector< Symbols::ISymbol * > VSymbolTable::InnerSymbolTable::RetainNamedSubSymbols( Symbols::ISymbol *inOwner, Symbols::IFile *inOwnerFile, SortingOptions inOptions, CDB4DBaseContext *inContext )
{
	// We may have been given an owner (though it is legal for the owner to be NULL), and
	// we want to locate subsymbols for it.  The basic idea is that we want to perform a
	// SELECT * FROM Symbols WHERE Symbols.OwnerID = inOwner.UniqueID AND Symbols.Name IS NOT ""
	std::vector< Symbols::ISymbol * > ret;
	
	if (!fDatabase)
		return ret;
	
	// Let's get a new query object for us to fill out
	CDB4DQuery *query = fSymbolsTable->NewQuery();
	
	if (inOwner)
	{
		VLong id( inOwner->GetID() );
		query->AddCriteria( fSymbolOwnerIDField, DB4D_Equal, id );
	}
	else
	{
		VLong id( 0 );
		query->AddCriteria( fSymbolOwnerIDField, DB4D_IsNull, id );
	}
	
	VString emptyString = "";
	query->AddLogicalOperator( DB4D_And );
	query->AddCriteria( fSymbolNameField, DB4D_NotEqual, emptyString );
	
	// If the user passed in a file to filter by, we want to add another criteria that
	// specifies Symbols.FileID = inOwnerFile.UniqueID
	if (inOwnerFile)
	{
		query->AddLogicalOperator( DB4D_And );
		VLong id( inOwnerFile->GetID() );
		query->AddCriteria( fSymbolFileIDField, DB4D_Equal, id );
	}
	
	CDB4DSelection *results = fSymbolsTable->ExecuteQuery( query, inContext );
	if (results)
	{
		// If the user passed in some sorting options, we want to pass those along to DB4D
		DB4D_FieldID sortingField = -1;
		switch (inOptions)
		{
			case kByName:
				sortingField = fSymbolNameField->GetID( inContext );
			break;
				
			case kByLineNumber:
				sortingField = fSymbolLineNumberField->GetID( inContext );
			break;
				
			case kByKind:
				sortingField = fSymbolKindField->GetID( inContext );
			break;
				
			case kDefault:
				break;
		}
		
		if (-1 != sortingField)
			results->SortSelection( sortingField, true, NULL, inContext );
		
		RecIDType total = results->CountRecordsInSelection( inContext );
		for (RecIDType ctr = 1; ctr <= total; ctr++)
		{
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			if (record)
			{
				ret.push_back( SymbolFromRecord( fSymbolsTable, record, inContext ) );
				record->Release();
			}
		}
		results->Release();
	}
	query->Release();
	
	return ret;
}



void VSymbolTable::InnerSymbolTable::ReleaseSymbols(std::vector<Symbols::ISymbol*>& inSymbols)
{
	int size = inSymbols.size();
	for(int index=0; index<size; index++)
		ReleaseRefCountable(&inSymbols[index]);
}



std::vector< Symbols::IFile * > VSymbolTable::InnerSymbolTable::RetainFilesByName( const VString &inName, CDB4DBaseContext *inContext )
{
	std::vector< Symbols::IFile * > ret;
	
	if (!fDatabase)
		return ret;
	
	// We are going to be searching based on the Path of the symbol, but we
	// will be doing a containment search so the path doesn't have to be a
	// perfect match.
	
	// Let's get a new query object for us to fill out
	CDB4DQuery *query = fFilesTable->NewQuery();
	UniChar likeChar = inContext->GetIntlMgr()->GetWildChar();
	query->AddCriteria( fFilePathField, DB4D_Like, VString( likeChar ) + inName + VString( likeChar ) );
	
	CDB4DSelection *results = fFilesTable->ExecuteQuery( query, inContext );
	if (results)
	{
		RecIDType total = results->CountRecordsInSelection( inContext );
		for (RecIDType ctr = 1; ctr <= total; ctr++)
		{
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			if ( testAssert ( record != NULL ) )
			{
				ret.push_back( FileFromRecord( fFilesTable, record, inContext ) );
				record->Release();
			}
		}
		results->Release();
	}
	query->Release();
	
	return ret;
}



std::vector< Symbols::IFile * > VSymbolTable::InnerSymbolTable::RetainFilesByPathAndBaseFolder( const VString &inName, const ESymbolFileBaseFolder inBaseFolder, CDB4DBaseContext *inContext )
{
	std::vector< Symbols::IFile * >	ret;
	VLong	baseFolderLong(inBaseFolder);
	
	if (!fDatabase)
		return ret;
	
	// Let's get a new query object for us to fill out
	CDB4DQuery *query = fFilesTable->NewQuery();
	query->AddCriteria( fFilePathField, DB4D_Equal, inName );
	query->AddLogicalOperator(DB4D_And);
	query->AddCriteria( fFileBaseFolderField, DB4D_Equal, baseFolderLong);
	
	
	CDB4DSelection *results = fFilesTable->ExecuteQuery( query, inContext );
	if (results)
	{
		RecIDType total = results->CountRecordsInSelection( inContext );
		for (RecIDType ctr = 1; ctr <= total; ctr++)
		{
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			if ( testAssert ( record != NULL ) )
			{
				ret.push_back( FileFromRecord( fFilesTable, record, inContext ) );
				record->Release();
			}
		}
		results->Release();
	}
	query->Release();
	
	return ret;
}



void VSymbolTable::InnerSymbolTable::ReleaseFiles(std::vector<Symbols::IFile*>& inFiles)
{
	int size = inFiles.size();
	for(int index=0; index<size; index++)
		ReleaseRefCountable(&inFiles[index]);
}



Symbols::IFile *VSymbolTable::InnerSymbolTable::GetFileByID( sLONG inID, CDB4DBaseContext *inContext )
{
	Symbols::IFile* ret = NULL;
	if( !fDatabase )
		return ret;
	
	CDB4DIndex* index = fDatabase->FindAndRetainIndexByName( "FileIDIndex", inContext );
	if(!index)
		return ret;
	
	// Since we're only interested in one record, and there's only one search criteria, we can make use
	// of the index directly.
	VLong id( inID );
	VErrorDB4D err = VE_OK;
	VCompareOptions options;
	RecIDType recID = index->FindKey( id, err, inContext, NULL, NULL, options );
	if (kDB4D_NullRecordID != recID)
	{
		CDB4DRecord *rec = fFilesTable->LoadRecord( recID, DB4D_Do_Not_Lock, inContext );
		if (rec)
		{
			ret = FileFromRecord( fFilesTable, rec, inContext );
			rec->Release();
		}
	}
	
	index->Release();
	
	return ret;
}



Symbols::ISymbol *VSymbolTable::InnerSymbolTable::GetSymbolByID( sLONG inID, CDB4DBaseContext *inContext )
{
	Symbols::ISymbol* ret = NULL;
	if (!fDatabase)
		return ret;
	
	CDB4DIndex *index = fDatabase->FindAndRetainIndexByName( "SymbolIDIndex", inContext );
	if (!index)
	{
		return ret;
	}
	
	// Since we're only interested in one record, and there's only one search criteria, we can make use
	// of the index directly.
	VLong id( inID );
	VErrorDB4D err = VE_OK;
	VCompareOptions options;
	RecIDType recID = index->FindKey( id, err, inContext, NULL, NULL, options );
	if (kDB4D_NullRecordID != recID)
	{
		CDB4DRecord *rec = fSymbolsTable->LoadRecord( recID, DB4D_Do_Not_Lock, inContext );
		if (rec)
		{
			ret = SymbolFromRecord( fSymbolsTable, rec, inContext );
			rec->Release();
		}
	}
	
	index->Release();
	
	return ret;
}



VString VSymbolTable::InnerSymbolTable::GetStringField( CDB4DRecord *inRecord, CDB4DField *inField ) const
{
	VString ret;
	inRecord->GetString( inField, ret );
	return ret;
}



sLONG VSymbolTable::InnerSymbolTable::GetIntegerField( CDB4DRecord *inRecord, CDB4DField *inField, bool *isNull ) const
{
	if (isNull)
		*isNull = false;
	
	VValueSingle *value = inRecord->GetFieldValue( inField );
	if (value)
	{
		if (isNull && value->IsNull())
			*isNull = true;
		
		return value->GetLong();
	}
	return 0;
}



uLONG8 VSymbolTable::InnerSymbolTable::GetLongIntegerField( CDB4DRecord *inRecord, CDB4DField *inField ) const
{
	sLONG8 ret;
	inRecord->GetLong8( inField, &ret );
	return (uLONG8)ret;
}



bool VSymbolTable::InnerSymbolTable::GetBooleanField(CDB4DRecord* inRecord, CDB4DField* inField) const
{
	Boolean temp;
	inRecord->GetBoolean(inField, &temp);

	bool ret;
	(temp == 0) ? ret=false : ret=true;
	return ret;
}



Symbols::IExtraInfo* VSymbolTable::InnerSymbolTable::ExtraInfoFromRecord( CDB4DTable *inTable, CDB4DRecord *inRecord, CDB4DBaseContext *inContext )
{
	xbox_assert( inRecord );
	
	VSymbolExtraInfo *ret = new VSymbolExtraInfo();
	
	ret->SetIntegerData( GetIntegerField( inRecord, fExtrasIntegerDataField, NULL ) );
	ret->SetStringData( GetStringField( inRecord, fExtrasStringDataField ) );
	ret->SetKind( (Symbols::IExtraInfo::Kind)GetIntegerField( inRecord, fExtrasKindField, NULL ) );
	
	return ret;
}



Symbols::IFile *VSymbolTable::InnerSymbolTable::FileFromRecord( CDB4DTable *inTable, CDB4DRecord *inRecord, CDB4DBaseContext *inContext )
{
	xbox_assert( inRecord );
	
	VSymbolFile *ret = new VSymbolFile();
	
	ret->SetID( GetIntegerField( inRecord, fFileIDField, NULL ) );
	ret->SetPath( GetStringField( inRecord, fFilePathField ) );
	ret->SetBaseFolder( (ESymbolFileBaseFolder) GetIntegerField( inRecord, fFileBaseFolderField, NULL ) );
	ret->SetExecutionContext( (ESymbolFileExecContext) GetIntegerField( inRecord, fFileExecContextField, NULL ) );
	ret->SetModificationTimestamp( GetLongIntegerField( inRecord, fFileModificationTimeField ) );
	ret->SetResolveState( GetBooleanField(inRecord, fFileResolvedStateField) );
	
	return ret;
}



Symbols::ISymbol *VSymbolTable::InnerSymbolTable::SymbolFromRecord( CDB4DTable *inTable, CDB4DRecord *inRecord, CDB4DBaseContext *inContext, bool inFullyLoad )
{
	xbox_assert( inRecord );
	
	// There can be recursion in the symbol loading scheme.  For instance, when
	// a function has a prototype, that prototype has a owner which is usually the
	// function itself.  That means we'll loop forever unless we track whether we've
	// seen this id or not.  So we're going to load the ID up from the record, and if
	// we've seen it before, we'll simply return a pointer to the symbol we were working
	// on.  If we've not seen it before, then we'll add a mapping of ID->Symbol so that
	// any recusion can be caught.  When we're done constructing the symbol, we can go
	// ahead and remove the ID from our map.
	sLONG uniqueID = GetIntegerField( inRecord, fSymbolIDField, NULL );
	
	Symbols::ISymbol *recRet = NULL;
	fRecursionTaskLock->Lock();
	std::map< sLONG, Symbols::ISymbol * >::iterator iter = fRecursionSet.find( uniqueID );
	if (iter != fRecursionSet.end())
	{
		recRet = iter->second;
		recRet->Retain();
	}
	fRecursionTaskLock->Unlock();
	
	if (recRet)
		return recRet;
	
	VSymbol *ret = new VSymbol();
	
	fRecursionTaskLock->Lock();
	ret->Retain();
	fRecursionSet[ uniqueID ] = ret;			// Add this symbol to our mapping
	fRecursionTaskLock->Unlock();
	ret->SetID( uniqueID );
	ret->SetName( GetStringField( inRecord, fSymbolNameField ) );
	ret->SetScriptDocComment( GetStringField( inRecord, fSymbolScriptDocTextField ) );
	ret->SetKind( (int)GetIntegerField( inRecord, fSymbolKindField, NULL ) );
	ret->SetLineNumber( GetIntegerField( inRecord, fSymbolLineNumberField, NULL ) );
	ret->SetLineCompletionNumber( GetIntegerField( inRecord, fSymbolLineCompletionNumberField, NULL ) );
	ret->SetUndefinedState( GetBooleanField(inRecord, fSymbolUndefinedStateField) );
	ret->SetInstanceState( GetBooleanField(inRecord, fSymbolInstanceStateField) );
	ret->SetReferenceState(  GetBooleanField(inRecord, fSymbolReferenceStateField) );
	ret->SetEditionState( GetBooleanField(inRecord, fSymbolEditionStateField) );
	ret->SetFullName( GetStringField( inRecord, fSymbolFullNameField) );
	
	VString wafKind = GetStringField( inRecord, fSymbolWAFKindField );
	if ( ! wafKind.IsEmpty() )
		ret->SetWAFKind( wafKind );
	
	if (inFullyLoad)
	{
		// Get the unique identifier for the prototype and owner IDs.  We will
		// search the symbols table for those next.  The prototype field is actually
		// a comma-separated string of IDs.  So we will get a list of symbols to
		// add, but if one of the IDs has gone stale, we simply won't add it to the list.
		bool isNull = false;
		VString prototypeIDs = GetStringField( inRecord, fSymbolPrototypeIDField );
		if (!prototypeIDs.IsEmpty())
		{
			VectorOfVString IDs;
			prototypeIDs.GetSubStrings( ',', IDs, false, true );
			
			std::vector< Symbols::ISymbol * > prototypes;
			for (VectorOfVString::iterator iter = IDs.begin(); iter != IDs.end(); ++iter)
			{
				sLONG prototypeID = (*iter).GetLong();
				// Prototypes can go stale, and that is fine.  If we are unable to load the prototype
				// from the field we've been given, we want to update the record so that the prototype
				// field is set to null.  This will help us the next time we go to load it.
				Symbols::ISymbol *prototypeSymbol = GetSymbolByID( prototypeID, inContext );
				if (prototypeSymbol)
				{
					prototypes.push_back( prototypeSymbol );
				}
			}
			
			if( !prototypes.empty() )
			{
				// We've loaded up at least one prototype, so let's add all of them to our symbol
				ret->AddPrototypes( prototypes );
				
				// modif SLA : memory leak
				for (std::vector< Symbols::ISymbol * >::iterator iter = prototypes.begin(); iter != prototypes.end(); ++iter)
					(*iter)->Release();
			}
		}
		
		sLONG ownerID = GetIntegerField( inRecord, fSymbolOwnerIDField, &isNull );
		if (!isNull)
		{
			Symbols::ISymbol *owner = GetSymbolByID( ownerID, inContext );
			ret->SetOwner( owner );
			if (owner)
			{
				/*std::vector< Symbols::ISymbol * > ownerReturnTypes = owner->GetReturnTypes();
				 for (std::vector< Symbols::ISymbol * >::iterator iter = ownerReturnTypes.begin(); iter != ownerReturnTypes.end(); ++iter)
				 if ( (*iter)->GetID() == ret->GetID() )
				 (*iter)->Release();*/
				owner->Release();
			}
		}
		
		// Return types are actually a comma-separated list of IDs
		VString returnTypeIDs = GetStringField( inRecord, fSymbolReturnTypeIDField );
		if (!returnTypeIDs.IsEmpty())
		{
			VectorOfVString IDs;
			returnTypeIDs.GetSubStrings( ',', IDs, false, true );
			for (VectorOfVString::iterator iter = IDs.begin(); iter != IDs.end(); ++iter)
			{
				sLONG returnTypeID = (*iter).GetLong();
				Symbols::ISymbol *returnType = GetSymbolByID( returnTypeID, inContext );
				if (returnType)
				{
					ret->AddReturnType( returnType );
					returnType->Release();
				}
			}
		}
		
		sLONG referenceID = GetIntegerField( inRecord, fSymbolReferenceIDField, &isNull );
		if (!isNull)
		{
			Symbols::ISymbol *ref = GetSymbolByID( referenceID, inContext );
			ret->SetReferenceSymbol( ref );
			if (ref)
				ref->Release();
		}
		
		sLONG fileID = GetIntegerField( inRecord, fSymbolFileIDField, NULL );
		Symbols::IFile *file = GetFileByID( fileID, inContext );
		if (file)
		{
			ret->SetFile(file);
			file->Release();
		}
	}
	
	fRecursionTaskLock->Lock();
	fRecursionSet.erase( uniqueID );		// Remove the symbol from our mapping
	ret->Release();
	fRecursionTaskLock->Unlock();
	
	return ret;
}



bool VSymbolTable::InnerSymbolTable::RecordFromFile( Symbols::IFile *inFile, CDB4DRecord *ioRecord, CDB4DBaseContext *inContext )
{
	xbox_assert( inFile );
	xbox_assert( ioRecord );
	
	ioRecord->SetString( fFilePathField, inFile->GetPath() );
	ioRecord->SetLong( fFileBaseFolderField, inFile->GetBaseFolder() );
	ioRecord->SetLong( fFileExecContextField, inFile->GetExecutionContext() );
	ioRecord->SetLong8( fFileModificationTimeField, inFile->GetModificationTimestamp() );
	ioRecord->SetLong( fFileResolvedStateField, inFile->GetResolveState() );
	
	return true;
}



bool VSymbolTable::InnerSymbolTable::RecordFromExtraInfo( const Symbols::ISymbol *inOwner, const Symbols::IExtraInfo *inExtraInfo, CDB4DRecord *ioRecord, CDB4DBaseContext *inContext )
{
	xbox_assert( inOwner );
	xbox_assert( inExtraInfo );
	xbox_assert( ioRecord );
	
	ioRecord->SetLong( fExtrasKindField, inExtraInfo->GetKind() );
	ioRecord->SetString( fExtrasStringDataField, inExtraInfo->GetStringData() );
	ioRecord->SetLong( fExtrasIntegerDataField, inExtraInfo->GetIntegerData() );
	ioRecord->SetLong( fExtrasOwnerSymbolIDField, inOwner->GetID() );
	
	return true;
}



bool VSymbolTable::InnerSymbolTable::UpdateFile( Symbols::IFile *inFile, CDB4DBaseContext *inContext )
{
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	JSLog log(true);
	log.SetTitle( CVSTR("SymbolTable::UpdateFile") );
	log.Append( CVSTR("Path"),			inFile->GetPath());
#endif //ACTIVATE_JSLOG
#endif //ACTIVATE_SYMBOL_TABLE_TRACES

	// We want to do an update operation, like the following
	// UPDATE Files SET ModificationTimestamp = newStamp, Path = newPath WHERE UniqueID = inID
	if (!fDatabase)
		return false;
	
	// Let's get a new query object for us to fill out
	CDB4DQuery *query = fFilesTable->NewQuery();
	VLong id( inFile->GetID() );
	query->AddCriteria( fFileIDField, DB4D_Equal, id );
	
	CDB4DSelection *results = fFilesTable->ExecuteQuery( query, inContext );
	CDB4DRecord *record = NULL;
	if (results && results->CountRecordsInSelection( inContext ) > 0)
	{
		record = results->LoadSelectedRecord( 1, DB4D_Keep_Lock_With_Record, inContext );
	}
	results->Release();
	query->Release();
	
	bool ret = false;
	if (record)
	{
		// We found a record, so now we want to stick the new path and modification
		// timestamp into it, then save it back out
		record->SetString( fFilePathField, inFile->GetPath() );
		record->SetLong8( fFileModificationTimeField, inFile->GetModificationTimestamp() );
		
		if (record->Save())
			ret = true;
		
		record->Release();
	}
	
	return ret;
}



bool VSymbolTable::InnerSymbolTable::UpdateSymbol( Symbols::ISymbol *inSym, CDB4DBaseContext *inContext )
{
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	JSLog log(true);
	log.SetTitle( CVSTR("SymbolTable::UpdateSymbol") );
	
		JSLog logSym;
		logSym.SetTitle( CVSTR("SymbolTable::UpdateSymbol") );
		logSym.Append( CVSTR("FileID"),			inSym->GetFile()->GetID());
		logSym.Append( CVSTR("ID"),				inSym->GetID());
		logSym.Append( CVSTR("Name"),			inSym->GetName());
		logSym.Append( CVSTR("Kind"),			inSym->GetKindString(inSym->GetKind()));
		logSym.Append( CVSTR("XKind"),			inSym->GetAuxKindString(inSym->GetAuxillaryKindInformation()));
		logSym.Append( CVSTR("Undef"),			inSym->GetUndefinedState());
		logSym.Append( CVSTR("Inst"),			inSym->GetInstanceState());
		logSym.Append( CVSTR("Ref"),			inSym->GetReferenceState());
		logSym.Append( CVSTR("Edited"),			inSym->GetEditionState());
		logSym.Append( CVSTR("FName"),			inSym->GetFullName());

		Symbols::ISymbol* owner = inSym->GetOwner();
		if( owner )
		{
			logSym.Append( CVSTR("OwID"),		owner->GetID());
			logSym.Append( CVSTR("OwName"),		owner->GetName());
			logSym.Append( CVSTR("OwKind"),		owner->GetKindString(owner->GetKind()));
			logSym.Append( CVSTR("OwXKind"),	owner->GetAuxKindString(owner->GetAuxillaryKindInformation()));
			logSym.Append( CVSTR("OwUndef"),	owner->GetUndefinedState());
			logSym.Append( CVSTR("OwInst"),		owner->GetInstanceState());
			logSym.Append( CVSTR("OwRef"),		owner->GetReferenceState());
			logSym.Append( CVSTR("OwFName"),	owner->GetFullName());
		}

		std::vector<Symbols::ISymbol*> prototypes = inSym->GetPrototypes();
		for(VIndex index2=0; index2<prototypes.size(); index2++)
		{
			Symbols::ISymbol* proto = prototypes[index2];
			
			logSym.Append( CVSTR("PrID"),		proto->GetID());
			logSym.Append( CVSTR("PrName"),		proto->GetName());
			logSym.Append( CVSTR("PrKind"),		proto->GetKindString(proto->GetKind()));
			logSym.Append( CVSTR("PrXKind"),	proto->GetAuxKindString(proto->GetAuxillaryKindInformation()));
			logSym.Append( CVSTR("PrRef"),		proto->GetReferenceState());
			logSym.Append( CVSTR("PrFName"),	proto->GetFullName());
		}
		
		const Symbols::ISymbol* reference = inSym->RetainReferencedSymbol();
		if( reference != inSym )
		{
			logSym.Append( CVSTR("RfID"),		reference->GetID());
			logSym.Append( CVSTR("RfName"),		reference->GetName());
			logSym.Append( CVSTR("RfKind"),		reference->GetKindString(reference->GetKind()));
			logSym.Append( CVSTR("RfXKind"),	reference->GetAuxKindString(reference->GetAuxillaryKindInformation()));
			logSym.Append( CVSTR("RfUndef"),	reference->GetUndefinedState());
			logSym.Append( CVSTR("RfInst"),		reference->GetInstanceState());
			logSym.Append( CVSTR("RfRef"),		reference->GetReferenceState());
			logSym.Append( CVSTR("RfFName"),	reference->GetFullName());
		}
		ReleaseRefCountable(&reference);

		std::vector<Symbols::ISymbol*> returns = inSym->GetReturnTypes();
		for(VIndex index3=0; index3<returns.size(); index3++)
		{
			Symbols::ISymbol* returnSym = returns[index3];
			
			logSym.Append( CVSTR("RtID"),		returnSym->GetID());
			logSym.Append( CVSTR("RtName"),		returnSym->GetName());
			logSym.Append( CVSTR("RtKind"),		returnSym->GetKindString(returnSym->GetKind()));
			logSym.Append( CVSTR("RtXKind"),	returnSym->GetAuxKindString(returnSym->GetAuxillaryKindInformation()));
			logSym.Append( CVSTR("RtUndef"),	returnSym->GetUndefinedState());
			logSym.Append( CVSTR("RtInst"),		returnSym->GetInstanceState());
			logSym.Append( CVSTR("RtRef"),		returnSym->GetReferenceState());
			logSym.Append( CVSTR("RtFName"),	returnSym->GetFullName());
		}
		
		logSym.Print();
#endif //ACTIVATE_JSLOG
#endif //ACTIVATE_SYMBOL_TABLE_TRACES

	bool ret = false;
	if (!fDatabase)
		return ret;
	
	CDB4DIndex *index = fDatabase->FindAndRetainIndexByName( "SymbolIDIndex", inContext );
	if (!index)
	{
		return ret;
	}
	
	// Since we're only interested in one record, and there's only one search criteria, we can make use
	// of the index directly.
	VLong id( inSym->GetID() );
	VErrorDB4D err = VE_OK;
	VCompareOptions options;
	RecIDType recID = index->FindKey( id, err, inContext, NULL, NULL, options );
	if (kDB4D_NullRecordID != recID)
	{
		CDB4DRecord *record = fSymbolsTable->LoadRecord( recID, DB4D_Keep_Lock_With_Record, inContext );
		if (record)
		{
			VString prototypeStr, returnTypeStr;
			
			// calculate protoype field
			std::vector< Symbols::ISymbol * > protos = inSym->GetPrototypes();
			if (!protos.empty())
			{
				VectorOfVString parts;
				for (std::vector< Symbols::ISymbol * >::iterator iter = protos.begin(); iter != protos.end(); ++iter)
				{
					VString id;
					id.FromLong( (*iter)->GetID() );
					parts.push_back( id );
				}
				prototypeStr.Join( parts, ',' );
			}
			
			std::vector< Symbols::ISymbol * > returnTypes = inSym->GetReturnTypes();
			if (!returnTypes.empty())
			{
				VectorOfVString parts;
				for (std::vector< Symbols::ISymbol * >::iterator iter = returnTypes.begin(); iter != returnTypes.end(); ++iter)
				{
					VString id;
					id.FromLong( (*iter)->GetID() );
					parts.push_back( id );
				}
				returnTypeStr.Join( parts, ',' );
			}
			
			record->SetString(fSymbolNameField, inSym->GetName() );
			if (inSym->GetFile())
				record->SetLong(fSymbolFileIDField, inSym->GetFile()->GetID() );
			record->SetString(fSymbolPrototypeIDField, prototypeStr );
			if (inSym->GetOwner())
				record->SetLong(fSymbolOwnerIDField, inSym->GetOwner()->GetID() );
			record->SetLong(fSymbolKindField, inSym->GetFullKindInformation() );
			record->SetString(fSymbolWAFKindField, inSym->GetWAFKind() );
			record->SetString(fSymbolScriptDocTextField,inSym->GetScriptDocComment() );
			record->SetLong(fSymbolLineNumberField, inSym->GetLineNumber() );
			record->SetLong(fSymbolLineCompletionNumberField, inSym->GetLineCompletionNumber() );
			record->SetString(fSymbolReturnTypeIDField, returnTypeStr );
			if( inSym->GetReferenceState() )
			{
				const Symbols::ISymbol *ref = inSym->RetainReferencedSymbol();
				record->SetLong(fSymbolReferenceIDField, ref->GetID() );
				ref->Release();
			}
			
			record->SetLong( fSymbolUndefinedStateField, inSym->GetUndefinedState() );
			record->SetLong( fSymbolInstanceStateField,  inSym->GetInstanceState() );
			record->SetLong( fSymbolReferenceStateField, inSym->GetReferenceState() );
			record->SetLong( fSymbolEditionStateField, inSym->GetEditionState() );
			record->SetString( fSymbolFullNameField, inSym->GetFullName() );

			if (record->Save())
				ret = true;
			
			record->Release();
		}
	}
	
	index->Release();
	
	return ret;
}



bool VSymbolTable::InnerSymbolTable::AddFile( Symbols::IFile* inFile, CDB4DBaseContext *inContext )
{
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	JSLog log(true);
	log.SetTitle( CVSTR("SymbolTable::AddFile") );
	log.Append( CVSTR("Path"), inFile->GetPath() );
#endif //ACTIVATE_JSLOG
#endif //ACTIVATE_SYMBOL_TABLE_TRACES
	
	if (!fDatabase)
		return false;
	
	bool bSucceeded = true;
	
	// Make a new record for the file
	CDB4DRecord *record = fFilesTable->NewRecord( inContext );
	if( !record )
	{
		bSucceeded = false;
	}
	else
	{
		if( !RecordFromFile( inFile, record, inContext ) )
			bSucceeded = false;
		
		// Now we can save that record back to the table
		if (bSucceeded && !record->Save())
			bSucceeded = false;
		
		// Now we can set the file's ID
		inFile->SetID( GetIntegerField( record, fFileIDField, NULL ) );
		
		record->Release();
	}
	
	return bSucceeded;
}



bool VSymbolTable::InnerSymbolTable::AddSymbols( std::vector< Symbols::ISymbol * > inSymbols, CDB4DBaseContext *inContext )
{
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	JSLog log(true);
	log.SetTitle( CVSTR("SymbolTable::AddSymbols") );
	log.Append( CVSTR("Count"),		inSymbols.size() );
#endif //ACTIVATE_JSLOG
#endif //ACTIVATE_SYMBOL_TABLE_TRACES

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	JSLog logComputeFullName(true);
	logComputeFullName.SetTitle( CVSTR("SymbolTable::AddSymbols::ComputeFullName") );
	logComputeFullName.Append( CVSTR("Count"),		inSymbols.size() );
#endif //ACTIVATE_JSLOG
#endif //ACTIVATE_SYMBOL_TABLE_TRACES
	for(VIndex index=0; index<inSymbols.size(); index++)
	{
		Symbols::ISymbol* current = inSymbols[index];
		if( !current->GetFullName().GetLength() )
			current->ComputeFullName();

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
		JSLog logSym;
		logSym.SetTitle( CVSTR("SymbolTable::AddSymbols") );
		logSym.Append( CVSTR("FileID"),			current->GetFile()->GetID());
		logSym.Append( CVSTR("ID"),				current->GetID());
		logSym.Append( CVSTR("Name"),			current->GetName());
		logSym.Append( CVSTR("Kind"),			current->GetKindString(current->GetKind()));
		logSym.Append( CVSTR("XKind"),			current->GetAuxKindString(current->GetAuxillaryKindInformation()));
		logSym.Append( CVSTR("Undef"),			current->GetUndefinedState());
		logSym.Append( CVSTR("Inst"),			current->GetInstanceState());
		logSym.Append( CVSTR("Ref"),			current->GetReferenceState());
		logSym.Append( CVSTR("Edited"),			current->GetEditionState());
		logSym.Append( CVSTR("FName"),			current->GetFullName());

		Symbols::ISymbol* owner = current->GetOwner();
		if( owner )
		{
			logSym.Append( CVSTR("OwID"),		owner->GetID());
			logSym.Append( CVSTR("OwName"),		owner->GetName());
			logSym.Append( CVSTR("OwKind"),		owner->GetKindString(owner->GetKind()));
			logSym.Append( CVSTR("OwXKind"),	owner->GetAuxKindString(owner->GetAuxillaryKindInformation()));
			logSym.Append( CVSTR("OwUndef"),	owner->GetUndefinedState());
			logSym.Append( CVSTR("OwInst"),		owner->GetInstanceState());
			logSym.Append( CVSTR("OwRef"),		owner->GetReferenceState());
			logSym.Append( CVSTR("OwFName"),	owner->GetFullName());
		}

		std::vector<Symbols::ISymbol*> prototypes = current->GetPrototypes();
		for(VIndex index2=0; index2<prototypes.size(); index2++)
		{
			Symbols::ISymbol* proto = prototypes[index2];
			
			logSym.Append( CVSTR("PrID"),		proto->GetID());
			logSym.Append( CVSTR("PrName"),		proto->GetName());
			logSym.Append( CVSTR("PrKind"),		proto->GetKindString(proto->GetKind()));
			logSym.Append( CVSTR("PrXKind"),	proto->GetAuxKindString(proto->GetAuxillaryKindInformation()));
			logSym.Append( CVSTR("PrFName"),	proto->GetFullName());
		}

		const Symbols::ISymbol* reference = current->RetainReferencedSymbol();
		if( reference != current )
		{
			logSym.Append( CVSTR("RfID"),		reference->GetID());
			logSym.Append( CVSTR("RfName"),		reference->GetName());
			logSym.Append( CVSTR("RfKind"),		reference->GetKindString(reference->GetKind()));
			logSym.Append( CVSTR("RfXKind"),	reference->GetAuxKindString(reference->GetAuxillaryKindInformation()));
			logSym.Append( CVSTR("RfUndef"),	reference->GetUndefinedState());
			logSym.Append( CVSTR("RfInst"),		reference->GetInstanceState());
			logSym.Append( CVSTR("RfRef"),		reference->GetReferenceState());
			logSym.Append( CVSTR("RfFName"),	reference->GetFullName());
		}
		ReleaseRefCountable(&reference);
		
		std::vector<Symbols::ISymbol*> returns = current->GetReturnTypes();
		for(VIndex index3=0; index3<returns.size(); index3++)
		{
			Symbols::ISymbol* returnSym = returns[index3];
			
			logSym.Append( CVSTR("RtID"),		returnSym->GetID());
			logSym.Append( CVSTR("RtName"),		returnSym->GetName());
			logSym.Append( CVSTR("RtKind"),		returnSym->GetKindString(returnSym->GetKind()));
			logSym.Append( CVSTR("RtXKind"),	returnSym->GetAuxKindString(returnSym->GetAuxillaryKindInformation()));
			logSym.Append( CVSTR("RtUndef"),	returnSym->GetUndefinedState());
			logSym.Append( CVSTR("RtInst"),		returnSym->GetInstanceState());
			logSym.Append( CVSTR("RtRef"),		returnSym->GetReferenceState());
			logSym.Append( CVSTR("RtFName"),	returnSym->GetFullName());
		}
		
		logSym.Print();
#endif //ACTIVATE_JSLOG
#endif //ACTIVATE_SYMBOL_TABLE_TRACES
	}
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	logComputeFullName.Print();
#endif //ACTIVATE_JSLOG
#endif //ACTIVATE_SYMBOL_TABLE_TRACES

	// Adding symbols is a bit trickier than adding files due to relation tracking.  We
	// need to add all of the symbols to the table before setting up any relationships
	// between the symbols.  This is required because there can be circular references
	// between symbols.  For instance, a function has a prototype symbol, but that prototype
	// could be a child symbol of the function itself.  But in that case, the prototype's
	// owner symbol is the function.  So both symbols must already be in the table (so that
	// their unique IDs are assigned) before we can update the relationship.
	
	if (!fDatabase)
		return false;

	// if a user action needs to access to database for getting suggestions,
	// it will put a lock on all updates so it can get all the db CPU usage
	// and get faster results, which is important for a user request.
	CheckUpdatesAllowed();
	
	bool bSucceeded = true;
	
	CDB4DSelection *selection = fSymbolsTable->NewSelection( DB4D_Sel_SmallSel );
	SymbolCollectionManager collection( inSymbols, fSymbolsTable, inContext );
	{
		StErrorContextInstaller errorContext( VE_DB4D_CANNOTSAVERECORD, VE_DB4D_CANNOT_COMPLETE_COLLECTION_TO_DATA, VE_OK );
		CDB4DSet *locked = NULL;
		if (VE_OK != selection->CollectionToData( collection, inContext, false, false, locked, NULL ))
			bSucceeded = false;
		
		if (locked)
			locked->Release();
	}
	selection->Release();
	
	return bSucceeded;
}




bool VSymbolTable::InnerSymbolTable::DeleteSymbolsForFile( Symbols::IFile *inFile, bool inOnlyEditedSymbols, CDB4DBaseContext *inContext )
{
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	JSLog log(true);
	log.SetTitle( CVSTR("SymbolTable::DeleteSymbolsForFile") );
	log.Append( CVSTR("Path"),			inFile->GetPath() );
#endif //ACTIVATE_JSLOG
#endif //ACTIVATE_SYMBOL_TABLE_TRACES
	
	xbox_assert( inFile );
	
	CheckUpdatesAllowed();
	
	// We need to find the symbols in the table we're looking for.  This is akin to doing a
	// SELECT * FROM Symbols WHERE FileID = inFile.UniqueID
	CDB4DQuery* query = fSymbolsTable->NewQuery();
	if( !query )
		return false;

	VLong id( inFile->GetID() );
	query->AddCriteria( fSymbolFileIDField, DB4D_Equal, id );
	
	
	CDB4DSelection* results = fSymbolsTable->ExecuteQuery( query, inContext );
	bool bRet = true;
	if( results )
	{
		// Delete the records we've found. However, we want to ignore any problems due to other threads having
		// locked a record that we're trying to lock.  This can happen when two threads are trying to delete
		// records that cascade into records we're trying to delete as well.  We can safely ignore these issues
		// since the end result is the same: we all want to delete the same stuff.
		StErrorContextInstaller errorContext( VE_DB4D_RECORDISLOCKED, VE_DB4D_CANNOT_DELETE_ALL_RECORDS_IN_SEL, VE_OK );
			
		RecIDType total = results->CountRecordsInSelection( inContext );
		for (RecIDType i = 1; i <= total; i++)
		{
			VErrorDB4D err = fSymbolsTable->DeleteRecord( results->GetSelectedRecordID( i, inContext ), inContext );
			if (err != VE_OK && err != VE_DB4D_RECORDISLOCKED && err != VE_DB4D_CANNOT_DELETE_ALL_RECORDS_IN_SEL)
			{
				// Something has gone wrong with deleting the record, and we want to report it back to the user
				bRet = false;
			}
		}
		results->Release();
	}
	
	query->Release();
	
	return true;
}




Symbols::ISymbol *VSymbolTable::InnerSymbolTable::GetSymbolOwnerByLine( Symbols::IFile *inOwnerFile, int inLineNumber, CDB4DBaseContext *inContext )
{
	// This is performing the following SQL search:
	// SELECT * FROM Symbols WHERE
	//	Symbols.FileID = inOwnerFile.UniqueID AND Symbols.Kind = kKindFunctionDeclaration AND
	//	Symbols.LineNumber <= inLineNumber AND Symbols.LineCompletionNumber >= inLineNumber
	//
	// Note that this may return multiple symbols.  We are doing a narrowest-fit search though,
	// and looking for the symbol which most-closely surrounds the given line number.  That is
	// accomplished by looking for the symbol whose starting line number is closest to the given
	// line number.  This function should return null if ths line number resides in the global
	// namespace section of the file, which means we won't find any functions enclosing it.
	
 	if (!fDatabase)
		return NULL;
	
	xbox_assert( inOwnerFile );
	
	CDB4DQuery *query = fSymbolsTable->NewQuery();
	if (!query)
	{
		return NULL;
	}
	
	// Symbols.FileID = inOwnerFile.UniqueID
	VLong id( inOwnerFile->GetID() );
	query->AddCriteria( fSymbolFileIDField, DB4D_Equal, id );
	
	// AND Symbols.LineNumber <= inLineNumber
	VLong lineNumber( inLineNumber );
	query->AddLogicalOperator( DB4D_And );
	query->AddCriteria( fSymbolLineNumberField, DB4D_LowerOrEqual, lineNumber );
	
	// AND Symbols.LineCompletionNumber >= inLineNumber
	query->AddLogicalOperator( DB4D_And );
	query->AddCriteria( fSymbolLineCompletionNumberField, DB4D_GreaterOrEqual, lineNumber );
	
	CDB4DSelection *results = fSymbolsTable->ExecuteQuery( query, inContext );
	std::vector< Symbols::ISymbol * > syms;
	if (results)
	{
		RecIDType total = results->CountRecordsInSelection( inContext );
		for (RecIDType ctr = 1; ctr <= total; ctr++)
		{
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			if (record)
			{
				syms.push_back( SymbolFromRecord( fSymbolsTable, record, inContext) );
				record->Release();
			}
		}
		results->Release();
	}
	query->Release();
	
	// Now that we have a list of symbols, we want to find the one that is a narrowest match
	// to our given line number.  Note that it is possible we don't have any match at all!
	Symbols::ISymbol *ret = NULL;
	for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter)
	{
		if (!ret)
		{
			// We've not picked any match yet, so this one is good enough
			if ( (*iter)->IsFunctionKind() )
				ret = *iter;
		}
		else
		{
			// We have something to test against.  Check to see if the iterator's distance to the
			// line is less than the current return symbol's distance.  This will tell us which symbol
			// is closer.
			if ((inLineNumber - (*iter)->GetLineNumber()) < (inLineNumber - ret->GetLineNumber()))
			{
				ret = *iter;
			}
		}
	}
	
	// Now we need to clean up all of the symbols except for the one we're returning to the caller
	if (ret)
		ret->Retain();
	
	for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter)
		(*iter)->Release();
	
	return ret;
}



std::vector< Symbols::IExtraInfo * > VSymbolTable::InnerSymbolTable::GetExtraInformation( const Symbols::ISymbol *inOwnerSymbol, Symbols::IExtraInfo::Kind inKind, CDB4DBaseContext *inContext )
{
	std::vector< Symbols::IExtraInfo * > ret;
	if (!fDatabase)
		return ret;
	
	// Let's get a new query object for us to fill out
	CDB4DQuery *query = fExtrasTable->NewQuery();
	query->AddCriteria( fExtrasOwnerSymbolIDField, DB4D_Equal, VLong( inOwnerSymbol->GetID() ) );
	query->AddLogicalOperator( DB4D_And );
	query->AddCriteria( fExtrasKindField, DB4D_Equal, VLong( (sLONG)inKind ) );
	CDB4DSelection *results = fExtrasTable->ExecuteQuery( query, inContext );
	if (results)
	{
		RecIDType total = results->CountRecordsInSelection( inContext );
		for (RecIDType ctr = 1; ctr <= total; ctr++)
		{
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			ret.push_back( ExtraInfoFromRecord( fExtrasTable, record, inContext ) );
			record->Release();
		}
		results->Release();
	}
	query->Release();
	
	return ret;
}



bool VSymbolTable::InnerSymbolTable::AddExtraInformation( const Symbols::ISymbol *inOwnerSymbol, const Symbols::IExtraInfo *inInfo, CDB4DBaseContext *inContext )
{
	if (!fDatabase)
		return false;
	
	CheckUpdatesAllowed();
	
	bool bSucceeded = true;
	// Make a new record for the information
	CDB4DRecord *record = fExtrasTable->NewRecord( inContext );
	if (!record)
		bSucceeded = false;
	
	if (bSucceeded && !RecordFromExtraInfo( inOwnerSymbol, inInfo, record, inContext ))
		bSucceeded = false;
	
	// Now we can save that record back to the table
	if (bSucceeded && !record->Save())
		bSucceeded = false;
	
	record->Release();
	
	return bSucceeded;
}



void VSymbolTable::InnerSymbolTable::SetSymbolsEditionState(const Symbols::IFile* inFile, bool inEditionState, std::vector<sLONG>& outIds, CDB4DBaseContext* inContext)
{
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	JSLog log(true);
	log.SetTitle( CVSTR("SymbolTable::SetSymbolsEditionState") );
#endif //ACTIVATE_JSLOG
#endif //ACTIVATE_SYMBOL_TABLE_TRACES

	if( inFile && fDatabase && fSymbolsTable )
	{

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
		DumpSymbols(inFile->GetID(), inContext);
#endif //ACTIVATE_JSLOG
#endif //ACTIVATE_SYMBOL_TABLE_TRACES

		// Let's get a new query object for us to fill out
		CDB4DQuery* query = fSymbolsTable->NewQuery();
		VLong id( inFile->GetID() );
		query->AddCriteria( fSymbolFileIDField, DB4D_Equal, id );
		
		CDB4DSelection* results = fSymbolsTable->ExecuteQuery( query, inContext );
		if( results )
		{
			RecIDType total = results->CountRecordsInSelection( inContext );
			for( RecIDType index=1; index<=total; index++)
			{
				CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Keep_Lock_With_Record, inContext );
				if( record )
				{
					record->SetLong(fSymbolEditionStateField, VLong(inEditionState) );
					record->Save();
					sLONG id = GetIntegerField(record, fSymbolIDField, NULL);
					outIds.push_back(id);
				}
				
				// !!! Release memory !!!
				ReleaseRefCountable(&record);
			}
		}

#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
		DumpSymbols(inFile->GetID(), inContext);
#endif //ACTIVATE_JSLOG
#endif //ACTIVATE_SYMBOL_TABLE_TRACES
		
		// !!! Release memory !!!
		ReleaseRefCountable(&results);
		ReleaseRefCountable(&query);
	}
}




void VSymbolTable::InnerSymbolTable::GetFileIdsReferencingDeadSymbols(const std::vector<sLONG>& inSymbolIds, std::vector<sLONG>& outFileIds, CDB4DBaseContext* inContext)
{
#if ACTIVATE_SYMBOL_TABLE_TRACES
#if ACTIVATE_JSLOG
	JSLog log(true);
	log.SetTitle( CVSTR("SymbolTable::GetFileIdsReferencingDeadSymbols") );
	log.Append(CVSTR("SymsCount"), inSymbolIds.size());
#endif //ACTIVATE_JSLOG
#endif //ACTIVATE_SYMBOL_TABLE_TRACES

	if( fDatabase && fSymbolsTable )
	{
		// We want to get all file ids in which we reference a dead symbol.
		// By referencing we mean having a prototype id, an owner id, a return type id or a reference id of a symbol which doesn't exist anymore
		
		UniChar wildChar = inContext->GetIntlMgr()->GetWildChar();
		
		for(VIndex index=0; index<inSymbolIds.size(); index++)
		{
			
			
			
			// Let's get a new query object for us to fill out
			CDB4DQuery* query = fSymbolsTable->NewQuery();
			
			VLong symId( inSymbolIds[index] );

			// Prototype pattern to find could be ID,* or *,ID or *,ID,*
			VString protoypeIdPattern1;		//ID,*
			protoypeIdPattern1.AppendLong(symId);
			protoypeIdPattern1.AppendString(",");
			protoypeIdPattern1.AppendUniChar(wildChar);
			query->AddCriteria( fSymbolPrototypeIDField, DB4D_Equal, protoypeIdPattern1 );
			query->AddLogicalOperator( DB4D_OR );

			VString protoypeIdPattern2;		//*,ID
			protoypeIdPattern2.AppendUniChar(wildChar);
			protoypeIdPattern2.AppendString(",");
			protoypeIdPattern2.AppendLong(symId);
			query->AddCriteria( fSymbolPrototypeIDField, DB4D_Equal, protoypeIdPattern2 );
			query->AddLogicalOperator( DB4D_OR );

			VString protoypeIdPattern3;		//*,ID,*
			protoypeIdPattern3.AppendUniChar(wildChar);
			protoypeIdPattern3.AppendString(",");
			protoypeIdPattern3.AppendLong(symId);
			protoypeIdPattern3.AppendString(",");
			protoypeIdPattern3.AppendUniChar(wildChar);
			query->AddCriteria( fSymbolPrototypeIDField, DB4D_Equal, protoypeIdPattern3 );
			query->AddLogicalOperator( DB4D_OR );

			// No specific pattern for owner id, just search it
			query->AddCriteria( fSymbolOwnerIDField, DB4D_Equal, symId );
			query->AddLogicalOperator( DB4D_OR );
			
			// No specific pattern for reference id, just search it
			query->AddCriteria( fSymbolReferenceIDField, DB4D_Equal, symId );
			query->AddLogicalOperator( DB4D_OR );
			
			// Return type pattern to find could be ID,* or *,ID or *,ID,*
			VString returnTypeIdPattern1;		//ID,*
			returnTypeIdPattern1.AppendLong(symId);
			returnTypeIdPattern1.AppendString(",");
			returnTypeIdPattern1.AppendUniChar(wildChar);
			query->AddCriteria( fSymbolReturnTypeIDField, DB4D_Equal, returnTypeIdPattern1 );
			query->AddLogicalOperator( DB4D_OR );
			
			VString returnTypeIdPattern2;		//*,ID
			returnTypeIdPattern2.AppendUniChar(wildChar);
			returnTypeIdPattern2.AppendString(",");
			returnTypeIdPattern2.AppendLong(symId);
			query->AddCriteria( fSymbolReturnTypeIDField, DB4D_Equal, returnTypeIdPattern2 );
			query->AddLogicalOperator( DB4D_OR );
			
			VString returnTypeIdPattern3;		//*,ID,*
			returnTypeIdPattern3.AppendUniChar(wildChar);
			returnTypeIdPattern3.AppendString(",");
			returnTypeIdPattern3.AppendLong(symId);
			returnTypeIdPattern3.AppendString(",");
			returnTypeIdPattern3.AppendUniChar(wildChar);
			query->AddCriteria( fSymbolReturnTypeIDField, DB4D_Equal, returnTypeIdPattern3 );

			CDB4DSelection* results = fSymbolsTable->ExecuteQuery( query, inContext );
			if( results )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; index++)
				{
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					if( record )
					{
						sLONG fileId = GetIntegerField(record, fSymbolFileIDField, NULL);
						if( find(outFileIds.begin(), outFileIds.end(), fileId) == outFileIds.end() )
							outFileIds.push_back(fileId);
					}
					
					// !!! Release memory !!!
					ReleaseRefCountable(&record);
				}
			}
			
			// !!! Release memory !!!
			ReleaseRefCountable(&results);
			ReleaseRefCountable(&query);

			
			
		}
	}
}

Symbols::IFile* VSymbolTable::InnerSymbolTable::RetainFileByID(sLONG inID, CDB4DBaseContext* inContext)
{
	Symbols::IFile* ret = NULL;
	
	if( fDatabase && fFilesTable )
	{
		// Let's get a new query object for us to fill out
		CDB4DQuery* query = fFilesTable->NewQuery();
		
		VLong fileId( inID );
		query->AddCriteria( fFileIDField, DB4D_Equal, fileId );
		
		CDB4DSelection* results = fFilesTable->ExecuteQuery( query, inContext );
		if( results )
		{
			RecIDType total = results->CountRecordsInSelection( inContext );
			for( RecIDType index=1; index<=total; index++)
			{
				CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
				if( record )
					ret = FileFromRecord(fFilesTable, record, inContext);
				
				// !!! Release memory !!!
				ReleaseRefCountable(&record);
			}
		}
		
		// !!! Release memory !!!
		ReleaseRefCountable(&results);
		ReleaseRefCountable(&query);
	}
	
	return ret;
}

void VSymbolTable::InnerSymbolTable::GetSymbolIdsByFileAndEditionState(sLONG inFileId, bool inEditionState, std::vector<sLONG>& outSymbolIds, CDB4DBaseContext* inContext)
{
	if( fDatabase && fSymbolsTable )
	{
		CDB4DQuery* query = fSymbolsTable->NewQuery();
		if( query )
		{
			VLong fileId(inFileId);
			query->AddCriteria(fSymbolFileIDField, DB4D_Equal, fileId);
			query->AddLogicalOperator(DB4D_And);
			
			VBoolean state(inEditionState);
			query->AddCriteria(fSymbolEditionStateField, DB4D_Equal, state);
			
			CDB4DSelection* results = fSymbolsTable->ExecuteQuery(query, inContext);
			if( results )
			{
				RecIDType total = results->CountRecordsInSelection( inContext );
				for( RecIDType index=1; index<=total; index++)
				{
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					if( record )
					{
						sLONG id = GetIntegerField(record, fSymbolIDField, NULL);
						outSymbolIds.push_back(id);
					}
					
					// !!! Release memory !!!
					ReleaseRefCountable(&record);
				}
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
}

#if ACTIVATE_SYMBOL_TABLE_LIVE_REQUEST
void VSymbolTable::InnerSymbolTable::Dump(const XBOX::VString& inRequest, CDB4DBaseContext* inContext)
{
	// TAB is our expected separator
	VectorOfVString tokens;
	inRequest.GetSubStrings( CVSTR(" "), tokens);

	VString tableName				= "";
	VString fieldName				= "";
	VString fieldValueComparator	= "";
	VString valueString				= "";
	bool	valueStringSet			= false;
	sLONG	valueLong				= 0;
	bool	valueLongSet			= false;
	bool	valueNULLSet			= false;
	
	unsigned int expectedTokenType = 0;

	CDB4DQuery* query = NULL;

	for(VectorOfVString::const_iterator it=tokens.begin(); it!=tokens.end(); ++it)
	{
		VString tok = (*it);
		switch(expectedTokenType)
		{
			case 0: // TABLE NAME OR SPECIAL COMMAND
			{
				if( tok==CVSTR("scopes") || tok==CVSTR("names") || tok==CVSTR("definitions") || tok==CVSTR("prototypes") || tok==CVSTR("returns") || tok==CVSTR("types") || tok==CVSTR("references") || tok==CVSTR("items") || tok==CVSTR("symbols") || tok==CVSTR("files") || tok==CVSTR("extras") )
				{
					tableName = tok;
					expectedTokenType = 1;
				}
				else if( tok==CVSTR("tables") )
				{
					JSLog log;
					log.SetTitle(CVSTR("STLR"));
					log.Append(CVSTR("Tables"), CVSTR("scopes, names, definitions, prototypes, returns, types, references, items, files"));
				}
				else if( tok==CVSTR("fieldsof") )
				{
					expectedTokenType = 5;
				}
				else
				{
					JSLog error;
					error.SetTitle("Symbol table live requesting");
					error.Append("Error", "Unkown table name (allowed : scopes, names, definitions, prototypes, returns, types, references, items, symbols, files, extras)");
					return;
				}
			}
				break;
				
			case 1:	// FIELD NAME
			{
				if( tableName==CVSTR("symbols") )
				{
					if( tok==CVSTR("id") || tok==CVSTR("name") || tok==CVSTR("fileid") || tok==CVSTR("prototypeid") || tok ==CVSTR("ownerid") || tok==CVSTR("kind") || tok==CVSTR("wafkind") || tok==CVSTR("scriptdoctext") || tok==CVSTR("linenumber")
						|| tok==CVSTR("linecompletionnumber") || tok==CVSTR("returntypeid") || tok==CVSTR("referenceid") || tok==CVSTR("undefinedstate") || tok==CVSTR("instancestate") || tok==CVSTR("referencestate") || tok==CVSTR("editionstate") || tok==CVSTR("symbolfullname") )
					{
						fieldName = tok;
						expectedTokenType=2;
					}
					else
					{
						JSLog error;
						error.SetTitle("Symbol table live requesting");
						error.Append("Error", "Unkown field name for table symbols (allowed : id, name, fileid, prototypeid, ownerid, kind, wafkind, scriptdoctext, linenumber, linecompletionnumber, returntypeid, referenceid, undefinedstate, instancestate, referencestate, editionstate, symbolfullname)");
						return;
					}

				}
				else if( tableName==CVSTR("files") )
				{
					if( tok==CVSTR("id") || tok==CVSTR("path") || tok==CVSTR("basefolder") || tok==CVSTR("executioncontext") || tok==CVSTR("modificationtimestamp") || tok==CVSTR("resolvedstate") )
					{
						fieldName = tok;
						expectedTokenType = 2;
					}
					else
					{
						JSLog error;
						error.SetTitle("Symbol table live requesting");
						error.Append("Error", "Unkown field name for table files (allowed : id, path, basefolder, executioncontext, modificationtimestamp, resolvedstate)");
						return;
					}
					
				}
				else if( tableName==CVSTR("extras") )
				{
					if( tok==CVSTR("id") || tok==CVSTR("ownerid") || tok==CVSTR("kind") || tok==CVSTR("stringdata") || tok==CVSTR("integerdata") )
					{
						fieldName = tok;
						expectedTokenType = 2;
					}
					else
					{
						JSLog error;
						error.SetTitle("Symbol table live requesting");
						error.Append("Error", "Unkown field name for table extras (allowed : id, ownerid, kind, stringdata, integerdata)");
						return;
					}
					
				}
				else if( tableName==CVSTR("scopes") )
				{
					if( tok==CVSTR("id") || tok==CVSTR("name") || tok==CVSTR("fullname") || tok==CVSTR("type") || tok==CVSTR("sourceid") || tok==CVSTR("executioncontext") || tok==CVSTR("parentid") || tok==CVSTR("status") || tok==CVSTR("constructorflag") )
					{
						fieldName = tok;
						expectedTokenType = 2;
					}
					else
					{
						JSLog error;
						error.SetTitle("Symbol table live requesting");
						error.Append("Error", "Unknown field name for table scopes (allowed : id, name, fullname, type, sourceid, executioncontext, parentid, status, constructorflag)");
						return;
					}
				}
				else if( tableName==CVSTR("names") )
				{
					if( tok==CVSTR("id") || tok==CVSTR("value") || tok==CVSTR("scopeid") || tok==CVSTR("executioncontext") || tok==CVSTR("status") || tok==CVSTR("sourceid") )
					{
						fieldName = tok;
						expectedTokenType = 2;
					}
					else
					{
						JSLog error;
						error.SetTitle("Symbol table live requesting");
						error.Append("Error", "Unknown field name for table names (allowed : id, value, scopeid, executioncontext, status, sourceid)");
						return;
					}
				}
				else if( tableName==CVSTR("definitions") )
				{
					if( tok==CVSTR("id") || tok==CVSTR("scopeid") || tok==CVSTR("nameid") || tok==CVSTR("kind") || tok==CVSTR("kind") || tok==CVSTR("visibility") || tok==CVSTR("executioncontext") || tok==CVSTR("status") || tok==CVSTR("sourceid") || tok==CVSTR("linenumber") )
					{
						fieldName = tok;
						expectedTokenType = 2;
					}
					else
					{
						JSLog error;
						error.SetTitle("Symbol table live requesting");
						error.Append("Error", "Unknown field name for table definitions (allowed : id, scopeid, nameid, kind, visibility, executioncontext, status, sourceid, linenumber)");
						return;
					}
				}
				else if( tableName==CVSTR("prototypes") )
				{
					if( tok==CVSTR("id") || tok==CVSTR("fromname") || tok==CVSTR("toname") || tok==CVSTR("executioncontext") || tok==CVSTR("status") || tok==CVSTR("sourceid") )
					{
						fieldName = tok;
						expectedTokenType = 2;
					}
					else
					{
						JSLog error;
						error.SetTitle("Symbol table live requesting");
						error.Append("Error", "Unknown field name for table prototypes (allowed : id, fromname, toname, executioncontext, status, sourceid)");
						return;
					}
				}
				else if( tableName==CVSTR("returns") )
				{
					if( tok==CVSTR("id") || tok==CVSTR("scopeid") || tok==CVSTR("name") || tok==CVSTR("status") || tok==CVSTR("sourceid") )
					{
						fieldName = tok;
						expectedTokenType = 2;
					}
					else
					{
						JSLog error;
						error.SetTitle("Symbol table live requesting");
						error.Append("Error", "Unknown field name for table prototypes (allowed : id, scopeid, name, status, sourceid)");
						return;
					}
				}
				else if( tableName==CVSTR("types") )
				{
					if( tok==CVSTR("id") || tok==CVSTR("definitionid") || tok==CVSTR("name") || tok==CVSTR("status") || tok==CVSTR("sourceid") )
					{
						fieldName = tok;
						expectedTokenType = 2;
					}
					else
					{
						JSLog error;
						error.SetTitle("Symbol table live requesting");
						error.Append("Error", "Unknown field name for table prototypes (allowed : id, definitionid, name, status, sourceid)");
						return;
					}
				}
				else if( tableName==CVSTR("references") )
				{
					if( tok==CVSTR("id") || tok==CVSTR("type") || tok==CVSTR("fromid") || tok==CVSTR("toid") || tok==CVSTR("status") || tok==CVSTR("sourceid") )
					{
						fieldName = tok;
						expectedTokenType = 2;
					}
					else
					{
						JSLog error;
						error.SetTitle("Symbol table live requesting");
						error.Append("Error", "Unknown field name for table references (allowed : id, type, fromid, toid, status, sourceid)");
						return;
					}
				}
				else if( tableName==CVSTR("items") )
				{
					if( tok==CVSTR("id") || tok==CVSTR("sourceid") || tok==CVSTR("name") || tok==CVSTR("line") || tok==CVSTR("status") )
					{
						fieldName = tok;
						expectedTokenType = 2;
					}
					else
					{
						JSLog error;
						error.SetTitle("Symbol table live requesting");
						error.Append("Error", "Unknown field name for table references (allowed : id, sourceid, name, line, status)");
						return;
					}
				}
			}
				break;
				
			case 2:	// FIELD VALUE COMPARATOR
			{
				if( tok==CVSTR("==") || tok==CVSTR("!=") || tok==CVSTR("<") || tok==CVSTR(">") || tok==CVSTR("<=") || tok==CVSTR(">=") )
				{
					fieldValueComparator = tok;
					expectedTokenType = 3;
				}
				else
				{
					JSLog error;
					error.SetTitle("Symbol table live requesting");
					error.Append("Error", "Unkown field value condition (allowed : ==, !=, <, >, <=, >=)");
					return;
				}
			}
				break;
				
			case 3:	// FIELD VALUE
			{
				if( tableName==CVSTR("symbols") )
				{
					if( fieldName==CVSTR("name") || fieldName==CVSTR("prototypeid") || fieldName==CVSTR("wafkind") || fieldName==CVSTR("scriptdoctext") || fieldName==CVSTR("returntypeid") || fieldName==CVSTR("symbolfullname") )
					{
						if( tok==CVSTR("\"\"") || tok==CVSTR("\'\'") )	valueString		=	CVSTR("");
						else											valueString		=	tok;
						valueStringSet		=	true;
						valueLongSet		=	false;
						valueNULLSet		=	false;
					}
					else
					{
						valueStringSet		=	false;
						if( tok==CVSTR("0L") )
						{
							valueLongSet		=	false;
							valueNULLSet		=	true;
						}
						else
						{
							valueLong			=	tok.GetLong();
							valueLongSet		=	true;
							valueNULLSet		=	false;
						}
					}
				}
				else if( tableName==CVSTR("files") )
				{
					if( fieldName==CVSTR("path") )
					{
						if( tok==CVSTR("\"\"") || tok==CVSTR("\'\'") )	valueString		=	CVSTR("");
						else											valueString		=	tok;
						valueStringSet		=	true;
						valueLongSet		=	false;
						valueNULLSet		=	false;
					}
					else
					{
						valueStringSet		=	false;
						if( tok==CVSTR("0L") )
						{
							valueLongSet		=	false;
							valueNULLSet		=	true;
						}
						else
						{
							valueLong			=	tok.GetLong();
							valueLongSet		=	true;
							valueNULLSet		=	false;
						}
					}
				}
				else if( tableName==CVSTR("extras") )
				{
					if( fieldName==CVSTR("stringdata") )
					{
						if( tok==CVSTR("\"\"") || tok==CVSTR("\'\'") )	valueString		=	CVSTR("");
						else											valueString		=	tok;
						valueStringSet		=	true;
						valueLongSet		=	false;
						valueNULLSet		=	false;
					}
					else
					{
						valueStringSet		=	false;
						if( tok==CVSTR("0L") )
						{
							valueLongSet		=	false;
							valueNULLSet		=	true;
						}
						else
						{
							valueLong			=	tok.GetLong();
							valueLongSet		=	true;
							valueNULLSet		=	false;
						}
					}
				}
				else if( tableName==CVSTR("scopes") )
				{
					if( fieldName==CVSTR("name") || fieldName==CVSTR("fullname") )
					{
						if( tok==CVSTR("\"\"") || tok==CVSTR("\'\'") )	valueString		=	CVSTR("");
						else											valueString		=	tok;
						valueStringSet	=	true;
						valueLongSet	=	false;
						valueNULLSet	=	false;
					}
					else
					{
						valueStringSet		=	false;
						if( tok==CVSTR("0L") )
						{
							valueLongSet		=	false;
							valueNULLSet		=	true;
						}
						else
						{
							valueLong			=	tok.GetLong();
							valueLongSet		=	true;
							valueNULLSet		=	false;
						}
					}
				}
				else if( tableName==CVSTR("names") )
				{
					if( fieldName==CVSTR("value") )
					{
						if( tok==CVSTR("\"\"") || tok==CVSTR("\'\'") )	valueString		=	CVSTR("");
						else											valueString		=	tok;
						valueStringSet	=	true;
						valueLongSet	=	false;
						valueNULLSet	=	false;
					}
					else
					{
						valueStringSet		=	false;
						if( tok==CVSTR("0L") )
						{
							valueLongSet		=	false;
							valueNULLSet		=	true;
						}
						else
						{
							valueLong			=	tok.GetLong();
							valueLongSet		=	true;
							valueNULLSet		=	false;
						}
					}
				}
				else if( tableName==CVSTR("definitions") )
				{
					valueStringSet		=	false;
					if( tok==CVSTR("0L") )
					{
						valueLongSet		=	false;
						valueNULLSet		=	true;
					}
					else
					{
						valueLong			=	tok.GetLong();
						valueLongSet		=	true;
						valueNULLSet		=	false;
					}
				}
				else if( tableName==CVSTR("prototypes") )
				{
					if( fieldName==CVSTR("fromname") || fieldName==CVSTR("toname") )
					{
						if( tok==CVSTR("\"\"") || tok==CVSTR("\'\'") )	valueString		=	CVSTR("");
						else											valueString		=	tok;
						valueStringSet	=	true;
						valueLongSet	=	false;
						valueNULLSet	=	false;
					}
					else
					{
						valueStringSet		=	false;
						if( tok==CVSTR("0L") )
						{
							valueLongSet		=	false;
							valueNULLSet		=	true;
						}
						else
						{
							valueLong			=	tok.GetLong();
							valueLongSet		=	true;
							valueNULLSet		=	false;
						}
					}
				}
				else if( tableName==CVSTR("returns") )
				{
					if( fieldName==CVSTR("name") )
					{
						if( tok==CVSTR("\"\"") || tok==CVSTR("\'\'") )	valueString		=	CVSTR("");
						else											valueString		=	tok;
						valueStringSet	=	true;
						valueLongSet	=	false;
						valueNULLSet	=	false;
					}
					else
					{
						valueStringSet		=	false;
						if( tok==CVSTR("0L") )
						{
							valueLongSet		=	false;
							valueNULLSet		=	true;
						}
						else
						{
							valueLong			=	tok.GetLong();
							valueLongSet		=	true;
							valueNULLSet		=	false;
						}
					}
				}
				else if( tableName==CVSTR("types") )
				{
					if( fieldName==CVSTR("name") )
					{
						if( tok==CVSTR("\"\"") || tok==CVSTR("\'\'") )	valueString		=	CVSTR("");
						else											valueString		=	tok;
						valueStringSet	=	true;
						valueLongSet	=	false;
						valueNULLSet	=	false;
					}
					else
					{
						valueStringSet		=	false;
						if( tok==CVSTR("0L") )
						{
							valueLongSet		=	false;
							valueNULLSet		=	true;
						}
						else
						{
							valueLong			=	tok.GetLong();
							valueLongSet		=	true;
							valueNULLSet		=	false;
						}
					}
				}
				else if( tableName==CVSTR("references") )
				{
					valueStringSet		=	false;
					if( tok==CVSTR("0L") )
					{
						valueLongSet		=	false;
						valueNULLSet		=	true;
					}
					else
					{
						valueLong			=	tok.GetLong();
						valueLongSet		=	true;
						valueNULLSet		=	false;
					}
				}
				else if( tableName==CVSTR("items") )
				{
					if( fieldName==CVSTR("name") )
					{
						if( tok==CVSTR("\"\"") || tok==CVSTR("\'\'") )	valueString		=	CVSTR("");
						else											valueString		=	tok;
						valueStringSet	=	true;
						valueLongSet	=	false;
						valueNULLSet	=	false;
					}
					else
					{
						valueStringSet		=	false;
						if( tok==CVSTR("0L") )
						{
							valueLongSet		=	false;
							valueNULLSet		=	true;
						}
						else
						{
							valueLong			=	tok.GetLong();
							valueLongSet		=	true;
							valueNULLSet		=	false;
						}
					}
				}

				if( query == NULL )
				{
					// CREATE REQUEST
					if( tableName==CVSTR("symbols") )		query = fSymbolsTable->NewQuery();
					else if( tableName==CVSTR("files") )	query = fFilesTable->NewQuery();
					else if( tableName==CVSTR("extras") )	query = fExtrasTable->NewQuery();
					else if( tableName==CVSTR("scopes") )		query = fScopesTable->NewQuery();
					else if( tableName==CVSTR("names") )		query = fSymbolNamesTable->NewQuery();
					else if( tableName==CVSTR("definitions") )	query = fSymbolDefinitionsTable->NewQuery();
					else if( tableName==CVSTR("prototypes") )	query = fSymbolPrototypesTable->NewQuery();
					else if( tableName==CVSTR("returns") )		query = fSymbolReturnsTable->NewQuery();
					else if( tableName==CVSTR("types") )		query = fSymbolTypesTable->NewQuery();
					else if( tableName==CVSTR("references") )	query = fSymbolReferencesTable->NewQuery();
					else if( tableName==CVSTR("items") )		query = fOutlineItemsTable->NewQuery();
				}
				
				DB4DComparator comparator;
				if( fieldValueComparator==CVSTR("==") )
				{
					if( valueString.EndsWith(CVSTR("*")) == true || valueString.EndsWith(CVSTR("@")) == true )
					{
						// Remove last char
						valueString.SubString(1, valueString.GetLength()-1);
						comparator=DB4D_BeginsWith;
					}
					else
					{
						(valueNULLSet==false) ? comparator=DB4D_Equal : comparator=DB4D_IsNull;
					}
				}
				else if( fieldValueComparator==CVSTR("!=") )	(valueNULLSet==false) ? comparator=DB4D_NotEqual : comparator=DB4D_IsNotNull;
				else if( fieldValueComparator==CVSTR(">") )		comparator=DB4D_Greater;
				else if( fieldValueComparator==CVSTR(">=") )	comparator=DB4D_GreaterOrEqual;
				else if( fieldValueComparator==CVSTR("<") )		comparator=DB4D_Lower;
				else if( fieldValueComparator==CVSTR("<=") )	comparator=DB4D_LowerOrEqual;
				
				if( query )
				{
					if( tableName==CVSTR("symbols") )
					{
						if( fieldName==CVSTR("name") )								query->AddCriteria(fSymbolNameField, comparator, valueString);
						else if( fieldName==CVSTR("prototypeid") )					query->AddCriteria(fSymbolPrototypeIDField, comparator, valueString);
						else if( fieldName==CVSTR("wafkind") )						query->AddCriteria(fSymbolWAFKindField, comparator, valueString);
						else if( fieldName==CVSTR("scriptdoctext") )				query->AddCriteria(fSymbolScriptDocTextField, comparator, valueString);
						else if( fieldName==CVSTR("returntypeid") )					query->AddCriteria(fSymbolReturnTypeIDField, comparator, valueString);
						else if( fieldName==CVSTR("symbolfullname") )				query->AddCriteria(fSymbolFullNameField, comparator, valueString);
						else if( fieldName==CVSTR("id") )							(valueNULLSet==true) ? query->AddCriteria(fSymbolIDField, comparator, VLong(0)) : query->AddCriteria(fSymbolIDField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("fileid") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolFileIDField, comparator, VLong(0)) : query->AddCriteria(fSymbolFileIDField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("ownerid") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolOwnerIDField, comparator, VLong(0)) : query->AddCriteria(fSymbolOwnerIDField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("kind") )							(valueNULLSet==true) ? query->AddCriteria(fSymbolKindField, comparator, VLong(0)) : query->AddCriteria(fSymbolKindField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("linenumber") )					(valueNULLSet==true) ? query->AddCriteria(fSymbolLineNumberField, comparator, VLong(0)) : query->AddCriteria(fSymbolLineNumberField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("linecompletionnumber") )			(valueNULLSet==true) ? query->AddCriteria(fSymbolLineCompletionNumberField, comparator, VLong(0)) : query->AddCriteria(fSymbolLineCompletionNumberField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("referenceid") )					(valueNULLSet==true) ? query->AddCriteria(fSymbolReferenceIDField, comparator, VLong(0)) : query->AddCriteria(fSymbolReferenceIDField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("undefinedstate") )				(valueNULLSet==true) ? query->AddCriteria(fSymbolUndefinedStateField, comparator, VLong(0)) : query->AddCriteria(fSymbolUndefinedStateField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("instancestate") )				(valueNULLSet==true) ? query->AddCriteria(fSymbolInstanceStateField, comparator, VLong(0)) : query->AddCriteria(fSymbolInstanceStateField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("referencestate") )				(valueNULLSet==true) ? query->AddCriteria(fSymbolReferenceStateField, comparator, VLong(0)) : query->AddCriteria(fSymbolReferenceStateField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("editionstate") )					(valueNULLSet==true) ? query->AddCriteria(fSymbolEditionStateField, comparator, VLong(0)) : query->AddCriteria(fSymbolEditionStateField, comparator, VLong(valueLong));
					}
					else if( tableName==CVSTR("files") )
					{
						if( fieldName==CVSTR("path") )								query->AddCriteria(fFilePathField, comparator, valueString);
						else if( fieldName==CVSTR("id") )							(valueNULLSet==true) ? query->AddCriteria(fFileIDField, comparator, VLong(0)) : query->AddCriteria(fFileIDField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("basefolder") )					(valueNULLSet==true) ? query->AddCriteria(fFileBaseFolderField, comparator, VLong(0)) : query->AddCriteria(fFileBaseFolderField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("executioncontext") )				(valueNULLSet==true) ? query->AddCriteria(fFileExecContextField, comparator, VLong(0)) : query->AddCriteria(fFileExecContextField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("modificationtimestamp") )		(valueNULLSet==true) ? query->AddCriteria(fFileModificationTimeField, comparator, VLong(0)) : query->AddCriteria(fFileModificationTimeField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("resolvedstate") )				(valueNULLSet==true) ? query->AddCriteria(fFileResolvedStateField, comparator, VLong(0)) : query->AddCriteria(fFileResolvedStateField, comparator, VLong(valueLong));
					}
					else if( tableName==CVSTR("extras") )
					{
						if( fieldName==CVSTR("stringdata") )						query->AddCriteria(fExtrasStringDataField, comparator, valueString);
						else if( fieldName==CVSTR("id") )							(valueNULLSet==true) ? query->AddCriteria(fExtraIDField, comparator, VLong(0)) : query->AddCriteria(fExtraIDField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("ownerid") )						(valueNULLSet==true) ? query->AddCriteria(fExtrasOwnerSymbolIDField, comparator, VLong(0)) : query->AddCriteria(fExtrasOwnerSymbolIDField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("kind") )							(valueNULLSet==true) ? query->AddCriteria(fExtrasKindField, comparator, VLong(0)) : query->AddCriteria(fExtrasKindField, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("integerdata") )					(valueNULLSet==true) ? query->AddCriteria(fExtrasIntegerDataField, comparator, VLong(0)) : query->AddCriteria(fExtrasIntegerDataField, comparator, VLong(valueLong));
					}
					else if( tableName==CVSTR("scopes") )
					{
						if( fieldName==CVSTR("name") )								query->AddCriteria(fScopeFieldName, comparator, valueString);
						else if( fieldName==CVSTR("fullname") )						query->AddCriteria(fScopeFieldFullName, comparator, valueString);
						else if( fieldName==CVSTR("status") )						(valueNULLSet==true) ? query->AddCriteria(fScopeFieldStatus, comparator, VLong(0)) : query->AddCriteria(fScopeFieldStatus, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("id") )							(valueNULLSet==true) ? query->AddCriteria(fScopeFieldId, comparator, VLong(0)) : query->AddCriteria(fScopeFieldId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("type") )							(valueNULLSet==true) ? query->AddCriteria(fScopeFieldType, comparator, VLong(0)) : query->AddCriteria(fScopeFieldType, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("sourceid") )						(valueNULLSet==true) ? query->AddCriteria(fScopeFieldSourceId, comparator, VLong(0)) : query->AddCriteria(fScopeFieldSourceId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("executioncontext") )				(valueNULLSet==true) ? query->AddCriteria(fScopeFieldExecutionContext, comparator, VLong(0)) : query->AddCriteria(fScopeFieldExecutionContext, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("parentid") )						(valueNULLSet==true) ? query->AddCriteria(fScopeFieldParentId, comparator, VLong(0)) : query->AddCriteria(fScopeFieldParentId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("constructorflag") )				(valueNULLSet==true) ? query->AddCriteria(fScopeFieldConstructorFlag, comparator, VLong(0)) : query->AddCriteria(fScopeFieldConstructorFlag, comparator, VLong(valueLong));
					}
					else if( tableName==CVSTR("names") )
					{
						if( fieldName==CVSTR("value") )								query->AddCriteria(fSymbolNameFieldValue, comparator, valueString);
						else if( fieldName==CVSTR("status") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolNameFieldStatus, comparator, VLong(0)) : query->AddCriteria(fSymbolNameFieldStatus, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("id") )							(valueNULLSet==true) ? query->AddCriteria(fSymbolNameFieldId, comparator, VLong(0)) : query->AddCriteria(fSymbolNameFieldId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("sourceid") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolNameFieldSourceId, comparator, VLong(0)) : query->AddCriteria(fSymbolNameFieldSourceId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("scopeid") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolNameFieldScopeId, comparator, VLong(0)) : query->AddCriteria(fSymbolNameFieldScopeId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("executioncontext") )				(valueNULLSet==true) ? query->AddCriteria(fSymbolNameFieldExecutionContext, comparator, VLong(0)) : query->AddCriteria(fSymbolNameFieldExecutionContext, comparator, VLong(valueLong));
					}
					else if( tableName==CVSTR("definitions") )
					{
						if( fieldName==CVSTR("id") )								(valueNULLSet==true) ? query->AddCriteria(fSymbolDefinitionFieldId, comparator, VLong(0)) : query->AddCriteria(fSymbolDefinitionFieldId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("status") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolDefinitionFieldStatus, comparator, VLong(0)) : query->AddCriteria(fSymbolDefinitionFieldStatus, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("scopeid") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolDefinitionFieldScopeId, comparator, VLong(0)) : query->AddCriteria(fSymbolDefinitionFieldScopeId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("nameid") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolDefinitionFieldNameId, comparator, VLong(0)) : query->AddCriteria(fSymbolDefinitionFieldNameId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("kind") )							(valueNULLSet==true) ? query->AddCriteria(fSymbolDefinitionFieldKind, comparator, VLong(0)) : query->AddCriteria(fSymbolDefinitionFieldKind, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("executioncontext") )				(valueNULLSet==true) ? query->AddCriteria(fSymbolDefinitionFieldExecutionContext, comparator, VLong(0)) : query->AddCriteria(fSymbolDefinitionFieldExecutionContext, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("sourceid") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolDefinitionFieldSourceId, comparator, VLong(0)) : query->AddCriteria(fSymbolDefinitionFieldSourceId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("linenumber") )					(valueNULLSet==true) ? query->AddCriteria(fSymbolDefinitionFieldLineNumber, comparator, VLong(0)) : query->AddCriteria(fSymbolDefinitionFieldLineNumber, comparator, VLong(valueLong));
					}
					else if( tableName==CVSTR("prototypes") )
					{
						if( fieldName==CVSTR("id") )								(valueNULLSet==true) ? query->AddCriteria(fSymbolPrototypeFieldId, comparator, VLong(0)) : query->AddCriteria(fSymbolPrototypeFieldId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("status") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolPrototypeFieldStatus, comparator, VLong(0)) : query->AddCriteria(fSymbolPrototypeFieldStatus, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("sourceid") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolPrototypeFieldSourceId, comparator, VLong(0)) : query->AddCriteria(fSymbolPrototypeFieldSourceId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("fromname") )						query->AddCriteria(fSymbolPrototypeFieldFromName, comparator, valueString);
						else if( fieldName==CVSTR("toname") )						query->AddCriteria(fSymbolPrototypeFieldToName, comparator, valueString);
						else if( fieldName==CVSTR("executioncontext") )				(valueNULLSet==true) ? query->AddCriteria(fSymbolPrototypeFieldExecutionContext, comparator, VLong(0)) : query->AddCriteria(fSymbolPrototypeFieldExecutionContext, comparator, VLong(valueLong));
					}
					else if( tableName==CVSTR("returns") )
					{
						if( fieldName==CVSTR("id") )								(valueNULLSet==true) ? query->AddCriteria(fSymbolReturnFieldId, comparator, VLong(0)) : query->AddCriteria(fSymbolReturnFieldId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("status") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolReturnFieldStatus, comparator, VLong(0)) : query->AddCriteria(fSymbolReturnFieldStatus, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("sourceid") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolReturnFieldSourceId, comparator, VLong(0)) : query->AddCriteria(fSymbolReturnFieldSourceId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("scopeid") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolReturnFieldScopeId, comparator, VLong(0)) : query->AddCriteria(fSymbolReturnFieldScopeId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("name") )							query->AddCriteria(fSymbolReturnFieldName, comparator, valueString);
					}
					else if( tableName==CVSTR("types") )
					{
						if( fieldName==CVSTR("id") )								(valueNULLSet==true) ? query->AddCriteria(fSymbolTypeFieldId, comparator, VLong(0)) : query->AddCriteria(fSymbolTypeFieldId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("status") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolTypeFieldStatus, comparator, VLong(0)) : query->AddCriteria(fSymbolTypeFieldStatus, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("sourceid") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolTypeFieldSourceId, comparator, VLong(0)) : query->AddCriteria(fSymbolTypeFieldSourceId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("definitionid") )					(valueNULLSet==true) ? query->AddCriteria(fSymbolTypeFieldDefinitionId, comparator, VLong(0)) : query->AddCriteria(fSymbolTypeFieldDefinitionId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("name") )							query->AddCriteria(fSymbolTypeFieldName, comparator, valueString);
					}
					else if( tableName==CVSTR("references") )
					{
						if( fieldName==CVSTR("id") )								(valueNULLSet==true) ? query->AddCriteria(fSymbolReferenceFieldId, comparator, VLong(0)) : query->AddCriteria(fSymbolReferenceFieldId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("status") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolReferenceFieldStatus, comparator, VLong(0)) : query->AddCriteria(fSymbolReferenceFieldStatus, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("type") )							(valueNULLSet==true) ? query->AddCriteria(fSymbolReferenceFieldType, comparator, VLong(0)) : query->AddCriteria(fSymbolReferenceFieldType, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("fromid") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolReferenceFieldFromId, comparator, VLong(0)) : query->AddCriteria(fSymbolReferenceFieldFromId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("toid") )							(valueNULLSet==true) ? query->AddCriteria(fSymbolReferenceFieldToId, comparator, VLong(0)) : query->AddCriteria(fSymbolReferenceFieldToId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("sourceid") )						(valueNULLSet==true) ? query->AddCriteria(fSymbolReferenceFieldSourceId, comparator, VLong(0)) : query->AddCriteria(fSymbolReferenceFieldSourceId, comparator, VLong(valueLong));
					}
					else if( tableName==CVSTR("items") )
					{
						if( fieldName==CVSTR("id") )								(valueNULLSet==true) ? query->AddCriteria(fOutlineItemFieldId, comparator, VLong(0)) : query->AddCriteria(fOutlineItemFieldId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("status") )						(valueNULLSet==true) ? query->AddCriteria(fOutlineItemFieldStatus, comparator, VLong(0)) : query->AddCriteria(fOutlineItemFieldStatus, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("sourceid") )						(valueNULLSet==true) ? query->AddCriteria(fOutlineItemFieldSourceId, comparator, VLong(0)) : query->AddCriteria(fOutlineItemFieldSourceId, comparator, VLong(valueLong));
						else if( fieldName==CVSTR("name") )							query->AddCriteria(fOutlineItemFieldName, comparator, valueString);
						else if( fieldName==CVSTR("line") )							(valueNULLSet==true) ? query->AddCriteria(fOutlineItemFieldLine, comparator, VLong(0)) : query->AddCriteria(fOutlineItemFieldLine, comparator, VLong(valueLong));
					}
				}
				expectedTokenType	=	4;
			}
				break;
				
			case 4: // CONJONCTION
			{
				if( query != NULL )
				{
					if( tok==CVSTR("and") )
						query->AddLogicalOperator(DB4D_And);
					else if( tok==CVSTR("or") )
						query->AddLogicalOperator(DB4D_OR);
				}
				
				expectedTokenType = 0;
			}
				break;
				
			case 5: // SPECIAL COMMAND FIELDSOF
			{
				JSLog log;
				log.SetTitle("STLR");

				tableName = tok;
				
				if( tableName==CVSTR("files") )
				{
					log.Append(CVSTR("Table"), CVSTR("files"));
					
					VString fields;
					fields.AppendString(CVSTR("path"));
					fields.AppendString(CVSTR(", id"));
					fields.AppendString(CVSTR(", basefolder"));
					fields.AppendString(CVSTR(", executioncontext"));
					fields.AppendString(CVSTR(", modificationtimestamp"));
					fields.AppendString(CVSTR(", resolvedstate"));
					log.Append(CVSTR("Fields"), fields);
				}
				else if( tableName==CVSTR("scopes") )
				{
					log.Append(CVSTR("FieldsOf"), CVSTR("scopes"));
					
					VString fields;
					fields.AppendString(CVSTR("name"));
					fields.AppendString(CVSTR(", fullname"));
					fields.AppendString(CVSTR(", status"));
					fields.AppendString(CVSTR(", id"));
					fields.AppendString(CVSTR(", type"));
					fields.AppendString(CVSTR(", sourceid"));
					fields.AppendString(CVSTR(", executioncontext"));
					fields.AppendString(CVSTR(", parentid"));
					fields.AppendString(CVSTR(", constructorflag"));
					log.Append(CVSTR("Fields"), fields);
				}
				else if( tableName==CVSTR("names") )
				{
					log.Append(CVSTR("Table"), CVSTR("names"));
					
					VString fields;
					fields.AppendString(CVSTR("value"));
					fields.AppendString(CVSTR(", status"));
					fields.AppendString(CVSTR(", id"));
					fields.AppendString(CVSTR(", sourceid"));
					fields.AppendString(CVSTR(", scopeid"));
					fields.AppendString(CVSTR(", executioncontext"));
					log.Append(CVSTR("Fields"), fields);
				}
				else if( tableName==CVSTR("definitions") )
				{
					log.Append(CVSTR("Table"), CVSTR("definitions"));
					
					VString fields;
					fields.AppendString(CVSTR("id"));
					fields.AppendString(CVSTR(", status"));
					fields.AppendString(CVSTR(", scopeid"));
					fields.AppendString(CVSTR(", nameid"));
					fields.AppendString(CVSTR(", kind"));
					fields.AppendString(CVSTR(", executioncontext"));
					fields.AppendString(CVSTR(", sourceid"));
					fields.AppendString(CVSTR(", linenumber"));
					log.Append(CVSTR("Fields"), fields);
				}
				else if( tableName==CVSTR("prototypes") )
				{
					log.Append(CVSTR("Table"), CVSTR("prototypes"));
					
					VString fields;
					fields.AppendString(CVSTR("id"));
					fields.AppendString(CVSTR(", status"));
					fields.AppendString(CVSTR(", sourceid"));
					fields.AppendString(CVSTR(", fromname"));
					fields.AppendString(CVSTR(", toname"));
					fields.AppendString(CVSTR(", executioncontext"));
					log.Append(CVSTR("Fields"), fields);
				}
				else if( tableName==CVSTR("returns") )
				{
					log.Append(CVSTR("Table"), CVSTR("returns"));
					
					VString fields;
					fields.AppendString(CVSTR("id"));
					fields.AppendString(CVSTR(", status"));
					fields.AppendString(CVSTR(", sourceid"));
					fields.AppendString(CVSTR(", scopeid"));
					fields.AppendString(CVSTR(", name"));
					log.Append(CVSTR("Fields"), fields);
				}
				else if( tableName==CVSTR("types") )
				{
					log.Append(CVSTR("Table"), CVSTR("types"));
					
					VString fields;
					fields.AppendString(CVSTR("id"));
					fields.AppendString(CVSTR(", status"));
					fields.AppendString(CVSTR(", sourceid"));
					fields.AppendString(CVSTR(", definitionid"));
					fields.AppendString(CVSTR(", name"));
					log.Append(CVSTR("Fields"), fields);
				}
				else if( tableName==CVSTR("references") )
				{
					log.Append(CVSTR("Table"), CVSTR("references"));
					
					VString fields;
					fields.AppendString(CVSTR("id"));
					fields.AppendString(CVSTR(", status"));
					fields.AppendString(CVSTR(", type"));
					fields.AppendString(CVSTR(", fromid"));
					fields.AppendString(CVSTR(", toid"));
					fields.AppendString(CVSTR(", sourceid"));
					log.Append(CVSTR("Fields"), fields);
				}
				else if( tableName==CVSTR("items") )
				{
					log.Append(CVSTR("Table"), CVSTR("items"));
					
					VString fields;
					fields.AppendString(CVSTR("id"));
					fields.AppendString(CVSTR(", status"));
					fields.AppendString(CVSTR(", sourceid"));
					fields.AppendString(CVSTR(", name"));
					fields.AppendString(CVSTR(", line"));
					log.Append(CVSTR("Fields"), fields);
				}
				else
				{
					JSLog error;
					error.SetTitle("Symbol table live requesting");
					error.Append("Error", "Unknown table name (allowed : scopes, names, definitions, prototypes, returns, types, references, items, files)");
				}
				return;
			}

			default:
				break;
				
		} // ENDSWITCH
		
	} // ENDFOR
	
	
	// EXECUTE REQUEST
	CDB4DSelection* results = NULL;
	if( tableName==CVSTR("symbols") )			results = fSymbolsTable->ExecuteQuery(query, inContext);
	else if( tableName==CVSTR("files") )		results = fFilesTable->ExecuteQuery(query, inContext);
	else if( tableName==CVSTR("extras") )		results = fExtrasTable->ExecuteQuery(query, inContext);
	else if( tableName==CVSTR("scopes") )		results = fScopesTable->ExecuteQuery(query, inContext);
	else if( tableName==CVSTR("names") )		results = fSymbolNamesTable->ExecuteQuery(query, inContext);
	else if( tableName==CVSTR("definitions") )	results = fSymbolDefinitionsTable->ExecuteQuery(query, inContext);
	else if( tableName==CVSTR("prototypes") )	results = fSymbolPrototypesTable->ExecuteQuery(query, inContext);
	else if( tableName==CVSTR("returns") )		results = fSymbolReturnsTable->ExecuteQuery(query, inContext);
	else if( tableName==CVSTR("types") )		results = fSymbolTypesTable->ExecuteQuery(query, inContext);
	else if( tableName==CVSTR("references") )	results = fSymbolReferencesTable->ExecuteQuery(query, inContext);
	else if( tableName==CVSTR("items") )		results = fOutlineItemsTable->ExecuteQuery(query, inContext);
	
	if( results )
	{
		RecIDType total = results->CountRecordsInSelection( inContext );
		for( RecIDType index=1; index<=total; index++)
		{
			CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
			if( record )
			{
				if( tableName==CVSTR("symbols") )
				{
					Symbols::ISymbol* symbol = SymbolFromRecord(fSymbolsTable, record, inContext);
					if( symbol )
					{
						JSLog log;
						log.SetTitle( CVSTR("STLR") );
						log.Append( CVSTR("Traced"),		CVSTR("Symbol"));
						log.Append( CVSTR("FileID"),		symbol->GetFile()->GetID());
						log.Append( CVSTR("ID"),			symbol->GetID());
						log.Append( CVSTR("Name"),			symbol->GetName());
						log.Append( CVSTR("OwnID"),			symbol->GetOwner() ? symbol->GetOwner()->GetID() : 0);
						log.Append( CVSTR("OwnName"),		symbol->GetOwner() ? symbol->GetOwner()->GetName() : "");
						log.Append( CVSTR("Kind"),			symbol->GetKindString(symbol->GetKind()));
						log.Append( CVSTR("XKind"),			symbol->GetAuxKindString(symbol->GetAuxillaryKindInformation()));
						log.Append( CVSTR("Undef"),			symbol->GetUndefinedState());
						log.Append( CVSTR("Inst"),			symbol->GetInstanceState());
						log.Append( CVSTR("Ref"),			symbol->GetReferenceState());
						log.Append( CVSTR("Edited"),		symbol->GetEditionState());
						log.Append( CVSTR("FName"),			symbol->GetFullName());
					}
					ReleaseRefCountable(&symbol);
				}
				else if( tableName==CVSTR("files") )
				{
					Symbols::IFile* file = FileFromRecord(fFilesTable, record, inContext);
					if( file )
					{
						JSLog log;
						log.SetTitle( CVSTR("STLR") );
						log.Append( CVSTR("Traced"),		CVSTR("File"));
						log.Append( CVSTR("ID"),			file->GetID());
						log.Append( CVSTR("BFolder"),		file->GetBaseFolder());
						
						sLONG executionContext = file->GetExecutionContext();
						if( executionContext == eSymbolFileExecContextClient )		{ log.Append( CVSTR("ExecutionContext"), "1 : Client"); }
						else if( executionContext == eSymbolFileExecContextServer )	{ log.Append( CVSTR("ExecutionContext"), "2 : Server"); }
						
						log.Append( CVSTR("RState"),		file->GetResolveState());
						log.Append( CVSTR("TimeStamp"),		file->GetModificationTimestamp());
						log.Append( CVSTR("Path"),			file->GetPath());
					}
					ReleaseRefCountable(&file);
				}
				else if( tableName==CVSTR("scopes") )
				{
					JSLog log;
					log.SetTitle(CVSTR("STLR"));
					log.Append( CVSTR("Traced"),			CVSTR("Scope"));
					log.Append( CVSTR("ID"),				GetIntegerField(record, fScopeFieldId, NULL));
					log.Append( CVSTR("Status"),			GetIntegerField(record, fScopeFieldStatus, NULL));
					log.Append( CVSTR("SourceId"),			GetIntegerField(record, fScopeFieldSourceId, NULL));

					log.Append( CVSTR("ConstructorFlag"),	GetBooleanField(record,	fScopeFieldConstructorFlag));

					sLONG executionContext = GetIntegerField(record, fScopeFieldExecutionContext, NULL);
					if( executionContext == eSymbolFileExecContextClient )		{ log.Append( CVSTR("ExecutionContext"), "1 : Client"); }
					else if( executionContext == eSymbolFileExecContextServer )	{ log.Append( CVSTR("ExecutionContext"), "2 : Server"); }
					
					sLONG type = GetIntegerField(record, fScopeFieldType, NULL);
					if( type == eSymbolScopeTypeAll )						{ log.Append( CVSTR("Type"), "0 : All"); }
					else if( type == eSymbolScopeTypeUndefined )			{ log.Append( CVSTR("Type"), "1 : Undefined"); }
					else if( type == eSymbolScopeTypeGlobal )				{ log.Append( CVSTR("Type"), "2 : Global"); }
					else if( type == eSymbolScopeTypeFunctionDeclaration )	{ log.Append( CVSTR("Type"), "3 : FunctionDeclaration"); }
					else if( type == eSymbolScopeTypeFunctionExpression )	{ log.Append( CVSTR("Type"), "4 : FunctionExpression"); }
					else if( type == eSymbolScopeTypeCatch )				{ log.Append( CVSTR("Type"), "5 : Catch"); }
					else if( type == eSymbolScopeTypeWith )					{ log.Append( CVSTR("Type"), "6 : With"); }
					else if( type == eSymbolScopeTypeObject )				{ log.Append( CVSTR("Type"), "7 : Object"); }

					log.Append( CVSTR("FromLine"),			GetIntegerField(record, fScopeFieldFromLine, NULL));
					log.Append( CVSTR("ToLine"),			GetIntegerField(record, fScopeFieldToLine, NULL));
					log.Append( CVSTR("FromOffset"),		GetIntegerField(record, fScopeFieldFromOffset, NULL));
					log.Append( CVSTR("ToOffset"),			GetIntegerField(record, fScopeFieldToOffset, NULL));
					log.Append( CVSTR("ParentID"),			GetIntegerField(record, fScopeFieldParentId, NULL));
					log.Append( CVSTR("Name"),				GetStringField(record,	fScopeFieldName));
					log.Append( CVSTR("FullName"),			GetStringField(record,	fScopeFieldFullName));
				}
				else if( tableName==CVSTR("names") )
				{
					JSLog log;
					log.SetTitle(CVSTR("STLR"));
					log.Append( CVSTR("Traced"),			CVSTR("Name"));
					log.Append( CVSTR("ID"),				GetIntegerField(record, fSymbolNameFieldId, NULL));
					log.Append( CVSTR("Status"),			GetIntegerField(record, fSymbolNameFieldStatus, NULL));
					log.Append( CVSTR("SourceId"),			GetIntegerField(record, fSymbolNameFieldSourceId, NULL));
					
					sLONG executionContext = GetIntegerField(record, fSymbolNameFieldExecutionContext, NULL);
					if( executionContext == eSymbolFileExecContextClient )		{ log.Append( CVSTR("ExecutionContext"), "1 : Client"); }
					else if( executionContext == eSymbolFileExecContextServer )	{ log.Append( CVSTR("ExecutionContext"), "2 : Server"); }
					
					log.Append( CVSTR("ScopeId"),			GetIntegerField(record, fSymbolNameFieldScopeId, NULL));
					log.Append( CVSTR("Value"),				GetStringField(record,	fSymbolNameFieldValue));
				}
				else if( tableName==CVSTR("definitions") )
				{
					JSLog log;
					log.SetTitle(CVSTR("STLR"));
					log.Append( CVSTR("Traced"),			CVSTR("Definition"));
					log.Append( CVSTR("ID"),				GetIntegerField(record, fSymbolDefinitionFieldId, NULL));
					log.Append( CVSTR("Status"),			GetIntegerField(record, fSymbolDefinitionFieldStatus, NULL));
					log.Append( CVSTR("SourceId"),			GetIntegerField(record, fSymbolDefinitionFieldSourceId, NULL));

					sLONG executionContext = GetIntegerField(record, fSymbolDefinitionFieldExecutionContext, NULL);
					if( executionContext == eSymbolFileExecContextClient )		{ log.Append( CVSTR("ExecutionContext"), "1 : Client"); }
					else if( executionContext == eSymbolFileExecContextServer )	{ log.Append( CVSTR("ExecutionContext"), "2 : Server"); }
					
					log.Append( CVSTR("ScopeId"),			GetIntegerField(record, fSymbolDefinitionFieldScopeId, NULL));
					
					log.Append( CVSTR("LineNumber"),	GetIntegerField(record, fSymbolDefinitionFieldLineNumber, NULL));

					sLONG nameId = GetIntegerField(record, fSymbolDefinitionFieldNameId, NULL);
					VString name;
					name.AppendLong(nameId);
					name.AppendString(" : ");
					VString temp;
					GetName(nameId, temp, inContext);
					name.AppendString(temp);
					log.Append( CVSTR("Name"), name);

					sLONG kind = GetIntegerField(record, fSymbolDefinitionFieldKind, NULL);
					if( kind == eSymbolDefinitionKindKeyword )					{ log.Append( CVSTR("Kind"), "00 : Keyword"); }
					else if( kind == eSymbolDefinitionKindFunctionDeclaration )	{ log.Append( CVSTR("Kind"), "01 : FunctionDeclaration"); }
					else if( kind == eSymbolDefinitionKindFunctionExpression )	{ log.Append( CVSTR("Kind"), "02 : FunctionExpression"); }
					else if( kind == eSymbolDefinitionKindCatch )				{ log.Append( CVSTR("Kind"), "03 : Catch"); }
					else if( kind == eSymbolDefinitionKindWith )				{ log.Append( CVSTR("Kind"), "04 : With"); }
					else if( kind == eSymbolDefinitionKindUndefined )			{ log.Append( CVSTR("Kind"), "05 : Undefined"); }
					else if( kind == eSymbolDefinitionKindObject )				{ log.Append( CVSTR("Kind"), "06 : Object"); }
					else if( kind == eSymbolDefinitionKindProperty )			{ log.Append( CVSTR("Kind"), "07 : Property"); }
					else if( kind == eSymbolDefinitionKindInstanceProperty )	{ log.Append( CVSTR("Kind"), "08 : InstanceProperty"); }
					else if( kind == eSymbolDefinitionKindVariable )			{ log.Append( CVSTR("Kind"), "09 : Variable"); }
					else if( kind == eSymbolDefinitionKindLocalVariable )		{ log.Append( CVSTR("Kind"), "10 : LocalVariable"); }
				}
				else if( tableName==CVSTR("prototypes") )
				{
					JSLog log;
					log.SetTitle(CVSTR("STLR"));
					log.Append( CVSTR("Traced"),			CVSTR("Prototypes"));
					log.Append( CVSTR("ID"),				GetIntegerField(record, fSymbolPrototypeFieldId, NULL));
					log.Append( CVSTR("Status"),			GetIntegerField(record, fSymbolPrototypeFieldStatus, NULL));
					log.Append( CVSTR("SourceId"),			GetIntegerField(record, fSymbolPrototypeFieldSourceId, NULL));
					
					sLONG executionContext = GetIntegerField(record, fSymbolPrototypeFieldExecutionContext, NULL);
					if( executionContext == eSymbolFileExecContextClient )		{ log.Append( CVSTR("ExecutionContext"), "1 : Client"); }
					else if( executionContext == eSymbolFileExecContextServer )	{ log.Append( CVSTR("ExecutionContext"), "2 : Server"); }
					
					log.Append( CVSTR("FromName"),			GetStringField(record,	fSymbolPrototypeFieldFromName));
					log.Append( CVSTR("ToName"),			GetStringField(record,	fSymbolPrototypeFieldToName));
				}
				else if( tableName==CVSTR("returns") )
				{
					JSLog log;
					log.SetTitle(CVSTR("STLR"));
					log.Append( CVSTR("Traced"),	CVSTR("Returns"));
					log.Append( CVSTR("ID"),		GetIntegerField(record, fSymbolReturnFieldId, NULL));
					log.Append( CVSTR("Status"),	GetIntegerField(record, fSymbolReturnFieldStatus, NULL));
					log.Append( CVSTR("SourceId"),	GetIntegerField(record, fSymbolReturnFieldSourceId, NULL));
					log.Append( CVSTR("ScopeId"),	GetIntegerField(record, fSymbolReturnFieldScopeId, NULL));
					log.Append( CVSTR("Name"),		GetStringField(record,	fSymbolReturnFieldName));
				}
				else if( tableName==CVSTR("types") )
				{
					JSLog log;
					log.SetTitle(CVSTR("STLR"));
					log.Append( CVSTR("Traced"),	CVSTR("Types"));
					log.Append( CVSTR("ID"),			GetIntegerField(record, fSymbolTypeFieldId, NULL));
					
					sLONG definitionId = GetIntegerField(record, fSymbolTypeFieldDefinitionId, NULL);
					VString name;
					GetDefinitionName(definitionId, name, inContext);
					
					VString definitionName;
					definitionName.AppendLong(definitionId);
					definitionName.AppendString(" : ");
					definitionName.AppendString(name);
					log.Append( CVSTR("DefinitionId"),	definitionName);
					
					log.Append( CVSTR("Name"),			GetStringField(record,	fSymbolTypeFieldName));
				}
				else if( tableName==CVSTR("references") )
				{
					JSLog log;
					log.SetTitle(CVSTR("STLR"));
					log.Append( CVSTR("Traced"),	CVSTR("References"));
					log.Append( CVSTR("ID"),		GetIntegerField(record, fSymbolReferenceFieldId, NULL));
					log.Append( CVSTR("Status"),	GetIntegerField(record, fSymbolReferenceFieldStatus, NULL));
					log.Append( CVSTR("Type"),		GetIntegerField(record, fSymbolReferenceFieldType, NULL));
					log.Append( CVSTR("SourceId"),	GetIntegerField(record, fSymbolReferenceFieldSourceId, NULL));
					log.Append( CVSTR("FromId"),	GetIntegerField(record, fSymbolReferenceFieldFromId, NULL));
					log.Append( CVSTR("ToId"),		GetIntegerField(record,	fSymbolReferenceFieldToId, NULL));
				}
				else if( tableName==CVSTR("items") )
				{
					JSLog log;
					log.SetTitle(CVSTR("STLR"));
					log.Append( CVSTR("Traced"),	CVSTR("OutlineItems"));
					log.Append( CVSTR("ID"),		GetIntegerField(record, fOutlineItemFieldId, NULL));
					log.Append( CVSTR("SourceId"),	GetIntegerField(record, fOutlineItemFieldSourceId, NULL));
					log.Append( CVSTR("Name"),		GetStringField(record,  fOutlineItemFieldName));
					log.Append( CVSTR("Line"),		GetIntegerField(record, fOutlineItemFieldLine, NULL));
					log.Append( CVSTR("Status"),	GetIntegerField(record, fOutlineItemFieldStatus, NULL));
				}
			}
			ReleaseRefCountable(&record);
		}

		JSLog log;
		log.SetTitle( CVSTR("STLR") );
		log.Append( CVSTR("Count"), total);
	}

	ReleaseRefCountable(&results);
	ReleaseRefCountable(&query);
}
#endif


void VSymbolTable::InnerSymbolTable::DumpSymbols(sLONG inFileId, CDB4DBaseContext* inContext)
{
	if( fDatabase && fSymbolsTable )
	{
		// Let's get a new query object for us to fill out
		CDB4DQuery* query = fSymbolsTable->NewQuery();
		if( query )
		{
			VLong fileId(inFileId);
			query->AddCriteria(fSymbolFileIDField, DB4D_Equal, fileId);
			
			CDB4DSelection* results = fSymbolsTable->ExecuteQuery(query, inContext);
			if( results )
			{
				RecIDType total = results->CountRecordsInSelection(inContext);
				for( RecIDType index=1; index<=total; index++)
				{
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					{
						Symbols::ISymbol* current = SymbolFromRecord(fSymbolsTable, record, inContext);
						if( current )
						{
							JSLog logSym;
							logSym.SetTitle( CVSTR("SymbolTable::DumpSymbols") );
							logSym.Append( CVSTR("FileID"),			current->GetFile()->GetID());
							logSym.Append( CVSTR("ID"),				current->GetID());
							logSym.Append( CVSTR("Name"),			current->GetName());
							logSym.Append( CVSTR("Kind"),			current->GetKindString(current->GetKind()));
							logSym.Append( CVSTR("XKind"),			current->GetAuxKindString(current->GetAuxillaryKindInformation()));
							logSym.Append( CVSTR("Undef"),			current->GetUndefinedState());
							logSym.Append( CVSTR("Inst"),			current->GetInstanceState());
							logSym.Append( CVSTR("Ref"),			current->GetReferenceState());
							logSym.Append( CVSTR("Edited"),			current->GetEditionState());
							logSym.Append( CVSTR("FName"),			current->GetFullName());
							logSym.Print();
							
							ReleaseRefCountable(&current);
						}
						
						ReleaseRefCountable(&record);
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
}

void VSymbolTable::InnerSymbolTable::RetainSymbolsFromFullName(const VString& inFullName, std::vector<Symbols::ISymbol*>& outSymbols, CDB4DBaseContext* inContext)
{
	if( fDatabase && fSymbolsTable )
	{
		// Let's get a new query object for us to fill out
		CDB4DQuery* query = fSymbolsTable->NewQuery();
		if( query )
		{
			query->AddCriteria(fSymbolFullNameField, DB4D_Equal, inFullName);
			
			CDB4DSelection* results = fSymbolsTable->ExecuteQuery(query, inContext);
			if( results )
			{
				RecIDType total = results->CountRecordsInSelection(inContext);
				for( RecIDType index=1; index<=total; index++)
				{
					CDB4DRecord* record = results->LoadSelectedRecord( index, DB4D_Do_Not_Lock, inContext );
					{
						Symbols::ISymbol* current = SymbolFromRecord(fSymbolsTable, record, inContext);
						outSymbols.push_back(current);
						
						ReleaseRefCountable(&record);
					}
				}
				
				ReleaseRefCountable(&results);
			}
			
			ReleaseRefCountable(&query);
		}
	}
}





ISymbolTable* NewSymbolTable(const bool& inUseNextGen)
{
	return new VSymbolTable(inUseNextGen);
}
