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

#include "JavaScriptAST.h"
#include "SymbolTable.h"

using namespace JavaScriptAST;

class DeclarationVisitor : public Visitor
{
private:
	JavaScriptParserDelegate *fDelegate;
	std::vector< Symbols::ISymbol * > fOwnershipStack;
	ISymbolTable *fSymTable;
	Symbols::IFile *fOwningFile;
	std::vector< Symbols::ISymbol * > fSymbolsToAdd, fSymbolsToAddToStatement, fSymbolsToUpdate;
	std::multimap< Symbols::ISymbol *, Symbols::IExtraInfo * > fExtraInfoToAdd;

	virtual bool VisitProgramNodeLeave( class ProgramNode *inNode );

	virtual bool VisitFunctionExpressionNodeEnter( class FunctionExpressionNode *inNode );
	virtual bool VisitFunctionExpressionNodeLeave( class FunctionExpressionNode *inNode );
	virtual bool VisitFunctionDeclarationStatementNodeEnter( class FunctionDeclarationStatementNode *inNode );
	virtual bool VisitFunctionDeclarationArgumentsNode( class FunctionDeclarationArgumentsNode *inNode );
	virtual bool VisitFunctionDeclarationStatementNodeLeave( class FunctionDeclarationStatementNode *inNode );
	virtual bool VisitVariableDeclarationStatementNodeEnter( class VariableDeclarationStatementNode *inNode );
	virtual bool VisitAssignExpressionNode( class AssignExpressionNode *inNode );
	virtual bool VisitObjectLiteralNodeEnter( class ObjectLiteralNode *inNode );
	virtual bool VisitObjectLiteralFieldNodeEnter( class ObjectLiteralFieldNode *inNode );
	virtual bool VisitObjectLiteralNodeLeave( class ObjectLiteralNode *inNode );
	virtual bool VisitForExpressionInClauseNode( class ForExpressionInClauseNode *inNode );
	virtual bool VisitReturnStatementNode( class ReturnStatementNode *inNode );
	virtual bool VisitCatchStatementNodeEnter( class CatchStatementNode *inNode );
	virtual bool VisitCatchStatementNodeLeave( class CatchStatementNode *inNode );

	// Note that GetSubSymbols retains the data in the vector, so it is up to the caller to release the 
	// vector's contents when they are done.
	void GetSubSymbols( Symbols::ISymbol *inOwner, std::vector< Symbols::ISymbol * > &outSubSymbols );
	void GetLikeNamedSubSymbols( Symbols::ISymbol *inOwner, const VString &inText, std::vector< Symbols::ISymbol * > &outSubSymbols );
	Symbols::ISymbol *GetLikeNamedSubSymbol( Symbols::ISymbol *inOwner, const VString &inText );

	Symbols::ISymbol *GetSymbolFromLeftHandSideNode( class LeftHandSideExpressionNode *inNode, bool inCreateIfNeeded = true );
	bool UnqualifiedLookup( const VString &inIdent, Symbols::ISymbol **outSymbol );

	// This is a close cousin to GetSymbolFromLeftHandSideNode, but differs in a major way that you need 
	// to be aware of.  When you use QualifiedLookup, it walks up the prototype chain, looking for the
	// symbols you care about.  But the GetSymbolFromLeftHandSideNode does not -- this is an important
	// distinction to make when you're going to be adding symbols or not.  If you're trying to add symbols
	// because the expression is on the lhs, then you want to call GetSymbolFromLeftHandSideNode.  But if
	// the expression is on the rhs where you're not going to be adding symbols, you want to call QualifiedLookup.
	bool QualifiedLookup( class Node *inNode, Symbols::ISymbol **outSymbol );

	// "Fake" assignment is the case where you gin up a fake LHS symbol so that you can make this call.  When you do
	// that, you want to make sure that this call doesn't make associations with the LHS symbol passed in, since that
	// symbol doesn't really exist.  A good example of this would be when working with return statements.
	bool VisitRightHandSideOfAssignment( Symbols::ISymbol *inLHS, class Node *inRHS, bool inForFakeAssignment = false );

	Symbols::ISymbol	*GetThis();
	void				CreateNewInstance( Symbols::ISymbol *inLHS, Symbols::ISymbol *inOfThisType, const sLONG inLevel = 0);
	void				CreateNewInternalInstance( Symbols::ISymbol *inLHS, Symbols::ISymbol *inOfThisType);
	Symbols::ISymbol*	CreateSymbolPrototype( Symbols::ISymbol *inSym );
	XBOX::VString		BuildFunctionSignature(const VString& inIdentifier, class FunctionDeclarationArgumentsNode *inArgs);

	void SetSymbolReturnTypesFromScriptDoc(Symbols::ISymbol* inSym,  bool inAddToUpdateList = true, bool inUpdateSymbol = false);
	void SetSymbolClassesFromScriptDoc(Symbols::ISymbol* inSym,  bool inAddToUpdateList = true, bool inUpdateSymbol = false);
	void SetSymbolTypesFromScriptDoc(Symbols::ISymbol* inSym, bool inAddToUpdateList = true, bool inUpdateSymbol = false);
	void SetSymbolPrototypes(Symbols::ISymbol* inSym, const VectorOfVString& inTypes, bool inAddToUpdateList, bool inUpdateSymbol);
	void SetSymbolClassKindFromScriptDoc(Symbols::ISymbol* inSym);
	
	Symbols::ISymbol* RetainSymbolPrototype(const VString& inSymbolName, const ESymbolFileExecContext& inExecContext);

public:
	DeclarationVisitor( ISymbolTable *inSymTable, Symbols::IFile *inFile, JavaScriptParserDelegate *inDelegate );
	virtual ~DeclarationVisitor();

	ISymbolTable *GetSymbolTable() { return fSymTable; }
};

DeclarationVisitor::DeclarationVisitor( ISymbolTable *inSymTable, Symbols::IFile *inFile, JavaScriptParserDelegate *inDelegate ) : 
fSymTable( inSymTable ), fOwningFile( inFile ), fDelegate( inDelegate )
{
	// The global symbol table is always the root of our ownership stack
	fOwnershipStack.push_back( NULL );

	if (fDelegate) {
		// If we have a delegate, ask them for the context symbol for adding new declarations.
		// We will add this to the ownership list after the global symbol table.
		Symbols::ISymbol *contextSymbol = fDelegate->GetDeclarationContext( );
		if (contextSymbol)
			fOwnershipStack.push_back( contextSymbol );
	}
}

DeclarationVisitor::~DeclarationVisitor()
{
	for (std::vector< Symbols::ISymbol * >::iterator iter = fSymbolsToAdd.begin(); iter != fSymbolsToAdd.end(); ++iter)
	{
		(*iter)->Release();
	}

	for (std::multimap< Symbols::ISymbol *, Symbols::IExtraInfo * >::iterator iter = fExtraInfoToAdd.begin(); iter != fExtraInfoToAdd.end(); ++iter)
	{
		iter->first->Release();
		iter->second->Release();
	}

	for (std::vector< Symbols::ISymbol * >::iterator iter = fSymbolsToUpdate.begin(); iter != fSymbolsToUpdate.end(); ++iter)
	{
		(*iter)->Release();
	}
}

void DeclarationVisitor::GetSubSymbols( Symbols::ISymbol *inOwner, std::vector< Symbols::ISymbol * > &outSubSymbols )
{
	// First, search the database for any entries the owner contains
	if (!inOwner || (inOwner && inOwner->HasID())) {
		outSubSymbols = fSymTable->GetNamedSubSymbols( inOwner );
	}

	// Then add on any entries we can locate from our set of symbols to add
	for (std::vector< Symbols::ISymbol * >::iterator iter = fSymbolsToAdd.begin(); iter != fSymbolsToAdd.end(); ++iter) {
		if ((*iter)->GetOwner() == inOwner) {
			(*iter)->Retain();
			outSubSymbols.push_back( *iter );
		}
	}
}

void DeclarationVisitor::GetLikeNamedSubSymbols( Symbols::ISymbol *inOwner, const VString &inText, std::vector< Symbols::ISymbol * > &outSubSymbols )
{
	// See if we can locate the subsymbol in our set of symbols we want to add
	for (std::vector< Symbols::ISymbol * >::iterator iter = fSymbolsToAdd.begin(); iter != fSymbolsToAdd.end(); ++iter)
	{
		if ((*iter)->GetOwner() == inOwner && (*iter)->GetName().EqualToString( inText, true ))
		{
			outSubSymbols.push_back( *iter );
			(*iter)->Retain();
		}
	}

	// Check the symbol table too
	if ( !inOwner || (inOwner && inOwner->HasID() ) )
	{
        Symbols::IFile* pOwnerFile = (inText == "exports") ? this->fOwningFile : NULL;
		std::vector< Symbols::ISymbol * > syms = fSymTable->GetSymbolsByName( inOwner, inText, pOwnerFile );
		for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter)
			outSubSymbols.push_back( *iter );
	}
}

Symbols::ISymbol *DeclarationVisitor::GetLikeNamedSubSymbol( Symbols::ISymbol *inOwner, const VString &inText )
{
	Symbols::ISymbol *ret = NULL;
	std::vector< Symbols::ISymbol * > subSymbols;

	GetLikeNamedSubSymbols(inOwner, inText, subSymbols);
	for (std::vector< Symbols::ISymbol * >::iterator iter = subSymbols.begin(); iter != subSymbols.end(); ++iter)
	{
		if ( iter == subSymbols.begin() )
		{
			ret = *iter;
			ret->Retain();
		}

		(*iter)->Release();
	}

	return ret;
}

