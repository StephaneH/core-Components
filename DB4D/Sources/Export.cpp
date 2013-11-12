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
#include "4DDBHeaders.h"


FolderPosStack::FolderPosStack(sLONG maxlevel, sLONG NbfilesPerLevel, VFolder* inBaseFolder, const VString& inSubFolderName)
{
	fMaxLevel = maxlevel;
	fNbfilesPerLevel = NbfilesPerLevel;
	fFolderPos.resize(maxlevel, 0);
	fFolderPos[fMaxLevel-1] = -1;
	fCurFolder = nil;
	fBaseFolder = RetainRefCountable(inBaseFolder);
	fSubFolderName = inSubFolderName;
}


FolderPosStack::~FolderPosStack()
{
	QuickReleaseRefCountable(fBaseFolder);
	QuickReleaseRefCountable(fCurFolder);
}



void FolderPosStack::_GetNextPos(sLONG level, bool& outFolderHasChanged)
{
	sLONG* cur = &(fFolderPos[level-1]);
	(*cur)++;
	if (*cur >= fNbfilesPerLevel && level > 1)
	{
		*cur = 0;
		outFolderHasChanged = true;
		_GetNextPos(level-1, outFolderHasChanged);
	}
}


sLONG FolderPosStack::GetNextPos(VFolder* &outFolder, VError& err)
{
	err = VE_OK;
	bool FolderHasChanged = false;
	_GetNextPos(fMaxLevel, FolderHasChanged);
	sLONG result = 0;
	for (sLONG i = 1; i <= fMaxLevel; i++)
	{
		result = result * fNbfilesPerLevel;
		result = result + fFolderPos[i-1];
	}

	if (FolderHasChanged || fCurFolder == nil)
	{
		QuickReleaseRefCountable(fCurFolder);
		if (!fBaseFolder->Exists())
			err = fBaseFolder->Create();

		VFolder* subFolder = RetainRefCountable(fBaseFolder);
		for (sLONG i = 1; i < fMaxLevel && err == VE_OK; i++)
		{
			VFolder* sub2 = new VFolder(*subFolder, fSubFolderName+ToString(fFolderPos[i-1]));
			if (!sub2->Exists())
				err = sub2->Create();
			QuickReleaseRefCountable(subFolder);
			subFolder = sub2;
		}
		fCurFolder = subFolder;
	}
	
	outFolder = fCurFolder;

	return result;
}



// ----------------------------------------------------------------------------

const UniChar ExportJob::kHexTable[16] = { 
	
	L'0', L'1', L'2', L'3',
	L'4', L'5', L'6', L'7',
	L'8', L'9', L'a', L'b',
	L'c', L'd', L'e', L'f',
	
};

sLONG ExportJob::_ComputeMaxLevel(sLONG maxelems, sLONG nbelemsPerLine)
{
	if (nbelemsPerLine < 50)
		nbelemsPerLine = 50;
	sLONG nb = 1;
	while (maxelems > nbelemsPerLine)
	{
		nb++;
		maxelems = maxelems / nbelemsPerLine;
	}
	return nb;
}


void ExportJob::_Init(sLONG inMaxBlobs, sLONG8 inMaxFileSize)
{
	fNbBlobLevels = _ComputeMaxLevel(inMaxBlobs, fNbBlobsPerLevel);
	fNbLinesPerFile = -1;
	if (fBinaryExport)
		fMaxFileSize = 0;
	else
		fMaxFileSize = inMaxFileSize;
	if (fMaxFileSize == 0 && !fBinaryExport)
		fMaxFileSize = 100*1024*1024;
	if (fMaxFileSize < 100000 && fMaxFileSize != 0)
		fMaxFileSize = 100000;
	fFileName = L"Export";
	fBlobName = L"Blob";
	fTextName = L"Text";
	fPictName = L"Pict";
	if (fBinaryExport)
		fFileExt = L".binary";
	else
		fFileExt = L".sql";
	fBlobExt = L".BLOB";
	fTextExt = L".txt";
	fPictExt = L".4PCT";

	fBlobFolders = nil;
	fCurrentData = nil;
	//fPictFolders = nil;
	//fTextFolders = nil;

	fCurrentLine = 0;
	fCurrentBlob = 0;
	fCurrentDataNumber = 0;
	numblob = 0;

}


