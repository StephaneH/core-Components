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



VError WafSelection::buildFromJS(VJSValue& val)
{
	VError err = VE_OK;

	if (!val.IsUndefined() && val.IsObject())
	{
		VJSObject oval(val.GetObject());
		VString mode;
		oval.GetPropertyAsString("_mode", nil, mode);
		if (mode.IsEmpty())
			mode = "single";
		fMode = mode;

		VJSValue vrows(oval.GetProperty("_rows"));
		VJSValue vranges(oval.GetProperty("_ranges"));
		VJSValue vbutrows(oval.GetProperty("_butRows"));
		if (!vrows.IsUndefined() && vrows.IsArray())
		{
			VJSArray arows(vrows.GetObject());
			sLONG nbelem = arows.GetLength();
			rows.reserve(nbelem);
			for (sLONG i = 0; i < nbelem && err == VE_OK; i++)
			{
				VJSValue elem(arows.GetValueAt(i));
				if (elem.IsNumber())
				{
					sLONG n;
					elem.GetLong(&n);
					rows.push_back(n);
				}
			}
		}

		if (!vranges.IsUndefined() && vranges.IsArray())
		{
			VJSArray aranges(vranges.GetObject());
			sLONG nbelem = aranges.GetLength();
			ranges.reserve(nbelem);
			for (sLONG i = 0; i < nbelem && err == VE_OK; i++)
			{
				VJSValue elem(aranges.GetValueAt(i));
				if (elem.IsObject())
				{
					VJSObject oelem(elem.GetObject());
					bool existstart = false, existend = false;
					sLONG start = oelem.GetPropertyAsLong("start", nil, &existstart);
					sLONG end = oelem.GetPropertyAsLong("end", nil, &existend);
					if (existend && existstart)
					{
						WafSelectionRange range;
						range.start = start;
						range.end = end;
						ranges.push_back(range);
					}
				}
			}
		}

		if (!vbutrows.IsUndefined() && vbutrows.IsArray())
		{
			VJSArray abutrows(vbutrows.GetObject());
			sLONG nbelem = abutrows.GetLength();
			butRows.reserve(nbelem);
			for (sLONG i = 0; i < nbelem && err == VE_OK; i++)
			{
				VJSValue elem(abutrows.GetValueAt(i));
				if (elem.IsNumber())
				{
					sLONG n;
					elem.GetLong(&n);
					butRows.push_back(n);
				}
			}
		}

	}
	return err;
}


sLONG WafSelection::count() const
{
	sLONG result = (sLONG)rows.size();
	for (WafSelectionRangeVector::const_iterator cur = ranges.begin(), end = ranges.end(); cur != end; cur++)
	{
		result += cur->end - cur->start + 1;
	}
	result -= (sLONG)butRows.size();
	return result;
}


VError WafSelection::AddElem()
{
	if (lastadded != -1 && lastadded == (curelem - 1))
	{
		VSize nbrows = rows.size();
		if (nbrows > 0 && rows[nbrows-1] == lastadded)
		{
			rows.pop_back();
			WafSelectionRange range;
			range.start = lastadded;
			range.end = curelem;
			ranges.push_back(range);
		}
		else
		{
			VSize nbranges = ranges.size();
			assert(nbranges > 0);
			ranges[nbranges-1].end = curelem;
		}
	}
	else
	{
		rows.push_back(curelem);
	}
	lastadded = curelem;
	++curelem;

	return VE_OK;
}


WafSelection& WafSelection::operator = (const WafSelection& other)
{
	curelem = other.curelem;
	lastadded = other.lastadded;
	rows = other.rows;
	ranges = other.ranges;
	butRows = other.butRows;
	fMode = other.fMode;
	return *this;
}




						//------------------------------------------------------------------------------------


VError RestRequestHandler::SetPattern( const VString& inPattern)
{
	VTaskLock lock( &fMutex);
	fPattern = inPattern;
	return VE_OK;
}


VError RestRequestHandler::GetPatterns( vector<VString>* outPatterns) const
{
	VTaskLock lock( &fMutex);
	outPatterns->push_back( fPattern);
	return VE_OK;
}



//-----------------------------------------------------------------------------
#if UseDB4DJSContext
// temporary code

VCriticalSection tempContextStoreMutex;
vector<tempContextStore> tempStores;

void releaseAllTempStores()
{
	VTaskLock lock(&tempContextStoreMutex);
	for (vector<tempContextStore>::iterator cur = tempStores.begin(), end = tempStores.end(); cur != end; cur++)
	{
		cur->xcontext->FreeAllJSFuncs();
		cur->xcontext->Release();

		QuickReleaseRefCountable(cur->xJSContext);
		delete cur->xJSDelegate;
		
	}
	tempStores.clear();
}


bool addTempContextStore(tempContextStore& tempStore)
{
	VTaskLock lock(&tempContextStoreMutex);
	if (tempStores.size() < kMaxContextStores)
	{
		tempStore.usedBy = VTask::GetCurrentID();
		tempStore.pos = tempStores.size();
		tempStores.push_back(tempStore);
		return true;
	}
	else
		return false;
}


bool getTempContextStore(tempContextStore& tempStore)
{
	VTaskLock lock(&tempContextStoreMutex);
	for (vector<tempContextStore>::iterator cur = tempStores.begin(), end = tempStores.end(); cur != end; cur++)
	{
		if (cur->usedBy == 0)
		{
			cur->usedBy = VTask::GetCurrentID();
			tempStore = *cur;
			return true;
		}
	}
	return false;
}


void releaseTempStore(tempContextStore& tempStore)
{
	VTaskLock lock(&tempContextStoreMutex);
	tempStores[tempStore.pos].usedBy = 0;
}



// end of temporary code
#endif
//-----------------------------------------------------------------------------


void RestTools::InitUAG()
{
	VString username, password;
	if (url.GetValue(L"$user", username) && url.GetValue(L"$password", password))
	{
		SetCurrentUser(username, password);
	}
	//else
	//req.SetCurrentUser(*inRequest.GetUserName(), *inRequest.GetPassword());
}


void RestTools::GetWAFFiles()
{
	toJSON = false;
	VString allfiles;
	if (url.GetValue(rest::path,allfiles))
	{
		IDB4D_ApplicationIntf *applicationIntf = VDBMgr::GetManager()->GetApplicationInterface();
		if (applicationIntf != nil)
		{
			VFolder* walib = applicationIntf->RetainWaLibFolder();
			if (walib != nil)
			{
				VFolder waf(*walib, L"WAF");
				VectorOfVString allpath;
				allfiles.GetSubStrings(',',allpath, false, true);
				err = VE_OK;
				for (VectorOfVString::iterator cur = allpath.begin(), end = allpath.end(); cur != end && err == VE_OK; cur++)
				{
					VString path = *cur;
					if (path[0] == '/')
						path.SubString(2, path.GetLength()-1);
					VFile FileToLoad(waf, path, FPS_POSIX);
					{
						VFileStream ffinput(&FileToLoad);
						err = ffinput.OpenReading();
						if (err == VE_OK)
						{
							ffinput.GuessCharSetFromLeadingBytes(VTC_UTF_8);
							VString text;
							err = ffinput.GetText(text);
							if (err == VE_OK)
							{
								fOutput->PutText(text);
								fOutput->PutText(L"\n");
							}
							ffinput.CloseReading();
						}
					}
				}
				walib->Release();
			}
		}
	}
	VString filekind;
	if (err == VE_OK && url.GetValue(rest::kind, filekind))
	{
		if (filekind == L"js")
		{
			fResponse->AddResponseHeader(L"Content-Type", L"application/javascript", true);
		}
		else if (filekind == L"css")
		{
			fResponse->AddResponseHeader(L"Content-Type", L"text/css", true);
		}
		else
		{
			fResponse->AddResponseHeader(L"Content-Type", L"text/text", true);
		}
	}

	if (err != VE_OK)
	{
		fResponse->AddResponseHeader(L"Content-Type", L"application/json; charset=UTF-8", true);
		SetHTTPError(rest::http_bad_request);
		fOutput->SetPos(0);
	}

}

void RestTools::GetServerAndBaseInfo()
{
	VValueBag bag;
	
	const VString* s = fURL->NextPart();

	if (s == nil || *s == "cacheInfo")
	{
		CDB4DManager* db4D = XBOX::VComponentManager::RetainComponentOfType<CDB4DManager>();
		if (db4D != nil)
		{
			VCppMemMgr* cachemgr = db4D->GetCacheMemoryManager();
			if (cachemgr != nil)
			{
				VSize totalmem, usedmem;
				cachemgr->GetMemUsageInfo(totalmem, usedmem);
				bag.SetLong8(L"cacheSize", totalmem);
				bag.SetLong8(L"usedCache", usedmem);
			}
			db4D->Release();
		}
	}

	if (s == nil || *s == "entitySet")
		VDBMgr::GetManager()->GetKeptDataSetsInfo(false, bag);

	if (s == nil || *s == "progressInfo")
	{
		const VString* s2 = fURL->NextPart();
		if (s2 != nil)
		{
			VString stopstr;
			if (fURL->GetValue(rest::stop, stopstr) && stopstr == "true")
			{
				VProgressIndicator* p = VProgressManager::RetainProgressIndicator(*s2);
				if (p != nil)
				{
					p->UserAbort();
					p->Release();
				}
			}
		}
		VProgressManager::GetProgressInfo(bag, s2);
	}

	VString resultString;
	if (fToXML)
	{
		SaveBagToXML(bag, L"info", resultString, IsPrettyFormatting(), nil, true);
	}
	else
	{
		bag.GetJSONString(resultString, IsPrettyFormatting() ? JSON_PrettyFormatting : JSON_Default);
	}
	PutText(resultString);
}


void RestTools::SetRequestFormat()
{
	VString ss;

	toJSON = true;

	s = url.NextPart();
	if (s != nil && *s == L"ado.net")
	{
		s = url.NextPart();
		SetPureJSon(false);
	}

	if (url.GetValue(rest::format, ss))
	{
		if (ss == L"xml")
		{
			SetToXML(true);
			toJSON = false;
		}
		else if (ss == L"json")
			SetPureJSon(true);
		else if (ss == L"ado.net")
			SetPureJSon(false);
		else
		{
			SetHTTPError(rest::http_bad_request);
			err = ThrowError(rest::unsupported_format, ss);
		}
	}

	if (url.GetValue(rest::imageinfo, ss))
	{
		SetWithImageInfo(ss == "true" || ss == "1");
	}

}


