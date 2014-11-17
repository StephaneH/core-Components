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
#ifndef _SCRIPTDOC_H_
#define _SCRIPTDOC_H_

namespace ScriptDocTokenValues {
	enum {
		START = 300,
		END,
		ELEMENT,
	};
}

class ScriptDocLexer : public VLexerBase {
private:
	bool AdvanceOneToken( int &outToken );
	bool fFoundTags, fFoundComment;

	bool IsScriptDocStart( UniChar ch );
	bool ConsumeScriptDocStart();

	bool IsScriptDocTag( UniChar ch ) { return (ch == CHAR_COMMERCIAL_AT); }
	bool ConsumeScriptDocTag();
	bool ConsumeComment( VString &outComment );
	bool ConsumeTypeTag();
	bool ConsumeParamTag();
	bool ConsumeParamValueTag();
	bool ConsumeReturnTag();
	
	bool ConsumeTypes( VString &outTypes, bool &outDoFurtherProcessing );
	bool ConsumeName( VString &outName, bool &outIsOptional, bool &outDoFurtherProcessing );

	VLexerStringInput *fStrLexerInput;

protected:
	virtual void ReportLexerError( VError inError, const char *inMessage ) {}

	virtual bool IsWhitespace( UniChar inChar )  { return (inChar == CHAR_SPACE || inChar == CHAR_CONTROL_0009); }
	virtual bool IsLineEnding( UniChar inChar )	 { return (inChar == CHAR_CONTROL_000A || inChar == CHAR_CONTROL_000D); }

	virtual bool IsMultiLineCommentStart( UniChar inChar, sLONG &outType ) { return false; }
	virtual bool IsMultiLineCommentEnd( UniChar inChar, sLONG &outCharsToConsume, sLONG inType ) { return false; }
	virtual bool IsSingleLineCommentStart( UniChar inChar ) { return false; }
	virtual bool IsStringStart( UniChar inChar, sLONG &outType, sLONG &outValue ) { return false; }
	virtual bool IsStringEnd( UniChar inChar, sLONG inType )  { return false; }
	virtual bool IsStringEscapeSequence( UniChar inChar, sLONG inType )  { return false; }
	virtual bool IsStringContinuationSequence( UniChar inChar, sLONG inType ) { return false; }
	virtual bool ConsumeEscapeSequence( sLONG inType, UniChar &outCharacterToAdd )  { return false; }

	virtual bool IsIdentifierStart( UniChar inChar )  { return false; }
	virtual bool IsIdentifierCharacter( UniChar inChar )  { return false; }

	virtual int GetNumericTokenValueFromNumericType( ENumericType inType )  { return 0; }
	virtual bool IsNumericStart( UniChar inChar, ENumericType &outType )  { return false; }
	virtual bool ConsumeNumber( VString *ioNumber, TokenList *ioTokens, ENumericType &ioType )  { return false; }

public:
	explicit ScriptDocLexer() : VLexerBase() { fFoundTags = false; fFoundComment = false, fStrLexerInput = NULL; }
	explicit ScriptDocLexer( VString *inText ) : VLexerBase() { fFoundTags = false; fFoundComment = false; fStrLexerInput = NULL; SetLexerInput( inText ); }

	~ScriptDocLexer() { if (NULL != fStrLexerInput) delete fStrLexerInput; };

	void SetLexerInput( VString *inText ) { fStrLexerInput = new VLexerStringInput(); 
											fStrLexerInput->Init( inText ); VLexerBase::SetLexerInput( fStrLexerInput );
											fFoundTags = false; fFoundComment = false;
										}

	virtual int GetNextTokenForParser();
	virtual VError Tokenize( TokenList &outTokens, bool inContinuationOfComment = false, sLONG inOpenStringType = 0 ) { return VE_UNIMPLEMENTED; }

	class Element : public IScriptDocCommentField {
	private:
		virtual int GetKind() { return (int)Type(); }
		virtual VString GetContent() { return FormatTextForDisplay(); }
		virtual VValueBag *GetContents() { VValueBag *ret = new VValueBag();  BuildContents( ret ); return ret; }

	protected:

		Element() {}
		Element( const VString &name ) : fElementName( name ) {}

		virtual void BuildContents( VValueBag *inBag ) {}

	public:
		virtual ElementType Type() { return kUnknown; }
		virtual bool Targets( const VString &inTarget ) { return false; }
		virtual VString FormatTextForDisplay() = 0;
		VString fElementName;
	};

	class CommentElement : public Element {
	protected:
		virtual void BuildContents( VValueBag *inBag ) { inBag->SetString( ScriptDocKeys::Comment, fCommentText ); }

	public:
		CommentElement( const VString &text ) : Element( "" ), fCommentText( text ) {}
		virtual ElementType Type() { return kComment; }

		virtual VString FormatTextForDisplay() { return fCommentText; }

		VString fCommentText;
	};

	class ClassElement : public Element {
	public:
		ClassElement( const VString &desc ) : Element( "class" ), fDesc( desc ) {}
		virtual ElementType Type() { return kClass; }

		virtual VString FormatTextForDisplay() { return "Class: " + fDesc; }
		VString fDesc;
	};

	class ConstructorElement : public Element {
	public:
		ConstructorElement() : Element( "constructor" ) {}
		virtual ElementType Type() { return kConstructor; }
		virtual VString FormatTextForDisplay() { return "Construtor"; }
	};

	class DeprecatedElement : public Element {
	public:
		DeprecatedElement( const VString &desc ) : Element( "deprecated" ), fDesc( desc ) {}
		virtual ElementType Type() { return kDeprecated; }
		virtual VString FormatTextForDisplay() { return "Deprecated: " + fDesc; }
	private:
		VString fDesc;
	};

