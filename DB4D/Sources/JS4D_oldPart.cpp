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


DB4DSelectionIterator::DB4DSelectionIterator(CDB4DSelection* inSel, bool inReadOnly, bool inAutoSave, CDB4DBaseContext* inContext)
{
	fReadOnly = inReadOnly;
	fAutoSave = inAutoSave;
	fSel = RetainRefCountable(inSel);
	fCurRec = nil;
	fCurPos = -1;
	fSelSize = fSel->CountRecordsInSelection(inContext);
	fTable = fSel->GetBaseRef()->RetainNthTable(fSel->GetTableRef());
}


DB4DSelectionIterator::~DB4DSelectionIterator()
{
	ReleaseCurCurec();
	QuickReleaseRefCountable(fSel);
	QuickReleaseRefCountable(fTable);
}


CDB4DRecord* DB4DSelectionIterator::GetCurRec(CDB4DBaseContext* inContext)
{
	if (fCurPos == -2)
	{
		ReleaseCurCurec();
		fCurPos = 0;
		if (fCurPos >= fSelSize)
			fCurPos = -1;
		if (fCurPos != -1)
			fCurRec = fSel->LoadSelectedRecord(fCurPos+1, fReadOnly ? DB4D_Do_Not_Lock : DB4D_Keep_Lock_With_Record, inContext, false);		
	}
		
	return fCurRec;
}


VError DB4DSelectionIterator::ReLoadCurRec(CDB4DBaseContext* inContext, bool readonly)
{
	VError err = VE_OK;
	
	ReleaseCurCurec();
	if (fCurPos == -2)
	{
		fCurPos = 0;
		if (fCurPos >= fSelSize)
			fCurPos = -1;
	}
	if (fCurPos != -1)
		fCurRec = fSel->LoadSelectedRecord(fCurPos+1, readonly ? DB4D_Do_Not_Lock : DB4D_Keep_Lock_With_Record, inContext, false);		
	
	return err ;	
}


sLONG DB4DSelectionIterator::GetCurRecID()
{
	if (fCurPos == -2)
	{
		if (fSelSize == 0)
			return -1;
		else
			return fSel->GetSelectedRecordID(1, nil);
	}
	else
	{
		if (fCurPos == -1)
			return -1;
		else
			return fSel->GetSelectedRecordID(fCurPos+1, nil);
	}
}


void DB4DSelectionIterator::ReleaseCurCurec()
{
	if (fCurRec != nil)
	{
		if (!fReadOnly && fAutoSave)
		{
			if (fCurRec->IsModified())
				fCurRec->Save();
		}
		fCurRec->Release();
		fCurRec = nil;
	}
}


void DB4DSelectionIterator::First(CDB4DBaseContext* inContext)
{
	ReleaseCurCurec();
	/*
	if (fSelSize > 0)
	{
		fCurPos = 0;
		fCurRec = fSel->LoadSelectedRecord(fCurPos+1, fReadOnly ? DB4D_Do_Not_Lock : DB4D_Keep_Lock_With_Record, inContext, false);
	}
	else
	 */
		fCurPos = -2;
	
}

void DB4DSelectionIterator::Next(CDB4DBaseContext* inContext)
{
	ReleaseCurCurec();
	if (fCurPos == -2)
		fCurPos = 0;
	else
	{
		if (fCurPos != -1)
			fCurPos++;
	}
	{
		if (fCurPos >= fSelSize || fCurPos == -1)
		{
			fCurPos = -1;
		}
		else
		{
			fCurRec = fSel->LoadSelectedRecord(fCurPos+1, fReadOnly ? DB4D_Do_Not_Lock : DB4D_Keep_Lock_With_Record, inContext, false);			
		}
	}
}





//======================================================




void VJSTable::_AllRecords(VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	VError err = VE_OK;
	CDB4DSelection* sel = inTable->SelectAllRecords(GetRealContext(ioParms, inTable), &err);
	if (sel != nil)
	{
		ioParms.ReturnValue(VJSSelection::CreateInstance(ioParms.GetContextRef(), sel));
		sel->Release();
	}
}


