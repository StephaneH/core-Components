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




/*
 *
 *
 *
 */
SymbolScopeDelaration::SymbolScopeDelaration(SymbolScopeDelaration* inParent, const XBOX::VString& inName, const EASTSymbolScopeType& inType):
fParent(inParent),
fName(inName),
fType(inType)
{
}

SymbolScopeDelaration::~SymbolScopeDelaration()
{
	ISymbolDeclaration::DeleteSymbolDeclarations( fAssignments );
	ISymbolDeclaration::DeleteSymbolDeclarations( fVariables );
	ISymbolDeclaration::DeleteSymbolDeclarations( fFunctions );
	ISymbolDeclaration::DeleteSymbolDeclarations( fFunctionExpressions );
	ISymbolDeclaration::DeleteSymbolDeclarations( fNews );

	VIndex total = fChildren.size();
	for(VIndex index=total-1; index>=0; --index)
	{
		// Get the last element
		SymbolScopeDelaration* cur = fChildren[index];

		// !!! RELEASE MEMORY !!!
		delete cur;
		cur = NULL;
		
		// Remove the last element from the vector
		fChildren.pop_back();
	}
}

SymbolScopeDelaration* SymbolScopeDelaration::GetParentScope() const
{
	return fParent;
}

const XBOX::VString& SymbolScopeDelaration::GetScopeName() const
{
	return fName;
}
const EASTSymbolScopeType& SymbolScopeDelaration::GetScopeType() const
{
	return fType;
}
	
void SymbolScopeDelaration::AddChildScope(SymbolScopeDelaration* inChild)
{
	fChildren.push_back(inChild);
}

XBOX::VIndex SymbolScopeDelaration::GetChildScopeCount() const
{
	return fChildren.size();
}

SymbolScopeDelaration* SymbolScopeDelaration::GetChildScopeAt(XBOX::VIndex inIndex) const
{
	return fChildren[inIndex];
}

void SymbolScopeDelaration::AddAssignmentDeclaration(ISymbolDeclaration* inDeclaration)
{
	fAssignments.push_back(inDeclaration);
}

const std::vector<ISymbolDeclaration*>& SymbolScopeDelaration::GetAssignmentDeclarations() const
{
	return fAssignments;
}

void SymbolScopeDelaration::AddVariableDeclaration(ISymbolDeclaration* inDeclaration)
{
	fVariables.push_back(inDeclaration);
}

const std::vector<ISymbolDeclaration*>& SymbolScopeDelaration::GetVariableDeclarations() const
{
	return fVariables;
}

void SymbolScopeDelaration::AddFunctionDeclaration(ISymbolDeclaration* inDeclaration)
{
	fFunctions.push_back(inDeclaration);
}

const std::vector<ISymbolDeclaration*>& SymbolScopeDelaration::GetFunctionDeclarations() const
{
	return fFunctions;
}

void SymbolScopeDelaration::AddFunctionExpressionDeclaration(ISymbolDeclaration* inDeclaration)
{
	fFunctionExpressions.push_back(inDeclaration);
}

const std::vector<ISymbolDeclaration*>& SymbolScopeDelaration::GetFunctionExpressionDeclarations() const
{
	return fFunctionExpressions;
}

void SymbolScopeDelaration::AddNewDeclaration(ISymbolDeclaration* inDeclaration)
{
	fNews.push_back(inDeclaration);
}

const std::vector<ISymbolDeclaration*>& SymbolScopeDelaration::GetNewDeclarations() const
{
	return fNews;
}









void SymbolScopeDelaration::_PrintScope(const XBOX::VString& inTabSpace, SymbolScopeDelaration* inScope)
{
	// Print Scope Infos
	XBOX::VString msg = inTabSpace;
	msg.AppendString("SCOPE TYPE : ");
	switch(inScope->GetScopeType())
	{
		case SymbolScopeDelaration::kTypeGlobal:	msg.AppendString("GLOBAL");			break;
		case SymbolScopeDelaration::kTypeFunction:	msg.AppendString("FUNCTION");		break;
		case SymbolScopeDelaration::kTypeCatch:		msg.AppendString("CATCH");			break;
		case SymbolScopeDelaration::kTypeWith:		msg.AppendString("WITH");			break;
		default:									msg.AppendString("UNKOWN");			break;
	}
	msg.AppendString("\n");
	DebugMsg(msg);
}

void SymbolScopeDelaration::_PrintDeclarations(const XBOX::VString& inTabSpace, const XBOX::VString& inCountMsg, const std::vector<ISymbolDeclaration*>& inDeclarations)
{
	XBOX::VString msg = inTabSpace;
	msg.AppendString(inCountMsg);
	msg.AppendLong(inDeclarations.size());
	msg.AppendString("\n");
	DebugMsg(msg);
	for(std::vector<ISymbolDeclaration*>::const_iterator it=inDeclarations.begin(); it!=inDeclarations.end(); ++it)
	{
		msg = inTabSpace;
		(*it)->ToString(msg);
		msg.AppendString("\n");
		DebugMsg(msg);
	}
}

void SymbolScopeDelaration::Print(SymbolScopeDelaration* inScope, const XBOX::VString& inTabSpace)
{
	XBOX::VString curTab = inTabSpace + "\t";
	
	// Print scope basic informations
	_PrintScope(curTab, inScope);
	
	// Print variable declarations
	_PrintDeclarations(curTab, "VARIABLE DECLARATIONS COUNT : ", inScope->SymbolScopeDelaration::GetVariableDeclarations());
	
	// Print function declarations
	_PrintDeclarations(curTab, "FUNCTION DECLARATIONS COUNT : ", inScope->SymbolScopeDelaration::GetFunctionDeclarations());

	// Print function declarations
	_PrintDeclarations(curTab, "FUNCTION EXPRESSION DECLARATIONS COUNT : ", inScope->SymbolScopeDelaration::GetFunctionExpressionDeclarations());

	// Print assignment declarations
	_PrintDeclarations(curTab, "ASSIGNMENT DECLARATIONS COUNT : ", inScope->SymbolScopeDelaration::GetAssignmentDeclarations());

	// Print new declarations
	_PrintDeclarations(curTab, "NEW DECLARATIONS COUNT : ", inScope->SymbolScopeDelaration::GetNewDeclarations());

	// Move to child scope
	XBOX::VIndex total = inScope->GetChildScopeCount();
	for(XBOX::VIndex index=0; index<total ; index++)	{	SymbolScopeDelaration::Print(inScope->GetChildScopeAt(index), curTab);	}
}









/*
 *
 *
 *
 */
Node::Node( int inLineNumber ):
fLineNumber( inLineNumber ),
fLineCompletion( -1 ),
fScriptDoc( NULL )
{
}
	
Node::~Node()
{
	delete fScriptDoc;
}
	
void Node::AttachScriptDocComment( ScriptDocComment *inComment )
{
	fScriptDoc = inComment;
}
	
ScriptDocComment* Node::GetScriptDocComment()
{
	return fScriptDoc;
}

bool Node::IsLeftHandSideExpression() const
{
	return false;
}
	
bool Node::IsLiteral() const
{
	return false;
}
	
int Node::GetLineNumber()
{
	return fLineNumber;
}
	
int Node::GetLineCompletion()
{
	return ((-1 == fLineCompletion)?(fLineNumber):(fLineCompletion));
}
	
XBOX::VString Node::GetWAFComment()
{
	return fWAFComment;
}
	
void Node::AttachWAFComment(const XBOX::VString& inWAFComment)
{
	fWAFComment = inWAFComment;
	if ( fWAFComment[fWAFComment.GetLength() - 1] == CHAR_CONTROL_000D)
		fWAFComment.SubString(1, fWAFComment.GetLength() - 1);
}

EASTNodeType Node::GetNodeTypeNG() const
{
	return Node::kTypeBaseNode;
}

const XBOX::VString& Node::GetIdentifierNG() const
{
	return fName;
}

	
	
/*
 *
 *
 *
 */
ProgramNode::ProgramNode( Node *inStatements ):
Node( -1 ),
fStatements( inStatements )
{
}

ProgramNode::~ProgramNode()
{
	delete fStatements;
}

bool ProgramNode::Accept( class Visitor *visit )
{
	if (!visit->VisitProgramNodeEnter( this ))
		return false;
		
	if (fStatements)
		fStatements->Accept( visit );
		
	return visit->VisitProgramNodeLeave( this );
}

EASTNodeType ProgramNode::GetNodeTypeNG() const
{
	return Node::kTypeProgramNode;
}



/*
 *
 *
 *
 */
FunctionCallArgumentsNode::FunctionCallArgumentsNode( int inLineNo ):
Node( inLineNo )
{
}
	
FunctionCallArgumentsNode::~FunctionCallArgumentsNode()
{
	while (!fArgs.empty())
	{
		delete fArgs.back();
		fArgs.pop_back();
	}
}
	
bool FunctionCallArgumentsNode::Accept( class Visitor *visit )
{
	if (!visit->VisitFunctionCallArgumentsNodeEnter( this ))
		return false;
	
	for (std::vector< Node * >::iterator iter = fArgs.begin(); iter != fArgs.end(); ++iter)
	{
		if (!(*iter)->Accept( visit ))
			return false;
	}
	
	return visit->VisitFunctionCallArgumentsNodeLeave( this );
}
	
void FunctionCallArgumentsNode::AddArgument( Node *inNode )
{
	fArgs.push_back( inNode );
}

size_t FunctionCallArgumentsNode::ArgumentCount()
{
	return fArgs.size();
}
	
Node* FunctionCallArgumentsNode::GetArgument( size_t inIndex )
{
	return fArgs[ inIndex ];
}

EASTNodeType FunctionCallArgumentsNode::GetNodeTypeNG() const
{
	return Node::kTypeFunctionCallArgumentsNode;
}




/*
 *
 *
 *
 */
FunctionDeclarationArgumentsNode::FunctionDeclarationArgumentsNode( int inLineNo ):
Node( inLineNo )
{
}

FunctionDeclarationArgumentsNode::~FunctionDeclarationArgumentsNode()
{
	while (!fArgs.empty())
	{
		delete fArgs.back();
		fArgs.pop_back();
	}
}

bool FunctionDeclarationArgumentsNode::Accept( class Visitor *visit )
{
	if (!visit->VisitFunctionDeclarationArgumentsNodeEnter( this ))
		return false;
	
	for (std::vector< Node * >::iterator iter = fArgs.begin(); iter != fArgs.end(); ++iter)
	{
		if (!(*iter)->Accept( visit ))
			return false;
	}
	
	return visit->VisitFunctionDeclarationArgumentsNodeLeave( this );
}

void FunctionDeclarationArgumentsNode::AddArgument( Node *inNode )
{
	fArgs.push_back( inNode );
}

size_t FunctionDeclarationArgumentsNode::ArgumentCount()
{
	return fArgs.size();
}

Node* FunctionDeclarationArgumentsNode::GetArgument( size_t inIndex )
{
	return fArgs[ inIndex ];
}

EASTNodeType FunctionDeclarationArgumentsNode::GetNodeTypeNG() const
{
	return Node::kTypeFunctionDeclarationArgumentsNode;
}




/*
 *
 *
 *
 */
LeftHandSideExpressionNode::LeftHandSideExpressionNode( int inLineNo ):
Node( inLineNo )
{
}

bool LeftHandSideExpressionNode::IsLeftHandSideExpression() const
{
	return true;
}
	
bool LeftHandSideExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitLeftHandSideExpressionNodeEnter( this ) )
		return false;
	
	return visit->VisitLeftHandSideExpressionNodeLeave( this );
}

EASTNodeType LeftHandSideExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeLeftHandSideExpressionNode;
}




/*
 *
 *
 *
 */
UnaryExpressionNode::UnaryExpressionNode( Node *inLHS, int inLineNo ):
Node( inLineNo ),
fLHS( inLHS )
{
}

UnaryExpressionNode::~UnaryExpressionNode()
{
	delete fLHS;
}

bool UnaryExpressionNode::Accept( class Visitor *visit )
{
	if (fLHS)
		return fLHS->Accept( visit );
	
	return false;
}

EASTNodeType UnaryExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeUnaryExpressionNode;
}




/*
 *
 *
 *
 */
DeleteExpressionNode::DeleteExpressionNode( Node *inLHS, int inLineNo ):
UnaryExpressionNode( inLHS, inLineNo )
{
}

bool DeleteExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitDeleteExpressionNodeEnter( this ) )
		return false;
	
	if( !UnaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitDeleteExpressionNodeLeave( this );
}

EASTNodeType DeleteExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeDeleteExpressionNode;
}




/*
 *
 *
 *
 */
VoidExpressionNode::VoidExpressionNode( Node *inLHS, int inLineNo ):
UnaryExpressionNode( inLHS, inLineNo )
{
}

bool VoidExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitVoidExpressionNodeEnter( this ) )
		return false;
	
	if( !UnaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitVoidExpressionNodeLeave( this );
}

EASTNodeType VoidExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeVoidExpressionNode;
}




/*
 *
 *
 *
 */
TypeOfExpressionNode::TypeOfExpressionNode( Node *inLHS, int inLineNo ):
UnaryExpressionNode( inLHS, inLineNo )
{
}

bool TypeOfExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitTypeOfExpressionNodeEnter( this ) )
		return false;
	
	if( !UnaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitTypeOfExpressionNodeLeave( this );
}

EASTNodeType TypeOfExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeTypeOfExpressionNode;
}




/*
 *
 *
 *
 */
PreIncrementorExpressionNode::PreIncrementorExpressionNode( Node *inLHS, int inLineNo ):
UnaryExpressionNode( inLHS, inLineNo )
{
}

bool PreIncrementorExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitPreIncrementorExpressionNodeEnter( this ) )
		return false;
	
	if( !UnaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitPreIncrementorExpressionNodeLeave( this );
}

EASTNodeType PreIncrementorExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypePreIncrementorExpressionNode;
}




/*
 *
 *
 *
 */
PreDecrementorExpressionNode::PreDecrementorExpressionNode( Node *inLHS, int inLineNo ):
UnaryExpressionNode( inLHS, inLineNo )
{
}

bool PreDecrementorExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitPreDecrementorExpressionNodeEnter( this ) )
		return false;
	
	if( !UnaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitPreDecrementorExpressionNodeLeave( this );
}

EASTNodeType PreDecrementorExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypePreDecrementorExpressionNode;
}




/*
 *
 *
 *
 */
UnaryPlusExpressionNode::UnaryPlusExpressionNode( Node *inLHS, int inLineNo ):
UnaryExpressionNode( inLHS, inLineNo )
{
}

bool UnaryPlusExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitUnaryPlusExpressionNodeEnter( this ) )
		return false;
	
	if( !UnaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitUnaryPlusExpressionNodeLeave( this );
}

EASTNodeType UnaryPlusExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeUnaryPlusExpressionNode;
}




/*
 *
 *
 *
 */
UnaryNegateExpressionNode::UnaryNegateExpressionNode( Node *inLHS, int inLineNo ):
UnaryExpressionNode( inLHS, inLineNo )
{
}

bool UnaryNegateExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitUnaryNegateExpressionNodeEnter( this ) )
		return false;
	
	if( !UnaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitUnaryNegateExpressionNodeLeave( this );
}

EASTNodeType UnaryNegateExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeUnaryNegateExpressionNode;
}




/*
 *
 *
 *
 */
BitwiseNotExpressionNode::BitwiseNotExpressionNode( Node *inLHS, int inLineNo ):
UnaryExpressionNode( inLHS, inLineNo )
{
}

bool BitwiseNotExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitBitwiseNotExpressionNodeEnter( this ) )
		return false;
	
	if( !UnaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitBitwiseNotExpressionNodeLeave( this );
}

EASTNodeType BitwiseNotExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeBitwiseNotExpressionNode;
}




/*
 *
 *
 *
 */
LogicalNotExpressionNode::LogicalNotExpressionNode( Node *inLHS, int inLineNo ):
UnaryExpressionNode( inLHS, inLineNo )
{
}

bool LogicalNotExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitLogicalNotExpressionNodeEnter( this ) )
		return false;
	
	if( !UnaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitLogicalNotExpressionNodeLeave( this );
}

EASTNodeType LogicalNotExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeLogicalNotExpressionNode;
}




/*
 *
 *
 *
 */
PostIncrementorNode::PostIncrementorNode( Node *inLHS, int inLineNo ):
UnaryExpressionNode( inLHS, inLineNo )
{
}

bool PostIncrementorNode::Accept( class Visitor *visit )
{
	if( !visit->VisitPostIncrementorNodeEnter( this ) )
		return false;
	
	if( !UnaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitPostIncrementorNodeLeave( this );
}

EASTNodeType PostIncrementorNode::GetNodeTypeNG() const
{
	return Node::kTypePostIncrementorNode;
}




/*
 *
 *
 *
 */
PostDecrementorNode::PostDecrementorNode( Node *inLHS, int inLineNo ):
UnaryExpressionNode( inLHS, inLineNo )
{
}

bool PostDecrementorNode::Accept( class Visitor *visit )
{
	if( !visit->VisitPostDecrementorNodeEnter( this ) )
		return false;
	
	if( !UnaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitPostDecrementorNodeLeave( this );
}

EASTNodeType PostDecrementorNode::GetNodeTypeNG() const
{
	return Node::kTypePostDecrementorNode;
}




/*
 *
 *
 *
 */
BinaryExpressionNode::BinaryExpressionNode( Node *inLHS, Node *inRHS, int inLineNo ):
Node( inLineNo ),
fLHS( inLHS ),
fRHS( inRHS )
{
}

BinaryExpressionNode::~BinaryExpressionNode()
{
	delete fLHS;
	delete fRHS;
}

bool BinaryExpressionNode::Accept( class Visitor *visit )
{
	if (fLHS && !fLHS->Accept( visit ))
		return false;
	
	if (fRHS && !fRHS->Accept( visit ))
		return false;
	
	return true;
}

Node* BinaryExpressionNode::GetLHS() const
{
	return fLHS;
}

Node* BinaryExpressionNode::GetRHS() const
{
	return fRHS;
}

EASTNodeType BinaryExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeBinaryExpressionNode;
}




/*
 *
 *
 *
 */
AdditionExpressionNode::AdditionExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool AdditionExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitAdditionExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitAdditionExpressionNodeLeave( this );
}

EASTNodeType AdditionExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeAdditionExpressionNode;
}




