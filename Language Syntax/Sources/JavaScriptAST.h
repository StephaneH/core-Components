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
	/*
	 *
	 *
	 *
	 */
	class SymbolDefinitionContext
	{
	public:
		SymbolDefinitionContext():fLocalVariableFlag(false),fVariableFlag(false),fInstancePropertyFlag(false),fPropertyFlag(false),fFromReturnFlag(false),fFromNewFlag(false),fFromAssignmentFlag(false),fScopeCreationForLastIdentifiersChainElementFlag(false), fRhsAssignmentTypeName("") {}
		virtual ~SymbolDefinitionContext() {}
		
		void SetLocalVariableFlag(const bool& inFlag) { fLocalVariableFlag = inFlag; }
		bool GetLocalVariableFlag() const { return fLocalVariableFlag; }

		void SetVariableFlag(const bool& inFlag) { fVariableFlag = inFlag; }
		bool GetVariableFlag() const { return fVariableFlag; }
		
		void SetInstancePropertyFlag(const bool& inFlag) { fInstancePropertyFlag = inFlag; }
		bool GetInstancePropertyFlag() const { return fInstancePropertyFlag; }
		
		void SetPropertyFlag(const bool& inFlag) { fPropertyFlag = inFlag; }
		bool GetPropertyFlag() const { return fPropertyFlag; }
		
		void SetFromReturnFlag(const bool& inFlag) { fFromReturnFlag = inFlag; }
		bool GetFromReturnFlag() const { return fFromReturnFlag; }

		void SetFromNewFlag(const bool& inFlag) { fFromNewFlag = inFlag; }
		bool GetFromNewFlag() const { return fFromNewFlag; }

		void SetFromAssignmentFlag(const bool& inFlag) { fFromAssignmentFlag = inFlag; }
		bool GetFromAssignmentFlag() const { return fFromAssignmentFlag; }
		
		void SetScopeCreationForLastIdentifiersChainElementFlag(const bool& inFlag) { fScopeCreationForLastIdentifiersChainElementFlag = inFlag; }
		bool GetScopeCreationForLastIdentifiersChainElementFlag() const { return fScopeCreationForLastIdentifiersChainElementFlag; }

		void SetFromDeclarationType(const ESymbolDeclarationType& inValue) { fFromDeclarationType = inValue; }
		ESymbolDeclarationType GetFromDeclarationType() const { return fFromDeclarationType; }
		
		void SetRhsAssignmentTypeName(const XBOX::VString& inTypeName) { fRhsAssignmentTypeName = inTypeName; }
		const XBOX::VString& GtRhsAssignmentTypeName(const XBOX::VString& inTypeName) const { return fRhsAssignmentTypeName; }

	private:
		// Not Copiable
		SymbolDefinitionContext(const SymbolDefinitionContext&);
		SymbolDefinitionContext& operator=(const SymbolDefinitionContext&);
		
		bool fLocalVariableFlag;
		bool fVariableFlag;
		bool fInstancePropertyFlag;
		bool fPropertyFlag;
		bool fFromReturnFlag;
		bool fFromNewFlag;
		bool fFromAssignmentFlag;
		bool fScopeCreationForLastIdentifiersChainElementFlag;
		ESymbolDeclarationType fFromDeclarationType;
		XBOX::VString fRhsAssignmentTypeName;
	};

	/*
	 *
	 *
	 *
	 */
	class ISymbolDeclaration
	{
	public:
		virtual ~ISymbolDeclaration() {}
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const = 0;
		virtual ISymbolDeclaration* Clone() const = 0;
		virtual void ToString(XBOX::VString&) const = 0;
		virtual void ToDatabase(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const {}
		
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset) {}
		
		virtual bool HasRhsDeclaration() const { return false; }
		virtual ESymbolDeclarationType GetRhsDeclarationType() const { return eSymbolDeclarationTypeNone; }
		
		static void DeleteSymbolDeclaration(ISymbolDeclaration* inDeclaration)
		{
			{
				if( inDeclaration )
				{
					delete inDeclaration;
					inDeclaration = NULL;
				}
			}
		}
		
		static void DeleteSymbolDeclarations(std::vector<ISymbolDeclaration*>& inDeclarations)
		{
			for(std::vector<ISymbolDeclaration*>::const_iterator it=inDeclarations.begin(); it!=inDeclarations.end(); ++it)
				ISymbolDeclaration::DeleteSymbolDeclaration( (*it) );
		}
	};
	
	class TypesDeclaration : public ISymbolDeclaration
	{
	public:
		TypesDeclaration();
		virtual ~TypesDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
		void AppendTypeDeclaration(ISymbolDeclaration* inDeclaration);
		

	private:
		// Not Copiable
		TypesDeclaration(const TypesDeclaration&);
		TypesDeclaration& operator=(const TypesDeclaration&);
		
		std::vector<ISymbolDeclaration*> fTypes;
        sLONG fAtLine;
	};

	
	
	class VariableDeclaration : public ISymbolDeclaration
	{
	public:
		VariableDeclaration();
		virtual ~VariableDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void ToDatabase(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
		void SetName(const XBOX::VString& inName);
		void SetRhsTypeName(const XBOX::VString& inRhsTypeName);
		void SetRhs(ISymbolDeclaration* inDeclaration);
		void SetLocalFlag(const bool& inValue);
		
	private:
		// Not Copiable
		VariableDeclaration(const VariableDeclaration&);
		VariableDeclaration& operator=(const VariableDeclaration&);
		
		XBOX::VString fName;
		XBOX::VString fRhsTypeName;
		ISymbolDeclaration* fRhs;
		bool fLocalFlag;
		sLONG fAtLine;
	};
	
	
	
	class ObjectFieldDeclaration : public ISymbolDeclaration
	{
	public:
		ObjectFieldDeclaration();
		virtual ~ObjectFieldDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
		void SetLhs(ISymbolDeclaration* inValue);
		void SetRhs(ISymbolDeclaration* inValue);
		
	private:
		// Not Copiable
		ObjectFieldDeclaration(const ObjectFieldDeclaration&);
		ObjectFieldDeclaration& operator=(const ObjectFieldDeclaration&);
		
		ISymbolDeclaration* fLhs;
		ISymbolDeclaration* fRhs;
        sLONG fAtLine;
	};
	
	
	
	
	
	class ObjectDeclaration : public ISymbolDeclaration
	{
	public:
		ObjectDeclaration();
		virtual ~ObjectDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void ToDatabase(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
		void SetName(const XBOX::VString& inName);
		void AddObjectFieldDeclaration(ISymbolDeclaration* inDeclaration);

	private:
		// Not Copiable
		ObjectDeclaration(const ObjectDeclaration&);
		ObjectDeclaration& operator=(const ObjectDeclaration&);
		
		XBOX::VString fName;
		
		std::vector<ISymbolDeclaration*> fFields;
		
		sLONG fFromLine;
		sLONG fFromOffset;
		sLONG fToLine;
		sLONG fToOffset;
	};
	
	
	
	
	
	class FunctionCallDeclaration : public ISymbolDeclaration
	{
	public:
		FunctionCallDeclaration();
		virtual ~FunctionCallDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
		void SetName(const XBOX::VString& inName);
		void SetArgumentsDeclaration(ISymbolDeclaration* inDeclaration);
		
	private:
		// Not Copiable
		FunctionCallDeclaration(const FunctionCallDeclaration&);
		FunctionCallDeclaration& operator=(const FunctionCallDeclaration&);
		
		XBOX::VString fName;
		ISymbolDeclaration* fArguments;
        sLONG fAtLine;
	};
	
	
	
	
	
	class NewDeclaration : public ISymbolDeclaration
	{
	public:
		NewDeclaration();
		virtual ~NewDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void ToDatabase(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
		void SetIdentifiersDeclaration(ISymbolDeclaration* inDeclaration);
		
	private:
		// Not Copiable
		NewDeclaration(const NewDeclaration&);
		NewDeclaration& operator=(const NewDeclaration&);
		
		ISymbolDeclaration* fIdentifiers;
        sLONG fAtLine;
	};
	
	
	
	
	
	class ReturnsDeclaration : public ISymbolDeclaration
	{
	public:
		ReturnsDeclaration();
		virtual ~ReturnsDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void ToDatabase(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
		void AppendReturnDeclaration(ISymbolDeclaration* inDeclaration);
		
	private:
		// Not Copiable
		ReturnsDeclaration(const ReturnsDeclaration&);
		ReturnsDeclaration& operator=(const ReturnsDeclaration&);
		
		std::vector<ISymbolDeclaration*> fReturns;
        sLONG fAtLine;
	};
	
	
	
	
	
	class FunctionArgumentsDeclaration : public ISymbolDeclaration
	{
	public:
		FunctionArgumentsDeclaration();
		virtual ~FunctionArgumentsDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
		void AppendArgumentDeclaration(ISymbolDeclaration* inArgument);
		
	private:
		// Not Copiable
		FunctionArgumentsDeclaration(const FunctionArgumentsDeclaration&);
		FunctionArgumentsDeclaration& operator=(const FunctionArgumentsDeclaration&);
		
		std::vector<ISymbolDeclaration*> fArguments;
        sLONG fAtLine;
	};
	
	
	
	
	
	class FunctionExpressionDeclaration : public ISymbolDeclaration
	{
	public:
		FunctionExpressionDeclaration();
		virtual ~FunctionExpressionDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
		void SetName(const XBOX::VString& inName);
		void SetArgumentsDeclaration(ISymbolDeclaration* inDeclaration);
		void SetReturnsDeclaration(ISymbolDeclaration* inDeclaration);
		
	private:
		// Not Copiable
		FunctionExpressionDeclaration(const FunctionExpressionDeclaration&);
		FunctionExpressionDeclaration& operator=(const FunctionExpressionDeclaration&);
		
		XBOX::VString fName;
		ISymbolDeclaration* fArguments;
		ISymbolDeclaration* fReturns;
		
		sLONG fFromLine;
		sLONG fFromOffset;
		sLONG fToLine;
		sLONG fToOffset;
	};
	
	
	
	
	
	class StringLiteralDeclaration : public ISymbolDeclaration
	{
	public:
		StringLiteralDeclaration();
		virtual ~StringLiteralDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void ToDatabase(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
	private:
		// Not Copiable
		StringLiteralDeclaration(const StringLiteralDeclaration&);
		StringLiteralDeclaration& operator=(const StringLiteralDeclaration&);
        sLONG fAtLine;
	};
	
	
	
	
	
	class NumberLiteralDeclaration : public ISymbolDeclaration
	{
	public:
		NumberLiteralDeclaration();
		virtual ~NumberLiteralDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void ToDatabase(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
	private:
		// Not Copiable
		NumberLiteralDeclaration(const NumberLiteralDeclaration&);
		NumberLiteralDeclaration& operator=(const NumberLiteralDeclaration&);
        sLONG fAtLine;
	};
	
	
	
	
	
	class BooleanLiteralDeclaration : public ISymbolDeclaration
	{
	public:
		BooleanLiteralDeclaration();
		virtual ~BooleanLiteralDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void ToDatabase(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
	private:
		// Not Copiable
		BooleanLiteralDeclaration(const BooleanLiteralDeclaration&);
		BooleanLiteralDeclaration& operator=(const BooleanLiteralDeclaration&);
        sLONG fAtLine;
	};
	
	
	
	
	
	class RegExpLiteralDeclaration : public ISymbolDeclaration
	{
	public:
		RegExpLiteralDeclaration();
		virtual ~RegExpLiteralDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void ToDatabase(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
	private:
		// Not Copiable
		RegExpLiteralDeclaration(const RegExpLiteralDeclaration&);
		RegExpLiteralDeclaration& operator=(const RegExpLiteralDeclaration&);
        sLONG fAtLine;
	};
	
	
	
	
	
	class ArrayLiteralDeclaration : public ISymbolDeclaration
	{
	public:
		ArrayLiteralDeclaration();
		virtual ~ArrayLiteralDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void ToDatabase(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
	private:
		// Not Copiable
		ArrayLiteralDeclaration(const ArrayLiteralDeclaration&);
		ArrayLiteralDeclaration& operator=(const ArrayLiteralDeclaration&);
        sLONG fAtLine;
	};
	
	
	
	
	
	class NullLiteralDeclaration : public ISymbolDeclaration
	{
	public:
		NullLiteralDeclaration();
		virtual ~NullLiteralDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
	private:
		// Not Copiable
		NullLiteralDeclaration(const NullLiteralDeclaration&);
		NullLiteralDeclaration& operator=(const NullLiteralDeclaration&);
        sLONG fAtLine;
	};
	
	
	
	
	
	class UndefinedDeclaration : public ISymbolDeclaration
	{
	public:
		UndefinedDeclaration();
		virtual ~UndefinedDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
	private:
		// Not Copiable
		UndefinedDeclaration(const UndefinedDeclaration&);
		UndefinedDeclaration& operator=(const UndefinedDeclaration&);
		sLONG fAtLine;
	};
	
	
	
	
	
	class IdentifiersChainSymbolDeclaration : public ISymbolDeclaration
	{
	public:
		IdentifiersChainSymbolDeclaration();
		virtual ~IdentifiersChainSymbolDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void ToDatabase(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
		void AppendIdentifier(const XBOX::VString& inIdentifier);
		VIndex GetIdentifierCount() const;
		const XBOX::VString& GetIdentifierAt(const VIndex& inIndex) const;
		
	private:
		// Not Copiable
		IdentifiersChainSymbolDeclaration(const IdentifiersChainSymbolDeclaration&);
		IdentifiersChainSymbolDeclaration& operator=(const IdentifiersChainSymbolDeclaration&);

		void ProcessAsAssignmentDeclaration(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const;
		void ProcessAsNewDeclaration(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const;
		void ProcessAsReturnDeclaration(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const;

		std::vector<XBOX::VString> fIdentifiers;
        sLONG fAtLine;
	};
	
	
	
	
	
	class AssignmentDeclaration : public ISymbolDeclaration
	{
	public:
		AssignmentDeclaration();
		virtual ~AssignmentDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void ToDatabase(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
		virtual bool HasRhsDeclaration() const;
		virtual ESymbolDeclarationType GetRhsDeclarationType() const;

		ISymbolDeclaration* GetRhs() const;
		void SetLhs(ISymbolDeclaration* inValue);
		void SetRhs(ISymbolDeclaration* inValue);
		
	private:
		// Not Copiable
		AssignmentDeclaration(const AssignmentDeclaration&);
		AssignmentDeclaration& operator=(const AssignmentDeclaration&);
		
		ISymbolDeclaration* fLhs;
		ISymbolDeclaration* fRhs;
        sLONG fAtLine;
	};
	
	
	
	
	
	class PropertyDeclaration : public ISymbolDeclaration
	{
	public:
		PropertyDeclaration();
		virtual ~PropertyDeclaration();
		virtual void GetSymbolDeclarationType(ESymbolDeclarationType& outTypeValue, XBOX::VString& outTypeName) const;
		virtual ISymbolDeclaration* Clone() const;
		virtual void ToString(XBOX::VString& inDump) const;
		virtual void ToDatabase(ISymbolTable* inSymbolTable, const sLONG& inParentScopeId, const sLONG& inSourceId, const ESymbolFileExecContext& inExecutionContext, SymbolDefinitionContext& inDefinitionContext, sLONG& outSymbolId) const;
		virtual void SetBoundaries(const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset);
		
		void SetName(const XBOX::VString& inName);
		void SetRhsTypeName(const XBOX::VString& inRhsTypeName);
		void SetRhs(ISymbolDeclaration* inDeclaration);
		void SetInstancePropertyFlag(const bool& inFlag);

	private:
		// Not Copiable
		PropertyDeclaration(const PropertyDeclaration&);
		PropertyDeclaration& operator=(const PropertyDeclaration&);
		
		XBOX::VString fName;
		XBOX::VString fRhsTypeName;
		ISymbolDeclaration* fRhs;
		bool fInstancePropertyFlag;
        sLONG fAtLine;
	};
	
	
	
	
	
	
	
	
	
	/*
	 *
	 *
	 *
	 */
	class SymbolScopeDeclaration
	{
	public:
		SymbolScopeDeclaration(SymbolScopeDeclaration* inParent, const XBOX::VString& inName, const XBOX::VString& inFullName, const ESymbolScopeType& inType, const sLONG& inFromLine, const sLONG& inFromOffset, const sLONG& inToLine, const sLONG& inToOffset, const bool& inConstructorFlag, const bool& inExtendFlag, const XBOX::VString& inExtendedName);
		virtual ~SymbolScopeDeclaration();
		
		/*
		 * SCOPE DATA ACCESS
		 */
		SymbolScopeDeclaration*		GetParentScope() const;
		const XBOX::VString&		GetScopeName() const;
		const XBOX::VString&		GetScopeFullName() const;
		const ESymbolScopeType&		GetScopeType() const;
		
		sLONG GetFromLine() const;
		sLONG GetFromOffset() const;
		sLONG GetToLine() const;
		sLONG GetToOffset() const;
		
		bool GetConstructorFlag() const;
		
		bool GetExtendFlag() const;
		void GetExtendedName(XBOX::VString& outExtendedName) const;

		/*
		 * SCOPE BROWSING
		 */
		void					AddChildScope(SymbolScopeDeclaration*);
		XBOX::VIndex			GetChildScopeCount() const;
		SymbolScopeDeclaration*	GetChildScopeAt(XBOX::VIndex) const;
		SymbolScopeDeclaration* GetChildScope(const XBOX::VString& inScopeName) const;
		
		/*
		 * DECLARATION
		 */
		void AddPropertyDeclaration(ISymbolDeclaration*);
		const std::vector<ISymbolDeclaration*>& GetPropertyDeclarations() const;

		void AddVariableDeclaration(ISymbolDeclaration*);
		const std::vector<ISymbolDeclaration*>& GetVariableDeclarations() const;
		
		void AddReturnDeclaration(ISymbolDeclaration*);
		const std::vector<ISymbolDeclaration*>& GetReturnDeclarations() const;

		void AddAssignmentDeclaration(ISymbolDeclaration*);
		const std::vector<ISymbolDeclaration*>& GetAssignmentDeclarations() const;
		
	private:
		/*
		 * WE WANT THIS STUFF NOT TO BE COPIED WITHOUT WE REQUEST IT
		 */
		SymbolScopeDeclaration(const SymbolScopeDeclaration&);
		SymbolScopeDeclaration& operator=(const SymbolScopeDeclaration&);
		
		/*
		 * SCOPE MEMBERS
		 */
		SymbolScopeDeclaration*					fParent;
		XBOX::VString							fName;
		XBOX::VString							fFullName;
		ESymbolScopeType						fType;
		std::vector<SymbolScopeDeclaration*>	fChildren;
		
		sLONG fFromLine;
		sLONG fFromOffset;
		sLONG fToLine;
		sLONG fToOffset;
		
		bool fConstructorFlag;
		
		bool			fExtendFlag;
		XBOX::VString	fExtendedName;

		std::vector<ISymbolDeclaration*>	fProperties;
		std::vector<ISymbolDeclaration*>	fVariables;
		std::vector<ISymbolDeclaration*>	fReturns;
		std::vector<ISymbolDeclaration*>	fAssignments;
	};
	
	
	
	
	
	#define GLOBAL_CONTEXT_SERVER_CLASS		CVSTR("Application")
	#define GLOBAL_CONTEXT_SERVER_INSTANCE	CVSTR("application")

	// This is the base Visitor class used for performing analysis on the AST.  You should implement
	// a subclass of the visitor to perform actual operations.
	class Visitor {
	public:
		static Visitor* GetDeclarationParser( const bool& inUseNextGen, ISymbolTable* inSymTable, Symbols::IFile* inOwningFile, JavaScriptParserDelegate* inDelegate = NULL, SymbolScopeDeclaration* inDeclarationScopes = NULL);

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
		
		virtual bool VisitFunctionDeclarationArgumentsNodeEnter			( class FunctionDeclarationArgumentsNode *inNode )			{ return true; }
		virtual bool VisitFunctionDeclarationArgumentsNodeLeave			( class FunctionDeclarationArgumentsNode *inNode )			{ return true; }
		
		virtual bool VisitFunctionCallArgumentsNodeEnter				( class FunctionCallArgumentsNode *inNode )					{ return true; }
		virtual bool VisitFunctionCallArgumentsNodeLeave				( class FunctionCallArgumentsNode *inNode )					{ return true; }
		
		virtual bool VisitLeftHandSideExpressionNodeEnter				( class LeftHandSideExpressionNode *inNode )				{ return true; }
		virtual bool VisitLeftHandSideExpressionNodeLeave				( class LeftHandSideExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitUnaryExpressionNodeEnter						( class UnaryExpressionNode *inNode )						{ return true; }
		virtual bool VisitUnaryExpressionNodeLeave						( class UnaryExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitDeleteExpressionNodeEnter						( class DeleteExpressionNode *inNode )						{ return true; }
		virtual bool VisitDeleteExpressionNodeLeave						( class DeleteExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitVoidExpressionNodeEnter						( class VoidExpressionNode *inNode )						{ return true; }
		virtual bool VisitVoidExpressionNodeLeave						( class VoidExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitTypeOfExpressionNodeEnter						( class TypeOfExpressionNode *inNode )						{ return true; }
		virtual bool VisitTypeOfExpressionNodeLeave						( class TypeOfExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitPreIncrementorExpressionNodeEnter				( class PreIncrementorExpressionNode *inNode )				{ return true; }
		virtual bool VisitPreIncrementorExpressionNodeLeave				( class PreIncrementorExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitPreDecrementorExpressionNodeEnter				( class PreDecrementorExpressionNode *inNode )				{ return true; }
		virtual bool VisitPreDecrementorExpressionNodeLeave				( class PreDecrementorExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitUnaryPlusExpressionNodeEnter					( class UnaryPlusExpressionNode *inNode )					{ return true; }
		virtual bool VisitUnaryPlusExpressionNodeLeave					( class UnaryPlusExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitUnaryNegateExpressionNodeEnter				( class UnaryNegateExpressionNode *inNode )					{ return true; }
		virtual bool VisitUnaryNegateExpressionNodeLeave				( class UnaryNegateExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitBitwiseNotExpressionNodeEnter					( class BitwiseNotExpressionNode *inNode )					{ return true; }
		virtual bool VisitBitwiseNotExpressionNodeLeave					( class BitwiseNotExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitLogicalNotExpressionNodeEnter					( class LogicalNotExpressionNode *inNode )					{ return true; }
		virtual bool VisitLogicalNotExpressionNodeLeave					( class LogicalNotExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitPostIncrementorNodeEnter						( class PostIncrementorNode *inNode )						{ return true; }
		virtual bool VisitPostIncrementorNodeLeave						( class PostIncrementorNode *inNode )						{ return true; }
		
		virtual bool VisitPostDecrementorNodeEnter						( class PostDecrementorNode *inNode )						{ return true; }
		virtual bool VisitPostDecrementorNodeLeave						( class PostDecrementorNode *inNode )						{ return true; }
		
		virtual bool VisitBinaryExpressionNodeEnter						( class BinaryExpressionNode *inNode )						{ return true; }
		virtual bool VisitBinaryExpressionNodeLeave						( class BinaryExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitAdditionExpressionNodeEnter					( class AdditionExpressionNode *inNode )					{ return true; }
		virtual bool VisitAdditionExpressionNodeLeave					( class AdditionExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitSubtractionExpressionNodeEnter				( class SubtractionExpressionNode *inNode )					{ return true; }
		virtual bool VisitSubtractionExpressionNodeLeave				( class SubtractionExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitMultiplicationExpressionNodeEnter				( class MultiplicationExpressionNode *inNode )				{ return true; }
		virtual bool VisitMultiplicationExpressionNodeLeave				( class MultiplicationExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitDivisionExpressionNodeEnter					( class DivisionExpressionNode *inNode )					{ return true; }
		virtual bool VisitDivisionExpressionNodeLeave					( class DivisionExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitLeftShiftExpressionNodeEnter					( class LeftShiftExpressionNode *inNode )					{ return true; }
		virtual bool VisitLeftShiftExpressionNodeLeave					( class LeftShiftExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitSignedRightShiftExpressionNodeEnter			( class SignedRightShiftExpressionNode *inNode )			{ return true; }
		virtual bool VisitSignedRightShiftExpressionNodeLeave			( class SignedRightShiftExpressionNode *inNode )			{ return true; }
		
		virtual bool VisitUnsignedRightShiftExpressionNodeEnter			( class UnsignedRightShiftExpressionNode *inNode )			{ return true; }
		virtual bool VisitUnsignedRightShiftExpressionNodeLeave			( class UnsignedRightShiftExpressionNode *inNode )			{ return true; }
		
		virtual bool VisitInExpressionNodeEnter							( class InExpressionNode *inNode )							{ return true; }
		virtual bool VisitInExpressionNodeLeave							( class InExpressionNode *inNode )							{ return true; }
		
		virtual bool VisitLessThanExpressionNodeEnter					( class LessThanExpressionNode *inNode )					{ return true; }
		virtual bool VisitLessThanExpressionNodeLeave					( class LessThanExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitGreaterThanExpressionNodeEnter				( class GreaterThanExpressionNode *inNode )					{ return true; }
		virtual bool VisitGreaterThanExpressionNodeLeave				( class GreaterThanExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitLessThanOrEqualToExpressionNodeEnter			( class LessThanOrEqualToExpressionNode *inNode )			{ return true; }
		virtual bool VisitLessThanOrEqualToExpressionNodeLeave			( class LessThanOrEqualToExpressionNode *inNode )			{ return true; }
		
		virtual bool VisitGreaterThanOrEqualToExpressionNodeEnter		( class GreaterThanOrEqualToExpressionNode *inNode )		{ return true; }
		virtual bool VisitGreaterThanOrEqualToExpressionNodeLeave		( class GreaterThanOrEqualToExpressionNode *inNode )		{ return true; }
		
		virtual bool VisitInstanceOfExpressionNodeEnter					( class InstanceOfExpressionNode *inNode )					{ return true; }
		virtual bool VisitInstanceOfExpressionNodeLeave					( class InstanceOfExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitEqualityExpressionNodeEnter					( class EqualityExpressionNode *inNode )					{ return true; }
		virtual bool VisitEqualityExpressionNodeLeave					( class EqualityExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitNonEqualityExpressionNodeEnter				( class NonEqualityExpressionNode *inNode )					{ return true; }
		virtual bool VisitNonEqualityExpressionNodeLeave				( class NonEqualityExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitStrictEqualityExpressionNodeEnter				( class StrictEqualityExpressionNode *inNode )				{ return true; }
		virtual bool VisitStrictEqualityExpressionNodeLeave				( class StrictEqualityExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitStrictNonEqualityExpressionNodeEnter			( class StrictNonEqualityExpressionNode *inNode )			{ return true; }
		virtual bool VisitStrictNonEqualityExpressionNodeLeave			( class StrictNonEqualityExpressionNode *inNode )			{ return true; }
		
		virtual bool VisitBitwiseAndExpressionNodeEnter					( class BitwiseAndExpressionNode *inNode )					{ return true; }
		virtual bool VisitBitwiseAndExpressionNodeLeave					( class BitwiseAndExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitBitwiseXorExpressionNodeEnter					( class BitwiseXorExpressionNode *inNode )					{ return true; }
		virtual bool VisitBitwiseXorExpressionNodeLeave					( class BitwiseXorExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitBitwiseOrExpressionNodeEnter					( class BitwiseOrExpressionNode *inNode )					{ return true; }
		virtual bool VisitBitwiseOrExpressionNodeLeave					( class BitwiseOrExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitLogicalAndExpressionNodeEnter					( class LogicalAndExpressionNode *inNode )					{ return true; }
		virtual bool VisitLogicalAndExpressionNodeLeave					( class LogicalAndExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitLogicalOrExpressionNodeEnter					( class LogicalOrExpressionNode *inNode )					{ return true; }
		virtual bool VisitLogicalOrExpressionNodeLeave					( class LogicalOrExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitModulusExpressionNodeEnter					( class ModulusExpressionNode *inNode )						{ return true; }
		virtual bool VisitModulusExpressionNodeLeave					( class ModulusExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitAssignMultiplyExpressionNodeEnter				( class AssignMultiplyExpressionNode *inNode )				{ return true; }
		virtual bool VisitAssignMultiplyExpressionNodeLeave				( class AssignMultiplyExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitAssignAddExpressionNodeEnter					( class AssignAddExpressionNode *inNode )					{ return true; }
		virtual bool VisitAssignAddExpressionNodeLeave					( class AssignAddExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitAssignSubtractExpressionNodeEnter				( class AssignSubtractExpressionNode *inNode )				{ return true; }
		virtual bool VisitAssignSubtractExpressionNodeLeave				( class AssignSubtractExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitAssignDivideExpressionNodeEnter				( class AssignDivideExpressionNode *inNode )				{ return true; }
		virtual bool VisitAssignDivideExpressionNodeLeave				( class AssignDivideExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitAssignModulusExpressionNodeEnter				( class AssignModulusExpressionNode *inNode )				{ return true; }
		virtual bool VisitAssignModulusExpressionNodeLeave				( class AssignModulusExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitAssignLeftShiftExpressionNodeEnter			( class AssignLeftShiftExpressionNode *inNode )				{ return true; }
		virtual bool VisitAssignLeftShiftExpressionNodeLeave			( class AssignLeftShiftExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitAssignSignedRightShiftExpressionNodeEnter		( class AssignSignedRightShiftExpressionNode *inNode )		{ return true; }
		virtual bool VisitAssignSignedRightShiftExpressionNodeLeave		( class AssignSignedRightShiftExpressionNode *inNode )		{ return true; }
		
		virtual bool VisitAssignUnsignedRightShiftExpressionNodeEnter	( class AssignUnsignedRightShiftExpressionNode *inNode )	{ return true; }
		virtual bool VisitAssignUnsignedRightShiftExpressionNodeLeave	( class AssignUnsignedRightShiftExpressionNode *inNode )	{ return true; }
		
		virtual bool VisitAssignBitwiseAndExpressionNodeEnter			( class AssignBitwiseAndExpressionNode *inNode )			{ return true; }
		virtual bool VisitAssignBitwiseAndExpressionNodeLeave			( class AssignBitwiseAndExpressionNode *inNode )			{ return true; }
		
		virtual bool VisitAssignBitwiseOrExpressionNodeEnter			( class AssignBitwiseOrExpressionNode *inNode )				{ return true; }
		virtual bool VisitAssignBitwiseOrExpressionNodeLeave			( class AssignBitwiseOrExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitAssignBitwiseXorExpressionNodeEnter			( class AssignBitwiseXorExpressionNode *inNode )			{ return true; }
		virtual bool VisitAssignBitwiseXorExpressionNodeLeave			( class AssignBitwiseXorExpressionNode *inNode )			{ return true; }
		
		virtual bool VisitAssignExpressionNodeEnter						( class AssignExpressionNode *inNode )						{ return true; }
		virtual bool VisitAssignExpressionNodeLeave						( class AssignExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitAssignmentExpressionNodeEnter					( class AssignmentExpressionNode *inNode )					{ return true; }
		virtual bool VisitAssignmentExpressionNodeLeave					( class AssignmentExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitCommaExpressionNodeEnter						( class CommaExpressionNode *inNode )						{ return true; }
		virtual bool VisitCommaExpressionNodeLeave						( class CommaExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitConditionalExpressionNodeEnter				( class ConditionalExpressionNode *inNode )					{ return true; }
		virtual bool VisitConditionalExpressionNodeLeave				( class ConditionalExpressionNode *inNode )					{ return true; }
		
		virtual bool VisitFunctionCallExpressionNodeEnter				( class FunctionCallExpressionNode *inNode )				{ return true; }
		virtual bool VisitFunctionCallExpressionNodeLeave				( class FunctionCallExpressionNode *inNode )				{ return true; }
		
		virtual bool VisitNewNodeEnter									( class NewNode *inNode )									{ return true; }
		virtual bool VisitNewNodeLeave									( class NewNode *inNode )									{ return true; }
		
		virtual bool VisitArrayExpressionNodeEnter						( class ArrayExpressionNode *inNode )						{ return true; }
		virtual bool VisitArrayExpressionNodeLeave						( class ArrayExpressionNode *inNode )						{ return true; }
		
		virtual bool VisitThisNodeEnter									( class ThisNode *inNode )									{ return true; }
		virtual bool VisitThisNodeLeave									( class ThisNode *inNode )									{ return true; }
		
		virtual bool VisitIdentifierNodeEnter							( class IdentifierNode *inNode )							{ return true; }
		virtual bool VisitIdentifierNodeLeave							( class IdentifierNode *inNode )							{ return true; }
		
		virtual bool VisitArrayLiteralNodeEnter							( class ArrayLiteralNode *inNode )							{ return true; }
		virtual bool VisitArrayLiteralNodeLeave							( class ArrayLiteralNode *inNode )							{ return true; }
		
		virtual bool VisitNullLiteralNodeEnter							( class NullLiteralNode *inNode )							{ return true; }
		virtual bool VisitNullLiteralNodeLeave							( class NullLiteralNode *inNode )							{ return true; }
		
		virtual bool VisitStringLiteralNodeEnter						( class StringLiteralNode *inNode )							{ return true; }
		virtual bool VisitStringLiteralNodeLeave						( class StringLiteralNode *inNode )							{ return true; }
		
		virtual bool VisitNumericLiteralNodeEnter						( class NumericLiteralNode *inNode )						{ return true; }
		virtual bool VisitNumericLiteralNodeLeave						( class NumericLiteralNode *inNode )						{ return true; }
		
		virtual bool VisitBooleanLiteralNodeEnter						( class BooleanLiteralNode *inNode )						{ return true; }
		virtual bool VisitBooleanLiteralNodeLeave						( class BooleanLiteralNode *inNode )						{ return true; }
		
		virtual bool VisitRegExLiteralNodeEnter							( class RegExLiteralNode *inNode )							{ return true; }
		virtual bool VisitRegExLiteralNodeLeave							( class RegExLiteralNode *inNode )							{ return true; }
		
		virtual bool VisitStatementNodeEnter							( class StatementNode *inNode )								{ return true; }
		virtual bool VisitStatementNodeLeave							( class StatementNode *inNode )								{ return true; }
		
		virtual bool VisitEmptyStatementNodeEnter						( class EmptyStatementNode *inNode )						{ return true; }
		virtual bool VisitEmptyStatementNodeLeave						( class EmptyStatementNode *inNode )						{ return true; }
		
		virtual bool VisitLabeledStatementNodeEnter						( class LabeledStatementNode *inNode )						{ return true; }
		virtual bool VisitLabeledStatementNodeLeave						( class LabeledStatementNode *inNode )						{ return true; }
		
		virtual bool VisitIfStatementNodeEnter							( class IfStatementNode *inNode )							{ return true; }
		virtual bool VisitIfStatementNodeLeave							( class IfStatementNode *inNode )							{ return true; }
		
		virtual bool VisitContinueStatementNodeEnter					( class ContinueStatementNode *inNode )						{ return true; }
		virtual bool VisitContinueStatementNodeLeave					( class ContinueStatementNode *inNode )						{ return true; }
		
		virtual bool VisitBreakStatementNodeEnter						( class BreakStatementNode *inNode )						{ return true; }
		virtual bool VisitBreakStatementNodeLeave						( class BreakStatementNode *inNode )						{ return true; }
		
		virtual bool VisitDebuggerStatementNodeEnter					( class DebuggerStatementNode *inNode )						{ return true; }
		virtual bool VisitDebuggerStatementNodeLeave					( class DebuggerStatementNode *inNode )						{ return true; }
		
		virtual bool VisitReturnStatementNodeEnter						( class ReturnStatementNode *inNode )						{ return true; }
		virtual bool VisitReturnStatementNodeLeave						( class ReturnStatementNode *inNode )						{ return true; }
		
		virtual bool VisitWithStatementNodeEnter						( class WithStatementNode *inNode )							{ return true; }
		virtual bool VisitWithStatementNodeLeave						( class WithStatementNode *inNode )							{ return true; }
		
		virtual bool VisitThrowStatementNodeEnter						( class ThrowStatementNode *inNode )						{ return true; }
		virtual bool VisitThrowStatementNodeLeave						( class ThrowStatementNode *inNode )						{ return true; }
		
		virtual bool VisitTryStatementNodeEnter							( class TryStatementNode *inNode )							{ return true; }
		virtual bool VisitTryStatementNodeLeave							( class TryStatementNode *inNode )							{ return true; }
		
		virtual bool VisitSwitchStatementNodeEnter						( class SwitchStatementNode *inNode )						{ return true; }
		virtual bool VisitSwitchStatementNodeLeave						( class SwitchStatementNode *inNode )						{ return true; }
		
		virtual bool VisitCaseClauseNodeEnter							( class CaseClauseNode *inNode )							{ return true; }
		virtual bool VisitCaseClauseNodeLeave							( class CaseClauseNode *inNode )							{ return true; }
		
		virtual bool VisitDefaultClauseNodeEnter						( class DefaultClauseNode *inNode )							{ return true; }
		virtual bool VisitDefaultClauseNodeLeave						( class DefaultClauseNode *inNode )							{ return true; }
		
		virtual bool VisitDoStatementNodeEnter							( class DoStatementNode *inNode )							{ return true; }
		virtual bool VisitDoStatementNodeLeave							( class DoStatementNode *inNode )							{ return true; }
		
		virtual bool VisitWhileStatementNodeEnter						( class WhileStatementNode *inNode )						{ return true; }
		virtual bool VisitWhileStatementNodeLeave						( class WhileStatementNode *inNode )						{ return true; }
		
		virtual bool VisitForStatementNodeEnter							( class ForStatementNode *inNode )							{ return true; }
		virtual bool VisitForStatementNodeLeave							( class ForStatementNode *inNode )							{ return true; }
		
		virtual bool VisitForExpressionTriClauseNodeEnter				( class ForExpressionTriClauseNode *inNode )				{ return true; }
		virtual bool VisitForExpressionTriClauseNodeLeave				( class ForExpressionTriClauseNode *inNode )				{ return true; }
		
		virtual bool VisitForExpressionInClauseNodeEnter				( class ForExpressionInClauseNode *inNode )					{ return true; }
		virtual bool VisitForExpressionInClauseNodeLeave				( class ForExpressionInClauseNode *inNode )					{ return true; }
		
		virtual bool VisitStatementListNodeEnter						( class StatementList *inNode )								{ return true; }
		virtual bool VisitStatementListNodeLeave						( class StatementList *inNode )								{ return true; }
	};
	
	
	
	
	
	
	
	
	
	typedef sLONG EASTNodeType;
	// This is an abstract class which all AST nodes must inherit from
	/*
	 *
	 *
	 *
	 */
	class Node
	{
	public:
		enum
		{
/*	0	*/	kTypeUndef=0,
/*	1	*/	kTypeBaseNode,
/*	2	*/	kTypeProgramNode,
/*	3	*/	kTypeFunctionCallArgumentsNode,
/*	4	*/	kTypeFunctionDeclarationArgumentsNode,
/*	5	*/	kTypeLeftHandSideExpressionNode,
/*	6	*/	kTypeUnaryExpressionNode,
/*	7	*/	kTypeDeleteExpressionNode,
/*	8	*/	kTypeVoidExpressionNode,
/*	9	*/	kTypeTypeOfExpressionNode,
/*	10	*/	kTypePreIncrementorExpressionNode,
/*	11	*/	kTypePreDecrementorExpressionNode,
/*	12	*/	kTypeUnaryPlusExpressionNode,
/*	13	*/	kTypeUnaryNegateExpressionNode,
/*	14	*/	kTypeBitwiseNotExpressionNode,
/*	15	*/	kTypeLogicalNotExpressionNode,
/*	16	*/	kTypePostIncrementorNode,
/*	17	*/	kTypePostDecrementorNode,
/*	18	*/	kTypeBinaryExpressionNode,
/*	19	*/	kTypeAdditionExpressionNode,
/*	20	*/	kTypeSubtractionExpressionNode,
/*	21	*/	kTypeMultiplicationExpressionNode,
/*	22	*/	kTypeDivisionExpressionNode,
/*	23	*/	kTypeLeftShiftExpressionNode,
/*	24	*/	kTypeSignedRightShiftExpressionNode,
/*	25	*/	kTypeUnsignedRightShiftExpressionNode,
/*	26	*/	kTypeInExpressionNode,
/*	27	*/	kTypeLessThanExpressionNode,
/*	28	*/	kTypeGreaterThanExpressionNode,
/*	29	*/	kTypeLessThanOrEqualToExpressionNode,
/*	30	*/	kTypeGreaterThanOrEqualToExpressionNode,
/*	31	*/	kTypeInstanceOfExpressionNode,
/*	32	*/	kTypeEqualityExpressionNode,
/*	33	*/	kTypeNonEqualityExpressionNode,
/*	34	*/	kTypeStrictEqualityExpressionNode,
/*	35	*/	kTypeStrictNonEqualityExpressionNode,
/*	36	*/	kTypeBitwiseAndExpressionNode,
/*	37	*/	kTypeBitwiseXorExpressionNode,
/*	38	*/	kTypeBitwiseOrExpressionNode,
/*	39	*/	kTypeLogicalAndExpressionNode,
/*	40	*/	kTypeLogicalOrExpressionNode,
/*	41	*/	kTypeModulusExpressionNode,
/*	42	*/	kTypeAssignmentExpressionNode,
/*	43	*/	kTypeAssignMultiplyExpressionNode,
/*	44	*/	kTypeAssignAddExpressionNode,
/*	45	*/	kTypeAssignSubtractExpressionNode,
/*	46	*/	kTypeAssignDivideExpressionNode,
/*	47	*/	kTypeAssignModulusExpressionNode,
/*	48	*/	kTypeAssignLeftShiftExpressionNode,
/*	49	*/	kTypeAssignSignedRightShiftExpressionNode,
/*	50	*/	kTypeAssignUnsignedRightShiftExpressionNode,
/*	51	*/	kTypeAssignBitwiseAndExpressionNode,
/*	52	*/	kTypeAssignBitwiseOrExpressionNode,
/*	53	*/	kTypeAssignBitwiseXorExpressionNode,
/*	54	*/	kTypeAssignExpressionNode,
/*	55	*/	kTypeCommaExpressionNode,
/*	56	*/	kTypeConditionalExpressionNode,
/*	57	*/	kTypeMemberExpressionNode,
/*	58	*/	kTypeFunctionExpressionNode,
/*	59	*/	kTypeFunctionCallExpressionNode,
/*	60	*/	kTypeNewNode,
/*	61	*/	kTypeArrayExpressionNode,
/*	62	*/	kTypeDotExpressionNode,
/*	63	*/	kTypePrimaryExpressionNode,
/*	64	*/	kTypeThisNode,
/*	65	*/	kTypeIdentifierNode,
/*	66	*/	kTypeLiteralNode,
/*	67	*/	kTypeArrayLiteralNode,
/*	68	*/	kTypeNullLiteralNode,
/*	69	*/	kTypeStringLiteralNode,
/*	70	*/	kTypeNumericLiteralNode,
/*	71	*/	kTypeBooleanLiteralNode,
/*	72	*/	kTypeObjectLiteralFieldNode,
/*	73	*/	kTypeObjectLiteralNode,
/*	74	*/	kTypeRegExLiteralNode,
/*	75	*/	kTypeStatementNode,
/*	76	*/	kTypeEmptyStatementNode,
/*	77	*/	kTypeLabeledStatementNode,
/*	78	*/	kTypeFunctionDeclarationStatementNode,
/*	79	*/	kTypeIfStatementNode,
/*	80	*/	kTypeContinueStatementNode,
/*	81	*/	kTypeBreakStatementNode,
/*	82	*/	kTypeDebuggerStatementNode,
/*	83	*/	kTypeReturnStatementNode,
/*	84	*/	kTypeWithStatementNode,
/*	85	*/	kTypeThrowStatementNode,
/*	86	*/	kTypeCatchStatementNode,
/*	87	*/	kTypeTryStatementNode,
/*	88	*/	kTypeSwitchStatementNode,
/*	89	*/	kTypeCaseClauseNode,
/*	90	*/	kTypeDefaultClauseNode,
/*	91	*/	kTypeVariableDeclarationStatementNode,
/*	92	*/	kTypeVariableDeclarationListStatementNode,
/*	93	*/	kTypeDoStatementNode,
/*	94	*/	kTypeWhileStatementNode,
/*	95	*/	kTypeForStatementNode,
/*	96	*/	kTypeForExpressionTriClauseNode,
/*	97	*/	kTypeForExpressionInClauseNode,
/*	98	*/	kTypeStatementListNode
		};

		virtual ~Node();

		virtual void AttachScriptDocComment( ScriptDocComment *inComment );
		ScriptDocComment *GetScriptDocComment();

		virtual EASTNodeType GetNodeTypeNG() const = 0;
		virtual const XBOX::VString& GetIdentifierNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const { return NULL; }
		virtual ISymbolDeclaration* GetRhsDeclaration() const { return NULL; }
		
		virtual void FillIdentifiersChainDeclaration(IdentifiersChainSymbolDeclaration&) const {}
		virtual void FillFunctionExpressionDeclaration(FunctionExpressionDeclaration&) const {}
		virtual void FillFunctionArgumentsDeclaration(FunctionArgumentsDeclaration&) const {}
		virtual void FillReturnsDeclaration(ReturnsDeclaration&) const {}
		virtual void FillFunctionCallDeclaration(FunctionCallDeclaration&) const {}
		virtual void FillObjectDeclaration(ObjectDeclaration&) const {}
		virtual void FillVariableDeclaration(VariableDeclaration&) const {}
		virtual void FillAssignmentDeclaration(AssignmentDeclaration&) const {}
		virtual void FillNewDeclaration(NewDeclaration&) const {}

		bool HasConstructorTag() const;
		bool HasExtendTag(XBOX::VString& outToName) const;
		bool HasReturnTag(std::vector<XBOX::VString>& outReturnNames) const;
		bool HasFunctionParameterTag(std::map<VString, VString>& outParamsNameAndValues) const;

		void SetFromLine(const sLONG& inFromLine) { fFromLine = inFromLine; }
		void SetToLine(const sLONG& inToLine) { fToLine = inToLine; }
		void SetFromOffset(const sLONG& inFromOffset) { fFromOffset = inFromOffset; }
		void SetToOffset(const sLONG& inToOffset) { fToOffset = inToOffset; }

		sLONG GetFromLine() const { return fFromLine; }
		sLONG GetToLine() const { return fToLine; }
		sLONG GetFromOffset() const { return fFromOffset; }
		sLONG GetToOffset() const { return fToOffset; }

		virtual bool Accept( class Visitor *visit ) = 0;
		virtual bool IsLeftHandSideExpression() const;
		virtual bool IsLiteral() const;
		int GetLineNumber();
		int GetLineCompletion();

		XBOX::VString	GetWAFComment();
		void			AttachWAFComment(const XBOX::VString& inWAFComment);

	protected:
		Node( int inLineNumber );
		
		int fLineNumber;
		int fLineCompletion;
		ScriptDocComment* fScriptDoc;
		XBOX::VString fWAFComment;
		XBOX::VString fName;
		
		sLONG fFromLine;
		sLONG fToLine;
		sLONG fFromOffset;
		sLONG fToOffset;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
		
	protected:
		Node*	fStatements;
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
		Node* GetArgument( size_t inIndex );

		virtual EASTNodeType GetNodeTypeNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillFunctionArgumentsDeclaration(FunctionArgumentsDeclaration& inDeclaration) const;

	protected:
		std::vector<Node*> fArgs;
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
		void AddArgument( IdentifierNode* inNode );

		size_t ArgumentCount();
		IdentifierNode* GetArgument( size_t inIndex );
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillFunctionArgumentsDeclaration(FunctionArgumentsDeclaration& inDeclaration) const;
		
	protected:
		std::vector<IdentifierNode*> fArgs;
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

		virtual EASTNodeType GetNodeTypeNG() const;
		
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
	public:
		virtual EASTNodeType GetNodeTypeNG() const;
		
	protected:
		UnaryExpressionNode( Node *inLHS, int inLineNo );
		virtual ~UnaryExpressionNode();

		virtual bool Accept( class Visitor *visit );
		Node* fLHS;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		virtual ISymbolDeclaration* GetDeclaration() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

	protected:
		BinaryExpressionNode( Node *inLHS, Node *inRHS, int inLineNo );
		virtual ~BinaryExpressionNode();

		virtual bool Accept( class Visitor *visit );

		Node* fLHS;
		Node* fRHS;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

	public:
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillIdentifiersChainDeclaration(IdentifiersChainSymbolDeclaration& inDeclaration) const;
		virtual void FillAssignmentDeclaration(AssignmentDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
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
		
		virtual ISymbolDeclaration* GetRhsDeclaration() const;

		virtual EASTNodeType GetNodeTypeNG() const;
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
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		
		virtual EASTNodeType GetNodeTypeNG() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;

	protected:
		Node* fExpr;
		Node* fTrueExpr;
		Node* fFalseExpr;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class MemberExpressionNode : public LeftHandSideExpressionNode
	{
	public:
		virtual EASTNodeType GetNodeTypeNG() const;
		
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

		FunctionDeclarationArgumentsNode *GetArgs() const;
		
		virtual EASTNodeType GetNodeTypeNG() const;
		virtual const XBOX::VString& GetIdentifierNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual ISymbolDeclaration* GetRhsDeclaration() const;
		virtual void FillIdentifiersChainDeclaration(IdentifiersChainSymbolDeclaration& inDeclaration) const;
		virtual void FillFunctionExpressionDeclaration(FunctionExpressionDeclaration& inDeclaration) const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;

	protected:
		VString fIdentifier;
		FunctionDeclarationArgumentsNode* fArgs;
		Node* fBody;
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

		const VString&	GetIdentifier() const;
		Node* GetLHS() const;
        FunctionCallArgumentsNode* GetArgs();
		
		virtual EASTNodeType GetNodeTypeNG() const;
		virtual const XBOX::VString& GetIdentifierNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillFunctionCallDeclaration(FunctionCallDeclaration& inDeclaration) const;
		
	protected:
		VString fIdentifier;
		Node* fLHS;
		FunctionCallArgumentsNode* fArgs;
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

		Node* GetOperand() const;
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual ISymbolDeclaration* GetRhsDeclaration() const;

		virtual void FillNewDeclaration(NewDeclaration& inDeclaration) const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
		
	protected:
		Node* fRHS;
		Node* fArgs;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillIdentifiersChainDeclaration(IdentifiersChainSymbolDeclaration& inDeclaration) const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
		
	protected:
		Node* fLHS;
		Node* fRHS;
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

		Node* GetLHS() const;
		Node* GetRHS() const;
		
		virtual EASTNodeType GetNodeTypeNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual ISymbolDeclaration* GetRhsDeclaration() const;

		virtual void FillIdentifiersChainDeclaration(IdentifiersChainSymbolDeclaration& inDeclaration) const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
		
	protected:
		Node* fLHS;
		Node* fRHS;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class PrimaryExpressionNode : public MemberExpressionNode
	{
	public:
		virtual EASTNodeType GetNodeTypeNG() const;
		
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillIdentifiersChainDeclaration(IdentifiersChainSymbolDeclaration& inDeclaration) const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		virtual const XBOX::VString& GetIdentifierNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual ISymbolDeclaration* GetRhsDeclaration() const;

		virtual void FillIdentifiersChainDeclaration(IdentifiersChainSymbolDeclaration& inDeclaration) const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
		
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

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
		
		virtual EASTNodeType GetNodeTypeNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual ISymbolDeclaration* GetRhsDeclaration() const;

		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual ISymbolDeclaration* GetRhsDeclaration() const;

		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		virtual const XBOX::VString& GetIdentifierNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual ISymbolDeclaration* GetRhsDeclaration() const;

		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;

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
		
		virtual EASTNodeType GetNodeTypeNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual ISymbolDeclaration* GetRhsDeclaration() const;

		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;

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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual ISymbolDeclaration* GetRhsDeclaration() const;

		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;

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
		
		virtual EASTNodeType GetNodeTypeNG() const;

		virtual void FillObjectDeclaration(ObjectDeclaration& inDeclaration) const;
		virtual void FillIdentifiersChainDeclaration(IdentifiersChainSymbolDeclaration& inDeclaration) const;
		virtual void FillAssignmentDeclaration(AssignmentDeclaration& inDeclaration) const;

	protected:
		Node* fIdent;
		Node* fValue;
	};
	
	
	
	/*
	 *
	 *
	 *
	 */
	class ObjectLiteralNode : public LiteralNode
	{
	public:
		ObjectLiteralNode( const VString& inName, int inLineNo );
		virtual ~ObjectLiteralNode();

		void AddField( ObjectLiteralFieldNode *inField );
	
		virtual bool Accept( class Visitor *visit );
		
		const VString&	GetIdentifier() const;
		void			SetIdentifier(const VString& inIdentifier);

		virtual EASTNodeType GetNodeTypeNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual ISymbolDeclaration* GetRhsDeclaration() const;

		virtual void FillObjectDeclaration(ObjectDeclaration& inDeclaration) const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;

	protected:
		VString fIdentifier;
		std::vector<ObjectLiteralFieldNode*> mFields;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual ISymbolDeclaration* GetRhsDeclaration() const;

		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;

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
	public:
		virtual EASTNodeType GetNodeTypeNG() const;
		
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
	protected:
		Node* fStatement;
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

		const VString&						GetIdentifier() const;
		FunctionDeclarationArgumentsNode*	GetArgs() const;
		Node*								GetStatements() const;
		
		virtual EASTNodeType GetNodeTypeNG() const;
		virtual const XBOX::VString& GetIdentifierNG() const;

		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillFunctionExpressionDeclaration(FunctionExpressionDeclaration& inDeclaration) const;
		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;
		virtual void FillObjectDeclaration(ObjectDeclaration& inDeclaration) const;

	protected:
		FunctionDeclarationArgumentsNode*	fArgs;
		Node*								fStatements;
		VString								fIdent;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;

	protected:
		Node* fExpr;
		Node* mTrueStatements;
		Node* mFalseStatements;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

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
		
		virtual EASTNodeType GetNodeTypeNG() const;
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

		Node* GetExpression() const;
		
		virtual EASTNodeType GetNodeTypeNG() const;

		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;

	protected:
		Node* fExpr;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

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
		
		virtual EASTNodeType GetNodeTypeNG() const;
		
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

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
		
		virtual EASTNodeType GetNodeTypeNG() const;

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
		
		virtual EASTNodeType GetNodeTypeNG() const;

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
		
		virtual EASTNodeType GetNodeTypeNG() const;

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
		
		virtual void AttachScriptDocComment( ScriptDocComment *inComment );
		
		virtual bool Accept( class Visitor *visit );

		Node *GetIdentifier();
		Node *GetAssignment();
		
		virtual EASTNodeType GetNodeTypeNG() const;
		virtual const XBOX::VString& GetIdentifierNG() const;
		
		virtual ISymbolDeclaration* GetDeclaration() const;
		virtual void FillIdentifiersChainDeclaration(IdentifiersChainSymbolDeclaration& inDeclaration) const;
		virtual void FillAssignmentDeclaration(AssignmentDeclaration& inDeclaration) const;
		virtual void FillVariableDeclaration(VariableDeclaration& inDeclaration) const;

	protected:
		Node* fIdent;
		Node* mAssignment;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

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
		
		virtual EASTNodeType GetNodeTypeNG() const;

	protected:
		Node* fExpr;
		Node* fStatement;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

	protected:
		Node* fExpr;
		Node* fStatement;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

	protected:
		Node* fExpr;
		Node* fStatement;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

	protected:
		Node* fDecl;
		Node* fTest;
		Node* fLoop;
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

		Node* GetForDeclaration() const;
		
		virtual EASTNodeType GetNodeTypeNG() const;

	protected:
		Node* fDecl;
		Node* fExpr;
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
		
		virtual EASTNodeType GetNodeTypeNG() const;

		virtual void FillReturnsDeclaration(ReturnsDeclaration& inDeclaration) const;

	protected:
		std::vector<Node*> fStatementList;
	};
}

#endif // _JAVA_SCRIPT_AST_H_