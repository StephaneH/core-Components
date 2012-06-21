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
#ifndef _JAVA_SCRIPT_AST_H_
#define _JAVA_SCRIPT_AST_H_

namespace JavaScriptError {
	typedef enum Code {
		kNoError,
		kSyntaxError,
		kExpectedLiteral,
		kExpectedIdentifier,
		kIllegalLHS,
	} Code;
}

class JavaScriptParserDelegate {
public:
	virtual void Error( JavaScriptError::Code code, sLONG line, sLONG offset, void *cookie ) = 0;
	virtual void BlockOpener( sLONG line, sLONG offset, void *cookie ) = 0;
	virtual void BlockCloser( sLONG line, sLONG offset, void *cookie ) = 0;
	virtual Symbols::ISymbol *GetDeclarationContext( void *cookie ) = 0;
};

namespace JavaScriptAST {

	#define GLOBAL_CONTEXT_SERVER_CLASS		CVSTR("Application")
	#define GLOBAL_CONTEXT_SERVER_INSTANCE	CVSTR("application")

	// This is the base Visitor class used for performing analysis on the AST.  You should implement
	// a subclass of the visitor to perform actual operations.
	class Visitor {
	public:
		static Visitor *GetDeclarationParser( ISymbolTable *inSymTable, Symbols::IFile *inOwningFile, JavaScriptParserDelegate *inDelegate = NULL, void *inDelegateCookie = NULL );
//		static Visitor *GetCallGraphWalker();	// See information in .cpp file for why this is commented out

	protected:
		Visitor() { }

	public:
		virtual ~Visitor() {}

		virtual bool VisitFunctionExpressionNodeEnter( class FunctionExpressionNode *inNode ) { return true; }
		virtual bool VisitFunctionExpressionNodeLeave( class FunctionExpressionNode *inNode ) { return true; }
		virtual bool VisitFunctionDeclarationStatementNodeEnter( class FunctionDeclarationStatementNode *inNode ) { return true; }
		virtual bool VisitFunctionDeclarationStatementNodeLeave( class FunctionDeclarationStatementNode *inNode ) { return true; }
		virtual bool VisitVariableDeclarationStatementNodeEnter( class VariableDeclarationStatementNode *inNode ) { return true; }
		virtual bool VisitVariableDeclarationStatementNodeLeave( class VariableDeclarationStatementNode *inNode ) { return true; }
		virtual bool VisitVariableDeclarationListStatementNodeEnter( class VariableDeclarationListStatementNode *inNode ) { return true; }
		virtual bool VisitVariableDeclarationListStatementNodeLeave( class VariableDeclarationListStatementNode *inNode ) { return true; }
		virtual bool VisitObjectLiteralNodeEnter( class ObjectLiteralNode *inNode ) { return true; }
		virtual bool VisitObjectLiteralNodeLeave( class ObjectLiteralNode *inNode ) { return true; }
		virtual bool VisitObjectLiteralFieldNodeEnter( class ObjectLiteralFieldNode *inNode ) { return true; }
		virtual bool VisitObjectLiteralFieldNodeLeave( class ObjectLiteralFieldNode *inNode ) { return true; }
		virtual bool VisitCatchStatementNodeEnter( class CatchStatementNode *inNode ) { return true; }
		virtual bool VisitCatchStatementNodeLeave( class CatchStatementNode *inNode ) { return true; }
		virtual bool VisitDotExpressionNodeEnter( class DotExpressionNode *inNode ) { return true; }
		virtual bool VisitDotExpressionNodeLeave( class DotExpressionNode *inNode ) { return true; }

