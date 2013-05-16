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
#ifndef _HTMLPARSER_H_
#define _HTMLPARSER_H_

namespace HTMLTokenValues {
	enum {
		TEXT = 300,
		ENTITY,				// eg, &nbsp;
		START_TAG,			// eg, <p>
		END_TAG,			// eg, </p>
		CLOSED_TAG,			// eg, <br />
		OPENED_START_TAG,	// eg, <a 
		COMMENT,
		ATTRIBUTE,			// eg, href = "test.html"
	};
}

class HTMLParser {
public:
	class ICallback {
	public:
		typedef struct BasicInformation {
			sLONG fStartPosition;
			sLONG fLength;
			const void *fCookie;
		} BasicInformation;

		virtual void TextParsed( const BasicInformation *inInfo ) = 0;
		virtual void OpenTagOpened( const BasicInformation *inInfo, const VString &inName ) = 0;
		virtual void OpenTagClosed( const BasicInformation *inInfo, bool inIsSelfClosed ) = 0;
		virtual void AttributeNameParsed( const BasicInformation *inInfo, const VString &inName ) = 0;
		virtual void AttributeValueParsed( const BasicInformation *inInfo, const VString &inName, const VString &inValue ) = 0;
		virtual void ClosingTagParsed( const BasicInformation *inInfo, const VString &inName ) = 0;
		virtual void EntityParsed( const BasicInformation *inInfo, const VString &inName ) = 0;
		virtual void CommentParsed( const BasicInformation *inInfo, bool inIsUnfinished ) = 0;
		virtual void DocTypeParsed( const BasicInformation *inInfo ) = 0;
		virtual void DocTypeNameParsed( const BasicInformation *inInfo, const VString &inName ) = 0;
		virtual void DocTypeValueParsed( const BasicInformation *inInfo, const VString &inValue ) = 0;
	};

	// The parser state class is a memento that tracks state information across calls to Parse.  It is used
	// so that we can restart parsing from any given location without a source file by giving the next chunk
	// of source code to parse, and the previous state information.
	class State : public XBOX::IRefCountable {
	private:
		friend class HTMLParser;		// Only the HTML parser can create, access or mutate one of these!

		typedef enum Category {
			kDefault,
			kParsingStartTag,
			kParsingEndTag,
			kEndOfInput,
			kParsingComments,
			kParsingEntities,
			kParsingAttributeName, 
			kParsingAttributeValue,
			kParsingBangTag,
			kParsingDocType,
			kParsingDocTypeExternalType,
			kParsingDocTypePublicIdentifier,
			kParsingSystemIdentifier,
			kSkipToCloseBracket,
		} Category;

		State();

		State *Clone() const;

		// Data members for state information go here
		ICallback *fCallback;
		const void *fCallbackCookie;
		Category fCurrentState;

		VString fLastTagName;
		VString fLastAttributeName;
	};

	HTMLParser();
	virtual ~HTMLParser();

	// Parses a chunk of source code, using the previous state information to restart parsing.  the inState parameter
	// can be NULL, in which case it is assumed that you are parsing the start of a document.
	void Parse( const VString &inSource, const State *inState, State **outState, const ICallback *inCallback = NULL, const void *inCallbackCookie = NULL );

protected:
	class HTMLLexer *fLexer;

private:
	void ProcessDefault( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState );
	void ProcessEndTag( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState );
	void ProcessStartTag( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState );
	void ProcessAttributeName( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState );
	void ProcessAttributeValue( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState );
	void ProcessBangTag( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState );
	
	void SkipWhitespace( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState );
	void SkipToCloseBracket( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState );

	bool ExpectValue( TokenList::iterator &ioStart, TokenList::iterator end, int inValue );
	bool ExpectType( TokenList::iterator &ioStart, TokenList::iterator end, ILexerToken::TYPE inType );
};

#endif // _HTMLPARSER_H_