sLONG ExportJob::GetNextBlob(VFolder* &outFolder, VError& err)
{
	outFolder = nil;
	if (fBlobFolders == nil)
	{
		VFolder* blobfolder = new VFolder(*fBaseFolder, L"BLOBS");
		fBlobFolders = new FolderPosStack(fNbBlobLevels, fNbBlobsPerLevel, blobfolder, L"Blobs");
		blobfolder->Release();
	}
	return fBlobFolders->GetNextPos(outFolder, err);
}


VError ExportJob::PutSqlHeader()
{
	VString s;
	VError err = fCurrentData->PutText(L"INSERT INTO [");
	fTable->GetName(s);
	if (err == VE_OK)
		err = fCurrentData->PutText(s);
	
	if (err == VE_OK)
		err = fCurrentData->PutText(L"] ( ");
	if (err == VE_OK)
	{
		bool first = true;
		sLONG nb = fTable->GetNbCrit();
		for (sLONG i = 1; i<=nb && err == VE_OK; i++)
		{
			VString name;
			Field* crit = fTable->RetainField(i);
			if (crit != nil)
			{
				crit->GetName(name);
				if (first)
					first = false;
				else
					err = fCurrentData->PutText(L" , ");
				if (err == VE_OK)
					err = fCurrentData->PutText(L"["+name+L"]");
				crit->Release();
			}
		}
		
	}
	if (err == VE_OK)
		err = fCurrentData->PutText(L" )\nVALUES\n");
		
	return err;	
	
}


const sLONG SensHeader = 0x44014402;

VError ExportJob::OpenDataStream()
{
	VError err = CloseDataStream();
	if (err == VE_OK)
	{
		fCurrentDataNumber++;
		fCurrentLine = 0;
		VString s = fFileName;
		if (fCurrentDataNumber != 1)
			s = s + ToString(fCurrentDataNumber);
		s = s + fFileExt;

		VFile* datafile = new VFile(*fBaseFolder, s);
		if (err == VE_OK)
		{
			fCurrentData = new VFileStream(datafile);
			err = fCurrentData->OpenReading();
			if (fBinaryExport)
			{
				if (err == VE_OK)
				{
					sLONG ll;
					err = fCurrentData->GetLong(ll);
					if (ll != SensHeader)
					{
						ByteSwap(&ll);
						if (ll == SensHeader)
							fCurrentData->SetNeedSwap(true);
						else
							err = ThrowBaseError(VE_DB4D_WRONG_STREAM_HEADER);
					}
				}
			}
			else
			{
				if (err == VE_OK)
				{
					err = fCurrentData->GuessCharSetFromLeadingBytes(VTC_UTF_8);
				}
			}
		}
	}

	return err;
}


VError ExportJob::CreateDataStream()
{
	VError err = CloseDataStream();
	if (err == VE_OK)
	{
		fCurrentDataNumber++;
		fCurrentLine = 0;
		VString s = fFileName;
		if (fCurrentDataNumber != 1)
			s = s + ToString(fCurrentDataNumber);
		s = s + fFileExt;

		VFile* datafile = new VFile(*fBaseFolder, s);
		err = datafile->Create();
		if (err == VE_OK)
		{
			fCurrentData = new VFileStream(datafile);
			err = fCurrentData->OpenWriting();
			if (fBinaryExport)
			{
				if (err == VE_OK)
					err = fCurrentData->PutLong(SensHeader);
			}
			else
			{
				if (err == VE_OK)
				{
					fCurrentData->SetCharSet(VTC_UTF_8 );
					fCurrentData->SetCarriageReturnMode(eCRM_NATIVE );
					err = fCurrentData->WriteBOM();
					if (err == VE_OK)
						err = PutSqlHeader();
				}
			}
		}
	}

	return err;
}


VError ExportJob::CloseDataStream()
{
	VError err = VE_OK;
	if (fCurrentData != nil)
	{
		if (fImport)
		{
			err = fCurrentData->CloseReading();
		}
		else
		{
			if (!fBinaryExport)
				err = fCurrentData->PutText(L";\n");
			err = fCurrentData->CloseWriting();
		}
		delete fCurrentData;
		fCurrentData = nil;
	}
	return err;
}