void VJSTable::_Query(VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	VString querystring;
	VError err = VE_OK;
	if (ioParms.GetStringParam(1, querystring))
	{
		bool withlock = false;
		ioParms.GetBoolParam(2, &withlock);
		CDB4DQuery* query = inTable->NewQuery();
		if (query != nil)
		{
			VString orderby;
			err = query->BuildFromString(querystring, orderby);
			if (err == VE_OK)
			{
				vector<VString> ParamNames;
				vector<VValueSingle*> ParamValues;

				err = query->GetParams(ParamNames, ParamValues);
				if (err == VE_OK)
				{
					vector<VValueSingle*>::iterator curvalue = ParamValues.begin();
					for (vector<VString>::iterator curname = ParamNames.begin(), endname = ParamNames.end(); curname != endname; curname++, curvalue++)
					{
						ioParms.EvaluateScript(*curname, &*curvalue);
					}
					err = query->SetParams(ParamNames, ParamValues);
					if (err == VE_OK)
					{	 
						CDB4DSelection* sel = inTable->ExecuteQuery(query, GetRealContext(ioParms, inTable), nil, nil, withlock ? DB4D_Keep_Lock_With_Transaction : DB4D_Do_Not_Lock, 0, nil, &err);
						if (sel != nil)
						{
							sLONG nbparam = (sLONG) ioParms.CountParams();
							CDB4DSortingCriterias* sortcrit = inTable->NewSortingCriterias();
							CDB4DField* curfield = nil;
							bool curascent = true;
							for (sLONG i = 3; i <= nbparam; i++)
							{
								CDB4DField* newfield = ioParms.GetParamObjectPrivateData<VJSField>(i);
								if (newfield != nil)
								{
									newfield->Retain();
									if (curfield != nil)
									{
										sortcrit->AddCriteria(curfield, curascent);
										curascent = true;
										curfield->Release();
									}
									curfield = newfield;
								}
								else
								{
									VString ascentstring;
									if (ioParms.GetStringParam(i, ascentstring))
									{
										if (ascentstring == L">")
											curascent = false;
									}
								}
							}
							if (curfield != nil)
							{
								sortcrit->AddCriteria(curfield, curascent);
								curfield->Release();
							}
							if (sel->SortSelection(sortcrit, nil, GetRealContext(ioParms, inTable)))
							{
								ioParms.ReturnValue(VJSSelection::CreateInstance(ioParms.GetContextRef(), sel));
							}
							sel->Release();
						}
					}
				}
			}
			query->Release();
		}
	}
}


void VJSTable::_Find(VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	VString querystring;
	VError err = VE_OK;
	if (ioParms.GetStringParam(1, querystring))
	{
		bool withlock = false;
		ioParms.GetBoolParam(2, &withlock);
		CDB4DQuery* query = inTable->NewQuery();
		if (query != nil)
		{
			VString orderby;
			err = query->BuildFromString(querystring, orderby);
			if (err == VE_OK)
			{
				CDB4DBaseContext* context = GetRealContext(ioParms, inTable);
				CDB4DSelection* sel = inTable->ExecuteQuery(query, context, nil, nil, withlock ? DB4D_Keep_Lock_With_Transaction : DB4D_Do_Not_Lock, 0, nil, &err);
				if (sel != nil)
				{
					DB4DSelectionIterator* itersel = new DB4DSelectionIterator(sel, false, true, context);
					itersel->First(context);

					ioParms.ReturnValue(VJSSelectionIterator::CreateInstance(ioParms.GetContextRef(), itersel));
					sel->Release();
				}
			}
			query->Release();
		}
	}
}



void VJSTable::_NewRecord(VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	CDB4DRecord* rec = inTable->NewRecord(GetRealContext(ioParms, inTable));
	if (rec != nil)
	{
		ioParms.ReturnValue(VJSRecord::CreateInstance(ioParms.GetContextRef(), rec));
		rec->Release();
	}
}


