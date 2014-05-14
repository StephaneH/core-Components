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
#ifndef __EXPORT__
#define __EXPORT__

class FolderPosStack
{
	public:
		FolderPosStack(sLONG maxlevel, sLONG NbfilesPerLevel, VFolder* inBaseFolder, const VString& inSubFolderName);
		~FolderPosStack();

		sLONG GetNextPos(VFolder* &outFolder, VError& err);


	protected:
		void _GetNextPos(sLONG level, bool& outFolderHasChanged);

		sLONG fMaxLevel;
		sLONG fNbfilesPerLevel;
		vector<sLONG> fFolderPos;
		VFolder* fBaseFolder;
		VFolder* fCurFolder;
		VString fSubFolderName;

};


class ExportJob
{
	public:

		ExportJob(Table* inTable, VFolder* inBaseFolder, VProgressIndicator* inProgress, sLONG inMaxBlobs, ExportOption& options)
		{
			fTable = RetainRefCountable(inTable);
			fBaseFolder = RetainRefCountable(inBaseFolder);
			fNbBlobsPerLevel = options.NbBlobsPerLevel;
			fProgress = RetainRefCountable(inProgress);
			fCreateFolder = options.CreateFolder;
			fBinaryExport = options.BinaryExport;
			fJSONExport = options.JSONExport;
			fImport = options.Import;
			fNbFicToImport = 0;
			fBlobThresholdSize = options.BlobThresholdSize;
			fRows = nil;
			_Init(inMaxBlobs, options.MaxSQLTextSize);
		}

		~ExportJob()
		{
			QuickReleaseRefCountable(fRows);
			QuickReleaseRefCountable(fTable);
			QuickReleaseRefCountable(fBaseFolder);
			QuickReleaseRefCountable(fProgress);

			if (fBlobFolders != nil)
				delete fBlobFolders;

			CloseDataStream();
			/*
			if (fPictFolders != nil)
				delete fPictFolders;

			if (fTextFolders != nil)
				delete fTextFolders;
			*/
		}

		inline VFolder* GetRecordFolder()
		{
			return fBaseFolder;
		}

		sLONG GetNextBlob(VFolder* &outFolder, VError& err);

		inline sLONG GetNextPict(VFolder* &outFolder, VError& err)
		{
			return GetNextBlob(outFolder, err);
		}

		inline sLONG GetNextText(VFolder* &outFolder, VError& err)
		{
			return GetNextBlob(outFolder, err);
		}

		inline sLONG GetNbFicToImport() const
		{
			return fNbFicToImport;
		}

		VError StartJob(sLONG NbFicToExport);
		VError StopJob();

		VError PutSqlHeader();
	
		VError CreateDataStream();
		VError OpenDataStream();
		VError CloseDataStream();

		//VError WriteOneRecord(FicheOnDisk* ficD, BaseTaskInfo* context);
		VError WriteOneRecord(FicheInMem* fic, BaseTaskInfo* context);

		FicheInMem* ReadOneRecord(BaseTaskInfo* context, VError& err);


	protected:
		void _Init(sLONG inMaxBlobs, sLONG8 inMaxFileSize);
		sLONG _ComputeMaxLevel(sLONG maxelems, sLONG nbelemsPerLine);

		VFolder* fBaseFolder;
		Table* fTable;
		VProgressIndicator* fProgress;
		sLONG fNbBlobLevels;
		sLONG fNbBlobsPerLevel;
		sLONG fNbLinesPerFile;
		sLONG8 fMaxFileSize;
		bool fCreateFolder, fBinaryExport, fImport, fJSONExport;
		VString fFileName;
		VString fBlobName;
		VString fTextName;
		VString fPictName;
		VString fFileExt;
		VString fBlobExt;
		VString fTextExt;
		VString fPictExt;

		FolderPosStack* fBlobFolders;
		//FolderPosStack* fPictFolders;
		//FolderPosStack* fTextFolders;

		VFileStream* fCurrentData;

		sLONG fCurrentDataNumber;
		sLONG fCurrentLine;
		sLONG fCurrentBlob;
		sLONG numblob;
		sLONG fNbFicToImport;
		sLONG fBlobThresholdSize;

		VJSONArray* fRows;
	
	private:
	
		// Size (in characters) for hexadecimal dump buffer using SQL scripts 
		// (small blobs, texts, or pictures). (Default size is rather small
		// because VFileStream writes are already buffered (0x10000 sized)).
	
		static const sLONG kDumpBufferSize = 128;
	
		static const UniChar kHexTable[16];
	
		uBYTE	fBinaryDumpBuffer[kDumpBufferSize];
		UniChar fTextDumpBuffer[2 * kDumpBufferSize + 1];
	
		VError	_DumpBlob (VBlob *blob);
		VError	_DumpBytes (sLONG numberBytes);

		VError SetPropertyAsHex (VJSONObject* recobj, const VString& fieldname, VBlob *blob);

};

#endif