/*
 *
 *
 *
 */
SubtractionExpressionNode::SubtractionExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool SubtractionExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitSubtractionExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitSubtractionExpressionNodeLeave( this );
}

EASTNodeType SubtractionExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeSubtractionExpressionNode;
}




/*
 *
 *
 *
 */
MultiplicationExpressionNode::MultiplicationExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool MultiplicationExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitMultiplicationExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitMultiplicationExpressionNodeLeave( this );
}

EASTNodeType MultiplicationExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeMultiplicationExpressionNode;
}




/*
 *
 *
 *
 */
DivisionExpressionNode::DivisionExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool DivisionExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitDivisionExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitDivisionExpressionNodeLeave( this );
}

EASTNodeType DivisionExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeDivisionExpressionNode;
}




/*
 *
 *
 *
 */
LeftShiftExpressionNode::LeftShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool LeftShiftExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitLeftShiftExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitLeftShiftExpressionNodeLeave( this );
}

EASTNodeType LeftShiftExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeLeftShiftExpressionNode;
}




/*
 *
 *
 *
 */
SignedRightShiftExpressionNode::SignedRightShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool SignedRightShiftExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitSignedRightShiftExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitSignedRightShiftExpressionNodeLeave( this );
}

EASTNodeType SignedRightShiftExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeSignedRightShiftExpressionNode;
}




/*
 *
 *
 *
 */
UnsignedRightShiftExpressionNode::UnsignedRightShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool UnsignedRightShiftExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitUnsignedRightShiftExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitUnsignedRightShiftExpressionNodeLeave( this );
}

EASTNodeType UnsignedRightShiftExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeUnsignedRightShiftExpressionNode;
}




/*
 *
 *
 *
 */
InExpressionNode::InExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool InExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitInExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitInExpressionNodeLeave( this );
}

EASTNodeType InExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeInExpressionNode;
}




/*
 *
 *
 *
 */
LessThanExpressionNode::LessThanExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool LessThanExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitLessThanExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitLessThanExpressionNodeLeave( this );
}

EASTNodeType LessThanExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeLessThanExpressionNode;
}




/*
 *
 *
 *
 */
GreaterThanExpressionNode::GreaterThanExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool GreaterThanExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitGreaterThanExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitGreaterThanExpressionNodeLeave( this );
}

EASTNodeType GreaterThanExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeGreaterThanExpressionNode;
}




/*
 *
 *
 *
 */
LessThanOrEqualToExpressionNode::LessThanOrEqualToExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool LessThanOrEqualToExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitLessThanOrEqualToExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitLessThanOrEqualToExpressionNodeLeave( this );
}

EASTNodeType LessThanOrEqualToExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeLessThanOrEqualToExpressionNode;
}




/*
 *
 *
 *
 */
GreaterThanOrEqualToExpressionNode::GreaterThanOrEqualToExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool GreaterThanOrEqualToExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitGreaterThanOrEqualToExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitGreaterThanOrEqualToExpressionNodeLeave( this );
}

EASTNodeType GreaterThanOrEqualToExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeGreaterThanOrEqualToExpressionNode;
}




/*
 *
 *
 *
 */
InstanceOfExpressionNode::InstanceOfExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool InstanceOfExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitInstanceOfExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitInstanceOfExpressionNodeLeave( this );
}

EASTNodeType InstanceOfExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeInstanceOfExpressionNode;
}




/*
 *
 *
 *
 */
EqualityExpressionNode::EqualityExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool EqualityExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitEqualityExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitEqualityExpressionNodeLeave( this );
}

EASTNodeType EqualityExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeEqualityExpressionNode;
}




/*
 *
 *
 *
 */
NonEqualityExpressionNode::NonEqualityExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool NonEqualityExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitNonEqualityExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitNonEqualityExpressionNodeLeave( this );
}

EASTNodeType NonEqualityExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeNonEqualityExpressionNode;
}




/*
 *
 *
 *
 */
StrictEqualityExpressionNode::StrictEqualityExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool StrictEqualityExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitStrictEqualityExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitStrictEqualityExpressionNodeLeave( this );
}

EASTNodeType StrictEqualityExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeStrictEqualityExpressionNode;
}




/*
 *
 *
 *
 */
StrictNonEqualityExpressionNode::StrictNonEqualityExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool StrictNonEqualityExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitStrictNonEqualityExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitStrictNonEqualityExpressionNodeLeave( this );
}

EASTNodeType StrictNonEqualityExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeStrictNonEqualityExpressionNode;
}




/*
 *
 *
 *
 */
BitwiseAndExpressionNode::BitwiseAndExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool BitwiseAndExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitBitwiseAndExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitBitwiseAndExpressionNodeLeave( this );
}

EASTNodeType BitwiseAndExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeBitwiseAndExpressionNode;
}




/*
 *
 *
 *
 */
BitwiseXorExpressionNode::BitwiseXorExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool BitwiseXorExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitBitwiseXorExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitBitwiseXorExpressionNodeLeave( this );
}

EASTNodeType BitwiseXorExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeBitwiseXorExpressionNode;
}




/*
 *
 *
 *
 */
BitwiseOrExpressionNode::BitwiseOrExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool BitwiseOrExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitBitwiseOrExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitBitwiseOrExpressionNodeLeave( this );
}

EASTNodeType BitwiseOrExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeBitwiseOrExpressionNode;
}




/*
 *
 *
 *
 */
LogicalAndExpressionNode::LogicalAndExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool LogicalAndExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitLogicalAndExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitLogicalAndExpressionNodeLeave( this );
}

EASTNodeType LogicalAndExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeLogicalAndExpressionNode;
}




/*
 *
 *
 *
 */
LogicalOrExpressionNode::LogicalOrExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool LogicalOrExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitLogicalOrExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitLogicalOrExpressionNodeLeave( this );
}

EASTNodeType LogicalOrExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeLogicalOrExpressionNode;
}




/*
 *
 *
 *
 */
ModulusExpressionNode::ModulusExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool ModulusExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitModulusExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitModulusExpressionNodeLeave( this );
}

EASTNodeType ModulusExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeModulusExpressionNode;
}




/*
 *
 *
 *
 */
AssignmentExpressionNode::AssignmentExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool AssignmentExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitAssignmentExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitAssignmentExpressionNodeLeave( this );
}

EASTNodeType AssignmentExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeAssignmentExpressionNode;
}




/*
 *
 *
 *
 */
AssignMultiplyExpressionNode::AssignMultiplyExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
AssignmentExpressionNode( inLeft, inRight, inLineNo )
{
}

bool AssignMultiplyExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitAssignMultiplyExpressionNodeEnter( this ) )
		return false;
	
	if( !AssignmentExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitAssignMultiplyExpressionNodeLeave( this );
}

EASTNodeType AssignMultiplyExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeAssignMultiplyExpressionNode;
}




/*
 *
 *
 *
 */
AssignAddExpressionNode::AssignAddExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
AssignmentExpressionNode( inLeft, inRight, inLineNo )
{
}

bool AssignAddExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitAssignAddExpressionNodeEnter( this ) )
		return false;
	
	if( !AssignmentExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitAssignAddExpressionNodeLeave( this );
}

EASTNodeType AssignAddExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeAssignAddExpressionNode;
}




/*
 *
 *
 *
 */
AssignSubtractExpressionNode::AssignSubtractExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
AssignmentExpressionNode( inLeft, inRight, inLineNo )
{
}

bool AssignSubtractExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitAssignSubtractExpressionNodeEnter( this ) )
		return false;
	
	if( !AssignmentExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitAssignSubtractExpressionNodeLeave( this );
}

EASTNodeType AssignSubtractExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeAssignSubtractExpressionNode;
}




/*
 *
 *
 *
 */
AssignDivideExpressionNode::AssignDivideExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
AssignmentExpressionNode( inLeft, inRight, inLineNo )
{
}

bool AssignDivideExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitAssignDivideExpressionNodeEnter( this ) )
		return false;
	
	if( !AssignmentExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitAssignDivideExpressionNodeLeave( this );
}

EASTNodeType AssignDivideExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeAssignDivideExpressionNode;
}




/*
 *
 *
 *
 */
AssignModulusExpressionNode::AssignModulusExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
AssignmentExpressionNode( inLeft, inRight, inLineNo )
{
}

bool AssignModulusExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitAssignModulusExpressionNodeEnter( this ) )
		return false;
	
	if( !AssignmentExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitAssignModulusExpressionNodeLeave( this );
}

EASTNodeType AssignModulusExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeAssignModulusExpressionNode;
}




/*
 *
 *
 *
 */
AssignLeftShiftExpressionNode::AssignLeftShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
AssignmentExpressionNode( inLeft, inRight, inLineNo )
{
}

bool AssignLeftShiftExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitAssignLeftShiftExpressionNodeEnter( this ) )
		return false;
	
	if( !AssignmentExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitAssignLeftShiftExpressionNodeLeave( this );
}

EASTNodeType AssignLeftShiftExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeAssignLeftShiftExpressionNode;
}




/*
 *
 *
 *
 */
AssignSignedRightShiftExpressionNode::AssignSignedRightShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
AssignmentExpressionNode( inLeft, inRight, inLineNo )
{
}

bool AssignSignedRightShiftExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitAssignSignedRightShiftExpressionNodeEnter( this ) )
		return false;
	
	if( !AssignmentExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitAssignSignedRightShiftExpressionNodeLeave( this );
}

EASTNodeType AssignSignedRightShiftExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeAssignSignedRightShiftExpressionNode;
}




/*
 *
 *
 *
 */
AssignUnsignedRightShiftExpressionNode::AssignUnsignedRightShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
AssignmentExpressionNode( inLeft, inRight, inLineNo )
{
}

bool AssignUnsignedRightShiftExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitAssignUnsignedRightShiftExpressionNodeEnter( this ) )
		return false;
	
	if( !AssignmentExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitAssignUnsignedRightShiftExpressionNodeLeave( this );
}

EASTNodeType AssignUnsignedRightShiftExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeAssignUnsignedRightShiftExpressionNode;
}




/*
 *
 *
 *
 */
AssignBitwiseAndExpressionNode::AssignBitwiseAndExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
AssignmentExpressionNode( inLeft, inRight, inLineNo )
{
}

bool AssignBitwiseAndExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitAssignBitwiseAndExpressionNodeEnter( this ) )
		return false;
	
	if( !AssignmentExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitAssignBitwiseAndExpressionNodeLeave( this );
}

EASTNodeType AssignBitwiseAndExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeAssignBitwiseAndExpressionNode;
}




/*
 *
 *
 *
 */
AssignBitwiseOrExpressionNode::AssignBitwiseOrExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
AssignmentExpressionNode( inLeft, inRight, inLineNo )
{
}

bool AssignBitwiseOrExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitAssignBitwiseOrExpressionNodeEnter( this ) )
		return false;
	
	if( !AssignmentExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitAssignBitwiseOrExpressionNodeLeave( this );
}

EASTNodeType AssignBitwiseOrExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeAssignBitwiseOrExpressionNode;
}




/*
 *
 *
 *
 */
AssignBitwiseXorExpressionNode::AssignBitwiseXorExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
AssignmentExpressionNode( inLeft, inRight, inLineNo )
{
}

bool AssignBitwiseXorExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitAssignBitwiseXorExpressionNodeEnter( this ) )
		return false;
	
	if( !AssignmentExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitAssignBitwiseXorExpressionNodeLeave( this );
}

EASTNodeType AssignBitwiseXorExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeAssignBitwiseXorExpressionNode;
}




/*
 *
 *
 *
 */
AssignExpressionNode::AssignExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
AssignmentExpressionNode( inLeft, inRight, inLineNo )
{
}

bool AssignExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitAssignExpressionNodeEnter( this ) )
		return false;
	
	if( !AssignmentExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitAssignExpressionNodeLeave( this );
}

EASTNodeType AssignExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeAssignExpressionNode;
}




/*
 *
 *
 *
 */
CommaExpressionNode::CommaExpressionNode( Node *inLeft, Node *inRight, int inLineNo ):
BinaryExpressionNode( inLeft, inRight, inLineNo )
{
}

bool CommaExpressionNode::Accept( class Visitor *visit )
{
	if( !visit->VisitCommaExpressionNodeEnter( this ) )
		return false;
	
	if( !BinaryExpressionNode::Accept( visit ) )
		return false;
	
	return visit->VisitCommaExpressionNodeLeave( this );
}

EASTNodeType CommaExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeCommaExpressionNode;
}




/*
 *
 *
 *
 */
ConditionalExpressionNode::ConditionalExpressionNode( Node *inExpression, Node *trueExpr, Node *falseExpr, int inLineNo ):
Node( inLineNo ),
fExpr( inExpression ),
fTrueExpr( trueExpr ),
fFalseExpr( falseExpr )
{
}

ConditionalExpressionNode::~ConditionalExpressionNode()
{
	delete fExpr;
	delete fTrueExpr;
	delete fFalseExpr;
}

bool ConditionalExpressionNode::Accept( class Visitor *visit )
{
	if (!visit->VisitConditionalExpressionNodeEnter( this ))
		return false;
		
	if (fExpr && !fExpr->Accept( visit ))
		return false;
	
	if (fTrueExpr && !fTrueExpr->Accept( visit ))
		return false;
	
	if (fFalseExpr && !fFalseExpr->Accept(visit ))
		return false;

	return visit->VisitConditionalExpressionNodeLeave( this );
}

EASTNodeType ConditionalExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeConditionalExpressionNode;
}




/*
 *
 *
 *
 */
MemberExpressionNode::MemberExpressionNode( int inLineNo ):
LeftHandSideExpressionNode( inLineNo)
{
}

EASTNodeType MemberExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeMemberExpressionNode;
}




/*
 *
 *
 *
 */
FunctionExpressionNode::FunctionExpressionNode( VString inIdent, FunctionDeclarationArgumentsNode *inArgs, Node *inBody, int inLineNo, int inEndLineNo ):
MemberExpressionNode( inLineNo ),
fIdentifier( inIdent ),
fArgs( inArgs ),
fBody( inBody )
{
	fLineCompletion = inEndLineNo;
}

FunctionExpressionNode::~FunctionExpressionNode()
{
	delete fArgs;
	delete fBody;
}

bool FunctionExpressionNode::Accept( class Visitor *visit )
{
	if (visit->VisitFunctionExpressionNodeEnter( this ))
	{
		bool keepGoing = true;
		if (keepGoing && fArgs)
			keepGoing = fArgs->Accept( visit );
		
		if (keepGoing && fBody)
			keepGoing = fBody->Accept( visit );
	}
	return visit->VisitFunctionExpressionNodeLeave( this );
}

const VString& FunctionExpressionNode::GetIdentifier() const
{
	return fIdentifier;
}

void FunctionExpressionNode::SetIdentifier(const VString& inIdentifier)
{
	fIdentifier = inIdentifier;
}

const VString& FunctionExpressionNode::GetFullName() const
{
	return fFullName;
}

void FunctionExpressionNode::SetFullName(const VString& inName)
{
	fFullName = inName;
}

FunctionDeclarationArgumentsNode* FunctionExpressionNode::GetArgs() const
{
	return fArgs;
}

EASTNodeType FunctionExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeFunctionExpressionNode;
}

const XBOX::VString& FunctionExpressionNode::GetIdentifierNG() const
{
	return fIdentifier;
}




/*
 *
 *
 *
 */
FunctionCallExpressionNode::FunctionCallExpressionNode( VString inIdent, Node *inLHS, FunctionCallArgumentsNode *inArgs, int inLineNo ):
MemberExpressionNode( inLineNo ),
fIdentifier( inIdent ),
fLHS( inLHS ),
fArgs( inArgs )
{
}

FunctionCallExpressionNode::~FunctionCallExpressionNode()
{
	delete fLHS;
	delete fArgs;
}

bool FunctionCallExpressionNode::Accept( class Visitor *visit )
{
	if (!visit->VisitFunctionCallExpressionNodeEnter( this ))
		return false;
	
	if (fLHS && !fLHS->Accept( visit ))
		return false;
	
	if (fArgs && !fArgs->Accept( visit ))
		return false;
	
	return visit->VisitFunctionCallExpressionNodeLeave( this );
}
	
const VString& FunctionCallExpressionNode::GetIdentifier() const
{
	return fIdentifier;
}


Node* FunctionCallExpressionNode::GetLHS() const
{
	return fLHS;
}

FunctionCallArgumentsNode* FunctionCallExpressionNode::GetArgs()
{
	return fArgs;
}

EASTNodeType FunctionCallExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeFunctionCallExpressionNode;
}

const XBOX::VString& FunctionCallExpressionNode::GetIdentifierNG() const
{
	return fIdentifier;
}




/*
 *
 *
 *
 */
NewNode::NewNode( Node *inRhs, Node *inArgs, int inLineNo ):
MemberExpressionNode( inLineNo ),
fRHS( inRhs ),
fArgs( inArgs )
{
}

NewNode::~NewNode()
{
	delete fRHS;
	delete fArgs;
}

bool NewNode::Accept( class Visitor *visit )
{
	if (!visit->VisitNewNodeEnter( this ))
		return false;
	
	if (fRHS && !fRHS->Accept( visit ))
		return false;
	
	if (fArgs && !fArgs->Accept( visit ))
		return false;
	
	return visit->VisitNewNodeLeave( this );
}

Node* NewNode::GetOperand() const
{
	return fRHS;
}

EASTNodeType NewNode::GetNodeTypeNG() const
{
	return Node::kTypeNewNode;
}




/*
 *
 *
 *
 */
ArrayExpressionNode::ArrayExpressionNode( Node *inLhs, Node *inRhs, int inLineNo ):
MemberExpressionNode( inLineNo ),
fLHS( inLhs ),
fRHS( inRhs )
{
}

ArrayExpressionNode::~ArrayExpressionNode()
{
	delete fLHS;
	delete fRHS;
}

