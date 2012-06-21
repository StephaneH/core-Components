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

// The VSymbolTable class is actually an adapter that wraps around an InnerSymbolTable class.  The
// inner class is what actually performs all of the work.  The VSymbolTable class merely wraps an inner
// inner class and a context object.  We do this so that access to the symbol table can be thread-safe
// in an efficient manner -- the inner class is always handed a context object that is only used for a
// single thread, even if the actual underlying database is shared between threads.  If any APIs are added
// to the VSymbolTable interface, they should also be added to the InnerSymbolTable interface and actually
// implemented there; taking their context object as a parameter.
class VSymbolTable : public ISymbolTable, public VObject {
friend class SymbolCollectionManager;

private:
	class InnerSymbolTable : public IRefCountable {
	friend class SymbolCollectionManager;

	private:
		const sLONG kCurrentVersion;

		// This field is retrieved from the management database, and updated before we close
		// the symbol table down.  We use it to service requests for symbol IDs.
		uLONG fNextUniqueSymbolID;

		CDB4DManager *fDatabaseManager;
		CDB4DBase *fDatabase;
		VSyncEvent	fAllowedToAddSymbolsEvent;

		std::map< sLONG, Symbols::ISymbol * > fRecursionSet;
		VCriticalSection *fRecursionTaskLock;

		bool InitializeNewDatabase( CDB4DBaseContext *inContext );

		Symbols::ISymbol *SymbolFromRecord( CDB4DTable *inTable, CDB4DRecord *inRecord, CDB4DBaseContext *inContext, bool inFullyLoad = true );

		Symbols::IExtraInfo *ExtraInfoFromRecord( CDB4DTable *inTable, CDB4DRecord *inRecord, CDB4DBaseContext *inContext );
		bool RecordFromExtraInfo( const Symbols::ISymbol *inOwner, const Symbols::IExtraInfo *inExtraInfo, CDB4DRecord *ioRecord, CDB4DBaseContext *inContext );

		bool RecordFromFile( Symbols::IFile *inFile, CDB4DRecord *ioRecord, CDB4DBaseContext *inContext );
		Symbols::IFile *FileFromRecord( CDB4DTable *inTable, CDB4DRecord *inRecord, CDB4DBaseContext *inContext );
		Symbols::IFile *GetFileByID( sLONG inID, CDB4DBaseContext *inContext );

		std::vector< Symbols::ISymbol * > GetSymbolsByNameHelper( sLONG inOwnerID, const VString &inName, sLONG inOwnerFileID, CDB4DBaseContext *inContext );

		VString GetStringField( CDB4DRecord *inRecord, CDB4DField *inField );
		sLONG GetIntegerField( CDB4DRecord *inRecord, CDB4DField *inField, bool *isNull );
		uLONG8 GetLongIntegerField( CDB4DRecord *inRecord, CDB4DField *inField ); 

		CDB4DRecord *FindOrCreateRecordByID( CDB4DTable *inTable, Symbols::ISymbol *inSymbol, CDB4DBaseContext *inContext );

		bool OpenDataFile( const VFile &inDatabaseFile, CDB4DBaseContext *inContext );

		bool DeleteDatabaseOnDisk( const VFile &inDatabaseFile );

		bool CheckRequiredStructure( CDB4DBaseContext *inContext );

		virtual void CheckUpdatesAllowed() { fAllowedToAddSymbolsEvent.Lock(); }
	public:

		// FM : try to optimize access to symbol table by using a sync event
		// so it will lock all updates to database by all the threads parsing files
		// when a user action requires some suggestions.
		virtual void LockUpdates() { fAllowedToAddSymbolsEvent.Reset(); }
		virtual void UnlockUpdates() { fAllowedToAddSymbolsEvent.Unlock(); }
		// -----


		InnerSymbolTable();
		virtual ~InnerSymbolTable();

		CDB4DBaseContext *GetContextForNewThread();
		uLONG GetNextUniqueID() { return (uLONG)VInterlocked::Increment( (sLONG *)&fNextUniqueSymbolID ); }

		virtual bool OpenSymbolDatabase( const VFile &inDatabaseFile, CDB4DBaseContext **outContext );
		virtual bool AddFiles( std::vector< Symbols::IFile * > inFiles, CDB4DBaseContext *inContext );
		virtual bool AddSymbols( std::vector< Symbols::ISymbol * > inSymbols, CDB4DBaseContext *inContext );

		virtual bool UpdateFile( Symbols::IFile *inFile, CDB4DBaseContext *inContext );
		virtual bool UpdateSymbol( Symbols::ISymbol *inSym, CDB4DBaseContext *inContext );

		virtual std::vector< Symbols::ISymbol * > GetSymbolsByName( const VString &inName, CDB4DBaseContext *inContext );
		virtual std::vector< Symbols::ISymbol * > GetSymbolsByName( Symbols::ISymbol *inOwner, const VString &inName, Symbols::IFile *inOwnerFile, CDB4DBaseContext *inContext );
		virtual std::vector< Symbols::ISymbol * > GetNamedSubSymbols( Symbols::ISymbol *inOwner, Symbols::IFile *inOwnerFile, SortingOptions inOptions, CDB4DBaseContext *inContext );
		virtual std::vector< Symbols::ISymbol * > GetSymbolsForOutline( Symbols::ISymbol *inOwner, Symbols::IFile *inOwnerFile, SortingOptions inOptions, CDB4DBaseContext *inContext );
		virtual std::vector< Symbols::ISymbol * > GetLikeNamedSubSymbols( Symbols::ISymbol *inOwner, const VString &inLikeName, CDB4DBaseContext *inContext, const bool inFullyLoad = false );
		virtual std::vector< Symbols::ISymbol * > GetClassSymbols( Symbols::ISymbol *inOwner, const VString &inName, Symbols::IFile *inOwnerFile, CDB4DBaseContext *inContext );
		virtual std::vector< Symbols::IFile * > GetFilesByPathAndBaseFolder( const VString &inName, const ESymbolFileBaseFolder inBaseFolder, CDB4DBaseContext *inContext );
		virtual std::vector< Symbols::IFile * > GetFilesByName( const VString &inName, CDB4DBaseContext *inContext );

		virtual Symbols::ISymbol *GetSymbolOwnerByLine( Symbols::IFile *inOwnerFile, int inLineNumber, CDB4DBaseContext *inContext );
		virtual Symbols::ISymbol *GetSymbolByID( sLONG inID, CDB4DBaseContext *inContext );

		virtual bool DeleteSymbol( Symbols::ISymbol * inSymbol, CDB4DBaseContext *inContext );
		virtual bool DeleteFile( Symbols::IFile *inFile, CDB4DBaseContext *inContext );
		virtual bool DeleteSymbolsForFile( Symbols::IFile *inFile, CDB4DBaseContext *inContext );

		virtual sLONG CountFiles( CDB4DBaseContext *inContext );

		virtual std::vector< Symbols::IExtraInfo * > GetExtraInformation( const Symbols::ISymbol *inOwnerSymbol, Symbols::IExtraInfo::Kind inKind, CDB4DBaseContext *inContext );
		virtual bool AddExtraInformation( const Symbols::ISymbol *inOwnerSymbol, const Symbols::IExtraInfo *inInfo, CDB4DBaseContext *inContext );
	};

	InnerSymbolTable *fTable;
	CDB4DBaseContext *fContext;
	VString fTableFolder;

	// This was pulled wholesale out of VSolution -- too bad it's not part of VFilePath!!
	void GetPathRelativeToFolderPath(const VString& inFolderPath, const VString& inFullPath, const ESymbolFileBaseFolder inBaseFolder, VString& outRelativePath) const
	{
		if (inFolderPath == inFullPath)
		{
			outRelativePath = "./";
		}
		else
		{	
			// Relative path calculation for Project/Solution/Server files
			if (eSymbolFileBaseFolderProject == inBaseFolder || eSymbolFileBaseFolderSolution == inBaseFolder || eSymbolFileBaseFolderServer == inBaseFolder)
			{
				VString baseFolder(inFolderPath);
				outRelativePath = inFullPath;
				sLONG start = 1;
				if (baseFolder.GetUniChar(start) == outRelativePath.GetUniChar(start))
				{
					sLONG baseFolderLength  = baseFolder.GetLength();
					sLONG outRelativePathLength = outRelativePath.GetLength();
					sLONG min = baseFolderLength;
					if (outRelativePathLength < min)
						min = outRelativePathLength;
					while ((start <= min) && (baseFolder.GetUniChar(start) == outRelativePath.GetUniChar(start))) start++;
					sLONG pos = baseFolder.FindUniChar('/', start - 1, true);
					start = pos + 1;
					sLONG nbSeparator = 0;
					for (sLONG j = start; j <= baseFolderLength; j++)
					{
						if (baseFolder.GetUniChar(j) == '/')
							nbSeparator++;
					}
					VString prefix;
					if (baseFolderLength > start)
					{
						for (sLONG j = 0; j < nbSeparator; j++)
						{
							prefix += "..";
							prefix += "/";
						}
					}
					else
					{
						prefix += ".";
						prefix += "/";
					}
					if (start <= outRelativePath.GetLength())
					{
						outRelativePath.SubString(start, outRelativePathLength - start + 1);
						outRelativePath = prefix + outRelativePath;
					}
					else
						outRelativePath = prefix;
				}
			}
			// Relative path calculation for Studio files
			else if (eSymbolFileBaseFolderStudio == inBaseFolder)
			{
				VFilePath	appPath = VProcess::Get()->GetExecutableFolderPath();
				VString		appPathStr;

				appPath.GetPosixPath(appPathStr);

				outRelativePath = inFullPath;
				outRelativePath.Replace(CVSTR("./"), 1, appPathStr.GetLength());
			}

		}

	}
public:
	VSymbolTable() : fTable( NULL ), fContext( NULL ) {}
	virtual ~VSymbolTable() { 
		if (fContext)	fContext->Release();
		if (fTable)		fTable->Release(); 
	}

	virtual void LockUpdates() {fTable->LockUpdates();}
	virtual void UnlockUpdates() {fTable->UnlockUpdates();}

	virtual void GetRelativePath( const XBOX::VFilePath &inFilePath, const ESymbolFileBaseFolder inBaseFolder, XBOX::VString &outFilePath ) {
		VString convertedPath;
		inFilePath.GetPosixPath( convertedPath );
		GetPathRelativeToFolderPath( fTableFolder, convertedPath, inBaseFolder, outFilePath );
	}

	virtual ISymbolTable *CloneForNewThread() { 
		VSymbolTable *ret = new VSymbolTable(); 
		ret->fTable = fTable;
		ret->fTableFolder = fTableFolder;
		if (fTable)	{
			fTable->Retain();		// Because a new table shares ownership of this
			ret->fContext = fTable->GetContextForNewThread();
		}
		return ret; 
	}

	virtual uLONG GetNextUniqueID() {
		if (fTable) {
			return fTable->GetNextUniqueID();
		} else {
			return 0;
		}
	}

	virtual bool OpenSymbolDatabase( const VFile &inDatabaseFile ) {
		xbox_assert( !fTable );

		// Grab the parent folder for the database
		VFolder *parent = inDatabaseFile.RetainParentFolder();
		parent->GetPath().GetPosixPath( fTableFolder );
		parent->Release();

		fTable = new InnerSymbolTable();
		return fTable->OpenSymbolDatabase( inDatabaseFile, &fContext );
	}

	virtual bool AddFiles( std::vector< Symbols::IFile * > inFiles ) {
		if (fTable) {
			return fTable->AddFiles( inFiles, fContext );
		}
		return false;
	}
	virtual bool AddSymbols( std::vector< Symbols::ISymbol * > inSymbols ) {
		if (fTable) {
			return fTable->AddSymbols( inSymbols, fContext );
		}
		return false;
	}

	virtual bool UpdateFile( Symbols::IFile *inFile ) {
		if (fTable) {
			return fTable->UpdateFile( inFile, fContext );
		}
		return false;
	}

	virtual bool UpdateSymbol( Symbols::ISymbol * inSymbol ) {
		if (fTable) {
			return fTable->UpdateSymbol( inSymbol, fContext );
		}
		return false;
	}

	virtual std::vector< Symbols::ISymbol * > GetSymbolsByName( const VString &inName ) {
		if (fTable) {
			return fTable->GetSymbolsByName( inName, fContext );
		}
		std::vector< Symbols::ISymbol * > ret;
		return ret;
	}
	virtual std::vector< Symbols::ISymbol * > GetSymbolsByName( Symbols::ISymbol *inOwner, const VString &inName, Symbols::IFile *inOwnerFile = NULL ) {
		if (fTable) {
			return fTable->GetSymbolsByName( inOwner, inName, inOwnerFile, fContext );
		}
		std::vector< Symbols::ISymbol * > ret;
		return ret;
	}
	
	virtual std::vector< Symbols::ISymbol * > GetClassSymbols( Symbols::ISymbol *inOwner, const VString &inName, Symbols::IFile *inOwnerFile = NULL ) {
		if (fTable) {
			return fTable->GetClassSymbols( inOwner, inName, inOwnerFile, fContext );
		}
		std::vector< Symbols::ISymbol * > ret;
		return ret;
	}
	virtual std::vector< Symbols::ISymbol * > GetNamedSubSymbols( Symbols::ISymbol *inOwner, Symbols::IFile *inOwnerFile = NULL, SortingOptions inOptions = kDefault ) {
		if (fTable) {
			return fTable->GetNamedSubSymbols( inOwner, inOwnerFile, inOptions, fContext );
		}
		std::vector< Symbols::ISymbol * > ret;
		return ret;
	}