VError ExportJob::StopJob()
{
	VError err = CloseDataStream();
	return err;
}


VError ExportJob::StartJob(sLONG NbFicToExport)
{
	VError err = VE_OK;

	if (fImport)
	{
		if (fBaseFolder->Exists() && fCreateFolder)
		{
			VString foldername;
			if (fBinaryExport)
				foldername = L"BinaryExport";
			else
				foldername = L"SQLExport";
			VFolder* subfolder = new VFolder(*fBaseFolder, foldername);
			fBaseFolder->Release();
			fBaseFolder = subfolder;
		}

		err = OpenDataStream();
		if (err == VE_OK && fBinaryExport)
		{
			err = fCurrentData->GetLong(fNbFicToImport);
		}
	}
	else
	{

		if (!fBaseFolder->Exists())
		{
			err = fBaseFolder->Create();
		}

		if (fCreateFolder)
		{
			VString foldername;
			if (fBinaryExport)
				foldername = L"BinaryExport";
			else
				foldername = L"SQLExport";
			VFolder* subfolder = new VFolder(*fBaseFolder, foldername);
			fBaseFolder->Release();
			fBaseFolder = subfolder;
		}

		if (fBaseFolder->Exists())
		{
			err = fBaseFolder->DeleteContents(true);
		}
		else
			err = fBaseFolder->Create();

		
		err = CreateDataStream();
		if (err == VE_OK && fBinaryExport)
		{
			err = fCurrentData->PutLong(NbFicToExport);
		}
	}

	return err;
}


bool ExportJob::ReadOneRecord(FicheInMem* fic, BaseTaskInfo* context, VError& err)
{
	bool okrec = false;
	sLONG nbcrit = 0;
	err = fCurrentData->GetLong(nbcrit);
	if (err == VE_OK)
	{
		if (nbcrit != -1)
		{
			for (sLONG i = 0; i < nbcrit && err == VE_OK; i++)
			{
				sLONG TypeInRec = 0;
				err = fCurrentData->GetLong(TypeInRec);
				if (TypeInRec != VK_EMPTY && err == VE_OK)
				{
					VValueSingle* cvfrom = (VValueSingle*)VValue::NewValueFromValueKind(TypeInRec);
					if (cvfrom != nil)
					{
						err = cvfrom->ReadFromStream(fCurrentData);
						if (err == VE_OK)
						{
							VValueSingle* cv = fic->GetNthField(i+1, err);
							if (cv != nil)
							{
								cv->FromValue(*cvfrom);
								fic->Touch(i+1);
							}
						}
						delete cvfrom;
					}
				}
			}
			if (err == VE_OK)
				okrec = true;
		}
	}

	return okrec;
}