bool ArrayExpressionNode::Accept( class Visitor *visit )
{
	if (!visit->VisitArrayExpressionNodeEnter( this ))
		return false;
	
	if (fLHS && !fLHS->Accept( visit ))
		return false;
	
	if (fRHS && !fRHS->Accept( visit ))
		return false;
	
	return visit->VisitArrayExpressionNodeLeave( this );
}

Node* ArrayExpressionNode::GetLHS() const
{
	return fLHS;
}
	
Node* ArrayExpressionNode::GetRHS() const
{
	return fRHS;
}

EASTNodeType ArrayExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeArrayExpressionNode;
}




/*
 *
 *
 *
 */
DotExpressionNode::DotExpressionNode( Node *inLhs, Node *inRhs, int inLineNo ):
MemberExpressionNode( inLineNo ),
fLHS( inLhs ),
fRHS( inRhs )
{
}

DotExpressionNode::~DotExpressionNode()
{
	delete fLHS;
	delete fRHS;
}

bool DotExpressionNode::Accept( class Visitor *visit )
{
	if (visit->VisitDotExpressionNodeEnter( this ))
	{
		if (fLHS && !fLHS->Accept( visit ))
			return false;
		
		if (fRHS && !fRHS->Accept( visit ))
			return false;
	}
	return visit->VisitDotExpressionNodeLeave( this );
}

Node* DotExpressionNode::GetLHS() const
{
	return fLHS;
}

Node* DotExpressionNode::GetRHS() const
{
	return fRHS;
}

EASTNodeType DotExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypeDotExpressionNode;
}




/*
 *
 *
 *
 */
PrimaryExpressionNode::PrimaryExpressionNode( int inLineNo ):
MemberExpressionNode( inLineNo )
{
}

EASTNodeType PrimaryExpressionNode::GetNodeTypeNG() const
{
	return Node::kTypePrimaryExpressionNode;
}




/*
 *
 *
 *
 */
ThisNode::ThisNode( int inLineNo ):
PrimaryExpressionNode( inLineNo )
{
}

bool ThisNode::Accept( class Visitor *visit )
{
	if( !visit->VisitThisNodeEnter( this ) )
		return false;
	
	return visit->VisitThisNodeLeave( this );
}

EASTNodeType ThisNode::GetNodeTypeNG() const
{
	return Node::kTypeThisNode;
}




/*
 *
 *
 *
 */
IdentifierNode::IdentifierNode( VString inIdent, int inLineNo ):
PrimaryExpressionNode( inLineNo ),
fIdentifier( inIdent )
{
}

bool IdentifierNode::Accept( class Visitor *visit )
{
	if( !visit->VisitIdentifierNodeEnter( this ) )
		return false;
	
	return visit->VisitIdentifierNodeLeave( this );
}

VString IdentifierNode::GetText() const
{
	return fIdentifier;
}

EASTNodeType IdentifierNode::GetNodeTypeNG() const
{
	return Node::kTypeIdentifierNode;
}

const XBOX::VString& IdentifierNode::GetIdentifierNG() const
{
	return fIdentifier;
}






/*
 *
 *
 *
 */
LiteralNode::LiteralNode( int inLineNo ):
PrimaryExpressionNode( inLineNo )
{
}

bool LiteralNode::IsLiteral() const
{
	return true;
}

EASTNodeType LiteralNode::GetNodeTypeNG() const
{
	return Node::kTypeLiteralNode;
}




/*
 *
 *
 *
 */
ArrayLiteralNode::ArrayLiteralNode( int inLineNo ):
LiteralNode( inLineNo )
{
}

bool ArrayLiteralNode::Accept( class Visitor *visit )
{
	if( !visit->VisitArrayLiteralNodeEnter( this ) )
		return false;
	
	return visit->VisitArrayLiteralNodeLeave( this );
}

EASTNodeType ArrayLiteralNode::GetNodeTypeNG() const
{
	return Node::kTypeArrayLiteralNode;
}




/*
 *
 *
 *
 */
NullLiteralNode::NullLiteralNode( int inLineNo ):
LiteralNode( inLineNo )
{
}

bool NullLiteralNode::Accept( class Visitor *visit )
{
	if( !visit->VisitNullLiteralNodeEnter( this ) )
		return false;
	
	return visit->VisitNullLiteralNodeLeave( this );
}

EASTNodeType NullLiteralNode::GetNodeTypeNG() const
{
	return Node::kTypeNullLiteralNode;
}




/*
 *
 *
 *
 */
StringLiteralNode::StringLiteralNode( VString inValue, int inLineNo ):
LiteralNode( inLineNo ),
fValue( inValue )
{
}

bool StringLiteralNode::Accept( class Visitor *visit )
{
	if( !visit->VisitStringLiteralNodeEnter( this ) )
		return false;
	
	return visit->VisitStringLiteralNodeLeave( this );
}

VString StringLiteralNode::GetValue() const
{
	return fValue;
}

EASTNodeType StringLiteralNode::GetNodeTypeNG() const
{
	return Node::kTypeStringLiteralNode;
}

const XBOX::VString& StringLiteralNode::GetIdentifierNG() const
{
	return fValue;
}




/*
 *
 *
 *
 */
NumericLiteralNode::NumericLiteralNode( const VString &inValue, int inLineNo ):
LiteralNode( inLineNo ),
fValue( inValue )
{
}

bool NumericLiteralNode::Accept( class Visitor *visit )
{
	if( !visit->VisitNumericLiteralNodeEnter( this ) )
		return false;
	
	return visit->VisitNumericLiteralNodeLeave( this );
}

VString NumericLiteralNode::GetValue() const
{
	return fValue;
}

EASTNodeType NumericLiteralNode::GetNodeTypeNG() const
{
	return Node::kTypeNumericLiteralNode;
}




/*
 *
 *
 *
 */
BooleanLiteralNode::BooleanLiteralNode( bool inValue, int inLineNo ):
LiteralNode( inLineNo ),
fValue( inValue )
{
}

bool BooleanLiteralNode::Accept( class Visitor *visit )
{
	if( !visit->VisitBooleanLiteralNodeEnter( this ) )
		return false;
	
	return visit->VisitBooleanLiteralNodeLeave( this );
}

bool BooleanLiteralNode::GetValue() const
{
	return fValue;
}

EASTNodeType BooleanLiteralNode::GetNodeTypeNG() const
{
	return Node::kTypeBooleanLiteralNode;
}




/*
 *
 *
 *
 */
ObjectLiteralFieldNode::ObjectLiteralFieldNode( Node *inIdent, Node *inValue, int inLineNo ):
Node( inLineNo ),
fIdent( inIdent ),
fValue( inValue )
{
}

ObjectLiteralFieldNode::~ObjectLiteralFieldNode()
{
	delete fIdent;
	delete fValue;
}

Node* ObjectLiteralFieldNode::GetIdentifier()
{
	return fIdent;
}

Node* ObjectLiteralFieldNode::GetValue()
{
	return fValue;
}

bool ObjectLiteralFieldNode::Accept( class Visitor *visit )
{
	if (visit->VisitObjectLiteralFieldNodeEnter( this ))
	{
		bool keepGoing = true;
		
		if (keepGoing && fIdent)
			keepGoing = fIdent->Accept( visit );
		
		if (keepGoing && fValue)
			keepGoing = fValue->Accept( visit );
	}
	return visit->VisitObjectLiteralFieldNodeLeave( this );
}

EASTNodeType ObjectLiteralFieldNode::GetNodeTypeNG() const
{
	return Node::kTypeObjectLiteralFieldNode;
}




/*
 *
 *
 *
 */
ObjectLiteralNode::ObjectLiteralNode( int inLineNo ):
LiteralNode( inLineNo )
{
}

ObjectLiteralNode::~ObjectLiteralNode()
{
	while (!mFields.empty())
	{
		delete mFields.back();
		mFields.pop_back();
	}
}

void ObjectLiteralNode::AddField( ObjectLiteralFieldNode *inField )
{
	mFields.push_back( inField );
}

bool ObjectLiteralNode::Accept( class Visitor *visit )
{
	if (visit->VisitObjectLiteralNodeEnter( this ))
	{
		for (std::vector< ObjectLiteralFieldNode * >::iterator iter = mFields.begin(); iter != mFields.end(); ++iter)
		{
			if (!(*iter)->Accept( visit ))
				break;
		}
	}
	return visit->VisitObjectLiteralNodeLeave( this );
}

EASTNodeType ObjectLiteralNode::GetNodeTypeNG() const
{
	return Node::kTypeObjectLiteralNode;
}




/*
 *
 *
 *
 */
RegExLiteralNode::RegExLiteralNode( VString inBody, VString inFlags, int inLineNo ):
LiteralNode( inLineNo ),
fBody( inBody ),
mFlags( inFlags )
{
}

bool RegExLiteralNode::Accept( class Visitor *visit )
{
	if( !visit->VisitRegExLiteralNodeEnter( this ) )
		return false;
	
	return visit->VisitRegExLiteralNodeLeave( this );
}

VString RegExLiteralNode::GetBody() const
{
	return fBody;
}

VString RegExLiteralNode::GetFlags() const
{
	return mFlags;
}

EASTNodeType RegExLiteralNode::GetNodeTypeNG() const
{
	return Node::kTypeRegExLiteralNode;
}




/*
 *
 *
 *
 */
StatementNode::StatementNode( int inLineNo ):
Node( inLineNo )
{
}

bool StatementNode::Accept( class Visitor *visit )
{
	if( !visit->VisitStatementNodeEnter( this ) )
		return false;
	
	return visit->VisitStatementNodeLeave( this );
}

EASTNodeType StatementNode::GetNodeTypeNG() const
{
	return Node::kTypeStatementNode;
}




/*
 *
 *
 *
 */
EmptyStatementNode::EmptyStatementNode( int inLineNo ):
StatementNode( inLineNo )
{
}

bool EmptyStatementNode::Accept( class Visitor *visit )
{
	if( !visit->VisitEmptyStatementNodeEnter( this ) )
		return false;
	
	return  visit->VisitEmptyStatementNodeLeave( this );
}

EASTNodeType EmptyStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeEmptyStatementNode;
}




/*
 *
 *
 *
 */
LabeledStatementNode::LabeledStatementNode( VString inIdent, Node *inStatement, int inLineNo ):
StatementNode( inLineNo ),
fIdent( inIdent ),
fStatement( inStatement )
{
}

LabeledStatementNode::~LabeledStatementNode()
{
	delete fStatement;
}

bool LabeledStatementNode::Accept( class Visitor *visit )
{
	if (!visit->VisitLabeledStatementNodeEnter( this ))
		return false;
	
	if (fStatement && !fStatement->Accept( visit ))
		return false;
	
	return visit->VisitLabeledStatementNodeLeave( this );
}

EASTNodeType LabeledStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeLabeledStatementNode;
}




/*
 *
 *
 *
 */
FunctionDeclarationStatementNode::FunctionDeclarationStatementNode( VString inIdent, FunctionDeclarationArgumentsNode *inArgs, Node *inStatements, int inBeginLineNo, int inEndLineNo ):
StatementNode( inBeginLineNo ),
fIdent( inIdent ),
fArgs( inArgs ),
fStatements( inStatements )
{
	fLineCompletion = inEndLineNo;
}

FunctionDeclarationStatementNode::~FunctionDeclarationStatementNode()
{
	delete fArgs;
	delete fStatements;
}

bool FunctionDeclarationStatementNode::Accept( class Visitor *visit )
{
	if (visit->VisitFunctionDeclarationStatementNodeEnter( this ))
	{
		bool keepGoing = true;
		if (keepGoing && fArgs)
			keepGoing = fArgs->Accept( visit );
		
		if (keepGoing && fStatements)
			keepGoing = fStatements->Accept( visit );
	}
	return visit->VisitFunctionDeclarationStatementNodeLeave( this );
}

const VString& FunctionDeclarationStatementNode::GetIdentifier() const
{
	return fIdent;
}

FunctionDeclarationArgumentsNode* FunctionDeclarationStatementNode::GetArgs() const
{
	return fArgs;
}

Node* FunctionDeclarationStatementNode::GetStatements() const
{
	return fStatements;
}

EASTNodeType FunctionDeclarationStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeFunctionDeclarationStatementNode;
}

const XBOX::VString& FunctionDeclarationStatementNode::GetIdentifierNG() const
{
	return fIdent;
}




/*
 *
 *
 *
 */
IfStatementNode::IfStatementNode( Node *inExpr, Node *inTrueStatements, Node *inFalseStatements, int inLineNo ):
StatementNode( inLineNo ),
fExpr( inExpr ),
mTrueStatements( inTrueStatements ),
mFalseStatements( inFalseStatements )
{
}

IfStatementNode::~IfStatementNode()
{
	delete fExpr;
	delete mTrueStatements;
	delete mFalseStatements;
}

bool IfStatementNode::Accept( class Visitor *visit )
{
	if (!visit->VisitIfStatementNodeEnter( this ))
		return false;
	
	if (fExpr && !fExpr->Accept( visit ))
		return false;
	
	if (mTrueStatements && !mTrueStatements->Accept( visit ))
		return false;
	
	if (mFalseStatements && !mFalseStatements->Accept( visit ))
		return false;
	
	return visit->VisitIfStatementNodeLeave( this );
}

EASTNodeType IfStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeIfStatementNode;
}




/*
 *
 *
 *
 */
ContinueStatementNode::ContinueStatementNode( Node *inIdent, int inLineNo ):
StatementNode( inLineNo ),
fIdent( inIdent )
{
}

ContinueStatementNode::~ContinueStatementNode()
{
	delete fIdent;
}

bool ContinueStatementNode::Accept( class Visitor *visit )
{
	if (!visit->VisitContinueStatementNodeEnter( this ))
		return false;
	
	if (fIdent && !fIdent->Accept( visit ))
		return false;
	
	return visit->VisitContinueStatementNodeLeave( this );
}

EASTNodeType ContinueStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeContinueStatementNode;
}




/*
 *
 *
 *
 */
BreakStatementNode::BreakStatementNode( Node *inIdent, int inLineNo ):
StatementNode( inLineNo ),
fIdent( inIdent )
{
}

BreakStatementNode::~BreakStatementNode()
{
	delete fIdent;
}

bool BreakStatementNode::Accept( class Visitor *visit )
{
	if (!visit->VisitBreakStatementNodeEnter( this ))
		return false;
	
	if (fIdent && !fIdent->Accept( visit ))
		return false;
	
	return visit->VisitBreakStatementNodeLeave( this );
}

EASTNodeType BreakStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeBreakStatementNode;
}




/*
 *
 *
 *
 */
DebuggerStatementNode::DebuggerStatementNode( int inLineNo ):
StatementNode( inLineNo )
{
}

DebuggerStatementNode::~DebuggerStatementNode()
{
}

bool DebuggerStatementNode::Accept( class Visitor *visit )
{
	if (!visit->VisitDebuggerStatementNodeEnter( this ))
		return false;
	
	return visit->VisitDebuggerStatementNodeLeave( this );
}

EASTNodeType DebuggerStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeDebuggerStatementNode;
}




/*
 *
 *
 *
 */
ReturnStatementNode::ReturnStatementNode( Node *inExpr, int inLineNo ):
StatementNode( inLineNo ),
fExpr( inExpr )
{
}

ReturnStatementNode::~ReturnStatementNode()
{
	delete fExpr;
}

bool ReturnStatementNode::Accept( class Visitor *visit )
{
	if (!visit->VisitReturnStatementNodeEnter( this ))
		return false;
	
	if (fExpr && !fExpr->Accept( visit ))
		return false;
	
	return visit->VisitReturnStatementNodeLeave( this );
}

Node* ReturnStatementNode::GetExpression() const
{
	return fExpr;
}

EASTNodeType ReturnStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeReturnStatementNode;
}





/*
 *
 *
 *
 */
WithStatementNode::WithStatementNode( Node *inExpr, Node *inStatement, int inLineNo ):
StatementNode( inLineNo ),
fExpr( inExpr ),
fStatement( inStatement )
{
}

WithStatementNode::~WithStatementNode()
{
	delete fExpr;
	delete fStatement;
}

bool WithStatementNode::Accept( class Visitor *visit )
{
	if (!visit->VisitWithStatementNodeEnter( this ))
		return false;
	
	if (fExpr && !fExpr->Accept( visit ))
		return false;
	
	if (fStatement && !fStatement->Accept( visit ))
		return false;
	
	return visit->VisitWithStatementNodeLeave( this );
}

EASTNodeType WithStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeWithStatementNode;
}




/*
 *
 *
 *
 */
ThrowStatementNode::ThrowStatementNode( Node *inExpr, int inLineNo ):
StatementNode( inLineNo ),
fExpr( inExpr )
{
}

ThrowStatementNode::~ThrowStatementNode()
{
	delete fExpr;
}

bool ThrowStatementNode::Accept( class Visitor *visit )
{
	if (!visit->VisitThrowStatementNodeEnter( this ))
		return false;
	
	if (fExpr && !fExpr->Accept( visit ))
		return false;
	
	return visit->VisitThrowStatementNodeLeave( this );
}

EASTNodeType ThrowStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeThrowStatementNode;
}




/*
 *
 *
 *
 */
CatchStatementNode::CatchStatementNode( Node *inIdent, Node *inStatements, int inLineNo ):
Node( inLineNo ),
fIdent( inIdent ),
fStatements( inStatements )
{
	fLineCompletion = inStatements ? inStatements->GetLineCompletion() : -1;
}

CatchStatementNode::~CatchStatementNode()
{
	delete fIdent;
	delete fStatements;
}

bool CatchStatementNode::Accept( class Visitor *visit )
{
	if (visit->VisitCatchStatementNodeEnter( this ))
	{
		bool keepGoing = true;
		if (keepGoing && fIdent)
			keepGoing = fIdent->Accept( visit );
		
		if (keepGoing && fStatements)
			keepGoing = fStatements->Accept( visit );
	}
	return visit->VisitCatchStatementNodeLeave( this );
}

Node* CatchStatementNode::GetStatements()
{
	return fStatements;
}

Node* CatchStatementNode::GetIdentifier()
{
	return fIdent;
}

EASTNodeType CatchStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeCatchStatementNode;
}




/*
 *
 *
 *
 */
TryStatementNode::TryStatementNode( Node *inStatements, Node *inCatch, Node *inFinally, int inLineNo ):
StatementNode( inLineNo ),
fStatements( inStatements ),
mCatch( inCatch ),
mFinally( inFinally )
{
}

TryStatementNode::~TryStatementNode()
{
	delete fStatements;
	delete mCatch;
	delete mFinally;
}

