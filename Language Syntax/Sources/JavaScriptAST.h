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

class JavaScriptParserDelegate {
public:
	virtual void Error( XBOX::VFilePath, sLONG inLine, sLONG inOffset, const XBOX::VString& inMessage ) = 0;
	virtual Symbols::ISymbol *GetDeclarationContext( ) = 0;
};

namespace JavaScriptAST {

	#define GLOBAL_CONTEXT_SERVER_CLASS		CVSTR("Application")
	#define GLOBAL_CONTEXT_SERVER_INSTANCE	CVSTR("application")

	// This is the base Visitor class used for performing analysis on the AST.  You should implement
	// a subclass of the visitor to perform actual operations.
	class Visitor {
	public:
		static Visitor *GetDeclarationParser( ISymbolTable *inSymTable, Symbols::IFile *inOwningFile, JavaScriptParserDelegate *inDelegate = NULL );

	protected:
		Visitor() { }

	public:
		virtual ~Visitor() {}

		virtual bool VisitFunctionExpressionNodeEnter					( class FunctionExpressionNode *inNode )					{ return true; }
		virtual bool VisitFunctionExpressionNodeLeave					( class FunctionExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitFunctionDeclarationStatementNodeEnter			( class FunctionDeclarationStatementNode *inNode )			{ return true; }
		virtual bool VisitFunctionDeclarationStatementNodeLeave			( class FunctionDeclarationStatementNode *inNode )			{ return true; }
		
		virtual bool VisitVariableDeclarationStatementNodeEnter			( class VariableDeclarationStatementNode *inNode )			{ return true; }
		virtual bool VisitVariableDeclarationStatementNodeLeave			( class VariableDeclarationStatementNode *inNode )			{ return true; }
		
		virtual bool VisitVariableDeclarationListStatementNodeEnter		( class VariableDeclarationListStatementNode *inNode )		{ return true; }
		virtual bool VisitVariableDeclarationListStatementNodeLeave		( class VariableDeclarationListStatementNode *inNode )		{ return true; }
		
		virtual bool VisitObjectLiteralNodeEnter						( class ObjectLiteralNode *inNode )							{ return true; }
		virtual bool VisitObjectLiteralNodeLeave						( class ObjectLiteralNode *inNode )							{ return true; }
		
		virtual bool VisitObjectLiteralFieldNodeEnter					( class ObjectLiteralFieldNode *inNode )					{ return true; }
		virtual bool VisitObjectLiteralFieldNodeLeave					( class ObjectLiteralFieldNode *inNode )					{ return true; }
		
		virtual bool VisitCatchStatementNodeEnter						( class CatchStatementNode *inNode )						{ return true; }
		virtual bool VisitCatchStatementNodeLeave						( class CatchStatementNode *inNode )						{ return true; }
		
		virtual bool VisitDotExpressionNodeEnter						( class DotExpressionNode *inNode )							{ return true; }
		virtual bool VisitDotExpressionNodeLeave						( class DotExpressionNode *inNode )							{ return true; }
		
		virtual bool VisitProgramNodeEnter								( class ProgramNode *inNode )								{ return true; }
		virtual bool VisitProgramNodeLeave								( class ProgramNode *inNode )								{ return true; }
		
		virtual bool VisitFunctionDeclarationArgumentsNode				( class FunctionDeclarationArgumentsNode *inNode )			{ return true; }
		
		virtual bool VisitFunctionCallArgumentsNode						( class FunctionCallArgumentsNode *inNode )					{ return true; }
		
		virtual bool VisitLeftHandSideExpressionNode					( class LeftHandSideExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitUnaryExpressionNode							( class UnaryExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitDeleteExpressionNode							( class DeleteExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitVoidExpressionNode							( class VoidExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitTypeOfExpressionNode							( class TypeOfExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitPreIncrementorExpressionNode					( class PreIncrementorExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitPreDecrementorExpressionNode					( class PreDecrementorExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitUnaryPlusExpressionNode						( class UnaryPlusExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitUnaryNegateExpressionNode						( class UnaryNegateExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitBitwiseNotExpressionNode						( class BitwiseNotExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitLogicalNotExpressionNode						( class LogicalNotExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitPostIncrementorNode							( class PostIncrementorNode *inNode )						{ return true; }
		
		virtual bool VisitPostDecrementorNode							( class PostDecrementorNode *inNode )						{ return true; }
		
		virtual bool VisitBinaryExpressionNode							( class BinaryExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitAdditionExpressionNode						( class AdditionExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitSubtractionExpressionNode						( class SubtractionExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitMultiplicationExpressionNode					( class MultiplicationExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitDivisionExpressionNode						( class DivisionExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitLeftShiftExpressionNode						( class LeftShiftExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitSignedRightShiftExpressionNode				( class SignedRightShiftExpressionNode *inNode )			{ return true; }
		
		virtual bool VisitUnsignedRightShiftExpressionNode				( class UnsignedRightShiftExpressionNode *inNode )			{ return true; }
		
		virtual bool VisitInExpressionNode								( class InExpressionNode *inNode )							{ return true; }
		
		virtual bool VisitLessThanExpressionNode						( class LessThanExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitGreaterThanExpressionNode						( class GreaterThanExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitLessThanOrEqualToExpressionNode				( class LessThanOrEqualToExpressionNode *inNode )			{ return true; }
		
		virtual bool VisitGreaterThanOrEqualToExpressionNode			( class GreaterThanOrEqualToExpressionNode *inNode )		{ return true; }
		
		virtual bool VisitInstanceOfExpressionNode						( class InstanceOfExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitEqualityExpressionNode						( class EqualityExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitNonEqualityExpressionNode						( class NonEqualityExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitStrictEqualityExpressionNode					( class StrictEqualityExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitStrictNonEqualityExpressionNode				( class StrictNonEqualityExpressionNode *inNode )			{ return true; }
		
		virtual bool VisitBitwiseAndExpressionNode						( class BitwiseAndExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitBitwiseXorExpressionNode						( class BitwiseXorExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitBitwiseOrExpressionNode						( class BitwiseOrExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitLogicalAndExpressionNode						( class LogicalAndExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitLogicalOrExpressionNode						( class LogicalOrExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitModulusExpressionNode							( class ModulusExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitAssignMultiplyExpressionNode					( class AssignMultiplyExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitAssignAddExpressionNode						( class AssignAddExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitAssignSubtractExpressionNode					( class AssignSubtractExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitAssignDivideExpressionNode					( class AssignDivideExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitAssignModulusExpressionNode					( class AssignModulusExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitAssignLeftShiftExpressionNode					( class AssignLeftShiftExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitAssignSignedRightShiftExpressionNode			( class AssignSignedRightShiftExpressionNode *inNode )		{ return true; }
		
		virtual bool VisitAssignUnsignedRightShiftExpressionNode		( class AssignUnsignedRightShiftExpressionNode *inNode )	{ return true; }
		
		virtual bool VisitAssignBitwiseAndExpressionNode				( class AssignBitwiseAndExpressionNode *inNode )			{ return true; }
		
		virtual bool VisitAssignBitwiseOrExpressionNode					( class AssignBitwiseOrExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitAssignBitwiseXorExpressionNode				( class AssignBitwiseXorExpressionNode *inNode )			{ return true; }
		
		virtual bool VisitAssignExpressionNode							( class AssignExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitAssignmentExpressionNode						( class AssignmentExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitCommaExpressionNode							( class CommaExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitConditionalExpressionNode						( class ConditionalExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitFunctionCallExpressionNode					( class FunctionCallExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitNewNode										( class NewNode *inNode )									{ return true; }
		
		virtual bool VisitArrayExpressionNode							( class ArrayExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitThisNode										( class ThisNode *inNode )									{ return true; }
		
		virtual bool VisitIdentifierNode								( class IdentifierNode *inNode )							{ return true; }
		
		virtual bool VisitArrayLiteralNode								( class ArrayLiteralNode *inNode )							{ return true; }
		
		virtual bool VisitNullLiteralNode								( class NullLiteralNode *inNode )							{ return true; }
		
		virtual bool VisitStringLiteralNode								( class StringLiteralNode *inNode )							{ return true; }
		
		virtual bool VisitNumericLiteralNode							( class NumericLiteralNode *inNode )						{ return true; }
		
		virtual bool VisitBooleanLiteralNode							( class BooleanLiteralNode *inNode )						{ return true; }
		
		virtual bool VisitRegExLiteralNode								( class RegExLiteralNode *inNode )							{ return true; }
		
		virtual bool VisitStatementNode									( class StatementNode *inNode )								{ return true; }
		
		virtual bool VisitEmptyStatementNode							( class EmptyStatementNode *inNode )						{ return true; }
		
		virtual bool VisitLabeledStatementNode							( class LabeledStatementNode *inNode )						{ return true; }
		
		virtual bool VisitIfStatementNode								( class IfStatementNode *inNode )							{ return true; }
		
		virtual bool VisitContinueStatementNode							( class ContinueStatementNode *inNode )						{ return true; }
		
		virtual bool VisitBreakStatementNode							( class BreakStatementNode *inNode )						{ return true; }
		
		virtual bool VisitDebuggerStatementNode							( class DebuggerStatementNode *inNode )						{ return true; }
		
		virtual bool VisitReturnStatementNode							( class ReturnStatementNode *inNode )						{ return true; }
		
		virtual bool VisitWithStatementNode								( class WithStatementNode *inNode )							{ return true; }
		
		virtual bool VisitThrowStatementNode							( class ThrowStatementNode *inNode )						{ return true; }
		
		virtual bool VisitTryStatementNode								( class TryStatementNode *inNode )							{ return true; }
		
		virtual bool VisitSwitchStatementNode							( class SwitchStatementNode *inNode )						{ return true; }
		
		virtual bool VisitCaseClauseNode								( class CaseClauseNode *inNode )							{ return true; }
		
		virtual bool VisitDefaultClauseNode								( class DefaultClauseNode *inNode )							{ return true; }
		
		virtual bool VisitDoStatementNode								( class DoStatementNode *inNode )							{ return true; }
		
		virtual bool VisitWhileStatementNode							( class WhileStatementNode *inNode )						{ return true; }
		
		virtual bool VisitForStatementNode								( class ForStatementNode *inNode )							{ return true; }
		
		virtual bool VisitForExpressionTriClauseNode					( class ForExpressionTriClauseNode *inNode )				{ return true; }
		
		virtual bool VisitForExpressionInClauseNode						( class ForExpressionInClauseNode *inNode )					{ return true; }
		
		virtual bool VisitStatementList									( class StatementList *inNode )								{ return true; }
	};

	

	// This is an abstract class which all AST nodes must inherit from
	/*
	 *
	 *
	 *
	 */
	class Node
	{
	public:
		virtual ~Node();

		void AttachScriptDocComment( ScriptDocComment *inComment );
		ScriptDocComment *GetScriptDocComment();

		virtual bool Accept( class Visitor *visit ) = 0;
		virtual bool IsLeftHandSideExpression() const;
		virtual bool IsLiteral() const;
		int GetLineNumber();
		int GetLineCompletion();

		XBOX::VString	GetWAFComment();
		void			AttachWAFComment(const XBOX::VString& inWAFComment);
		
	protected:
		Node( int inLineNumber );
		
		int fLineNumber, fLineCompletion;
		ScriptDocComment *fScriptDoc;
		XBOX::VString fWAFComment;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class ProgramNode : public Node
	{
	public:
		ProgramNode( Node *inStatements );
		virtual ~ProgramNode();
		
		virtual bool Accept( class Visitor *visit );
		
	protected:
		Node *fStatements;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class FunctionCallArgumentsNode : public Node
	{
	public:
		FunctionCallArgumentsNode( int inLineNo );
		virtual ~FunctionCallArgumentsNode();

		virtual bool Accept( class Visitor *visit );
		void AddArgument( Node *inNode );

		size_t ArgumentCount();
		Node *GetArgument( size_t inIndex );

	protected:
		std::vector< Node * > fArgs;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class FunctionDeclarationArgumentsNode : public Node
	{
	public:
		FunctionDeclarationArgumentsNode( int inLineNo );
		virtual ~FunctionDeclarationArgumentsNode();

		virtual bool Accept( class Visitor *visit );
		void AddArgument( Node *inNode );

		size_t ArgumentCount();
		Node *GetArgument( size_t inIndex );

	protected:
		std::vector< Node * > fArgs;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class LeftHandSideExpressionNode : public Node
	{
	public:
		virtual bool IsLeftHandSideExpression() const;
		virtual bool Accept( class Visitor *visit );

	protected:
		LeftHandSideExpressionNode( int inLineNo );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class UnaryExpressionNode : public Node
	{
	protected:
		UnaryExpressionNode( Node *inLHS, int inLineNo );
		virtual ~UnaryExpressionNode();

		virtual bool Accept( class Visitor *visit );
		Node *fLHS;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class DeleteExpressionNode : public UnaryExpressionNode
	{
	public:
		DeleteExpressionNode( Node *inLHS, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class VoidExpressionNode : public UnaryExpressionNode
	{
	public:
		VoidExpressionNode( Node *inLHS, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class TypeOfExpressionNode : public UnaryExpressionNode
	{
	public:
		TypeOfExpressionNode( Node *inLHS, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class PreIncrementorExpressionNode : public UnaryExpressionNode
	{
	public:
		PreIncrementorExpressionNode( Node *inLHS, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class PreDecrementorExpressionNode : public UnaryExpressionNode
	{
	public:
		PreDecrementorExpressionNode( Node *inLHS, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class UnaryPlusExpressionNode : public UnaryExpressionNode
	{
	public:
		UnaryPlusExpressionNode( Node *inLHS, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class UnaryNegateExpressionNode : public UnaryExpressionNode
	{
	public:
		UnaryNegateExpressionNode( Node *inLHS, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class BitwiseNotExpressionNode : public UnaryExpressionNode
	{
	public:
		BitwiseNotExpressionNode( Node *inLHS, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class LogicalNotExpressionNode : public UnaryExpressionNode
	{
	public:
		LogicalNotExpressionNode( Node *inLHS, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class PostIncrementorNode : public UnaryExpressionNode
	{
	public:
		PostIncrementorNode( Node *inLHS, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class PostDecrementorNode : public UnaryExpressionNode
	{
	public:
		PostDecrementorNode( Node *inLHS, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class BinaryExpressionNode : public Node
	{
	public:
		Node *GetLHS() const;
		Node *GetRHS() const;

	protected:
		BinaryExpressionNode( Node *inLHS, Node *inRHS, int inLineNo );
		virtual ~BinaryExpressionNode();

		virtual bool Accept( class Visitor *visit );

		Node *fLHS, *fRHS;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class AdditionExpressionNode : public BinaryExpressionNode
	{
	public:
		AdditionExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class SubtractionExpressionNode : public BinaryExpressionNode
	{
	public:
		SubtractionExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class MultiplicationExpressionNode : public BinaryExpressionNode
	{
	public:
		MultiplicationExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class DivisionExpressionNode : public BinaryExpressionNode
	{
	public:
		DivisionExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class LeftShiftExpressionNode : public BinaryExpressionNode
	{
	public:
		LeftShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class SignedRightShiftExpressionNode : public BinaryExpressionNode
	{
	public:
		SignedRightShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class UnsignedRightShiftExpressionNode : public BinaryExpressionNode
	{
	public:
		UnsignedRightShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class InExpressionNode : public BinaryExpressionNode
	{
	public:
		InExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class LessThanExpressionNode : public BinaryExpressionNode
	{
	public:
		LessThanExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class GreaterThanExpressionNode : public BinaryExpressionNode
	{
	public:
		GreaterThanExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class LessThanOrEqualToExpressionNode : public BinaryExpressionNode
	{
	public:
		LessThanOrEqualToExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class GreaterThanOrEqualToExpressionNode : public BinaryExpressionNode
	{
	public:
		GreaterThanOrEqualToExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class InstanceOfExpressionNode : public BinaryExpressionNode
	{
	public:
		InstanceOfExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class EqualityExpressionNode : public BinaryExpressionNode
	{
	public:
		EqualityExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class NonEqualityExpressionNode : public BinaryExpressionNode
	{
	public:
		NonEqualityExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class StrictEqualityExpressionNode : public BinaryExpressionNode
	{
	public:
		StrictEqualityExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class StrictNonEqualityExpressionNode : public BinaryExpressionNode
	{
	public:
		StrictNonEqualityExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class BitwiseAndExpressionNode : public BinaryExpressionNode
	{
	public:
		BitwiseAndExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class BitwiseXorExpressionNode : public BinaryExpressionNode
	{
	public:
		BitwiseXorExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class BitwiseOrExpressionNode : public BinaryExpressionNode
	{
	public:
		BitwiseOrExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class LogicalAndExpressionNode : public BinaryExpressionNode
	{
	public:
		LogicalAndExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class LogicalOrExpressionNode : public BinaryExpressionNode
	{
	public:
		LogicalOrExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class ModulusExpressionNode : public BinaryExpressionNode
	{
	public:
		ModulusExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class AssignmentExpressionNode : public BinaryExpressionNode
	{
	protected:
		AssignmentExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class AssignMultiplyExpressionNode : public AssignmentExpressionNode
	{
	public:
		AssignMultiplyExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class AssignAddExpressionNode : public AssignmentExpressionNode
	{
	public:
		AssignAddExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class AssignSubtractExpressionNode : public AssignmentExpressionNode
	{
	public:
		AssignSubtractExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class AssignDivideExpressionNode : public AssignmentExpressionNode
	{
	public:
		AssignDivideExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class AssignModulusExpressionNode : public AssignmentExpressionNode
	{
	public:
		AssignModulusExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class AssignLeftShiftExpressionNode : public AssignmentExpressionNode
	{
	public:
		AssignLeftShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class AssignSignedRightShiftExpressionNode : public AssignmentExpressionNode
	{
	public:
		AssignSignedRightShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class AssignUnsignedRightShiftExpressionNode : public AssignmentExpressionNode
	{
	public:
		AssignUnsignedRightShiftExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class AssignBitwiseAndExpressionNode : public AssignmentExpressionNode
	{
	public:
		AssignBitwiseAndExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class AssignBitwiseOrExpressionNode : public AssignmentExpressionNode
	{
	public:
		AssignBitwiseOrExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class AssignBitwiseXorExpressionNode : public AssignmentExpressionNode
	{
	public:
		AssignBitwiseXorExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class AssignExpressionNode : public AssignmentExpressionNode
	{
	public:
		AssignExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class CommaExpressionNode : public BinaryExpressionNode
	{
	public:
		CommaExpressionNode( Node *inLeft, Node *inRight, int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class ConditionalExpressionNode : public Node
	{
	public:
		ConditionalExpressionNode( Node *inExpression, Node *trueExpr, Node *falseExpr, int inLineNo );
		virtual ~ConditionalExpressionNode();

		virtual bool Accept( class Visitor *visit );

	protected:
		Node *fExpr, *fTrueExpr, *fFalseExpr;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class MemberExpressionNode : public LeftHandSideExpressionNode
	{
	protected:
		MemberExpressionNode( int inLineNo );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class FunctionExpressionNode : public MemberExpressionNode
	{
	public:
		FunctionExpressionNode( VString inIdent, FunctionDeclarationArgumentsNode *inArgs, Node *inBody, int inLineNo, int inEndLineNo );
		virtual ~FunctionExpressionNode();

		virtual bool Accept( class Visitor *visit );

		const VString&	GetIdentifier() const;
		void			SetIdentifier(const VString& inIdentifier);

		const VString&	GetFullName() const;
		void			SetFullName(const VString& inName);

		FunctionDeclarationArgumentsNode *GetArgs() const;

	protected:
		VString fIdentifier;
		VString fFullName;
		FunctionDeclarationArgumentsNode *fArgs;
		Node *fBody;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class FunctionCallExpressionNode : public MemberExpressionNode
	{
	public:
		FunctionCallExpressionNode( VString inIdent, Node *inLHS, FunctionCallArgumentsNode *inArgs, int inLineNo );
		virtual ~FunctionCallExpressionNode();

		virtual bool Accept( class Visitor *visit );

		const VString& GetIdentifier() const;
		Node *GetLHS() const;
        FunctionCallArgumentsNode* GetArgs();

	protected:
		VString fIdentifier;
		Node *fLHS;
		FunctionCallArgumentsNode *fArgs;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class NewNode : public MemberExpressionNode
	{
	public:
		NewNode( Node *inRhs, Node *inArgs, int inLineNo );
		virtual ~NewNode();

		virtual bool Accept( class Visitor *visit );

		Node *GetOperand() const;

	protected:
		Node *fRHS, *fArgs;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class ArrayExpressionNode : public MemberExpressionNode
	{
	public:
		ArrayExpressionNode( Node *inLhs, Node *inRhs, int inLineNo );
		virtual ~ArrayExpressionNode();

		virtual bool Accept( class Visitor *visit );

		Node *GetLHS() const;
		Node *GetRHS() const;

	protected:
		Node *fLHS;
		Node *fRHS;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class DotExpressionNode : public MemberExpressionNode
	{
	public:
		DotExpressionNode( Node *inLhs, Node *inRhs, int inLineNo );
		virtual ~DotExpressionNode();

		virtual bool Accept( class Visitor *visit );

		Node *GetLHS() const;
		Node *GetRHS() const;

	protected:
		Node *fLHS, *fRHS;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class PrimaryExpressionNode : public MemberExpressionNode
	{
	protected:
		PrimaryExpressionNode( int inLineNo );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class ThisNode : public PrimaryExpressionNode
	{
	public:
		ThisNode( int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class IdentifierNode : public PrimaryExpressionNode
	{
	public:
		IdentifierNode( VString inIdent, int inLineNo );
		virtual bool Accept( class Visitor *visit );
		
		VString GetText() const;

	protected:
		VString fIdentifier;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class LiteralNode : public PrimaryExpressionNode
	{
	public:
		virtual bool IsLiteral() const;

	protected:
		LiteralNode( int inLineNo );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class ArrayLiteralNode : public LiteralNode
	{
	public:
		ArrayLiteralNode( int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class NullLiteralNode : public LiteralNode
	{
	public:
		NullLiteralNode( int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class StringLiteralNode : public LiteralNode
	{
	public:
		StringLiteralNode( VString inValue, int inLineNo );
		virtual bool Accept( class Visitor *visit );

		VString GetValue() const;

	protected:
		VString fValue;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class NumericLiteralNode : public LiteralNode
	{
	public:
		NumericLiteralNode( const VString &inValue, int inLineNo );
		virtual bool Accept( class Visitor *visit );

		VString GetValue() const;

	protected:
		VString fValue;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class BooleanLiteralNode : public LiteralNode
	{
	public:
		BooleanLiteralNode( bool inValue, int inLineNo );
		virtual bool Accept( class Visitor *visit );

		bool GetValue() const;

	protected:
		bool fValue;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class ObjectLiteralFieldNode : public Node
	{
	public:
		ObjectLiteralFieldNode( Node *inIdent, Node *inValue, int inLineNo );
		virtual ~ObjectLiteralFieldNode();

		Node *GetIdentifier();
		Node *GetValue();

		virtual bool Accept( class Visitor *visit );

	protected:
		Node *fIdent, *fValue;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class ObjectLiteralNode : public LiteralNode
	{
	public:
		ObjectLiteralNode( int inLineNo );
		virtual ~ObjectLiteralNode();

		void AddField( ObjectLiteralFieldNode *inField );
	
		virtual bool Accept( class Visitor *visit );

	protected:
		std::vector< ObjectLiteralFieldNode * > mFields;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class RegExLiteralNode : public LiteralNode
	{
	public:
		RegExLiteralNode( VString inBody, VString inFlags, int inLineNo );
		virtual bool Accept( class Visitor *visit );

		VString GetBody() const;
		VString GetFlags() const;

	protected:
		VString fBody, mFlags;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class StatementNode : public Node
	{
	protected:
		StatementNode( int inLineNo );

		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class EmptyStatementNode : public StatementNode
	{
	public:
		EmptyStatementNode( int inLineNo );
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class LabeledStatementNode : public StatementNode
	{
	public:
		LabeledStatementNode( VString inIdent, Node *inStatement, int inLineNo );
		virtual ~LabeledStatementNode();
		virtual bool Accept( class Visitor *visit );
		
	protected:
		Node *fStatement;
		VString fIdent;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class FunctionDeclarationStatementNode : public StatementNode
	{
	public:
		FunctionDeclarationStatementNode( VString inIdent, FunctionDeclarationArgumentsNode *inArgs, Node *inStatements, int inBeginLineNo, int inEndLineNo );

		virtual ~FunctionDeclarationStatementNode();
		virtual bool Accept( class Visitor *visit );

		const VString& GetIdentifier() const;
		FunctionDeclarationArgumentsNode *GetArgs() const;

	protected:
		FunctionDeclarationArgumentsNode *fArgs;
		Node *fStatements;
		VString fIdent;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class IfStatementNode : public StatementNode
	{
	public:
		IfStatementNode( Node *inExpr, Node *inTrueStatements, Node *inFalseStatements, int inLineNo );
		virtual ~IfStatementNode();
		virtual bool Accept( class Visitor *visit );

	protected:
		Node *fExpr, *mTrueStatements, *mFalseStatements;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class ContinueStatementNode : public StatementNode
	{
	public:
		ContinueStatementNode( Node *inIdent, int inLineNo );
		virtual ~ContinueStatementNode();
		virtual bool Accept( class Visitor *visit );
		
	protected:
		Node *fIdent;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class BreakStatementNode : public StatementNode
	{
	public:
		BreakStatementNode( Node *inIdent, int inLineNo );
		virtual ~BreakStatementNode();
		
		virtual bool Accept( class Visitor *visit );

	protected:
		Node *fIdent;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class DebuggerStatementNode : public StatementNode
	{
	public:
		DebuggerStatementNode( int inLineNo );
		virtual ~DebuggerStatementNode();
		
		virtual bool Accept( class Visitor *visit );
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class ReturnStatementNode : public StatementNode
	{
	public:
		ReturnStatementNode( Node *inExpr, int inLineNo );
		virtual ~ReturnStatementNode();
		
		virtual bool Accept( class Visitor *visit );

		Node *GetExpression() const;

	protected:
		Node *fExpr;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class WithStatementNode : public StatementNode
	{
	public:
		WithStatementNode( Node *inExpr, Node *inStatement, int inLineNo );
		virtual ~WithStatementNode();
		
		virtual bool Accept( class Visitor *visit );
		
	protected:
		Node *fExpr, *fStatement;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class ThrowStatementNode : public StatementNode
	{
	public:
		ThrowStatementNode( Node *inExpr, int inLineNo );
		virtual ~ThrowStatementNode();
		
		virtual bool Accept( class Visitor *visit );

	protected:
		Node *fExpr;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class CatchStatementNode : public Node
	{
	public:
		CatchStatementNode( Node *inIdent, Node *inStatements, int inLineNo );
		virtual ~CatchStatementNode();
		
		virtual bool Accept( class Visitor *visit );

		Node *GetStatements();
		Node *GetIdentifier();

	protected:
		Node *fIdent, *fStatements;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class TryStatementNode : public StatementNode
	{
	public:
		TryStatementNode( Node *inStatements, Node *inCatch, Node *inFinally, int inLineNo );
		virtual ~TryStatementNode();
		
		virtual bool Accept( class Visitor *visit );

	protected:
		Node *fStatements, *mCatch, *mFinally;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class SwitchStatementNode : public StatementNode
	{
	public:
		SwitchStatementNode( Node *inExpr, int inLineNo );
		virtual ~SwitchStatementNode();

		void AddCase( Node *inCase );
		virtual bool Accept( class Visitor *visit );

	protected:
		Node *fExpr;
		std::vector< Node * > mCases;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class CaseClauseNode : public Node
	{
	public:
		CaseClauseNode( Node *inExpr, Node *inStatement, int inLineNo );
		virtual ~CaseClauseNode();
		
		virtual bool Accept( class Visitor *visit );

	protected:
		Node *fExpr, *fStatement;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class DefaultClauseNode : public Node
	{
	public:
		DefaultClauseNode( Node *inStatement, int inLineNo );
		virtual ~DefaultClauseNode();
		virtual bool Accept( class Visitor *visit );

	protected:
		Node *fStatement;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class VariableDeclarationStatementNode : public StatementNode
	{
	public:
		VariableDeclarationStatementNode( Node *inIdent, Node *inAssignmentExpr, int inLineNo );
		virtual ~VariableDeclarationStatementNode();
		
		virtual bool Accept( class Visitor *visit );

		Node *GetIdentifier();
		Node *GetAssignment();

	protected:
		Node *fIdent, *mAssignment;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class VariableDeclarationListStatementNode : public StatementNode
	{
	public:
		VariableDeclarationListStatementNode( VariableDeclarationStatementNode *inFirst, int inLineNo );
		virtual ~VariableDeclarationListStatementNode();

		void AddDeclaration( VariableDeclarationStatementNode *inDecl );
		virtual bool Accept( class Visitor *visit );

	protected:
		std::vector< VariableDeclarationStatementNode * > fDecls;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class DoStatementNode : public StatementNode
	{
	public:
		DoStatementNode( Node *inExpr, Node *inStatement, int inLineNo );
		virtual ~DoStatementNode();
		virtual bool Accept( class Visitor *visit );

	protected:
		Node *fExpr, *fStatement;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class WhileStatementNode : public StatementNode
	{
	public:
		WhileStatementNode( Node *inExpr, Node *inStatement, int inLineNo );
		virtual ~WhileStatementNode();
		
		virtual bool Accept( class Visitor *visit );

	protected:
		Node *fExpr, *fStatement;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class ForStatementNode : public StatementNode
	{
	public:
		ForStatementNode( Node *inExpr, Node *inStatement, int inLineNo );
		virtual ~ForStatementNode();
		
		virtual bool Accept( class Visitor *visit );

	protected:
		Node *fExpr, *fStatement;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class ForExpressionTriClauseNode : public Node
	{
	public:
		ForExpressionTriClauseNode( Node *inDecl, Node *inTest, Node *inLoop, int inLineNo );
		virtual ~ForExpressionTriClauseNode();
		
		virtual bool Accept( class Visitor *visit );

	protected:
		Node *fDecl, *fTest, *fLoop;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class ForExpressionInClauseNode : public Node
	{
	public:
		ForExpressionInClauseNode( Node *inDecl, Node *inExpr, int inLineNo );
		virtual ~ForExpressionInClauseNode();
		
		virtual bool Accept( class Visitor *visit );

		Node *GetDeclaration() const;

	protected:
		Node *fDecl, *fExpr;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class StatementList : public StatementNode
	{
	public:
		StatementList( int inLineNo );
		void AddStatement( Node *inStatement );

		void SetListCompletionLine( int inLineNo );

		virtual ~StatementList();
		
		virtual bool Accept( class Visitor *visit );

	protected:
		std::vector< Node * > fStatementList;
	};
}

#endif // _JAVA_SCRIPT_AST_H_