VError ExportJob::WriteOneRecord(FicheInMem* fic, BaseTaskInfo* context)
{
	VError err = VE_OK;
	if (fProgress != nil)
		fProgress->Increment();

	if (MustStopTask(fProgress))
		err = ThrowBaseError(VE_DB4D_ACTIONCANCELEDBYUSER);

	if (err == VE_OK)
	{
		if (fCurrentData->GetSize() > fMaxFileSize && fMaxFileSize != 0)
		{
			err = CloseDataStream();
			if (err == VE_OK)
				err = CreateDataStream();
		}
	}
	if (err == VE_OK)
	{
		if (fBinaryExport)
		{
			sLONG nb = fTable->GetNbCrit();
			err = fCurrentData->PutLong(nb);
			for (sLONG i = 1; i <= nb && err == VE_OK; i++)
			{
				VString s;
				Field* crit = fTable->RetainField(i);
				if (crit != nil)
				{
					sLONG TypeInRec;
					VValueSingle* cv = fic->GetNthField(i, err);
					if (cv == nil || cv->IsNull())
					{
						TypeInRec = VK_EMPTY;
					}
					else
					{
						TypeInRec = cv->GetTrueValueKind();
					}
					err = fCurrentData->PutLong(TypeInRec);
					if (err == VE_OK && TypeInRec != VK_EMPTY)
					{
						err = cv->WriteToStream(fCurrentData);
					}
					crit->Release();
				}
			}

		}
		else
		{
			if (fCurrentLine != 0)
			{
				if (fCurrentLine >= 1000)
				{
					err = fCurrentData->PutText(L";\n\n");
					err = PutSqlHeader();
					fCurrentLine = 0;
				}
				else
					err = fCurrentData->PutText(L",\n");
			}
			fCurrentLine++;
			sLONG nb = fTable->GetNbCrit();
			bool first = true;
			err = fCurrentData->PutText(L"(");
			for (sLONG i = 1; i <= nb && err == VE_OK; i++)
			{
				VString s;
				Field* crit = fTable->RetainField(i);
				if (crit != nil)
				{
					if (first)
						first = false;
					else
						err = fCurrentData->PutText(L" , ");
					if (err == VE_OK)
					{
						VValueSingle* cv = fic->GetNthField(i, err);
						if (cv == nil || cv->IsNull())
						{
							err = fCurrentData->PutText(L"NULL");
						}
						else
						{
							sLONG TypeInRec = cv->GetTrueValueKind();
							switch(TypeInRec)
							{
								case VK_BLOB:
								case VK_BLOB_DB4D:
									{
										VBlob* cvblob = (VBlob*)cv;
										
										if (cvblob->GetSize() <= fBlobThresholdSize) {
										
											err = _DumpBlob(cvblob);
											
										} else {
										
											VFolder* folder = nil;
											sLONG notused = GetNextBlob(folder, err);
											if (err == VE_OK)
											{
												VString blobname = fBlobName+ToString(numblob)+fBlobExt;
												VFile* blobfile = new VFile(*folder, blobname);
												err = blobfile->Create();
												if (err == VE_OK)
												{
													VFileStream blobdata(blobfile);
													err = blobdata.OpenWriting();
													if (err == VE_OK)
													{
														err = cvblob->WriteToStream(&blobdata);
														if (err == VE_OK)
															err = blobdata.CloseWriting();
														else
															blobdata.CloseWriting();
													}
													if (err == VE_OK)
													{
														VString filename;
														blobfile->GetRelativeURL(fBaseFolder, filename, false);
														err = fCurrentData->PutText(L"INFILE '"+filename+L"'");
													}
												}
												numblob++;
											}
											
										}
									}
									break;
		
								case VK_TEXT:
								case VK_TEXT_UTF8:
									{
										VBlobText* letext = (VBlobText*)cv;
										VIndex length = letext->GetLength();
										
										if (length * sizeof(UniChar) <= fBlobThresholdSize) {
											
											const UniChar *p = letext->GetCPointer();
											sLONG n, i, j;
											
											err = fCurrentData->PutText(L"'");
											while (err == VE_OK && length) {
												
												n = length >= kDumpBufferSize ? kDumpBufferSize : length;
												for (i = j = 0; i < n; i++) {
												
													// Quote the \' character.
													
													if (*p == L'\'') {
														
														fTextDumpBuffer[j++] = L'\'';
														
													}
													fTextDumpBuffer[j++] = *p++;
													
												}
												fTextDumpBuffer[j] = L'\0';
												length -= n;

												ECarriageReturnMode		crModeOld = fCurrentData->GetCarriageReturnMode();
												fCurrentData->SetCarriageReturnMode(eCRM_NONE);
												err = fCurrentData->PutText(VString(fTextDumpBuffer, -1));
												fCurrentData->SetCarriageReturnMode(crModeOld);
												if (err != VE_OK)
													break;
												
											}
											if (err == VE_OK) {
												
												err = fCurrentData->PutText(L"'");
												
											}
											
										} else { 

											VFolder* folder = nil;
											sLONG notused = GetNextBlob(folder, err);
											if (err == VE_OK)
											{
												VString blobname = fTextName+ToString(numblob)+fTextExt;
												VFile* blobfile = new VFile(*folder, blobname);
												err = blobfile->Create();
												if (err == VE_OK)
												{
													VFileStream blobdata(blobfile);
													err = blobdata.OpenWriting();
													if (err == VE_OK)
													{
														blobdata.SetCharSet(VTC_UTF_8);
														blobdata.SetCarriageReturnMode(eCRM_NATIVE);
														err = blobdata.WriteBOM();
														if (err == VE_OK)
															err = blobdata.PutText(*letext);
														if (err == VE_OK)
															err = blobdata.CloseWriting();
														else
															blobdata.CloseWriting();
													}
													if (err == VE_OK)
													{
														VString filename;
														blobfile->GetRelativeURL(fBaseFolder, filename, false);
														err = fCurrentData->PutText(L"INFILE '"+filename+L"'");
													}
												}
												numblob++;
											}
										}
									}
									break;

								case VK_IMAGE:
#if !VERSION_LINUX   // Postponed Linux Implementation !
									{
										if (cv->GetFullSpaceOnDisk() <= fBlobThresholdSize) {

											VPicture *vPicture = (VPicture *) cv;
											VBlobWithPtr blob;
		 
											if ((err = vPicture->SaveToBlob(&blob, true, 0)) == VE_OK) {
												
												err = _DumpBlob(&blob);

											}
											
										} else {
											
											VFolder* folder = nil;
											sLONG notused = GetNextBlob(folder, err);
											if (err == VE_OK)
											{
												sLONG xnumblob = numblob;
												VString extension;
												err = FullExportPicture_GetOutputFileKind(*cv, extension);
												if (err == VE_OK)
												{
													VString blobname = fPictName+ToString(numblob)+L"."+extension;
													VFile* blobfile = new VFile(*folder, blobname);
													err = blobfile->Create();
													if (err == VE_OK)
													{
														VFileStream blobdata(blobfile);
														err = blobdata.OpenWriting();
														if (err == VE_OK)
														{
															err = FullExportPicture_Export(*cv, blobdata);
															if (err == VE_OK)
																err = blobdata.CloseWriting();
															else
																blobdata.CloseWriting();
														}
													}
													if (err == VE_OK)
													{
														VString filename;
														blobfile->GetRelativeURL(fBaseFolder, filename, false);
														err = fCurrentData->PutText(L"INFILE '"+filename+L"'");
													}
													
												}
												numblob++;
											}
										}
									}
#endif  //(Postponed Linux Implementation)
									break;

								case VK_STRING:
								case VK_STRING_UTF8:
									{
										VString s2;
										cv->GetString(s);
										sLONG len = s.GetLength();
										UniChar* dest = s2.GetCPointerForWrite(len*2+4);
										const UniChar* source = s.GetCPointer();
										sLONG len2 = 1;
										*dest = 39;
										dest++;
										for (sLONG i = 0; i < len; i++)
										{
											UniChar c = *source;
											source++;
											*dest = c;
											dest++;
											len2++;
											if (c == 39)
											{
												*dest = c;
												dest++;
												len2++;
											}
										}
										*dest = 39;
										len2++;
										s2.Validate(len2);

										ECarriageReturnMode		crModeOld = fCurrentData->GetCarriageReturnMode();
										fCurrentData->SetCarriageReturnMode(eCRM_NONE);
										err = fCurrentData->PutText(s2);
										fCurrentData->SetCarriageReturnMode(crModeOld);
									}
									break;

								case VK_TIME:
									{
										sWORD year, month, day, hour, minute, second, millisecond;
										char buffer[255];
										((VTime*)cv)->GetUTCTime(year, month, day, hour, minute, second, millisecond);
	#if COMPIL_VISUAL
										sprintf( buffer, "'%04d/%02d/%02d %02d:%02d:%02d:%02d'", year, month, day, hour, minute, second, millisecond);
	#else
										snprintf( buffer, sizeof( buffer), "'%04d/%02d/%02d %02d:%02d:%02d:%02d'", year, month, day, hour, minute, second, millisecond);
	#endif
										s.FromCString( buffer);
										err = fCurrentData->PutText(s);
									}
									break;

								case VK_BOOLEAN:
									if (cv->GetBoolean())
										s = L"1";
									else
										s = L"0";
									err = fCurrentData->PutText(s);
									break;

								case VK_REAL:
									cv->GetXMLString(s, XSO_Default);
									err = fCurrentData->PutText(s);
									break;

								case VK_UUID:
								case VK_DURATION:
									err = fCurrentData->PutText(L"'");
									if (err == VE_OK)
									{
										cv->GetString(s);
										err = fCurrentData->PutText(s);
										if (err == VE_OK)
											err = fCurrentData->PutText(L"'");
									}
									break;

								default:
									cv->GetString(s);
									err = fCurrentData->PutText(s);
									break;
							}

						}
					}
					crit->Release();
				}
			}
			if (err == VE_OK)
			{
				err = fCurrentData->PutText(L")");
			}
		}
	}

	return err;
}