		virtual bool VisitProgramNodeEnter( class ProgramNode *inNode ) { return true; }
		virtual bool VisitProgramNodeLeave( class ProgramNode *inNode ) { return true; }
		virtual bool VisitFunctionDeclarationArgumentsNode( class FunctionDeclarationArgumentsNode *inNode ) { return true; }
		virtual bool VisitFunctionCallArgumentsNode( class FunctionCallArgumentsNode *inNode ) { return true; }
		virtual bool VisitLeftHandSideExpressionNode( class LeftHandSideExpressionNode *inNode ) { return true; }
		virtual bool VisitUnaryExpressionNode( class UnaryExpressionNode *inNode ) { return true; }
		virtual bool VisitDeleteExpressionNode( class DeleteExpressionNode *inNode ) { return true; }
		virtual bool VisitVoidExpressionNode( class VoidExpressionNode *inNode ) { return true; }
		virtual bool VisitTypeOfExpressionNode( class TypeOfExpressionNode *inNode ) { return true; }
		virtual bool VisitPreIncrementorExpressionNode( class PreIncrementorExpressionNode *inNode ) { return true; }
		virtual bool VisitPreDecrementorExpressionNode( class PreDecrementorExpressionNode *inNode ) { return true; }
		virtual bool VisitUnaryPlusExpressionNode( class UnaryPlusExpressionNode *inNode ) { return true; }
		virtual bool VisitUnaryNegateExpressionNode( class UnaryNegateExpressionNode *inNode ) { return true; }
		virtual bool VisitBitwiseNotExpressionNode( class BitwiseNotExpressionNode *inNode ) { return true; }
		virtual bool VisitLogicalNotExpressionNode( class LogicalNotExpressionNode *inNode ) { return true; }
		virtual bool VisitPostIncrementorNode( class PostIncrementorNode *inNode ) { return true; }
		virtual bool VisitPostDecrementorNode( class PostDecrementorNode *inNode ) { return true; }
		virtual bool VisitBinaryExpressionNode( class BinaryExpressionNode *inNode ) { return true; }
		virtual bool VisitAdditionExpressionNode( class AdditionExpressionNode *inNode ) { return true; }
		virtual bool VisitSubtractionExpressionNode( class SubtractionExpressionNode *inNode ) { return true; }
		virtual bool VisitMultiplicationExpressionNode( class MultiplicationExpressionNode *inNode ) { return true; }
		virtual bool VisitDivisionExpressionNode( class DivisionExpressionNode *inNode ) { return true; }
		virtual bool VisitLeftShiftExpressionNode( class LeftShiftExpressionNode *inNode ) { return true; }
		virtual bool VisitSignedRightShiftExpressionNode( class SignedRightShiftExpressionNode *inNode ) { return true; }
		virtual bool VisitUnsignedRightShiftExpressionNode( class UnsignedRightShiftExpressionNode *inNode ) { return true; }
		virtual bool VisitInExpressionNode( class InExpressionNode *inNode ) { return true; }
		virtual bool VisitLessThanExpressionNode( class LessThanExpressionNode *inNode ) { return true; }
		virtual bool VisitGreaterThanExpressionNode( class GreaterThanExpressionNode *inNode ) { return true; }
		virtual bool VisitLessThanOrEqualToExpressionNode( class LessThanOrEqualToExpressionNode *inNode ) { return true; }
		virtual bool VisitGreaterThanOrEqualToExpressionNode( class GreaterThanOrEqualToExpressionNode *inNode ) { return true; }
		virtual bool VisitInstanceOfExpressionNode( class InstanceOfExpressionNode *inNode ) { return true; }
		virtual bool VisitEqualityExpressionNode( class EqualityExpressionNode *inNode ) { return true; }
		virtual bool VisitNonEqualityExpressionNode( class NonEqualityExpressionNode *inNode ) { return true; }
		virtual bool VisitStrictEqualityExpressionNode( class StrictEqualityExpressionNode *inNode ) { return true; }
		virtual bool VisitStrictNonEqualityExpressionNode( class StrictNonEqualityExpressionNode *inNode ) { return true; }
		virtual bool VisitBitwiseAndExpressionNode( class BitwiseAndExpressionNode *inNode ) { return true; }
		virtual bool VisitBitwiseXorExpressionNode( class BitwiseXorExpressionNode *inNode ) { return true; }
		virtual bool VisitBitwiseOrExpressionNode( class BitwiseOrExpressionNode *inNode ) { return true; }
		virtual bool VisitLogicalAndExpressionNode( class LogicalAndExpressionNode *inNode ) { return true; }
		virtual bool VisitLogicalOrExpressionNode( class LogicalOrExpressionNode *inNode ) { return true; }
		virtual bool VisitModulusExpressionNode( class ModulusExpressionNode *inNode ) { return true; }
		virtual bool VisitAssignMultiplyExpressionNode( class AssignMultiplyExpressionNode *inNode ) { return true; }
		virtual bool VisitAssignAddExpressionNode( class AssignAddExpressionNode *inNode ) { return true; }
		virtual bool VisitAssignSubtractExpressionNode( class AssignSubtractExpressionNode *inNode ) { return true; }
		virtual bool VisitAssignDivideExpressionNode( class AssignDivideExpressionNode *inNode ) { return true; }
		virtual bool VisitAssignModulusExpressionNode( class AssignModulusExpressionNode *inNode ) { return true; }
		virtual bool VisitAssignLeftShiftExpressionNode( class AssignLeftShiftExpressionNode *inNode ) { return true; }
		virtual bool VisitAssignSignedRightShiftExpressionNode( class AssignSignedRightShiftExpressionNode *inNode ) { return true; }
		virtual bool VisitAssignUnsignedRightShiftExpressionNode( class AssignUnsignedRightShiftExpressionNode *inNode ) { return true; }
		virtual bool VisitAssignBitwiseAndExpressionNode( class AssignBitwiseAndExpressionNode *inNode ) { return true; }
		virtual bool VisitAssignBitwiseOrExpressionNode( class AssignBitwiseOrExpressionNode *inNode ) { return true; }
		virtual bool VisitAssignBitwiseXorExpressionNode( class AssignBitwiseXorExpressionNode *inNode ) { return true; }
		virtual bool VisitAssignExpressionNode( class AssignExpressionNode *inNode ) { return true; }
		// The assignment expression node is a base class of all the assignment expressions.  If you want a simple
		// = expression node, then you want an AssignExpressionNode instead of an AssignmentExpressionNode
		virtual bool VisitAssignmentExpressionNode( class AssignmentExpressionNode *inNode ) { return true; }
		virtual bool VisitCommaExpressionNode( class CommaExpressionNode *inNode ) { return true; }
		virtual bool VisitConditionalExpressionNode( class ConditionalExpressionNode *inNode ) { return true; }
		virtual bool VisitFunctionCallExpressionNode( class FunctionCallExpressionNode *inNode ) { return true; }
		virtual bool VisitNewNode( class NewNode *inNode ) { return true; }
		virtual bool VisitArrayExpressionNode( class ArrayExpressionNode *inNode ) { return true; }
		virtual bool VisitThisNode( class ThisNode *inNode ) { return true; }
		virtual bool VisitIdentifierNode( class IdentifierNode *inNode ) { return true; }
		virtual bool VisitArrayLiteralNode( class ArrayLiteralNode *inNode ) { return true; }
		virtual bool VisitNullLiteralNode( class NullLiteralNode *inNode ) { return true; }
		virtual bool VisitStringLiteralNode( class StringLiteralNode *inNode ) { return true; }
		virtual bool VisitNumericLiteralNode( class NumericLiteralNode *inNode ) { return true; }
		virtual bool VisitBooleanLiteralNode( class BooleanLiteralNode *inNode ) { return true; }
		virtual bool VisitRegExLiteralNode( class RegExLiteralNode *inNode ) { return true; }
		virtual bool VisitStatementNode( class StatementNode *inNode ) { return true; }
		virtual bool VisitEmptyStatementNode( class EmptyStatementNode *inNode ) { return true; }
		virtual bool VisitLabeledStatementNode( class LabeledStatementNode *inNode ) { return true; }
		virtual bool VisitIfStatementNode( class IfStatementNode *inNode ) { return true; }
		virtual bool VisitContinueStatementNode( class ContinueStatementNode *inNode ) { return true; }
		virtual bool VisitBreakStatementNode( class BreakStatementNode *inNode ) { return true; }
		virtual bool VisitDebuggerStatementNode( class DebuggerStatementNode *inNode ) { return true; }
		virtual bool VisitReturnStatementNode( class ReturnStatementNode *inNode ) { return true; }
		virtual bool VisitWithStatementNode( class WithStatementNode *inNode ) { return true; }
		virtual bool VisitThrowStatementNode( class ThrowStatementNode *inNode ) { return true; }
		virtual bool VisitTryStatementNode( class TryStatementNode *inNode ) { return true; }
		virtual bool VisitSwitchStatementNode( class SwitchStatementNode *inNode ) { return true; }
		virtual bool VisitCaseClauseNode( class CaseClauseNode *inNode ) { return true; }
		virtual bool VisitDefaultClauseNode( class DefaultClauseNode *inNode ) { return true; }
		virtual bool VisitDoStatementNode( class DoStatementNode *inNode ) { return true; }
		virtual bool VisitWhileStatementNode( class WhileStatementNode *inNode ) { return true; }
		virtual bool VisitForStatementNode( class ForStatementNode *inNode ) { return true; }
		virtual bool VisitForExpressionTriClauseNode( class ForExpressionTriClauseNode *inNode ) { return true; }
		virtual bool VisitForExpressionInClauseNode( class ForExpressionInClauseNode *inNode ) { return true; }
		virtual bool VisitStatementList( class StatementList *inNode ) { return true; }
	};

	

	// This is an abstract class which all AST nodes must inherit from
	class Node {
	public:
		virtual ~Node()
		{
#if VERSIONDEBUG
			sNbNodes--;
#endif
			delete fScriptDoc;
		}

		void AttachScriptDocComment( ScriptDocComment *inComment ) { fScriptDoc = inComment; }
		ScriptDocComment *GetScriptDocComment() { return fScriptDoc; }