	class ExtendsElement : public Element {
	public:
		ExtendsElement( const VString &baseClass ) : Element( "extends" ), fBaseClass( baseClass ) {}
		virtual ElementType Type() { return kExtends; }
		virtual VString FormatTextForDisplay() { return "Extends: " + fBaseClass; }
	//private:
		VString fBaseClass;
	};

	class UnknownElement : public Element {
	protected:
		virtual void BuildContents( VValueBag *inBag ) { 
			inBag->SetString( ScriptDocKeys::Tag, fTag );
			inBag->SetString( ScriptDocKeys::Data, fData ); 
		}

	public:
		UnknownElement( const VString &tag, const VString &data ) : Element( "unknown" ), fData( data ), fTag( tag ) {}
		virtual ElementType Type() { return kUnknown; }

		virtual VString FormatTextForDisplay() { return fTag + ": " + fData; }
		VString fTag, fData;
	};

	class TypeElement : public Element {
	protected:
		virtual void BuildContents( VValueBag *inBag ) { inBag->SetString( ScriptDocKeys::Types, fTypes ); }

	public:
		TypeElement( const VString &types ) : Element( "type" ), fTypes( types ) {}
		virtual ElementType Type() { return kType; }

		virtual VString FormatTextForDisplay() { return "Type: " + fTypes; }
		VString fTypes;
	};

	class ParamElement : public Element {
	protected:
		virtual void BuildContents( VValueBag *inBag ) {
			inBag->SetString( ScriptDocKeys::Name, fName );
			inBag->SetString( ScriptDocKeys::Types, fTypes );
			inBag->SetString( ScriptDocKeys::Comment, fDesc );
			inBag->SetBool( ScriptDocKeys::IsOptional, fOptional );
		}

	public:
		ParamElement( const VString &name, const VString &types, const VString &desc, bool isOptional = false ) : Element( "param" ), fName( name ),
			fTypes( types ), fDesc( desc ), fOptional( isOptional ) {}
		virtual ElementType Type() { return kParam; }
		virtual bool Targets( const VString &inTarget ) { return (inTarget == fName); }

		virtual VString FormatTextForDisplay();

		VString fName, fTypes, fDesc;
		bool fOptional;
	};


	class ParamValueElement : public Element {
	protected:
		virtual void BuildContents( VValueBag *inBag ) {
			inBag->SetString( ScriptDocKeys::Name, fName );
			inBag->SetString( ScriptDocKeys::Values, fValues );
		}

	public:
		ParamValueElement( const VString &name, const VString &values ) : Element( "paramValue" ), fName( name ), fValues( values ) {}
		virtual ElementType Type() { return kParamValue; }
		virtual bool Targets( const VString &inTarget ) { return (inTarget == fName); }

		virtual VString FormatTextForDisplay();

		VString fName, fValues;
	};


	class ReturnElement : public Element {
	protected:
		virtual void BuildContents( VValueBag *inBag ) {
			inBag->SetString( ScriptDocKeys::Types, fTypeList );
			inBag->SetString( ScriptDocKeys::Comment, fDesc );
		}

	public:
		ReturnElement( const VString &typeList, const VString &description ):
			Element( "return" ), 
			fTypeList( typeList ), 
			fDesc( description ) 
		{
			fTypeList.GetSubStrings("|", fReturnNames);
		}
		virtual ElementType Type() { return kReturn; }

		virtual VString FormatTextForDisplay() { return "Returns: " + fTypeList + VString( (UniChar)CHAR_CONTROL_000D ) + fDesc; }
		VString fTypeList, fDesc;
		std::vector<XBOX::VString> fReturnNames;
	};

	class InheritsElement : public Element {
	public:
		InheritsElement( const VString &inheritsList ) : Element( "inherits" ), fList( inheritsList ) {}
		virtual ElementType Type() { return kInherits; }

		virtual VString FormatTextForDisplay() { return "Inherits: " + fList; }
		VString fList;
	};

	class PrivateElement : public Element {
	public:
		PrivateElement() : Element( "private" ) {}
		virtual ElementType Type() { return kPrivate; }
		virtual VString FormatTextForDisplay() { return "Private"; }
	};

	// The caller is responsible for the ownership of all elements.  If they do not delete
	// the element if it's created, then that element will be leaked.  This isn't ideal, but
	// it is at least consistent.
	Element *GetElement();

private:
	// This declaration needs to follow the class definition for Element.
	Element *fLastElement;
};

class ScriptDocComment : public XBOX::VObject, public XBOX::IRefCountable {
private:
	VString fSourceText;
	std::vector< ScriptDocLexer::Element * > fElements;

	ScriptDocComment( const VString &inSource ) : fSourceText( inSource ) {}
public:
	// ScriptDocComments can only be created via the factory function because we want to ensure
	// you do not get a partially-correct ScriptDocComment in the event there's a parsing error.
	static ScriptDocComment *Create( const VString &text );
	virtual ~ScriptDocComment();

	sLONG ElementCount() { return fElements.size(); }
	ScriptDocLexer::Element *GetElement( sLONG inIndex ) { return ((inIndex < ElementCount()) ? fElements[ inIndex ] : NULL); }	
	ScriptDocLexer::Element *GetElement( IScriptDocCommentField::ElementType inType );
	void					 GetElements( IScriptDocCommentField::ElementType inType, std::vector<ScriptDocLexer::Element *>& outElements );
	ScriptDocLexer::Element *GetTargetElement( const VString &inTarget );

	VString GetText() const { return fSourceText; }
};
#endif // _SCRIPTDOC_H_