void RestTools::SetVerb()
{
	restmethod = RestMethodeRetrieve;
	if (wasapost || wasaput)
		restmethod = RestMethodeUpdate;
	if (wasadelete)
		restmethod = RestMethodeDelete;

	if (url.GetValue(rest::method,restmethodName) && err == VE_OK)
	{
		if (restmethodName == L"validate")
			restmethod = RestMethodeValidate;
		else if (restmethodName == L"delete")
			restmethod = RestMethodeDelete;
		else if (restmethodName == L"update")
			restmethod = RestMethodeUpdate;
		else if (restmethodName == L"refresh")
			restmethod = RestMethodeRefresh;
		else if (restmethodName == L"retrieve")
			restmethod = RestMethodeRetrieve;
		else if (restmethodName == L"entityset")
			restmethod = RestMethodeBuildDataSet;
		else if (restmethodName == L"release")
			restmethod = RestMethodeReleaseDataSet;
		else if (restmethodName == L"subentityset")
			restmethod = RestMethodeBuildSubDataSet;
		else if (restmethodName == L"flush")
			restmethod = RestMethodeFlush;
		else
		{
			err = ThrowError(rest::unknown_rest_method, restmethodName);
			SetHTTPError(rest::http_bad_request);
		}
	}

}


void RestTools::AnalyseGetCatalog()
{
	if (labase != nil)
	{
		if (restmethod == RestMethodeRefresh)
		{
			err = labase->ReLoadEntityModels();
			if (err != VE_OK)
				SetHTTPError(rest::http_internal_error);
		}
		if (err == VE_OK)
		{
			getDef = true;
			s = url.NextPart();
			if (s != nil)
			{
				if (*s == L"$all")
				{
					err = labase->RetainAllEntityModels(allEMs, nil, true, !fAllowAutoModel);
					getAllEntitiesDef = true;
					if (err != VE_OK)
					{
						SetHTTPError(rest::http_internal_error);
						getDef = false;
					}
				}
				else
				{
					VectorOfVString emNames;
					s->GetSubStrings(',', emNames, false, true);
					for (VectorOfVString::iterator cur = emNames.begin(), end = emNames.end(); cur != end; cur++)
					{
						VString sname = *cur;
						Table* tt = nil;
#if AllowDefaultEMBasedOnTables
						if (sname.GetUniChar(1) == kEntityTablePrefixChar)
						{
							sname.SubString(2, sname.GetLength()-1);
							tt = labase->FindAndRetainTableRef(sname);
						}
#endif
						EntityModel* em = nil;
						if (tt == nil)
						{
							em = labase->FindEntity(*cur, !fAllowAutoModel);
							if (em != nil)
							{
								em->Retain();
							}
						}
						else
						{
#if AllowDefaultEMBasedOnTables
							em = EntityModel::BuildLocalEntityModel(tt);
#endif
							tt->Release();
						}
						if (em == nil)
						{
							err = ThrowError(rest::entity_not_found, *cur);
							SetHTTPError(rest::http_not_found);
							getDef = false;
						}
						else
						{
							allEMs.push_back(em);
						}
					}
					if (!allEMs.empty())
					{
						em = VImpCreator<EntityModel>::GetImpObject(allEMs[0]);
						em->Retain();
					}
				}
			}
			else
			{
				getAllEntitiesDef2 = true;
				//if (restmethod == RestMethodeRetrieve)
				{
					err = labase->RetainAllEntityModels(allEMs, nil, true, !fAllowAutoModel);
					if (err != VE_OK)
					{
						SetHTTPError(rest::http_internal_error);
						getDef = false;
					}
				}
			}
		}
	}
}


void RestTools::AnalyseGetModelAndEntities()
{
	if (labase != nil)
	{
		VectorOfVString allIDs;
		VString valstr;
		bool withkeys = false;
		bool newRec = false;

		sLONG p1 = s->FindUniChar('(');
		if (p1 <= 0)
		{
			tablename = *s;
		}
		else
		{
			withkeys = true;
			s->GetSubString(1, p1-1, tablename);

			Wordizer ww(*s);
			ww.SetCurPos(p1);
			ww.GetNextWord(valstr, ')', false, true);
			if (valstr.IsEmpty())
				newRec = true;
			else
			{
				Wordizer ww2(valstr);
				ww2.ExctractStrings(allIDs, true, ',', true);
			}
		}

		VString realtablename;
		Table* tt = nil;
#if AllowDefaultEMBasedOnTables
		if (s->GetLength() > 1 && tablename.GetUniChar(1) == kEntityTablePrefixChar)
		{
			tablename.GetSubString(2, tablename.GetLength()-1, realtablename);
			tt = labase->FindAndRetainTableRef(realtablename);
		}
#endif
		if (tt == nil)
		{
			em = labase->FindEntity(tablename, !fAllowAutoModel);
			if (em != nil)
			{
				if (em->getScope() != escope_public)
					em = nil;
				else
					em->Retain();
			}
		}
		else
		{
#if AllowDefaultEMBasedOnTables
			em = EntityModel::BuildLocalEntityModel(tt);
#endif
			tt->Release();
		}

		if (em == nil)
		{
			err = ThrowError(rest::entity_not_found, tablename);
			SetHTTPError(rest::http_not_found);
		}
		else
		{
			if (withkeys && !newRec)
			{
				if (allIDs.size() <= 1)
					fEntityKey = allIDs[0];
				else
				{
					selectedEntities = em->NewCollection(allIDs, err, fContext);
					dejaselected = true;
				}


				if (err != VE_OK)
				{
					SetHTTPError(rest::http_not_found);
				}
			}
		}
	}
}


void RestTools::AnalyseGetAttributes()
{
	if (!getDef)
	{
		while (s != nil && err == VE_OK)
		{
			if (*s == L"$entityset" || *s == L"$$entityset")
			{
				VUUID xID;
				s = url.NextPart();
				if (s != nil)
				{
					xID.FromString(*s);
					curdataset = VDBMgr::GetManager()->RetainKeptDataSet(xID);
					if (curdataset != nil)
					{
						if (em != curdataset->GetModel())
						{
							err = ThrowError(rest::dataset_not_matching_entitymodel, *s, em->GetName());
							SetHTTPError(rest::http_not_found);
							ReleaseRefCountable(&curdataset);
						}
					}
					else
					{
						VString ss;
						if (url.GetValue(rest::savedfilter, fSavedQuery) || url.GetValue(rest::savedorderby, ss))
						{
							
							EntityCollection* sel = nil;
							VString querystring = fSavedQuery;
							VString orderbystring;
							if (querystring != L"$all" && !querystring.IsEmpty())
							{
								SearchTab search(em);
								err = search.BuildFromString(querystring, orderbystring, fContext, em);
								if (err == VE_OK)
								{
									{
										sel = em->executeQuery(&search, fContext, nil,  GetProgressIndicator(), DB4D_Do_Not_Lock, 0, nil, &err);
									}
								}
							}
							else
							{
								if (sel == nil)
								{
									sel = em->SelectAllEntities(fContext, &err);
								}
							}

							if (orderbystring.IsEmpty())
								orderbystring = em->GetDefaultOrderBy();

							if ((url.GetValue(rest::savedorderby, ss) || !orderbystring.IsEmpty()) && err == VE_OK && sel != nil)
							{
								if (em != nil)
								{
									EntityAttributeSortedSelection* SortingAtt = new EntityAttributeSortedSelection(em);
									if (orderbystring.IsEmpty())
									{
										if (!em->BuildListOfSortedAttributes(ss, *SortingAtt, fContext, false, true, nil))
										{
											err = ThrowError(rest::wrong_list_of_attribute_to_order_by, ss, em->GetName());
											SetHTTPError(rest::http_not_found);
										}
									}
									else
									{
										if (!em->BuildListOfSortedAttributes(orderbystring, *SortingAtt, fContext, false, true, nil))
										{
											err = ThrowError(rest::wrong_list_of_attribute_to_order_by, orderbystring, em->GetName());
											SetHTTPError(rest::http_not_found);
										}
									}
									EntityCollection* truesel = sel->SortSel(err, SortingAtt, fContext, GetProgressIndicator());

									if (truesel == nil)
									{
										truesel = sel;
										truesel->Retain();
									}

									QuickReleaseRefCountable(sel);
									sel = truesel;
								}										
							}

							if (err == VE_OK)
							{
								uLONG timeout = 600 * 1000; // 10 mn
								VString stimeout;
								if (url.GetValue(rest::timeout, stimeout))
								{
									timeout = stimeout.GetLong() * 1000;
								}
								curdataset = new DataSet(em, sel, timeout);
								curdataset->SetID(xID);
								if (!VDBMgr::GetManager()->AddKeptDataSet(curdataset))
								{
									curdataset->Release();
									curdataset = nil;
								}
							}

						}
						else
						{
							err = ThrowError(rest::dataset_not_found, s);
							SetHTTPError(rest::http_not_found);
						}
					}
				}
			}
			else
			{
				VString lasuite;
				sLONG p1 = s->FindUniChar('(');
				if (p1 <= 0)
				{
					lasuite = *s;
				}
				else
				{
					s->GetSubString(1, p1-1, lasuite);
					VString params;
					Wordizer ww(*s);
					ww.SetCurPos(p1);
					ww.GetNextWord(params, ')', false, true);

					Wordizer ww2(params);
					ww2.ExctractStrings(methParams, true, ',', true);
				}
				meth = em->getMethod(lasuite, true);
				if (meth == nil)
				{
					if (!em->BuildListOfSortedAttributes(*s, attributes, fContext, false, false, this))
					{
						err = ThrowError(rest::cannot_build_list_of_attribute, *s, em->GetName());
						SetHTTPError(rest::http_not_found);
					}
				}
				else
				{
					if (restmethod != RestMethodeBuildDataSet && restmethod != RestMethodeBuildSubDataSet)
						restmethod = RestMethodeRetrieve;				
					if (wasapost || wasaput)
					{
						{
							if (methParams.empty())
							{
								fJsonMethParams = GetInputString();
							}
						}

					}
					else
					{
						url.GetValue(rest::params, fJsonMethParams);
					}
				}
			}
			s = url.NextPart();
		}

		VString ss;
		if (url.GetValue(rest::emMethod, ss))
		{
			meth = em->getMethod(ss, true);
			if (meth != nil)
			{
				if (restmethod != RestMethodeBuildDataSet && restmethod != RestMethodeBuildSubDataSet)
					restmethod = RestMethodeRetrieve;				
				if (wasapost || wasaput)
				{
					fJsonMethParams = GetInputString();
				}
			}
		}
	}
}