bool TryStatementNode::Accept( class Visitor *visit )
{
	if (!visit->VisitTryStatementNodeEnter( this ))
		return false;

	if (fStatements && !fStatements->Accept( visit ))
		return false;
	
	if (mCatch && !mCatch->Accept( visit ))
		return false;
	
	if (mFinally && !mFinally->Accept( visit ))
		return false;
	
	return visit->VisitTryStatementNodeLeave( this );
}

EASTNodeType TryStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeTryStatementNode;
}




/*
 *
 *
 *
 */
SwitchStatementNode::SwitchStatementNode( Node *inExpr, int inLineNo ):
StatementNode( inLineNo ),
fExpr( inExpr )
{
}

SwitchStatementNode::~SwitchStatementNode()
{
	delete fExpr;
	while (!mCases.empty())
	{
		delete mCases.back();
		mCases.pop_back();
	}
}

void SwitchStatementNode::AddCase( Node *inCase )
{
	mCases.push_back( inCase );
}

bool SwitchStatementNode::Accept( class Visitor *visit )
{
	if (!visit->VisitSwitchStatementNodeEnter( this ))
		return false;

	if (fExpr && !fExpr->Accept( visit ))
		return false;
	
	for (std::vector< Node * >::iterator iter = mCases.begin(); iter != mCases.end(); ++iter)
	{
		if (!(*iter)->Accept( visit ))
			return false;
	}
	
	return visit->VisitSwitchStatementNodeLeave( this );
}

EASTNodeType SwitchStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeSwitchStatementNode;
}




/*
 *
 *
 *
 */
CaseClauseNode::CaseClauseNode( Node *inExpr, Node *inStatement, int inLineNo ):
Node( inLineNo ),
fExpr( inExpr ),
fStatement( inStatement )
{
}

CaseClauseNode::~CaseClauseNode()
{
	delete fExpr;
	delete fStatement;
}

bool CaseClauseNode::Accept( class Visitor *visit )
{
	if (!visit->VisitCaseClauseNodeEnter( this ))
		return false;
	
	if (fExpr && !fExpr->Accept( visit ))
		return false;
	
	if (fStatement && !fStatement->Accept( visit ))
		return false;
	
	return visit->VisitCaseClauseNodeLeave( this );
}

EASTNodeType CaseClauseNode::GetNodeTypeNG() const
{
	return Node::kTypeCaseClauseNode;
}




/*
 *
 *
 *
 */
DefaultClauseNode::DefaultClauseNode( Node *inStatement, int inLineNo ):
Node( inLineNo ),
fStatement( inStatement )
{
}

DefaultClauseNode::~DefaultClauseNode()
{
	delete fStatement;
}

bool DefaultClauseNode::Accept( class Visitor *visit )
{
	if (!visit->VisitDefaultClauseNodeEnter( this ))
		return false;
	
	if (fStatement && !fStatement->Accept( visit ))
		return false;
	
	return visit->VisitDefaultClauseNodeLeave( this );
}

EASTNodeType DefaultClauseNode::GetNodeTypeNG() const
{
	return Node::kTypeDefaultClauseNode;
}




/*
 *
 *
 *
 */
VariableDeclarationStatementNode::VariableDeclarationStatementNode( Node *inIdent, Node *inAssignmentExpr, int inLineNo ):
StatementNode( inLineNo ),
fIdent( inIdent ),
mAssignment( inAssignmentExpr )
{
}

VariableDeclarationStatementNode::~VariableDeclarationStatementNode()
{
	delete fIdent;
	delete mAssignment;
}

bool VariableDeclarationStatementNode::Accept( class Visitor *visit )
{
	if (visit->VisitVariableDeclarationStatementNodeEnter( this ))
	{
		bool keepGoing = true;
		if (keepGoing && fIdent)
			keepGoing = fIdent->Accept( visit );
		
		if (keepGoing && mAssignment)
			keepGoing = mAssignment->Accept( visit );
	}
	return visit->VisitVariableDeclarationStatementNodeLeave( this );
}

Node* VariableDeclarationStatementNode::GetIdentifier()
{
	return fIdent;
}

Node* VariableDeclarationStatementNode::GetAssignment()
{
	return mAssignment;
}

EASTNodeType VariableDeclarationStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeVariableDeclarationStatementNode;
}

const XBOX::VString& VariableDeclarationStatementNode::GetIdentifierNG() const
{
	return fIdent->GetIdentifierNG();
}




/*
 *
 *
 *
 */
VariableDeclarationListStatementNode::VariableDeclarationListStatementNode( VariableDeclarationStatementNode *inFirst, int inLineNo ):
StatementNode( inLineNo )
{
	AddDeclaration( inFirst );
}

VariableDeclarationListStatementNode::~VariableDeclarationListStatementNode()
{
	while (!fDecls.empty())
	{
		delete fDecls.back();
		fDecls.pop_back();
	}
}

void VariableDeclarationListStatementNode::AddDeclaration( VariableDeclarationStatementNode *inDecl )
{
	fDecls.push_back( inDecl );
}

bool VariableDeclarationListStatementNode::Accept( class Visitor *visit )
{
	if (visit->VisitVariableDeclarationListStatementNodeEnter( this ))
	{
		for (std::vector< VariableDeclarationStatementNode * >::iterator iter = fDecls.begin(); iter != fDecls.end(); ++iter)
		{
			if (!(*iter)->Accept( visit ))
				break;
		}
	}
	
	return visit->VisitVariableDeclarationListStatementNodeLeave( this );
}

EASTNodeType VariableDeclarationListStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeVariableDeclarationListStatementNode;
}




/*
 *
 *
 *
 */
DoStatementNode::DoStatementNode( Node *inExpr, Node *inStatement, int inLineNo ):
StatementNode( inLineNo ),
fExpr( inExpr ),
fStatement( inStatement )
{
}

DoStatementNode::~DoStatementNode()
{
	delete fExpr;
	delete fStatement;
}

bool DoStatementNode::Accept( class Visitor *visit )
{
	if (!visit->VisitDoStatementNodeEnter( this ))
		return false;
	
	if (fExpr && !fExpr->Accept( visit ))
		return false;
	
	if (fStatement && !fStatement->Accept( visit ))
		return false;

	return visit->VisitDoStatementNodeLeave( this );
}

EASTNodeType DoStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeDoStatementNode;
}




/*
 *
 *
 *
 */
WhileStatementNode::WhileStatementNode( Node *inExpr, Node *inStatement, int inLineNo ):
StatementNode( inLineNo ),
fExpr( inExpr ),
fStatement( inStatement )
{
}

WhileStatementNode::~WhileStatementNode()
{
	delete fExpr;
	delete fStatement;
}

bool WhileStatementNode::Accept( class Visitor *visit )
{
	if (!visit->VisitWhileStatementNodeEnter( this ))
		return false;
	
	if (fExpr && !fExpr->Accept( visit ))
		return false;
	
	if (fStatement && !fStatement->Accept( visit ))
		return false;
	
	return visit->VisitWhileStatementNodeLeave( this );
}

EASTNodeType WhileStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeWhileStatementNode;
}




/*
 *
 *
 *
 */
ForStatementNode::ForStatementNode( Node *inExpr, Node *inStatement, int inLineNo ):
StatementNode( inLineNo ),
fExpr( inExpr ),
fStatement( inStatement )
{
}

ForStatementNode::~ForStatementNode()
{
	delete fExpr;
	delete fStatement;
}

bool ForStatementNode::Accept( class Visitor *visit )
{
	if (!visit->VisitForStatementNodeEnter( this ))
		return false;
	
	if (fExpr && !fExpr->Accept( visit ))
		return false;
	
	if (fStatement && !fStatement->Accept( visit ))
		return false;
	
	return visit->VisitForStatementNodeLeave( this );
}

EASTNodeType ForStatementNode::GetNodeTypeNG() const
{
	return Node::kTypeForStatementNode;
}




/*
 *
 *
 *
 */
ForExpressionTriClauseNode::ForExpressionTriClauseNode( Node *inDecl, Node *inTest, Node *inLoop, int inLineNo ):
Node( inLineNo ),
fDecl( inDecl ),
fTest( inTest ),
fLoop( inLoop )
{
}

ForExpressionTriClauseNode::~ForExpressionTriClauseNode()
{
	delete fDecl;
	delete fTest;
	delete fLoop;
}

bool ForExpressionTriClauseNode::Accept( class Visitor *visit )
{
	if (!visit->VisitForExpressionTriClauseNodeEnter( this ))
		return false;
	
	if (fDecl && !fDecl->Accept( visit ))
		return false;
	
	if (fTest && !fTest->Accept( visit ))
		return false;
	
	if (fLoop && !fLoop->Accept( visit ))
		return false;
	
	return visit->VisitForExpressionTriClauseNodeLeave( this );
}

EASTNodeType ForExpressionTriClauseNode::GetNodeTypeNG() const
{
	return Node::kTypeForExpressionTriClauseNode;
}




/*
 *
 *
 *
 */
ForExpressionInClauseNode::ForExpressionInClauseNode( Node *inDecl, Node *inExpr, int inLineNo ):
Node( inLineNo ),
fDecl( inDecl ),
fExpr( inExpr )
{
}
	
ForExpressionInClauseNode::~ForExpressionInClauseNode()
{
	delete fDecl;
	delete fExpr;
}

bool ForExpressionInClauseNode::Accept( class Visitor *visit )
{
	if (!visit->VisitForExpressionInClauseNodeEnter( this ))
		return false;
	
	if (fDecl && !fDecl->Accept( visit ))
		return false;
	
	if (fExpr && !fExpr->Accept( visit ))
		return false;
	
	return visit->VisitForExpressionInClauseNodeLeave( this );
}

Node* ForExpressionInClauseNode::GetForDeclaration() const
{
	return fDecl;
}

EASTNodeType ForExpressionInClauseNode::GetNodeTypeNG() const
{
	return Node::kTypeForExpressionInClauseNode;
}




/*
 *
 *
 *
 */
StatementList::StatementList( int inLineNo ):
StatementNode( inLineNo )
{
}

void StatementList::AddStatement( Node *inStatement )
{
	fStatementList.push_back( inStatement );
	fLineCompletion = XBOX::Max( fLineCompletion, inStatement->GetLineNumber() );
}

void StatementList::SetListCompletionLine( int inLineNo )
{
	fLineCompletion = XBOX::Max( fLineCompletion, inLineNo );
}

StatementList::~StatementList()
{
	while (!fStatementList.empty())
	{
		delete fStatementList.back();
		fStatementList.pop_back();
	}
}

bool StatementList::Accept( class Visitor *visit )
{
	if (!visit->VisitStatementListNodeEnter( this ))
		return false;
	
	for (std::vector< Node * >::iterator iter = fStatementList.begin(); iter != fStatementList.end(); ++iter)
	{
		(*iter)->Accept( visit );
		if (VTask::GetCurrent()->IsDying())
			return false;
	}
	
	return visit->VisitStatementListNodeLeave( this );
}

EASTNodeType StatementList::GetNodeTypeNG() const
{
	return Node::kTypeStatementListNode;
}



































































/*
 *
 *
 *
 */
class DeclarationVisitor : public Visitor
{
public:
	DeclarationVisitor( ISymbolTable *inSymTable, Symbols::IFile *inFile, JavaScriptParserDelegate *inDelegate );
	virtual ~DeclarationVisitor();
	
private:
	JavaScriptParserDelegate*									fDelegate;
	std::vector<Symbols::ISymbol*>								fOwnershipStack;
	ISymbolTable*												fSymTable;
	Symbols::IFile*												fOwningFile;
	std::vector<Symbols::ISymbol*>								fSymbolsToAdd;
	std::vector<Symbols::ISymbol*>								fSymbolsToAddToStatement;
	std::vector<Symbols::ISymbol*>								fSymbolsToUpdate;
	std::multimap<Symbols::ISymbol*, Symbols::IExtraInfo*>		fExtraInfoToAdd;
	
	virtual bool VisitProgramNodeLeave						( class ProgramNode *inNode );
	
	virtual bool VisitFunctionExpressionNodeEnter			( class FunctionExpressionNode *inNode );
	virtual bool VisitFunctionExpressionNodeLeave			( class FunctionExpressionNode *inNode );
	
	virtual bool VisitFunctionDeclarationStatementNodeEnter	( class FunctionDeclarationStatementNode *inNode );
	virtual bool VisitFunctionDeclarationStatementNodeLeave	( class FunctionDeclarationStatementNode *inNode );
	
	virtual bool VisitVariableDeclarationStatementNodeEnter	( class VariableDeclarationStatementNode *inNode );

	virtual bool VisitFunctionDeclarationArgumentsNodeEnter	( class FunctionDeclarationArgumentsNode *inNode );
	
	virtual bool VisitAssignExpressionNodeEnter				( class AssignExpressionNode *inNode );
	
	virtual bool VisitObjectLiteralNodeEnter				( class ObjectLiteralNode *inNode );
	virtual bool VisitObjectLiteralNodeLeave				( class ObjectLiteralNode *inNode );

	virtual bool VisitObjectLiteralFieldNodeEnter			( class ObjectLiteralFieldNode *inNode );
	
	virtual bool VisitForExpressionInClauseNodeEnter		( class ForExpressionInClauseNode *inNode );
	
	virtual bool VisitReturnStatementNodeEnter				( class ReturnStatementNode *inNode );
	
	virtual bool VisitCatchStatementNodeEnter				( class CatchStatementNode *inNode );
	virtual bool VisitCatchStatementNodeLeave				( class CatchStatementNode *inNode );
	
	virtual bool VisitBothFunctionExpressionAndDeclarationNode(Node* inNode);
	
	// Note that GetSubSymbols retains the data in the vector, so it is up to the caller to release the
	// vector's contents when they are done.
	void GetSubSymbols( Symbols::ISymbol *inOwner, std::vector< Symbols::ISymbol * > &outSubSymbols );
	void RetainLikeNamedSubSymbols( Symbols::ISymbol *inOwner, const VString &inText, std::vector< Symbols::ISymbol * > &outSubSymbols );
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
	
	void SetSymbolReturnTypesFromScriptDoc	(Symbols::ISymbol* inSym,  bool inAddToUpdateList = true, bool inUpdateSymbol = false);
	void SetSymbolClassKindFromScriptDoc	(Symbols::ISymbol* inSym);
	
	void SetSymbolClassesFromScriptDoc		(Symbols::ISymbol* inSym,  bool inAddToUpdateList = true, bool inUpdateSymbol = false);
	void SetSymbolTypesFromScriptDoc		(Symbols::ISymbol* inSym, bool inAddToUpdateList = true, bool inUpdateSymbol = false);
	// Use only in the two followings methods
	void SetSymbolPrototypes				(Symbols::ISymbol* inSym, const VectorOfVString& inTypes, bool inAddToUpdateList, bool inUpdateSymbol);
	
	Symbols::ISymbol* RetainSymbolPrototype(Symbols::ISymbol* inOwner, const VString& inSymbolName, const ESymbolFileExecContext& inExecContext);
};



/*
 *
 *
 *
 */
DeclarationVisitor::DeclarationVisitor( ISymbolTable *inSymTable, Symbols::IFile *inFile, JavaScriptParserDelegate *inDelegate ) :
fSymTable( inSymTable ), fOwningFile( inFile ), fDelegate( inDelegate )
{
	// The global symbol table is always the root of our ownership stack
	fOwnershipStack.push_back( NULL );
	
	if (fDelegate)
	{
		// If we have a delegate, ask them for the context symbol for adding new declarations.
		// We will add this to the ownership list after the global symbol table.
		Symbols::ISymbol *contextSymbol = fDelegate->GetDeclarationContext( );
		if (contextSymbol)
			fOwnershipStack.push_back( contextSymbol );
	}
}



/*
 *
 *
 *
 */
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



/*
 *
 *
 *
 */
void DeclarationVisitor::GetSubSymbols( Symbols::ISymbol *inOwner, std::vector< Symbols::ISymbol * > &outSubSymbols )
{
	// First, search the database for any entries the owner contains
	if (!inOwner || (inOwner && inOwner->HasID()))
	{
		outSubSymbols = fSymTable->RetainNamedSubSymbols( inOwner );
	}
	
	// Then add on any entries we can locate from our set of symbols to add
	for (std::vector< Symbols::ISymbol * >::iterator iter = fSymbolsToAdd.begin(); iter != fSymbolsToAdd.end(); ++iter)
	{
		if ((*iter)->GetOwner() == inOwner)
		{
			(*iter)->Retain();
			outSubSymbols.push_back( *iter );
		}
	}
}



/*
 *
 *
 *
 */
void DeclarationVisitor::RetainLikeNamedSubSymbols( Symbols::ISymbol *inOwner, const VString &inText, std::vector< Symbols::ISymbol * > &outSubSymbols )
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
        //Symbols::IFile* pOwnerFile = (inText == CVSTR("exports") ) ? this->fOwningFile : NULL;
		Symbols::IFile* pOwnerFile = NULL;
		std::vector< Symbols::ISymbol * > syms = fSymTable->RetainSymbolsByName( inOwner, inText, pOwnerFile );
		for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter)
			outSubSymbols.push_back( *iter );
	}
}



/*
 *
 *
 *
 */