VError ExportJob::_DumpBlob (VBlob *blob)

{
	VError err;
	VSize size = blob->GetSize();
	sLONG index, numberBytes;
	
	if ((err = fCurrentData->PutText(L"X'")) == VE_OK) {
	
		index = 0;
		while (size) {
			
			numberBytes = (sLONG)size >= kDumpBufferSize ? kDumpBufferSize : (sLONG)size;
			if ((err = blob->GetData(fBinaryDumpBuffer, numberBytes, index)) != VE_OK) {
				
				break;
				
			}
			if ((err = _DumpBytes(numberBytes)) != VE_OK) {
				
				break;
				
			}
			
			index += numberBytes;
			size -= numberBytes;
			
		}
		
	}
	if (err == VE_OK) {
		
		err = fCurrentData->PutText(L"'");
		
	}
	
	return err;
}

VError ExportJob::_DumpBytes (sLONG numberBytes)
{
	sLONG i;
	
	for (i = 0; i < numberBytes; i++) {
	
		fTextDumpBuffer[2 * i] = kHexTable[(fBinaryDumpBuffer[i] >> 4) & 0xf];
		fTextDumpBuffer[2 * i + 1] = kHexTable[fBinaryDumpBuffer[i] & 0xf];
		
	}
	fTextDumpBuffer[2 * i] = L'\0';
	
	return fCurrentData->PutText(VString(fTextDumpBuffer, -1));
}