void RestTools::AnalyseGetExpandAttributes()
{
	VString ss;
	if (attributes.size() == 1 && !getDef && err == VE_OK)
	{
		const EntityAttribute* att = attributes.begin()->fAttribute;
		if (att->GetDataKind() == VK_IMAGE)
		{
			onepictfield = true;
		}
	}

	if (url.GetValue(rest::asArray, ss))
	{
		if (ss == "1" || ss == "true")
			fToArray = true;
	}

	if (url.GetValue(rest::queryLimit, ss))
	{
		fQueryLimit = ss.GetLong();
		if (fQueryLimit < 0)
			fQueryLimit = 0;
	}


	if (em != nil && err == VE_OK)
	{
		if ((meth == nil) && url.GetValue(rest::expand, ss))
		{
			if (fToArray)
			{
				if (!em->BuildListOfSortedAttributes(ss, attributes, fContext, false, false, this))
				{
					err = ThrowError(rest::cannot_build_list_of_attribute, ss, em->GetName());
					SetHTTPError(rest::http_not_found);
				}			
			}

			if (err == VE_OK)
			{
				expandattributes = new EntityAttributeSelection(em);
				if (ss == L"$all")
				{
					em->GetAllAttributes(*expandattributes, this);
				}
				else
				{
					
					if (!em->BuildListOfAttributes(ss, *expandattributes, false, this))
					{
						err = ThrowError(rest::cannot_build_list_of_attribute_for_expand, ss, em->GetName());
						SetHTTPError(rest::http_not_found);
					}
				}
			}
		}
	}
}


void RestTools::AnalyseGetOtherStuff()
{
	VString ss;
	if (url.GetValue(rest::imageformat, ss) && err == VE_OK)
	{
		if (ss != L"best")
			imageformatstring = ss;
		waitforimagemime = true;
	}
	if (url.GetValue(rest::prettyformatting, ss) || url.GetValue(rest::prettyprint, ss) && err == VE_OK)
	{
		SetPrettyFormatting(ss == L"true" || ss == L"1");
	}

	if (url.GetValue(rest::atOnce,ss) || url.GetValue(rest::atomic,ss) && err == VE_OK)
	{
		SetAtomic(ss == L"1" || ss == L"true");
	}
}


void RestTools::WorkOnCatalog()
{
	if (labase != nil)
	{
		VString ss;
		switch (restmethod)
		{
			case RestMethodeFlush:
				VDBMgr::GetManager()->FlushCache(labase, false);
				break;
			case RestMethodeRefresh:
			case RestMethodeRetrieve:
				/*
				if (getdatabasedef)
				{
					bool metadata = url.GetValue(rest::metadata, ss);
					VValueBag bag;
					for (sLONG i = 1, nb = labase->GetNBTable(); i <= nb && err == VE_OK; i++)
					{
						Table* tt = labase->RetainTable(i);
						if (tt != nil)
						{
							EntityModel* em = EntityModel::BuildLocalEntityModel(tt);
							if (em != nil)
							{
								BagElement subbag(bag, d4::dataClasses);
								em->ToBag(*subbag, false, metadata, !fToXML, this);
								em->Release();
							}
							tt->Release();
						}
					}
					if (fToXML)
					{
						VString xml;
						SaveBagToXML(bag, L"Catalog", xml, IsPrettyFormatting(), nil, true);
						PutText(xml);
					}
					else
					{
						VString jsonStr;
						bag.GetJSONString(jsonStr, IsPrettyFormatting() ? JSON_PrettyFormatting : JSON_Default);
						PutText(jsonStr);
					}
				}
				else
				*/
				{
					if (em != nil && allEMs.size() == 1)
					{
						VValueBag bag;
						em->ToBag(bag, false, url.GetValue(rest::metadata, ss), !fToXML, this);
						if (fToXML)
						{
							VString xml;
							SaveBagToXML(bag, L"entityModel", xml, IsPrettyFormatting(), nil, true);
							PutText(xml);
						}
						else
						{
							VString jsonStr;
							bag.GetJSONString(jsonStr, IsPrettyFormatting() ? JSON_PrettyFormatting : JSON_Default);
							PutText(jsonStr);
						}

					}
					else
					{
						bool dbstruct = false;
						bool metadata = url.GetValue(rest::metadata, ss);
						if (ss == L"relational" || ss == L"db" || ss == L"database")
							dbstruct = true;
						VValueBag bag;
						if (getAllEntitiesDef || em != nil)
						{
							if (getAllEntitiesDef && metadata)
							{
								if (dbstruct)
									err = labase->SaveToBagDeep(bag, ss, true, true, true);
								else
									err = labase->GetEntityCatalog(!fAllowAutoModel)->SaveEntityModels(bag, fToXML);
							}
							else
							{
								for (vector<CDB4DEntityModel*>::iterator cur = allEMs.begin(), end = allEMs.end(); cur != end; cur++)
								{
									CDB4DEntityModel* xem = *cur;
									EntityModel* em1 = VImpCreator<EntityModel>::GetImpObject(xem);
									if (em1->getScope() == escope_public)
									{
										BagElement subbag(bag, d4::dataClasses);
										em1->ToBag(*subbag, false, url.GetValue(rest::metadata, ss), !fToXML, this);
									}
								}
							}
						}
						else
						{
							if (getAllEntitiesDef2 && metadata)
							{
								if (dbstruct)
									err = labase->SaveToBagDeep(bag, ss, true, true, true);
								else
									err = labase->GetEntityCatalog(!fAllowAutoModel)->SaveEntityModels(bag, fToXML);
							}
							else
							{
								for (vector<CDB4DEntityModel*>::iterator cur = allEMs.begin(), end = allEMs.end(); cur != end; cur++)
								{
									CDB4DEntityModel* xem = *cur;
									EntityModel* em1 = VImpCreator<EntityModel>::GetImpObject(xem);
									if (em1->getScope() == escope_public)
									{
										BagElement subbag(bag, d4::dataClasses);
										subbag->SetAttribute(d4::name, xem->GetEntityName());
										VString uri;
										CalculateURI(uri, VImpCreator<EntityModel>::GetImpObject(xem), L"", false, false);
										subbag->SetAttribute(d4::uri, uri);
										CalculateDataURI(uri, VImpCreator<EntityModel>::GetImpObject(xem), L"", false, false);
										subbag->SetAttribute(d4::dataURI, uri);
									}
								}
							}
						}
						if (fToXML)
						{
							VString xml;
							SaveBagToXML(bag, L"Catalog", xml, IsPrettyFormatting(), nil, true);
							PutText(xml);
						}
						else
						{
							VString jsonStr;
							bag.GetJSONString(jsonStr, IsPrettyFormatting() ? JSON_PrettyFormatting : JSON_Default);
							PutText(jsonStr);
						}
					}
				}

				break;

			case RestMethodeUpdate:
			case RestMethodeValidate:
				{
					LocalEntityModelCatalog* newcat = new LocalEntityModelCatalog(labase);
					VValueBag bagData;
					if (fToXML)
					{
						err = LoadBagFromXML(GetInputString(), L"EntityModelCatalog", bagData);
					}
					else
					{
						err = bagData.FromJSONString(GetInputString(), JSON_Default);
					}
					if (err == VE_OK)
					{
						err = newcat->LoadEntityModels(bagData, false);
					}

					if (restmethod == RestMethodeUpdate && err == VE_OK)
					{
						labase->SetEntityCatalog(newcat);
						err = labase->SaveEntityModels();
					}

					if (err != VE_OK)
					{
						SetHTTPError(rest::http_bad_request);
					}

					QuickReleaseRefCountable(newcat);

				}

				break;

			default:
				err = ThrowError(rest::method_not_applicable, restmethodName);
				SetHTTPError(rest::http_bad_request);
				break;
		}
	}
}

void RestTools::CallMethod(EntityCollection* sel, EntityRecord* erec, DataSet* inDataSet)
{
	VJSContext jscontext( fJSGlobalContext);
	VJSValue result(jscontext);
	if (!methParams.empty() || fJsonMethParams.IsEmpty())
	{
		if (meth->GetKind() == emeth_sel && sel != nil)
		{
			err = em->callMethod(meth->GetName(), methParams, result, fContext, sel, nil);
		}
		else if (meth->GetKind() == emeth_static)
		{
			err = em->callMethod(meth->GetName(), methParams, result, fContext, nil, nil);
		}
		else if (meth->GetKind() == emeth_rec && erec != nil)
		{
			err = em->callMethod(meth->GetName(), methParams, result, fContext, nil, erec);
		}
	}
	else
	{
		if (meth->GetKind() == emeth_sel && sel != nil)
		{
			err = em->callMethod(meth->GetName(), fJsonMethParams, result, fContext, sel, nil);
		}
		else if (meth->GetKind() == emeth_static)
		{
			err = em->callMethod(meth->GetName(), fJsonMethParams, result, fContext, nil, nil);
		}
		else if (meth->GetKind() == emeth_rec && erec != nil)
		{
			err = em->callMethod(meth->GetName(), fJsonMethParams, result, fContext, nil, erec);
		}
	}

	ReturnJSResult(err, result, nil, inDataSet);

}