	virtual std::vector< Symbols::ISymbol * > GetSymbolsForOutline( Symbols::ISymbol *inOwner, Symbols::IFile *inOwnerFile, SortingOptions inOptions ) {
		if (fTable) {
			return fTable->GetSymbolsForOutline( inOwner, inOwnerFile, inOptions, fContext );
		}
		std::vector< Symbols::ISymbol * > ret;
		return ret;
	}

	virtual std::vector< Symbols::ISymbol * > GetLikeNamedSubSymbols( Symbols::ISymbol *inOwner, const VString &inLikeName, const bool inFullyLoad ) {
		if (fTable) {
			return fTable->GetLikeNamedSubSymbols( inOwner, inLikeName, fContext, inFullyLoad );
		}
		std::vector< Symbols::ISymbol * > ret;
		return ret;
	}
	virtual std::vector< Symbols::IFile * > GetFilesByPathAndBaseFolder( const VFilePath &inPath, const ESymbolFileBaseFolder inBaseFolder ) {
		// Convert the input path into a relative file path
		VString path;
		GetRelativePath(inPath, inBaseFolder, path );
		if (fTable) {
			return fTable->GetFilesByPathAndBaseFolder( path, inBaseFolder, fContext );
		}
		std::vector< Symbols::IFile * > ret;
		return ret;
	}
	virtual std::vector< Symbols::IFile * > GetFilesByName( const VString &inName ) {
		if (fTable) {
			return fTable->GetFilesByName( inName, fContext );
		}
		std::vector< Symbols::IFile * > ret;
		return ret;
	}

	virtual Symbols::ISymbol *GetSymbolOwnerByLine( Symbols::IFile *inOwnerFile, int inLineNumber ) {
		if (fTable) {
			return fTable->GetSymbolOwnerByLine( inOwnerFile, inLineNumber, fContext );
		}
		return NULL;
	}
	virtual Symbols::ISymbol *GetSymbolByID( sLONG inID ) {
		if (fTable) {
			return fTable->GetSymbolByID( inID, fContext );
		}
		return NULL;
	}

	virtual bool DeleteSymbol( Symbols::ISymbol * inSymbol ) {
		if (fTable) {
			return fTable->DeleteSymbol( inSymbol, fContext );
		}
		return false;
	}
	virtual bool DeleteFile( Symbols::IFile *inFile ) {
		if (fTable) {
			return fTable->DeleteFile( inFile, fContext );
		}
		return false;
	}
	virtual bool DeleteSymbolsForFile( Symbols::IFile *inFile ) {
		if (fTable) {
			return fTable->DeleteSymbolsForFile( inFile, fContext );
		}
		return false;
	}

	virtual sLONG CountFiles() {
		if (fTable) {
			return fTable->CountFiles( fContext );
		}
		return 0;
	}

	virtual std::vector< Symbols::IExtraInfo * > GetExtraInformation( const Symbols::ISymbol *inOwnerSymbol, Symbols::IExtraInfo::Kind inKind ) {
		if (fTable) {
			return fTable->GetExtraInformation( inOwnerSymbol, inKind, fContext );
		}
		std::vector< Symbols::IExtraInfo * > ret;
		return ret;
	}
	virtual bool AddExtraInformation( const Symbols::ISymbol *inOwnerSymbol, const Symbols::IExtraInfo *inInfo ) {
		if (fTable) {
			return fTable->AddExtraInformation( inOwnerSymbol, inInfo, fContext );
		}
		return false;
	}
};

// This section is devoted to named entities in the database.
const char * kSymbolTable = "Symbols";					// Table
	const char * kSymbolID = "UniqueID";					// Int32, Unique, Auto-Sequence, Non-Null, Primary Key
	const char * kSymbolName = "Name";						// String
	const char * kSymbolFileID = "FileID";					// Int32, Non-Null
	const char * kSymbolPrototypeID = "PrototypeID";		// String, Comma-separated list of Int32 IDs
	const char * kSymbolOwnerID = "OwnerID";				// Int32
	const char * kSymbolKind = "Kind";						// Int16
	const char * kSymbolWAFKind = "WAFKind";				// String
	const char * kSymbolScriptDocText = "ScriptDocText";	// Text
	const char * kLineNumber = "LineNumber";				// Int32
	const char * kLineCompletionNumber = "LineCompletionNumber";		// Int32
	const char * kSymbolReturnTypeID = "ReturnTypeID";		// String, Comma-separated list of Int32 IDs
	const char * kSymbolReferenceID = "ReferenceID";		// Int32

const char * kFileTable = "Files";						// Table
	const char * kFileID = "UniqueID";						// Int32, Unique, Auto-Sequence, Non-Null, Primary Key
	const char * kFilePath = "Path";						// String, Non-Null
	const char * kFileBaseFolder = "BaseFolder";			// Int16
	const char * kFileExecContext = "ExecutionContext";		// Int16
	const char * kFileModificationTime = "ModificationTimestamp";		// UInt64, Non-Null
	
	

const char * kExtrasTable = "ExtraInfo";				// Table
	const char * kExtrasOwnerSymbolID = "OwnerID";			// Int32, Non-Null, Primary Key
	const char * kExtrasKind = "Kind";						// Int32, Non-Null
	const char * kExtrasStringData = "StringData";			// String
	const char * kExtrasIntegerData = "IntegerData";		// Int32

const char * kVersionTable = "Version";					// Table
	const char * kVersionNumber = "VersionNumber";			// Int32, Non-Null

class SymbolCollectionManager : public DB4DCollectionManager {
private:
	const std::vector< Symbols::ISymbol * > &fSymbolList;
	CDB4DTable *fTable;
	CDB4DBaseContext *fContext;
	CDB4DField *fSymbolIDField, *fSymbolNameField, *fSymbolFileIDField, *fSymbolPrototypeField,
				*fSymbolOwnerField,	*fSymbolKindField, *fSymbolWAFKindField,*fSymbolScriptDocField, 
				*fSymbolLineNumberField, *fSymbolLineCompletionNumberField, *fSymbolReturnTypeField, *fReferenceIDField;
	
public:
	SymbolCollectionManager( const std::vector< Symbols::ISymbol * > &inSymbolList, CDB4DTable *in4DTable, CDB4DBaseContext *inContext ) : 
		fSymbolList( inSymbolList ), fTable( in4DTable ), fContext( inContext ) {
		fSymbolIDField = fTable->FindAndRetainField( kSymbolID, inContext );
		fSymbolNameField = fTable->FindAndRetainField( kSymbolName, inContext );
		fSymbolFileIDField = fTable->FindAndRetainField( kSymbolFileID, inContext );
		fSymbolPrototypeField = fTable->FindAndRetainField( kSymbolPrototypeID, inContext );
		fSymbolOwnerField = fTable->FindAndRetainField( kSymbolOwnerID, inContext );
		fSymbolKindField = fTable->FindAndRetainField( kSymbolKind, inContext );
		fSymbolWAFKindField = fTable->FindAndRetainField( kSymbolWAFKind, inContext );
		fSymbolScriptDocField = fTable->FindAndRetainField( kSymbolScriptDocText, inContext );
		fSymbolLineNumberField = fTable->FindAndRetainField( kLineNumber, inContext );
		fSymbolLineCompletionNumberField = fTable->FindAndRetainField( kLineCompletionNumber, inContext );
		fSymbolReturnTypeField = fTable->FindAndRetainField( kSymbolReturnTypeID, inContext );
		fReferenceIDField = fTable->FindAndRetainField( kSymbolReferenceID, inContext );
	}
	virtual ~SymbolCollectionManager() {
		fSymbolIDField->Release();
		fSymbolNameField->Release();
		fSymbolFileIDField->Release();
		fSymbolPrototypeField->Release();
		fSymbolOwnerField->Release();
		fSymbolKindField->Release();
		fSymbolWAFKindField->Release();
		fSymbolScriptDocField->Release();
		fSymbolLineNumberField->Release();
		fSymbolLineCompletionNumberField->Release();
		fSymbolReturnTypeField->Release();
		fReferenceIDField->Release();
	}

	virtual VErrorDB4D SetCollectionSize(RecIDType size, Boolean ClearData = true) { return VE_DB4D_NOTIMPLEMENTED; }
	virtual RecIDType GetCollectionSize() { 
		return fSymbolList.size(); 
	}
	virtual sLONG GetNumberOfColumns() {
		return 12;	// One for each of the columns on the symbol
	}
	virtual	bool AcceptRawData() { return false; }
	virtual CDB4DField* GetColumnRef(sLONG ColumnNumber) {
		// Column numbering starts from 1
		switch (ColumnNumber) {
			case 1:	return fSymbolIDField;
			case 2:	return fSymbolNameField;
			case 3:	return fSymbolFileIDField;
			case 4:	return fSymbolPrototypeField;
			case 5:	return fSymbolOwnerField;
			case 6:	return fSymbolKindField;
			case 7:	return fSymbolWAFKindField;
			case 8:	return fSymbolScriptDocField;
			case 9:	return fSymbolLineNumberField;
			case 10: return fSymbolLineCompletionNumberField;
			case 11: return fSymbolReturnTypeField;
			case 12: return fReferenceIDField;
		}
		return NULL;
	}
	virtual VErrorDB4D SetNthElement(RecIDType ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle& inValue) { return VE_DB4D_NOTIMPLEMENTED; }
	virtual	XBOX::VSize GetNthElementSpace( RecIDType inIndex, sLONG inColumnNumber, XBOX::ValueKind inRawValueKind, bool *outRejected, XBOX::VError& outError) { return 0; }
	virtual	void* WriteNthElementToPtr( RecIDType inIndex, sLONG inColumnNumber, void *outRawValue, XBOX::ValueKind inRawValueKind, bool *outRejected, XBOX::VError& outError) { return NULL; }
	virtual VErrorDB4D SetNthElementRawData(RecIDType ElemNumber, sLONG ColumnNumber, const void *inRawValue, XBOX::ValueKind inRawValueKind, bool *outRejected) { return VE_DB4D_NOTIMPLEMENTED; }
	virtual VErrorDB4D GetNthElement(RecIDType ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle*& outValue, bool *outDisposeIt) {
		if (VTask::GetCurrent()->IsDying()) {
			// If our thread has been asked to die, we're going to bail out as quickly as possible, so we
			// will just fail at the task of saving out elements.
			return VE_DB4D_CANNOTSAVERECORD;
		}

		// Element numbers start at 1, as do column numbers
		Symbols::ISymbol *sym = fSymbolList[ ElemNumber - 1 ];
		outValue = NULL;
		switch (ColumnNumber) {
			case 1:	{
				if (testAssert( sym->HasID() )) {
					outValue = new VLong( sym->GetID() );
				}
			} break;
			case 2: {
				outValue = new VString( sym->GetName() );
			} break;
			case 3: {
				Symbols::IFile *file = sym->GetFile();
				if (file) {
					outValue = new VLong( file->GetID() );
				}
			} break;
			case 4: {
				std::vector< Symbols::ISymbol * > protos = sym->GetPrototypes();
				if (!protos.empty()) {
					VectorOfVString parts;
					for (std::vector< Symbols::ISymbol * >::iterator iter = protos.begin(); iter != protos.end(); ++iter) {
						VString id;
						id.FromLong( (*iter)->GetID() );
						parts.push_back( id );
					}
					outValue = new VString();
					((VString *)outValue )->Join( parts, ',' );
				}
			} break;
			case 5: {
				Symbols::ISymbol *owner = sym->GetOwner();
				if (owner  && owner->HasID() ) {
					outValue = new VLong( owner->GetID() );
				}
			} break;
			case 6: {
				outValue = new VLong( sym->GetFullKindInformation() );
			} break;
			case 7: {
				outValue = new VString( sym->GetWAFKind() );
			} break;
			case 8: {
				outValue = new VString( sym->GetScriptDocComment() );
			} break;
			case 9: {
				outValue = new VLong( sym->GetLineNumber() );
			} break;
			case 10:	{
				outValue = new VLong( sym->GetLineCompletionNumber() );
			} break;
			case 11: {
				std::vector< Symbols::ISymbol * > protos = sym->GetReturnTypes();
				if (!protos.empty()) {
					VectorOfVString parts;
					for (std::vector< Symbols::ISymbol * >::iterator iter = protos.begin(); iter != protos.end(); ++iter) {
						VString id;
						id.FromLong( (*iter)->GetID() );
						parts.push_back( id );
					}
					outValue = new VString();
					((VString *)outValue )->Join( parts, ',' );
				}
			} break;
			case 12: {
				if (sym->GetAuxillaryKindInformation() & Symbols::ISymbol::kReferenceValue) {
					const Symbols::ISymbol *ref = sym->Dereference();
					sLONG id = ref->GetID();
					ref->Release();
					xbox_assert( sym->GetID() != id );
					outValue = new VLong( id );
				}
			} break;
			default: return VE_DB4D_INVALIDRECORD;
		}
		*outDisposeIt = (outValue != NULL);
		return VE_OK;
	}
	virtual VErrorDB4D AddOneElement(sLONG ColumnNumber, const XBOX::VValueSingle& inValue) { return VE_DB4D_NOTIMPLEMENTED; }
	virtual sLONG GetSignature() { return 0; }
	virtual XBOX::VError PutInto(XBOX::VStream& outStream) { return VE_DB4D_NOTIMPLEMENTED; }
};