bool DeclarationVisitor::VisitReturnStatementNode( ReturnStatementNode *inNode )
{
	Symbols::ISymbol *functionOwner = fOwnershipStack.back();
	if (!functionOwner || ! functionOwner->IsFunctionKind() )
		return true;

	// We have a function, let's assign it some return type information based on the node
	// we have here.  The way we will do this is by ginning up a temporary symbol and pretending
	// it is the right-hand side of an assignment expression (which is a typical way to handle
	// return statements anyways).  The prototype of that symbol will be the function's return type.
	Symbols::ISymbol *fakeSym = new VSymbol();
	Node *expressionNode = inNode->GetExpression();
	bool processExpression = VisitRightHandSideOfAssignment( fakeSym, expressionNode, true );

	// The fake symbol's prototype is the one we're really after.  If we already have a return type
	// symbol, then we want to replace it if we've been given futher information.  However, if we
	// have not been given any return type information from this call though, we will retain the
	// original data.
	Symbols::ISymbol *refFakeSym = const_cast< Symbols::ISymbol * >( fakeSym->RetainReferencedSymbol() );
	
	functionOwner->AddReturnTypes( refFakeSym->GetPrototypes() );
	fakeSym->Release();
	refFakeSym->Release();
	
	return processExpression;
}

bool DeclarationVisitor::VisitProgramNodeLeave( class ProgramNode *inNode )
{
	// If the current thread is dying, we don't want to add any symbols to the table
	// because that's an incredibly expensive operation.  Instead, we will bail out and
	// tell the caller the process failed so that the thread can die quicker.
	if (VTask::GetCurrent()->IsDying())	return false;

	// Now that we're done processing this program for declarations, we can actually update the
	// symbol table!
	bool ret = false;
	if (fSymTable) {
		ret = fSymTable->AddSymbols( fSymbolsToAdd );

		// Having added all of the symbols to the symbol table, we can add any extra information to it as well
		for (std::multimap< Symbols::ISymbol *, Symbols::IExtraInfo * >::iterator iter = fExtraInfoToAdd.begin(); ret && iter != fExtraInfoToAdd.end(); ++iter) {
			// It is possible we need to do some further processing of the extra information before we add it 
			// to the symbol table.  This gives us the chance to do things like resolve symbol pointers into
			// symbol IDs.
			Symbols::IExtraInfo *info = iter->second;
			if (info->GetKind() == Symbols::IExtraInfo::kKindPropertyAssignedFunctionExpression) {
				// Function expression assignments store a pointer to the affected symbol.  Pull the symbol out
				// and resolve it to an ID here
				Symbols::ISymbol *affectedSymbol = (Symbols::ISymbol *)info->GetNonPersistentCookie();
				info->SetIntegerData( affectedSymbol->GetID() );
				affectedSymbol->Release();	// We retained it when we created the link between the symbol and the info
			}
			ret = fSymTable->AddExtraInformation( iter->first, info );
		}

		// Now our symbols are stored in database, we can try to complete them
		for (std::vector<Symbols::ISymbol*>::iterator it = fSymbolsToUpdate.begin(); it != fSymbolsToUpdate.end(); ++it)
		{
			SetSymbolReturnTypesFromScriptDoc(*it, false, true);
			SetSymbolClassesFromScriptDoc(*it, false, true);
		}
	}

return ret;
}

bool DeclarationVisitor::VisitVariableDeclarationStatementNodeEnter( class VariableDeclarationStatementNode *inNode )
{
	VString identText = static_cast< IdentifierNode * >( inNode->GetIdentifier() )->GetText();
	VString wafComment = inNode->GetWAFComment();

	Symbols::ISymbol *sym = new VSymbol();
	sym->SetID( fSymTable->GetNextUniqueID() );
	sym->SetFile( fOwningFile );
	sym->SetName( identText );
	sym->SetLineNumber( inNode->GetIdentifier()->GetLineNumber() );

	if ( ! wafComment.IsEmpty() )
		sym->SetWAFKind( wafComment );

	// If owner is a class, variable will be a private property
	Symbols::ISymbol *owner = fOwnershipStack.back();
	if ( owner && owner->GetKind() == Symbols::ISymbol::kKindClass )
		sym->SetKind( Symbols::ISymbol::kKindPrivateProperty );
	else
		sym->SetKind( Symbols::ISymbol::kKindLocalVariableDeclaration );

	sym->SetOwner( owner );

	if ( inNode->GetScriptDocComment() )
	{
		sym->SetScriptDocComment( inNode->GetScriptDocComment()->GetText() );
		SetSymbolTypesFromScriptDoc( sym );
	}

	fSymbolsToAdd.push_back( sym );

	// If we have an assignment, we want to add information about it here.
	Node *assignment = inNode->GetAssignment();
	if (assignment) {
		return VisitRightHandSideOfAssignment( sym, assignment );
	}

	return true;
}

bool DeclarationVisitor::VisitCatchStatementNodeEnter( class CatchStatementNode *inNode )
{
	Symbols::ISymbol *sym = new VSymbol();
	sym->SetID( fSymTable->GetNextUniqueID() );
	sym->SetFile( fOwningFile );
	sym->SetLineNumber( inNode->GetLineNumber() );
	sym->SetLineCompletionNumber( inNode->GetLineCompletion() );
	sym->SetKind( Symbols::ISymbol::kKindCatchBlock );

	fSymbolsToAdd.push_back( sym );

	Node *ident = inNode->GetIdentifier();
	if (ident) {
		// The identifier will be treated as a "parameter" to our catch block function
		Symbols::ISymbol *arg = new VSymbol();
		arg->SetID( fSymTable->GetNextUniqueID() );
		arg->SetName( static_cast< IdentifierNode * >( ident )->GetText() );
		arg->SetFile( fOwningFile );
		arg->SetLineNumber( inNode->GetLineNumber() );
		arg->SetKind( Symbols::ISymbol::kKindLocalVariableDeclaration );
		arg->SetOwner( sym );

		CreateNewInstance( arg, GetLikeNamedSubSymbol( NULL, "Error" ) );
		fSymbolsToAdd.push_back( arg );
	}

	fOwnershipStack.push_back( sym );
	return true;
}

bool DeclarationVisitor::VisitCatchStatementNodeLeave( class CatchStatementNode *inNode )
{
	Symbols::ISymbol *object = fOwnershipStack.back();
	fOwnershipStack.pop_back();

	object->SetOwner( fOwnershipStack.back() );
	return true;
}

bool DeclarationVisitor::VisitFunctionExpressionNodeEnter( class FunctionExpressionNode *inNode )
{
	// Create the function symbol
 	VSymbol *sym = new VSymbol();
	sym->SetID( fSymTable->GetNextUniqueID() );
	sym->SetFile( fOwningFile );
	sym->SetLineNumber( inNode->GetLineNumber() );
	sym->SetLineCompletionNumber( inNode->GetLineCompletion() );
	sym->SetKind( Symbols::ISymbol::kKindFunctionDeclaration );
	sym->SetName( inNode->GetIdentifier() );

	CreateNewInstance( sym, GetLikeNamedSubSymbol( NULL, "Function" ) );

	VString firstChar;
	const UniChar* name = sym->GetName().GetCPointer();
	if (name && name[0] >= 'A' && name[0] <= 'Z')
		sym->SetKind( Symbols::ISymbol::kKindClass );
	

	// Try to set complete symbol return types
	if (inNode->GetScriptDocComment())
	{
		sym->SetScriptDocComment( inNode->GetScriptDocComment()->GetText() );
		SetSymbolReturnTypesFromScriptDoc(sym);
		SetSymbolClassesFromScriptDoc(sym);
		SetSymbolClassKindFromScriptDoc(sym);
		
	}

	VSymbol *constructor = NULL;

	if (sym->GetKind() == Symbols::ISymbol::kKindClass)
	{
		constructor = new VSymbol();
		constructor->SetID( fSymTable->GetNextUniqueID() );
		constructor->SetName( sym->GetName() );
		constructor->SetOwner( sym );
		constructor->SetFile( fOwningFile );
		constructor->SetLineNumber( inNode->GetLineNumber() );
		constructor->SetKind( Symbols::ISymbol::kKindClassConstructor );

		if ( inNode->GetScriptDocComment() )
			constructor->SetScriptDocComment( inNode->GetScriptDocComment()->GetText() );
	}

	Symbols::ISymbol* prototype = CreateSymbolPrototype(sym);	

	// Push it onto our symbol owner stack
	fOwnershipStack.push_back( sym );

	fSymbolsToAdd.push_back( sym );
	fSymbolsToAdd.push_back( prototype );

	if (NULL != constructor)
		fSymbolsToAdd.push_back( constructor );

	return true;
}