void RestTools::ReturnJSResult(VError err, VJSValue& result, JS4D::ExceptionRef excep, DataSet* inDataSet)
{
	VJSContext jscontext( fJSGlobalContext);
	if (err == VE_OK && excep == nil)
	{
		EntitySelectionIterator* xrec = result.GetObjectPrivateData<VJSEntitySelectionIterator>();
		if (xrec != nil)
		{
			EntityRecord* xerec = xrec->GetCurRec(fContext);
			if (xerec == nil)
			{
				PutJsonBeginObject();
				PutJsonPropertyName(L"result");
				PutText(L"null");
				PutJsonEndObject();			
			}
			else
				err = EntityRecordToJSON(VImpCreator<EntityRecord>::GetImpObject(xerec), attributes, expandattributes, nil, true);
		}
		else
		{
			EntityCollection* xsel = result.GetObjectPrivateData<VJSEntitySelection>();
			if (xsel != nil)
			{
				EntityModel* resModel = xsel->GetModel();
				if (testAssert(resModel != nil))
				{
					RetrieveSelDelimiters();
					EntityModel* resEM = resModel;
					EntityAttributeSortedSelection resAtts(resEM);
					EntityAttributeSelection* resExpandAttributes = nil;
					VString ss;
					if (url.GetValue(rest::expand, ss))
					{
						resExpandAttributes = new EntityAttributeSelection(resModel);
						if (ss == L"$all")
						{
							resModel->GetAllAttributes(*resExpandAttributes, this);
						}
						else
						{
							
							if (!resModel->BuildListOfAttributes(ss, *resExpandAttributes, false, this))
							{
								err = ThrowError(rest::cannot_build_list_of_attribute_for_expand, ss, resModel->GetName());
								SetHTTPError(rest::http_not_found);
							}
						}
					}

					uLONG timeout = 2 * 60 * 60 * 1000; // 2 heures
					VString stimeout;
					if (url.GetValue(rest::timeout, stimeout))
					{
						timeout = stimeout.GetLong() * 1000;
					}
					DataSet* newset = nil;

					if (restmethod == RestMethodeBuildDataSet)
					{
						newset = new DataSet(resEM, xsel, timeout);
						if (!VDBMgr::GetManager()->AddKeptDataSet(newset))
						{
							newset->Release();
							newset = nil;
						}
						else
						{
							VString uri;
							CalculateURI(uri, newset, resEM, L"", false, false);
							fResponse->AddResponseHeader(L"Content-Location", uri, true);
						}
					}

					err = SelToJSON(resEM, xsel, resAtts, resExpandAttributes, nil, true, skipfirst, countelem, newset, true);
				}

			}
			else
			{
				bool alreadyTaken = false;
				if (result.IsObject())
				{
					VJSObject resultObj(result.GetObject());
					if (resultObj.HasProperty("HTTPStream"))
					{
						VJSValue valStream(resultObj.GetProperty("HTTPStream"));

						VJSPictureContainer* piccontainer = valStream.GetObjectPrivateData<VJSImage>();

						if ( piccontainer != nil)
						{
							alreadyTaken = true;
							VPicture *vpic = piccontainer->GetPict();
							VString mime;
							err = ExtractBestPictureForWeb(*vpic, fResponse->GetResponseBody(),"", false, mime);
							fResponse->AddResponseHeader(L"Content-Type", mime, true);
						}
						else
						{
							bool mustdeleteStream = false;
							VStream* stream = valStream.GetObjectPrivateData<VJSStream>();
							if (stream == nil)
							{
								VJSTextStreamState *inStreamState = valStream.GetObjectPrivateData<VJSTextStream>();
								if (inStreamState != nil)
								{
									stream = inStreamState->GetStream();
									if (stream != nil)
									{
										if (stream->IsReading())
											stream->CloseReading();
										if (stream->IsWriting())
											stream->CloseWriting();
									}
								}
								else
								{
									VFile* file = valStream.GetFile();
									if (file != nil)
									{
										stream = new VFileStream(file);
										mustdeleteStream = true;
									}
								}
							}
							if (stream != nil)
							{
								alreadyTaken = true;
								err = stream->PutContentInto(fOutput);
								if (mustdeleteStream)
									delete stream;
							}
						}
					}

					if (alreadyTaken)
					{
						toJSON = false;
						fToXML = false;
						VJSObject objheaders(resultObj.GetPropertyAsObject("headers"));
						if (objheaders.IsObject())
						{
							vector<VString> propnames;
							objheaders.GetPropertyNames(propnames);
							for (vector<VString>::iterator cur = propnames.begin(), end = propnames.end(); cur != end; ++cur)
							{
								VString sVal;
								objheaders.GetPropertyAsString(*cur, nil, sVal);
								fResponse->AddResponseHeader(*cur, sVal, true);
							}
						}
					}
				}

				if (!alreadyTaken)
				{
					PutJsonBeginObject();

					if (inDataSet != nil)
					{
						PutJsonPropertyName(L"__ENTITYSET");
						VString uri;
						CalculateURI(uri, inDataSet, em, L"", false, true);
						PutText(uri);
						PutJsonSeparator();
						NewLine();	
					}

					PutJsonPropertyName(L"result");
					VJSJSON JSON(jscontext);
					VString outJson;
					//VValueSingle* xresult = result.CreateVValue();
					JSON.Stringify(result, outJson, nil);
					if (outJson == L"undefined")
						outJson = L"null";
					PutText(outJson);

					PutJsonEndObject();
				}
			}
		}
	}
	else
	{
		SetHTTPError(rest::http_internal_error);
		PutJsonBeginObject();

		if (inDataSet != nil)
		{
			PutJsonPropertyName(L"__ENTITYSET");
			VString uri;
			CalculateURI(uri, inDataSet, em, L"", false, true);
			PutText(uri);
			PutJsonSeparator();
			NewLine();	
		}

		VValueBag errbag;
		BuildErrorStack(errbag);
		dejaerror = true;
		VBagArray* errors = errbag.GetElements(d4::__ERROR);
		if (errors != nil)
		{
			VString errjson;
			sLONG level = 0;
			errors->_GetJSONString(errjson, level, false, JSON_Default);
			PutJsonPropertyName(L"__ERROR");
			PutText(errjson);			
		}
		else
		{
			VJSJSON JSON(jscontext);
			VString outJson;
			VJSValue excepval(jscontext, excep);
			JSON.Stringify(excepval, outJson, nil);
			if (outJson == L"undefined")
				outJson = L"null";
			PutJsonPropertyName(L"__ERROR");
			PutText(outJson);
		}
		PutJsonEndObject();
	}


}


void RestTools::RetrieveSelDelimiters()
{
	VString ss;
	countelem = -1;
	skipfirst = 0;
	if (url.GetValue(rest::top, ss) || url.GetValue(rest::limit, ss))
	{
		countelem = ss.GetLong();
	}
	else
	{
		if (!fToArray && em != nil)
		{
			countelem = em->GetDefaultTopSizeInUse();
			if (countelem <= 0)
				countelem = -1;
		}
	}

	if (url.GetValue(rest::skip, ss))
	{
		skipfirst = ss.GetLong();
	}
}


