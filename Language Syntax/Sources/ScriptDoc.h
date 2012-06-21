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
	bool ConsumeSeeTag();
	bool ConsumeExceptionTag();
	bool ConsumeTypeTag();
	bool ConsumeParamTag();
	bool ConsumeParamValueTag();
	bool ConsumeReturnTag();
	bool ConsumePropertyTag();
	bool ConsumeThrowsTag();
	bool ConsumeMethodTag();
	
	bool ConsumeTypes( VString &outTypes, bool &outDoFurtherProcessing );
	bool ConsumeName( VString &outName, bool &outIsOptional, bool &outDoFurtherProcessing );

	VLexerStringInput *fStrLexerInput;

	#if _DEBUG
		// The VJavaScriptSyntax constructor has friend acess to the ScriptDocLexer
		// class so that it can call the Test static function to do unit testing.
		friend class VJavaScriptSyntax;
		static void Test( void );
	#endif

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

	class AliasElement : public Element {
	public:
		AliasElement( const VString &alias ) : Element( "alias" ), fAlias( alias ) {}
		virtual ElementType Type() { return kAlias; }

		virtual VString FormatTextForDisplay() { return "Alias: " + fAlias; }
		VString fAlias;
	};

	class AttributesElement : public Element {
	public:
		AttributesElement( const VString &attributes ) : Element( "attributes" ), fAttributes( attributes ) {}
		virtual ElementType Type() { return kAttributes; }

		virtual VString FormatTextForDisplay() { return "Attributes: " + fAttributes; }
		VString fAttributes;
	};

	class AuthorElement : public Element {
	public:
		AuthorElement( const VString &author ) : Element( "author" ), fAuthorName( author ) {}
		virtual ElementType Type() { return kAuthor; }

		virtual VString FormatTextForDisplay() { return "Author: " + fAuthorName; }
		VString fAuthorName;
	};

	class ClassElement : public Element {
	public:
		ClassElement( const VString &desc ) : Element( "class" ), fDesc( desc ) {}
		virtual ElementType Type() { return kClass; }

		virtual VString FormatTextForDisplay() { return "Class: " + fDesc; }
		VString fDesc;
	};

	class ClassDescriptionElement : public Element {
	public:
		ClassDescriptionElement( const VString &desc ) : Element( "classDescription" ), fDesc( desc ) {}
		virtual ElementType Type() { return kClassDescription; }

		virtual VString FormatTextForDisplay() { return "Class Description: " + fDesc; }
		VString fDesc;
	};


	class DefaultElement : public Element {
	public:
		DefaultElement( const VString &value ) : Element( "default" ), fValue( value ) {}
		virtual ElementType Type() { return kDefault; }

		virtual VString FormatTextForDisplay() { return "Default: " + fValue; }
		VString fValue;
	};

	class ProjectDescriptionElement : public Element {
	public:
		ProjectDescriptionElement( const VString &desc ) : Element( "projectDescription" ), fDesc( desc ) {}
		virtual ElementType Type() { return kProjectDescription; }
		
		virtual VString FormatTextForDisplay() { return "Project Description: " + fDesc; }
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

	class IdElement : public Element {
	public:
		IdElement( const VString &name ) : Element( "id" ), fID( name ) {}
		virtual ElementType Type() { return kID; }

		virtual VString FormatTextForDisplay() { return "ID: " + fID; }
		VString fID;
	};

	class MemberOfElement : public Element {
	public:
		MemberOfElement( const VString &name ) : Element( "memberOf" ), fClass( name ) {}
		virtual ElementType Type() { return kMemberOf; }

		virtual VString FormatTextForDisplay() { return "Member Of: " + fClass; }
		VString fClass;
	};

	class MethodElement : public Element {
	public:
		MethodElement() : Element( "method" ) {}
		virtual ElementType Type() { return kMethod; }
		virtual VString FormatTextForDisplay() { return "Method"; }
	};

	class ModuleElement : public Element {
	public:
		ModuleElement( const VString &name ) : Element( "module" ), fName( name ) {}
		virtual ElementType Type() { return kModule; }

		virtual VString FormatTextForDisplay() { return "Module: " + fName; }
		VString fName;
	};

	class NamespaceElement : public Element {
	public:
		NamespaceElement( const VString &name ) : Element( "namespace" ), fName( name ) {}
		virtual ElementType Type() { return kNamespace; }

		virtual VString FormatTextForDisplay() { return "Namespace: " + fName; }
		VString fName;
	};

	class SDocElement : public Element {
	public:
		SDocElement( const VString &path ) : Element( "sdoc" ), fPath( path ) {}
		virtual ElementType Type() { return kSDoc; }

		virtual VString FormatTextForDisplay() { return ""; }
		VString fPath;
	};

	class SeeElement : public Element {
	public:
		SeeElement( const VString &className, const VString &methodName ) : Element( "see" ), fClassName( className ), fMethodName( methodName ) {}
		virtual ElementType Type() { return kSee; }

		virtual VString FormatTextForDisplay() { return "See: " + (fClassName.GetLength())?(fClassName + "." + fMethodName):(fMethodName); }
		VString fClassName, fMethodName;
	};

	class SinceElement : public Element {
	public:
		SinceElement( const VString &vers ) : Element( "since" ), fVersion( vers ) {}
		virtual ElementType Type() { return kSince; }

		virtual VString FormatTextForDisplay() { return "Since: " + fVersion; }
		VString fVersion;
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

	class VersionElement : public Element {
	public:
		VersionElement( const VString &vers ) : Element( "version" ), fVersion( vers ) {}
		virtual ElementType Type() { return kVersion; }

		virtual VString FormatTextForDisplay() { return "Version: " + fVersion; }
		VString fVersion;
	};

	class ExceptionElement : public Element {
	public:
		ExceptionElement( const VString &name, const VString &desc ) : Element( "exception" ), fName( name ), fDesc( desc ) {}
		virtual ElementType Type() { return kException; }

		virtual VString FormatTextForDisplay() { return "Throws: " + fName + VString( (UniChar)CHAR_CONTROL_000D) + fDesc; }
		VString fName, fDesc;
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
		ReturnElement( const VString &typeList, const VString &description ) : Element( "return" ), fTypeList( typeList ), fDesc( description ) {}
		virtual ElementType Type() { return kReturn; }

		virtual VString FormatTextForDisplay() { return "Returns: " + fTypeList + VString( (UniChar)CHAR_CONTROL_000D ) + fDesc; }
		VString fTypeList, fDesc;
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

	class PropertyElement : public Element {
	public:
		PropertyElement( const VString &typeList, const VString &name, const VString &desc) : Element( "property" ), fTypeList( typeList ), fName( name ), fDesc( desc) {}
		virtual ElementType Type() { return kProperty; }

		virtual VString FormatTextForDisplay() { return "Property, Type: " + fTypeList + VString( (UniChar)CHAR_CONTROL_000D) + fName + VString( (UniChar)CHAR_CONTROL_000D) + fDesc ; }
		VString fTypeList, fName, fDesc;

	};

	class RequiresElement : public Element {
	public:
		RequiresElement( const VString &description ) : Element( "requires" ), fDescription( description ) {}
		virtual ElementType Type() { return kRequires; }

		virtual VString FormatTextForDisplay() { return "Requires: " + fDescription; }
		VString fDescription;
	};

	class StaticElement : public Element {
	public:
		StaticElement() : Element( "static" ) {}
		virtual ElementType Type() { return kStatic; }
		virtual VString FormatTextForDisplay() { return "Static"; }
	};

	class ThrowsElement : public Element {
	public:
		ThrowsElement( const VString &type, const VString &desc) : Element( "throws" ), fType( type ), fDesc( desc) {}
		virtual ElementType Type() { return kThrows; }

		virtual VString FormatTextForDisplay() { return "Throws, Type: " + fType + VString( (UniChar)CHAR_CONTROL_000D) + fDesc ; }
		VString fType, fDesc;

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