bool DeclarationVisitor::VisitFunctionExpressionNodeLeave( class FunctionExpressionNode *inNode )
{
	// Remove our symbol from the ownership stack
	Symbols::ISymbol *object = fOwnershipStack.back();
	fOwnershipStack.pop_back();

	// And add it to the parent
	object->SetOwner( fOwnershipStack.back() );

	// Kludge way to add application instance to global context.
	if ( inNode->GetIdentifier() == GLOBAL_CONTEXT_SERVER_CLASS )
	{
		Symbols::ISymbol* prototype = GetLikeNamedSubSymbol(object, CVSTR("prototype") );
		if (NULL != prototype)
		{
			std::vector< Symbols::ISymbol * > subSyms; 
			GetSubSymbols( prototype, subSyms );

			for (std::vector< Symbols::ISymbol * >::iterator it = subSyms.begin(); it != subSyms.end(); ++it)
			{
				if ( (*it)->GetName() != GLOBAL_CONTEXT_SERVER_INSTANCE )
				{
					VSymbol *fakeSym = new VSymbol();
					fakeSym->SetID( fSymTable->GetNextUniqueID() );
					fakeSym->SetFile( fOwningFile );
					fakeSym->SetLineNumber( (*it)->GetLineNumber() );
					fakeSym->SetLineCompletionNumber( (*it)->GetLineCompletionNumber() );
					fakeSym->SetKind( Symbols::ISymbol::kKindLocalVariableDeclaration );
					fakeSym->SetName( (*it)->GetName() );
					fakeSym->SetOwner( NULL);
					fakeSym->SetReferenceSymbol( *it );

					(*it)->Retain();
					fSymbolsToAdd.push_back( fakeSym );
				}

				(*it)->Release();
			}
		}
	}

	return true;
}

bool DeclarationVisitor::VisitFunctionDeclarationStatementNodeEnter( class FunctionDeclarationStatementNode *inNode )
{
	// Create the function symbol
	VSymbol *sym = new VSymbol();
	sym->SetID( fSymTable->GetNextUniqueID() );
	sym->SetFile( fOwningFile );
	sym->SetName( inNode->GetIdentifier() );
	sym->SetLineNumber( inNode->GetLineNumber() );
	sym->SetLineCompletionNumber( inNode->GetLineCompletion() );
	sym->SetKind( Symbols::ISymbol::kKindFunctionDeclaration );

	CreateNewInstance( sym, GetLikeNamedSubSymbol( NULL, "Function" ) );

	VString firstChar;
	const UniChar* name = sym->GetName().GetCPointer();
	if (name && name[0] >= 'A' && name[0] <= 'Z')
		sym->SetKind( Symbols::ISymbol::kKindClass );
	

	// Try to set complete symbol return types
	if ( inNode->GetScriptDocComment() )
	{
		sym->SetScriptDocComment( inNode->GetScriptDocComment()->GetText() );
		SetSymbolReturnTypesFromScriptDoc(sym);
		SetSymbolClassesFromScriptDoc(sym);
		SetSymbolClassKindFromScriptDoc(sym);
		
	}

	VSymbol *constructor = NULL;

	if (sym->GetKind() == Symbols::ISymbol::kKindClass)
	{
		constructor = new VSymbol();
		constructor->SetID( fSymTable->GetNextUniqueID() );
		constructor->SetName( sym->GetName() );
		constructor->SetOwner( sym );
		constructor->SetFile( fOwningFile );
		constructor->SetLineNumber( inNode->GetLineNumber() );
		constructor->SetKind( Symbols::ISymbol::kKindClassConstructor );

		if ( inNode->GetScriptDocComment() )
			constructor->SetScriptDocComment( inNode->GetScriptDocComment()->GetText() );
	}

	Symbols::ISymbol* prototype = CreateSymbolPrototype(sym);
	
	// Push it onto our symbol owner stack
	fOwnershipStack.push_back( sym );

	fSymbolsToAdd.push_back( sym );
	if (NULL != constructor)
		fSymbolsToAdd.push_back( constructor );
	fSymbolsToAdd.push_back( prototype );

	return true;
}

bool DeclarationVisitor::VisitFunctionDeclarationStatementNodeLeave( class FunctionDeclarationStatementNode *inNode )
{
	// Remove our symbol from the ownership stack
	Symbols::ISymbol *object = fOwnershipStack.back();
	fOwnershipStack.pop_back();

	// If parent is a class, this function will be a private member
	Symbols::ISymbol *owner = fOwnershipStack.back();
	if (NULL != owner)
	{
		if ( owner->GetKind() == Symbols::ISymbol::kKindClass )
			object->SetKind( Symbols::ISymbol::kKindClassPrivateMethod );
		else if ( owner->IsObjectLiteral() )
			object->SetKind( Symbols::ISymbol::kKindClassPublicMethod );
	}

	// And add it to the parent
	object->SetOwner( owner );

	// Build symbol extra info
	VSymbolExtraInfo *extraInfo = new VSymbolExtraInfo();

	extraInfo->SetKind( Symbols::IExtraInfo::kKindFunctionSignature );
	extraInfo->SetStringData( BuildFunctionSignature(inNode->GetIdentifier(), inNode->GetArgs() ) );
	fExtraInfoToAdd.insert( std::pair< Symbols::ISymbol *, Symbols::IExtraInfo * >( object, extraInfo ) );
	object->Retain();		// Because we added it to the extra info list

	if (object->GetKind() == Symbols::ISymbol::kKindClass)
	{
		Symbols::ISymbol *constructor = GetLikeNamedSubSymbol( object, object->GetName() );
		if (NULL != constructor)
		{
			fExtraInfoToAdd.insert( std::pair< Symbols::ISymbol *, Symbols::IExtraInfo * >( constructor, extraInfo ) );
			extraInfo->Retain();
		}
	}


	return true;
}

bool DeclarationVisitor::VisitFunctionDeclarationArgumentsNode( class FunctionDeclarationArgumentsNode *inNode )
{
	Symbols::ISymbol *owner = fOwnershipStack.back();

	ScriptDocComment *sdocComment = NULL;
	if (owner && !owner->GetScriptDocComment().IsEmpty()) {
		sdocComment = ScriptDocComment::Create( owner->GetScriptDocComment() );
	}

	for (size_t i = 0; i < inNode->ArgumentCount(); i++) {
		Node *arg = inNode->GetArgument( i );

		IdentifierNode *identArg = dynamic_cast< IdentifierNode * >( arg );
		if (identArg) {
			VSymbol *sym = new VSymbol();
			sym->SetID( fSymTable->GetNextUniqueID() );
			sym->SetFile( fOwningFile );
			sym->SetName( identArg->GetText() );
			sym->SetLineNumber( identArg->GetLineNumber() );
			sym->SetKind( Symbols::ISymbol::kKindFunctionParameterDeclaration | Symbols::ISymbol::kInstanceValue );
			sym->SetOwner( owner );

			if (sdocComment) {
				// If we have a ScriptDoc comment for the owner, then we will see if
				// it mentions anything about this argument.  If it does, perhaps it
				// will tell us what type the target expects.
				ScriptDocLexer::Element *element = sdocComment->GetTargetElement( identArg->GetText() );
				if (element && element->Type() == IScriptDocCommentField::kParam) {
					// We have a parameter element, so let's see if it has some type information
					VString typeInfo = static_cast< ScriptDocLexer::ParamElement * >( element )->fTypes;

					// Since it is possible to accept multiple types of information, but a symbol
					// can only have one prototype, we will bail out if the parameter can use multiple
					// types.  It's not perfect, but hey, that's the JavaScript mantra, right?
					VectorOfVString types;
				
					typeInfo.GetSubStrings( CHAR_VERTICAL_LINE, types, false, true);
					if ( ! types.empty() )
					{
						for (VectorOfVString::iterator itType = types.begin(); itType != types.end(); ++itType)
						{
							Symbols::ISymbol *typeSym = NULL;
							if (UnqualifiedLookup( *itType, &typeSym ))
							{
								CreateNewInstance( sym, typeSym );
								typeSym->Release();
							}
						}
					}
				}
			}

			fSymbolsToAdd.push_back( sym );
		}
	}

	if (sdocComment)
		sdocComment->Release();

	return true;
}

bool DeclarationVisitor::VisitForExpressionInClauseNode( class ForExpressionInClauseNode *inNode )
{
	// We need to handle the In expression clause of a for loop in order to ensure the lhs of the
	// in expression is declared as appropriate.  Note that we do not need to do this for a tri-clause
	// because the first clause is always either a var declaration list (which is handled by the
	// variable declaration visitor), or an assignment expression (which is handled by the assignment
	// expresion visitor).
	//
	// If the LHS node is a variable declaration node, we don't want to do any special processing.  However,
	// if it is just an identifier, we want to make a new global variable for it.
	//
	// Note that this rule only pertains to in expressions used within a for loop.
	IdentifierNode *ident = dynamic_cast< IdentifierNode * >( inNode->GetDeclaration() );
	if (ident) {
		// Look to see whether we have this variable declared within the scope already.  In that case, we
		// don't want to make a new global symbol for it.
		Symbols::ISymbol *foundSym = NULL;
		if (!UnqualifiedLookup( ident->GetText(), &foundSym )) {
			// We have a simple identifier instead of a variable declaration statement, so let's add this identifier
			// to the global symbol table
			VSymbol *sym = new VSymbol();
			sym->SetID( fSymTable->GetNextUniqueID() );
			sym->SetFile( fOwningFile );
			sym->SetName( ident->GetText() );
			sym->SetLineNumber( ident->GetLineNumber() );
			sym->SetKind( Symbols::ISymbol::kKindLocalVariableDeclaration | Symbols::ISymbol::kInstanceValue );
			sym->SetOwner( NULL );		// This is a global variable because it's not been declared with 'var'
			fSymbolsToAdd.push_back( sym );
		} else {
			// We did find a symbol already
			foundSym->Release();
		}
	}
	return true;
}