void RestTools::RetrieveOrDeleteEntitySel()
{
	if (meth != nil && meth->GetKind() == emeth_static)
	{
		CallMethod(nil, nil, nil);
	}
	else
	{
		EntityCollection* sel = nil;
		VString ss;
		RetrieveSelDelimiters();

		/*
		countelem = -1;
		skipfirst = 0;
		if (url.GetValue(rest::top, ss) || url.GetValue(rest::limit, ss))
		{
			countelem = ss.GetLong();
		}
		else
		{
			if (!fToArray)
			{
				countelem = em->GetDefaultTopSizeInUse();
				if (countelem <= 0)
					countelem = -1;
			}
		}

		if (url.GetValue(rest::skip, ss))
		{
			skipfirst = ss.GetLong();
		}
		*/
		EntityAttributeSortedSelection* SortingAtt = nil;


		if (dejaselected)
			sel = RetainRefCountable(selectedEntities);
		else
		{
			if (curdataset != nil)
				sel = RetainRefCountable(curdataset->GetSel());
		}
		VString querystring;
		VString orderbystring;
		if (url.GetValue(rest::filter, querystring))
		{
			if (url.GetValue(rest::querypath, ss))
				SetWithQueryPath(ss == L"true" || ss == L"1");
			if (url.GetValue(rest::queryplan, ss))
				SetWithQueryPlan(ss == L"true" || ss == L"1");


			if (em->QueriesAreProcessedRemotely())
			{
				VJSONArray* params = nil;
				VString paramstring;
				if (url.GetValue(rest::params, paramstring))
				{
					VJSONImporter json(paramstring);
					VJSONValue paramsVal;
					err = json.Parse(paramsVal);
					if (err == VE_OK)
					{
						if (paramsVal.IsArray())
							params = RetainRefCountable(paramsVal.GetArray());
					}
				}
				if (err == VE_OK)
				{
					EntityCollection* oldsel = sel;
					sel = em->executeQuery(querystring, params, fContext, oldsel, GetProgressIndicator(), DB4D_Do_Not_Lock, 0, nil, &err);
					QuickReleaseRefCountable(oldsel);
				}
				QuickReleaseRefCountable(params);
			}
			else
			{
				Boolean olddescribe = fContext->ShouldDescribeQuery();
				fContext->MustDescribeQuery(WithQueryPath() || WithQueryPlan());

				QueryParamElementVector qparams;
				{
					VString paramstring;
					if (url.GetValue(rest::params, paramstring))
					{
						VJSContext jscontext( fJSGlobalContext);
						VJSJSON json(jscontext);
						JS4D::ExceptionRef except = nil;
						VJSValue params(json.Parse(paramstring, &except));
						if (!params.IsNull() && !params.IsUndefined() && params.IsInstanceOf("Array"))
						{
							VJSObject valobj(jscontext);
							params.GetObject(valobj);
							VJSArray paramArr(valobj);
							for (sLONG i = 0, nb = paramArr.GetLength(); i < nb; i++)
							{
								VJSValue jsval(paramArr.GetValueAt(i));
								if (jsval.IsArray())
								{
									VJSArray arr(jsval, false);
									qparams.push_back(QueryParamElement(arr));
								}
								else
								{
									VValueSingle* cv = jsval.CreateVValue();
									if (cv == nil)
									{
										cv = new VString();
										cv->SetNull(true);
									}
									if (cv->GetValueKind() == VK_TIME)
									{
										VValueSingle* cv2 = jsval.CreateVValue(nil, true);
										qparams.push_back(QueryParamElement(cv, cv2));
									}
									else
										qparams.push_back(QueryParamElement(cv));
								}
							}
						}
					}
				}
				
				SearchTab search(em);
				err = search.BuildFromString(querystring, orderbystring, fContext, em, false, &qparams);
				if (err == VE_OK)
				{
					VString paramstring;
					if (url.GetValue(rest::params, paramstring))
					{
						VJSContext jscontext( fJSGlobalContext);
						VJSJSON json(jscontext);
						JS4D::ExceptionRef except = nil;
						VJSValue params(json.Parse(paramstring, &except));
						if (!params.IsNull() && !params.IsUndefined() && params.IsInstanceOf("Array"))
						{
							VJSObject valobj(jscontext);
							params.GetObject(valobj);
							VJSArray paramArr(valobj);
							vector<VString> ParamNames;
							QueryParamElementVector ParamValues;

							err = search.GetParams(ParamNames, ParamValues);
							if (err == VE_OK)
							{
								QueryParamElementVector::iterator curvalue = ParamValues.begin();
								for (vector<VString>::iterator curname = ParamNames.begin(), endname = ParamNames.end(); curname != endname; curname++, curvalue++)
								{
									const VString& s = *curname;
									if (s.GetLength() == 1)
									{
										UniChar c = s[0];
										if (c >= '1' && c <= '9')
										{
											sLONG paramnum = s.GetLong()-1;
											VJSValue jsval(paramArr.GetValueAt(paramnum));
											if (jsval.IsArray())
											{
												VJSArray jarr(jsval, false);
												curvalue->Dispose();
												*curvalue = QueryParamElement(jarr);
											}
											else
											{
												VValueSingle* cv = jsval.CreateVValue();
												if (cv != nil)
												{
													curvalue->Dispose();
													if (cv->GetValueKind() == VK_TIME)
													{
														VValueSingle* cv2 = jsval.CreateVValue(nil, true);
														*curvalue = QueryParamElement(cv, cv2);
													}
													else
														*curvalue = QueryParamElement(cv);
												}
											}
										}
									}
								}
								err = search.SetParams(ParamNames, ParamValues);
							}
			
						}
					}

					{
						EntityCollection* oldsel = sel;
						sel = em->executeQuery(&search, fContext, oldsel, GetProgressIndicator(), DB4D_Do_Not_Lock, 0, nil, &err);
						QuickReleaseRefCountable(oldsel);
					}

					if (WithQueryPlan())
					{
						SetQueryPlan(fContext->GetLastQueryPlan());
					}
					if (WithQueryPath())
					{
						SetQueryPath(fContext->GetLastQueryPath());
					}
				}
				fContext->MustDescribeQuery(olddescribe);
			}
		}
		else
		{
			if (sel == nil)
			{
				sel = em->SelectAllEntities(fContext, &err);
			}
		}

		if (url.GetValue(rest::removeFromSet, ss))
		{
			sLONG posToRemove = ss.GetLong();
			if (posToRemove >=0 && posToRemove < sel->GetLength(fContext))
			{
				bool refonly = false;
				VString sref;
				if (url.GetValue(rest::removeRefOnly, sref))
				{
					refonly = (sref == "true" || sref == "1");
				}
				if (!refonly)
				{
					EntityRecord* recToDel = sel->LoadEntity(posToRemove, fContext, err, DB4D_Do_Not_Lock);
					if (recToDel != nil)
					{
						err = recToDel->Drop();
						recToDel->Release();
					}
				}
				if (err == VE_OK)
					sel->RemoveSelectedRange(posToRemove, posToRemove, fContext);
			}
		}

		if (url.GetValue(rest::addToSet, ss) && err == VE_OK)
		{
			VJSContext jscontext( fJSGlobalContext);
			VJSJSON json(jscontext);
			VJSValue val(json.Parse(ss));
			if (!val.IsUndefined() && val.IsArray())
			{
				VJSArray arr(val.GetObject());
				sLONG nbElem = arr.GetLength();
				for (sLONG i = 0; i < nbElem && err == VE_OK; i++)
				{
					EntityRecord* erec = nil;
					{
						StErrorContextInstaller errs(false);
						VError err2;
						VJSValue key(arr.GetValueAt(i));
						if (key.IsObject())
						{
							// traite ici les cles composees
							erec = em->findEntityWithPrimKey(key.GetObject(), fContext, err2, DB4D_Do_Not_Lock);
						}
						else
						{
							VValueSingle* cv = key.CreateVValue();
							if (cv != nil)
							{
								VectorOfVValue keys(true);
								keys.push_back(cv);
								erec = em->findEntityWithPrimKey(keys, fContext, err2, DB4D_Do_Not_Lock);
							}
						}
					}
					if (erec != nil)
					{
						err = sel->AddEntity(erec, true);
						erec->Release();
					}
				}
			}

		}

		if (url.GetValue(rest::fromSel, ss) && err == VE_OK)
		{
			VJSContext jscontext( fJSGlobalContext);
			VJSJSON json(jscontext);
			VJSValue val(json.Parse(ss));
			WafSelection wafsel(val);

			EntityCollection* newsel = sel->NewFromWafSelection(&wafsel, fContext);
			sel->Release();
			sel = newsel;

		
		}


		if (orderbystring.IsEmpty())
			orderbystring = em->GetDefaultOrderBy();

		if ((url.GetValue(rest::orderby, ss) || !orderbystring.IsEmpty()) && err == VE_OK)
		{
			if (em != nil)
			{
				SortingAtt = new EntityAttributeSortedSelection(em);
				if (orderbystring.IsEmpty())
				{
					if (!em->BuildListOfSortedAttributes(ss, *SortingAtt, fContext, false, true, nil))
					{
						err = ThrowError(rest::wrong_list_of_attribute_to_order_by, ss, em->GetName());
						SetHTTPError(rest::http_not_found);
					}
				}
				else
				{
					if (!em->BuildListOfSortedAttributes(orderbystring, *SortingAtt, fContext, false, true, nil))
					{
						err = ThrowError(rest::wrong_list_of_attribute_to_order_by, orderbystring, em->GetName());
						SetHTTPError(rest::http_not_found);
					}
				}
			}										
		}

		if (sel != nil && err == VE_OK)
		{
			/*
			if (fQueryLimit > 0)
				sel->ReduceSel(fQueryLimit);
				*/

			if (restmethod == RestMethodeDelete)
			{
				if (fImportAtomic)
					fContext->StartTransaction(err);
				if (err == VE_OK)
				{
					err = sel->DropEntities(fContext, nil, nil);
					if (fImportAtomic)
					{
						if (err == VE_OK)
							fContext->CommitTransaction();
						else
							fContext->RollBackTransaction();
					}
				}
			}
			else
			{
				DataSet* newset = nil;
				VString logicoper;
				bool alreadyResult = false;

				EntityCollection* truesel = nil;

				if (url.GetValue(rest::logicOperator, logicoper))
				{
					EntityCollection* newsel = nil;
					alreadyResult = MixCollection(sel, logicoper, newsel);
					if (newsel != nil)
					{
						CopyRefCountable(&sel, newsel);
						newsel->Release();
					}
					truesel = sel;
					truesel->Retain();
				}
				else
				{
					WafSelection wafkeepsel;
					WafSelection* wafkeepselptr = nil;
					VString keepSelStr;
					if (url.GetValue(rest::keepSel, keepSelStr))
					{
						VJSContext jscontext( fJSGlobalContext);
						VJSJSON json(jscontext);
						VJSValue val(json.Parse(keepSelStr));
						wafkeepsel.buildFromJS(val);
						wafkeepselptr = &wafkeepsel;
						newwafkeepselptr = new WafSelection();
					}

					truesel = sel->SortSel(err, SortingAtt, fContext,  GetProgressIndicator(), wafkeepselptr, newwafkeepselptr);

					if (truesel == nil)
					{
						truesel = sel;
						truesel->Retain();
					}
				}

				if (restmethod == RestMethodeBuildDataSet && err == VE_OK)
				{
					if (curdataset != nil && curdataset->GetSel() == truesel)
					{
						newset = RetainRefCountable(curdataset);
					}
					else
					{
						uLONG timeout = 2 * 60 * 60 * 1000; // 2 heures
						VString stimeout;
						if (url.GetValue(rest::timeout, stimeout))
						{
							timeout = stimeout.GetLong() * 1000;
						}
						newset = new DataSet(em, truesel, timeout);
						if (!VDBMgr::GetManager()->AddKeptDataSet(newset))
						{
							newset->Release();
							newset = nil;
						}
						else
						{
							VString uri;
							CalculateURI(uri, newset, em, L"", false, false);
							fResponse->AddResponseHeader(L"Content-Location", uri, true);
						}
					}
				}

				if (err == VE_OK && !alreadyResult)
				{
					if (em != nil)
					{
						if (meth != nil)
						{
							CallMethod(sel, nil, newset);
						}
						else
						{
							VString keyval;
							VString computeval;
							if (url.GetValue(rest::findKey, keyval))
							{
								FindKeyInCollection(keyval, truesel);
							}
							else if (url.GetValue(rest::compute, computeval))
							{
								ComputeOnCollection(computeval, truesel);
							}
							else
							{
								VString distinctVal;
								bool mustDistinct = false;;
								if (url.GetValue(rest::distinct, distinctVal))
								{
									if (distinctVal == L"true")
										mustDistinct = true;
								}
								if (mustDistinct)
									GetDistinctValues(sel);
								else
								{
									if (fToArray)
									{
										VJSContext jscontext( fJSGlobalContext);
										VJSArray arr(jscontext);
										bool nokey = false;
										VString nokeys;
										if (url.GetValue(rest::noKey, nokeys))
										{
											if (nokeys == "1" || nokeys == "true")
												nokey = true;
										}
										arr = sel->ToJsArray(fContext, jscontext, attributes, expandattributes, SortingAtt, !nokey, false, skipfirst, countelem, err);

										VJSJSON JSON(jscontext);
										VString outJson;
										JSON.Stringify(arr, outJson, nil);
										if (outJson == L"undefined")
											outJson = L"null";
										PutText(outJson);
									}
									else if (fToXML)
									{
										VValueBag bag;
										err = SelToBag(bag, em, truesel, attributes, expandattributes, SortingAtt, true, skipfirst, countelem, newset, true);
										if (err == VE_OK)
										{
											VString xml;
											SaveBagToXML(bag, L"result", xml, IsPrettyFormatting(), nil, true);
											PutText(xml);
										}
									}
									else
										err = SelToJSON(em, truesel, attributes, expandattributes, SortingAtt, true, skipfirst, countelem, newset, true);
								}
							}
						}
					}
				}

				truesel->Release();
				sel->Release();
				QuickReleaseRefCountable(newset);
			}
		}

		if (SortingAtt != nil)
			delete SortingAtt;
	}
}