void VJSTable::_NewSelection(VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	bool keepsorted = ioParms.GetBoolParam( 1, L"KeepSorted", L"AnyOrder");
	CDB4DSelection* sel = inTable->NewSelection(keepsorted ? DB4D_Sel_SmallSel : DB4D_Sel_Bittab);
	if (sel != nil)
	{
		ioParms.ReturnValue(VJSSelection::CreateInstance(ioParms.GetContextRef(), sel));
		sel->Release();
	}
}


//======================================================



void VJSSelection::Initialize( const VJSParms_initialize& inParms, CDB4DSelection* inSelection)
{
	inSelection->Retain();
}


void VJSSelection::Finalize( const VJSParms_finalize& inParms, CDB4DSelection* inSelection)
{
	inSelection->Release();
}


void VJSSelection::GetProperty( VJSParms_getProperty& ioParms, CDB4DSelection* inSelection)
{
	sLONG num;
	if (ioParms.GetPropertyNameAsLong( &num) && (num >= 0) )
	{
		CDB4DRecord* rec = inSelection->LoadSelectedRecord(num+1, DB4D_Keep_Lock_With_Record, GetRealContext(ioParms, inSelection), false);
		if (rec != nil)
		{
			ioParms.ReturnValue(VJSRecord::CreateInstance(ioParms.GetContextRef(), rec));
			rec->Release();
		}

	}
}


void VJSSelection::_SetReadOnly(VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
}


void VJSSelection::_Count(VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	ioParms.ReturnNumber(inSelection->CountRecordsInSelection(GetRealContext(ioParms, inSelection)));
}


void VJSSelection::_First(VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	bool readonly = false;
	if (ioParms.CountParams() > 0 && ioParms.IsStringParam(1))
	{
		VString s;
		ioParms.GetStringParam(1, s);
		if (s == L"ReadOnly")
			readonly = true;
	}
	else
		ioParms.GetBoolParam(1, &readonly);
	CDB4DBaseContext* context = GetRealContext(ioParms, inSelection);
	DB4DSelectionIterator* seliter = new DB4DSelectionIterator(inSelection, readonly, true, context);
	seliter->First(context);
	ioParms.ReturnValue(VJSSelectionIterator::CreateInstance(ioParms.GetContextRef(), seliter));
}


void VJSSelection::_OrderBy(VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	sLONG nbparam = (sLONG) ioParms.CountParams();
	CDB4DTable* table = inSelection->GetBaseRef()->RetainNthTable(inSelection->GetTableRef());
	if (table != nil)
	{
		CDB4DSortingCriterias* sortcrit = table->NewSortingCriterias();
		CDB4DField* curfield = nil;
		bool curascent = true;
		for (sLONG i = 1; i <= nbparam; i++)
		{
			CDB4DField* newfield = ioParms.GetParamObjectPrivateData<VJSField>(i);
			if (newfield != nil)
			{
				newfield->Retain();
				if (curfield != nil)
				{
					sortcrit->AddCriteria(curfield, curascent);
					curascent = true;
					curfield->Release();
				}
				curfield = newfield;
			}
			else
			{
				VString ascentstring;
				if (ioParms.GetStringParam(i, ascentstring))
				{
					if (ascentstring == L">")
						curascent = false;
				}
			}
		}
		if (curfield != nil)
		{
			sortcrit->AddCriteria(curfield, curascent);
			curfield->Release();
		}
		if (!inSelection->SortSelection(sortcrit, nil, GetRealContext(ioParms, inSelection)))
		{
		}
		table->Release();
	}
	ioParms.ReturnThis();
}