// NOTE: About Object Literals
//
//	According to the spec (Section 11.1.5), all object literals are created as if they were made
//	with a call to "new Object."  That means we will be creating a nameless symbol for the literal
//	itself, and giving it a prototype.  The prototype will inherit from the Object symbol.  When
//	we process an object literal field, the field will become a public property that lives on the
//	owner object's prototype.
bool DeclarationVisitor::VisitObjectLiteralNodeEnter( ObjectLiteralNode *inNode )
{
	// Create the object symbol
	VSymbol *sym = new VSymbol();
	sym->SetID( fSymTable->GetNextUniqueID() );
	sym->SetFile( fOwningFile );
	sym->SetLineNumber( inNode->GetLineNumber() );
	sym->SetKind( Symbols::ISymbol::kKindObjectLiteral );
	// NOTE: Object literals do not have a name, and that's OK

	// Create a new instance of the object as if we called new Object();
	CreateNewInstance( sym, GetLikeNamedSubSymbol( NULL, "Object" ) );
	

	// Push it onto our symbol owner stack
	fOwnershipStack.push_back( sym );
	fSymbolsToAdd.push_back( sym );

	return true;
}

bool DeclarationVisitor::VisitObjectLiteralFieldNodeEnter( class ObjectLiteralFieldNode *inNode )
{
	// Get the identifier for this field
	VString identStr;
	if (dynamic_cast< IdentifierNode * >( inNode->GetIdentifier() )) {
		identStr = dynamic_cast< IdentifierNode * >( inNode->GetIdentifier() )->GetText();
	} else if (dynamic_cast< NumericLiteralNode * >( inNode->GetIdentifier() )) {
		identStr = dynamic_cast< NumericLiteralNode * >( inNode->GetIdentifier() )->GetValue();
	} else if (dynamic_cast< StringLiteralNode * >( inNode->GetIdentifier() )) {
		// We want to strip off the open and closing quotes from the string literal
		identStr = dynamic_cast< StringLiteralNode * >( inNode->GetIdentifier() )->GetValue();
		identStr.SubString( 2, identStr.GetLength() - 2 );
	}

	VSymbol *sym = new VSymbol();
	sym->SetID( fSymTable->GetNextUniqueID() );
	sym->SetFile( fOwningFile );
	sym->SetName( identStr );
	sym->SetLineNumber( inNode->GetIdentifier()->GetLineNumber() );
	sym->SetKind( Symbols::ISymbol::kKindPublicProperty );
	sym->SetOwner( fOwnershipStack.back() );

	if ( inNode->GetIdentifier() && inNode->GetIdentifier()->GetScriptDocComment() )
	{
		sym->SetScriptDocComment( inNode->GetIdentifier()->GetScriptDocComment()->GetText() );
		SetSymbolTypesFromScriptDoc( sym );
	}

	fSymbolsToAdd.push_back( sym );

	// Now we want to treat this like an assignment
	return VisitRightHandSideOfAssignment( sym, inNode->GetValue() );
}

bool DeclarationVisitor::VisitObjectLiteralNodeLeave( ObjectLiteralNode *inNode )
{
	// Remove our symbol from the ownership stack
	Symbols::ISymbol *object = fOwnershipStack.back();
	fOwnershipStack.pop_back();

	// And add it to the parent
	object->SetOwner( fOwnershipStack.back() );

	return true;
}

bool DeclarationVisitor::QualifiedLookup( class Node *inNode, Symbols::ISymbol **outSymbol )
{
	if (!outSymbol)	return false;
	*outSymbol = NULL;

	// Given a qualified expression, we want to look up the symbol being referenced.  We do this by walking our
	// way down the left-hand side until we hit an identifier node.  That's an unqualified lookup which allows
	// us to start searching.
	if (dynamic_cast< IdentifierNode * >( inNode )) {
		// A left-hand side that's a pure identifier means this is the start of the lookup operation.  This
		// lookup is an unqualified one
		if (!UnqualifiedLookup( static_cast< IdentifierNode * >( inNode )->GetText(), outSymbol ))	return false;
	} else if (dynamic_cast< DotExpressionNode * >( inNode )) {
		Symbols::ISymbol *lhsSym = NULL;
		// If the left-hand side is still a dot expression, then we have more searching to do
		if (!QualifiedLookup( static_cast< DotExpressionNode * >( inNode )->GetLHS(), &lhsSym ))	return false;

		// Now that we've figured out what the left-hand side's symbol is, the right-hand side should be an identifier
		if (dynamic_cast< IdentifierNode * >( static_cast< DotExpressionNode * >( inNode )->GetRHS() ) == NULL) {
			lhsSym->Release();
			return false;
		}

		VString identText = static_cast< IdentifierNode * >( static_cast< DotExpressionNode * >( inNode )->GetRHS() )->GetText();

		// So let's look for the symbol on our left-hand side
		std::vector< Symbols::ISymbol * > lookup_list;
		lookup_list.push_back( const_cast< Symbols::ISymbol * >( lhsSym->RetainReferencedSymbol() ) );
		Symbols::ISymbol *temp = lookup_list.back();
		while (!lookup_list.empty()) {
			Symbols::ISymbol *lookup = lookup_list.back();
			lookup_list.pop_back();

			// See if we can find the symbol with the proper name
			Symbols::ISymbol *sym = GetLikeNamedSubSymbol( lookup, identText );

			if (sym)
			{
				// We only care about public properties, since this is a qualified lookup
				if (sym->IsPublicPropertyKind() )
					*outSymbol = const_cast< Symbols::ISymbol * >( sym->RetainReferencedSymbol() );
				sym->Release();
			}

			if (*outSymbol)	break;

			// Otherwise, move up the prototype chain
			lookup_list.insert( lookup_list.end(), lookup->GetPrototypes().begin(), lookup->GetPrototypes().end() );
		}

		temp->Release();
		lhsSym->Release();
	}

	return (*outSymbol) != NULL;
}

bool DeclarationVisitor::UnqualifiedLookup( const VString &inIdent, Symbols::ISymbol **outSymbol )
{
	*outSymbol = NULL;

	// We want to walk the symbol stack to see if we can locate this identifier somewhere.  If we
	// can, then we return true.  If we can't locate the symbol, we return false.
	for (std::vector< Symbols::ISymbol * >::reverse_iterator iter = fOwnershipStack.rbegin(); iter != fOwnershipStack.rend(); ++iter) {
		if ((*iter) == NULL) {
			// We need to check the global symbol table, which are symbols whose Owner field
			// is set to NULL.
			*outSymbol = GetLikeNamedSubSymbol( NULL, inIdent );
			if (*outSymbol)
				return true;
			return false;
		}
		
		// Check to see if any local variables, function declarations, or parameter names that the current
		// symbol owns match the text we've passed in.  If they don't, we'll move up a level in the ownership
		// stack.
		std::vector< Symbols::ISymbol * > subSyms;
		GetSubSymbols( *iter, subSyms );
		for (std::vector< Symbols::ISymbol * >::iterator subs = subSyms.begin(); subs != subSyms.end(); ++subs) {
			// First, test to make sure it's a private property
			int symKind = (*subs)->GetKind();
			if ( ( symKind != Symbols::ISymbol::kKindClassConstructor )  &&
				 ( symKind == Symbols::ISymbol::kKindLocalVariableDeclaration     ||
				   symKind == Symbols::ISymbol::kKindPrivateProperty              ||
				   symKind == Symbols::ISymbol::kKindClassPrivateMethod           ||
				   symKind == Symbols::ISymbol::kKindFunctionParameterDeclaration ||
				   symKind == Symbols::ISymbol::kKindCatchBlock                   ||
				   (*subs)->IsFunctionKind() ) ) {

				// Next, check to see whether the text matches too
				if ((*subs)->GetName().EqualToString( inIdent, true )) {
					// It is possible that we've actually already saved a symbol in our return slot.  However, we
					// want to give preference to symbols that live in the same file as the one being parsed.  So
					// check to see if that's the case here.  If our new symbol is in the same file as the one being
					// parsed, we'll use it instead.
					if (!(*outSymbol)) {
						// We don't have a return symbol yet, so we just want to keep this one regardless
						*outSymbol = *subs;
						(*outSymbol)->Retain();
					} else {
						// We already have a symbol stored here, so let's see if this one is better.  The only way it can
						// be better is if we have a context file, and if the context file's path matches the iterator's
						// file's path.
						if (fOwningFile) {
							VString contextFilePath = fOwningFile->GetPath();
							VString symbolFilePath = ((*subs)->GetFile()) ? (*subs)->GetFile()->GetPath() : CVSTR( "" );
							VString oldSymbolFilePath = ((*outSymbol)->GetFile()) ? (*outSymbol)->GetFile()->GetPath() : CVSTR( "" );

							if (!oldSymbolFilePath.EqualTo( contextFilePath, true )) {
								// The old symbol was in a different file from the context file
								if (symbolFilePath.EqualTo( contextFilePath, true )) {
									// But the new symbol is in the same file, so we found a replacement!
									(*outSymbol)->Release();
									(*outSymbol) = *subs;
									(*outSymbol)->Retain();
								}
							}
						}
					}
				}
			}
			(*subs)->Release();
		}
		if (*outSymbol)	return true;
	}

	if (outSymbol)	*outSymbol = NULL;

	return false;
}

Symbols::ISymbol *DeclarationVisitor::GetThis()
{
	// When we encounter the "this" node, we will return the owning function's prototype symbol for it.  This
	// isn't strictly correct since the this keyword actually relies on the caller as well as the context,
	// but we don't have the ability to determine who the caller is at compile time.  Note that we are getting
	// the external prototype property, not the internal prototype symbol
	for (std::vector< Symbols::ISymbol * >::reverse_iterator iter = fOwnershipStack.rbegin(); iter != fOwnershipStack.rend(); ++iter)
		if (*iter && (*iter)->IsFunctionKind() )
			return GetLikeNamedSubSymbol( *iter, "prototype" );

	return NULL;
}

