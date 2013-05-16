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

VComponentLibrary *gLanguageSyntaxCompLib = NULL;

void XBOX::xDllMain()
{
	gLanguageSyntaxCompLib = new LanguageSyntaxComponentLibrary(kCOMPONENT_TYPE_LIST, kCOMPONENT_TYPE_COUNT);
}



// ---------------------------------------------------------------------------
// LanguageSyntaxComponentLibrary::LanguageSyntaxComponentLibrary(const CImpDescriptor*, VIndex)
// ---------------------------------------------------------------------------
//

LanguageSyntaxComponentLibrary::LanguageSyntaxComponentLibrary(const CImpDescriptor* inTypeList, VIndex inTypeCount) : VComponentLibrary(inTypeList, inTypeCount)
{
}


// ---------------------------------------------------------------------------
// LanguageSyntaxComponentLibrary::~LanguageSyntaxComponentLibrary
// ---------------------------------------------------------------------------
//

LanguageSyntaxComponentLibrary::~LanguageSyntaxComponentLibrary()
{
}

	
// ---------------------------------------------------------------------------
// LanguageSyntaxComponentLibrary::DoRegister
// ---------------------------------------------------------------------------
//
void LanguageSyntaxComponentLibrary::DoRegister()
{
}


// ---------------------------------------------------------------------------
// LanguageSyntaxComponentLibrary::DoUnregister
// ---------------------------------------------------------------------------
//
void LanguageSyntaxComponentLibrary::DoUnregister()
{
}

class JSResult : public IJSLanguageSyntaxTesterResults
{
private:
	std::vector< VString > mResults;

public:
	JSResult( std::vector< VString > results ) : mResults( results ) {}
	virtual size_t GetResultCount() const { return mResults.size(); }
	virtual VString GetResult( size_t inIndex ) { 
		VString ret;
		if (inIndex >= 0 && inIndex < GetResultCount())	ret = mResults[ inIndex ];
		return ret;
	}

};

class JSDefinitionResult : public IJSLanguageSyntaxTesterDefinitionResults
{
private:
	std::vector< IDefinition > mResults;

public:
	JSDefinitionResult( std::vector< IDefinition > results ) : mResults( results ) {}
	virtual size_t GetResultCount() const { return mResults.size(); }
	virtual IDefinition GetResult( size_t inIndex ) { 
		IDefinition ret;
		if (inIndex >= 0 && inIndex < GetResultCount())	ret = mResults[ inIndex ];
		return ret;
	}
};



class JSColorResult : public IJSLanguageSyntaxTesterColorResults
{
private:
	size_t fOffset, fLength;
	sLONG fStyle;

public:
	JSColorResult( size_t inOffset, size_t inLength, sLONG inStyle ) : fOffset( inOffset ), fLength( inLength ), fStyle( inStyle ) {}

	virtual size_t GetOffset() const { return fOffset; }
	virtual size_t GetLength() const { return fLength; }
	virtual sLONG GetStyle() const { return fStyle; }
};

void VJSLanguageSyntaxTester::Initialize( const VJSParms_initialize &inParams, void *)
{
}

void VJSLanguageSyntaxTester::Finalize( const VJSParms_finalize &inParams, void * )
{
}

void VJSLanguageSyntaxTester::_GetSyntaxColoring( XBOX::VJSParms_callStaticFunction &ioParams, void * )
{
	// We expect two parameters -- the extension for the type of information we have (js, html, etc) and
	// a line of text to be colored.  We will perform the coloring, and return back to the user a list
	// of objects that contain the style information.
	if (ioParams.CountParams() < 2)	return;

	VString extension, data;
	if (!ioParams.GetStringParam( 1, extension ))	return;
	if (!ioParams.GetStringParam( 2, data ))		return;

	CLanguageSyntaxComponent *languageSyntax = (CLanguageSyntaxComponent *)VComponentManager::RetainComponent( (CType)CLanguageSyntaxComponent::Component_Type );
	if (!languageSyntax)	return;
	ISyntaxInterface *syntaxEngine = languageSyntax->GetSyntaxByExtension( extension );
	if (syntaxEngine) {
		// Now that we've got the syntax engine, we can ask it for the coloring information
		std::vector< size_t > offsets;
		std::vector< size_t > lengths;
		std::vector< sLONG > styles;
		syntaxEngine->GetColoringForTesting( data, offsets, lengths, styles );

		// Loop over the returned results, turning them into a vector of VJSValue objects
		std::vector< VJSValue > results;
		std::vector< JSColorResult * > releaseList;
		for (size_t i = 0; i < offsets.size(); i++) {
			JSColorResult *result = new JSColorResult( offsets[ i ], lengths[ i ], styles[ i ] );
			releaseList.push_back( result );
			results.push_back( VJSLanguageSyntaxTesterColorResults::CreateInstance( ioParams.GetContextRef(), result ) );
		}

		// Return an array of our results
		VJSArray arr( ioParams.GetContextRef() );
		arr.PushValues( results );
		ioParams.ReturnValue( arr );

		for (std::vector< JSColorResult * >::iterator iter = releaseList.begin(); iter != releaseList.end(); ++iter) {
			(*iter)->Release();
		}
	}

	languageSyntax->Release();
}