VSymbolTable::InnerSymbolTable::InnerSymbolTable() : fDatabaseManager( NULL ), fDatabase( NULL ), fNextUniqueSymbolID( 0 ), 
	kCurrentVersion( 6 )
{
	fDatabaseManager = VComponentManager::RetainComponentOfType< CDB4DManager >();
	fRecursionTaskLock = new VCriticalSection();
	fAllowedToAddSymbolsEvent.Unlock();	// we are allowed to add symbols again
}

VSymbolTable::InnerSymbolTable::~InnerSymbolTable()
{
	if (fDatabase)			fDatabase->CloseAndRelease();
	if (fDatabaseManager)	fDatabaseManager->Release();

	delete fRecursionTaskLock;
}

CDB4DBaseContext *VSymbolTable::InnerSymbolTable::GetContextForNewThread()
{
	if (!fDatabase)	return NULL;
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
	CDB4DTable *versionTable = fDatabase->FindAndRetainTable( kVersionTable, inContext );
	if (versionTable) {
		CDB4DRecord *rec = versionTable->LoadRecord( 0, DB4D_Do_Not_Lock, inContext );
		if (rec) {
			CDB4DField *versionField = versionTable->FindAndRetainField( kVersionNumber, inContext );
			if (versionField) {
				sLONG version = GetIntegerField( rec, versionField, NULL );
				if (version < kCurrentVersion)	ret = false;
				versionField->Release();
			} else ret = false;
			rec->Release();
		} else ret = false;
		versionTable->Release();
	} else ret = false;

	CDB4DTable *symbolsTable = fDatabase->FindAndRetainTable( kSymbolTable, inContext );
	if (symbolsTable) {
		CDB4DField *referencesField = symbolsTable->FindAndRetainField( kSymbolReferenceID, inContext );
		if (referencesField)	referencesField->Release();
		else					ret = false;

		symbolsTable->Release();
	} else ret = false;

	return ret;
}