void DeclarationVisitor::CreateNewInstance( Symbols::ISymbol *inLHS, Symbols::ISymbol *inOfThisType, const sLONG inLevel )
{
	// We've been given a LHS symbol that we want to turn into the type from the inOfThisType parameter.
	// This means that we want to set up the LHS side's prototype chain.  However, the left-hand side could
	// be a reference to another symbol (for instance, the lhs could be a local variable that references a
	// function expression and we want that function expression), so we need to dereference the lhs.
	if ( NULL != inLHS && NULL != inOfThisType )
	{
		Symbols::ISymbol *refOfThisType =  const_cast< Symbols::ISymbol * >( inOfThisType->RetainReferencedSymbol() );

		// Internal instantiation
		if ( ! inLevel || refOfThisType->GetName() != CVSTR("Function") )
			CreateNewInternalInstance( inLHS, refOfThisType );
		
		// External instantiation
		std::vector< Symbols::ISymbol * > prototypes = refOfThisType->GetPrototypes();
		for (std::vector< Symbols::ISymbol * >::iterator iter = prototypes.begin(); iter != prototypes.end(); ++iter)
			CreateNewInstance( inLHS, (*iter)->GetOwner(), inLevel + 1);

		// At the end of recursive instantiation process
		// we want to make our instance a JSCore Object.
		if ( ! inLevel )
			CreateNewInternalInstance( inLHS, GetLikeNamedSubSymbol( NULL, "Object" ) );

		inLHS->AddAuxillaryKindInformation( Symbols::ISymbol::kInstanceValue );
		refOfThisType->Release();
	}
}


void DeclarationVisitor::CreateNewInternalInstance( Symbols::ISymbol *inLHS, Symbols::ISymbol *inOfThisType)
{
	if (NULL != inLHS && NULL != inOfThisType)
	{
		inLHS->Retain();
		inOfThisType->Retain();

		Symbols::ISymbol *prototype = GetLikeNamedSubSymbol( inOfThisType, "prototype" );
		if ( NULL != prototype )
		{
			inLHS->AddPrototype( prototype );
			prototype->Release();
		}

		inLHS->Release();
		inOfThisType->Release();
	}
}

Symbols::ISymbol* DeclarationVisitor::CreateSymbolPrototype( Symbols::ISymbol *inSym)
{
	// Functions get an external prototype property as well as an internal one.
	VSymbol *prototype = new VSymbol();
	prototype->SetID( fSymTable->GetNextUniqueID() );
	prototype->SetName( "prototype" );
	prototype->SetOwner( inSym );
	prototype->SetFile( fOwningFile );
	prototype->SetLineNumber( inSym->GetLineNumber() );
	prototype->SetKind( Symbols::ISymbol::kKindPublicProperty );

	// The external prototype property is always of type Object by default, as though it
	// were set via a call to new Object();
	CreateNewInstance( prototype, GetLikeNamedSubSymbol( NULL, "Object" ) );

	return prototype;
}

VString DeclarationVisitor::BuildFunctionSignature(const VString& inIdentifier, class FunctionDeclarationArgumentsNode *inArgs)
{
	// We have some extra information we want to associate with function declarations.  Namely, 
	// we want to add information about the function's signature.  So let's calculate what that is.
	VString signature = inIdentifier + CVSTR( "(" );

	if (inArgs)
	{
		for (size_t i = 0; i < inArgs->ArgumentCount(); i++)
		{
			Node *arg = inArgs->GetArgument( i );
			if (dynamic_cast< IdentifierNode * >( arg ))
			{
				if (i != 0)	signature += CVSTR( ", " );
				signature += dynamic_cast< IdentifierNode * >( arg )->GetText();
			}
		}
	}
	signature += CVSTR( ")" );

	return signature;
}


Symbols::ISymbol *DeclarationVisitor::GetSymbolFromLeftHandSideNode( class LeftHandSideExpressionNode *inNode, bool inCreateIfNeeded )
{
	// We've been given a left-hand side expression and we want to locate the symbol it represents.  However, it is
	// possible that the symbol doesn't exist yet.  If that's the case, we want to make a new symbol and add it to the
	// proper owner before returning it to the caller.  That is because we are assuming that the only reason we want to
	// look up a lhs symbol is because it's part of an assignment expression (or its moral equivalent), in which case the
	// symbol should be added to the table.
	if (!inNode)	return NULL;		// Sanity check

	if (dynamic_cast< IdentifierNode * >( inNode ))
	{
		Symbols::ISymbol *sym = NULL, *refSym = NULL;

		if (UnqualifiedLookup( dynamic_cast< IdentifierNode * >( inNode )->GetText(), &sym ) )
		{
			// As we want already defined symbol to appear in outline to handle the case of new
			// object properties defined in current document we cancel the result of unqualified
			// lookup if found symbol doen't belong to current file. This solution is a good
			// choice to have a correct representation of current file in outline
			if (sym->GetFile() && sym->GetFile()->GetID() != fOwningFile->GetID())
			{
				refSym = sym;
				sym = NULL;
			}
		}

		if (NULL == sym && inCreateIfNeeded)
		{
			// If we couldn't find the basic identifier in the stack, then we need to gin up a new symbol.  Since
			// this symbol isn't a part of a variable declaration, it is assumed to be a global symbol
			sym = new VSymbol();
			sym->SetID( fSymTable->GetNextUniqueID() );
			sym->SetFile( fOwningFile );
			sym->SetName( dynamic_cast< IdentifierNode * >( inNode )->GetText() );
			sym->SetLineNumber( inNode->GetLineNumber() );
			sym->SetKind( NULL != refSym ? refSym->GetKind() : Symbols::ISymbol::kKindLocalVariableDeclaration );
			sym->SetOwner( NULL );		// This declaration is a global variable
			sym->SetReferenceSymbol(refSym); 

			fSymbolsToAdd.push_back( sym );
			

			// We want to keep track of statement new symbols which will be flag as undefined when they are 
			// the LHS of a dot expression node
			fSymbolsToAddToStatement.push_back( sym );

			// Since we are adding this to the table, we don't want to destroy it right away
			sym->Retain();
		}
		return sym;
	} else if (dynamic_cast< DotExpressionNode * >( inNode ) || dynamic_cast< ArrayExpressionNode * >( inNode )) {

		Symbols::ISymbol *sym = NULL;
		if (dynamic_cast< DotExpressionNode * >( inNode ))
			sym = GetSymbolFromLeftHandSideNode( dynamic_cast< LeftHandSideExpressionNode * >( dynamic_cast< DotExpressionNode * >( inNode )->GetLHS() ), inCreateIfNeeded );
		else
			sym = GetSymbolFromLeftHandSideNode( dynamic_cast< LeftHandSideExpressionNode * >( dynamic_cast< ArrayExpressionNode * >( inNode )->GetLHS() ), inCreateIfNeeded );
		if (sym)
		{

			// If parent symbol has just been created in the current statement we can flag it as undefined
			// It will help us to find the real symbol definition in other statements or files
			//std::vector< Symbols::ISymbol * >::const_iterator it = find(fSymbolsToAddToStatement.begin(), fSymbolsToAddToStatement.end(), sym);
			//if (it != fSymbolsToAddToStatement.end())
			//	sym->AddAuxillaryKindInformation(Symbols::ISymbol::kKindUndefined);
			std::vector< Symbols::ISymbol * >::const_iterator it = find(fSymbolsToAdd.begin(), fSymbolsToAdd.end(), sym);
			if (it != fSymbolsToAdd.end() && inNode->GetLineNumber() == sym->GetLineNumber())
                ( sym->GetName() == "exports" ) ? sym->RemoveAuxillaryKindInformation(Symbols::ISymbol::kKindUndefined) : sym->AddAuxillaryKindInformation(Symbols::ISymbol::kKindUndefined);

			// This is the point where we need to dereference what this symbol may be referencing
			Symbols::ISymbol *refSym = const_cast< Symbols::ISymbol * >( sym->RetainReferencedSymbol() );

			// Now we want to see if we can find the sub-symbol from the RHS of the dot operator.  The LHS gives us the
			// reference to a symbol. However, since we know this is for assignment (it must be, that's all the declaration
			// parser cares about), then the symbol must be owned by the LHS, and we don't care about searching up the prototype
			// chain for any information.
			VString rhsText;
			sLONG	rhsLineNumber;
			bool	rhsValid = false;

			if (dynamic_cast< DotExpressionNode * >( inNode ))
			{
				IdentifierNode*	rhs = dynamic_cast< IdentifierNode * >( dynamic_cast< DotExpressionNode * >( inNode )->GetRHS() );
				if (rhs)
				{
					rhsText =  rhs->GetText();
					rhsLineNumber = rhs->GetLineNumber();
					rhsValid = true;
				}
			}
			else
			{
				StringLiteralNode* rhs = dynamic_cast< StringLiteralNode * >( dynamic_cast< ArrayExpressionNode * >( inNode )->GetRHS() );
				if (rhs)
				{
					rhsText =  rhs->GetValue();
					rhsText.SubString(2, rhsText.GetLength() - 2); // Suppress the string apostrphe or quotation mark
					rhsLineNumber = rhs->GetLineNumber();
					rhsValid = true;
				}
			}

			Symbols::ISymbol *ret = NULL;
			if (rhsValid)
			{
				Symbols::ISymbol *childSym = NULL;

				// "onAfterInit hack": Kludge way to manage JavaScript code generated for GUI Script 
				// event handlers: as "WAF.onAfterInit" can be multiple defined and we don't want to 
				// overwrite others definitions, "WAF" symbol will be the local one and "onAfterInit"
				// will be its child.
				if ( sym->GetName() == CVSTR("WAF") && rhsText == CVSTR("onAfterInit") )
				{
					refSym->Release();
					refSym = sym;
					refSym->Retain();
				}
				else
					childSym = GetLikeNamedSubSymbol( refSym, rhsText );

				if (!childSym && inCreateIfNeeded)
				{
					// We couldn't find the sub symbol, which means we need to create a new symbol to attach
					// to the owner.  We will assume that it's a public property because we're doing a qualified lookup
					childSym = new VSymbol();
					childSym->SetID( fSymTable->GetNextUniqueID() );
					childSym->SetFile( fOwningFile );
					childSym->SetName( rhsText );
					childSym->SetLineNumber( rhsLineNumber );
					childSym->SetOwner( refSym );
					if ( refSym && refSym->GetKind() == Symbols::ISymbol::kKindClass && rhsText != CVSTR( "prototype" ) )
						childSym->SetKind( Symbols::ISymbol::kKindStaticProperty );
					else
						childSym->SetKind( Symbols::ISymbol::kKindPublicProperty );

					if ( inNode->GetScriptDocComment() )
					{
						childSym->SetScriptDocComment( inNode->GetScriptDocComment()->GetText() );
						SetSymbolTypesFromScriptDoc( childSym );
					}

					fSymbolsToAdd.push_back( childSym );

					// We want to keep track of statement new symbols which will be flag as undefined when they are 
					// the LHS of a dot expression node
					fSymbolsToAddToStatement.push_back( childSym );

					// Since we're adding this symbol to the table, we don't want to destroy it
					childSym->Retain();
				}
				ret = childSym;
			}
			refSym->Release();
			sym->Release();
			return ret;
		}
	} else if (dynamic_cast< ThisNode * >( inNode )) {
		return GetThis();
	} 

	return NULL;
}