void VJSSelection::_Add(VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	VError err = VE_OK;
	if (ioParms.CountParams() > 0)
	{
		bool atTheEnd = ioParms.GetBoolParam( 2, L"AtTheEnd", L"AnyWhere");
		CDB4DSelection* sel = ioParms.GetParamObjectPrivateData<VJSSelection>(1);
		if (sel != nil)
		{
			for (sLONG i = 0, nb = sel->CountRecordsInSelection(nil); i < nb && err == VE_OK; i++)
			{
				err = inSelection->AddRecordID(sel->GetSelectedRecordID(i+1, nil), atTheEnd);
			}
		}
		else
		{
			CDB4DRecord* rec = ioParms.GetParamObjectPrivateData<VJSRecord>(1);
			if (rec != nil)
			{
				err = inSelection->AddRecord(rec, atTheEnd);
			}
			else
			{
				DB4DSelectionIterator* seliter = ioParms.GetParamObjectPrivateData<VJSSelectionIterator>(1);
				if (seliter != nil)
				{
					sLONG recid = seliter->GetCurRecID();
					if (recid >= 0)
						err = inSelection->AddRecordID(recid, atTheEnd);
				}
				else
				{
					if (ioParms.IsNumberParam(1))
					{
						sLONG recid;
						ioParms.GetLongParam(1, &recid);
						if (recid >= 0)
							err = inSelection->AddRecordID(recid, atTheEnd);
					}
				}
			}
		}
	}
	ioParms.ReturnThis();
}


void VJSSelection::_ExportAsSQL(VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	VError err = VE_OK;
	VFolder* folder = ioParms.RetainFolderParam( 1);
	if (folder != nil)
	{
		sLONG nbBlobsPerLevel = 200;
		Real maxSQLFileSize = 0;
		ioParms.GetLongParam(2, &nbBlobsPerLevel);
		ioParms.GetRealParam(3, &maxSQLFileSize);
		if (nbBlobsPerLevel <= 0)
			nbBlobsPerLevel = 200;
		err = inSelection->ExportToSQL(GetBaseContext( ioParms), folder, nil, nbBlobsPerLevel, (sLONG8)maxSQLFileSize);
		folder->Release();
	}
}	


void VJSSelection::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "first", js_callStaticFunction<_First>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setReadOnly", js_callStaticFunction<_SetReadOnly>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "count", js_callStaticFunction<_Count>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "length", js_callStaticFunction<_Count>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "orderBy", js_callStaticFunction<_OrderBy>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "add", js_callStaticFunction<_Add>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "exportAsSQL", js_callStaticFunction<_ExportAsSQL>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0}
	};

	outDefinition.className = "Selection";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.getProperty = js_getProperty<GetProperty>;
	outDefinition.staticFunctions = functions;
}







//======================================================




void VJSRecord::Initialize( const VJSParms_initialize& inParms, CDB4DRecord* inRecord)
{
	inRecord->Retain();
}


void VJSRecord::Finalize( const VJSParms_finalize& inParms, CDB4DRecord* inRecord)
{
	inRecord->Release();
}



void VJSRecord::GetPropertyNames( VJSParms_getPropertyNames& ioParms, CDB4DRecord* inRecord)
{
	CDB4DTable* inTable = inRecord->RetainTable();
	sLONG nb = inTable->CountFields();
	for (sLONG i = 1; i <= nb; i++)
	{
		VString fieldname;
		CDB4DField* ff = inTable->RetainNthField(i);
		if (ff != nil)
		{
			ff->GetName(fieldname);
			ioParms.AddPropertyName(fieldname);
			ff->Release();
		}
	}
	inTable->Release();
}