Symbols::ISymbol* DeclarationVisitor::GetLikeNamedSubSymbol( Symbols::ISymbol *inOwner, const VString &inText )
{
	Symbols::ISymbol* ret = NULL;
	std::vector< Symbols::ISymbol * > subSymbols;
	
	RetainLikeNamedSubSymbols(inOwner, inText, subSymbols);
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



/*
 *
 *
 *
 */
bool DeclarationVisitor::VisitReturnStatementNodeEnter( ReturnStatementNode *inNode )
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



/*
 *
 *
 *
 */
bool DeclarationVisitor::VisitProgramNodeLeave( class ProgramNode *inNode )
{
	// If the current thread is dying, we don't want to add any symbols to the table
	// because that's an incredibly expensive operation.  Instead, we will bail out and
	// tell the caller the process failed so that the thread can die quicker.
	if (VTask::GetCurrent()->IsDying())	return false;
	
	// Now that we're done processing this program for declarations, we can actually update the
	// symbol table!
	bool ret = false;
	if (fSymTable)
	{
		ret = fSymTable->AddSymbols( fSymbolsToAdd );
		
		// Having added all of the symbols to the symbol table, we can add any extra information to it as well
		for (std::multimap< Symbols::ISymbol *, Symbols::IExtraInfo * >::iterator iter = fExtraInfoToAdd.begin(); ret && iter != fExtraInfoToAdd.end(); ++iter)
		{
			// It is possible we need to do some further processing of the extra information before we add it
			// to the symbol table.  This gives us the chance to do things like resolve symbol pointers into
			// symbol IDs.
			Symbols::IExtraInfo *info = iter->second;
			if (info->GetKind() == Symbols::IExtraInfo::kKindPropertyAssignedFunctionExpression)
			{
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



/*
 *
 *
 *
 */
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
	if (assignment)
	{
		return VisitRightHandSideOfAssignment( sym, assignment );
	}
	
	return true;
}



/*
 *
 *
 *
 */
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
	if (ident)
	{
		// The identifier will be treated as a "parameter" to our catch block function
		Symbols::ISymbol *arg = new VSymbol();
		arg->SetID( fSymTable->GetNextUniqueID() );
		arg->SetName( static_cast< IdentifierNode * >( ident )->GetText() );
		arg->SetFile( fOwningFile );
		arg->SetLineNumber( inNode->GetLineNumber() );
		arg->SetKind( Symbols::ISymbol::kKindLocalVariableDeclaration );
		arg->SetOwner( sym );
		
		CreateNewInstance( arg, GetLikeNamedSubSymbol( NULL, CVSTR("Error") ) );
		fSymbolsToAdd.push_back( arg );
	}
	fOwnershipStack.push_back( sym );
	return true;
}



/*
 *
 *
 *
 */
bool DeclarationVisitor::VisitCatchStatementNodeLeave( class CatchStatementNode *inNode )
{
	Symbols::ISymbol *object = fOwnershipStack.back();
	fOwnershipStack.pop_back();
	
	object->SetOwner( fOwnershipStack.back() );
	return true;
}



/*
 *
 *
 *
 */
bool DeclarationVisitor::VisitFunctionExpressionNodeEnter( class FunctionExpressionNode *inNode )
{
	return VisitBothFunctionExpressionAndDeclarationNode(inNode);
}



/*
 *
 *
 *
 */
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



/*
 *
 *
 *
 */
bool DeclarationVisitor::VisitBothFunctionExpressionAndDeclarationNode(Node* inNode)
{
	// Create the function symbol
	VSymbol *sym = new VSymbol();
	sym->SetID( fSymTable->GetNextUniqueID() );
	sym->SetFile( fOwningFile );
	sym->SetName( inNode->GetIdentifierNG() );
	sym->SetLineNumber( inNode->GetLineNumber() );
	sym->SetLineCompletionNumber( inNode->GetLineCompletion() );
	sym->SetKind( Symbols::ISymbol::kKindFunctionDeclaration );
	
	CreateNewInstance( sym, GetLikeNamedSubSymbol( NULL, CVSTR("Function") ) );
	
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
	{
		fSymbolsToAdd.push_back( constructor );
	}
	
	fSymbolsToAdd.push_back( prototype );
	
	return true;
}

bool DeclarationVisitor::VisitFunctionDeclarationStatementNodeEnter( class FunctionDeclarationStatementNode *inNode )
{
	return VisitBothFunctionExpressionAndDeclarationNode(inNode);
}



/*
 *
 *
 *
 */
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



/*
 *
 *
 *
 */
bool DeclarationVisitor::VisitFunctionDeclarationArgumentsNodeEnter( class FunctionDeclarationArgumentsNode *inNode )
{
	Symbols::ISymbol *owner = fOwnershipStack.back();
	
	ScriptDocComment *sdocComment = NULL;
	if (owner && !owner->GetScriptDocComment().IsEmpty())
	{
		sdocComment = ScriptDocComment::Create( owner->GetScriptDocComment() );
	}
	
	for (size_t i = 0; i < inNode->ArgumentCount(); i++)
	{
		Node *arg = inNode->GetArgument( i );
		
		IdentifierNode *identArg = dynamic_cast< IdentifierNode * >( arg );
		if (identArg)
		{
			VSymbol *sym = new VSymbol();
			sym->SetID( fSymTable->GetNextUniqueID() );
			sym->SetFile( fOwningFile );
			sym->SetName( identArg->GetText() );
			sym->SetLineNumber( identArg->GetLineNumber() );
			sym->SetKind( Symbols::ISymbol::kKindFunctionParameterDeclaration );
			sym->SetInstanceState(true);
			sym->SetOwner( owner );
			
			if (sdocComment)
			{
				// If we have a ScriptDoc comment for the owner, then we will see if
				// it mentions anything about this argument.  If it does, perhaps it
				// will tell us what type the target expects.
				ScriptDocLexer::Element *element = sdocComment->GetTargetElement( identArg->GetText() );
				if (element && element->Type() == IScriptDocCommentField::kParam)
				{
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



/*
 *
 *
 *
 */
bool DeclarationVisitor::VisitForExpressionInClauseNodeEnter( class ForExpressionInClauseNode *inNode )
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
	IdentifierNode *ident = dynamic_cast< IdentifierNode * >( inNode->GetForDeclaration() );
	if (ident)
	{
		// Look to see whether we have this variable declared within the scope already.  In that case, we
		// don't want to make a new global symbol for it.
		Symbols::ISymbol *foundSym = NULL;
		if (!UnqualifiedLookup( ident->GetText(), &foundSym ))
		{
			// We have a simple identifier instead of a variable declaration statement, so let's add this identifier
			// to the global symbol table
			VSymbol *sym = new VSymbol();
			sym->SetID( fSymTable->GetNextUniqueID() );
			sym->SetFile( fOwningFile );
			sym->SetName( ident->GetText() );
			sym->SetLineNumber( ident->GetLineNumber() );
			sym->SetKind( Symbols::ISymbol::kKindLocalVariableDeclaration );
			sym->SetInstanceState(true);
			sym->SetOwner( NULL );		// This is a global variable because it's not been declared with 'var'
			
			fSymbolsToAdd.push_back( sym );
		}
		else
		{
			// We did find a symbol already
			foundSym->Release();
		}
	}
	
	return true;
}




/*
 *
 *
 *
 */
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
	CreateNewInstance( sym, GetLikeNamedSubSymbol( NULL, CVSTR("Object") ) );
	
	
	// Push it onto our symbol owner stack
	fOwnershipStack.push_back( sym );
	
	fSymbolsToAdd.push_back( sym );
	
	return true;
}



/*
 *
 *
 *
 */
bool DeclarationVisitor::VisitObjectLiteralFieldNodeEnter( class ObjectLiteralFieldNode *inNode )
{
	// Get the identifier for this field
	VString identStr;
	if (dynamic_cast< IdentifierNode * >( inNode->GetIdentifier() ))
	{
		identStr = dynamic_cast< IdentifierNode * >( inNode->GetIdentifier() )->GetText();
	}
	else if (dynamic_cast< NumericLiteralNode * >( inNode->GetIdentifier() ))
	{
		identStr = dynamic_cast< NumericLiteralNode * >( inNode->GetIdentifier() )->GetValue();
	}
	else if (dynamic_cast< StringLiteralNode * >( inNode->GetIdentifier() ))
	{
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



/*
 *
 *
 *
 */
bool DeclarationVisitor::VisitObjectLiteralNodeLeave( ObjectLiteralNode *inNode )
{
	// Remove our symbol from the ownership stack
	Symbols::ISymbol *object = fOwnershipStack.back();
	fOwnershipStack.pop_back();
	
	// And add it to the parent
	object->SetOwner( fOwnershipStack.back() );
	
	return true;
}



/*
 *
 *
 *
 */
bool DeclarationVisitor::QualifiedLookup( class Node *inNode, Symbols::ISymbol **outSymbol )
{
	if (!outSymbol)
		return false;
	
	*outSymbol = NULL;
	
	// Given a qualified expression, we want to look up the symbol being referenced.  We do this by walking our
	// way down the left-hand side until we hit an identifier node.  That's an unqualified lookup which allows
	// us to start searching.
	if (dynamic_cast< IdentifierNode * >( inNode ))
	{
		// A left-hand side that's a pure identifier means this is the start of the lookup operation.  This
		// lookup is an unqualified one
		if (!UnqualifiedLookup( static_cast< IdentifierNode * >( inNode )->GetText(), outSymbol ))
			return false;
	}
	else if (dynamic_cast< DotExpressionNode * >( inNode ))
	{
		Symbols::ISymbol *lhsSym = NULL;
		// If the left-hand side is still a dot expression, then we have more searching to do
		if (!QualifiedLookup( static_cast< DotExpressionNode * >( inNode )->GetLHS(), &lhsSym ))
			return false;
		
		// Now that we've figured out what the left-hand side's symbol is, the right-hand side should be an identifier
		if (dynamic_cast< IdentifierNode * >( static_cast< DotExpressionNode * >( inNode )->GetRHS() ) == NULL)
		{
			lhsSym->Release();
			return false;
		}
		
		VString identText = static_cast< IdentifierNode * >( static_cast< DotExpressionNode * >( inNode )->GetRHS() )->GetText();
		
		// So let's look for the symbol on our left-hand side
		std::vector< Symbols::ISymbol * > lookup_list;
		lookup_list.push_back( const_cast< Symbols::ISymbol * >( lhsSym->RetainReferencedSymbol() ) );
		Symbols::ISymbol *temp = lookup_list.back();
		while (!lookup_list.empty())
		{
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
			
			if (*outSymbol)
				break;
			
			// Otherwise, move up the prototype chain
			lookup_list.insert( lookup_list.end(), lookup->GetPrototypes().begin(), lookup->GetPrototypes().end() );
		}
		
		temp->Release();
		lhsSym->Release();
	}
	
	return (*outSymbol) != NULL;
}



/*
 *
 *
 *
 */
bool DeclarationVisitor::UnqualifiedLookup( const VString &inIdent, Symbols::ISymbol **outSymbol )
{
	*outSymbol = NULL;
	
	// We want to walk the symbol stack to see if we can locate this identifier somewhere.  If we
	// can, then we return true.  If we can't locate the symbol, we return false.
	for (std::vector< Symbols::ISymbol * >::reverse_iterator iter = fOwnershipStack.rbegin(); iter != fOwnershipStack.rend(); ++iter)
	{
		if ((*iter) == NULL)
		{
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
		for (std::vector< Symbols::ISymbol * >::iterator subs = subSyms.begin(); subs != subSyms.end(); ++subs)
		{
			// First, test to make sure it's a private property
			int symKind = (*subs)->GetKind();
			if ( ( symKind != Symbols::ISymbol::kKindClassConstructor )  &&
				( symKind == Symbols::ISymbol::kKindLocalVariableDeclaration     ||
				 symKind == Symbols::ISymbol::kKindPrivateProperty              ||
				 symKind == Symbols::ISymbol::kKindClassPrivateMethod           ||
				 symKind == Symbols::ISymbol::kKindFunctionParameterDeclaration ||
				 symKind == Symbols::ISymbol::kKindCatchBlock                   ||
				 (*subs)->IsFunctionKind() ) )
			{
				// Next, check to see whether the text matches too
				if ((*subs)->GetName().EqualToString( inIdent, true ))
				{
					// It is possible that we've actually already saved a symbol in our return slot.  However, we
					// want to give preference to symbols that live in the same file as the one being parsed.  So
					// check to see if that's the case here.  If our new symbol is in the same file as the one being
					// parsed, we'll use it instead.
					if (!(*outSymbol))
					{
						// We don't have a return symbol yet, so we just want to keep this one regardless
						*outSymbol = *subs;
						(*outSymbol)->Retain();
					}
					else
					{
						// We already have a symbol stored here, so let's see if this one is better.  The only way it can
						// be better is if we have a context file, and if the context file's path matches the iterator's
						// file's path.
						if (fOwningFile)
						{
							VString contextFilePath = fOwningFile->GetPath();
							VString symbolFilePath = ((*subs)->GetFile()) ? (*subs)->GetFile()->GetPath() : CVSTR( "" );
							VString oldSymbolFilePath = ((*outSymbol)->GetFile()) ? (*outSymbol)->GetFile()->GetPath() : CVSTR( "" );
							
							if (!oldSymbolFilePath.EqualTo( contextFilePath, true ))
							{
								// The old symbol was in a different file from the context file
								if (symbolFilePath.EqualTo( contextFilePath, true ))
								{
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
		
		if (*outSymbol)
			return true;
	}
	
	if (outSymbol)
		*outSymbol = NULL;
	
	return false;
}



/*
 *
 *
 *
 */
Symbols::ISymbol* DeclarationVisitor::GetThis()
{
	// When we encounter the "this" node, we will return the owning function's prototype symbol for it.  This
	// isn't strictly correct since the this keyword actually relies on the caller as well as the context,
	// but we don't have the ability to determine who the caller is at compile time.  Note that we are getting
	// the external prototype property, not the internal prototype symbol
	for (std::vector< Symbols::ISymbol * >::reverse_iterator iter = fOwnershipStack.rbegin(); iter != fOwnershipStack.rend(); ++iter)
	{
		if (*iter && (*iter)->IsFunctionKind() )
		{
			return GetLikeNamedSubSymbol( *iter, CVSTR("prototype") );
		}
	}
	
	return NULL;
}



/*
 *
 *
 *
 */
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
			CreateNewInternalInstance( inLHS, GetLikeNamedSubSymbol( NULL, CVSTR("Object") ) );
		
		inLHS->SetInstanceState(true);
		
		refOfThisType->Release();
	}
}



/*
 *
 *
 *
 */
void DeclarationVisitor::CreateNewInternalInstance( Symbols::ISymbol *inLHS, Symbols::ISymbol *inOfThisType)
{
	if (NULL != inLHS && NULL != inOfThisType)
	{
		inLHS->Retain();
		inOfThisType->Retain();
		
		Symbols::ISymbol *prototype = GetLikeNamedSubSymbol( inOfThisType, CVSTR("prototype") );
		if ( NULL != prototype )
		{
			inLHS->AddPrototype( prototype );
			prototype->Release();
		}
		
		inLHS->Release();
		inOfThisType->Release();
	}
}



/*
 *
 *
 *
 */
Symbols::ISymbol* DeclarationVisitor::CreateSymbolPrototype( Symbols::ISymbol *inSym)
{
	// Functions get an external prototype property as well as an internal one.
	VSymbol *prototype = new VSymbol();
	prototype->SetID( fSymTable->GetNextUniqueID() );
	prototype->SetName( CVSTR("prototype") );
	prototype->SetOwner( inSym );
	prototype->SetFile( fOwningFile );
	prototype->SetLineNumber( inSym->GetLineNumber() );
	prototype->SetKind( Symbols::ISymbol::kKindPublicProperty );
	
	// The external prototype property is always of type Object by default, as though it
	// were set via a call to new Object();
	CreateNewInstance( prototype, GetLikeNamedSubSymbol( NULL, CVSTR("Object") ) );
	
	return prototype;
}



/*
 *
 *
 *
 */
VString DeclarationVisitor::BuildFunctionSignature(const VString& inIdentifier, class FunctionDeclarationArgumentsNode *inArgs)
{
	// We have some extra information we want to associate with function declarations.  Namely,
	// we want to add information about the function's signature.  So let's calculate what that is.
	VString signature = inIdentifier + CVSTR("(");
	
	if (inArgs)
	{
		size_t argsCount = inArgs->ArgumentCount();
		for (size_t i=0; i<argsCount; i++)
		{
			Node* arg = inArgs->GetArgument( i );
			if( dynamic_cast<IdentifierNode*>(arg) )
			{
				if (i != 0)
					signature += CVSTR(", ");
				
				signature += dynamic_cast< IdentifierNode * >( arg )->GetText();
			}
		}
	}
	
	signature += CVSTR(")");
	
	return signature;
}



/*
 *
 *
 *
 */
Symbols::ISymbol* DeclarationVisitor::GetSymbolFromLeftHandSideNode( class LeftHandSideExpressionNode *inNode, bool inCreateIfNeeded )
{
	// We've been given a left-hand side expression and we want to locate the symbol it represents.  However, it is
	// possible that the symbol doesn't exist yet.  If that's the case, we want to make a new symbol and add it to the
	// proper owner before returning it to the caller.  That is because we are assuming that the only reason we want to
	// look up a lhs symbol is because it's part of an assignment expression (or its moral equivalent), in which case the
	// symbol should be added to the table.
	if (!inNode)
		return NULL;		// Sanity check
	
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
		
	}
	else if (dynamic_cast< DotExpressionNode * >( inNode ) || dynamic_cast< ArrayExpressionNode * >( inNode ))
	{
		Symbols::ISymbol *sym = NULL;
		if (dynamic_cast< DotExpressionNode * >( inNode ))
			sym = GetSymbolFromLeftHandSideNode( dynamic_cast< LeftHandSideExpressionNode * >( dynamic_cast< DotExpressionNode * >( inNode )->GetLHS() ), inCreateIfNeeded );
		else
			sym = GetSymbolFromLeftHandSideNode( dynamic_cast< LeftHandSideExpressionNode * >( dynamic_cast< ArrayExpressionNode * >( inNode )->GetLHS() ), inCreateIfNeeded );
		
		if (sym)
		{
			// If parent symbol has just been created in the current statement we can flag it as undefined
			// It will help us to find the real symbol definition in other statements or files
			std::vector< Symbols::ISymbol * >::const_iterator it = find(fSymbolsToAdd.begin(), fSymbolsToAdd.end(), sym);
			if (it != fSymbolsToAdd.end() && inNode->GetLineNumber() == sym->GetLineNumber())
			{
				//( sym->GetName() == CVSTR("exports") ) ? sym->SetUndefinedState(false) : sym->SetUndefinedState(true);
				sym->SetUndefinedState(true);
			}
			
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
					if ( refSym && refSym->GetKind() == Symbols::ISymbol::kKindClass && rhsText != CVSTR("prototype") )
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
	}
	else if (dynamic_cast< ThisNode * >( inNode ))
	{
		return GetThis();
	}
	
	return NULL;
}



/*
 *
 *
 *
 */
Symbols::ISymbol* DeclarationVisitor::RetainSymbolPrototype(Symbols::ISymbol* inOwner, const VString& inSymbolName, const ESymbolFileExecContext& inExecContext)
{
	Symbols::ISymbol* ret = NULL;
	
	std::vector<Symbols::ISymbol*> syms;
	RetainLikeNamedSubSymbols(inOwner, inSymbolName, syms);
	
	std::vector< Symbols::ISymbol * >::iterator it;
	for (it = syms.begin(); it != syms.end(); ++it)
	{
		if( !ret )
		{
			Symbols::ISymbol* sym = GetLikeNamedSubSymbol((*it), CVSTR("prototype") );
			if( sym && sym->GetFile()->GetExecutionContext() == inExecContext )
				ret = sym;
		}
		
		(*it)->Release();
	}
	
	return ret;
}



/*
 *
 *
 *
 */
bool DeclarationVisitor::VisitRightHandSideOfAssignment( Symbols::ISymbol *inLHS, class Node *inNode, bool inForFakeAssignment )
{
	// This function performs the actual assignment of information from the RHS to the LHS.  Generally,
	// the most important pieces of information being copied over are prototypes, but there are situations
	// where other information is assigned as well, such as whether it is an instance value, public properties, etc.
	if (!inLHS)
		return true;
	
	inLHS->SetUndefinedState(false);
	
	if (dynamic_cast< IdentifierNode * >( inNode ) || dynamic_cast< DotExpressionNode * >( inNode ))
	{
		Symbols::ISymbol *ident = NULL;
		if (QualifiedLookup( inNode, &ident ))
		{
			// The left-hand side now references the right-hand side
			Symbols::ISymbol *temp = const_cast< Symbols::ISymbol * >( ident->RetainReferencedSymbol() );
			inLHS->SetReferenceSymbol( temp );
			temp->Release();
			ident->Release();
		}
	}
	else if (dynamic_cast< ThisNode * >( inNode ))
	{
		// In this case, we are saying the left hand side's prototype is the this pointer
		Symbols::ISymbol *thisSym = GetThis();
		if (thisSym) {
			inLHS->AddPrototype( thisSym );
			thisSym->Release();
		}
	}
	else if (dynamic_cast< ObjectLiteralNode * >( inNode ))
	{
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
			inLHS->SetInstanceState(true);
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
				prototype = RetainSymbolPrototype(newExpr->GetOwner(), newExpr->GetName(), execCtx);
				if( prototype )
					inLHS->AddPrototype(prototype);
				
				ReleaseRefCountable( &prototype );
				
				prototype = RetainSymbolPrototype( NULL, CVSTR("Entity"), execCtx);
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
	else if (dynamic_cast< FunctionExpressionNode * >( inNode ))
	{
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
		if (!syms.empty())
		{
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
	}
	else if (dynamic_cast< FunctionCallExpressionNode * >( inNode ))
	{
		// We have a function call, so we want to find the function's symbol and determine it's return
		// type, as that will be the right-hand symbol's new prototype.
		Node *functionCallLHS = dynamic_cast< FunctionCallExpressionNode * >( inNode )->GetLHS();
		Symbols::ISymbol *functionCall = NULL;
		if (QualifiedLookup( functionCallLHS, &functionCall ))
		{
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
			
			// WAK0082247
			// Make autocomplete works for method starting with an Uppercase (which is the rule for constructor method name
			IdentifierNode* idNode = dynamic_cast< IdentifierNode * >( functionCallLHS );
			if( idNode )
			{
				VString szId = idNode->GetText();
				if( !szId.IsEmpty() )
				{
					if( szId[0] >= 'A' && szId[0] <= 'Z' )
						isConstructorCall = true;
				}
			} // END OF WAK0082247
			
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
									Symbols::ISymbol *prototype = GetLikeNamedSubSymbol( entityModel, CVSTR("prototype") );
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
			typeSym = GetLikeNamedSubSymbol( NULL, CVSTR("String") );
		else if (dynamic_cast< NumericLiteralNode * >( node ))
			typeSym = GetLikeNamedSubSymbol( NULL, CVSTR("Number") );
		else if (dynamic_cast< BooleanLiteralNode * >( node ))
			typeSym = GetLikeNamedSubSymbol( NULL, CVSTR("Boolean") );
		else if (dynamic_cast< RegExLiteralNode * >( node ))
			typeSym = GetLikeNamedSubSymbol( NULL, CVSTR("RegExp") );
		else if (dynamic_cast< ArrayLiteralNode * >( node ))
			typeSym = GetLikeNamedSubSymbol( NULL, CVSTR("Array") );
		
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



/*
 *
 *
 *
 */
bool DeclarationVisitor::VisitAssignExpressionNodeEnter( class AssignExpressionNode *inNode )
{
	// The assignment expression is really the only tricky problem in declaration parsing.  The
	// name on the left-hand side of the assignment may become a new symbol.  But it may also be
	// an existing symbol.  We will try to locate the symbol, and if we cannot, we will create a
	// new one and add it to the proper owner.
	Symbols::ISymbol *lhs = GetSymbolFromLeftHandSideNode( dynamic_cast< LeftHandSideExpressionNode * >( inNode->GetLHS() ) );
	if (!lhs)
		return true;
	
	if (dynamic_cast< LeftHandSideExpressionNode * >( inNode->GetLHS() )->GetScriptDocComment())
	{
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



/*
 *
 *
 *
 */
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
						Symbols::ISymbol *prototype = GetLikeNamedSubSymbol( refReturnTypeSym, CVSTR("prototype") );
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



/*
 *
 *
 *
 */
void DeclarationVisitor::SetSymbolClassesFromScriptDoc(Symbols::ISymbol* inSym, bool inAddToUpdateList, bool inUpdateSymbol)
{
	ScriptDocComment *comment = ScriptDocComment::Create( inSym->GetScriptDocComment() );
	if ( NULL != comment )
	{
		// get appropriate JSDoc tag for iheritance
		ScriptDocLexer::Element *element = comment->GetElement( IScriptDocCommentField::kExtends );
		if (NULL == element)
			element = comment->GetElement( IScriptDocCommentField::kInherits );
		
		if ( NULL != element)
		{
			VString baseClasses = static_cast< ScriptDocLexer::ExtendsElement * >( element )->fBaseClass;
			VectorOfVString classes;
			
			baseClasses.GetSubStrings( CHAR_VERTICAL_LINE, classes, false, true);
			if ( !classes.empty() )
			{
				
				// WA0083240 : AVOID POSSIBLE INFINITE RECURSION WHILE TRYING TO EXTEND A CLASS WITH ITSELF
				bool possibleInfiniteRecursion = false;
				for(VectorOfVString::const_iterator cit=classes.begin(); cit!=classes.end(); ++cit)
				{
					if( (*cit) == inSym->GetName() )
					{
						possibleInfiniteRecursion = true;
						break;
					}
				}
				// WAK0083240 STOP
				if( !possibleInfiniteRecursion )
					SetSymbolPrototypes( inSym, classes, inAddToUpdateList, inUpdateSymbol );
			}
		}
		comment->Release();
	}
}



/*
 *
 *
 *
 */
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



/*
 *
 *
 *
 */
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



/*
 *
 *
 *
 */
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

































































/*
 * Note about execution context (scope)
 *	There is two kind of execution context : global and local (function)
 *	Context can be augmented in two cases : catch and with
 *	Each AST comes with at least a global context (set in the AST WALKER constructor)
 *	Each time we enter a function declaration / expression, catch or with node : create a new execution context and push it on the current one
 *	Each time we leave a function declaration / expression, catch or with node : pop the current execution context from the stack
 *
 */
class NextGenJsAstWalker : public Visitor
{
public:
	NextGenJsAstWalker();
	virtual ~NextGenJsAstWalker();
	
	virtual const SymbolScopeDelaration* GetScopeDeclaration() const
	{
		return &fScopes;
	}
	
private:
	/*
	 *
	 *	SCOPE MANAGEMENT
	 *
	 */
	SymbolScopeDelaration	fScopes;
	SymbolScopeDelaration*	fCurrentScope;
	
	void PushScope(const XBOX::VString& inName, const sLONG& inType)
	{
		SymbolScopeDelaration* newScope = new SymbolScopeDelaration(fCurrentScope, inName, inType);
		fCurrentScope->AddChildScope(newScope);
		fCurrentScope = newScope;
	}
	
	void PopScope()
	{
		SymbolScopeDelaration* parent = fCurrentScope->GetParentScope();
		fCurrentScope = parent;
	}
	
	
	
	/*
	 *
	 *	DECLARATIONS MANAGEMENT
	 *
	 */
	void AddAssignmentDeclarationToScope(ISymbolDeclaration* inDeclaration)
	{
		fCurrentScope->AddAssignmentDeclaration(inDeclaration);
	}

	void AddVariableDeclarationToScope(ISymbolDeclaration* inDeclaration)
	{
		fCurrentScope->AddVariableDeclaration(inDeclaration);
	}
	
	void AddFunctionDeclarationToScope(ISymbolDeclaration* inDeclaration)
	{
		fCurrentScope->AddFunctionDeclaration(inDeclaration);
	}
	
	void AddFunctionExpressionDeclarationToScope(ISymbolDeclaration* inDeclaration)
	{
		fCurrentScope->AddFunctionExpressionDeclaration(inDeclaration);
	}

	void AddNewDeclarationToScope(ISymbolDeclaration* inDeclaration)
	{
		fCurrentScope->AddNewDeclaration(inDeclaration);
	}


	
	
	
	/*
	 *
	 *	TRACES MANAGEMENT
	 *
	 */
	void PrintTrace(const XBOX::VString& inKey, const XBOX::VString& inValue)
	{
#if ACTIVATE_JSLOG
		JSLog log;
		log.SetTitle("AST NEXT GEN");
		log.Append(inKey, inValue);
#endif
	}
	
	
	
	
	
	
	/*
	 *
	 *	NODES
	 *
	 */
	virtual bool VisitFunctionDeclarationStatementNodeEnter			( class FunctionDeclarationStatementNode *inNode );
	virtual bool VisitFunctionDeclarationStatementNodeLeave			( class FunctionDeclarationStatementNode *inNode );
	
	virtual bool VisitFunctionExpressionNodeEnter					( class FunctionExpressionNode *inNode );
	virtual bool VisitFunctionExpressionNodeLeave					( class FunctionExpressionNode *inNode );

	virtual bool VisitCatchStatementNodeEnter						( class CatchStatementNode *inNode );
	virtual bool VisitCatchStatementNodeLeave						( class CatchStatementNode *inNode );

	virtual bool VisitWithStatementNodeEnter						( class WithStatementNode *inNode );
	virtual bool VisitWithStatementNodeLeave						( class WithStatementNode *inNode );
	
	virtual bool VisitVariableDeclarationStatementNodeEnter			( class VariableDeclarationStatementNode *inNode );
	virtual bool VisitVariableDeclarationStatementNodeLeave			( class VariableDeclarationStatementNode *inNode );
	
	virtual bool VisitVariableDeclarationListStatementNodeEnter		( class VariableDeclarationListStatementNode *inNode );
	virtual bool VisitVariableDeclarationListStatementNodeLeave		( class VariableDeclarationListStatementNode *inNode );
	
	virtual bool VisitObjectLiteralNodeEnter						( class ObjectLiteralNode *inNode );
	virtual bool VisitObjectLiteralNodeLeave						( class ObjectLiteralNode *inNode );
	
	virtual bool VisitObjectLiteralFieldNodeEnter					( class ObjectLiteralFieldNode *inNode );
	virtual bool VisitObjectLiteralFieldNodeLeave					( class ObjectLiteralFieldNode *inNode );
	
	virtual bool VisitDotExpressionNodeEnter						( class DotExpressionNode *inNode );
	virtual bool VisitDotExpressionNodeLeave						( class DotExpressionNode *inNode );
	
	virtual bool VisitProgramNodeEnter								( class ProgramNode *inNode );
	virtual bool VisitProgramNodeLeave								( class ProgramNode *inNode );
	
	virtual bool VisitFunctionDeclarationArgumentsNodeEnter			( class FunctionDeclarationArgumentsNode *inNode );
	virtual bool VisitFunctionDeclarationArgumentsNodeLeave			( class FunctionDeclarationArgumentsNode *inNode );

	virtual bool VisitFunctionCallArgumentsNodeEnter				( class FunctionCallArgumentsNode *inNode );
	virtual bool VisitFunctionCallArgumentsNodeLeave				( class FunctionCallArgumentsNode *inNode );

	virtual bool VisitLeftHandSideExpressionNodeEnter				( class LeftHandSideExpressionNode *inNode );
	virtual bool VisitLeftHandSideExpressionNodeLeave				( class LeftHandSideExpressionNode *inNode );

	virtual bool VisitUnaryExpressionNodeEnter						( class UnaryExpressionNode *inNode );
	virtual bool VisitUnaryExpressionNodeLeave						( class UnaryExpressionNode *inNode );

	virtual bool VisitDeleteExpressionNodeEnter						( class DeleteExpressionNode *inNode );
	virtual bool VisitDeleteExpressionNodeLeave						( class DeleteExpressionNode *inNode );
	
	virtual bool VisitVoidExpressionNodeEnter						( class VoidExpressionNode *inNode );
	virtual bool VisitVoidExpressionNodeLeave						( class VoidExpressionNode *inNode );
	
	virtual bool VisitTypeOfExpressionNodeEnter						( class TypeOfExpressionNode *inNode );
	virtual bool VisitTypeOfExpressionNodeLeave						( class TypeOfExpressionNode *inNode );
	
	virtual bool VisitPreIncrementorExpressionNodeEnter				( class PreIncrementorExpressionNode *inNode );
	virtual bool VisitPreIncrementorExpressionNodeLeave				( class PreIncrementorExpressionNode *inNode );
	
	virtual bool VisitPreDecrementorExpressionNodeEnter				( class PreDecrementorExpressionNode *inNode );
	virtual bool VisitPreDecrementorExpressionNodeLeave				( class PreDecrementorExpressionNode *inNode );
	
	virtual bool VisitUnaryPlusExpressionNodeEnter					( class UnaryPlusExpressionNode *inNode );
	virtual bool VisitUnaryPlusExpressionNodeLeave					( class UnaryPlusExpressionNode *inNode );
	
	virtual bool VisitUnaryNegateExpressionNodeEnter				( class UnaryNegateExpressionNode *inNode );
	virtual bool VisitUnaryNegateExpressionNodeLeave				( class UnaryNegateExpressionNode *inNode );
	
	virtual bool VisitBitwiseNotExpressionNodeEnter					( class BitwiseNotExpressionNode *inNode );
	virtual bool VisitBitwiseNotExpressionNodeLeave					( class BitwiseNotExpressionNode *inNode );
	
	virtual bool VisitLogicalNotExpressionNodeEnter					( class LogicalNotExpressionNode *inNode );
	virtual bool VisitLogicalNotExpressionNodeLeave					( class LogicalNotExpressionNode *inNode );
	
	virtual bool VisitPostIncrementorNodeEnter						( class PostIncrementorNode *inNode );
	virtual bool VisitPostIncrementorNodeLeave						( class PostIncrementorNode *inNode );
	
	virtual bool VisitPostDecrementorNodeEnter						( class PostDecrementorNode *inNode );
	virtual bool VisitPostDecrementorNodeLeave						( class PostDecrementorNode *inNode );
	
	virtual bool VisitBinaryExpressionNodeEnter						( class BinaryExpressionNode *inNode );
	virtual bool VisitBinaryExpressionNodeLeave						( class BinaryExpressionNode *inNode );
	
	virtual bool VisitAdditionExpressionNodeEnter					( class AdditionExpressionNode *inNode );
	virtual bool VisitAdditionExpressionNodeLeave					( class AdditionExpressionNode *inNode );
	
	virtual bool VisitSubtractionExpressionNodeEnter				( class SubtractionExpressionNode *inNode );
	virtual bool VisitSubtractionExpressionNodeLeave				( class SubtractionExpressionNode *inNode );
	
	virtual bool VisitMultiplicationExpressionNodeEnter				( class MultiplicationExpressionNode *inNode );
	virtual bool VisitMultiplicationExpressionNodeLeave				( class MultiplicationExpressionNode *inNode );
	
	virtual bool VisitDivisionExpressionNodeEnter					( class DivisionExpressionNode *inNode );
	virtual bool VisitDivisionExpressionNodeLeave					( class DivisionExpressionNode *inNode );
	
	virtual bool VisitLeftShiftExpressionNodeEnter					( class LeftShiftExpressionNode *inNode );
	virtual bool VisitLeftShiftExpressionNodeLeave					( class LeftShiftExpressionNode *inNode );
	
	virtual bool VisitSignedRightShiftExpressionNodeEnter			( class SignedRightShiftExpressionNode *inNode );
	virtual bool VisitSignedRightShiftExpressionNodeLeave			( class SignedRightShiftExpressionNode *inNode );
	
	virtual bool VisitUnsignedRightShiftExpressionNodeEnter			( class UnsignedRightShiftExpressionNode *inNode );
	virtual bool VisitUnsignedRightShiftExpressionNodeLeave			( class UnsignedRightShiftExpressionNode *inNode );
	
	virtual bool VisitInExpressionNodeEnter							( class InExpressionNode *inNode );
	virtual bool VisitInExpressionNodeLeave							( class InExpressionNode *inNode );
	
	virtual bool VisitLessThanExpressionNodeEnter					( class LessThanExpressionNode *inNode );
	virtual bool VisitLessThanExpressionNodeLeave					( class LessThanExpressionNode *inNode );
	
	virtual bool VisitGreaterThanExpressionNodeEnter				( class GreaterThanExpressionNode *inNode );
	virtual bool VisitGreaterThanExpressionNodeLeave				( class GreaterThanExpressionNode *inNode );
	
	virtual bool VisitLessThanOrEqualToExpressionNodeEnter			( class LessThanOrEqualToExpressionNode *inNode );
	virtual bool VisitLessThanOrEqualToExpressionNodeLeave			( class LessThanOrEqualToExpressionNode *inNode );
	
	virtual bool VisitGreaterThanOrEqualToExpressionNodeEnter		( class GreaterThanOrEqualToExpressionNode *inNode );
	virtual bool VisitGreaterThanOrEqualToExpressionNodeLeave		( class GreaterThanOrEqualToExpressionNode *inNode );
	
	virtual bool VisitInstanceOfExpressionNodeEnter					( class InstanceOfExpressionNode *inNode );
	virtual bool VisitInstanceOfExpressionNodeLeave					( class InstanceOfExpressionNode *inNode );
	
	virtual bool VisitEqualityExpressionNodeEnter					( class EqualityExpressionNode *inNode );
	virtual bool VisitEqualityExpressionNodeLeave					( class EqualityExpressionNode *inNode );
	
	virtual bool VisitNonEqualityExpressionNodeEnter				( class NonEqualityExpressionNode *inNode );
	virtual bool VisitNonEqualityExpressionNodeLeave				( class NonEqualityExpressionNode *inNode );
	
	virtual bool VisitStrictEqualityExpressionNodeEnter				( class StrictEqualityExpressionNode *inNode );
	virtual bool VisitStrictEqualityExpressionNodeLeave				( class StrictEqualityExpressionNode *inNode );
	
	virtual bool VisitStrictNonEqualityExpressionNodeEnter			( class StrictNonEqualityExpressionNode *inNode );
	virtual bool VisitStrictNonEqualityExpressionNodeLeave			( class StrictNonEqualityExpressionNode *inNode );
	
	virtual bool VisitBitwiseAndExpressionNodeEnter					( class BitwiseAndExpressionNode *inNode );
	virtual bool VisitBitwiseAndExpressionNodeLeave					( class BitwiseAndExpressionNode *inNode );
	
	virtual bool VisitBitwiseXorExpressionNodeEnter					( class BitwiseXorExpressionNode *inNode );
	virtual bool VisitBitwiseXorExpressionNodeLeave					( class BitwiseXorExpressionNode *inNode );
	
	virtual bool VisitBitwiseOrExpressionNodeEnter					( class BitwiseOrExpressionNode *inNode );
	virtual bool VisitBitwiseOrExpressionNodeLeave					( class BitwiseOrExpressionNode *inNode );
	
	virtual bool VisitLogicalAndExpressionNodeEnter					( class LogicalAndExpressionNode *inNode );
	virtual bool VisitLogicalAndExpressionNodeLeave					( class LogicalAndExpressionNode *inNode );
	
	virtual bool VisitLogicalOrExpressionNodeEnter					( class LogicalOrExpressionNode *inNode );
	virtual bool VisitLogicalOrExpressionNodeLeave					( class LogicalOrExpressionNode *inNode );
	
	virtual bool VisitModulusExpressionNodeEnter					( class ModulusExpressionNode *inNode );
	virtual bool VisitModulusExpressionNodeLeave					( class ModulusExpressionNode *inNode );
	
	virtual bool VisitAssignMultiplyExpressionNodeEnter				( class AssignMultiplyExpressionNode *inNode );
	virtual bool VisitAssignMultiplyExpressionNodeLeave				( class AssignMultiplyExpressionNode *inNode );
	
	virtual bool VisitAssignAddExpressionNodeEnter					( class AssignAddExpressionNode *inNode );
	virtual bool VisitAssignAddExpressionNodeLeave					( class AssignAddExpressionNode *inNode );
	
	virtual bool VisitAssignSubtractExpressionNodeEnter				( class AssignSubtractExpressionNode *inNode );
	virtual bool VisitAssignSubtractExpressionNodeLeave				( class AssignSubtractExpressionNode *inNode );
	
	virtual bool VisitAssignDivideExpressionNodeEnter				( class AssignDivideExpressionNode *inNode );
	virtual bool VisitAssignDivideExpressionNodeLeave				( class AssignDivideExpressionNode *inNode );
	
	virtual bool VisitAssignModulusExpressionNodeEnter				( class AssignModulusExpressionNode *inNode );
	virtual bool VisitAssignModulusExpressionNodeLeave				( class AssignModulusExpressionNode *inNode );
	
	virtual bool VisitAssignLeftShiftExpressionNodeEnter			( class AssignLeftShiftExpressionNode *inNode );
	virtual bool VisitAssignLeftShiftExpressionNodeLeave			( class AssignLeftShiftExpressionNode *inNode );
	
	virtual bool VisitAssignSignedRightShiftExpressionNodeEnter		( class AssignSignedRightShiftExpressionNode *inNode );
	virtual bool VisitAssignSignedRightShiftExpressionNodeLeave		( class AssignSignedRightShiftExpressionNode *inNode );
	
	virtual bool VisitAssignUnsignedRightShiftExpressionNodeEnter	( class AssignUnsignedRightShiftExpressionNode *inNode );
	virtual bool VisitAssignUnsignedRightShiftExpressionNodeLeave	( class AssignUnsignedRightShiftExpressionNode *inNode );
	
	virtual bool VisitAssignBitwiseAndExpressionNodeEnter			( class AssignBitwiseAndExpressionNode *inNode );
	virtual bool VisitAssignBitwiseAndExpressionNodeLeave			( class AssignBitwiseAndExpressionNode *inNode );
	
	virtual bool VisitAssignBitwiseOrExpressionNodeEnter			( class AssignBitwiseOrExpressionNode *inNode );
	virtual bool VisitAssignBitwiseOrExpressionNodeLeave			( class AssignBitwiseOrExpressionNode *inNode );
	
	virtual bool VisitAssignBitwiseXorExpressionNodeEnter			( class AssignBitwiseXorExpressionNode *inNode );
	virtual bool VisitAssignBitwiseXorExpressionNodeLeave			( class AssignBitwiseXorExpressionNode *inNode );
	
	virtual bool VisitAssignExpressionNodeEnter						( class AssignExpressionNode *inNode );
	virtual bool VisitAssignExpressionNodeLeave						( class AssignExpressionNode *inNode );
	
	virtual bool VisitAssignmentExpressionNodeEnter					( class AssignmentExpressionNode *inNode );
	virtual bool VisitAssignmentExpressionNodeLeave					( class AssignmentExpressionNode *inNode );
	
	virtual bool VisitCommaExpressionNodeEnter						( class CommaExpressionNode *inNode );
	virtual bool VisitCommaExpressionNodeLeave						( class CommaExpressionNode *inNode );
	
	virtual bool VisitConditionalExpressionNodeEnter				( class ConditionalExpressionNode *inNode );
	virtual bool VisitConditionalExpressionNodeLeave				( class ConditionalExpressionNode *inNode );
	
	virtual bool VisitFunctionCallExpressionNodeEnter				( class FunctionCallExpressionNode *inNode );
	virtual bool VisitFunctionCallExpressionNodeLeave				( class FunctionCallExpressionNode *inNode );
	
	virtual bool VisitNewNodeEnter									( class NewNode *inNode );
	virtual bool VisitNewNodeLeave									( class NewNode *inNode );
	
	virtual bool VisitArrayExpressionNodeEnter						( class ArrayExpressionNode *inNode );
	virtual bool VisitArrayExpressionNodeLeave						( class ArrayExpressionNode *inNode );
	
	virtual bool VisitThisNodeEnter									( class ThisNode *inNode );
	virtual bool VisitThisNodeLeave									( class ThisNode *inNode );
	
	virtual bool VisitIdentifierNodeEnter							( class IdentifierNode *inNode );
	virtual bool VisitIdentifierNodeLeave							( class IdentifierNode *inNode );
	
	virtual bool VisitArrayLiteralNodeEnter							( class ArrayLiteralNode *inNode );
	virtual bool VisitArrayLiteralNodeLeave							( class ArrayLiteralNode *inNode );
	
	virtual bool VisitNullLiteralNodeEnter							( class NullLiteralNode *inNode );
	virtual bool VisitNullLiteralNodeLeave							( class NullLiteralNode *inNode );
	
	virtual bool VisitStringLiteralNodeEnter						( class StringLiteralNode *inNode );
	virtual bool VisitStringLiteralNodeLeave						( class StringLiteralNode *inNode );
	
	virtual bool VisitNumericLiteralNodeEnter						( class NumericLiteralNode *inNode );
	virtual bool VisitNumericLiteralNodeLeave						( class NumericLiteralNode *inNode );
	
	virtual bool VisitBooleanLiteralNodeEnter						( class BooleanLiteralNode *inNode );
	virtual bool VisitBooleanLiteralNodeLeave						( class BooleanLiteralNode *inNode );
	
	virtual bool VisitRegExLiteralNodeEnter							( class RegExLiteralNode *inNode );
	virtual bool VisitRegExLiteralNodeLeave							( class RegExLiteralNode *inNode );
	
	virtual bool VisitStatementNodeEnter							( class StatementNode *inNode );
	virtual bool VisitStatementNodeLeave							( class StatementNode *inNode );
	
	virtual bool VisitEmptyStatementNodeEnter						( class EmptyStatementNode *inNode );
	virtual bool VisitEmptyStatementNodeLeave						( class EmptyStatementNode *inNode );
	
	virtual bool VisitLabeledStatementNodeEnter						( class LabeledStatementNode *inNode );
	virtual bool VisitLabeledStatementNodeLeave						( class LabeledStatementNode *inNode );
	
	virtual bool VisitIfStatementNodeEnter							( class IfStatementNode *inNode );
	virtual bool VisitIfStatementNodeLeave							( class IfStatementNode *inNode );
	
	virtual bool VisitContinueStatementNodeEnter					( class ContinueStatementNode *inNode );
	virtual bool VisitContinueStatementNodeLeave					( class ContinueStatementNode *inNode );
	
	virtual bool VisitBreakStatementNodeEnter						( class BreakStatementNode *inNode );
	virtual bool VisitBreakStatementNodeLeave						( class BreakStatementNode *inNode );
	
	virtual bool VisitDebuggerStatementNodeEnter					( class DebuggerStatementNode *inNode );
	virtual bool VisitDebuggerStatementNodeLeave					( class DebuggerStatementNode *inNode );
	
	virtual bool VisitReturnStatementNodeEnter						( class ReturnStatementNode *inNode );
	virtual bool VisitReturnStatementNodeLeave						( class ReturnStatementNode *inNode );
	
	virtual bool VisitThrowStatementNodeEnter						( class ThrowStatementNode *inNode );
	virtual bool VisitThrowStatementNodeLeave						( class ThrowStatementNode *inNode );
	
	virtual bool VisitTryStatementNodeEnter							( class TryStatementNode *inNode );
	virtual bool VisitTryStatementNodeLeave							( class TryStatementNode *inNode );
	
	virtual bool VisitSwitchStatementNodeEnter						( class SwitchStatementNode *inNode );
	virtual bool VisitSwitchStatementNodeLeave						( class SwitchStatementNode *inNode );
	
	virtual bool VisitCaseClauseNodeEnter							( class CaseClauseNode *inNode );
	virtual bool VisitCaseClauseNodeLeave							( class CaseClauseNode *inNode );
	
	virtual bool VisitDefaultClauseNodeEnter						( class DefaultClauseNode *inNode );
	virtual bool VisitDefaultClauseNodeLeave						( class DefaultClauseNode *inNode );
	
	virtual bool VisitDoStatementNodeEnter							( class DoStatementNode *inNode );
	virtual bool VisitDoStatementNodeLeave							( class DoStatementNode *inNode );
	
	virtual bool VisitWhileStatementNodeEnter						( class WhileStatementNode *inNode );
	virtual bool VisitWhileStatementNodeLeave						( class WhileStatementNode *inNode );
	
	virtual bool VisitForStatementNodeEnter							( class ForStatementNode *inNode );
	virtual bool VisitForStatementNodeLeave							( class ForStatementNode *inNode );
	
	virtual bool VisitForExpressionTriClauseNodeEnter				( class ForExpressionTriClauseNode *inNode );
	virtual bool VisitForExpressionTriClauseNodeLeave				( class ForExpressionTriClauseNode *inNode );
	
	virtual bool VisitForExpressionInClauseNodeEnter				( class ForExpressionInClauseNode *inNode );
	virtual bool VisitForExpressionInClauseNodeLeave				( class ForExpressionInClauseNode *inNode );
	
	virtual bool VisitStatementListNodeEnter						( class StatementList *inNode );
	virtual bool VisitStatementListNodeLeave						( class StatementList *inNode );
};



/*
 *
 *
 *
 */
NextGenJsAstWalker::NextGenJsAstWalker():
fScopes(NULL, CVSTR("global"), SymbolScopeDelaration::kTypeGlobal),
fCurrentScope(&fScopes)
{
}



/*
 *
 *
 *
 */
NextGenJsAstWalker::~NextGenJsAstWalker()
{
	SymbolScopeDelaration::Print(&fScopes, "");
}






/*
 *
 *
 *
 */
bool NextGenJsAstWalker::VisitFunctionDeclarationStatementNodeEnter(class FunctionDeclarationStatementNode *inNode )
{
	PrintTrace("Visit", "VisitFunctionDeclarationStatementNodeEnter");
	
	// Create a new function declaration that will be pushed on the function declaration stack
	AddFunctionDeclarationToScope(inNode->GetDeclaration());

	// Each function as its own execution context, let's create a new context and update the current execution context
	PushScope(inNode->GetIdentifier(), SymbolScopeDelaration::kTypeFunction);
	
	return true;
}



/*
 *
 *
 *
 */
bool NextGenJsAstWalker::VisitFunctionDeclarationStatementNodeLeave(class FunctionDeclarationStatementNode *inNode )
{
	PrintTrace("Visit", "VisitFunctionDeclarationStatementNodeLeave");
	
	// As we leave a function exection context, we have to setup the next execution to the parent of the current one
	PopScope();
	
	return true;
}



/*
 *
 *
 *
 */
bool NextGenJsAstWalker::VisitFunctionExpressionNodeEnter( class FunctionExpressionNode *inNode )
{
	PrintTrace("Visit", "VisitFunctionExpressionNodeEnter");
	
	// Create a new function declaration that will be pushed on the function declaration stack
	AddFunctionExpressionDeclarationToScope(inNode->GetDeclaration());

	// Each function as its own execution context, let's create a new context and update the current execution context
	PushScope(inNode->GetIdentifier(), SymbolScopeDelaration::kTypeFunction);

	return true;
}



/*
 *
 *
 *
 */
bool NextGenJsAstWalker::VisitFunctionExpressionNodeLeave( class FunctionExpressionNode *inNode )
{
	PrintTrace("Visit", "VisitFunctionExpressionNodeLeave");

	// As we leave a function exection context, we have to setup the next execution to the parent of the current one
	PopScope();
	
	return true;
}



/*
 *
 *
 *
 */
bool NextGenJsAstWalker::VisitCatchStatementNodeEnter(class CatchStatementNode *inNode )
{
	PrintTrace("Visit", "VisitCatchStatementNodeEnter");
	
	// Each catch as its own execution context, let's create a new context and update the current execution context
	PushScope(CVSTR(""), SymbolScopeDelaration::kTypeCatch);

	return true;
}



/*
 *
 *
 *
 */
bool NextGenJsAstWalker::VisitCatchStatementNodeLeave(class CatchStatementNode *inNode )
{
	PrintTrace("Visit", "VisitCatchStatementNodeLeave");
	
	// As we leave a catch exection context, we have to setup the next execution to the parent of the current one
	PopScope();

	return true;
}



/*
 *
 *
 *
 */
bool NextGenJsAstWalker::VisitWithStatementNodeEnter(class WithStatementNode *inNode )
{
	PrintTrace("Visit", "VisitWithStatementNodeEnter");

	// Each with as its own execution context, let's create a new context and update the current execution context
	PushScope(CVSTR(""), SymbolScopeDelaration::kTypeWith);

	return true;
}



/*
 *
 *
 *
 */
bool NextGenJsAstWalker::VisitWithStatementNodeLeave(class WithStatementNode *inNode )
{
	PrintTrace("Visit", "VisitWithStatementNodeLeave");

	// As we leave a with exection context, we have to setup the next execution to the parent of the current one
	PopScope();

	return true;
}



/*
 *
 *
 *
 */
bool NextGenJsAstWalker::VisitVariableDeclarationStatementNodeEnter(class VariableDeclarationStatementNode *inNode)
{
	PrintTrace("Visit", "VisitVariableDeclarationStatementNodeEnter");

	AddVariableDeclarationToScope(inNode->GetDeclaration());

	return true;
}

bool NextGenJsAstWalker::VisitVariableDeclarationStatementNodeLeave(class VariableDeclarationStatementNode *inNode)
{
	PrintTrace("Visit", "VisitVariableDeclarationStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitVariableDeclarationListStatementNodeEnter(class VariableDeclarationListStatementNode *inNode)
{
	PrintTrace("Visit", "VisitVariableDeclarationListStatementNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitVariableDeclarationListStatementNodeLeave(class VariableDeclarationListStatementNode *inNode)
{
	PrintTrace("Visit", "VisitVariableDeclarationListStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitObjectLiteralNodeEnter(class ObjectLiteralNode *inNode)
{
	PrintTrace("Visit", "VisitObjectLiteralNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitObjectLiteralNodeLeave(class ObjectLiteralNode *inNode)
{
	PrintTrace("Visit", "VisitObjectLiteralNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitObjectLiteralFieldNodeEnter(class ObjectLiteralFieldNode *inNode)
{
	PrintTrace("Visit", "VisitObjectLiteralFieldNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitObjectLiteralFieldNodeLeave(class ObjectLiteralFieldNode *inNode)
{
	PrintTrace("Visit", "VisitObjectLiteralFieldNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitDotExpressionNodeEnter(class DotExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitDotExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitDotExpressionNodeLeave(class DotExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitDotExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitProgramNodeEnter(class ProgramNode *inNode)
{
	PrintTrace("Visit", "VisitProgramNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitProgramNodeLeave(class ProgramNode *inNode)
{
	PrintTrace("Visit", "VisitProgramNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitFunctionDeclarationArgumentsNodeEnter(class FunctionDeclarationArgumentsNode *inNode)
{
	PrintTrace("Visit", "VisitFunctionDeclarationArgumentsNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitFunctionDeclarationArgumentsNodeLeave(class FunctionDeclarationArgumentsNode *inNode)
{
	PrintTrace("Visit", "VisitFunctionDeclarationArgumentsNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitFunctionCallArgumentsNodeEnter(class FunctionCallArgumentsNode *inNode)
{
	PrintTrace("Visit", "VisitFunctionCallArgumentsNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitFunctionCallArgumentsNodeLeave(class FunctionCallArgumentsNode *inNode)
{
	PrintTrace("Visit", "VisitFunctionCallArgumentsNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitLeftHandSideExpressionNodeEnter(class LeftHandSideExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitLeftHandSideExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitLeftHandSideExpressionNodeLeave(class LeftHandSideExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitLeftHandSideExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitUnaryExpressionNodeEnter(class UnaryExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitUnaryExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitUnaryExpressionNodeLeave(class UnaryExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitUnaryExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitDeleteExpressionNodeEnter(class DeleteExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitDeleteExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitDeleteExpressionNodeLeave(class DeleteExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitDeleteExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitVoidExpressionNodeEnter(class VoidExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitVoidExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitVoidExpressionNodeLeave(class VoidExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitVoidExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitTypeOfExpressionNodeEnter(class TypeOfExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitTypeOfExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitTypeOfExpressionNodeLeave(class TypeOfExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitTypeOfExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitPreIncrementorExpressionNodeEnter(class PreIncrementorExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitPreIncrementorExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitPreIncrementorExpressionNodeLeave(class PreIncrementorExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitPreIncrementorExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitPreDecrementorExpressionNodeEnter(class PreDecrementorExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitPreDecrementorExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitPreDecrementorExpressionNodeLeave(class PreDecrementorExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitPreDecrementorExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitUnaryPlusExpressionNodeEnter(class UnaryPlusExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitUnaryPlusExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitUnaryPlusExpressionNodeLeave(class UnaryPlusExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitUnaryPlusExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitUnaryNegateExpressionNodeEnter(class UnaryNegateExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitUnaryNegateExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitUnaryNegateExpressionNodeLeave(class UnaryNegateExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitUnaryNegateExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitBitwiseNotExpressionNodeEnter(class BitwiseNotExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitBitwiseNotExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitBitwiseNotExpressionNodeLeave(class BitwiseNotExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitBitwiseNotExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitLogicalNotExpressionNodeEnter(class LogicalNotExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitLogicalNotExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitLogicalNotExpressionNodeLeave(class LogicalNotExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitLogicalNotExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitPostIncrementorNodeEnter(class PostIncrementorNode *inNode)
{
	PrintTrace("Visit", "VisitPostIncrementorNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitPostIncrementorNodeLeave(class PostIncrementorNode *inNode)
{
	PrintTrace("Visit", "VisitPostIncrementorNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitPostDecrementorNodeEnter(class PostDecrementorNode *inNode)
{
	PrintTrace("Visit", "VisitPostDecrementorNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitPostDecrementorNodeLeave(class PostDecrementorNode *inNode)
{
	PrintTrace("Visit", "VisitPostDecrementorNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitBinaryExpressionNodeEnter(class BinaryExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitBinaryExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitBinaryExpressionNodeLeave(class BinaryExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitBinaryExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitAdditionExpressionNodeEnter(class AdditionExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAdditionExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitAdditionExpressionNodeLeave(class AdditionExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAdditionExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitSubtractionExpressionNodeEnter(class SubtractionExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitSubtractionExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitSubtractionExpressionNodeLeave(class SubtractionExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitSubtractionExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitMultiplicationExpressionNodeEnter(class MultiplicationExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitMultiplicationExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitMultiplicationExpressionNodeLeave(class MultiplicationExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitMultiplicationExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitDivisionExpressionNodeEnter(class DivisionExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitDivisionExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitDivisionExpressionNodeLeave(class DivisionExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitDivisionExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitLeftShiftExpressionNodeEnter(class LeftShiftExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitLeftShiftExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitLeftShiftExpressionNodeLeave(class LeftShiftExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitLeftShiftExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitSignedRightShiftExpressionNodeEnter(class SignedRightShiftExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitSignedRightShiftExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitSignedRightShiftExpressionNodeLeave(class SignedRightShiftExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitSignedRightShiftExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitUnsignedRightShiftExpressionNodeEnter(class UnsignedRightShiftExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitUnsignedRightShiftExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitUnsignedRightShiftExpressionNodeLeave(class UnsignedRightShiftExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitUnsignedRightShiftExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitInExpressionNodeEnter(class InExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitInExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitInExpressionNodeLeave(class InExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitInExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitLessThanExpressionNodeEnter(class LessThanExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitLessThanExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitLessThanExpressionNodeLeave(class LessThanExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitLessThanExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitGreaterThanExpressionNodeEnter(class GreaterThanExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitGreaterThanExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitGreaterThanExpressionNodeLeave(class GreaterThanExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitGreaterThanExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitLessThanOrEqualToExpressionNodeEnter(class LessThanOrEqualToExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitLessThanOrEqualToExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitLessThanOrEqualToExpressionNodeLeave(class LessThanOrEqualToExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitLessThanOrEqualToExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitGreaterThanOrEqualToExpressionNodeEnter(class GreaterThanOrEqualToExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitGreaterThanOrEqualToExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitGreaterThanOrEqualToExpressionNodeLeave(class GreaterThanOrEqualToExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitGreaterThanOrEqualToExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitInstanceOfExpressionNodeEnter(class InstanceOfExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitInstanceOfExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitInstanceOfExpressionNodeLeave(class InstanceOfExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitInstanceOfExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitEqualityExpressionNodeEnter(class EqualityExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitEqualityExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitEqualityExpressionNodeLeave(class EqualityExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitEqualityExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitNonEqualityExpressionNodeEnter(class NonEqualityExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitNonEqualityExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitNonEqualityExpressionNodeLeave(class NonEqualityExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitNonEqualityExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitStrictEqualityExpressionNodeEnter(class StrictEqualityExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitStrictEqualityExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitStrictEqualityExpressionNodeLeave(class StrictEqualityExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitStrictEqualityExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitStrictNonEqualityExpressionNodeEnter(class StrictNonEqualityExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitStrictNonEqualityExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitStrictNonEqualityExpressionNodeLeave(class StrictNonEqualityExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitStrictNonEqualityExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitBitwiseAndExpressionNodeEnter(class BitwiseAndExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitBitwiseAndExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitBitwiseAndExpressionNodeLeave(class BitwiseAndExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitBitwiseAndExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitBitwiseXorExpressionNodeEnter(class BitwiseXorExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitBitwiseXorExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitBitwiseXorExpressionNodeLeave(class BitwiseXorExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitBitwiseXorExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitBitwiseOrExpressionNodeEnter(class BitwiseOrExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitBitwiseOrExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitBitwiseOrExpressionNodeLeave(class BitwiseOrExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitBitwiseOrExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitLogicalAndExpressionNodeEnter(class LogicalAndExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitLogicalAndExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitLogicalAndExpressionNodeLeave(class LogicalAndExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitLogicalAndExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitLogicalOrExpressionNodeEnter(class LogicalOrExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitLogicalOrExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitLogicalOrExpressionNodeLeave(class LogicalOrExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitLogicalOrExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitModulusExpressionNodeEnter(class ModulusExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitModulusExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitModulusExpressionNodeLeave(class ModulusExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitModulusExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitAssignMultiplyExpressionNodeEnter(class AssignMultiplyExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignMultiplyExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitAssignMultiplyExpressionNodeLeave(class AssignMultiplyExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignMultiplyExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitAssignAddExpressionNodeEnter(class AssignAddExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignAddExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitAssignAddExpressionNodeLeave(class AssignAddExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignAddExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitAssignSubtractExpressionNodeEnter(class AssignSubtractExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignSubtractExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitAssignSubtractExpressionNodeLeave(class AssignSubtractExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignSubtractExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitAssignDivideExpressionNodeEnter(class AssignDivideExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignDivideExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitAssignDivideExpressionNodeLeave(class AssignDivideExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignDivideExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitAssignModulusExpressionNodeEnter(class AssignModulusExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignModulusExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitAssignModulusExpressionNodeLeave(class AssignModulusExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignModulusExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitAssignLeftShiftExpressionNodeEnter(class AssignLeftShiftExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignLeftShiftExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitAssignLeftShiftExpressionNodeLeave(class AssignLeftShiftExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignLeftShiftExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitAssignSignedRightShiftExpressionNodeEnter(class AssignSignedRightShiftExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignSignedRightShiftExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitAssignSignedRightShiftExpressionNodeLeave(class AssignSignedRightShiftExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignSignedRightShiftExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitAssignUnsignedRightShiftExpressionNodeEnter(class AssignUnsignedRightShiftExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignUnsignedRightShiftExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitAssignUnsignedRightShiftExpressionNodeLeave(class AssignUnsignedRightShiftExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignUnsignedRightShiftExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitAssignBitwiseAndExpressionNodeEnter(class AssignBitwiseAndExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignBitwiseAndExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitAssignBitwiseAndExpressionNodeLeave(class AssignBitwiseAndExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignBitwiseAndExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitAssignBitwiseOrExpressionNodeEnter(class AssignBitwiseOrExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignBitwiseOrExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitAssignBitwiseOrExpressionNodeLeave(class AssignBitwiseOrExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignBitwiseOrExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitAssignBitwiseXorExpressionNodeEnter(class AssignBitwiseXorExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignBitwiseXorExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitAssignBitwiseXorExpressionNodeLeave(class AssignBitwiseXorExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignBitwiseXorExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitAssignExpressionNodeEnter(class AssignExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitAssignExpressionNodeLeave(class AssignExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitAssignmentExpressionNodeEnter(class AssignmentExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignmentExpressionNodeEnter");

	AddAssignmentDeclarationToScope(inNode->GetDeclaration());

	return true;
}

bool NextGenJsAstWalker::VisitAssignmentExpressionNodeLeave(class AssignmentExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitAssignmentExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitCommaExpressionNodeEnter(class CommaExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitCommaExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitCommaExpressionNodeLeave(class CommaExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitCommaExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitConditionalExpressionNodeEnter(class ConditionalExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitConditionalExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitConditionalExpressionNodeLeave(class ConditionalExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitConditionalExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitFunctionCallExpressionNodeEnter(class FunctionCallExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitFunctionCallExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitFunctionCallExpressionNodeLeave(class FunctionCallExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitFunctionCallExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitNewNodeEnter(class NewNode *inNode)
{
	PrintTrace("Visit", "VisitNewNodeEnter");
	
	AddNewDeclarationToScope( inNode->GetDeclaration() );
	
	return true;
}

bool NextGenJsAstWalker::VisitNewNodeLeave(class NewNode *inNode)
{
	PrintTrace("Visit", "VisitNewNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitArrayExpressionNodeEnter(class ArrayExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitArrayExpressionNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitArrayExpressionNodeLeave(class ArrayExpressionNode *inNode)
{
	PrintTrace("Visit", "VisitArrayExpressionNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitThisNodeEnter(class ThisNode *inNode)
{
	PrintTrace("Visit", "VisitThisNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitThisNodeLeave(class ThisNode *inNode)
{
	PrintTrace("Visit", "VisitThisNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitIdentifierNodeEnter(class IdentifierNode *inNode)
{
	PrintTrace("Visit", "VisitIdentifierNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitIdentifierNodeLeave(class IdentifierNode *inNode)
{
	PrintTrace("Visit", "VisitIdentifierNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitArrayLiteralNodeEnter(class ArrayLiteralNode *inNode)
{
	PrintTrace("Visit", "VisitArrayLiteralNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitArrayLiteralNodeLeave(class ArrayLiteralNode *inNode)
{
	PrintTrace("Visit", "VisitArrayLiteralNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitNullLiteralNodeEnter(class NullLiteralNode *inNode)
{
	PrintTrace("Visit", "VisitNullLiteralNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitNullLiteralNodeLeave(class NullLiteralNode *inNode)
{
	PrintTrace("Visit", "VisitNullLiteralNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitStringLiteralNodeEnter(class StringLiteralNode *inNode)
{
	PrintTrace("Visit", "VisitStringLiteralNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitStringLiteralNodeLeave(class StringLiteralNode *inNode)
{
	PrintTrace("Visit", "VisitStringLiteralNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitNumericLiteralNodeEnter(class NumericLiteralNode *inNode)
{
	PrintTrace("Visit", "VisitNumericLiteralNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitNumericLiteralNodeLeave(class NumericLiteralNode *inNode)
{
	PrintTrace("Visit", "VisitNumericLiteralNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitBooleanLiteralNodeEnter(class BooleanLiteralNode *inNode)
{
	PrintTrace("Visit", "VisitBooleanLiteralNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitBooleanLiteralNodeLeave(class BooleanLiteralNode *inNode)
{
	PrintTrace("Visit", "VisitBooleanLiteralNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitRegExLiteralNodeEnter(class RegExLiteralNode *inNode)
{
	PrintTrace("Visit", "VisitRegExLiteralNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitRegExLiteralNodeLeave(class RegExLiteralNode *inNode)
{
	PrintTrace("Visit", "VisitRegExLiteralNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitStatementNodeEnter(class StatementNode *inNode)
{
	PrintTrace("Visit", "VisitStatementNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitStatementNodeLeave(class StatementNode *inNode)
{
	PrintTrace("Visit", "VisitStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitEmptyStatementNodeEnter(class EmptyStatementNode *inNode)
{
	PrintTrace("Visit", "VisitEmptyStatementNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitEmptyStatementNodeLeave(class EmptyStatementNode *inNode)
{
	PrintTrace("Visit", "VisitEmptyStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitLabeledStatementNodeEnter(class LabeledStatementNode *inNode)
{
	PrintTrace("Visit", "VisitLabeledStatementNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitLabeledStatementNodeLeave(class LabeledStatementNode *inNode)
{
	PrintTrace("Visit", "VisitLabeledStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitIfStatementNodeEnter(class IfStatementNode *inNode)
{
	PrintTrace("Visit", "VisitIfStatementNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitIfStatementNodeLeave(class IfStatementNode *inNode)
{
	PrintTrace("Visit", "VisitIfStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitContinueStatementNodeEnter(class ContinueStatementNode *inNode)
{
	PrintTrace("Visit", "VisitContinueStatementNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitContinueStatementNodeLeave(class ContinueStatementNode *inNode)
{
	PrintTrace("Visit", "VisitContinueStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitBreakStatementNodeEnter(class BreakStatementNode *inNode)
{
	PrintTrace("Visit", "VisitBreakStatementNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitBreakStatementNodeLeave(class BreakStatementNode *inNode)
{
	PrintTrace("Visit", "VisitBreakStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitDebuggerStatementNodeEnter(class DebuggerStatementNode *inNode)
{
	PrintTrace("Visit", "VisitDebuggerStatementNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitDebuggerStatementNodeLeave(class DebuggerStatementNode *inNode)
{
	PrintTrace("Visit", "VisitDebuggerStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitReturnStatementNodeEnter(class ReturnStatementNode *inNode)
{
	PrintTrace("Visit", "VisitReturnStatementNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitReturnStatementNodeLeave(class ReturnStatementNode *inNode)
{
	PrintTrace("Visit", "VisitReturnStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitThrowStatementNodeEnter(class ThrowStatementNode *inNode)
{
	PrintTrace("Visit", "VisitThrowStatementNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitThrowStatementNodeLeave(class ThrowStatementNode *inNode)
{
	PrintTrace("Visit", "VisitThrowStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitTryStatementNodeEnter(class TryStatementNode *inNode)
{
	PrintTrace("Visit", "VisitTryStatementNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitTryStatementNodeLeave(class TryStatementNode *inNode)
{
	PrintTrace("Visit", "VisitTryStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitSwitchStatementNodeEnter(class SwitchStatementNode *inNode)
{
	PrintTrace("Visit", "VisitSwitchStatementNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitSwitchStatementNodeLeave(class SwitchStatementNode *inNode)
{
	PrintTrace("Visit", "VisitSwitchStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitCaseClauseNodeEnter(class CaseClauseNode *inNode)
{
	PrintTrace("Visit", "VisitCaseClauseNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitCaseClauseNodeLeave(class CaseClauseNode *inNode)
{
	PrintTrace("Visit", "VisitCaseClauseNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitDefaultClauseNodeEnter(class DefaultClauseNode *inNode)
{
	PrintTrace("Visit", "VisitDefaultClauseNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitDefaultClauseNodeLeave(class DefaultClauseNode *inNode)
{
	PrintTrace("Visit", "VisitDefaultClauseNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitDoStatementNodeEnter(class DoStatementNode *inNode)
{
	PrintTrace("Visit", "VisitDoStatementNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitDoStatementNodeLeave(class DoStatementNode *inNode)
{
	PrintTrace("Visit", "VisitDoStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitWhileStatementNodeEnter(class WhileStatementNode *inNode)
{
	PrintTrace("Visit", "VisitWhileStatementNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitWhileStatementNodeLeave(class WhileStatementNode *inNode)
{
	PrintTrace("Visit", "VisitWhileStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitForStatementNodeEnter(class ForStatementNode *inNode)
{
	PrintTrace("Visit", "VisitForStatementNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitForStatementNodeLeave(class ForStatementNode *inNode)
{
	PrintTrace("Visit", "VisitForStatementNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitForExpressionTriClauseNodeEnter(class ForExpressionTriClauseNode *inNode)
{
	PrintTrace("Visit", "VisitForExpressionTriClauseNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitForExpressionTriClauseNodeLeave(class ForExpressionTriClauseNode *inNode)
{
	PrintTrace("Visit", "VisitForExpressionTriClauseNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitForExpressionInClauseNodeEnter(class ForExpressionInClauseNode *inNode)
{
	PrintTrace("Visit", "VisitForExpressionInClauseNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitForExpressionInClauseNodeLeave(class ForExpressionInClauseNode *inNode)
{
	PrintTrace("Visit", "VisitForExpressionInClauseNodeLeave");
	return true;
}




/*
 *
 *
 */
bool NextGenJsAstWalker::VisitStatementListNodeEnter(class StatementList *inNode)
{
	PrintTrace("Visit", "VisitStatementListNodeEnter");
	return true;
}

bool NextGenJsAstWalker::VisitStatementListNodeLeave(class StatementList *inNode)
{
	PrintTrace("Visit", "VisitStatementListNodeLeave");
	return true;
}


























































/*
 *
 *
 *
 */
Visitor *Visitor::GetDeclarationParser( ISymbolTable *inSymTable, Symbols::IFile *inOwningFile, JavaScriptParserDelegate *inDelegate )
{
#if ACTIVATE_NEXT_JS_AST_WALKER
	return new NextGenJsAstWalker();
#else
	return new DeclarationVisitor( inSymTable, inOwningFile, inDelegate );
#endif
}