void VJSLanguageSyntaxTester::_GetCompletions( XBOX::VJSParms_callStaticFunction &ioParams, void * )
{
	// We are expecting 5 parameters for JavaScript.  One for the path to the symbol table, one for the path to the test file (which
	// must already be a part of the symbol table), one for the line number within the file, one for the string that
	// we are going to be completing at that point and one for the completion mode.  If we get only three parameters, then it's a CSS completion test, which is
	// fine -- we just modify the parameters we pass into GetSuggestionsForTesting.
	
	VString symTablePathStr, testFilePathStr, completionString, completionModeStr;
	sLONG completionLocation;
	if (ioParams.CountParams() == 5) {
		if (!ioParams.GetStringParam( 1, symTablePathStr ))		return;
		if (!ioParams.GetStringParam( 2, completionString ))	return;
		if (!ioParams.GetStringParam( 3, testFilePathStr ))		return;
		if (!ioParams.GetLongParam( 4, &completionLocation ))	return;
		if (!ioParams.GetStringParam( 5, completionModeStr ))	return;
	} else if (ioParams.CountParams() == 3) {
		bool curlyBraces;
		if (!ioParams.GetStringParam( 1, completionString ))	return;
		if (!ioParams.GetBoolParam( 2, &curlyBraces ))			return;
		if (!ioParams.GetStringParam( 3, testFilePathStr ))		return;

		// We fudge the completion location information when working with CSS files so that it
		// denotes whether we're inside of curly braces or not.
		completionLocation = curlyBraces ? 1 : 0;
	} else {
		return;
	}
	
	VFilePath symbolTablePath( symTablePathStr, FPS_POSIX);
	VFilePath testFilePath( testFilePathStr, FPS_POSIX);

	if ( ! symbolTablePath.IsValid() || ! testFilePath.IsValid() )
		return;

	EJSCompletionTestingMode completionMode;
	if (completionModeStr == "text")
		completionMode = eText;
	else if (completionModeStr == "displayText")
		completionMode = eDisplayText;
	else
		return;

	CLanguageSyntaxComponent *languageSyntax = (CLanguageSyntaxComponent *)VComponentManager::RetainComponent( (CType)CLanguageSyntaxComponent::Component_Type );
	if (languageSyntax)
	{
		// First, load up the symbol table as an actual table instead of just a path string
		ISymbolTable *symTable = NULL;
		if (!symTablePathStr.IsEmpty())
		{
			symTable = languageSyntax->CreateSymbolTable();
			if ( symTable )
			{
				VFile file(symbolTablePath);
				if (symTable->OpenSymbolDatabase( file ))
				{
					// Now that we have all of the parameters grabbed, we can figure out which language syntax engine
					// we want to load up (based on the test file), and ask it to give us completions.  We can then take
					// those completions and turn them into an array of values to pass back to the caller.
					VString extension;
					testFilePath.GetExtension( extension );
					ISyntaxInterface *syntaxEngine = languageSyntax->GetSyntaxByExtension( extension );
					if (syntaxEngine)
					{
						// Now that we've got the syntax engine, we can ask it for the completions we need
						std::vector< VString > suggestions;
						syntaxEngine->GetSuggestionsForTesting( completionString, symTable, testFilePath.GetPath() , completionLocation, completionMode, suggestions );

						JSResult *results = new JSResult( suggestions );
						ioParams.ReturnValue( VJSLanguageSyntaxTesterResults::CreateInstance( ioParams.GetContextRef(), results ) );
						results->Release();
					}
				}
				symTable->Release();
			}
		}
		languageSyntax->Release();
	}
}