void RestTools::FindKeyInCollection(const VString& keyval, EntityCollection* sel)
{
	VString result = "-1";
	VectorOfVString vals;
	vals.push_back(keyval);
	RecIDType pos = sel->FindKey(vals, fContext, err);
	result.FromLong(pos);
	
	if (err == VE_OK)
	{
		PutJsonBeginObject();
		PutJsonPropertyName(L"result");
		PutText(result);
		PutJsonEndObject();			
	}
}


bool RestTools::MixCollection(EntityCollection* sel, const VString& logicOper, EntityCollection*& outSel)
{
	bool AlreadySentResult = false;
	outSel = nil;
	VString othercolurl;
	if (url.GetValue(rest::otherCollection, othercolurl))
	{
		DB4DConjunction oper = DB4D_NOTCONJ;
		if (logicOper == "and")
			oper = DB4D_And;
		else if (logicOper == "or")
			oper = DB4D_OR;
		else if (logicOper == "except")
			oper = DB4D_Except;
		else if (logicOper == "intersect")
			oper = DB4D_Intersect;
		else
			err = ThrowError(rest::wrong_logic_operator, logicOper);

		if (oper != DB4D_NOTCONJ)
		{
			VFullURL urlOther(othercolurl);
			const VString* entitysetref = urlOther.NextPart();
			if (entitysetref == nil)
			{
				err = ThrowError(rest::wrong_other_collection_ref, othercolurl);
			}
			else
			{
				VUUID xID;
				xID.FromString(*entitysetref);
				DataSet* othercolset = VDBMgr::GetManager()->RetainKeptDataSet(xID);
				if (othercolset != nil)
				{
					EntityModel* otherem = othercolset->GetModel();
					if (otherem != nil)
					{
						if (otherem->getScope() != escope_public)
							otherem = nil;
						else
							otherem->Retain();
					}
					if (otherem == nil)
						err = ThrowError(rest::wrong_other_collection_ref, othercolurl);
					else
					{
						switch (oper)
						{
							case DB4D_And:
								outSel = sel->And(othercolset->GetSel(), err, fContext);
								break;
							case DB4D_OR:
								outSel = sel->Or(othercolset->GetSel(), err, fContext);
								break;
							case DB4D_Except:
								outSel = sel->Minus(othercolset->GetSel(), err, fContext);
								break;
							case DB4D_Intersect:
								{
									bool result = sel->Intersect(othercolset->GetSel(), err, fContext);
									if (err == VE_OK)
									{
										AlreadySentResult = true;
										if (result)
											PutText("true");
										else
											PutText("false");
									}
								}
								break;
						}
					}
					
					othercolset->Release();
				}
				else
					err = ThrowError(rest::wrong_other_collection_ref, othercolurl);
			}
		}
	}
	else
		err = ThrowError(rest::missing_other_collection_ref);

	return AlreadySentResult;
}


void RestTools::ComputeOnCollection(const VString& computeval, EntityCollection* sel)
{
	VJSContext jscontext( fJSGlobalContext);
	VJSValue result(jscontext);
	bool okcompute = false;
	const EntityAttribute* att = nil;

	VString distinctVal;
	bool mustDistinct = false;;
	if (url.GetValue(rest::distinct, distinctVal))
	{
		if (distinctVal == L"true")
			mustDistinct = true;
	}

	DB4D_ColumnFormulae action = (DB4D_ColumnFormulae)EFormulaeActions[computeval];
	if (action == DB4D_Count || !attributes.empty())
	{
		if (computeval == "all" || computeval == "$all")
		{
			okcompute = true;
			VJSObject objresult(jscontext);
			err  = sel->Compute(attributes, objresult, fContext, jscontext, mustDistinct);
			if (err == VE_OK)
				result = objresult;
		}
		else
		{
			if (action != DB4D_ColumnFormulae_none)
			{
				if (mustDistinct)
				{
					if (action == DB4D_Sum)
						action = DB4D_Sum_distinct;
					else if (action == DB4D_Count)
						action = DB4D_Count_distinct;
					else if (action == DB4D_Average)
						action = DB4D_Average_distinct;
				}
				okcompute = true;
				if (action == DB4D_Count && attributes.empty())
				{
					result.SetNumber(em->countEntities(fContext));
				}
				else
				{
					att = attributes[0].fAttribute;
					err = sel->ComputeOnOneAttribute(att, action, result, fContext, jscontext);
				}
			}
			else
				err = ThrowError(rest::compute_action_does_not_exist, computeval);
		}
	}
	else
		err = ThrowError(rest::empty_attribute_list);

	if (err == VE_OK && okcompute)
	{
		VString jsonString;
		VJSJSON json(result.GetContextRef());
		json.Stringify(result, jsonString);
		PutText(jsonString);
	}
}


void RestTools::GetDistinctValues(EntityCollection* sel)
{
	EntityAttribute* att = nil;
	
	for (EntityAttributeSortedSelection::iterator cur = attributes.begin(), end = attributes.end(); cur != end && att == nil; cur++)
	{
		const EntityAttribute* xatt = cur->fAttribute;
		if (xatt != nil)
		{
			if (/*att->GetKind() == eattr_field ||*/ xatt->GetKind() != eattr_relation_1toN && xatt->GetKind() != eattr_relation_Nto1 && xatt->GetKind() != eattr_composition)
			{
				att = (EntityAttribute*)xatt;
			}
		}
	}

	if (att != nil)
	{
		VCompareOptions options;
		options.SetIntlManager(GetContextIntl(fContext));
		VJSContext jscontext( fJSGlobalContext);
		JSCollectionManager collection(jscontext, att->isSimpleDate());
		collection.SetNumberOfColumn(1);
		VError err = sel->GetDistinctValues(att, collection, fContext, nil, options);

		if (err == VE_OK)
		{
			countelem = -1;
			skipfirst = 0;
			VString ss;
			if (url.GetValue(rest::top, ss) || url.GetValue(rest::limit, ss))
			{
				countelem = ss.GetLong();
				if (countelem <0)
					countelem = 0;
			}

			if (url.GetValue(rest::skip, ss))
			{
				skipfirst = ss.GetLong();
			}


			VJSArray result(collection.getArrayRef(1));

			sLONG nbelem = result.GetLength();
			sLONG maxelem;

			if (skipfirst >= nbelem)
				skipfirst = nbelem - 1;
			if (skipfirst < 0)
				skipfirst = 0;
			if (countelem == -1)
				maxelem = nbelem;
			else
			{
				maxelem = skipfirst+countelem;
				if (maxelem > nbelem)
					maxelem = nbelem;
			}

			if (err == VE_OK)
			{
				VString jsonString;
				result.Remove(maxelem, nbelem-maxelem);
				result.Remove(0, skipfirst);
				VJSJSON json(result.GetContextRef());
				json.Stringify(result, jsonString);
				PutText(jsonString);
			}

		}
	}

}