Symbols::ISymbol*  DeclarationVisitor::RetainSymbolPrototype(const VString& inSymbolName, const ESymbolFileExecContext& inExecContext)
{
	Symbols::ISymbol* ret = NULL;

	std::vector<Symbols::ISymbol*> syms;
	GetLikeNamedSubSymbols(NULL, inSymbolName, syms);
	
	std::vector< Symbols::ISymbol * >::iterator it;
	for (it = syms.begin(); it != syms.end(); ++it)
	{
		if( !ret )
		{
			Symbols::ISymbol* sym = GetLikeNamedSubSymbol((*it), "prototype");
			if( sym && sym->GetFile()->GetExecutionContext() == inExecContext )
				ret = sym;
		}
		
		(*it)->Release();
	}

	return ret;
}

bool DeclarationVisitor::VisitRightHandSideOfAssignment( Symbols::ISymbol *inLHS, class Node *inNode, bool inForFakeAssignment )
{
	// This function performs the actual assignment of information from the RHS to the LHS.  Generally,
	// the most important pieces of information being copied over are prototypes, but there are situations
	// where other information is assigned as well, such as whether it is an instance value, public properties, etc.
	if (!inLHS)	return true;

	inLHS->RemoveAuxillaryKindInformation(Symbols::ISymbol::kKindUndefined);

	if (dynamic_cast< IdentifierNode * >( inNode ) || dynamic_cast< DotExpressionNode * >( inNode )) {
		Symbols::ISymbol *ident = NULL;
		if (QualifiedLookup( inNode, &ident )) {
			// The left-hand side now references the right-hand side
			Symbols::ISymbol *temp = const_cast< Symbols::ISymbol * >( ident->RetainReferencedSymbol() );
			inLHS->SetReferenceSymbol( temp );
			temp->Release();
			ident->Release();
		}
	} else if (dynamic_cast< ThisNode * >( inNode )) {
		// In this case, we are saying the left hand side's prototype is the this pointer
		Symbols::ISymbol *thisSym = GetThis();
		if (thisSym) {
			inLHS->AddPrototype( thisSym );
			thisSym->Release();
		}
	} else if (dynamic_cast< ObjectLiteralNode * >( inNode )) {
		// The object literal itself is the type that we want to assign to the object. 

		// Now we will manually process the right-hand side.  However, the default processing
		// behavior will be to add the symbol to the current owner.  The problem is: we don't
		// want to add it to the current owner!  But because object literals can be embedded in
		// one another, we need to use that behavior.  This provides us with quite the quandry,
		// but we will solve it with a faked symbol.

		VSymbol *orphanOwner = new VSymbol();
		fOwnershipStack.push_back( orphanOwner );

		inNode->Accept( this );
		
		// Grab our fake owner symbol
		fOwnershipStack.pop_back();

		// Now we can assign the literal we just created as the type for the left-hand side
		std::vector< Symbols::ISymbol * > syms;
		GetSubSymbols( orphanOwner, syms );
		xbox_assert( syms.empty() || syms.size() == 1 );
		if ( !syms.empty() )
		{
			inLHS->AddPrototypes( syms );
			syms[ 0 ]->SetOwner( fOwnershipStack.back() );
		
			for ( std::vector< Symbols::ISymbol *>::iterator it = syms.begin(); it != syms.end(); ++it)
				(*it)->Release();

			// The LHS is now an instance of that object literal (because the object literal can be
			// thought of as a short-hand call to new Object()), so we want to set the instance flag
			inLHS->AddAuxillaryKindInformation( Symbols::ISymbol::kInstanceValue );
		}

		orphanOwner->Release();

		// Since we manually processed the right-hand side, we don't want to process it a second time
		return false;
	}
	else if (dynamic_cast< NewNode * >( inNode ))
	{
		// Find the new expression's symbol.  This should be a symbol that references a function
		// name.  That function has an external prototype property that we will be using to set up
		// the prototype chain for the newly created object.
		Symbols::ISymbol *newExpr = NULL;
		if (QualifiedLookup( dynamic_cast< LeftHandSideExpressionNode * >( dynamic_cast< NewNode * >( inNode )->GetOperand() ), &newExpr ))
		{
			if( newExpr &&
				newExpr->GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModel )
			{
				Symbols::ISymbol* prototype = NULL;

				ESymbolFileExecContext execCtx = newExpr->GetFile()->GetExecutionContext();
					 
				// Special case : new entity model returns an entity, not an entity model
				prototype = RetainSymbolPrototype(newExpr->GetName(), execCtx);
				if( prototype )
					inLHS->AddPrototype(prototype);
				ReleaseRefCountable( &prototype );
				
				prototype = RetainSymbolPrototype("Entity", execCtx);
				if( prototype )
					inLHS->AddPrototype(prototype);
				ReleaseRefCountable( &prototype );

				// First of all, release the memory
				newExpr->Release();
			}
			else
			{
				CreateNewInstance( inLHS, newExpr );
				newExpr->Release();
			}
		}
	}
	else if (dynamic_cast< FunctionExpressionNode * >( inNode )) {
		// We are going to create a function symbol, and assign it to the left hand side.  This is
		// conceptually the same as if we make a function statement and assigned it to a variable,
		// just with a shorter syntax.
		VSymbol *orphanOwner = new VSymbol();
		FunctionExpressionNode *funcExprNode = dynamic_cast< FunctionExpressionNode * >( inNode );

		// Kludge to manage anonym or named function expressions: as function won't
		// be defined in the global context, we can change its name to the one of the
		// property which will referenced it. It will be great to make completion
		// work correctly.
		funcExprNode->SetIdentifier( inLHS->GetName() );

		fOwnershipStack.push_back( orphanOwner );

		inNode->Accept( this );
		
		// Grab our fake owner symbol
		fOwnershipStack.pop_back();

		// Now we can assign the function expression we just created to the left hand side.
		std::vector< Symbols::ISymbol * > syms;
		GetSubSymbols( orphanOwner, syms );
		xbox_assert( syms.empty() || syms.size() == 1 );
		if (!syms.empty()) {
			// The left-hand side now references the function expression
			inLHS->SetReferenceSymbol( syms[ 0 ] );

			// If the assigned variable is a public property, referenced
			// function becomes a public method: this method will be
			// considered as privileged if owner is a class declaration
			if (inLHS->GetKind() == Symbols::ISymbol::kKindPublicProperty)
			{
				if ( ! syms[ 0 ]->IsaClass() )
				{
					Symbols::ISymbol* owner = fOwnershipStack.back();
					if ( owner && owner->IsaClass() )
						syms[ 0 ]->SetKind(Symbols::ISymbol::kKindClassPrivilegedMethod);
					else
						syms[ 0 ]->SetKind(Symbols::ISymbol::kKindClassPublicMethod);
				}
			}
			else if ( inLHS->GetKind() == Symbols::ISymbol::kKindStaticProperty )
				syms[ 0 ]->SetKind(Symbols::ISymbol::kKindClassStaticMethod);
		
			syms[ 0 ]->AddAuxillaryKindInformation( Symbols::ISymbol::kKindFunctionExpression );

			if (!inForFakeAssignment)
			{
				VSymbolExtraInfo *link = new VSymbolExtraInfo();
				link->SetKind( Symbols::IExtraInfo::kKindPropertyAssignedFunctionExpression );
				// We set the function expression symbol as the Ptr data to associate with the link.  However,
				// once all of the symbols have been added to the table, we can go through the extra information
				// data, look for entries pertaining to function expression assignments, and convert that pointer
				// to symbol into its actual symbol ID value.  We cannot do this until AFTER all of the symbols have
				// been added to the symbol table though, which is the reason for this subterfuge right now.
				syms[ 0 ]->Retain();	// We'll release it when the program node leaves
				inLHS->Retain();
				link->SetNonPersistentCookie( (const void *)syms[ 0 ] );
				fExtraInfoToAdd.insert( std::pair< Symbols::ISymbol *, Symbols::IExtraInfo * >( inLHS, link ) );
				syms[ 0 ]->SetOwner( fOwnershipStack.back() );

				// Build symbol extra info
				VSymbolExtraInfo *extraInfo = new VSymbolExtraInfo();

				extraInfo->SetKind( Symbols::IExtraInfo::kKindFunctionSignature );
				extraInfo->SetStringData( BuildFunctionSignature(funcExprNode->GetIdentifier(), funcExprNode->GetArgs() ) );
				fExtraInfoToAdd.insert( std::pair< Symbols::ISymbol *, Symbols::IExtraInfo * >( syms[ 0 ], extraInfo ) );
				syms[ 0 ]->Retain();		// Because we added it to the extra info list

				if (syms[ 0 ]->GetKind() == Symbols::ISymbol::kKindClass)
				{
					Symbols::ISymbol *constructor = GetLikeNamedSubSymbol( syms[ 0 ], syms[ 0 ]->GetName() );
					if (NULL != constructor)
					{
						fExtraInfoToAdd.insert( std::pair< Symbols::ISymbol *, Symbols::IExtraInfo * >( constructor, extraInfo ) );
						extraInfo->Retain();
					}
				}

			}

			for ( std::vector< Symbols::ISymbol *>::iterator it = syms.begin(); it != syms.end(); ++it)
				(*it)->Release();
		}

		orphanOwner->Release();
		
		// Since we've manually processed the right-hand side, we don't want to process it a second time
		return false;
	} else if (dynamic_cast< FunctionCallExpressionNode * >( inNode )) {
		// We have a function call, so we want to find the function's symbol and determine it's return
		// type, as that will be the right-hand symbol's new prototype.
		Node *functionCallLHS = dynamic_cast< FunctionCallExpressionNode * >( inNode )->GetLHS();
		Symbols::ISymbol *functionCall = NULL;
		if (QualifiedLookup( functionCallLHS, &functionCall )) {
			// If called function  is a constructor, we create a new instance of object type
			VString	jsDoc = functionCall->GetScriptDocComment();
			bool	isConstructorCall = false;

			if (! jsDoc.IsEmpty())
			{
				ScriptDocComment *comment = ScriptDocComment::Create( jsDoc );
				if (NULL != comment)
				{
					ScriptDocLexer::Element *element = comment->GetElement( IScriptDocCommentField::kConstructor );
					if ( NULL != element )
						isConstructorCall = true;

					comment->Release();
				}
			}

			if (isConstructorCall)
				CreateNewInstance( inLHS, functionCall );
			else
			{
				// Kludge to manage the "[ds.]SomeEntityModel.anyMethodReturningAnEntity" 
				// assignment case: it will create a new instance of "SomeEntityModel" with
				// the left hand side expression
				if ( functionCall->GetReturnTypes().size() )
				{
					Symbols::ISymbol* returnType  = functionCall->GetReturnTypes().front();
					if ( returnType->GetOwner() && 
						( returnType->GetOwner()->GetName() == CVSTR("Entity") || 
						  returnType->GetOwner()->GetName() == CVSTR("EntityCollection") ) )
					{
						if ( dynamic_cast< DotExpressionNode* >( functionCallLHS ) )
						{
							VString	entityModelName;
							Node*	expression = dynamic_cast< DotExpressionNode* >( functionCallLHS )->GetLHS();

							// Handle "ds.MyEntityModel.anyMethodReturningAnEntity" expression
							if ( dynamic_cast< DotExpressionNode* >( expression ) )
							{
								Node *left = dynamic_cast< DotExpressionNode* >( expression )->GetLHS();
								if ( NULL != left && dynamic_cast< IdentifierNode* >( left )->GetText() == CVSTR("ds") )
								{
									Node *right = dynamic_cast< DotExpressionNode* >( expression )->GetRHS();
									if (NULL != right)
										entityModelName = dynamic_cast< IdentifierNode* >( right )->GetText();
								}
							}
							// Handle "MyEntityModel." expression
							else if ( dynamic_cast< IdentifierNode* >( expression ) )
								entityModelName = dynamic_cast< IdentifierNode* >( expression )->GetText();

							// Now we have our Entity Model name we can find its symbol and create
							// a new instance of it with the left hand side of expression
							if ( ! entityModelName.IsEmpty() )
							{
								Symbols::ISymbol *entityModel;
								if ( UnqualifiedLookup( entityModelName, &entityModel ) )
								{
									Symbols::ISymbol *prototype = GetLikeNamedSubSymbol( entityModel, "prototype" );
									if (NULL != prototype)
									{
										inLHS->AddPrototype(prototype);
										prototype->Release();
									}
									entityModel->Release();
								}
							}
						}
					}
				}

                // This is the right placeholder for setting up our symbol in case of require call
                // inNode contains the name of the module file, we must get its FileID
                // Using the FileID, we must get the UniqueID of symbol exports defined in the required module file
                // Using the UniqueID of exports symbols, we must set the ReferenceID of ou symbol
                if( functionCall->GetName() == "require" )
                {
                    // Get require args i.e. the module name
                    FunctionCallArgumentsNode* args = dynamic_cast<FunctionCallExpressionNode*>(inNode)->GetArgs();
                    
                    // Check if we only have 1 arg i.e. only one module name specified
                    if( args->ArgumentCount() == 1 )
                    {
                        // Get the module name node
                        StringLiteralNode* module = dynamic_cast<StringLiteralNode*>(args->GetArgument(0));
                        if( module )
                        {
                            VString moduleName = module->GetValue();
                            // Strip ' from module name
                            if( moduleName.BeginsWith("'") || moduleName.BeginsWith("\"") )  moduleName.Remove(1, 1);
                            if( moduleName.EndsWith("'") || moduleName.EndsWith("\"") )    moduleName.Remove(moduleName.GetLength(), 1);
                            
                            // Once we have the module name, we have to get its unique file ID
                            std::vector<Symbols::IFile*> files = this->fSymTable->GetFilesByName(moduleName);
                            if( files.size() >= 1 )
                            {
                                std::vector<Symbols::IFile*>::const_iterator file;
                                for(file=files.begin(); file!=files.end(); file++)
                                {
									if(		(*file)->GetBaseFolder() == eSymbolFileBaseFolderServerModules ||
											((*file)->GetBaseFolder() == eSymbolFileBaseFolderProject && (*file)->GetPath().BeginsWith("Modules/")) )
                                    {
                                        // Once we have the module file ID, we can get all "exports" symbols and search for the one declared in our module file
                                        std::vector<Symbols::ISymbol*> exports = this->fSymTable->GetSymbolsByName(NULL, "exports", (*file));
                                        if( exports.size() == 1 )
                                        {
                                            // Use this symbol as a reference for the LHS symbol
                                            inLHS->SetReferenceSymbol(exports.at(0));
                                            functionCall->AddReturnType(exports.at(0));
                                            break;
                                        }
                                    }                                    
                                }
								
								// Release memory
								for(file=files.begin(); file!=files.end(); file++)
									(*file)->Release();
                            }
                        }
                    }
                }

				// It wasn't an Entity Model assignment
				inLHS->AddPrototypes( functionCall->GetReturnTypes() );
			}

			functionCall->Release();
		}
	}
	else if ( dynamic_cast< LiteralNode * >( inNode ) || dynamic_cast< AdditionExpressionNode * >( inNode ) )
	{
		Node *node;
		
		// In case of addition expression we will use in node LHS as reference in place of in node.
		if ( dynamic_cast< AdditionExpressionNode * >( inNode ) )
			node = dynamic_cast< BinaryExpressionNode * >( inNode )->GetLHS();
		else
			node = inNode;

		// If we've gotten a literal, we need to attach some prototype information to the incoming symbol
 		Symbols::ISymbol *typeSym = NULL;
		std::vector< Symbols::ISymbol * > typeSyms;

		if (dynamic_cast< StringLiteralNode * >( node ))
			typeSym = GetLikeNamedSubSymbol( NULL, "String" );
		else if (dynamic_cast< NumericLiteralNode * >( node ))
			typeSym = GetLikeNamedSubSymbol( NULL, "Number" );
		else if (dynamic_cast< BooleanLiteralNode * >( node ))
			typeSym = GetLikeNamedSubSymbol( NULL, "Boolean" );
		else if (dynamic_cast< RegExLiteralNode * >( node ))
			typeSym = GetLikeNamedSubSymbol( NULL, "RegExp" );
		else if (dynamic_cast< ArrayLiteralNode * >( node ))
			typeSym = GetLikeNamedSubSymbol( NULL, "Array" );

		if (typeSym)
		{
			// The literals are created as if the user said new Type(), which means we need to
			// get the external prototype property of the literal.
			CreateNewInstance( inLHS, typeSym );
			typeSym->Release();
		}
	}
	
	return true;
}