void VJSRecord::GetProperty( VJSParms_getProperty& ioParms, CDB4DRecord* inRecord)
{
	VString propname;
	ioParms.GetPropertyName(propname);
	CDB4DTable* table = inRecord->RetainTable();
	sLONG numfield = table->GetFieldByName(propname);
	if (numfield > 0)
	{
		VError err = VE_OK;
		VValueSingle* cv = inRecord->GetFieldValue(numfield, false, &err);
		if (err == VE_OK)
		{
			if (cv != nil)
			{
				ioParms.ReturnVValue(*cv);
			}
		}
	}
	else
	{
		CDB4DRelation* rel = table->GetOwner(nil)->FindAndRetainRelationByName(propname, nil);
		if (rel != nil)
		{
			CDB4DField* source = nil;
			rel->RetainSource(source, nil);
			if (source != nil)
			{
				if (source->GetOwner()->GetID() == table->GetID())
				{
					CDB4DRecord* result = nil;
					rel->ActivateManyToOne(inRecord, result, GetRealContext(ioParms, table), false, false, false);
					if (result != nil)
					{
						ioParms.ReturnValue(VJSRecord::CreateInstance(ioParms.GetContextRef(), result));
						result->Release();
					}
				}
				else
				{
					CDB4DField* dest = nil;
					rel->RetainDestination(dest, nil);
					if (dest != nil)
					{
						if (dest->GetOwner()->GetID() == table->GetID())
						{
							CDB4DSelection* result = nil;
							rel->ActivateOneToMany(inRecord, result, GetRealContext(ioParms, table), false, false, false);
							if (result != nil)
							{
								ioParms.ReturnValue(VJSSelection::CreateInstance(ioParms.GetContextRef(), result));
								result->Release();								
							}
						}
						dest->Release();
					}
				}
				source->Release();
			}
			rel->Release();
		}
	}
	table->Release();
}


bool VJSRecord::SetProperty( VJSParms_setProperty& ioParms, CDB4DRecord* inRecord)
{
	VString propname;
	ioParms.GetPropertyName(propname);
	CDB4DTable* table = inRecord->RetainTable();
	sLONG numfield = table->GetFieldByName(propname);
	if (numfield > 0)
	{
		VError err = VE_OK;
		VValueSingle* cv = inRecord->GetFieldValue(numfield, false, &err);
		if (err == VE_OK)
		{
			if (cv != nil)
			{
				VValueSingle* cv2 = ioParms.CreatePropertyVValue();
				if (cv2 == nil)
				{
					cv->Clear();
					cv->SetNull(true);
				}
				else
				{
					cv->FromValue(*cv2);
					delete cv2;
				}
				inRecord->Touch(numfield);
			}
		}
	}
	table->Release();
	return true;
}


void VJSRecord::_Save(VJSParms_callStaticFunction& ioParms, CDB4DRecord* inRecord)
{
	VError err = VE_OK;
	inRecord->Save(&err);
}


void VJSRecord::_GetID(VJSParms_callStaticFunction& ioParms, CDB4DRecord* inRecord)
{
	ioParms.ReturnNumber(inRecord->GetID());
}


void VJSRecord::_IsModified(VJSParms_callStaticFunction& ioParms, CDB4DRecord* inRecord)
{
	ioParms.ReturnBool(inRecord->IsModified());
}


void VJSRecord::_IsNew(VJSParms_callStaticFunction& ioParms, CDB4DRecord* inRecord)
{
	ioParms.ReturnBool(inRecord->IsNew());
}


void VJSRecord::_Drop(VJSParms_callStaticFunction& ioParms, CDB4DRecord* inRecord)
{
	VError err = VE_OK;
	inRecord->Drop(&err);
}


void VJSRecord::_getTimeStamp(VJSParms_callStaticFunction& ioParms, CDB4DRecord* inRecord)
{
	VTime stamp;
	inRecord->GetTimeStamp(stamp);
	ioParms.ReturnTime(stamp);
}




void VJSRecord::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "save", js_callStaticFunction<_Save>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getID", js_callStaticFunction<_GetID>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isModified", js_callStaticFunction<_IsModified>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isNew", js_callStaticFunction<_IsNew>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "drop", js_callStaticFunction<_Drop>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "delete", js_callStaticFunction<_Drop>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getTimeStamp", js_callStaticFunction<_getTimeStamp>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0}
	};

	outDefinition.className = "Record";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.getProperty = js_getProperty<GetProperty>;
	outDefinition.setProperty = js_setProperty<SetProperty>;
	outDefinition.getPropertyNames = js_getPropertyNames<GetPropertyNames>;
	outDefinition.staticFunctions = functions;
}



//======================================================




void VJSSelectionIterator::Initialize( const VJSParms_initialize& inParms, DB4DSelectionIterator* inSelIter)
{
	// rien a faire pour l'instant
}


void VJSSelectionIterator::Finalize( const VJSParms_finalize& inParms, DB4DSelectionIterator* inSelIter)
{
	delete inSelIter;
}