void VJSLanguageSyntaxTester::_GetSymbol( XBOX::VJSParms_callStaticFunction &ioParams, void * )
{
	// We are expecting 4 JavaScript parameters:
	// - path to the symbol table
	// - symbol name
	// - symbol definition file
	// - symbol definition line number
	
	VString	symTablePathStr, symbolName, symbolDefPath;
	sLONG	symbolDefLineNumber;

	if (ioParams.CountParams() == 4) {
		if (!ioParams.GetStringParam( 1, symTablePathStr )) return;
		if (!ioParams.GetStringParam( 2, symbolName ))		return;
		if (!ioParams.GetStringParam( 3, symbolDefPath )) return;
		if (!ioParams.GetLongParam( 4, &symbolDefLineNumber ))		return;
	}
	else
		return;
	
	VFilePath symbolTablePath( symTablePathStr, FPS_POSIX);
	VFilePath symbolDefFilePath( symbolDefPath, FPS_POSIX);

	if ( ! symbolTablePath.IsValid() || ! symbolDefFilePath.IsValid() )
		return;

	CLanguageSyntaxComponent *languageSyntax = (CLanguageSyntaxComponent *)VComponentManager::RetainComponent( (CType)CLanguageSyntaxComponent::Component_Type );
	if (languageSyntax)
	{
		// First, load up the symbol table as an actual table instead of just a path string
		ISymbolTable *symTable = NULL;
		if (!symTablePathStr.IsEmpty())
		{
			symTable = languageSyntax->CreateSymbolTable();
			if (symTable)
			{
				VFile file(symbolTablePath);

				if (symTable->OpenSymbolDatabase( file ))
				{
					// Get symbol file
					std::vector< Symbols::IFile * > files = symTable->GetFilesByPathAndBaseFolder( symbolDefFilePath, eSymbolFileBaseFolderProject );	
					if ( ! files.empty() )
					{
						Symbols::IFile *file = files.front();
						file->Retain();
						for (std::vector< Symbols::IFile * >::iterator iter = files.begin(); iter != files.end(); ++iter)
							(*iter)->Release();

						// Gets symbol corresponding to given parameters
						Symbols::ISymbol* owner = symTable->GetSymbolOwnerByLine(file, symbolDefLineNumber);
						Symbols::ISymbol* sym = NULL;

						if (NULL != owner && owner->GetName() == symbolName)
						{
							sym = owner;
							sym->Retain();
						}
						
						if (NULL == sym)
						{
							std::vector< Symbols::ISymbol * >	syms = symTable->GetSymbolsByName(owner, symbolName,file);
							for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter)
							{
								if ( ( (*iter)->GetLineNumber() + 1 ) == symbolDefLineNumber )
								{
									sym = *iter;
									(*iter)->Retain();
								}
								(*iter)->Release();
							}	
						}

						if (NULL != sym)
						{
							VJSObject	result(ioParams.GetContextRef());
							VJSValue	jsval(ioParams.GetContextRef());
							VString		signature;

							symTable->GetSymbolSignature( sym, signature );

							result.MakeEmpty();

							jsval.SetString( sym->GetName() );
							result.SetProperty(L"name", jsval, JS4D::PropertyAttributeNone);

							jsval.SetString( sym->GetTypeName() );
							result.SetProperty(L"type", jsval, JS4D::PropertyAttributeNone);

							jsval.SetNumber( sym->GetLineNumber() + 1 );
							result.SetProperty(L"line", jsval, JS4D::PropertyAttributeNone);

							jsval.SetString( signature );
							result.SetProperty(L"signature", jsval, JS4D::PropertyAttributeNone);

							jsval.SetString( sym->GetKindString() );
							result.SetProperty(L"kind", jsval, JS4D::PropertyAttributeNone);

							ioParams.ReturnValue(result);
							sym->Release();
						}
						
						if (NULL != owner)
							owner->Release();
						file->Release();	
					}
				}
				symTable->Release();
			}
		}
		languageSyntax->Release();
	}
}