void RestTools::RetrieveOrDeleteOneEntity()
{
	if (em != nil)
	{
		EntityRecord* erec = nil;
		if (fEntityKey.IsEmpty())
			erec = em->newEntity(fContext);
		else
			erec = em->findEntityWithPrimKey(fEntityKey, fContext, err, DB4D_Do_Not_Lock);
		if (erec != nil)
		{
			if (meth != nil)
			{
				CallMethod(nil, erec, nil);
			}
			else
			{
				if (restmethod == RestMethodeBuildSubDataSet)
				{
					EntityAttributeItem* attitem = nil;
					if (expandattributes != nil)
					{
						for (EntityAttributeSelection::iterator cur = expandattributes->begin(), end = expandattributes->end(); cur != end && attitem == nil; cur++)
						{
							if (cur->fAttribute != nil)
								attitem = &(*cur);
						}

					}
					if (attitem != nil)
					{
						EntityAttribute* att = attitem->fAttribute;
						EntityAttributeValue* val = erec->getAttributeValue(att, err, fContext);
						if (val != nil)
						{
							EntityCollection* selsub = val->getRelatedSelection();
							EntityModel* emsub = att->GetSubEntityModel();
							if (emsub != nil && selsub != nil)
							{
								uLONG timeout = 2 * 60 * 60 * 1000; // 2 heures
								VString stimeout;
								if (url.GetValue(rest::timeout, stimeout))
								{
									timeout = stimeout.GetLong() * 1000;
								}
								DataSet* newset = new DataSet(emsub, selsub, timeout);
								if (!VDBMgr::GetManager()->AddKeptDataSet(newset))
								{
									newset->Release();
									newset = nil;
								}
								else
								{
									VString uri;
									CalculateURI(uri, newset, emsub, L"", false, false);
									fResponse->AddResponseHeader(L"Content-Location", uri, true);
								}

								if (err == VE_OK)
								{
									if (emsub != nil)
									{
										EntityAttributeSortedSelection atts(emsub);
										EntityAttributeSelection subExpanded(emsub);
										EntityAttributeSelection* subexpand = nil;

										VString ss;
										if (url.GetValue(rest::subExpand, ss))
										{
											if (ss == L"$all")
											{
												emsub->GetAllAttributes(subExpanded, this);
											}
											else
											{
												if (!emsub->BuildListOfAttributes(ss, subExpanded, false, this))
												{
													err = ThrowError(rest::cannot_build_list_of_attribute_for_expand, ss, emsub->GetName());
													SetHTTPError(rest::http_not_found);
												}
											}
											subexpand = &subExpanded;
										}

										if (err == VE_OK)
										{
											sLONG countitem = attitem->fCount;
											if (countitem == -1)
											{
												countitem = emsub->GetDefaultTopSize();
												if (countitem <= 0)
												{
													countitem = kDefaultTopSize;
												}
											}
											if (fToXML)
											{
												VValueBag bag;
												err = SelToBag(bag, emsub, selsub, atts, subexpand, nil, true, attitem->fSkip, countitem, newset, true);
												if (err == VE_OK)
												{
													VString xml;
													SaveBagToXML(bag, L"result", xml, IsPrettyFormatting(), nil, true);
													PutText(xml);
												}
											}
											else
												err = SelToJSON(emsub, selsub, atts, subexpand, nil, true, attitem->fSkip, countitem, newset, true);
										}
									}
								}
								QuickReleaseRefCountable(newset);

							}
							else
								err = ThrowError(rest::subentityset_cannot_be_applied_here);
						}
						else
							err = ThrowError(rest::subentityset_cannot_be_applied_here);
					}
					else
						err = ThrowError(rest::subentityset_cannot_be_applied_here);

					if (err != VE_OK)
						SetHTTPError(rest::http_internal_error);
				}
				else
				{
					if (onepictfield && waitforimagemime)
					{
						EntityAttributeValue* val = erec->getAttributeValue(attributes[0].fAttribute, err, fContext);
						if (val != nil && val->GetKind() == eav_vvalue)
						{
							VValueSingle* pict = val->getVValue();
							if (pict != nil && pict->GetValueKind() == VK_IMAGE)
							{
								if (restmethod == RestMethodeDelete)
								{
									pict->Clear();
									pict->SetNull(true);
									val->Touch(fContext);
									err = erec->Save(erec->GetStamp());
									if (err != VE_OK)
										SetHTTPError(rest::http_internal_error);

								}
								else
								{
									VString mime;
									err = ExtractBestPictureForWeb(*pict, fResponse->GetResponseBody(),imageformatstring, false, mime);
									fResponse->AddResponseHeader(L"Content-Type", mime, true);
									toJSON = false;
								}
							}
						}
					}
					else
					{
						if (restmethod == RestMethodeDelete)
						{
							err = erec->Drop();
							if (err != VE_OK)
								SetHTTPError(rest::http_internal_error);
						}
						else
						{
							if (fToXML)
							{
								VValueBag bag;
								bag.SetString(L"__entityModel", em->GetName());
								err = EntityRecordToBag(bag, erec, attributes, expandattributes, nil, true);
								if (err == VE_OK)
								{
									VString xml;
									SaveBagToXML(bag, L"result", xml, IsPrettyFormatting(), nil, true);
									PutText(xml);
								}
							}
							else
								EntityRecordToJSON(erec, attributes, expandattributes, nil, true);
						}
					}
				}
			}
		}
		else
		{
			if (err == VE_OK)
			{
				err = em->ThrowError(VE_DB4D_CANNOT_LOAD_ENTITY, &fEntityKey);
			}
			SetHTTPError(rest::http_not_found);
		}
		QuickReleaseRefCountable(erec);
	}
	else
	{
	}
}


void RestTools::WorkOnData()
{
	switch (restmethod)
	{
		case RestMethodeRetrieve:
		case RestMethodeDelete:
		case RestMethodeBuildDataSet:
		case RestMethodeBuildSubDataSet:
			if (fEntityKey.IsEmpty())
			{
				RetrieveOrDeleteEntitySel();
			}
			else
			{
				RetrieveOrDeleteOneEntity();
			}
			break;

		case RestMethodeReleaseDataSet:
			{
				if (curdataset != nil)
				{
					VDBMgr::GetManager()->ReleaseKeptDataSet(curdataset->GetID().GetBuffer());
				}
			}
			break;

		case RestMethodeUpdate:
			{
				if (onepictfield && em != nil)
				{
					if (!fEntityKey.IsEmpty())
					{
						fInput->ResetLastError();
						fInput->SetPos(0);
						VString rawPictType;
						if (url.GetValue(rest::rawPict, rawPictType))
						{
							VPictureCodecFactoryRef fact;
							VString mimetype = rawPictType;
							VPicture pict;
							const VPictureCodec* decoder = fact->RetainDecoderForMimeType(mimetype);
							if (decoder != nil)
							{
								sLONG nbbytes = fInput->GetSize();
								sLONG curpos = fInput->GetPos();
								VPictureDataProvider* datasource = VPictureDataProvider::Create(fInput, false, nbbytes, curpos );
								VPictureData* data = fact->CreatePictureData(*datasource);
								if (data != nil)
								{
									pict.FromVPictureData(data);
									data->Release();
								}
								datasource->Release();
								decoder->Release();
							}

							EntityRecord* erec = em->findEntityWithPrimKey(fEntityKey, fContext, err, DB4D_Do_Not_Lock);
							if (erec != nil)
							{
								const EntityAttribute* att = attributes.begin()->fAttribute;
								err = erec->setAttributeValue(att, &pict);
								if (err == VE_OK)
									err = erec->Save(erec->GetModificationStamp());
								erec->Release();
							}
						}
						else
						{
							sLONG nbbytes = fInput->GetSize();
							bool found = false;
							while (nbbytes > 3 && !found)
							{
								nbbytes--;
								uBYTE c = fInput->GetByte();
								if (c == 13)
								{
									nbbytes--;
									uBYTE c = fInput->GetByte();
									if (c == 10)
									{
										nbbytes--;
										uBYTE c = fInput->GetByte();
										if (c == 13)
										{
											nbbytes--;
											uBYTE c = fInput->GetByte();
											if (c == 10)
											{
												found = true;
											}
										}
									}
								}
							}

							if (found)
							{
								VPictureCodecFactoryRef fact;
								VString sPict;
								VString mimetype = "image/jpeg";
								fInputString.GetSubString(1, fInput->GetPos(), sPict);
								sLONG poscontent = sPict.Find("Content-Type:",1,false);
								if (poscontent > 0)
								{
									poscontent += 13;
									sLONG endcontent = sPict.FindUniChar(13, poscontent,false);
									if (endcontent <= 0)
										endcontent = sPict.GetLength();
									sPict.GetSubString(poscontent, endcontent-poscontent, mimetype);
									mimetype.TrimeSpaces();
								}

								VPicture pict;
								const VPictureCodec* decoder = fact->RetainDecoderForMimeType(mimetype);
								if (decoder != nil)
								{
									sLONG nbbytes = fInput->GetSize()-fInput->GetPos();
									sLONG curpos = fInput->GetPos();
									VPictureDataProvider* datasource = VPictureDataProvider::Create(fInput, false, nbbytes, curpos );
									VPictureData* data = fact->CreatePictureData(*datasource);
									if (data != nil)
									{
										pict.FromVPictureData(data);
										data->Release();
									}
									datasource->Release();
									decoder->Release();
								}

								EntityRecord* erec = em->findEntityWithPrimKey(fEntityKey, fContext, err, DB4D_Do_Not_Lock);
								if (erec != nil)
								{
									const EntityAttribute* att = attributes.begin()->fAttribute;
									err = erec->setAttributeValue(att, &pict);
									if (err == VE_OK)
										err = erec->Save(erec->GetModificationStamp());
									erec->Release();
								}
							}
						}
					}
				}
				else
				{
					VString ss;
					if (url.GetValue(rest::refresh, ss))
					{
						if (ss == "true")
							refreshOnly = true;
					}
					if (refreshOnly)
					{
						fContext->StartTransaction();
					}
					err = ImportEntities(em);
					if (refreshOnly)
					{
						fContext->RollBackTransaction();
					}
					if (err != VE_OK)
						SetHTTPError(rest::http_bad_request);
				}
			}
			break;
	}
}


void RestTools::ExecuteStaticRestMethod(const VString& JSNameSpace)
{
	VJSContext jscontext( fJSGlobalContext);
	VJSValue result2(jscontext);
	result2.SetNull();
	JS4D::ExceptionRef excep2 = nil;
	VError err = VE_OK;

	const VString* s = fURL->NextPart();

	if (s != nil)
	{
		VString lasuite;
		VectorOfVString methodParams;
		sLONG p1 = s->FindUniChar('(');
		if (p1 <= 0)
		{
			lasuite = *s;
		}
		else
		{
			s->GetSubString(1, p1-1, lasuite);
			VString params;
			Wordizer ww(*s);
			ww.SetCurPos(p1);
			ww.GetNextWord(params, ')', false, true);

			Wordizer ww2(params);
			ww2.ExctractStrings(methodParams, true, ',', true);
		}

		VString JsonMethodParams;

		if (wasapost || wasaput)
		{
			{
				if (methodParams.empty())
				{
					JsonMethodParams = GetInputString();
				}
			}

		}
		else
		{
			url.GetValue(rest::params, JsonMethodParams);
		}

		VJSValue result(jscontext);
		VString methodName = lasuite;
		VString funcName = JSNameSpace+"."+methodName;

		jscontext.EvaluateScript(funcName, nil, &result, nil, nil);

		if (result.IsObject())
		{
			VJSObject funcobj(result.GetObject());
			if (funcobj.IsFunction())
			{
				VString scope;
				funcobj.GetPropertyAsString("scope", nil, scope);
				if (scope != "private")
				{
					if (JsonMethodParams.IsEmpty() && !methodParams.empty())
					{
						JsonMethodParams = L"[";
						bool first = true;
						for (VectorOfVString::const_iterator cur = methodParams.begin(), end = methodParams.end(); cur != end; cur++)
						{
							if (first)
								first = false;
							else
								JsonMethodParams += L",";
							VString xjs;
							cur->GetJSONString(xjs, JSON_FormatDateIso | JSON_WithQuotesIfNecessary);
							JsonMethodParams += xjs;
						}
						JsonMethodParams += L"]";
					}

					VJSJSON json(jscontext);
					JS4D::ExceptionRef excep = nil;
					VJSValue params(jscontext);
					if (!JsonMethodParams.IsEmpty())
					{
						params = json.Parse(JsonMethodParams, &excep);
					}
					if (excep == nil)
					{
						vector<VJSValue> paramsValues;
						if (!JsonMethodParams.IsEmpty())
						{
							if (params.IsInstanceOf( CVSTR( "Array")))
							{
								VJSObject objarray(jscontext);
								params.GetObject(objarray);
								VJSArray xarray(objarray);
								for (sLONG i = 0, nb = xarray.GetLength(); i < nb; i++)
								{
									paramsValues.push_back(xarray.GetValueAt(i));
								}
							}
							else
							{
								paramsValues.push_back(params);
							}
						}

						jscontext.GetGlobalObject().CallFunction(funcobj, &paramsValues, &result2, &excep2);
					}
					else
						excep2 = excep;
				}
			}
		}
	}

	ReturnJSResult(err, result2, excep2, nil);

	/*
	if (excep2 != nil || err != VE_OK)
	{
		PutJsonBeginObject();

		VValueBag errbag;
		BuildErrorStack(errbag);
		dejaerror = true;
		VBagArray* errors = errbag.GetElements(d4::__ERROR);
		if (errors != nil)
		{
			VString errjson;
			sLONG level = 0;
			errors->_GetJSONString(errjson, level, false, JSON_Default);
			PutJsonPropertyName(L"__ERROR");
			PutText(errjson);			
		}
		else
		{
			VJSJSON JSON(jscontext);
			VString outJson;
			VJSValue excepval(jscontext, excep2);
			JSON.Stringify(excepval, outJson, nil);
			if (outJson == L"undefined")
				outJson = L"null";
			PutJsonPropertyName(L"__ERROR");
			PutText(outJson);
		}
		PutJsonEndObject();
	}
	else
	{
		PutJsonBeginObject();

		PutJsonPropertyName(L"result");
		VJSJSON JSON(jscontext);
		VString outJson;
		JSON.Stringify(result2, outJson, nil);
		if (outJson == L"undefined")
			outJson = L"null";
		PutText(outJson);

		PutJsonEndObject();
	}
	*/
}