void VJSSelectionIterator::GetPropertyNames( VJSParms_getPropertyNames& ioParms, DB4DSelectionIterator* inSelIter)
{
	CDB4DTable* inTable = inSelIter->GetTable();
	sLONG nb = inTable->CountFields();
	for (sLONG i = 1; i <= nb; i++)
	{
		VString fieldname;
		CDB4DField* ff = inTable->RetainNthField(i);
		if (ff != nil)
		{
			ff->GetName(fieldname);
			ioParms.AddPropertyName(fieldname);
			ff->Release();
		}
	}
}


void VJSSelectionIterator::GetProperty( VJSParms_getProperty& ioParms, DB4DSelectionIterator* inSelIter)
{
	VString propname;
	ioParms.GetPropertyName(propname);
	CDB4DTable* inTable = inSelIter->GetTable();
	sLONG numfield = inTable->GetFieldByName(propname);
	if (numfield > 0)
	{
		CDB4DRecord* inRecord = inSelIter->GetCurRec(GetRealContext(ioParms, inTable));
		if (inRecord != nil)
		{
			VError err = VE_OK;
			VValueSingle* cv = inRecord->GetFieldValue(numfield, false, &err);
			if (err == VE_OK)
			{
				if (cv != nil)
				{
					ioParms.ReturnVValue(*cv);
				}
			}
		}
	}
	else
	{
		CDB4DRelation* rel = inTable->GetOwner(nil)->FindAndRetainRelationByName(propname, nil);
		if (rel != nil)
		{
			CDB4DField* source = nil;
			rel->RetainSource(source, nil);
			if (source != nil)
			{
				if (source->GetOwner()->GetID() == inTable->GetID())
				{
					CDB4DBaseContext* context = GetRealContext(ioParms, inTable);
					CDB4DRecord* inRecord = inSelIter->GetCurRec(context);
					if (inRecord != nil)
					{
						CDB4DRecord* result = nil;
						rel->ActivateManyToOne(inRecord, result, context, false, false, false);
						if (result != nil)
						{
							ioParms.ReturnValue(VJSRecord::CreateInstance(ioParms.GetContextRef(), result));
							result->Release();
						}
					}
				}
				else
				{
					CDB4DField* dest = nil;
					rel->RetainDestination(dest, nil);
					if (dest != nil)
					{
						if (dest->GetOwner()->GetID() == inTable->GetID())
						{
							CDB4DBaseContext* context = GetRealContext(ioParms, inTable);
							CDB4DRecord* inRecord = inSelIter->GetCurRec(context);
							if (inRecord != nil)
							{
								CDB4DSelection* result = nil;
								rel->ActivateOneToMany(inRecord, result, context, false, false, false);
								if (result != nil)
								{
									ioParms.ReturnValue(VJSSelection::CreateInstance(ioParms.GetContextRef(), result));
									result->Release();								
								}
							}
						}
						dest->Release();
					}
				}
				source->Release();
			}
			rel->Release();
		}
	}
}


bool VJSSelectionIterator::SetProperty( VJSParms_setProperty& ioParms, DB4DSelectionIterator* inSelIter)
{
	VString propname;
	ioParms.GetPropertyName(propname);
	CDB4DTable* inTable = inSelIter->GetTable();
	CDB4DRecord* inRecord = inSelIter->GetCurRec(GetRealContext(ioParms, inTable));
	sLONG numfield = inTable->GetFieldByName(propname);
	if (numfield > 0)
	{
		VError err = VE_OK;
		VValueSingle* cv = inRecord->GetFieldValue(numfield, false, &err);
		if (err == VE_OK)
		{
			if (cv != nil)
			{
				VValueSingle* cv2 = ioParms.CreatePropertyVValue();
				if (cv2 == nil)
				{
					cv->Clear();
					cv->SetNull(true);
				}
				else
				{
					cv->FromValue(*cv2);
					delete cv2;
				}
				inRecord->Touch(numfield);
			}
		}
	}
	return true;
}