#if 0
VError ExportJob::WriteOneRecord(FicheOnDisk* ficD, BaseTaskInfo* context)
{
	VError err = VE_OK;
	if (fProgress != nil)
		fProgress->Increment();

	if (MustStopTask(fProgress))
		err = ThrowBaseError(VE_DB4D_ACTIONCANCELEDBYUSER);

	if (err == VE_OK)
	{
		if (fCurrentData->GetSize() > fMaxFileSize && fMaxFileSize != 0)
		{
			err = CloseDataStream();
			if (err == VE_OK)
				err = CreateDataStream();
		}
	}
	if (err == VE_OK)
	{
		if (fCurrentLine != 0)
		{
			if (fCurrentLine >= 1000)
			{
				err = fCurrentData->PutText(L";\n\n");
				err = PutSqlHeader();
				fCurrentLine = 0;
			}
			else
				err = fCurrentData->PutText(L",\n");
		}
		fCurrentLine++;
		sLONG nb = fTable->GetNbCrit();
		bool first = true;
		err = fCurrentData->PutText(L"(");
		for (sLONG i = 1; i <= nb && err == VE_OK; i++)
		{
			Field* crit = fTable->RetainField(i);
			if (crit != nil)
			{
				if (first)
					first = false;
				else
					err = fCurrentData->PutText(L" , ");
				if (err == VE_OK)
				{
					sLONG TypeInRec, TypeInTable = crit->GetTyp();
					void* p = ficD->GetDataPtr(i, &TypeInRec);
					if (p == nil)
					{
						err = fCurrentData->PutText(L"NULL");
					}
					else
					{
						CreVValue_Code Code;
						if (TypeInRec == VK_BLOB || TypeInRec == VK_IMAGE || TypeInRec == VK_TEXT || TypeInRec == VK_BLOB_DB4D)
						{
							sLONG numblob = *((sLONG*)p);
							if (numblob == -1)
							{
								err = fCurrentData->PutText(L"NULL");
							}
							else
							{
								switch(TypeInRec)
								{
									case VK_BLOB:
									case VK_BLOB_DB4D:
										if (TypeInTable == VK_BLOB || TypeInTable == VK_BLOB_DB4D)
										{
											BlobWithPtr* blob = (BlobWithPtr*)fTable->GetDF()->LoadBlob(numblob, &CreBlobWithPtr, false, err, context);
											if (blob != nil)
											{
												VFolder* folder = nil;
												sLONG notused = GetNextBlob(folder, err);
												if (err == VE_OK)
												{
													VString blobname = fBlobName+ToString(numblob)+fBlobExt;
													VFile* blobfile = new VFile(*folder, blobname);
													err = blobfile->Create();
													if (err == VE_OK)
													{
														VFileStream blobdata(blobfile);
														err = blobdata.OpenWriting();
														if (err == VE_OK)
														{
															err = blob->WriteToStream(&blobdata);
															if (err == VE_OK)
																err = blobdata.CloseWriting();
															else
																blobdata.CloseWriting();
														}
														if (err == VE_OK)
														{
															VString filename;
															blobfile->GetRelativeURL(fBaseFolder, filename, false);
															err = fCurrentData->PutText(L"INFILE '"+filename+L"'");
														}
													}
												}
												blob->Release();
											}
											else
											{
												if (err == VE_OK)
													err = fCurrentData->PutText(L"NULL");
											}
										}
										else
											err = fCurrentData->PutText(L"NULL");
										break;

									case VK_TEXT:
										if (TypeInTable == VK_BLOB || TypeInTable == VK_BLOB_DB4D || TypeInTable == VK_TEXT )
										{
											BlobText* blob = (BlobText*)fTable->GetDF()->LoadBlob(numblob, &CreBlobText, false, err, context);
											if (blob != nil)
											{
												VFolder* folder = nil;
												sLONG notused = GetNextBlob(folder, err);
												if (err == VE_OK)
												{
													VString blobname = fTextName+ToString(numblob)+fTextExt;
													VFile* blobfile = new VFile(*folder, blobname);
													err = blobfile->Create();
													if (err == VE_OK)
													{
														VFileStream blobdata(blobfile);
														err = blobdata.OpenWriting();
														if (err == VE_OK)
														{
															blobdata.SetCharSet(VTC_UTF_8);
															blobdata.SetCarriageReturnMode(eCRM_NATIVE);
															err = blobdata.WriteBOM();
															VBlobText letext(&numblob, blob);
															if (err == VE_OK)
																err = blobdata.PutText(letext);
															if (err == VE_OK)
																err = blobdata.CloseWriting();
															else
																blobdata.CloseWriting();
														}
														if (err == VE_OK)
														{
															VString filename;
															blobfile->GetRelativeURL(fBaseFolder, filename, false);
															err = fCurrentData->PutText(L"INFILE '"+filename+L"'");
														}
													}
												}
												blob->Release();
											}
											else
											{
												if (err == VE_OK)
													err = fCurrentData->PutText(L"NULL");
											}
										}
										else
											err = fCurrentData->PutText(L"NULL");
										break;

									case VK_IMAGE:
										if (TypeInTable == VK_BLOB || TypeInTable == VK_BLOB_DB4D || TypeInTable == VK_IMAGE )
										{
											BlobWithPtr* blob = (BlobWithPtr*)fTable->GetDF()->LoadBlob(numblob, &CreBlobWithPtr, false, err, context);
											if (blob != nil)
											{
												VFolder* folder = nil;
												sLONG notused = GetNextBlob(folder, err);
												if (err == VE_OK)
												{
													sLONG xnumblob = numblob;
													VBlob4DWithPtr vblob(&xnumblob, blob);
													VString extension;
													err = FullExportPicture_GetOutputFileKind(vblob, extension);
													if (err == VE_OK)
													{
														VString blobname = fPictName+ToString(numblob)+L"."+extension;
														VFile* blobfile = new VFile(*folder, blobname);
														err = blobfile->Create();
														if (err == VE_OK)
														{
															VFileStream blobdata(blobfile);
															err = blobdata.OpenWriting();
															if (err == VE_OK)
															{
																err = FullExportPicture_Export(vblob, blobdata);
																if (err == VE_OK)
																	err = blobdata.CloseWriting();
																else
																	blobdata.CloseWriting();
															}
														}
														if (err == VE_OK)
														{
															VString filename;
															blobfile->GetRelativeURL(fBaseFolder, filename, false);
															err = fCurrentData->PutText(L"INFILE '"+filename+L"'");
														}
															
													}

													/*
													VString blobname = fPictName+ToString(numblob)+fPictExt;
													VFile* blobfile = new VFile(*folder, blobname);
													err = blobfile->Create();
													if (err == VE_OK)
													{
														VFileStream blobdata(blobfile);
														err = blobdata.OpenWriting();
														if (err == VE_OK)
														{
															err = blob->WriteToStream(&blobdata);
															if (err == VE_OK)
																err = blobdata.CloseWriting();
															else
																blobdata.CloseWriting();
														}
														if (err == VE_OK)
														{
															VString filename;
															blobfile->GetRelativeURL(fBaseFolder, filename, false);
															err = fCurrentData->PutText(L"INFILE '"+filename+L"'");
														}
													}
													*/
												}
												blob->Release();
											}
											else
											{
												if (err == VE_OK)
													err = fCurrentData->PutText(L"NULL");
											}
										}
										else
											err = fCurrentData->PutText(L"NULL");
										break;
								}
							}
						}
						else
						{
							VValueSingle* cv = nil;
							if (TypeInRec == TypeInTable)
							{
								Code = FindCV(TypeInRec);
								if (Code != nil)
									cv = (*Code)(nil ,0 ,p, false, true, nil, creVValue_default);
							}
							else
							{
								if (TypeInTable == VK_BLOB || TypeInTable == VK_BLOB_DB4D || TypeInTable == VK_IMAGE)
									cv = nil;
								else
								{
									if (TypeInTable == VK_TEXT)
										cv = new VString;
									else
									{
										Code = FindCV(TypeInTable);
										if (Code == nil)
											cv = nil;
										else
											cv = (*Code)(nil ,0 ,nil, false, true, nil, creVValue_default);
									}
									if (cv != nil)
									{
										VValueSingle* cv2 = nil;
										Code = FindCV(TypeInRec);
										if (Code != nil)
											cv2 = (*Code)(nil ,0 ,p, false, true, nil, creVValue_default);
										if (cv2 != nil)
										{
											cv->FromValue(*cv2);
											delete cv2;
										}
									}
								}
							}

							if (cv == nil)
								err = fCurrentData->PutText(L"NULL");
							else
							{
								VString s;
								switch (TypeInTable)
								{
									case VK_STRING:
									case VK_TEXT:
									case VK_STRING_UTF8:
									case VK_TEXT_UTF8:
										{
											VString s2;
											cv->GetString(s);
											sLONG len = s.GetLength();
											UniChar* dest = s2.GetCPointerForWrite(len*2+4);
											const UniChar* source = s.GetCPointer();
											sLONG len2 = 1;
											*dest = 39;
											dest++;
											for (sLONG i = 0; i < len; i++)
											{
												UniChar c = *source;
												source++;
												*dest = c;
												dest++;
												len2++;
												if (c == 39)
												{
													*dest = c;
													dest++;
													len2++;
												}
											}
											*dest = 39;
											len2++;
											s2.Validate(len2);
											err = fCurrentData->PutText(s2);
										}
										break;

									case VK_TIME:
										{
											sWORD year, month, day, hour, minute, second, millisecond;
											char buffer[255];
											((VTime*)cv)->GetUTCTime(year, month, day, hour, minute, second, millisecond);
		#if COMPIL_VISUAL
											sprintf( buffer, "'%04d/%02d/%02d %02d:%02d:%02d:%02d'", year, month, day, hour, minute, second, millisecond);
		#else
											snprintf( buffer, sizeof( buffer), "'%04d/%02d/%02d %02d:%02d:%02d:%02d'", year, month, day, hour, minute, second, millisecond);
		#endif
											s.FromCString( buffer);
											err = fCurrentData->PutText(s);
										}
										break;

									case VK_BOOLEAN:
										if (cv->GetBoolean())
											s = L"1";
										else
											s = L"0";
										err = fCurrentData->PutText(s);
										break;

									case VK_REAL:
										cv->GetXMLString(s, XSO_Default);
										err = fCurrentData->PutText(s);
										break;

									case VK_UUID:
									case VK_DURATION:
										err = fCurrentData->PutText(L"'");
										if (err == VE_OK)
										{
											cv->GetString(s);
											err = fCurrentData->PutText(s);
											if (err == VE_OK)
												err = fCurrentData->PutText(L"'");
										}
										break;
										
									default:
										cv->GetString(s);
										err = fCurrentData->PutText(s);
										break;
								}
								delete cv;
							}

						}
					}
				}
				crit->Release();
			}
		}
		if (err == VE_OK)
		{
			err = fCurrentData->PutText(L")");
		}
	}

	return err;
}

#endif