bool DeclarationVisitor::VisitAssignExpressionNode( class AssignExpressionNode *inNode )
{
	// The assignment expression is really the only tricky problem in declaration parsing.  The
	// name on the left-hand side of the assignment may become a new symbol.  But it may also be
	// an existing symbol.  We will try to locate the symbol, and if we cannot, we will create a
	// new one and add it to the proper owner.
	Symbols::ISymbol *lhs = GetSymbolFromLeftHandSideNode( dynamic_cast< LeftHandSideExpressionNode * >( inNode->GetLHS() ) );
	if (!lhs)	return true;

	if (dynamic_cast< LeftHandSideExpressionNode * >( inNode->GetLHS() )->GetScriptDocComment()) {
		lhs->SetScriptDocComment( dynamic_cast< LeftHandSideExpressionNode * >( inNode->GetLHS() )->GetScriptDocComment()->GetText() );
	}

	// Once we have the left-hand side symbol worked out, we want to see if the right-hand side
	// makes for a simple assignment.  If the right-hand side is a literal, identifier or object 
	// literal, or new expression, we will convey the type information from the right-hand side to
	// the symbol we found on the left-hand side.
	bool ret = VisitRightHandSideOfAssignment( lhs, inNode->GetRHS() );
	lhs->Release();

	// Empty the statement new symbols list as 
	// they are already flagged as undefined
	fSymbolsToAddToStatement.empty();

	return ret;
}