void VJSSelectionIterator::_Save(VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter)
{
	VError err = VE_OK;
	CDB4DRecord* inRecord = inSelIter->GetCurRec(GetRealContext(ioParms, inSelIter->GetTable()));
	inRecord->Save(&err);
}


void VJSSelectionIterator::_Valid(VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter)
{
	ioParms.ReturnBool(inSelIter->GetCurPos() != -1);
}


void VJSSelectionIterator::_Next(VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter)
{
	inSelIter->Next(GetRealContext(ioParms, inSelIter->GetTable()));
	ioParms.ReturnBool(inSelIter->GetCurPos() != -1);
}


void VJSSelectionIterator::_Loaded(VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter)
{
	ioParms.ReturnBool(inSelIter->GetCurRec(GetRealContext(ioParms, inSelIter->GetTable())) != nil);
}


void VJSSelectionIterator::_GetID(VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter)
{
	CDB4DRecord* inRecord = inSelIter->GetCurRec(GetRealContext(ioParms, inSelIter->GetTable()));
	if (inRecord == nil)
		ioParms.ReturnNumber(-1);
	else
		ioParms.ReturnNumber(inRecord->GetID());
}


void VJSSelectionIterator::_IsModified(VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter)
{
	CDB4DRecord* inRecord = inSelIter->GetCurRec(GetRealContext(ioParms, inSelIter->GetTable()));
	if (inRecord == nil)
		ioParms.ReturnBool(false);
	else
		ioParms.ReturnBool(inRecord->IsModified());
}


void VJSSelectionIterator::_IsNew(VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter)
{
	CDB4DRecord* inRecord = inSelIter->GetCurRec(GetRealContext(ioParms, inSelIter->GetTable()));
	if (inRecord == nil)
		ioParms.ReturnBool(false);
	else
		ioParms.ReturnBool(inRecord->IsNew());
}


void VJSSelectionIterator::_Drop(VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter)
{
	VError err = VE_OK;
	CDB4DRecord* inRecord = inSelIter->GetCurRec(GetRealContext(ioParms, inSelIter->GetTable()));
	if (inRecord != nil)
		inRecord->Drop(&err);
	inSelIter->ReleaseCurCurec();
}


void VJSSelectionIterator::_Release(VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter)
{
	inSelIter->ReleaseCurCurec();
}


void VJSSelectionIterator::_Reload(VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter)
{
	VError err = VE_OK;
	bool readonly = ioParms.GetBoolParam( 1, L"ReadOnly", L"ReadWrite");
	err = inSelIter->ReLoadCurRec(GetRealContext(ioParms, inSelIter->GetTable()), readonly);
}


void VJSSelectionIterator::_getTimeStamp(VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter)
{
	VError err = VE_OK;
	CDB4DRecord* inRecord = inSelIter->GetCurRec(GetRealContext(ioParms, inSelIter->GetTable()));
	if (inRecord != nil)
	{
		VTime stamp;
		inRecord->GetTimeStamp(stamp);
		ioParms.ReturnTime(stamp);
	}
}


void VJSSelectionIterator::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "next", js_callStaticFunction<_Next>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "valid", js_callStaticFunction<_Valid>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "loaded", js_callStaticFunction<_Loaded>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "save", js_callStaticFunction<_Save>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getID", js_callStaticFunction<_GetID>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "reload", js_callStaticFunction<_Reload>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isModified", js_callStaticFunction<_IsModified>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isNew", js_callStaticFunction<_IsNew>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "drop", js_callStaticFunction<_Drop>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "delete", js_callStaticFunction<_Drop>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "release", js_callStaticFunction<_Release>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getTimeStamp", js_callStaticFunction<_getTimeStamp>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0}
	};

	outDefinition.className = "SelectionIterator";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.getProperty = js_getProperty<GetProperty>;
	outDefinition.setProperty = js_setProperty<SetProperty>;
	outDefinition.getPropertyNames = js_getPropertyNames<GetPropertyNames>;
	outDefinition.staticFunctions = functions;
}




//======================================================