		virtual bool Accept( class Visitor *visit ) = 0;
		virtual bool IsLeftHandSideExpression() const { return false; }
		virtual bool IsLiteral() const { return false; }
		int GetLineNumber() { return fLineNumber; }
		int GetLineCompletion() { return ((-1 == fLineCompletion)?(fLineNumber):(fLineCompletion)); }

		XBOX::VString	GetWAFComment() { return fWAFComment; }
		void			AttachWAFComment(const XBOX::VString& inWAFComment)
		{ 
			fWAFComment = inWAFComment;
			if ( fWAFComment[fWAFComment.GetLength() - 1] == CHAR_CONTROL_000D)
				fWAFComment.SubString(1, fWAFComment.GetLength() - 1);

		}

	static void	DebugDump();

	protected:
		Node( int inLineNumber ) : fLineNumber( inLineNumber ), fLineCompletion( -1 ), fScriptDoc( NULL )
		{
#if VERSIONDEBUG
			sNbNodes++;
#endif
		
		}
		int fLineNumber, fLineCompletion;
		ScriptDocComment *fScriptDoc;
		XBOX::VString fWAFComment;

#if VERSIONDEBUG
		static	sLONG	sNbNodes;
#endif
	};



	class ProgramNode : public Node {
	public:
		ProgramNode( Node *inStatements ) : Node( -1 ), fStatements( inStatements ) {}
		virtual ~ProgramNode() {
			delete fStatements;
			}
		virtual bool Accept( class Visitor *visit ) {
			if (!visit->VisitProgramNodeEnter( this ))	return false;
			if (fStatements)	fStatements->Accept( visit );
			return visit->VisitProgramNodeLeave( this );
		}
		
	protected:
		Node *fStatements;
	};

	class FunctionCallArgumentsNode : public Node {
	public:
		FunctionCallArgumentsNode( int inLineNo ) : Node( inLineNo ) {}
		virtual ~FunctionCallArgumentsNode() {
			while (!fArgs.empty()) {
				delete fArgs.back();
				fArgs.pop_back();
			}
		}

		virtual bool Accept( class Visitor *visit ) { 
			if (!visit->VisitFunctionCallArgumentsNode( this ))	return false;
			for (std::vector< Node * >::iterator iter = fArgs.begin(); iter != fArgs.end(); ++iter)	{
				if (!(*iter)->Accept( visit ))	return false;
			}
			return true;
		}
		void AddArgument( Node *inNode ) { fArgs.push_back( inNode ); }

		size_t ArgumentCount() { return fArgs.size(); }
		Node *GetArgument( size_t inIndex ) { return fArgs[ inIndex ]; }

	protected:
		std::vector< Node * > fArgs;
	};

	class FunctionDeclarationArgumentsNode : public Node {
	public:
		FunctionDeclarationArgumentsNode( int inLineNo ) : Node( inLineNo ) {}
		virtual ~FunctionDeclarationArgumentsNode() {
			while (!fArgs.empty()) {
				delete fArgs.back();
				fArgs.pop_back();
			}
		}

		virtual bool Accept( class Visitor *visit ) { 
			if (!visit->VisitFunctionDeclarationArgumentsNode( this ))	return false;
			for (std::vector< Node * >::iterator iter = fArgs.begin(); iter != fArgs.end(); ++iter)	{
				if (!(*iter)->Accept( visit ))	return false;
			}
			return true;
		}
		void AddArgument( Node *inNode ) { fArgs.push_back( inNode ); }

		size_t ArgumentCount() { return fArgs.size(); }
		Node *GetArgument( size_t inIndex ) { return fArgs[ inIndex ]; }

	protected:
		std::vector< Node * > fArgs;
	};

	class LeftHandSideExpressionNode : public Node {
	public:
		virtual bool IsLeftHandSideExpression() const { return true; }
		virtual bool Accept( class Visitor *visit ) { return visit->VisitLeftHandSideExpressionNode( this ); }

	protected:
		LeftHandSideExpressionNode( int inLineNo ) : Node( inLineNo ) {}
	};

	class UnaryExpressionNode : public Node {
	protected:
		UnaryExpressionNode( Node *inLHS, int inLineNo ) : Node( inLineNo ), fLHS( inLHS ) {}
		virtual ~UnaryExpressionNode() { delete fLHS; }

		virtual bool Accept( class Visitor *visit ) { 
			if (fLHS) {
				return fLHS->Accept( visit ); 
			}
			return false;
		}

		Node *fLHS;
	};