bool VSymbolTable::InnerSymbolTable::DeleteDatabaseOnDisk( const VFile &inDatabaseFile )
{
	bool ret = true;
	if (inDatabaseFile.Exists()) {
		ret &= (VE_OK == inDatabaseFile.Delete());
	}

	VFilePath path = inDatabaseFile.GetPath();
	path.SetExtension( RIAFileKind::kSymbolDataFileExtension );
	VFile dataFile( path );
	if (dataFile.Exists()) {
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
	if (dataFile.Exists()) {
		// We want to open the data file for the user
		if (!fDatabase->OpenData( dataFile, DB4D_Open_Convert_To_Higher_Version, inContext ))
			ret = false;
	} else {
		if (!fDatabase->CreateData( dataFile, 0, NULL, inContext ))
			ret = false;
	}
	
	return ret;
}

bool VSymbolTable::InnerSymbolTable::OpenSymbolDatabase( const VFile &inDatabaseFile, CDB4DBaseContext **outContext )
{
	if (!fDatabaseManager)	return false;

	// We want to keep any errors previously on the stack, but we don't want to add any new ones, since
	// we handle failure cases internally and retry manually.
	StErrorContextInstaller errorFilter( false );

	// Check to see if there is a file that already exists at the path given.  If
	// there is one, we assume it is the database the user wants.  If there's not
	// one, then we need to create a new database for the caller.
	if (inDatabaseFile.Exists()) {
		// The file exists, so let's open it
		fDatabase = fDatabaseManager->OpenBase( inDatabaseFile, DB4D_Open_Convert_To_Higher_Version );

		// Let's see if we were able to get a handle to the database
		if (!fDatabase)	{
			// We have a database, but we can't actually open it.  Let's see if we can delete
			// the database file and try again.
			if (DeleteDatabaseOnDisk( inDatabaseFile )) {
				return OpenSymbolDatabase( inDatabaseFile, outContext );
			} else {
				return false;
			}
		}

		*outContext = fDatabase->NewContext( NULL, NULL );

		// The datafile needs to be opened before we can perform certain operations
		if (!OpenDataFile( inDatabaseFile, *outContext ) || !CheckRequiredStructure( *outContext )) {
			(*outContext)->Release();
			fDatabase->CloseAndRelease();
			fDatabase = NULL;

			// Let's try to recover by deleting the database and structure file both, and
			// starting over.
			if (DeleteDatabaseOnDisk( inDatabaseFile )) {
				return OpenSymbolDatabase( inDatabaseFile, outContext );
			} else {
				return false;
			}
		}
	} else {
		// The file doesn't exist, so let's make a new one
		fDatabase = fDatabaseManager->CreateBase( inDatabaseFile, DB4D_Create_AllDefaultParamaters, VIntlMgr::GetDefaultMgr() );

		// Let's see if we were able to get a handle to the database
		if (!fDatabase)	return false;

		// Create a new context for this database
		*outContext = fDatabase->NewContext( NULL, NULL );

		// The datafile needs to be opened before we can perform certain operations
		if (!OpenDataFile( inDatabaseFile, *outContext )) {
			fDatabase->Release();
			(*outContext)->Release();
			fDatabase = NULL;
			return false;
		}

		// We need to initialize this database so that it's structure is correct
		if (!InitializeNewDatabase( *outContext )) {
			fDatabase->Release();
			(*outContext)->Release();
			fDatabase = NULL;
			return false;
		}
	}

	// Now that we've got a database opened, we're going to set the temp folder to the system's temp directory,
	// so that large databases do not trigger project refreshes constantly.
	fDatabase->SetTemporaryFolderSelector( DB4D_SystemTemporaryFolder );

	// We're going to take a look at the symbols table and try to find the greatest symbol
	// id in the table.  That will tell us our next unique symbol ID.  We default to zero in
	// the case where there are no records in the symbols table.
	CDB4DTable *symbolsTable = fDatabase->FindAndRetainTable( kSymbolTable, *outContext );
	if (symbolsTable) {
		fNextUniqueSymbolID = 0;
		CDB4DSelection *allRecords = symbolsTable->SelectAllRecords( *outContext );
		if (allRecords) {
			// We want the information in descending order so that we can load the first record as it will
			// have the max ID
			if (allRecords->CountRecordsInSelection( *outContext ) > 0) {
				CDB4DField *idField = symbolsTable->FindAndRetainField( kSymbolID, *outContext );
				if (idField) {
					allRecords->SortSelection( idField->GetID( *outContext ), false, NULL, *outContext );
					CDB4DRecord *record = allRecords->LoadSelectedRecord( 1, DB4D_Do_Not_Lock, *outContext );
					if (record) {
						fNextUniqueSymbolID = GetIntegerField( record, idField, NULL );
						record->Release();
					}
					idField->Release();
				}
			}
			allRecords->Release();
		}
		symbolsTable->Release();
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
	CDB4DTable *symbolsTable = fDatabase->CreateOutsideTable( kSymbolTable, inContext );
	if (!symbolsTable)	return false;

	CDB4DTable *filesTable = fDatabase->CreateOutsideTable( kFileTable, inContext );
	if (!filesTable) {
		symbolsTable->Release();
		return false;
	}

	CDB4DTable *extrasTable = fDatabase->CreateOutsideTable( kExtrasTable, inContext );
	if (!extrasTable) {
		symbolsTable->Release();
		filesTable->Release();
		return false;
	}

	CDB4DTable *versionTable = fDatabase->CreateOutsideTable( kVersionTable, inContext );
	if (!versionTable) {
		symbolsTable->Release();
		filesTable->Release();
		extrasTable->Release();
		return false;
	}

	int status = 0x1;
	inContext->StartTransaction();

	// We only want to test for uniqueness when in a debug build -- in theory, this is something
	// we can assure without requiring the DB to check for us.  So release builds will turn this off.
	#if _DEBUG
		const int kUnique = DB4D_Unique;
	#else
		const int kUnique = 0;
	#endif
	// Now we can fill out the fields of the symbol table
	status &= (VE_OK == symbolsTable->AddField( kSymbolName, DB4D_StrFix, -1, 0, NULL, inContext ));
	status &= (VE_OK == symbolsTable->AddField( kSymbolID, DB4D_Integer32, -1, DB4D_Not_Null | kUnique, NULL, inContext ));
	status &= (VE_OK == symbolsTable->AddField( kSymbolFileID, DB4D_Integer32, -1, DB4D_Not_Null, NULL, inContext ));
	status &= (VE_OK == symbolsTable->AddField( kSymbolScriptDocText, DB4D_StrFix, -1, 0, NULL, inContext ));
	status &= (VE_OK == symbolsTable->AddField( kSymbolKind, DB4D_Integer32, -1, 0, NULL, inContext ));
	status &= (VE_OK == symbolsTable->AddField( kSymbolWAFKind, DB4D_StrFix, -1, 0, NULL, inContext ));
	status &= (VE_OK == symbolsTable->AddField( kSymbolPrototypeID, DB4D_StrFix, -1, 0, NULL, inContext ));
	status &= (VE_OK == symbolsTable->AddField( kSymbolOwnerID, DB4D_Integer32, -1, 0, NULL, inContext ));
	status &= (VE_OK == symbolsTable->AddField( kLineNumber, DB4D_Integer32, -1, 0, NULL, inContext ));
	status &= (VE_OK == symbolsTable->AddField( kLineCompletionNumber, DB4D_Integer32, -1, 0, NULL, inContext ));
	status &= (VE_OK == symbolsTable->AddField( kSymbolReturnTypeID, DB4D_StrFix, -1, 0, NULL, inContext ));
	status &= (VE_OK == symbolsTable->AddField( kSymbolReferenceID, DB4D_Integer32, -1, 0, NULL, inContext ));
	

	// We can do the same for the file table
	status &= (VE_OK == filesTable->AddField( kFileID, DB4D_Integer32, -1, DB4D_Not_Null | kUnique | DB4D_AutoSeq, NULL, inContext ));
	status &= (VE_OK == filesTable->AddField( kFilePath, DB4D_StrFix, -1, DB4D_Not_Null, NULL, inContext ));
	status &= (VE_OK == filesTable->AddField( kFileBaseFolder, DB4D_Integer16, -1, DB4D_Not_Null, NULL, inContext ));
	status &= (VE_OK == filesTable->AddField( kFileExecContext, DB4D_Integer16, -1, DB4D_Not_Null, NULL, inContext ));
	status &= (VE_OK == filesTable->AddField( kFileModificationTime, DB4D_Integer64, -1, DB4D_Not_Null, NULL, inContext ));

	// And the extras table
	status &= (VE_OK == extrasTable->AddField( kExtrasOwnerSymbolID, DB4D_Integer32, -1, DB4D_Not_Null, NULL, inContext ));
	status &= (VE_OK == extrasTable->AddField( kExtrasKind, DB4D_Integer32, -1, DB4D_Not_Null, NULL, inContext ));
	status &= (VE_OK == extrasTable->AddField( kExtrasStringData, DB4D_StrFix, -1, 0, NULL, inContext ));
	status &= (VE_OK == extrasTable->AddField( kExtrasIntegerData, DB4D_Integer32, -1, 0, NULL, inContext ));

	// And the version table
	status &= (VE_OK == versionTable->AddField( kVersionNumber, DB4D_Integer32, -1, DB4D_Not_Null, NULL, inContext ));

	if (status) {
		// Add the tables to the database now
		status &= (VE_OK == fDatabase->AddTable( symbolsTable, inContext ));
		status &= (VE_OK == fDatabase->AddTable( filesTable, inContext ));
		status &= (VE_OK == fDatabase->AddTable( extrasTable, inContext ));
		status &= (VE_OK == fDatabase->AddTable( versionTable, inContext ));
	}

	CDB4DField *symbolUniqueID = symbolsTable->FindAndRetainField( kSymbolID, inContext );
	CDB4DField *fileUniqueID = filesTable->FindAndRetainField( kFileID, inContext );
	CDB4DField *filePath = filesTable->FindAndRetainField( kFilePath, inContext );
	CDB4DField *symbolFileID = symbolsTable->FindAndRetainField( kSymbolFileID, inContext );
	CDB4DField *symbolOwnerID = symbolsTable->FindAndRetainField( kSymbolOwnerID, inContext );
	CDB4DField *symbolPrototypeID = symbolsTable->FindAndRetainField( kSymbolPrototypeID, inContext );
	CDB4DField *symbolName = symbolsTable->FindAndRetainField( kSymbolName, inContext );
	CDB4DField *symbolKind = symbolsTable->FindAndRetainField( kSymbolKind, inContext );
	CDB4DField *symbolWAFKind = symbolsTable->FindAndRetainField( kSymbolWAFKind, inContext );
	CDB4DField *symbolLineNumber = symbolsTable->FindAndRetainField( kLineNumber, inContext );
	CDB4DField *extrasOwnerSymbol = extrasTable->FindAndRetainField( kExtrasOwnerSymbolID, inContext );
	CDB4DField *versionNumberField = versionTable->FindAndRetainField( kVersionNumber, inContext );
	

	// Set the version number for the table
	CDB4DRecord *rec = versionTable->NewRecord( inContext );
	rec->SetLong( versionNumberField, kCurrentVersion );
	rec->Save();
	rec->Release();

	// Now we want to set up primary keys
	// Because of the way we clean up everything, we need this to use its own
	// block scope.  This is purely to ensure that the primaryKeyArray is 
	// initialized and finalized properly.
	CDB4DFieldArray primaryKeyArray;
	primaryKeyArray.Add( symbolUniqueID );
	status &= (VE_OK == symbolsTable->SetPrimaryKey( primaryKeyArray, NULL, false, inContext ));
	primaryKeyArray.Delete( symbolUniqueID );

	primaryKeyArray.Add( fileUniqueID );
	status &= (VE_OK == filesTable->SetPrimaryKey( primaryKeyArray, NULL, false, inContext ));
	primaryKeyArray.Delete( fileUniqueID );

	// Now that we've set up some primary keys, let's set up some relations!
	CDB4DRelation *fileToSymbolRelation = fDatabase->CreateRelation( CVSTR( "FileToSymbol" ), CVSTR( "FileToSymbolM1R" ), symbolFileID, fileUniqueID, inContext );
	status &= (VE_OK == fileToSymbolRelation->SetForeignKey( true, inContext ));
	status &= (VE_OK == fileToSymbolRelation->SetReferentialIntegrity( true, true, inContext ));

	// We also want a relation going from a symbol's OwnerID to its UniqueID so that the deletion of an
	// owner causes the child to be deleted as well.
	CDB4DRelation *ownerToChildRelation = fDatabase->CreateRelation( CVSTR( "OwnerToChild" ), CVSTR( "OwnerToChildM1R" ), symbolOwnerID, symbolUniqueID, inContext );
	status &= (VE_OK == ownerToChildRelation->SetForeignKey( true, inContext ));
	status &= (VE_OK == ownerToChildRelation->SetReferentialIntegrity( true, true, inContext ));

	// And there's a relationship between a symbol and its extra information
	CDB4DRelation *symbolToExtraInfoRelation = fDatabase->CreateRelation( CVSTR( "SymbolToExtraInfo" ), CVSTR( "SymbolToExtraInfoM1R" ), extrasOwnerSymbol, symbolUniqueID, inContext );
	status &= (VE_OK == symbolToExtraInfoRelation->SetForeignKey( true, inContext ));
	status &= (VE_OK == symbolToExtraInfoRelation->SetReferentialIntegrity( true, true, inContext ));

	// Initialize some indexes to make searching go significantly faster
	CDB4DIndex *index = NULL;
	VSyncEvent *sync = new VSyncEvent;
	VString indexName = "SymbolIDIndex";
	status &= (VE_OK == fDatabase->CreateIndexOnOneField( symbolUniqueID, DB4D_Index_DefaultType, true, NULL, &indexName, &index, true, sync, inContext ));
	sync->Lock(); sync->Reset();
	if (index) { index->Release(); index = NULL; }
	indexName = "FileIDIndex";
	status &= (VE_OK == fDatabase->CreateIndexOnOneField( fileUniqueID, DB4D_Index_DefaultType, true, NULL, &indexName, &index, true, sync, inContext ));
	sync->Lock(); sync->Reset();
	if (index) { index->Release(); index = NULL; }
	indexName = "SymbolNameIndex";
	status &= (VE_OK == fDatabase->CreateIndexOnOneField( symbolName, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ));
	if (index) { index->Release(); index = NULL; }
	indexName = "SymbolLineNumberIndex";
	status &= (VE_OK == fDatabase->CreateIndexOnOneField( symbolKind, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ));
	if (index) { index->Release(); index = NULL; }
	indexName = "SymbolKindIndex";
	status &= (VE_OK == fDatabase->CreateIndexOnOneField( symbolLineNumber, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ));
	if (index) { index->Release(); index = NULL; }
	indexName = "FilePathIndex";
	status &= (VE_OK == fDatabase->CreateIndexOnOneField( filePath, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ));
	if (index) { index->Release(); index = NULL; }
	indexName = "SymbolOwnerIndex";
	status &= (VE_OK == fDatabase->CreateIndexOnOneField( symbolOwnerID, DB4D_Index_DefaultType, false, NULL, &indexName, &index, true, NULL, inContext ));
	if (index) { index->Release(); index = NULL; }

	versionNumberField->Release();
	ownerToChildRelation->Release();
	fileToSymbolRelation->Release();
	symbolToExtraInfoRelation->Release();
	symbolName->Release();
	symbolKind->Release();
	symbolLineNumber->Release();
	symbolOwnerID->Release();
	symbolPrototypeID->Release();
	symbolFileID->Release();
	symbolUniqueID->Release();
	fileUniqueID->Release();
	filePath->Release();
	extrasOwnerSymbol->Release();
	symbolsTable->Release();
	filesTable->Release();
	extrasTable->Release();
	versionTable->Release();

	if (status) {
		inContext->CommitTransaction();
	} else {
		inContext->RollBackTransaction();
	}
	
	return status != 0;
}

sLONG VSymbolTable::InnerSymbolTable::CountFiles( CDB4DBaseContext *inContext )
{
	CDB4DTable *table = fDatabase->FindAndRetainTable( kFileTable, inContext );
	sLONG ret = 0;
	if (table) {
		ret = table->CountRecordsInTable( inContext );
		table->Release();
	}
	return ret;
}

std::vector< Symbols::ISymbol * > VSymbolTable::InnerSymbolTable::GetSymbolsByNameHelper( sLONG inOwnerID, const VString &inName, sLONG inOwnerFileID, CDB4DBaseContext *inContext )
{
	std::vector< Symbols::ISymbol * > ret;
	if (!fDatabase)	return ret;

	// We are going to be searching based on the name of the symbol as well as who owns
	// the symbol.  The SQL search we are imitating is:
	// SELECT * FROM Symbols WHERE OwnerID = inOwnerID AND Name = inName
	//
	// Let's get a new query object for us to fill out.  Note that since we are searching
	// text, we are going to do a case-sensitive search.  While this isn't true for all
	// of the languages we support, it's true for the current priority case, which is JavaScript.
	// When we start to add support for other languages, we may have to carry case sensitivity
	// information with the symbol so that we can perform the correct operation.
	CDB4DTable *symbolsTable = fDatabase->FindAndRetainTable( kSymbolTable, inContext );
	if (!symbolsTable)	return ret;
	CDB4DField *nameField = symbolsTable->FindAndRetainField( kSymbolName, inContext );
	CDB4DField *ownerField = symbolsTable->FindAndRetainField( kSymbolOwnerID, inContext );
	CDB4DField *fileField = symbolsTable->FindAndRetainField( kSymbolFileID, inContext );
	if (!nameField || !ownerField || !fileField) {
		if (nameField)	nameField->Release();
		if (ownerField)	ownerField->Release();
		if (fileField)	fileField->Release();
		symbolsTable->Release();
		return ret;
	}
	
	CDB4DQuery *query = symbolsTable->NewQuery();
	query->AddCriteria( nameField, DB4D_Equal, inName, true );
	query->AddLogicalOperator( DB4D_And );
	if (inOwnerID) {
		VLong id( inOwnerID );
		query->AddCriteria( ownerField, DB4D_Equal, id );
	} else {
		VLong id( 0 );
		query->AddCriteria( ownerField, DB4D_IsNull, id );
	}

	if (inOwnerFileID) {
		// If we have a file ID, then we want to further filter the results based on the file
		VLong id( inOwnerFileID );
		query->AddLogicalOperator( DB4D_And );
		query->AddCriteria( fileField, DB4D_Equal, id );
	}
	
	CDB4DSelection *results = symbolsTable->ExecuteQuery( query, inContext );
	if (results) {
		RecIDType total = results->CountRecordsInSelection( inContext );
		for (RecIDType ctr = 1; ctr <= total; ctr++) {
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			if (record) {
				ret.push_back( SymbolFromRecord( symbolsTable, record, inContext ) );
				record->Release();
			}
		}
		results->Release();
	}

	query->Release();

	nameField->Release();
	ownerField->Release();
	fileField->Release();
	symbolsTable->Release();
	
	return ret;
}

std::vector< Symbols::ISymbol * > VSymbolTable::InnerSymbolTable::GetSymbolsByName( const VString &inName, CDB4DBaseContext *inContext )
{
	return GetSymbolsByNameHelper( 0, inName, 0, inContext );
}

std::vector< Symbols::ISymbol * > VSymbolTable::InnerSymbolTable::GetSymbolsByName( Symbols::ISymbol *inOwner, const VString &inName, Symbols::IFile *inOwnerFile, CDB4DBaseContext *inContext )
{
	return GetSymbolsByNameHelper( (inOwner) ? (inOwner->GetID()) : 0, inName, (inOwnerFile) ? (inOwnerFile->GetID()) : 0, inContext );
}

std::vector< Symbols::ISymbol * > VSymbolTable::InnerSymbolTable::GetClassSymbols( Symbols::ISymbol *inOwner, const VString &inName, Symbols::IFile *inOwnerFile, CDB4DBaseContext *inContext )
{
	std::vector< Symbols::ISymbol * > ret;
	if (!fDatabase)	return
		ret;

	CDB4DTable *symbolsTable = fDatabase->FindAndRetainTable( kSymbolTable, inContext );
	
	if (!symbolsTable)
		return ret;

	CDB4DField *nameField = symbolsTable->FindAndRetainField( kSymbolName, inContext );
	CDB4DField *ownerField = symbolsTable->FindAndRetainField( kSymbolOwnerID, inContext );
	CDB4DField *fileField = symbolsTable->FindAndRetainField( kSymbolFileID, inContext );
	CDB4DField *kindField = symbolsTable->FindAndRetainField( kSymbolKind, inContext );

	if (!nameField || !ownerField || !fileField || !kindField)
	{
		if (nameField)	nameField->Release();
		if (ownerField)	ownerField->Release();
		if (fileField)	fileField->Release();
		if (fileField)	fileField->Release();
		if (kindField)	kindField->Release();

		symbolsTable->Release();
		return ret;
	}
	
	CDB4DQuery *query = symbolsTable->NewQuery();
	query->AddCriteria( nameField, DB4D_Like, inName, true );
	query->AddLogicalOperator( DB4D_And );

	sLONG ownerId = inOwner ? inOwner->GetID() : 0;

	sLONG ownerFileId = inOwnerFile ? inOwnerFile->GetID() : 0;

	if (ownerFileId)
	{
		// If we have a file ID, then we want to further filter the results based on the file
		VLong id( ownerFileId );
		query->AddLogicalOperator( DB4D_And );
		query->AddCriteria( fileField, DB4D_Equal, id );
	}
	
	VLong kind( Symbols::ISymbol::kKindClassConstructor );
	query->AddLogicalOperator( DB4D_And );
	query->AddCriteria( kindField, DB4D_Equal, kind );

	CDB4DSelection *results = symbolsTable->ExecuteQuery( query, inContext );
	if (results)
	{
		RecIDType total = results->CountRecordsInSelection( inContext );
		results->SortSelection(nameField->GetID(), true);
		for (RecIDType ctr = 1; ctr <= total; ctr++)
		{
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			if (record)
			{
				ret.push_back( SymbolFromRecord( symbolsTable, record, inContext ) );
				record->Release();
			}
		}
		results->Release();
	}

	query->Release();

	nameField->Release();
	ownerField->Release();
	fileField->Release();
	kindField->Release();
	symbolsTable->Release();
	
	return ret;
}


std::vector< Symbols::ISymbol * > VSymbolTable::InnerSymbolTable::GetLikeNamedSubSymbols( Symbols::ISymbol *inOwner, const VString &inLikeName, CDB4DBaseContext *inContext, const bool inFullyLoad)
{
	// We may have been given an owner (though it is legal for the owner to be NULL), and
	// we want to locate subsymbols for it.  The basic idea is that we want to perform a
	// SELECT * FROM Symbols WHERE Symbols.OwnerID = inOwner.UniqueID AND Symbols.Name LIKE inLikeName
	std::vector< Symbols::ISymbol * > ret;

	// If the user didn't pass us any LIKE text, then we want to bail out early.  Since this is only
	// used to get us an approximate list, we don't want to monkey around with getting the user every
	// single globally accessible symbol in the system (as that is very expensive) -- at least let them
	// filter the list with a single letter!
	if (inLikeName.IsEmpty() || (inLikeName.GetLength() == 1 && inLikeName[0] == inContext->GetIntlMgr()->GetWildChar()))	return ret;

	if (!fDatabase)	return ret;
	
	CDB4DTable *symbolsTable = fDatabase->FindAndRetainTable( kSymbolTable, inContext );
	if (!symbolsTable)	return ret;

	CDB4DField *nameField = symbolsTable->FindAndRetainField( kSymbolName, inContext );
	CDB4DField *ownerField = symbolsTable->FindAndRetainField( kSymbolOwnerID, inContext );
	
	if (!nameField || !ownerField) {
		if (nameField)	nameField->Release();
		if (ownerField)	ownerField->Release();
		symbolsTable->Release();
	}
	
	// Let's get a new query object for us to fill out
	CDB4DQuery *query = symbolsTable->NewQuery();

	if (inOwner) {
		VLong id( inOwner->GetID() );
		query->AddCriteria( ownerField, DB4D_Equal, id );
	} else {
		VLong id( 0 );
		query->AddCriteria( ownerField, DB4D_IsNull, id );
	}
	
	query->AddLogicalOperator( DB4D_And );
	query->AddCriteria( nameField, DB4D_Like, inLikeName );

	VString emptyString = "";
	query->AddLogicalOperator( DB4D_And );
	query->AddCriteria( nameField, DB4D_NotEqual, emptyString );

	CDB4DSelection *results = symbolsTable->ExecuteQuery( query, inContext );
	if (results) {
		for (RecIDType ctr = 1; ctr <= results->CountRecordsInSelection( inContext ); ctr++) {
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			if (record) {
				ret.push_back( SymbolFromRecord( symbolsTable, record, inContext, inFullyLoad ) );
				record->Release();
			}
		}
		results->Release();
	}
	query->Release();

	nameField->Release();
	ownerField->Release();
	symbolsTable->Release();
	
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

	if (!fDatabase)	return ret;

	CDB4DTable *symbolsTable = fDatabase->FindAndRetainTable( kSymbolTable, inContext );
	if (!symbolsTable)	return ret;
	
	CDB4DField *ownerField = symbolsTable->FindAndRetainField( kSymbolOwnerID, inContext );
	CDB4DField *fileField = symbolsTable->FindAndRetainField( kSymbolFileID, inContext );
	CDB4DField *nameField = symbolsTable->FindAndRetainField( kSymbolName, inContext );
	if (!nameField || !ownerField || !fileField) {
		if (nameField)	nameField->Release();
		if (ownerField)	ownerField->Release();
		if (fileField)	fileField->Release();
		symbolsTable->Release();
		return ret;
	}
	
	// Let's get a new query object for us to fill out
	CDB4DQuery *query = symbolsTable->NewQuery();

	if (inOwner) {
		VLong id( inOwner->GetID() );
		query->AddCriteria( ownerField, DB4D_Equal, id );
	} else {
		VLong id( 0 );
		query->AddCriteria( ownerField, DB4D_IsNull, id );
	}
	
	// If the user passed in a file to filter by, we want to add another criteria that
	// specifies Symbols.FileID = inOwnerFile.UniqueID
	if (inOwnerFile) {
		query->AddLogicalOperator( DB4D_And );
		VLong id( inOwnerFile->GetID() );
		query->AddCriteria( fileField, DB4D_Equal, id );
	}

	CDB4DSelection *results = symbolsTable->ExecuteQuery( query, inContext );
	if (results) {
		// If the user passed in some sorting options, we want to pass those along to DB4D
		DB4D_FieldID sortingField = -1;
		switch (inOptions) {
			case kByName:			sortingField = nameField->GetID( inContext ); break;
			case kByLineNumber: {
				CDB4DField *lineNumField = symbolsTable->FindAndRetainField( kLineNumber, inContext );
				if (lineNumField) {
					sortingField = lineNumField->GetID( inContext ); 
					lineNumField->Release();
				}
			} break;
			case kByKind: {
				CDB4DField *kindField = symbolsTable->FindAndRetainField( kSymbolKind, inContext );
				if (kindField) {
					sortingField = kindField->GetID( inContext ); 
					kindField->Release();
				}
			} break;
		}
		if (-1 != sortingField)		results->SortSelection( sortingField, true, NULL, inContext );

		for (RecIDType ctr = 1; ctr <= results->CountRecordsInSelection( inContext ); ctr++) {
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			if (record) {
				ret.push_back( SymbolFromRecord( symbolsTable, record, inContext ) );
				record->Release();
			}
		}
		results->Release();
	}
	query->Release();

	nameField->Release();
	ownerField->Release();
	fileField->Release();
	symbolsTable->Release();

	return ret;
}

std::vector< Symbols::ISymbol * > VSymbolTable::InnerSymbolTable::GetNamedSubSymbols( Symbols::ISymbol *inOwner, Symbols::IFile *inOwnerFile, SortingOptions inOptions, CDB4DBaseContext *inContext )
{
	// We may have been given an owner (though it is legal for the owner to be NULL), and
	// we want to locate subsymbols for it.  The basic idea is that we want to perform a
	// SELECT * FROM Symbols WHERE Symbols.OwnerID = inOwner.UniqueID AND Symbols.Name IS NOT ""
	std::vector< Symbols::ISymbol * > ret;

	if (!fDatabase)	return ret;

	CDB4DTable *symbolsTable = fDatabase->FindAndRetainTable( kSymbolTable, inContext );
	if (!symbolsTable)	return ret;
	
	CDB4DField *ownerField = symbolsTable->FindAndRetainField( kSymbolOwnerID, inContext );
	CDB4DField *fileField = symbolsTable->FindAndRetainField( kSymbolFileID, inContext );
	CDB4DField *nameField = symbolsTable->FindAndRetainField( kSymbolName, inContext );
	if (!nameField || !ownerField || !fileField) {
		if (nameField)	nameField->Release();
		if (ownerField)	ownerField->Release();
		if (fileField)	fileField->Release();
		symbolsTable->Release();
		return ret;
	}
	
	// Let's get a new query object for us to fill out
	CDB4DQuery *query = symbolsTable->NewQuery();

	if (inOwner) {
		VLong id( inOwner->GetID() );
		query->AddCriteria( ownerField, DB4D_Equal, id );
	} else {
		VLong id( 0 );
		query->AddCriteria( ownerField, DB4D_IsNull, id );
	}
	
	VString emptyString = "";
	query->AddLogicalOperator( DB4D_And );
	query->AddCriteria( nameField, DB4D_NotEqual, emptyString );

	// If the user passed in a file to filter by, we want to add another criteria that
	// specifies Symbols.FileID = inOwnerFile.UniqueID
	if (inOwnerFile) {
		query->AddLogicalOperator( DB4D_And );
		VLong id( inOwnerFile->GetID() );
		query->AddCriteria( fileField, DB4D_Equal, id );
	}

	CDB4DSelection *results = symbolsTable->ExecuteQuery( query, inContext );
	if (results) {
		// If the user passed in some sorting options, we want to pass those along to DB4D
		DB4D_FieldID sortingField = -1;
		switch (inOptions) {
			case kByName:			sortingField = nameField->GetID( inContext ); break;
			case kByLineNumber: {
				CDB4DField *lineNumField = symbolsTable->FindAndRetainField( kLineNumber, inContext );
				if (lineNumField) {
					sortingField = lineNumField->GetID( inContext ); 
					lineNumField->Release();
				}
			} break;
			case kByKind: {
				CDB4DField *kindField = symbolsTable->FindAndRetainField( kSymbolKind, inContext );
				if (kindField) {
					sortingField = kindField->GetID( inContext ); 
					kindField->Release();
				}
			} break;
		}
		if (-1 != sortingField)		results->SortSelection( sortingField, true, NULL, inContext );

		for (RecIDType ctr = 1; ctr <= results->CountRecordsInSelection( inContext ); ctr++) {
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			if (record) {
				ret.push_back( SymbolFromRecord( symbolsTable, record, inContext ) );
				record->Release();
			}
		}
		results->Release();
	}
	query->Release();

	nameField->Release();
	ownerField->Release();
	fileField->Release();
	symbolsTable->Release();

	return ret;
}

std::vector< Symbols::IFile * > VSymbolTable::InnerSymbolTable::GetFilesByName( const VString &inName, CDB4DBaseContext *inContext )
{
	std::vector< Symbols::IFile * > ret;

	if (!fDatabase)	return ret;
	
	CDB4DTable *filesTable = fDatabase->FindAndRetainTable( kFileTable, inContext );
	if (!filesTable)	return ret;
	
	CDB4DField *pathField = filesTable->FindAndRetainField( kFilePath, inContext );
	if (!pathField) {
		filesTable->Release();
		return ret;
	}

	// We are going to be searching based on the Path of the symbol, but we
	// will be doing a containment search so the path doesn't have to be a
	// perfect match.

	// Let's get a new query object for us to fill out
	CDB4DQuery *query = filesTable->NewQuery();
	UniChar likeChar = inContext->GetIntlMgr()->GetWildChar();
	query->AddCriteria( pathField, DB4D_Like, VString( likeChar ) + inName + VString( likeChar ), true );
	
	CDB4DSelection *results = filesTable->ExecuteQuery( query, inContext );
	if (results) {
		for (RecIDType ctr = 1; ctr <= results->CountRecordsInSelection( inContext ); ctr++) {
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			ret.push_back( FileFromRecord( filesTable, record, inContext ) );
			record->Release();
		}
		results->Release();
	}
	query->Release();

	pathField->Release();
	filesTable->Release();
	
	return ret;
}

std::vector< Symbols::IFile * > VSymbolTable::InnerSymbolTable::GetFilesByPathAndBaseFolder( const VString &inName, const ESymbolFileBaseFolder inBaseFolder, CDB4DBaseContext *inContext )
{
	std::vector< Symbols::IFile * >	ret;
	VLong	baseFolderLong(inBaseFolder);

	if (!fDatabase)	return ret;

	CDB4DTable *filesTable = fDatabase->FindAndRetainTable( kFileTable, inContext );
	if (!filesTable)	return ret;
	
	CDB4DField *pathField = filesTable->FindAndRetainField( kFilePath, inContext );
	CDB4DField *baseFolderField = filesTable->FindAndRetainField( kFileBaseFolder, inContext );
	if (!pathField ||!baseFolderField) {
		filesTable->Release();
		if (pathField)
			pathField->Release();
		if (baseFolderField)
			baseFolderField->Release();
		return ret;
	}
	
	// Let's get a new query object for us to fill out
	CDB4DQuery *query = filesTable->NewQuery();
	query->AddCriteria( pathField, DB4D_Equal, inName);
	query->AddLogicalOperator(DB4D_And);
	query->AddCriteria( baseFolderField, DB4D_Equal, baseFolderLong);
	
	
	CDB4DSelection *results = filesTable->ExecuteQuery( query, inContext );
	if (results) {
		for (RecIDType ctr = 1; ctr <= results->CountRecordsInSelection( inContext ); ctr++) {
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			ret.push_back( FileFromRecord( filesTable, record, inContext ) );
			record->Release();
		}
		results->Release();
	}
	query->Release();
	
	pathField->Release();
	baseFolderField->Release();
	filesTable->Release();	

	return ret;
}


Symbols::IFile *VSymbolTable::InnerSymbolTable::GetFileByID( sLONG inID, CDB4DBaseContext *inContext )
{
	Symbols::IFile *ret = NULL;
	if (!fDatabase)	return ret;

	CDB4DTable *filesTable = fDatabase->FindAndRetainTable( kFileTable, inContext );
	if (!filesTable)	return ret;
	
	CDB4DIndex *index = fDatabase->FindAndRetainIndexByName( "FileIDIndex", inContext );
	if (!index) {
		filesTable->Release();
		return ret;
	}
	// Since we're only interested in one record, and there's only one search criteria, we can make use
	// of the index directly.
	VLong id( inID );
	VErrorDB4D err = VE_OK;
	VCompareOptions options;
	RecIDType recID = index->FindKey( id, err, inContext, NULL, NULL, options );
	if (kDB4D_NullRecordID != recID) {
		CDB4DRecord *rec = filesTable->LoadRecord( recID, DB4D_Do_Not_Lock, inContext );
		if (rec) {
			ret = FileFromRecord( filesTable, rec, inContext );
			rec->Release();
		}
	}

	index->Release();
	filesTable->Release();
	
	return ret;
}

Symbols::ISymbol *VSymbolTable::InnerSymbolTable::GetSymbolByID( sLONG inID, CDB4DBaseContext *inContext )
{
	Symbols::ISymbol *ret = NULL;
	if (!fDatabase)	return ret;

	CDB4DTable *symbolsTable = fDatabase->FindAndRetainTable( kSymbolTable, inContext );
	if (!symbolsTable)	return ret;
	
	CDB4DIndex *index = fDatabase->FindAndRetainIndexByName( "SymbolIDIndex", inContext );
	if (!index) {
		symbolsTable->Release();
		return ret;
	}
	
	// Since we're only interested in one record, and there's only one search criteria, we can make use
	// of the index directly.
	VLong id( inID );
	VErrorDB4D err = VE_OK;
	VCompareOptions options;
	RecIDType recID = index->FindKey( id, err, inContext, NULL, NULL, options );
	if (kDB4D_NullRecordID != recID) {
		CDB4DRecord *rec = symbolsTable->LoadRecord( recID, DB4D_Do_Not_Lock, inContext );
		if (rec) {
			ret = SymbolFromRecord( symbolsTable, rec, inContext );
			rec->Release();
		}
	}
	
	index->Release();
	symbolsTable->Release();	

	return ret;
}

VString VSymbolTable::InnerSymbolTable::GetStringField( CDB4DRecord *inRecord, CDB4DField *inField )
{
	VString ret;
	inRecord->GetString( inField, ret );
	return ret;
}

sLONG VSymbolTable::InnerSymbolTable::GetIntegerField( CDB4DRecord *inRecord, CDB4DField *inField, bool *isNull )
{
	if (isNull)	*isNull = false;

	VValueSingle *value = inRecord->GetFieldValue( inField );
	if (value) {
		if (isNull && value->IsNull())	*isNull = true;
		return value->GetLong();
	}
	return 0;
}

uLONG8 VSymbolTable::InnerSymbolTable::GetLongIntegerField( CDB4DRecord *inRecord, CDB4DField *inField )
{
	sLONG8 ret;
	inRecord->GetLong8( inField, &ret );
	return (uLONG8)ret;
}

Symbols::IExtraInfo *VSymbolTable::InnerSymbolTable::ExtraInfoFromRecord( CDB4DTable *inTable, CDB4DRecord *inRecord, CDB4DBaseContext *inContext )
{
	xbox_assert( inRecord );

	CDB4DField *intData = inTable->FindAndRetainField( kExtrasIntegerData, inContext );
	CDB4DField *strData = inTable->FindAndRetainField( kExtrasStringData, inContext );
	CDB4DField *kind = inTable->FindAndRetainField( kExtrasKind, inContext );	
	if (!intData || !strData || !kind) {
		if (intData)	intData->Release();
		if (strData)	strData->Release();
		if (kind)		kind->Release();
		return NULL;
	}
	VSymbolExtraInfo *ret = new VSymbolExtraInfo();
	ret->SetIntegerData( GetIntegerField( inRecord, intData, NULL ) );
	ret->SetStringData( GetStringField( inRecord, strData ) );
	ret->SetKind( (Symbols::IExtraInfo::Kind)GetIntegerField( inRecord, kind, NULL ) );

	intData->Release();
	strData->Release();
	kind->Release();
	
	return ret;
}

Symbols::IFile *VSymbolTable::InnerSymbolTable::FileFromRecord( CDB4DTable *inTable, CDB4DRecord *inRecord, CDB4DBaseContext *inContext )
{
	xbox_assert( inRecord );

	CDB4DField *id = inTable->FindAndRetainField( kFileID, inContext );
	CDB4DField *path = inTable->FindAndRetainField( kFilePath, inContext );
	CDB4DField *basefolder = inTable->FindAndRetainField( kFileBaseFolder, inContext );
	CDB4DField *execcontext = inTable->FindAndRetainField( kFileExecContext, inContext );
	CDB4DField *mod = inTable->FindAndRetainField( kFileModificationTime, inContext );
	
	if (!id || !path || !basefolder || !execcontext || !mod) {
		if (id)		id->Release();
		if (path)	path->Release();
		if (basefolder)	basefolder->Release();
		if (execcontext)	execcontext->Release();
		if (mod)	mod->Release();	
		return NULL;
	}
	
	VSymbolFile *ret = new VSymbolFile();
	ret->SetID( GetIntegerField( inRecord, id, NULL ) );
	ret->SetPath( GetStringField( inRecord, path ) );
	ret->SetBaseFolder( (ESymbolFileBaseFolder) GetIntegerField( inRecord, basefolder, NULL ) );
	ret->SetExecutionContext( (ESymbolFileExecContext) GetIntegerField( inRecord, execcontext, NULL ) );
	ret->SetModificationTimestamp( GetLongIntegerField( inRecord, mod ) );
	
	id->Release();
	path->Release();
	basefolder->Release();
	execcontext->Release();
	mod->Release();

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
	CDB4DField *idField = inTable->FindAndRetainField( kSymbolID, inContext );
	sLONG uniqueID = GetIntegerField( inRecord, idField, NULL );
	idField->Release();
	
	Symbols::ISymbol *recRet = NULL;
	fRecursionTaskLock->Lock();
	std::map< sLONG, Symbols::ISymbol * >::iterator iter = fRecursionSet.find( uniqueID );
	if (iter != fRecursionSet.end()) {
		recRet = iter->second;
		recRet->Retain();
	}
	fRecursionTaskLock->Unlock();

	if (recRet)	return recRet;

	VSymbol *ret = new VSymbol();
	
 	CDB4DField *nameField = inTable->FindAndRetainField( kSymbolName, inContext );
	CDB4DField *sdocField = inTable->FindAndRetainField( kSymbolScriptDocText, inContext );
	CDB4DField *kindField = inTable->FindAndRetainField( kSymbolKind, inContext );
	CDB4DField *wafKindField = inTable->FindAndRetainField( kSymbolWAFKind, inContext );
	CDB4DField *lineNumField = inTable->FindAndRetainField( kLineNumber, inContext );
	CDB4DField *lineNumCompField = inTable->FindAndRetainField( kLineCompletionNumber, inContext );
	CDB4DField *protoField = inTable->FindAndRetainField( kSymbolPrototypeID, inContext );
	CDB4DField *ownerField = inTable->FindAndRetainField( kSymbolOwnerID, inContext );
	CDB4DField *returnField = inTable->FindAndRetainField( kSymbolReturnTypeID, inContext );
	CDB4DField *fileField = inTable->FindAndRetainField( kSymbolFileID, inContext );
	CDB4DField *referenceField = inTable->FindAndRetainField( kSymbolReferenceID, inContext );


	fRecursionTaskLock->Lock();
	ret->Retain();
	fRecursionSet[ uniqueID ] = ret;			// Add this symbol to our mapping
	fRecursionTaskLock->Unlock();
	ret->SetID( uniqueID );
	ret->SetName( GetStringField( inRecord, nameField ) );
	ret->SetScriptDocComment( GetStringField( inRecord, sdocField ) );
	ret->SetKind( (int)GetIntegerField( inRecord, kindField, NULL ) );
	ret->SetLineNumber( GetIntegerField( inRecord, lineNumField, NULL ) );
	ret->SetLineCompletionNumber( GetIntegerField( inRecord, lineNumCompField, NULL ) );

	sLONG lineNumber = GetIntegerField( inRecord, lineNumField, NULL );

	VString wafKind = GetStringField( inRecord, wafKindField );
	if ( ! wafKind.IsEmpty() )
		ret->SetWAFKind( wafKind );

	if (inFullyLoad) {
		// Get the unique identifier for the prototype and owner IDs.  We will
		// search the symbols table for those next.  The prototype field is actually
		// a comma-separated string of IDs.  So we will get a list of symbols to
		// add, but if one of the IDs has gone stale, we simply won't add it to the list.
		bool isNull = false;
		VString prototypeIDs = GetStringField( inRecord, protoField );
		if (!prototypeIDs.IsEmpty()) {
			VectorOfVString IDs;
			prototypeIDs.GetSubStrings( ',', IDs, false, true );

			std::vector< Symbols::ISymbol * > prototypes;
			for (VectorOfVString::iterator iter = IDs.begin(); iter != IDs.end(); ++iter) {
				sLONG prototypeID = (*iter).GetLong();
				// Prototypes can go stale, and that is fine.  If we are unable to load the prototype
				// from the field we've been given, we want to update the record so that the prototype
				// field is set to null.  This will help us the next time we go to load it.
				Symbols::ISymbol *prototypeSymbol = GetSymbolByID( prototypeID, inContext );
				if (prototypeSymbol) {
					prototypes.push_back( prototypeSymbol );
				}
			}

			if (prototypes.empty()) {
				// There aren't any prototypes that we were able to load, so we'll just set the field to NULL
				// since all of the prototypes have gone stale.
				VErrorDB4D err;
				StErrorContextInstaller errorContext( VE_DB4D_CANNOTSAVERECORD, VE_DB4D_RECORDISLOCKED, VE_OK );

				inRecord->SetFieldToNull( protoField );
				inRecord->Save();
			} else {
				// We've loaded up at least one prototype, so let's add all of them to our symbol
				ret->AddPrototypes( prototypes );
				
				// modif SLA : memory leak
				for (std::vector< Symbols::ISymbol * >::iterator iter = prototypes.begin(); iter != prototypes.end(); ++iter)
					(*iter)->Release();
			}
		}

		sLONG ownerID = GetIntegerField( inRecord, ownerField, &isNull );
		if (!isNull) {
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
		VString returnTypeIDs = GetStringField( inRecord, returnField );
		if (!returnTypeIDs.IsEmpty()) {
			VectorOfVString IDs;
			returnTypeIDs.GetSubStrings( ',', IDs, false, true );
			for (VectorOfVString::iterator iter = IDs.begin(); iter != IDs.end(); ++iter) {
				sLONG returnTypeID = (*iter).GetLong();
				Symbols::ISymbol *returnType = GetSymbolByID( returnTypeID, inContext );
				if (returnType) {
					ret->AddReturnType( returnType );
					returnType->Release();
				}
			}
		}

		sLONG referenceID = GetIntegerField( inRecord, referenceField, &isNull );
		if (!isNull) {
			Symbols::ISymbol *ref = GetSymbolByID( referenceID, inContext );
			ret->SetReferenceSymbol( ref );
			if (ref)
				ref->Release();
		}

		sLONG fileID = GetIntegerField( inRecord, fileField, NULL );
		Symbols::IFile *file = GetFileByID( fileID, inContext );
		ret->SetFile(file);
		file->Release();
	}

	nameField->Release();
	sdocField->Release();
	kindField->Release();
	wafKindField->Release();
	lineNumField->Release();
	lineNumCompField->Release();
	protoField->Release();
	ownerField->Release();
	returnField->Release();
	fileField->Release();
	referenceField->Release();
	
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

	CDB4DTable *table = ioRecord->RetainTable();
	if (!table)	return false;
	
	CDB4DField *pathField = table->FindAndRetainField( kFilePath, inContext );
	CDB4DField *parentField = table->FindAndRetainField( kFileBaseFolder, inContext );
	CDB4DField *scopeField = table->FindAndRetainField( kFileExecContext, inContext );
	CDB4DField *modField = table->FindAndRetainField( kFileModificationTime, inContext );

	ioRecord->SetString( pathField, inFile->GetPath() );
	ioRecord->SetLong( parentField, inFile->GetBaseFolder() );
	ioRecord->SetLong( scopeField, inFile->GetExecutionContext() );
	ioRecord->SetLong8( modField, inFile->GetModificationTimestamp() );
	
	pathField->Release();
	parentField->Release();
	scopeField->Release();
	modField->Release();
	table->Release();

	return true;
}

bool VSymbolTable::InnerSymbolTable::RecordFromExtraInfo( const Symbols::ISymbol *inOwner, const Symbols::IExtraInfo *inExtraInfo, CDB4DRecord *ioRecord, CDB4DBaseContext *inContext )
{
	xbox_assert( inOwner );
	xbox_assert( inExtraInfo );
	xbox_assert( ioRecord );
	
	CDB4DTable *table = ioRecord->RetainTable();
	if (!table)	return false;
	
	CDB4DField *kind = table->FindAndRetainField( kExtrasKind, inContext );
	CDB4DField *strData = table->FindAndRetainField( kExtrasStringData, inContext );
	CDB4DField *intData = table->FindAndRetainField( kExtrasIntegerData, inContext );
	CDB4DField *owner = table->FindAndRetainField( kExtrasOwnerSymbolID, inContext );	

	ioRecord->SetLong( kind, inExtraInfo->GetKind() );
	ioRecord->SetString( strData, inExtraInfo->GetStringData() );
	ioRecord->SetLong( intData, inExtraInfo->GetIntegerData() );
	ioRecord->SetLong( owner, inOwner->GetID() );

	kind->Release();
	strData->Release();
	intData->Release();
	owner->Release();
	table->Release();
	return true;
}

bool VSymbolTable::InnerSymbolTable::UpdateFile( Symbols::IFile *inFile, CDB4DBaseContext *inContext )
{
	// We want to do an update operation, like the following
	// UPDATE Files SET ModificationTimestamp = newStamp, Path = newPath WHERE UniqueID = inID
	if (!fDatabase)	return false;

	CDB4DTable *filesTable = fDatabase->FindAndRetainTable( kFileTable, inContext );
	CDB4DField *idField = filesTable->FindAndRetainField( kFileID, inContext );
	CDB4DField *pathField = filesTable->FindAndRetainField( kFilePath, inContext );
	CDB4DField *modField = filesTable->FindAndRetainField( kFileModificationTime, inContext );
	
	// Let's get a new query object for us to fill out
	CDB4DQuery *query = filesTable->NewQuery();
	VLong id( inFile->GetID() );
	query->AddCriteria( idField, DB4D_Equal, id );
	
	CDB4DSelection *results = filesTable->ExecuteQuery( query, inContext );
	CDB4DRecord *record = NULL;
	if (results && results->CountRecordsInSelection( inContext ) > 0) {
		record = results->LoadSelectedRecord( 1, DB4D_Keep_Lock_With_Record, inContext );
	}
	results->Release();
	query->Release();

	bool ret = false;
	if (record) {
		// We found a record, so now we want to stick the new path and modification
		// timestamp into it, then save it back out
		record->SetString( pathField, inFile->GetPath() );
		record->SetLong8( modField, inFile->GetModificationTimestamp() );

		if (record->Save())	ret = true;

		record->Release();
	}

	idField->Release();
	pathField->Release();
	modField->Release();
	filesTable->Release();
	
	return ret;
}

bool VSymbolTable::InnerSymbolTable::UpdateSymbol( Symbols::ISymbol *inSym, CDB4DBaseContext *inContext )
{
	bool ret = false;
	if (!fDatabase)	return ret;

	CDB4DTable *symbolsTable = fDatabase->FindAndRetainTable( kSymbolTable, inContext );
	if (!symbolsTable)	return ret;

	CDB4DIndex *index = fDatabase->FindAndRetainIndexByName( "SymbolIDIndex", inContext );
	if (!index) {
		symbolsTable->Release();
		return ret;
	}

	// Since we're only interested in one record, and there's only one search criteria, we can make use
	// of the index directly.
	VLong id( inSym->GetID() );
	VErrorDB4D err = VE_OK;
	VCompareOptions options;
	RecIDType recID = index->FindKey( id, err, inContext, NULL, NULL, options );
	if (kDB4D_NullRecordID != recID) {
		CDB4DRecord *record = symbolsTable->LoadRecord( recID, DB4D_Keep_Lock_With_Record, inContext );
		if (record)
		{
			CDB4DField *nameField = symbolsTable->FindAndRetainField( kSymbolName, inContext );								// String
			CDB4DField *fileIdField = symbolsTable->FindAndRetainField( kSymbolFileID, inContext );							// Int32
			CDB4DField *prototypeField = symbolsTable->FindAndRetainField( kSymbolPrototypeID, inContext );					// String, Comma-separated list of Int32 IDs
			CDB4DField *ownerField = symbolsTable->FindAndRetainField( kSymbolOwnerID, inContext );							// Int32
			CDB4DField *kindField = symbolsTable->FindAndRetainField( kSymbolKind, inContext );								// Int16
			CDB4DField *WAFKindField = symbolsTable->FindAndRetainField( kSymbolWAFKind, inContext );						// String
			CDB4DField *scriptDocField = symbolsTable->FindAndRetainField( kSymbolScriptDocText, inContext );				// Text
			CDB4DField *lineNumberField = symbolsTable->FindAndRetainField( kLineNumber , inContext );						// Int32
			CDB4DField *lineCompletionNumberField = symbolsTable->FindAndRetainField( kLineCompletionNumber, inContext );	// Int32
			CDB4DField *returnTypeField = symbolsTable->FindAndRetainField( kSymbolReturnTypeID, inContext );				// String, Comma-separated list of Int32 IDs
			CDB4DField *referenceIDField = symbolsTable->FindAndRetainField( kSymbolReferenceID, inContext );				// Int32

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

			record->SetString(nameField, inSym->GetName() );
			if (inSym->GetFile())
				record->SetLong(fileIdField, inSym->GetFile()->GetID() );
			record->SetString(prototypeField, prototypeStr );
			if (inSym->GetOwner())
				record->SetLong(ownerField, inSym->GetOwner()->GetID() );
			record->SetLong(kindField, inSym->GetFullKindInformation() );
			record->SetString(WAFKindField, inSym->GetWAFKind() );
			record->SetString(scriptDocField,inSym->GetScriptDocComment() );
			record->SetLong(lineNumberField, inSym->GetLineNumber() );
			record->SetLong(lineCompletionNumberField, inSym->GetLineCompletionNumber() );
			record->SetString(returnTypeField, returnTypeStr );
			if (inSym->GetAuxillaryKindInformation() & Symbols::ISymbol::kReferenceValue)
			{
				const Symbols::ISymbol *ref = inSym->Dereference();
				sLONG id = ref->GetID();
				record->SetLong(referenceIDField, ref->GetID() );
				ref->Release();
			}

			if (record->Save())
				ret = true;

			record->Release();

			nameField->Release();
			fileIdField->Release();
			prototypeField->Release();
			ownerField->Release();
			kindField->Release();
			WAFKindField->Release();
			scriptDocField->Release();
			lineNumberField->Release();
			lineCompletionNumberField->Release();
			returnTypeField->Release();
			referenceIDField->Release();
		}
	}

	index->Release();
	symbolsTable->Release();	

	return ret;
}


bool VSymbolTable::InnerSymbolTable::AddFiles( std::vector< Symbols::IFile * > inFiles, CDB4DBaseContext *inContext )
{
	if (!fDatabase)	return false;

	CDB4DTable *filesTable = fDatabase->FindAndRetainTable(	kFileTable, inContext );
	CDB4DField *idField = filesTable->FindAndRetainField( kFileID, inContext );
	
	// Start a transaction
	inContext->StartTransaction();

	bool bSucceeded = true;
	for (std::vector< Symbols::IFile * >::iterator iter = inFiles.begin(); bSucceeded && iter != inFiles.end(); ++iter) {
		// Make a new record for the file
		CDB4DRecord *record = filesTable->NewRecord( inContext );
		if (!record) {
			bSucceeded = false;
			break;
		}
		
		if (!RecordFromFile( *iter, record, inContext ))	bSucceeded = false;

		// Now we can save that record back to the table
		if (bSucceeded && !record->Save()) bSucceeded = false;

		// Now we can set the file's ID
		(*iter)->SetID( GetIntegerField( record, idField, NULL ) );

		record->Release();
	}

	if (bSucceeded)	inContext->CommitTransaction();
	else			inContext->RollBackTransaction();
	
	idField->Release();
	filesTable->Release();
	
	return bSucceeded;
}

CDB4DRecord *VSymbolTable::InnerSymbolTable::FindOrCreateRecordByID( CDB4DTable *inTable, Symbols::ISymbol *inSymbol, CDB4DBaseContext *inContext )
{
	xbox_assert( inSymbol );
	
	if (inSymbol->HasID()) {
		VLong id( inSymbol->GetID() );

		CDB4DQuery *query = inTable->NewQuery();
		if (!query)	{
			return inTable->NewRecord( inContext );
		}

		CDB4DField *idField = inTable->FindAndRetainField( kSymbolID, inContext );
		query->AddCriteria( idField, DB4D_Equal, id );
		idField->Release();

		CDB4DSelection *results = inTable->ExecuteQuery( query, inContext );
		if (!results) {
			query->Release();
			return inTable->NewRecord( inContext );
		}

		CDB4DRecord *ret = NULL;
		if (results->CountRecordsInSelection( inContext )) {
			ret = results->LoadSelectedRecord( 1, DB4D_Keep_Lock_With_Record, inContext );
		} else {
			ret = inTable->NewRecord( inContext );
		}

		query->Release();
		results->Release();

		return ret;
	} else {
		return inTable->NewRecord( inContext );
	}
}

bool VSymbolTable::InnerSymbolTable::AddSymbols( std::vector< Symbols::ISymbol * > inSymbols, CDB4DBaseContext *inContext )
{
	// Adding symbols is a bit trickier than adding files due to relation tracking.  We
	// need to add all of the symbols to the table before setting up any relationships
	// between the symbols.  This is required because there can be circular references
	// between symbols.  For instance, a function has a prototype symbol, but that prototype
	// could be a child symbol of the function itself.  But in that case, the prototype's 
	// owner symbol is the function.  So both symbols must already be in the table (so that
	// their unique IDs are assigned) before we can update the relationship.

	if (!fDatabase)	return false;

	// if a user action needs to access to database for getting suggestions,
	// it will put a lock on all updates so it can get all the db CPU usage 
	// and get faster results, which is important for a user request.
	CheckUpdatesAllowed();

	bool bSucceeded = true;

	// Start a transaction for adding the symbols to the table
	inContext->StartTransaction();
	
	CDB4DTable *symbolsTable = fDatabase->FindAndRetainTable( kSymbolTable, inContext );
	CDB4DSelection *selection = symbolsTable->NewSelection( DB4D_Sel_SmallSel );
	SymbolCollectionManager collection( inSymbols, symbolsTable, inContext );
	{
		StErrorContextInstaller errorContext( VE_DB4D_CANNOTSAVERECORD, VE_DB4D_CANNOT_COMPLETE_COLLECTION_TO_DATA, VE_OK );
		CDB4DSet *locked = NULL;
		if (VE_OK != selection->CollectionToData( collection, inContext, false, false, locked, NULL ))
			bSucceeded = false;
		if (locked)	locked->Release();
	}
	selection->Release();

	if (bSucceeded) {
		inContext->CommitTransaction();
	} else {
		inContext->RollBackTransaction();
	}

	symbolsTable->Release();
	
	return bSucceeded;
}

bool VSymbolTable::InnerSymbolTable::DeleteFile( Symbols::IFile *inFile, CDB4DBaseContext *inContext )
{
	xbox_assert( inFile );
	xbox_assert( inFile->GetID() );

	CheckUpdatesAllowed();

	inContext->StartTransaction();
	
	CDB4DTable *filesTable = fDatabase->FindAndRetainTable( kFileTable, inContext );

	// We need to find the file in the table we're looking for.  This is akin to doing a 
	// SELECT * FROM Files WHERE UniqueID = inFile.UniqueID
	CDB4DQuery *query = filesTable->NewQuery();
	if (!query) {
		filesTable->Release();
		return false;
	}
	CDB4DField *idField = filesTable->FindAndRetainField( kFileID, inContext );

	bool bSucceeded = false;
	VLong id( inFile->GetID() );
	query->AddCriteria( idField, DB4D_Equal, id );

	CDB4DSelection *results = filesTable->ExecuteQuery( query, inContext );
	if (results) {
		// Delete the records we've found. However, we want to ignore any problems due to other threads having
		// locked a record that we're trying to lock.  This can happen when two threads are trying to delete
		// records that cascade into records we're trying to delete as well.  We can safely ignore these issues
		// since the end result is the same: we all want to delete the same stuff.
		StErrorContextInstaller errorContext( VE_DB4D_RECORDISLOCKED, VE_DB4D_CANNOT_DELETE_ALL_RECORDS_IN_SEL, VE_OK );
		CDB4DSet *lockedSet = lockedSet = filesTable->NewSet();
		bSucceeded = (VE_OK == results->DeleteRecords( inContext, lockedSet ));
		lockedSet->Release();
		results->Release();
	}
	
	query->Release();

	if (bSucceeded) {
		inContext->CommitTransaction();
	} else {
		inContext->RollBackTransaction();
	}
	
	idField->Release();
	filesTable->Release();

	return bSucceeded;
}

bool VSymbolTable::InnerSymbolTable::DeleteSymbolsForFile( Symbols::IFile *inFile, CDB4DBaseContext *inContext )
{
	xbox_assert( inFile );

	CheckUpdatesAllowed();

	CDB4DTable *symbolsTable = fDatabase->FindAndRetainTable( kSymbolTable, inContext );
	
	// We need to find the symbols in the table we're looking for.  This is akin to doing a 
	// SELECT * FROM Symbols WHERE FileID = inFile.UniqueID
	CDB4DQuery *query = symbolsTable->NewQuery();
	if (!query) {
		symbolsTable->Release();
		return false;
	}
	CDB4DField *fileField = symbolsTable->FindAndRetainField( kSymbolFileID, inContext );

	VLong id( inFile->GetID() );
	query->AddCriteria( fileField, DB4D_Equal, id );


	CDB4DSelection *results = symbolsTable->ExecuteQuery( query, inContext );
	bool bRet = true;
	if (results) {
		// Delete the records we've found. However, we want to ignore any problems due to other threads having
		// locked a record that we're trying to lock.  This can happen when two threads are trying to delete
		// records that cascade into records we're trying to delete as well.  We can safely ignore these issues
		// since the end result is the same: we all want to delete the same stuff.
		inContext->StartTransaction();
		{
			StErrorContextInstaller errorContext( VE_DB4D_RECORDISLOCKED, VE_DB4D_CANNOT_DELETE_ALL_RECORDS_IN_SEL, VE_OK );
			for (RecIDType i = 1; i <= results->CountRecordsInSelection( inContext ); i++) {
				VErrorDB4D err = symbolsTable->DeleteRecord( results->GetSelectedRecordID( i, inContext ), inContext );
				if (err != VE_OK && err != VE_DB4D_RECORDISLOCKED && err != VE_DB4D_CANNOT_DELETE_ALL_RECORDS_IN_SEL) {
					// Something has gone wrong with deleting the record, and we want to report it back to the user
					bRet = false;
				}
			}
		}
		results->Release();
		inContext->CommitTransaction();
	}
	
	query->Release();
	fileField->Release();
	symbolsTable->Release();
	
	return true;
}


bool VSymbolTable::InnerSymbolTable::DeleteSymbol( Symbols::ISymbol *inSymbol, CDB4DBaseContext *inContext )
{
	xbox_assert( inSymbol );
	xbox_assert( inSymbol->GetID() );

	CheckUpdatesAllowed();

	// When we delete a symbol, all of the child symbols (symbols whose OwnerID = this->UniqueID) are automatically
	// deleted by the database.  We are not going to manually update the prototype symbols that inSymbol may represent.
	// We are allowing them to go stale because the loading code is going to be responsible for handling that scenario.
	CDB4DTable *symbolsTable = fDatabase->FindAndRetainTable( kSymbolTable, inContext );

	// We need to find the symbol in the table we're looking for.  This is akin to doing a 
	// SELECT * FROM Symbols WHERE UniqueID = inSymbol.UniqueID
	CDB4DQuery *query = symbolsTable->NewQuery();
	if (!query) {
		symbolsTable->Release();
		return false;
	}
	CDB4DField *idField = symbolsTable->FindAndRetainField( kSymbolID, inContext );
	
	inContext->StartTransaction();

	bool bSucceeded = false;
	VLong id( inSymbol->GetID() );
	query->AddCriteria( idField, DB4D_Equal, id );

	CDB4DSelection *results = symbolsTable->ExecuteQuery( query, inContext );
	if (results) {
		// Delete the records we've found. However, we want to ignore any problems due to other threads having
		// locked a record that we're trying to lock.  This can happen when two threads are trying to delete
		// records that cascade into records we're trying to delete as well.  We can safely ignore these issues
		// since the end result is the same: we all want to delete the same stuff.
		StErrorContextInstaller errorContext( VE_DB4D_RECORDISLOCKED, VE_DB4D_CANNOT_DELETE_ALL_RECORDS_IN_SEL, VE_OK );
		CDB4DSet *lockedSet = lockedSet = symbolsTable->NewSet();
		bSucceeded = (VE_OK == results->DeleteRecords( inContext, lockedSet ));
		lockedSet->Release();
		results->Release();
	}
	
	query->Release();

	if (bSucceeded) {
		inContext->CommitTransaction();
	} else {
		inContext->RollBackTransaction();
	}

	idField->Release();
	symbolsTable->Release();
	
	return bSucceeded;
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
	
 	if (!fDatabase)	return NULL;

	xbox_assert( inOwnerFile );

	CDB4DTable *symbolsTable = fDatabase->FindAndRetainTable( kSymbolTable, inContext );
	CDB4DQuery *query = symbolsTable->NewQuery();
	if (!query)	{
		symbolsTable->Release();
		return NULL;
	}
	
	CDB4DField *fileField = symbolsTable->FindAndRetainField( kSymbolFileID, inContext );
	CDB4DField *kindField = symbolsTable->FindAndRetainField( kSymbolKind, inContext );
	CDB4DField *lineField = symbolsTable->FindAndRetainField( kLineNumber, inContext );
	CDB4DField *lineCompField = symbolsTable->FindAndRetainField( kLineCompletionNumber, inContext );

	// Symbols.FileID = inOwnerFile.UniqueID
	VLong id( inOwnerFile->GetID() );
	query->AddCriteria( fileField, DB4D_Equal, id );

	// AND Symbols.LineNumber <= inLineNumber
	VLong lineNumber( inLineNumber );
	query->AddLogicalOperator( DB4D_And );
	query->AddCriteria( lineField, DB4D_LowerOrEqual, lineNumber );

	// AND Symbols.LineCompletionNumber >= inLineNumber
	query->AddLogicalOperator( DB4D_And );
	query->AddCriteria( lineCompField, DB4D_GreaterOrEqual, lineNumber );

	CDB4DSelection *results = symbolsTable->ExecuteQuery( query, inContext );
	std::vector< Symbols::ISymbol * > syms;
	if (results) {
		for (RecIDType ctr = 1; ctr <= results->CountRecordsInSelection( inContext ); ctr++) {
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			if (record) {
				syms.push_back( SymbolFromRecord( symbolsTable, record, inContext) );
				record->Release();
			}
		}
		results->Release();
	}
	query->Release();

	// Now that we have a list of symbols, we want to find the one that is a narrowest match
	// to our given line number.  Note that it is possible we don't have any match at all!
	Symbols::ISymbol *ret = NULL;
	for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter) {
		if (!ret) {
			// We've not picked any match yet, so this one is good enough
			if ( (*iter)->IsFunctionKind() )
				ret = *iter;
		} else {
			// We have something to test against.  Check to see if the iterator's distance to the
			// line is less than the current return symbol's distance.  This will tell us which symbol
			// is closer.
			if ((inLineNumber - (*iter)->GetLineNumber()) < (inLineNumber - ret->GetLineNumber())) {
				ret = *iter;
			}
		}
	}

	// Now we need to clean up all of the symbols except for the one we're returning to the caller
	if (ret)	ret->Retain();
	for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter)	(*iter)->Release();
	
	fileField->Release();
	kindField->Release();
	lineField->Release();
	lineCompField->Release();
	symbolsTable->Release();
	return ret;
}

std::vector< Symbols::IExtraInfo * > VSymbolTable::InnerSymbolTable::GetExtraInformation( const Symbols::ISymbol *inOwnerSymbol, Symbols::IExtraInfo::Kind inKind, CDB4DBaseContext *inContext )
{
	std::vector< Symbols::IExtraInfo * > ret;
	if (!fDatabase)	return ret;
	
	CDB4DTable *extrasTable = fDatabase->FindAndRetainTable( kExtrasTable, inContext );
	CDB4DField *ownerField = extrasTable->FindAndRetainField( kExtrasOwnerSymbolID, inContext );
	CDB4DField *kindField = extrasTable->FindAndRetainField( kExtrasKind, inContext );

	// Let's get a new query object for us to fill out
	CDB4DQuery *query = extrasTable->NewQuery();
	query->AddCriteria( ownerField, DB4D_Equal, VLong( inOwnerSymbol->GetID() ) );
	query->AddLogicalOperator( DB4D_And );
	query->AddCriteria( kindField, DB4D_Equal, VLong( (sLONG)inKind ) );
	CDB4DSelection *results = extrasTable->ExecuteQuery( query, inContext );
	if (results) {
		for (RecIDType ctr = 1; ctr <= results->CountRecordsInSelection( inContext ); ctr++) {
			CDB4DRecord *record = results->LoadSelectedRecord( ctr, DB4D_Do_Not_Lock, inContext );
			ret.push_back( ExtraInfoFromRecord( extrasTable, record, inContext ) );
			record->Release();
		}
		results->Release();
	}
	query->Release();
	
	kindField->Release();
	ownerField->Release();
	extrasTable->Release();
	
	return ret;
}

bool VSymbolTable::InnerSymbolTable::AddExtraInformation( const Symbols::ISymbol *inOwnerSymbol, const Symbols::IExtraInfo *inInfo, CDB4DBaseContext *inContext )
{
	if (!fDatabase)	return false;

	CheckUpdatesAllowed();

	CDB4DTable *extrasTable = fDatabase->FindAndRetainTable( kExtrasTable, inContext );
	
	bool bSucceeded = true;
	// Make a new record for the information
	CDB4DRecord *record = extrasTable->NewRecord( inContext );
	if (!record) bSucceeded = false;
	
	if (bSucceeded && !RecordFromExtraInfo( inOwnerSymbol, inInfo, record, inContext ))	bSucceeded = false;

	// Now we can save that record back to the table
	if (bSucceeded && !record->Save()) bSucceeded = false;

	record->Release();
	extrasTable->Release();
	return bSucceeded;
}

ISymbolTable *NewSymbolTable()
{
	return new VSymbolTable();
}

#if _DEBUG
void TestSymbolNames( const std::vector< Symbols::ISymbol * > &syms, int count, ... )
{
	// If the caller expects no symbols, make sure that's true
	if (!count)	xbox_assert( syms.empty() );

	long numberFound = 0;
	for (std::vector< Symbols::ISymbol * >::const_iterator iter = syms.begin(); iter != syms.end(); ++iter) {
		VString iterName = (*iter)->GetName();

		va_list marker;
		va_start( marker, count );
		bool found = false;
		for (int i = 0; !found && i < count; i++) {
			if (iterName.EqualToString( va_arg( marker, char * ), true )) found = true;
		}
		va_end( marker );
		xbox_assert( found );
		if (found)	numberFound++;
	}

	xbox_assert( numberFound == count );
}

void SymbolTableTest::Test()
{
	// Note that all of the symbols we are creating are being leaked.  It'd be good
	// to clean those up at some point, but for right now, leaking them is easier.  Since
	// it is only in a debug build, it won't be the end of the world.
	VSymbolTable *table = new VSymbolTable();
	VFile *temp = VFile::CreateTemporaryFile();
	xbox_assert( temp );

	// We don't want the temp file to actually *exist* because the database
	// will try to read it in as a .4db file.  So we delete it
	temp->Delete();

	xbox_assert( table->OpenSymbolDatabase( *temp ) );

	// Let's make sure we're able to add some symbols to our new database.  First,
	// we'll make up a file that holds the symbols
	VSymbolFile *symFile = new VSymbolFile();
	symFile->SetPath( "TestFile.js" );

	// TEST
	symFile->SetBaseFolder( eSymbolFileBaseFolderProject );
	symFile->SetExecutionContext( eSymbolFileExecContextServer );


	symFile->SetModificationTimestamp( 0x0000000000000001 );

	VSymbolFile *jsCoreFile = new VSymbolFile();
	jsCoreFile->SetPath( "Java Script Core" );

	// TEST
	jsCoreFile->SetBaseFolder( eSymbolFileBaseFolderProject );
	jsCoreFile->SetExecutionContext( eSymbolFileExecContextServer );

	jsCoreFile->SetModificationTimestamp( 0x0000000000000000 );

	// Now we can make some symbols for that file to hold.  Here's the corresponding
	// JavaScript code for those symbols
	//
	// function WahooFunction() {
	//		var bar = 12;
	//		this.baz = "test";
	// }
	VSymbol *wahooFunction = new VSymbol();
	wahooFunction->SetID( table->GetNextUniqueID() );
	wahooFunction->SetName( "WahooFunction" );
	wahooFunction->SetFile( symFile );
	wahooFunction->SetKind( 2 );

	VSymbol *wahooPrototype = new VSymbol();
	wahooPrototype->SetID( table->GetNextUniqueID() );
	wahooPrototype->SetName( "prototype" );
	wahooPrototype->SetFile( jsCoreFile );
	wahooPrototype->SetKind( 4 );
	wahooPrototype->SetOwner( wahooFunction );
	wahooFunction->AddPrototype( wahooPrototype );

	VSymbol *barSymbol = new VSymbol();
	barSymbol->SetID( table->GetNextUniqueID() );
	barSymbol->SetName( "bar" );
	barSymbol->SetKind( 1 );
	barSymbol->SetFile( symFile );
	barSymbol->SetOwner( wahooFunction );

	VSymbol *bazSymbol = new VSymbol();
	bazSymbol->SetID( table->GetNextUniqueID() );
	bazSymbol->SetName( "baz" );
	bazSymbol->SetKind( 1 );
	bazSymbol->SetFile( symFile );
	bazSymbol->SetOwner( wahooPrototype );

	// Now that we've got some symbols, we can add them to the database.
	std::vector< Symbols::IFile * > files;
	files.push_back( symFile );
	files.push_back( jsCoreFile );
	xbox_assert( table->AddFiles( files ) );

	std::vector< Symbols::ISymbol * > syms;
	syms.push_back( bazSymbol );
	syms.push_back( barSymbol );
	syms.push_back( wahooPrototype );
	syms.push_back( wahooFunction );
	xbox_assert( table->AddSymbols( syms ) );
	syms.clear();

	// Let's see if we can get all of the symbols owned by WahooFunction
	syms = table->GetNamedSubSymbols( wahooFunction );
	TestSymbolNames( syms, 2, "bar", "prototype" );

	// Let's also test to make sure we can get all the symbols owned by its prototype
	syms = table->GetNamedSubSymbols( wahooPrototype );
	TestSymbolNames( syms, 1, "baz" );
	syms.clear();

	// Now let's see if we can get a named subsymbol
	syms = table->GetSymbolsByName( "WahooFunction" );
	TestSymbolNames( syms, 1, "WahooFunction" );
	syms.clear();

	syms = table->GetSymbolsByName( wahooPrototype, "baz" );
	TestSymbolNames( syms, 1, "baz" );
	syms.clear();

	// Make sure that no subsymbols come back when it's expected
	syms = table->GetSymbolsByName( "fluffy" );
	xbox_assert( syms.size() == 0 );

	// Let's delete a symbol with children and make sure that it really
	// goes away.
	xbox_assert( table->DeleteSymbol( wahooPrototype ) );
	syms = table->GetSymbolsByName( "baz" );
	xbox_assert( syms.empty() );
	syms.clear();

	// Since we deleted a prototype, we also want to make sure any symbols
	// which relied on that prototype are updated to reflect the fact they
	// no longer have a prototype.
	syms = table->GetSymbolsByName( "WahooFunction" );
	TestSymbolNames( syms, 1, "WahooFunction" );
	xbox_assert( syms[ 0 ]->GetPrototypes().empty() );
	syms.clear();

	// Let's add the prototype back on to the function and make sure the
	// state is updated properly.
	syms.push_back( wahooFunction );
	syms.push_back( wahooPrototype );
	syms.push_back( bazSymbol );
	xbox_assert( table->AddSymbols( syms ) );
	syms.clear();
	syms = table->GetSymbolsByName( wahooFunction, "prototype" );
	TestSymbolNames( syms, 1, "prototype" );
	syms.clear();
	syms = table->GetSymbolsByName( wahooPrototype, "baz" );
	TestSymbolNames( syms, 1, "baz" );
	syms.clear();
	syms = table->GetSymbolsByName( "WahooFunction" );
	TestSymbolNames( syms, 1, "WahooFunction" );
	xbox_assert( !syms[ 0 ]->GetPrototypes().empty() );
	syms.clear();

	// Delete all of the JSCore data
	xbox_assert( table->DeleteFile( jsCoreFile ) );
	// Make sure we no longer can find a prototype
	syms = table->GetSymbolsByName( wahooFunction, "prototype" );
	xbox_assert( syms.empty() );
	// When we ask to get the WahooFunction symbol, make sure it has no prototype set
	syms = table->GetSymbolsByName( "WahooFunction" );
	TestSymbolNames( syms, 1, "WahooFunction" );
	xbox_assert( syms[ 0 ]->GetPrototypes().empty() );
	syms.clear();

	// Add the JSCore file back to the files table, and add prototype back to the symbols table
	files.clear();
	files.push_back( jsCoreFile );
	xbox_assert( table->AddFiles( files ) );
	syms.push_back( wahooPrototype );
	syms.push_back( wahooFunction );
	syms.push_back( bazSymbol );
	xbox_assert( table->AddSymbols( syms ) );
	syms.clear();

	symFile->Release();
	jsCoreFile->Release();
	table->Release();
	temp->Delete();
	// Note that this still leaves the .4dd file around.  But it'll be in the temp
	// directory, so I think it's fine.
	temp->Release();
}
#endif // _DEBUG