void DeclarationVisitor::SetSymbolReturnTypesFromScriptDoc(Symbols::ISymbol* inSym, bool inAddToUpdateList, bool inUpdateSymbol)
{
	bool complete = true;
	sLONG nbrTypes = 0;

	ScriptDocComment *comment = ScriptDocComment::Create( inSym->GetScriptDocComment() );
	if ( NULL != comment )
	{
		ScriptDocLexer::Element *element = comment->GetElement( IScriptDocCommentField::kReturn );
		if ( NULL != element )
		{
			VString typeInfo = static_cast< ScriptDocLexer::ReturnElement * >( element )->fTypeList;
			VectorOfVString results;

			typeInfo.GetSubStrings( CHAR_VERTICAL_LINE, results, false, true);
			if ( ! results.empty() )
			{
				for (VectorOfVString::iterator itReturn = results.begin(); itReturn != results.end(); ++itReturn)
				{
					Symbols::ISymbol *returnTypeSym = NULL;
					if (UnqualifiedLookup( *itReturn, &returnTypeSym ))
					{
						Symbols::ISymbol *refReturnTypeSym = const_cast<Symbols::ISymbol *>( returnTypeSym->RetainReferencedSymbol() );
						Symbols::ISymbol *prototype = GetLikeNamedSubSymbol( refReturnTypeSym, "prototype" );
						if (NULL != prototype)
						{
							inSym->AddReturnType( prototype );
							prototype->Release();
							nbrTypes++;
						}
						returnTypeSym->Release();
						refReturnTypeSym->Release();
					}
					else
						complete = false;
				}

				// add symbol to update list if necessary
				if ( ! complete && inAddToUpdateList )
				{
					fSymbolsToUpdate.push_back(inSym);
					inSym->Retain();
				}

				// update symbol in database if necessary
				if (nbrTypes && inUpdateSymbol)
					fSymTable->UpdateSymbol(inSym);
			}
		}
		comment->Release();
	}
}

void DeclarationVisitor::SetSymbolClassesFromScriptDoc(Symbols::ISymbol* inSym, bool inAddToUpdateList, bool inUpdateSymbol)
{						 
	ScriptDocComment *comment = ScriptDocComment::Create( inSym->GetScriptDocComment() );
	if ( NULL != comment )
	{
		// get appropriate JSDoc tag for iheritance
		ScriptDocLexer::Element *element = comment->GetElement( IScriptDocCommentField::kExtends );
		if (NULL == element)
			element = comment->GetElement( IScriptDocCommentField::kInherits );
		if (NULL == element)
			element = comment->GetElement( IScriptDocCommentField::kAugments );

		if ( NULL != element)
		{
			VString baseClasses = static_cast< ScriptDocLexer::ExtendsElement * >( element )->fBaseClass;
			VectorOfVString classes;

			baseClasses.GetSubStrings( CHAR_VERTICAL_LINE, classes, false, true);
			if ( ! classes.empty() )
				SetSymbolPrototypes( inSym, classes, inAddToUpdateList, inUpdateSymbol );
		}
		comment->Release();
	}
}

void DeclarationVisitor::SetSymbolTypesFromScriptDoc(Symbols::ISymbol* inSym, bool inAddToUpdateList, bool inUpdateSymbol)
{						 
	ScriptDocComment *comment = ScriptDocComment::Create( inSym->GetScriptDocComment() );
	if ( NULL != comment )
	{
		ScriptDocLexer::Element *element = comment->GetElement( IScriptDocCommentField::kType );
		if ( NULL != element)
		{
			VString typesStr = static_cast< ScriptDocLexer::TypeElement * >( element )->fTypes;
			VectorOfVString types;

			typesStr.GetSubStrings( CHAR_VERTICAL_LINE, types, false, true);
			if ( ! types.empty() )
				SetSymbolPrototypes( inSym, types, inAddToUpdateList, inUpdateSymbol );
		}
		comment->Release();
	}
}

void DeclarationVisitor::SetSymbolPrototypes(Symbols::ISymbol* inSym, const VectorOfVString& inTypes, bool inAddToUpdateList, bool inUpdateSymbol)
{
	if ( NULL != inSym && inTypes.size() )
	{
		bool complete = true;
		sLONG nbrTypes = 0;

		for (VectorOfVString::const_iterator itType = inTypes.begin(); itType != inTypes.end(); ++itType)
		{
			Symbols::ISymbol *typeSym = NULL;
			if ( UnqualifiedLookup( *itType, &typeSym ) )
			{
				CreateNewInstance( inSym, typeSym );
				typeSym->Release();
				nbrTypes++;
			}
			else
				complete = false;
		}

		// add symbol to update list if necessary
		if ( ! complete && inAddToUpdateList )
		{
			fSymbolsToUpdate.push_back(inSym);
			inSym->Retain();
		}

		// update symbol in database if necessary
		if (nbrTypes && inUpdateSymbol)
			fSymTable->UpdateSymbol(inSym);
	}
}


void DeclarationVisitor::SetSymbolClassKindFromScriptDoc(Symbols::ISymbol* inSym)
{						 
	ScriptDocComment *comment = ScriptDocComment::Create( inSym->GetScriptDocComment() );
	if ( NULL != comment )
	{
		ScriptDocLexer::Element *element = comment->GetElement( IScriptDocCommentField::kClass );
		if (NULL == element)
			element = comment->GetElement( IScriptDocCommentField::kConstructor );

		if (NULL != element)
			inSym->SetKind( Symbols::ISymbol::kKindClass );

		comment->Release();
	}
}


Visitor *Visitor::GetDeclarationParser( ISymbolTable *inSymTable, Symbols::IFile *inOwningFile, JavaScriptParserDelegate *inDelegate )
{
	return new DeclarationVisitor( inSymTable, inOwningFile, inDelegate );
}

#if VERSIONDEBUG
sLONG Node::sNbNodes=0;
#endif
void Node::DebugDump()
{
#if VERSIONDEBUG
	VString nbNodesStr;
	VLong nbNodes(sNbNodes);

	nbNodes.GetString(nbNodesStr);
	nbNodesStr = CVSTR("JS Nodes: ") + nbNodesStr;
#if VERSIONWIN
	OutputDebugStringW( nbNodesStr.GetCPointer() );
#else
	;
#endif
#endif
}



/*******
*	I am disabling this code because it's not being used.  However, 
*	there is no reason we can't use it sometime in the future.  It
*	follows function calls to determine a call graph for the user.
*	It still needs some work in that NoteConnection is a noop, but
*	despite that, it's still functional.
********

class CallGraphWalker : public Visitor {
protected:
	std::vector< Node * > mCallStack;

	void NoteConnection( Node *fromNode, Node *toNode );

public:
	virtual bool VisitFunctionExpressionNodeEnter( class FunctionExpressionNode *inNode );
	virtual bool VisitFunctionExpressionNodeLeave( class FunctionExpressionNode *inNode );
	virtual bool VisitFunctionDeclarationStatementNodeEnter( class FunctionDeclarationStatementNode *inNode );
	virtual bool VisitFunctionDeclarationStatementNodeLeave( class FunctionDeclarationStatementNode *inNode );

	virtual bool VisitFunctionCallExpressionNode( class FunctionCallExpressionNode *inNode );
};

bool CallGraphWalker::VisitFunctionExpressionNodeEnter( FunctionExpressionNode *inNode )
{
	mCallStack.push_back( inNode );
	return true;
}

bool CallGraphWalker::VisitFunctionExpressionNodeLeave( FunctionExpressionNode *inNode )
{
	mCallStack.pop_back();
	return true;
}

bool CallGraphWalker::VisitFunctionDeclarationStatementNodeEnter( FunctionDeclarationStatementNode *inNode )
{
	mCallStack.push_back( inNode );
	return true;
}

bool CallGraphWalker::VisitFunctionDeclarationStatementNodeLeave( FunctionDeclarationStatementNode *inNode )
{
	mCallStack.pop_back();
	return true;
}

bool CallGraphWalker::VisitFunctionCallExpressionNode( FunctionCallExpressionNode *inNode )
{
	NoteConnection( mCallStack.empty() ? NULL : mCallStack.back(), inNode );
	return true;
}

void CallGraphWalker::NoteConnection( Node *fromNode, Node *toNode )
{
	int foo = 666;
}

Visitor *Visitor::GetCallGraphWalker()
{
	return new CallGraphWalker();
}
*/