	class DeleteExpressionNode : public UnaryExpressionNode {
	public:
		DeleteExpressionNode( Node *inLHS, int inLineNo ) : UnaryExpressionNode( inLHS, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitDeleteExpressionNode( this ) && UnaryExpressionNode::Accept( visit )); }
	};

	class VoidExpressionNode : public UnaryExpressionNode {
	public:
		VoidExpressionNode( Node *inLHS, int inLineNo ) : UnaryExpressionNode( inLHS, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitVoidExpressionNode( this ) && UnaryExpressionNode::Accept( visit )); }
	};

	class TypeOfExpressionNode : public UnaryExpressionNode {
	public:
		TypeOfExpressionNode( Node *inLHS, int inLineNo ) : UnaryExpressionNode( inLHS, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitTypeOfExpressionNode( this ) && UnaryExpressionNode::Accept( visit )); }
	};

	class PreIncrementorExpressionNode : public UnaryExpressionNode {
	public:
		PreIncrementorExpressionNode( Node *inLHS, int inLineNo ) : UnaryExpressionNode( inLHS, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitPreIncrementorExpressionNode( this ) && UnaryExpressionNode::Accept( visit )); }
	};

	class PreDecrementorExpressionNode : public UnaryExpressionNode {
	public:
		PreDecrementorExpressionNode( Node *inLHS, int inLineNo ) : UnaryExpressionNode( inLHS, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitPreDecrementorExpressionNode( this ) && UnaryExpressionNode::Accept( visit )); }
	};

	class UnaryPlusExpressionNode : public UnaryExpressionNode {
	public:
		UnaryPlusExpressionNode( Node *inLHS, int inLineNo ) : UnaryExpressionNode( inLHS, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitUnaryPlusExpressionNode( this ) && UnaryExpressionNode::Accept( visit )); }
	};

	class UnaryNegateExpressionNode : public UnaryExpressionNode {
	public:
		UnaryNegateExpressionNode( Node *inLHS, int inLineNo ) : UnaryExpressionNode( inLHS, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitUnaryNegateExpressionNode( this ) && UnaryExpressionNode::Accept( visit )); }
	};

	class BitwiseNotExpressionNode : public UnaryExpressionNode {
	public:
		BitwiseNotExpressionNode( Node *inLHS, int inLineNo ) : UnaryExpressionNode( inLHS, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitBitwiseNotExpressionNode( this ) && UnaryExpressionNode::Accept( visit )); }
	};

	class LogicalNotExpressionNode : public UnaryExpressionNode {
	public:
		LogicalNotExpressionNode( Node *inLHS, int inLineNo ) : UnaryExpressionNode( inLHS, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitLogicalNotExpressionNode( this ) && UnaryExpressionNode::Accept( visit )); }
	};

	class PostIncrementorNode : public UnaryExpressionNode {
	public:
		PostIncrementorNode( Node *inLHS, int inLineNo ) : UnaryExpressionNode( inLHS, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitPostIncrementorNode( this ) && UnaryExpressionNode::Accept( visit )); }
	};

	class PostDecrementorNode : public UnaryExpressionNode {
	public:
		PostDecrementorNode( Node *inLHS, int inLineNo ) : UnaryExpressionNode( inLHS, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitPostDecrementorNode( this ) && UnaryExpressionNode::Accept( visit )); }
	};

	class BinaryExpressionNode : public Node {
	public:
		Node *GetLHS() const { return fLHS; }
		Node *GetRHS() const { return fRHS; }

	protected:
		BinaryExpressionNode( Node *inLHS, Node *inRHS, int inLineNo ) : Node( inLineNo ), fLHS( inLHS ), fRHS( inRHS ) {}
		virtual ~BinaryExpressionNode() { 
			delete fLHS; 
			delete fRHS; 
		}

		virtual bool Accept( class Visitor *visit ) {
			if (fLHS && !fLHS->Accept( visit ))	return false;
			if (fRHS && !fRHS->Accept( visit ))	return false;
			return true;
		}

		Node *fLHS, *fRHS;
	};

	class AdditionExpressionNode : public BinaryExpressionNode {
	public:
		AdditionExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitAdditionExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class SubtractionExpressionNode : public BinaryExpressionNode {
	public:
		SubtractionExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitSubtractionExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class MultiplicationExpressionNode : public BinaryExpressionNode {
	public:
		MultiplicationExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitMultiplicationExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class DivisionExpressionNode : public BinaryExpressionNode {
	public:
		DivisionExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitDivisionExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class LeftShiftExpressionNode : public BinaryExpressionNode {
	public:
		LeftShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitLeftShiftExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class SignedRightShiftExpressionNode : public BinaryExpressionNode {
	public:
		SignedRightShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitSignedRightShiftExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class UnsignedRightShiftExpressionNode : public BinaryExpressionNode {
	public:
		UnsignedRightShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitUnsignedRightShiftExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class InExpressionNode : public BinaryExpressionNode {
	public:
		InExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitInExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class LessThanExpressionNode : public BinaryExpressionNode {
	public:
		LessThanExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitLessThanExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class GreaterThanExpressionNode : public BinaryExpressionNode {
	public:
		GreaterThanExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitGreaterThanExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class LessThanOrEqualToExpressionNode : public BinaryExpressionNode {
	public:
		LessThanOrEqualToExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitLessThanOrEqualToExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class GreaterThanOrEqualToExpressionNode : public BinaryExpressionNode {
	public:
		GreaterThanOrEqualToExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitGreaterThanOrEqualToExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class InstanceOfExpressionNode : public BinaryExpressionNode {
	public:
		InstanceOfExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitInstanceOfExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class EqualityExpressionNode : public BinaryExpressionNode {
	public:
		EqualityExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitEqualityExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class NonEqualityExpressionNode : public BinaryExpressionNode {
	public:
		NonEqualityExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitNonEqualityExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class StrictEqualityExpressionNode : public BinaryExpressionNode {
	public:
		StrictEqualityExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitStrictEqualityExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class StrictNonEqualityExpressionNode : public BinaryExpressionNode {
	public:
		StrictNonEqualityExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitStrictNonEqualityExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class BitwiseAndExpressionNode : public BinaryExpressionNode {
	public:
		BitwiseAndExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitBitwiseAndExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class BitwiseXorExpressionNode : public BinaryExpressionNode {
	public:
		BitwiseXorExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitBitwiseXorExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class BitwiseOrExpressionNode : public BinaryExpressionNode {
	public:
		BitwiseOrExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitBitwiseOrExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class LogicalAndExpressionNode : public BinaryExpressionNode {
	public:
		LogicalAndExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitLogicalAndExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class LogicalOrExpressionNode : public BinaryExpressionNode {
	public:
		LogicalOrExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitLogicalOrExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class ModulusExpressionNode : public BinaryExpressionNode {
	public:
		ModulusExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitModulusExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class AssignmentExpressionNode : public BinaryExpressionNode {
	protected:
		AssignmentExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitAssignmentExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class AssignMultiplyExpressionNode : public AssignmentExpressionNode {
	public:
		AssignMultiplyExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : AssignmentExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitAssignMultiplyExpressionNode( this ) && AssignmentExpressionNode::Accept( visit )); }
	};

	class AssignAddExpressionNode : public AssignmentExpressionNode {
	public:
		AssignAddExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : AssignmentExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitAssignAddExpressionNode( this ) && AssignmentExpressionNode::Accept( visit )); }
	};

	class AssignSubtractExpressionNode : public AssignmentExpressionNode {
	public:
		AssignSubtractExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : AssignmentExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitAssignSubtractExpressionNode( this ) && AssignmentExpressionNode::Accept( visit )); }
	};

	class AssignDivideExpressionNode : public AssignmentExpressionNode {
	public:
		AssignDivideExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : AssignmentExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitAssignDivideExpressionNode( this ) && AssignmentExpressionNode::Accept( visit )); }
	};

	class AssignModulusExpressionNode : public AssignmentExpressionNode {
	public:
		AssignModulusExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : AssignmentExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitAssignModulusExpressionNode( this ) && AssignmentExpressionNode::Accept( visit )); }
	};

	class AssignLeftShiftExpressionNode : public AssignmentExpressionNode {
	public:
		AssignLeftShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : AssignmentExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitAssignLeftShiftExpressionNode( this ) && AssignmentExpressionNode::Accept( visit )); }
	};

	class AssignSignedRightShiftExpressionNode : public AssignmentExpressionNode {
	public:
		AssignSignedRightShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : AssignmentExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitAssignSignedRightShiftExpressionNode( this ) && AssignmentExpressionNode::Accept( visit )); }
	};

	class AssignUnsignedRightShiftExpressionNode : public AssignmentExpressionNode {
	public:
		AssignUnsignedRightShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : AssignmentExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitAssignUnsignedRightShiftExpressionNode( this ) && AssignmentExpressionNode::Accept( visit )); }
	};

	class AssignBitwiseAndExpressionNode : public AssignmentExpressionNode {
	public:
		AssignBitwiseAndExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : AssignmentExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitAssignBitwiseAndExpressionNode( this ) && AssignmentExpressionNode::Accept( visit )); }
	};

	class AssignBitwiseOrExpressionNode : public AssignmentExpressionNode {
	public:
		AssignBitwiseOrExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : AssignmentExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitAssignBitwiseOrExpressionNode( this ) && AssignmentExpressionNode::Accept( visit )); }
	};

	class AssignBitwiseXorExpressionNode : public AssignmentExpressionNode {
	public:
		AssignBitwiseXorExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : AssignmentExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitAssignBitwiseXorExpressionNode( this ) && AssignmentExpressionNode::Accept( visit )); }
	};

	class AssignExpressionNode : public AssignmentExpressionNode {
	public:
		AssignExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : AssignmentExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitAssignExpressionNode( this ) && AssignmentExpressionNode::Accept( visit )); }
	};

	class CommaExpressionNode : public BinaryExpressionNode {
	public:
		CommaExpressionNode( Node *inLeft, Node *inRight, int inLineNo ) : BinaryExpressionNode( inLeft, inRight, inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return (visit->VisitCommaExpressionNode( this ) && BinaryExpressionNode::Accept( visit )); }
	};

	class ConditionalExpressionNode : public Node {
	public:
		ConditionalExpressionNode( Node *inExpression, Node *trueExpr, Node *falseExpr, int inLineNo ) : Node( inLineNo ), fExpr( inExpression ), fTrueExpr( trueExpr ), fFalseExpr( falseExpr ) {}
		virtual ~ConditionalExpressionNode() { delete fExpr; delete fTrueExpr; delete fFalseExpr; }

		virtual bool Accept( class Visitor *visit ) { 
			if (!visit->VisitConditionalExpressionNode( this ))	return false;
			if (fExpr && !fExpr->Accept( visit ))				return false;
			if (fTrueExpr && !fTrueExpr->Accept( visit ))		return false;
			if (fFalseExpr && !fFalseExpr->Accept(visit ))		return false;
			return true;
		}

	protected:
		Node *fExpr, *fTrueExpr, *fFalseExpr;
	};

	class MemberExpressionNode : public LeftHandSideExpressionNode {
	protected:
		MemberExpressionNode( int inLineNo ) : LeftHandSideExpressionNode( inLineNo) {}
	};

	class FunctionExpressionNode : public MemberExpressionNode {
	public:
		FunctionExpressionNode( VString inIdent, FunctionDeclarationArgumentsNode *inArgs, Node *inBody, int inLineNo, int inEndLineNo ) : MemberExpressionNode( inLineNo ), fIdentifier( inIdent ), fArgs( inArgs ), fBody( inBody ) { fLineCompletion = inEndLineNo;  }
		virtual ~FunctionExpressionNode() { delete fArgs; delete fBody; }

		virtual bool Accept( class Visitor *visit ) {
			if (visit->VisitFunctionExpressionNodeEnter( this )) {
				bool keepGoing = true;
				if (keepGoing && fArgs)	keepGoing = fArgs->Accept( visit );
				if (keepGoing && fBody)	keepGoing = fBody->Accept( visit );
			}
			return visit->VisitFunctionExpressionNodeLeave( this );
		}

		const VString&	GetIdentifier() const { return fIdentifier; }
		void			SetIdentifier(const VString& inIdentifier) { fIdentifier = inIdentifier; }
		FunctionDeclarationArgumentsNode *GetArgs() const { return fArgs; }

	protected:
		VString fIdentifier;
		FunctionDeclarationArgumentsNode *fArgs;
		Node *fBody;
	};

	class FunctionCallExpressionNode : public MemberExpressionNode {
	public:
		FunctionCallExpressionNode( Node *inLHS, FunctionCallArgumentsNode *inArgs, int inLineNo ) : MemberExpressionNode( inLineNo ), fLHS( inLHS ), fArgs( inArgs ) {}
		virtual ~FunctionCallExpressionNode() { delete fLHS; delete fArgs; }

		virtual bool Accept( class Visitor *visit ) { 
			if (!visit->VisitFunctionCallExpressionNode( this ))	return false;
			if (fLHS && !fLHS->Accept( visit ))	return false;
			if (fArgs && !fArgs->Accept( visit )) return false;
			return true;
		}

		Node *GetLHS() const { return fLHS; }

	protected:
		Node *fLHS;
		FunctionCallArgumentsNode *fArgs;
	};

	class NewNode : public MemberExpressionNode {
	public:
		NewNode( Node *inRhs, Node *inArgs, int inLineNo ) : MemberExpressionNode( inLineNo ), fRHS( inRhs ), fArgs( inArgs ) {}
		virtual ~NewNode() { delete fRHS; delete fArgs; }

		virtual bool Accept( class Visitor *visit ) { 
			if (!visit->VisitNewNode( this ))		return false;
			if (fRHS && !fRHS->Accept( visit ))		return false;
			if (fArgs && !fArgs->Accept( visit ))	return false;
			return true;
		}

		Node *GetOperand() const { return fRHS; }

	protected:
		Node *fRHS, *fArgs;
	};

	class ArrayExpressionNode : public MemberExpressionNode {
	public:
		ArrayExpressionNode( Node *inLhs, Node *inRhs, int inLineNo ) : MemberExpressionNode( inLineNo ), fLHS( inLhs ), fRHS( inRhs ) {}
		virtual ~ArrayExpressionNode() { delete fLHS; delete fRHS; }

		virtual bool Accept( class Visitor *visit ) { 
			if (!visit->VisitArrayExpressionNode( this ))	return false;
			if (fLHS && !fLHS->Accept( visit ))		return false;
			if (fRHS && !fRHS->Accept( visit ))		return false;
			return true;
		}

		Node *GetLHS() const { return fLHS; }
		Node *GetRHS() const { return fRHS; }

	protected:
		Node *fLHS;
		Node *fRHS;
	};

	class DotExpressionNode : public MemberExpressionNode {
	public:
		DotExpressionNode( Node *inLhs, Node *inRhs, int inLineNo ) : MemberExpressionNode( inLineNo ), fLHS( inLhs ), fRHS( inRhs ) {}
		virtual ~DotExpressionNode() { delete fLHS; delete fRHS; }

		virtual bool Accept( class Visitor *visit ) {
			if (visit->VisitDotExpressionNodeEnter( this )) {
				if (fLHS && !fLHS->Accept( visit ))		return false;
				if (fRHS && !fRHS->Accept( visit ))		return false;
			}
			return visit->VisitDotExpressionNodeLeave( this );
		}

		Node *GetLHS() const { return fLHS; }
		Node *GetRHS() const { return fRHS; }

	protected:
		Node *fLHS, *fRHS;
	};

	class PrimaryExpressionNode : public MemberExpressionNode {
	protected:
		PrimaryExpressionNode( int inLineNo ) : MemberExpressionNode( inLineNo ) {}
	};

	class ThisNode : public PrimaryExpressionNode {
	public:
		ThisNode( int inLineNo ) : PrimaryExpressionNode( inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return visit->VisitThisNode( this ); }
	};

	class IdentifierNode : public PrimaryExpressionNode {
	public:
		IdentifierNode( VString inIdent, int inLineNo ) : PrimaryExpressionNode( inLineNo ), fIdentifier( inIdent ) {}
		virtual bool Accept( class Visitor *visit ) { return visit->VisitIdentifierNode( this ); }
		
		VString GetText() const { return fIdentifier; }

	protected:
		VString fIdentifier;
	};

	// Represents all literals, but must be inheritted from by different
	// literal types
	class LiteralNode : public PrimaryExpressionNode {
	public:
		virtual bool IsLiteral() const { return true; }

	protected:
		LiteralNode( int inLineNo ) : PrimaryExpressionNode( inLineNo ) {}
	};

	class ArrayLiteralNode : public LiteralNode {
	public:
		ArrayLiteralNode( int inLineNo ) : LiteralNode( inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return visit->VisitArrayLiteralNode( this ); }
	};

	class NullLiteralNode : public LiteralNode {
	public:
		NullLiteralNode( int inLineNo ) : LiteralNode( inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return visit->VisitNullLiteralNode( this ); }
	};

	class StringLiteralNode : public LiteralNode {
	public:
		StringLiteralNode( VString inValue, int inLineNo ) : LiteralNode( inLineNo ), fValue( inValue ) {}
		virtual bool Accept( class Visitor *visit ) { return visit->VisitStringLiteralNode( this ); }

		VString GetValue() const { return fValue; }

	protected:
		VString fValue;
	};

	class NumericLiteralNode : public LiteralNode {
	public:
		NumericLiteralNode( const VString &inValue, int inLineNo ) : LiteralNode( inLineNo ), fValue( inValue ) {}
		virtual bool Accept( class Visitor *visit ) { return visit->VisitNumericLiteralNode( this ); }

		VString GetValue() const { return fValue; }

	protected:
		VString fValue;
	};

	class BooleanLiteralNode : public LiteralNode {
	public:
		BooleanLiteralNode( bool inValue, int inLineNo ) : LiteralNode( inLineNo ), fValue( inValue ) {}
		virtual bool Accept( class Visitor *visit ) { return visit->VisitBooleanLiteralNode( this ); }

		bool GetValue() const { return fValue; }

	protected:
		bool fValue;
	};

	class ObjectLiteralFieldNode : public Node {
	public:
		ObjectLiteralFieldNode( Node *inIdent, Node *inValue, int inLineNo ) : Node( inLineNo ), fIdent( inIdent ), fValue( inValue ) {}
		virtual ~ObjectLiteralFieldNode() { delete fIdent; delete fValue; }

		Node *GetIdentifier() { return fIdent; }
		Node *GetValue() { return fValue; }

		virtual bool Accept( class Visitor *visit ) {
			if (visit->VisitObjectLiteralFieldNodeEnter( this )) {
				bool keepGoing = true;
				if (keepGoing && fIdent)	keepGoing = fIdent->Accept( visit );
				if (keepGoing && fValue)	keepGoing = fValue->Accept( visit );
			}
			return visit->VisitObjectLiteralFieldNodeLeave( this );
		}

	protected:
		Node *fIdent, *fValue;
	};

	class ObjectLiteralNode : public LiteralNode {
	public:
		ObjectLiteralNode( int inLineNo ) : LiteralNode( inLineNo ) {}
		virtual ~ObjectLiteralNode() {
			while (!mFields.empty()) {
				delete mFields.back();
				mFields.pop_back();
			}
		}

		void AddField( ObjectLiteralFieldNode *inField ) { mFields.push_back( inField ); }

		virtual bool Accept( class Visitor *visit ) {
			if (visit->VisitObjectLiteralNodeEnter( this )) {
				for (std::vector< ObjectLiteralFieldNode * >::iterator iter = mFields.begin(); iter != mFields.end(); ++iter) {
					if (!(*iter)->Accept( visit ))	break;
				}
			}
			return visit->VisitObjectLiteralNodeLeave( this );
		}

	protected:
		std::vector< ObjectLiteralFieldNode * > mFields;
	};

	class RegExLiteralNode : public LiteralNode {
	public:
		RegExLiteralNode( VString inBody, VString inFlags, int inLineNo ) : LiteralNode( inLineNo ), fBody( inBody ), mFlags( inFlags ) {}
		virtual bool Accept( class Visitor *visit ) { return visit->VisitRegExLiteralNode( this ); }

		VString GetBody() const { return fBody; }
		VString GetFlags() const { return mFlags; }

	protected:
		VString fBody, mFlags;
	};

	class StatementNode : public Node {
	protected:
		StatementNode( int inLineNo ) : Node( inLineNo ) {}

		virtual bool Accept( class Visitor *visit ) { return visit->VisitStatementNode( this ); }

	};

	class EmptyStatementNode : public StatementNode {
	public:
		EmptyStatementNode( int inLineNo ) : StatementNode( inLineNo ) {}
		virtual bool Accept( class Visitor *visit ) { return StatementNode::Accept( visit ) && visit->VisitEmptyStatementNode( this ); }
	};

	class LabeledStatementNode : public StatementNode {
	public:
		LabeledStatementNode( VString inIdent, Node *inStatement, int inLineNo ) : StatementNode( inLineNo ), fIdent( inIdent ), fStatement( inStatement ) {}
		virtual ~LabeledStatementNode() { delete fStatement; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))				return false;
			if (!visit->VisitLabeledStatementNode( this ))	return false;
			if (fStatement && !fStatement->Accept( visit ))	return false;
			return true;
		}
		
	protected:
		Node *fStatement;
		VString fIdent;
	};

	class FunctionDeclarationStatementNode : public StatementNode {
	public:
		FunctionDeclarationStatementNode( VString inIdent, FunctionDeclarationArgumentsNode *inArgs, Node *inStatements, int inBeginLineNo, int inEndLineNo ) : StatementNode( inBeginLineNo ), fIdent( inIdent ), fArgs( inArgs ), fStatements( inStatements ) {  fLineCompletion = inEndLineNo; }

		virtual ~FunctionDeclarationStatementNode() { delete fArgs; delete fStatements; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))	return false;
			if (visit->VisitFunctionDeclarationStatementNodeEnter( this )) {
				bool keepGoing = true;
				if (keepGoing && fArgs)			keepGoing = fArgs->Accept( visit );
				if (keepGoing && fStatements)	keepGoing = fStatements->Accept( visit );
			}
			return visit->VisitFunctionDeclarationStatementNodeLeave( this );
		}

		const VString& GetIdentifier() const { return fIdent; }
		FunctionDeclarationArgumentsNode *GetArgs() const { return fArgs; }

	protected:
		FunctionDeclarationArgumentsNode *fArgs;
		Node *fStatements;
		VString fIdent;
	};

	class IfStatementNode : public StatementNode {
	public:
		IfStatementNode( Node *inExpr, Node *inTrueStatements, Node *inFalseStatements, int inLineNo ) : StatementNode( inLineNo ), fExpr( inExpr ), mTrueStatements( inTrueStatements ), mFalseStatements( inFalseStatements ) {}
		virtual ~IfStatementNode() { 
			delete fExpr; 
			delete mTrueStatements; 
			delete mFalseStatements; 
		}
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))			return false;
			if (!visit->VisitIfStatementNode( this ))	return false;
			if (fExpr && !fExpr->Accept( visit ))		return false;
			if (mTrueStatements && !mTrueStatements->Accept( visit ))	return false;
			if (mFalseStatements && !mFalseStatements->Accept( visit ))	return false;
			return true;
		}

	protected:
		Node *fExpr, *mTrueStatements, *mFalseStatements;
	};

	class ContinueStatementNode : public StatementNode {
	public:
		ContinueStatementNode( Node *inIdent, int inLineNo ) : StatementNode( inLineNo ), fIdent( inIdent ) {}
		virtual ~ContinueStatementNode() { delete fIdent; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))				return false;
			if (!visit->VisitContinueStatementNode( this ))	return false;
			if (fIdent && !fIdent->Accept( visit ))			return false;
			return true;
		}

	protected:
		Node *fIdent;
	};

	class BreakStatementNode : public StatementNode {
	public:
		BreakStatementNode( Node *inIdent, int inLineNo ) : StatementNode( inLineNo ), fIdent( inIdent ) {}
		virtual ~BreakStatementNode() { delete fIdent; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))				return false;
			if (!visit->VisitBreakStatementNode( this ))	return false;
			if (fIdent && !fIdent->Accept( visit ))			return false;
			return true;
		}

	protected:
		Node *fIdent;
	};

	class DebuggerStatementNode : public StatementNode {
	public:
		DebuggerStatementNode( int inLineNo ) : StatementNode( inLineNo ) {}
		virtual ~DebuggerStatementNode() { }
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))			return false;
			if (!visit->VisitDebuggerStatementNode( this ))	return false;
			return true;
		}
	};

	class ReturnStatementNode : public StatementNode {
	public:
		ReturnStatementNode( Node *inExpr, int inLineNo ) : StatementNode( inLineNo ), fExpr( inExpr ) {}
		virtual ~ReturnStatementNode() { delete fExpr; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))				return false;
			if (!visit->VisitReturnStatementNode( this ))	return false;
			if (fExpr && !fExpr->Accept( visit ))			return false;
			return true;
		}

		Node *GetExpression() const { return fExpr; }

	protected:
		Node *fExpr;
	};

	class WithStatementNode : public StatementNode {
	public:
		WithStatementNode( Node *inExpr, Node *inStatement, int inLineNo ) : StatementNode( inLineNo ), fExpr( inExpr ), fStatement( inStatement ) {}
		virtual ~WithStatementNode() { delete fExpr; delete fStatement; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))				return false;
			if (!visit->VisitWithStatementNode( this ))		return false;
			if (fExpr && !fExpr->Accept( visit ))			return false;
			if (fStatement && !fStatement->Accept( visit ))	return false;
			return true;
		}
		
	protected:
		Node *fExpr, *fStatement;
	};

	class ThrowStatementNode : public StatementNode {
	public:
		ThrowStatementNode( Node *inExpr, int inLineNo ) : StatementNode( inLineNo ), fExpr( inExpr ) {}
		virtual ~ThrowStatementNode() { delete fExpr; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))				return false;
			if (!visit->VisitThrowStatementNode( this ))	return false;
			if (fExpr && !fExpr->Accept( visit ))			return false;
			return true;
		}

	protected:
		Node *fExpr;
	};

	class CatchStatementNode : public Node {
	public:
		CatchStatementNode( Node *inIdent, Node *inStatements, int inLineNo ) : Node( inLineNo ), fIdent( inIdent ), fStatements( inStatements ) { fLineCompletion = inStatements ? inStatements->GetLineCompletion() : -1; }
		virtual ~CatchStatementNode() { delete fIdent; delete fStatements; }
		virtual bool Accept( class Visitor *visit ) { 
			if (visit->VisitCatchStatementNodeEnter( this )) {
				bool keepGoing = true;
				if (keepGoing && fIdent)		keepGoing = fIdent->Accept( visit );
				if (keepGoing && fStatements)	keepGoing = fStatements->Accept( visit );
			}
			return visit->VisitCatchStatementNodeLeave( this );
		}

		Node *GetStatements() { return fStatements; }
		Node *GetIdentifier() { return fIdent; }

	protected:
		Node *fIdent, *fStatements;
	};


	class TryStatementNode : public StatementNode {
	public:
		TryStatementNode( Node *inStatements, Node *inCatch, Node *inFinally, int inLineNo ) : StatementNode( inLineNo ), fStatements( inStatements ), mCatch( inCatch ), mFinally( inFinally ) {}
		virtual ~TryStatementNode() { delete fStatements; delete mCatch; delete mFinally; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))					return false;
			if (!visit->VisitTryStatementNode( this ))			return false;
			if (fStatements && !fStatements->Accept( visit ))	return false;
			if (mCatch && !mCatch->Accept( visit ))				return false;
			if (mFinally && !mFinally->Accept( visit ))			return false;
			return true;
		}

	protected:
		Node *fStatements, *mCatch, *mFinally;
	};

	class SwitchStatementNode : public StatementNode {
	public:
		SwitchStatementNode( Node *inExpr, int inLineNo ) : StatementNode( inLineNo ), fExpr( inExpr ) {}
		virtual ~SwitchStatementNode() { 
			delete fExpr; 
			while (!mCases.empty()) {
				delete mCases.back();
				mCases.pop_back();
			}
		}

		void AddCase( Node *inCase ) { mCases.push_back( inCase ); }
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))				return false;
			if (!visit->VisitSwitchStatementNode( this ))	return false;
			if (fExpr && !fExpr->Accept( visit ))			return false;
			for (std::vector< Node * >::iterator iter = mCases.begin(); iter != mCases.end(); ++iter) {
				if (!(*iter)->Accept( visit ))				return false;
			}
			return true;
		}

	protected:
		Node *fExpr;
		std::vector< Node * > mCases;
	};

	class CaseClauseNode : public Node {
	public:
		CaseClauseNode( Node *inExpr, Node *inStatement, int inLineNo ) : Node( inLineNo ), fExpr( inExpr ), fStatement( inStatement ) {}
		virtual ~CaseClauseNode() { delete fExpr; delete fStatement; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!visit->VisitCaseClauseNode( this ))	return false;
			if (fExpr && !fExpr->Accept( visit ))		return false;
			if (fStatement && !fStatement->Accept( visit ))	return false;
			return true;
		}

	protected:
		Node *fExpr, *fStatement;
	};

	class DefaultClauseNode : public Node {
	public:
		DefaultClauseNode( Node *inStatement, int inLineNo ) : Node( inLineNo ), fStatement( inStatement ) {}
		virtual ~DefaultClauseNode() { delete fStatement; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!visit->VisitDefaultClauseNode( this ))	return false;
			if (fStatement && !fStatement->Accept( visit ))	return false;
			return true;
		}

	protected:
		Node *fStatement;
	};

	class VariableDeclarationStatementNode : public StatementNode {
	public:
		VariableDeclarationStatementNode( Node *inIdent, Node *inAssignmentExpr, int inLineNo ) : StatementNode( inLineNo ), fIdent( inIdent ), mAssignment( inAssignmentExpr ) {}
		virtual ~VariableDeclarationStatementNode() { delete fIdent; delete mAssignment; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))	return false;
			if (visit->VisitVariableDeclarationStatementNodeEnter( this )) {
				bool keepGoing = true;
				if (keepGoing && fIdent)		keepGoing = fIdent->Accept( visit );
				if (keepGoing && mAssignment)	keepGoing = mAssignment->Accept( visit );
			}
			return visit->VisitVariableDeclarationStatementNodeLeave( this );
		}

		Node *GetIdentifier() { return fIdent; }
		Node *GetAssignment() { return mAssignment; }

	protected:
		Node *fIdent, *mAssignment;
	};

	class VariableDeclarationListStatementNode : public StatementNode {
	public:
		VariableDeclarationListStatementNode( VariableDeclarationStatementNode *inFirst, int inLineNo ) : StatementNode( inLineNo ) { AddDeclaration( inFirst ); }
		virtual ~VariableDeclarationListStatementNode() {
			while (!fDecls.empty()) {
				delete fDecls.back();
				fDecls.pop_back();
			}
		}

		void AddDeclaration( VariableDeclarationStatementNode *inDecl ) { fDecls.push_back( inDecl ); }
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))	return false;
			if (visit->VisitVariableDeclarationListStatementNodeEnter( this )) {
				for (std::vector< VariableDeclarationStatementNode * >::iterator iter = fDecls.begin(); iter != fDecls.end(); ++iter) {
					if (!(*iter)->Accept( visit ))	break;
				}
			}
			return visit->VisitVariableDeclarationListStatementNodeLeave( this );
		}

	protected:
		std::vector< VariableDeclarationStatementNode * > fDecls;
	};

	class DoStatementNode : public StatementNode {
	public:
		DoStatementNode( Node *inExpr, Node *inStatement, int inLineNo ) : StatementNode( inLineNo ), fExpr( inExpr ), fStatement( inStatement ) {}
		virtual ~DoStatementNode() { delete fExpr; delete fStatement; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))				return false;
			if (!visit->VisitDoStatementNode( this ))		return false;
			if (fExpr && !fExpr->Accept( visit ))			return false;
			if (fStatement && !fStatement->Accept( visit ))	return false;
			return true;
		}

	protected:
		Node *fExpr, *fStatement;
	};

	class WhileStatementNode : public StatementNode {
	public:
		WhileStatementNode( Node *inExpr, Node *inStatement, int inLineNo ) : StatementNode( inLineNo ), fExpr( inExpr ), fStatement( inStatement ) {}
		virtual ~WhileStatementNode() { delete fExpr; delete fStatement; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))				return false;
			if (!visit->VisitWhileStatementNode( this ))	return false;
			if (fExpr && !fExpr->Accept( visit ))			return false;
			if (fStatement && !fStatement->Accept( visit ))	return false;
			return true;
		}

	protected:
		Node *fExpr, *fStatement;
	};

	class ForStatementNode : public StatementNode {
	public:
		ForStatementNode( Node *inExpr, Node *inStatement, int inLineNo ) : StatementNode( inLineNo ), fExpr( inExpr ), fStatement( inStatement ) {}
		virtual ~ForStatementNode() { delete fExpr; delete fStatement; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))				return false;
			if (!visit->VisitForStatementNode( this ))		return false;
			if (fExpr && !fExpr->Accept( visit ))			return false;
			if (fStatement && !fStatement->Accept( visit ))	return false;
			return true;
		}

	protected:
		Node *fExpr, *fStatement;
	};

	class ForExpressionTriClauseNode : public Node {
	public:
		ForExpressionTriClauseNode( Node *inDecl, Node *inTest, Node *inLoop, int inLineNo ) : Node( inLineNo ), fDecl( inDecl ), fTest( inTest ), fLoop( inLoop ) {}
		virtual ~ForExpressionTriClauseNode() { delete fDecl; delete fTest; delete fLoop; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!visit->VisitForExpressionTriClauseNode( this ))	return false;
			if (fDecl && !fDecl->Accept( visit ))	return false;
			if (fTest && !fTest->Accept( visit ))	return false;
			if (fLoop && !fLoop->Accept( visit ))	return false;
			return true;
		}

	protected:
		Node *fDecl, *fTest, *fLoop;
	};

	class ForExpressionInClauseNode : public Node {
	public:
		ForExpressionInClauseNode( Node *inDecl, Node *inExpr, int inLineNo ) : Node( inLineNo ), fDecl( inDecl ), fExpr( inExpr ) {}
		virtual ~ForExpressionInClauseNode() { delete fDecl; delete fExpr; }
		virtual bool Accept( class Visitor *visit ) { 
			if (!visit->VisitForExpressionInClauseNode( this ))	return false;
			if (fDecl && !fDecl->Accept( visit ))	return false;
			if (fExpr && !fExpr->Accept( visit ))	return false;
			return true;
		}

		Node *GetDeclaration() const { return fDecl; }

	protected:
		Node *fDecl, *fExpr;
	};

	class StatementList : public StatementNode {
	public:
		StatementList( int inLineNo ) : StatementNode( inLineNo ) {}
		void AddStatement( Node *inStatement ) { 
			fStatementList.push_back( inStatement ); 
			fLineCompletion = XBOX::Max( fLineCompletion, inStatement->GetLineNumber() );
		}

		void SetListCompletionLine( int inLineNo ) { fLineCompletion = XBOX::Max( fLineCompletion, inLineNo ); }

		virtual ~StatementList() {
			while (!fStatementList.empty()) {
				delete fStatementList.back();
				fStatementList.pop_back();
			}
		}
		virtual bool Accept( class Visitor *visit ) { 
			if (!StatementNode::Accept( visit ))		return false;
			if (!visit->VisitStatementList( this ))		return false;
			for (std::vector< Node * >::iterator iter = fStatementList.begin(); iter != fStatementList.end(); ++iter) {
				(*iter)->Accept( visit );
				if (VTask::GetCurrent()->IsDying())		return false;
			}
			return true;
		}

	protected:
		std::vector< Node * > fStatementList;
	};
}

#endif // _JAVA_SCRIPT_AST_H_