void RestTools::ExecuteRequest()
{
	VString ss;
#if VERSIONDEBUG
	SetPrettyFormatting(true);
#endif

	if (labase != nil)
		labase->BuildAutoModelCatalog(fContext);

	err = VE_OK;
	InitUAG();
	wasapost = fResponse->GetRequestMethod() == HTTP_POST;
	wasaput = fResponse->GetRequestMethod() == HTTP_PUT;
	wasadelete = fResponse->GetRequestMethod() == HTTP_DELETE;

	s = url.NextPart();
	if (s != nil && *s == L"rest" && url.IsValid())
	{
		SetRequestFormat();
		SetVerb();

		if (s != nil && err == VE_OK)
		{
			if (*s == L"$file")
			{
				GetWAFFiles();
			}
			else if (*s == L"$info")
			{
				GetServerAndBaseInfo();
			}
			else if (*s == L"$directory")
			{
				ExecuteStaticRestMethod("RestDirectoryAccess");
			}
			else if (*s == L"$impexp")
			{
				ExecuteStaticRestMethod("RestImpExpAccess");
			}
			/*
			else if (*s == L"$database" || *s == L"$relationalModel")
			{
				getDef = true;
				getdatabasedef = true;
			}
			*/
			else if (*s == L"$service" || *s == L"$catalog")
			{
				AnalyseGetCatalog();
			}
			else
			{
				AnalyseGetModelAndEntities();
			}

			if (em != nil || getDef)
			{
				attributes.SetModel(em);
				s = url.NextPart();

				AnalyseGetAttributes();
				AnalyseGetExpandAttributes();
				AnalyseGetOtherStuff();

				if (err == VE_OK)
				{
					if (getDef)
					{
						WorkOnCatalog();
					}
					else
					{
						WorkOnData();
					}
				}

				if (expandattributes != nil)
					delete expandattributes;
			}

			QuickReleaseRefCountable(curdataset);
			QuickReleaseRefCountable(em);

			for (vector<CDB4DEntityModel*>::iterator cur = allEMs.begin(), end = allEMs.end(); cur != end; cur++)
				(*cur)->Release();
		}


		if (fToXML)
		{
			fResponse->AddResponseHeader(L"Content-Type", L"text/xml; charset=UTF-8", true);
			toJSON = false;
		}

		if (toJSON)
			fResponse->AddResponseHeader(L"Content-Type", L"application/json; charset=UTF-8", true);

	}
	else
	{
		err = ThrowError(rest::url_is_malformed);
		SetHTTPError(rest::http_bad_request);
	}

	if (err != VE_OK)
	{
		if (fContext->PermErrorHappened())
			SetHTTPError(rest::http_perm_error);
		//if (GetHTTPError() != 401)
			GenereErrorReport();
		fResponse->SetResponseStatusCode((HTTPStatusCode)GetHTTPError());
	}
	else
	{
		GenerateInfoIfEmpty();
		if (GetHTTPError() == 0)
			fResponse->SetResponseStatusCode(HTTP_OK);
		else
			fResponse->SetResponseStatusCode((HTTPStatusCode)GetHTTPError());
	}


}



VError RestRequestHandler::HandleRequest( IHTTPResponse* inResponse)
{
	/*
	inRequest.GetAnswerStream()->SetCharSet(VTC_UTF_8, eCRM_NATIVE);
	inRequest.GetAnswerStream()->OpenWriting();
	inRequest.GetRequestStream()->OpenReading();
	*/

	DB4DJSRuntimeDelegate* xJSDelegate = nil;
	VJSGlobalContext* xJSContext = nil;


#if UseDB4DJSContext
	// temporary code

	CDB4DBaseContext* xcontext;
	BaseTaskInfo* context;
	tempContextStore tempStore;
	bool mustReleaseContext;

	if (getTempContextStore(tempStore))
	{
		mustReleaseContext = false;
		xcontext = tempStore.xcontext;
		context = ConvertContext(xcontext);
		xJSDelegate = tempStore.xJSDelegate;
		xJSContext = tempStore.xJSContext;
	}
	else
	{
		xcontext = fxBase->NewContext(nil, nil);
		context = ConvertContext(xcontext);
		xJSDelegate = new DB4DJSRuntimeDelegate(context);
		xJSContext = VJSGlobalContext::Create( xJSDelegate);
		CDB4DManager* db4D = XBOX::VComponentManager::RetainComponentOfType<CDB4DManager>();
		if (db4D != nil)
		{
			context->SetJSContext(xJSContext);
			db4D->InitializeJSGlobalObject(xJSContext, xcontext);
			VJSContext jjcontext(*xJSContext);
			VFile* fileJs = context->GetBase()->RetainCatalogJSFile();
			if (fileJs != nil)
			{
				VJSValue result(jjcontext);
				jjcontext.EvaluateScript(fileJs, &result, nil, nil);
				fileJs->Release();
			}
			db4D->Release();
		}
		tempStore.xcontext = xcontext;
		tempStore.xJSContext = xJSContext;
		tempStore.xJSDelegate = xJSDelegate;

		if (!addTempContextStore(tempStore))
			mustReleaseContext = true;
		else
			mustReleaseContext = false;
	}

	// end of temporary code
#else

	VError err = VE_OK;
	BaseTaskInfo *context = NULL;
	
	IDB4D_ApplicationIntf *applicationIntf = fApplicationRef == nil ? nil : VDBMgr::GetManager()->GetApplicationInterface();

	// code sale en attendant quelque chose de Stephane
	//global_ApplicationRef = fApplicationRef;

	VUUID userID;
	userID.SetNull(true);
	CDB4DBaseContext *baseContext = nil;

	VJSGlobalContext *globalJSContext = nil;

	if (fApplicationRef != nil)
	{
		globalJSContext = applicationIntf->RetainJSContext( fApplicationRef, err, true, &inResponse->GetRequest());
		if (globalJSContext != NULL &&  err == VE_OK)
		{
			VJSContext jsContext( globalJSContext);
			baseContext = ::GetDB4DBaseContextFromJSContext( jsContext, fxBase);
		}
	}
	else if (fxBase != nil)
	{
		baseContext = fxBase->NewContext(nil, nil);
		BaseTaskInfo* context = ConvertContext(baseContext);
		xJSDelegate = new DB4DJSRuntimeDelegate(context);
		globalJSContext = VJSGlobalContext::Create( xJSDelegate);
		context->SetJSContext(globalJSContext);
	}

	if (baseContext != NULL)
	{
		context = ConvertContext( baseContext);

		/*
		CUAGSession* session = inResponse->GetRequest().GetAuthenticationInfos()->GetUAGSession();
		//if (session != nil)
		{
			if (applicationIntf != nil)
				applicationIntf->SetUAGSession(globalJSContext, session);
			baseContext->SetCurrentUser(userID, session);
		}
		*/
	}


#endif

	if (globalJSContext != NULL)
	{
		RestTools req(context, &(inResponse->GetRequest().GetRequestBody()), &(inResponse->GetResponseBody()), inResponse->GetRequest().GetHost(), inResponse->GetRequestURL(), inResponse, globalJSContext, fApplicationRef);

		if (fApplicationRef == nil)
			req.AllowAutoModel();

		req.ExecuteRequest();
	}

#if UseDB4DJSContext
	// temporary code
	if (mustReleaseContext)
	{
		xcontext->FreeAllJSFuncs();
		xcontext->Release();

		QuickReleaseRefCountable(xJSContext);
		delete xJSDelegate;
	}
	else
		releaseTempStore(tempStore);
	// end of temporary code
#else

	if (baseContext != nil)
		baseContext->SetCurrentUser(userID, nil);
	if (fApplicationRef != nil)
	{
		applicationIntf->ReleaseJSContext( fApplicationRef, globalJSContext, inResponse);
	}
	else if (baseContext != nil)
	{
		baseContext->SetJSContext(NULL);
		baseContext->Release();
		QuickReleaseRefCountable(globalJSContext);
		delete xJSDelegate;
	}

#endif

	/*
	QuickReleaseRefCountable(xUAGBase);
	QuickReleaseRefCountable(xUAGcontext);
	*/

	return VE_OK;
}


bool RestRequestHandler::GetEnable()
{
	VTaskLock lock( &fMutex);
	return fEnable;
}


void RestRequestHandler::SetEnable( bool inEnable)
{
	VTaskLock lock( &fMutex);
	fEnable = inEnable;
}




//----- old code from here -----



VErrorDB4D VDB4DBase::ExecuteRestRequest(tempDB4DWebRequest& inRequest)
{
	VError err = VE_OK;
	return err;
}