void VJSLanguageSyntaxTester::_Tokenize( XBOX::VJSParms_callStaticFunction &ioParams, void * )
{
	// We are expecting 2 JavaScript parameters:
	// - source code to tokenize
	// - extension of the fake file

	VString sourceCode, extension;

	if (ioParams.CountParams() == 2)
	{
		if (!ioParams.GetStringParam( 1, sourceCode ))	return;
		if (!ioParams.GetStringParam( 2, extension ))	return;
	}
	else
		return;

	CLanguageSyntaxComponent *languageSyntax = (CLanguageSyntaxComponent *)VComponentManager::RetainComponent( (CType)CLanguageSyntaxComponent::Component_Type );
	if (languageSyntax)
	{
		ISyntaxInterface *syntaxEngine = languageSyntax->GetSyntaxByExtension( extension );

		if (syntaxEngine)
		{
			VectorOfVString	tokens;

			syntaxEngine->GetTokensForTesting(sourceCode, tokens);

			VJSArray result( ioParams.GetContextRef() );
			VJSValue jsval(ioParams.GetContextRef());

			for (VectorOfVString::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
			{
				jsval.SetString(*it);
				result.PushValue(jsval);
			}
			ioParams.ReturnValue(result);
 		}
		languageSyntax->Release();
	}
}


void VJSLanguageSyntaxTester::_GotoDefinition( XBOX::VJSParms_callStaticFunction &ioParams, void * )
{
	VString filePathStr, symTablePathStr, selectionStr;
	sLONG	line = 0;

	if (ioParams.CountParams() == 4)
	{
		if (!ioParams.GetStringParam( 1, symTablePathStr )) return;
		if (!ioParams.GetStringParam( 2, filePathStr ))		return;
		if (!ioParams.GetLongParam( 3, &line ))				return;
		if (!ioParams.GetStringParam( 4, selectionStr ))	return;
	}
	else
		return;

	VFilePath symbolTablePath( symTablePathStr, FPS_POSIX);
	VFilePath filePath( filePathStr, FPS_POSIX);

	if ( ! symbolTablePath.IsValid() || ! filePath.IsValid() )
		return;

	if ( line < 1 )
		return;

	CLanguageSyntaxComponent *languageSyntax = (CLanguageSyntaxComponent *)VComponentManager::RetainComponent( (CType)CLanguageSyntaxComponent::Component_Type );
	if (languageSyntax)
	{
		// First, load up the symbol table as an actual table instead of just a path string
		ISymbolTable *symTable = NULL;
		if (!symTablePathStr.IsEmpty())
		{
			symTable = languageSyntax->CreateSymbolTable();
			if (symTable)
			{
				VFile file(symbolTablePath);

				if (symTable->OpenSymbolDatabase( file ))
				{
					// Now that we have all of the parameters grabbed, we can figure out which language syntax engine
					// we want to load up (based on the test file), and ask it to give us completions.  We can then take
					// those completions and turn them into an array of values to pass back to the caller.
					VString extension;
					filePath.GetExtension( extension );
					ISyntaxInterface *syntaxEngine = languageSyntax->GetSyntaxByExtension( extension );
					if (syntaxEngine)
					{
						// Now that we've got the syntax engine, we can ask it for the completions we need
						std::vector< IDefinition > definitions;
						syntaxEngine->GetDefinitionsForTesting( selectionStr, symTable, filePath.GetPath(), line, definitions );

						JSDefinitionResult *results = new JSDefinitionResult( definitions );
						ioParams.ReturnValue( VJSLanguageSyntaxTesterDefinitionResults::CreateInstance( ioParams.GetContextRef(), results ) );
						results->Release();
					}
				}
				symTable->Release();
			}
		}
		languageSyntax->Release();
	}
}


class ParsingWaiter : public VObject {
	
public:
	ParsingWaiter( IDocumentParserManager *inManager ) : fManager( inManager ), fParsingDone(false) {
		fManager->GetParsingCompleteSignal().Connect( this, VTask::GetCurrent(), &ParsingWaiter::ParsingComplete );
	}
	
	virtual ~ParsingWaiter() {
		fManager->GetParsingCompleteSignal().Disconnect( this );
	}

	void Wait()
	{
		while( !fParsingDone)
			VTask::ExecuteMessagesWithTimeout( 1000 );
	}

private:
	IDocumentParserManager	*fManager;
	bool					fParsingDone;

	virtual void ParsingComplete( VFilePath inPath, sLONG inStatus ) { fParsingDone = true; }
};

void VJSLanguageSyntaxTester::_ParseFile( XBOX::VJSParms_callStaticFunction &ioParams, void * )
{
	// The caller has passed us the path to the symbol table and the path to the file to be
	// parsed.  We want to parse that file and then return back to the caller once the
	// parsing is complete.
	
	// We are expecting 4 parameters.  One for the path to the symbol table,
	// one for the path to the test file
	if (ioParams.CountParams() < 4)	return;
#if VERSIONDEBUG == 114
	CDB4DManager* DB4D = CDB4DManager::RetainManager();
	DB4D->StartRecordingMemoryLeaks();
#endif

	VString symTablePathStr, testFilePathStr, testFileBaseFolderStr, testFileExecContextStr;

	if (!ioParams.GetStringParam( 1, symTablePathStr ))	return;
	if (!ioParams.GetStringParam( 2, testFilePathStr ))	return;
	if (!ioParams.GetStringParam( 3, testFileBaseFolderStr ))	return;
	if (!ioParams.GetStringParam( 4, testFileExecContextStr ))	return;

	VFilePath testFilePath( testFilePathStr, FPS_POSIX);
	
	if ( ! testFilePath.IsValid() )
		return;

	ESymbolFileBaseFolder	baseFolder;
	ESymbolFileExecContext	execContext;

	// Get file base folder
	if (testFileBaseFolderStr == CVSTR("project"))
		baseFolder = eSymbolFileBaseFolderProject;
	else if (testFileBaseFolderStr == CVSTR("jsf"))
		baseFolder = eSymbolFileBaseFolderStudio;
	else
		return;

	// Get file execution context
	if (testFileExecContextStr == CVSTR("client"))
		execContext = eSymbolFileExecContextClient;
	else if (testFileExecContextStr == CVSTR("server"))
		execContext = eSymbolFileExecContextServer;
	else if (testFileExecContextStr == CVSTR("both"))
		execContext = eSymbolFileExecContextClientServer;
	else
		return;

	VFilePath			symbolTablePath( symTablePathStr, FPS_POSIX);

	if ( ! symbolTablePath.IsValid() )
		return;
	
	CLanguageSyntaxComponent *languageSyntax = (CLanguageSyntaxComponent *)VComponentManager::RetainComponent( (CType)CLanguageSyntaxComponent::Component_Type );
	if (languageSyntax)
	{
		// First, load up the symbol table as an actual table instead of just a path string
		ISymbolTable *symTable = languageSyntax->CreateSymbolTable();
		if (symTable)
		{
			VFile file(symbolTablePath);

			if (symTable->OpenSymbolDatabase( file))
			{
				IDocumentParserManager *parserManager = languageSyntax->CreateDocumentParserManager();
				// Limit parser waiter scope as destructor will disconnect parsing signal
				{
					ParsingWaiter	waiter( parserManager);
					VString			extension;

					testFilePath.GetExtension(extension);
					if ( extension == CVSTR("waModel") )
					{
						VLanguageSyntaxTesterCatalog* catalog = new VLanguageSyntaxTesterCatalog( testFilePath );

						parserManager->ScheduleTask( (const void *)0xFEEDFACE, catalog, symTable, IDocumentParserManager::kPriorityAboveNormal);
						catalog->Release();
					}
					else
					{
						VSymbolFileInfos	fileInfos(testFilePath, baseFolder, execContext);

						parserManager->ScheduleTask( (const void *)0xFEEDFACE, /* Just needs to be unique per scheduler, but since this is a static method, there is no this pointer*/
													 fileInfos, symTable, IDocumentParserManager::kPriorityAboveNormal );
					}
					waiter.Wait();
				}
				parserManager->Release();
			}
			symTable->Release();
		}
		languageSyntax->Release();
	}
#if VERSIONDEBUG == 114
	VString sleaks;
	DB4D->DumpMemoryLeaks(sleaks);
	DebugMsg(sleaks);
	if (!sleaks.IsEmpty())
	{
		sLONG xdebug = 1;
	}
	DB4D->StopRecordingMemoryLeaks();
	DB4D->Release();
#endif
}

void VJSLanguageSyntaxTester::GetDefinition( ClassDefinition &outDefinition )
{
	static inherited::StaticFunction szFunctions [ ] =
	{
		{ "getSyntaxColoring",	js_callStaticFunction< _GetSyntaxColoring >,	JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getCompletions",		js_callStaticFunction< _GetCompletions >,		JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getSymbol",			js_callStaticFunction< _GetSymbol >,			JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "parseFile",			js_callStaticFunction< _ParseFile >,			JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "tokenize",			js_callStaticFunction< _Tokenize >,				JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "gotoDefinition",		js_callStaticFunction< _GotoDefinition >,		JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0 }
	};
	
	outDefinition.className = "LanguageSyntaxTester";
	outDefinition.initialize = js_initialize< Initialize >;
	outDefinition.finalize = js_finalize< Finalize >;
	outDefinition.staticFunctions = szFunctions;
}


void VJSLanguageSyntaxTesterResults::Initialize( const VJSParms_initialize &inParams, IJSLanguageSyntaxTesterResults *inResults )
{
	if (inResults)	inResults->Retain();
}

void VJSLanguageSyntaxTesterResults::Finalize( const VJSParms_finalize &inParams,  IJSLanguageSyntaxTesterResults *inResults )
{
	if (inResults)	inResults->Release();
}

void VJSLanguageSyntaxTesterResults::_GetResult( VJSParms_callStaticFunction &ioParams, IJSLanguageSyntaxTesterResults *inResults )
{
	if (inResults) {
		sLONG index;
		if (ioParams.GetLongParam( 1, &index )) {
			ioParams.ReturnString( inResults->GetResult( index ) );
		}
	}
}

void VJSLanguageSyntaxTesterResults::_GetResultCount( VJSParms_callStaticFunction &ioParams, IJSLanguageSyntaxTesterResults *inResults )
{
	if (inResults) {
		ioParams.ReturnNumber( inResults->GetResultCount() );
	}
}

void VJSLanguageSyntaxTesterResults::GetDefinition( ClassDefinition &outDefinition )
{
	static inherited::StaticFunction szFunctions [ ] =
	{
		{ "getResultCount", js_callStaticFunction< _GetResultCount >, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getResult", js_callStaticFunction< _GetResult >, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0 }
	};
	
	outDefinition.className = "LanguageSyntaxTesterResults";
	outDefinition.initialize = js_initialize< Initialize >;
	outDefinition.finalize = js_finalize< Finalize >;
	outDefinition.staticFunctions = szFunctions;
}

void VJSLanguageSyntaxTesterColorResults::Initialize( const VJSParms_initialize &inParams, IJSLanguageSyntaxTesterColorResults *inResults )
{
	if (inResults)	inResults->Retain();
}

void VJSLanguageSyntaxTesterColorResults::Finalize( const VJSParms_finalize &inParams,  IJSLanguageSyntaxTesterColorResults *inResults )
{
	if (inResults)	inResults->Release();
}

void VJSLanguageSyntaxTesterColorResults::_GetOffset( VJSParms_callStaticFunction &ioParams, IJSLanguageSyntaxTesterColorResults *inResults )
{
	if (inResults) {
		ioParams.ReturnNumber< size_t >( inResults->GetOffset() );
	}
}

void VJSLanguageSyntaxTesterColorResults::_GetLength( VJSParms_callStaticFunction &ioParams, IJSLanguageSyntaxTesterColorResults *inResults )
{
	if (inResults) {
		ioParams.ReturnNumber< size_t >( inResults->GetLength() );
	}
}

void VJSLanguageSyntaxTesterColorResults::_GetStyle( VJSParms_callStaticFunction &ioParams, IJSLanguageSyntaxTesterColorResults *inResults )
{
	if (inResults) {
		ioParams.ReturnNumber< sLONG >( inResults->GetStyle() );
	}
}

void VJSLanguageSyntaxTesterColorResults::_GetProperty( VJSParms_getProperty &ioParams, IJSLanguageSyntaxTesterColorResults *inResults )
{
	VString propName;
	if (ioParams.GetPropertyName( propName )) {
		sLONG result = -1;
		if (propName.EqualTo( CVSTR( "kColumnName" ), true ))			result = eColumnName;
		else if (propName.EqualTo( CVSTR( "kTableName" ), true ))		result = eTableName;
		else if (propName.EqualTo( CVSTR( "kComment" ), true ))			result = eComment;
		else if (propName.EqualTo( CVSTR( "kKeyword" ), true ))			result = eKeyword;
		else if (propName.EqualTo( CVSTR( "kNormal" ), true ))			result = eNormal;
		else if (propName.EqualTo( CVSTR( "kNumber" ), true ))			result = eNumber;
		else if (propName.EqualTo( CVSTR( "kString" ), true ))			result = eString;
		else if (propName.EqualTo( CVSTR( "kName" ), true ))				result = eName;
		else if (propName.EqualTo( CVSTR( "kComparison" ), true ))		result = eComparison;
		else if (propName.EqualTo( CVSTR( "kFunctionKeyword" ), true ))	result = eFunctionKeyword;
		else if (propName.EqualTo( CVSTR( "kDebug" ), true ))			result = eDebug;

		if (-1 != result)	ioParams.ReturnNumber( result );
	}	
}

void VJSLanguageSyntaxTesterColorResults::GetDefinition( ClassDefinition &outDefinition )
{
	static inherited::StaticFunction szFunctions [ ] =
	{
		{ "getOffset", js_callStaticFunction< _GetOffset >, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getLength", js_callStaticFunction< _GetLength >, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getStyle", js_callStaticFunction< _GetStyle >, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0 }
	};
	
	static inherited::StaticValue szProperties[] = 
	{
		{ "kColumnName", js_getProperty< _GetProperty >, NULL, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "kTableName", js_getProperty< _GetProperty >, NULL, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "kComment", js_getProperty< _GetProperty >, NULL, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "kKeyword", js_getProperty< _GetProperty >, NULL, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "kNormal", js_getProperty< _GetProperty >, NULL, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "kNumber", js_getProperty< _GetProperty >, NULL, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "kString", js_getProperty< _GetProperty >, NULL, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "kName", js_getProperty< _GetProperty >, NULL, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "kComparison", js_getProperty< _GetProperty >, NULL, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "kFunctionKeyword", js_getProperty< _GetProperty >, NULL, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "kDebug", js_getProperty< _GetProperty >, NULL, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0, 0 }
	};

	outDefinition.className = "LanguageSyntaxTesterColorResults";
	outDefinition.initialize = js_initialize< Initialize >;
	outDefinition.finalize = js_finalize< Finalize >;
	outDefinition.staticFunctions = szFunctions;
	outDefinition.staticValues = szProperties;
}

void VJSLanguageSyntaxTesterDefinitionResults::Initialize( const VJSParms_initialize &inParams, IJSLanguageSyntaxTesterDefinitionResults *inResults )
{
	if (inResults)	inResults->Retain();
}

void VJSLanguageSyntaxTesterDefinitionResults::Finalize( const VJSParms_finalize &inParams,  IJSLanguageSyntaxTesterDefinitionResults *inResults )
{
	if (inResults)	inResults->Release();
}

void VJSLanguageSyntaxTesterDefinitionResults::_GetResult( VJSParms_callStaticFunction &ioParams, IJSLanguageSyntaxTesterDefinitionResults *inResults )
{
	if (inResults) {
		sLONG index;
		if (ioParams.GetLongParam( 1, &index ))
		{
			IDefinition definition = inResults->GetResult( index );
			VJSObject	result(ioParams.GetContextRef());
			VJSValue	jsval(ioParams.GetContextRef());

			result.MakeEmpty();

			jsval.SetString( definition.GetName() );
			result.SetProperty(L"name", jsval, JS4D::PropertyAttributeNone);

			jsval.SetString( definition.GetFilePath() );
			result.SetProperty(L"file", jsval, JS4D::PropertyAttributeNone);

			jsval.SetNumber( definition.GetLineNumber() + 1 );
			result.SetProperty(L"line", jsval, JS4D::PropertyAttributeNone);

			ioParams.ReturnValue(result);
		}
	}
}

void VJSLanguageSyntaxTesterDefinitionResults::_GetResultCount( VJSParms_callStaticFunction &ioParams, IJSLanguageSyntaxTesterDefinitionResults *inResults )
{
	if (inResults) {
		ioParams.ReturnNumber( inResults->GetResultCount() );
	}
}

void VJSLanguageSyntaxTesterDefinitionResults::GetDefinition( ClassDefinition &outDefinition )
{
	static inherited::StaticFunction szFunctions [ ] =
	{
		{ "getResultCount", js_callStaticFunction< _GetResultCount >,	JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getResult",		js_callStaticFunction< _GetResult >,		JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0 }
	};
	
	outDefinition.className = "LanguageSyntaxTesterDefinitionResults";
	outDefinition.initialize = js_initialize< Initialize >;
	outDefinition.finalize = js_finalize< Finalize >;
	outDefinition.staticFunctions = szFunctions;
}


VLanguageSyntaxTesterCatalog::VLanguageSyntaxTesterCatalog(XBOX::VFilePath& inFilePath)
:fFilePath(inFilePath)
,fCatalogBag(NULL)
{
	VErrorDB4D err = VE_OK;

	if ( fFilePath.IsValid() )
	{
		fCatalogBag = new VValueBag();

		VFile catalogFile(fFilePath);

		if ( catalogFile.Exists() )
		{
			CDB4DManager*		dB4DBaseManager = CDB4DManager::RetainManager();
			if (NULL != dB4DBaseManager)
			{
				sLONG flags = DB4D_Open_As_XML_Definition | DB4D_Open_No_Respart | DB4D_Open_StructOnly | DB4D_Open_Allow_Temporary_Convert_For_ReadOnly;

				CDB4DBase* dB4DBase = dB4DBaseManager->OpenBase(catalogFile, flags, NULL /* outErr */, XBOX::FA_READ);
				if (NULL != dB4DBase)
				{
					CDB4DBaseContext* dB4DBaseContext = dB4DBase->NewContext(nil, nil);
					if (NULL != dB4DBaseContext)
					{
						std::vector<XBOX::VRefPtr<CDB4DEntityModel> > entityModels;

						err = dB4DBase->RetainAllEntityModels(entityModels, dB4DBaseContext, true);
						if (err == VE_OK)
						{
							for( std::vector<XBOX::VRefPtr<CDB4DEntityModel> >::iterator oneEntityModel = entityModels.begin() ; oneEntityModel != entityModels.end() ; ++oneEntityModel)
							{			
								VValueBag* resolvedRepresentation = (*oneEntityModel)->CreateDefinition();
								fCatalogBag->AddElement(LS_TESTER_CATALOG_DATACLASSES, resolvedRepresentation);
								ReleaseRefCountable( &resolvedRepresentation);
							}
						}
						dB4DBaseContext->Release();
					}
					//dB4DBase->Release();
					dB4DBase->CloseAndRelease();
				}
				dB4DBaseManager->Release();
			}
		}
	}
}

VLanguageSyntaxTesterCatalog::~VLanguageSyntaxTesterCatalog()
{
	ReleaseRefCountable( &fCatalogBag);
}

const VValueBag *VLanguageSyntaxTesterCatalog::RetainCatalogBag() const
{
	return RetainRefCountable( fCatalogBag);
}

void VLanguageSyntaxTesterCatalog::GetEntityModelNames(VectorOfVString& outEntityNames) const
{
	const VValueBag *catalogBag = RetainCatalogBag();
	if (catalogBag != NULL)
	{
		const VBagArray* bagModels = catalogBag->GetElements( LS_TESTER_CATALOG_DATACLASSES );
		VIndex count = (bagModels != NULL) ? bagModels->GetCount() : 0;
		for( VIndex i = 1 ; i <= count ; ++i)
		{
			const VValueBag* bagEntityModel = bagModels->GetNth(i);
			if (bagEntityModel)
			{
				VString name;
				bagEntityModel->GetString(LS_TESTER_CATALOG_NAME, name);
				outEntityNames.push_back( name);
			}
		}
	}
	ReleaseRefCountable( &catalogBag);
}

void VLanguageSyntaxTesterCatalog::GetEntityModelAttributes(const VString& inEntityName, std::vector<IEntityModelCatalogAttribute* >& outAttributes) const
{
	const VValueBag *catalogBag = RetainCatalogBag();
	if (catalogBag != NULL)
	{
		const VBagArray* bagModels = catalogBag->GetElements( LS_TESTER_CATALOG_DATACLASSES );
		VIndex count = (bagModels != NULL) ? bagModels->GetCount() : 0;
		for( VIndex i = 1 ; i <= count ; ++i)
		{
			const VValueBag* bagEntityModel = bagModels->GetNth(i);
			if (bagEntityModel)
			{
				VString entityName;
				bagEntityModel->GetString(LS_TESTER_CATALOG_NAME, entityName);
				if (entityName == inEntityName)
				{
					const VBagArray* bagEntityModelAttributes = bagEntityModel->GetElements( LS_TESTER_CATALOG_ATTRIBUTES );
					if (bagEntityModelAttributes)
					{
						for (sLONG j = 1; j <= bagEntityModelAttributes->GetCount(); ++j)
						{
							VString name, type;

							bagEntityModelAttributes->GetNth(j)->GetString( LS_TESTER_CATALOG_NAME, name);
							bagEntityModelAttributes->GetNth(j)->GetString( LS_TESTER_CATALOG_TYPE, type);
							
							outAttributes.push_back( new VLanguageSyntaxTesterCatalogAttribute(name, type) );
						}
					}
					break;
				}
			}
		}
	}
	ReleaseRefCountable( &catalogBag);
}


void VLanguageSyntaxTesterCatalog::GetEntityModelMethods(const VString& inEntityName, std::vector< IEntityModelCatalogMethod* >& outMethods) const
{
	const VValueBag *catalogBag = RetainCatalogBag();
	if (catalogBag != NULL)
	{
		const VBagArray* bagModels = catalogBag->GetElements( LS_TESTER_CATALOG_DATACLASSES );
		VIndex count = (bagModels != NULL) ? bagModels->GetCount() : 0;
		for( VIndex i = 1 ; i <= count ; ++i)
		{
			const VValueBag* bagEntityModel = bagModels->GetNth(i);
			if (bagEntityModel)
			{
				VString entityName;
				bagEntityModel->GetString(LS_TESTER_CATALOG_NAME, entityName);
				if (entityName == inEntityName)
				{
					const VBagArray* bag = bagEntityModel->GetElements( LS_TESTER_CATALOG_METHODS );
					if (bag)
					{
						for (sLONG j = 1; j <= bag->GetCount(); ++j)
						{
							VString name, applyTo;

							bag->GetNth(j)->GetString( LS_TESTER_CATALOG_NAME, name);
							bag->GetNth(j)->GetString( LS_TESTER_CATALOG_APPLYTO, applyTo);
								
							outMethods.push_back( new VLanguageSyntaxTesterCatalogMethod(name, applyTo) );
						}
					}
					break;
				}
			}
		}
	}
	ReleaseRefCountable( &catalogBag);
}

