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

namespace rest
{
	CREATE_BAGKEY_2(top, "$top");
	CREATE_BAGKEY_2(expand, "$expand");
	CREATE_BAGKEY_2(subExpand, "$subExpand");
	CREATE_BAGKEY_2(all, "$all");
	CREATE_BAGKEY_2(format, "$format");
	CREATE_BAGKEY_2(imageformat, "$imageformat");
	CREATE_BAGKEY_2(binary, "$binary");
	CREATE_BAGKEY_2(prettyformatting, "$prettyformatting");
	CREATE_BAGKEY_2(prettyprint, "$prettyprint");
	CREATE_BAGKEY_2(method, "$method");
	CREATE_BAGKEY_2(atOnce, "$atOnce");
	CREATE_BAGKEY_2(atomic, "$atomic");
	CREATE_BAGKEY_2(metadata, "$metadata");
	//	CREATE_BAGKEY_2(max, "$max");
	CREATE_BAGKEY_2(limit, "$limit");
	CREATE_BAGKEY_2(skip, "$skip");
	CREATE_BAGKEY_2(filter, "$filter");
	CREATE_BAGKEY_2(params, "$params");
	CREATE_BAGKEY_2(savedfilter, "$savedfilter");
	CREATE_BAGKEY_2(querypath, "$querypath");
	CREATE_BAGKEY_2(queryplan, "$queryplan");
	CREATE_BAGKEY_2(orderby, "$orderby");
	CREATE_BAGKEY_2(subOrderby, "$subOrderby");
	CREATE_BAGKEY_2(savedorderby, "$savedorderby");
	CREATE_BAGKEY_2(timeout, "$timeout");
	CREATE_BAGKEY_2(changestamp, "$changestamp");
	CREATE_BAGKEY_2(imageinfo, "$imageinfo");
	CREATE_BAGKEY_2(emMethod, "$emMethod");
	CREATE_BAGKEY_2(attributes, "$attributes");
	CREATE_BAGKEY_2(kind, "$kind");
	CREATE_BAGKEY_2(path, "$path");
	CREATE_BAGKEY_2(distinct, "$distinct");
	CREATE_BAGKEY_2(refresh, "$refresh");
	CREATE_BAGKEY_2(progressinfo, "$progressinfo");
	CREATE_BAGKEY_2(stop, "$stop");
	CREATE_BAGKEY_2(asArray, "$asArray");
	CREATE_BAGKEY_2(noKey, "$noKey");
	CREATE_BAGKEY_2(retainPositions, "$retainPositions");
	CREATE_BAGKEY_2(removeFromSet, "$removeFromSet");
	CREATE_BAGKEY_2(removeRefOnly, "$removeRefOnly");
	CREATE_BAGKEY_2(addToSet, "$addToSet");
	CREATE_BAGKEY_2(queryLimit, "$queryLimit");
	CREATE_BAGKEY_2(fromSel, "$fromSel");
	CREATE_BAGKEY_2(keepSel, "$keepSel");
	CREATE_BAGKEY_2(rawPict, "$rawPict");
	CREATE_BAGKEY_2(findKey, "$findKey");
	CREATE_BAGKEY_2(compute, "$compute");
	CREATE_BAGKEY_2(logicOperator, "$logicOperator");
	CREATE_BAGKEY_2(otherCollection, "$otherCollection");
	CREATE_BAGKEY_2(reselect, "$reselect");

};




// --------------------------------------------------- 

#if 0

VError RestTools::StringToJSON(const VString& inS, VString& outS)
{
	sLONG len = inS.GetLength();
	UniChar* dest = outS.GetCPointerForWrite(len*2+4);
	const UniChar* source = inS.GetCPointer();
	sLONG len2 = 0;
	for (sLONG i = 0; i < len; i++)
	{
		UniChar c = *source;
		source++;
		switch (c)
		{

			case '"':
				*dest = '\\';
				dest++;
				*dest = '"';
				dest++;
				len2+=2;
				break;

			case '/':
				*dest = '\\';
				dest++;
				*dest = '/';
				dest++;
				len2+=2;
				break;

			case '\\':
				*dest = '\\';
				dest++;
				*dest = '\\';
				dest++;
				len2+=2;
				break;

			case 9:
				*dest = '\\';
				dest++;
				*dest = 't';
				dest++;
				len2+=2;
				break;

			case 13:
				*dest = '\\';
				dest++;
				*dest = 'r';
				dest++;
				len2+=2;
				break;

			case 10:
				*dest = '\\';
				dest++;
				*dest = 'n';
				dest++;
				len2+=2;
				break;

			case 12:
				*dest = '\\';
				dest++;
				*dest = 'f';
				dest++;
				len2+=2;
				break;

			case 8:
				*dest = '\\';
				dest++;
				*dest = 'b';
				dest++;
				len2+=2;
				break;

			default:
				*dest = c;
				dest++;
				len2++;
				break;
		}

	}
	outS.Validate(len2);
	return VE_OK;
}


VError RestTools::JSONtoString(const VString& inS, VString& outS)
{
	sLONG len = inS.GetLength();
	UniChar* dest = outS.GetCPointerForWrite(len);
	const UniChar* source = inS.GetCPointer();
	sLONG len2 = 0;
	for (sLONG i = 0; i < len; i++)
	{
		UniChar c = *source;
		source++;
		if (c == '\\')
		{
			c = *source;
			source++;
			i++;
			if (i < len)
			{
				switch (c)
				{
					case '\\':
					case '"':
					case '/':
						//  c is the same
						break;

					case 't':
						c = 9;
						break;

					case 'r':
						c = 13;
						break;

					case 'n':
						c = 10;
						break;

					case 'b':
						c = 8;
						break;

					case 'f':
						c = 12;
						break;
				}
				*dest = c;
				dest++;
				len2++;
			}
		}
		else
		{
			*dest = c;
			dest++;
			len2++;
		}
	}
	outS.Validate(len2);
	return VE_OK;
}

#endif


bool RestTools::sIsInited = false;
VValueBag RestTools::sAllRestKeywords;


void RestTools::DeInit()
{
	QuickReleaseRefCountable(fUserSession);
	QuickReleaseRefCountable(fQueryPlan);
	QuickReleaseRefCountable(fQueryPath);
	fInput->CloseReading();
	fOutput->CloseWriting();
	QuickReleaseRefCountable(fProgressIndicator);
	if (newwafkeepselptr != nil)
		delete newwafkeepselptr;
	QuickReleaseRefCountable(fUAGDirectory);
	QuickReleaseRefCountable(selectedEntities);
	QuickReleaseRefCountable(fRemoteClientRequestInfo);
	QuickReleaseRefCountable(previousCollection);
}


void RestTools::_staticInit()
{
	if (!sIsInited)
	{
		sIsInited = true;
		sAllRestKeywords.SetBool(rest::top, true);
		sAllRestKeywords.SetBool(rest::expand, true);
		sAllRestKeywords.SetBool(rest::format, true);
		sAllRestKeywords.SetBool(rest::imageformat, true);
		sAllRestKeywords.SetBool(rest::prettyformatting, true);
		sAllRestKeywords.SetBool(rest::prettyprint, true);
		sAllRestKeywords.SetBool(rest::method, true);
		sAllRestKeywords.SetBool(rest::atOnce, true);
		sAllRestKeywords.SetBool(rest::atomic, true);
		sAllRestKeywords.SetBool(rest::metadata, true);
		sAllRestKeywords.SetBool(rest::limit, true);
		sAllRestKeywords.SetBool(rest::skip, true);
		sAllRestKeywords.SetBool(rest::filter, true);
		sAllRestKeywords.SetBool(rest::querypath, true);
		sAllRestKeywords.SetBool(rest::queryplan, true);
		sAllRestKeywords.SetBool(rest::orderby, true);
		sAllRestKeywords.SetBool(rest::timeout, true);
		sAllRestKeywords.SetBool(rest::changestamp, true);
		sAllRestKeywords.SetBool(rest::imageinfo, true);
	}
}


UniChar RestTools::GetNextChar(bool& eof)
{
	UniChar result = 0;
	if (fCurChar-fStartChar >= fInputLen)
		eof = true;
	else
	{
		eof = false;
		result = *fCurChar;
		fCurChar++;
	}
	return result;
}


RestTools::JsonToken RestTools::GetNextJsonToken(VString& outString, bool* withquotes)
{
	outString.Clear();
	bool escaped = false, eof;
	if (withquotes != nil)
		*withquotes = false;
	UniChar c;

	do 
	{
		c = GetNextChar(eof);
		if (c > 32)
		{
			switch (c)
			{
				case '{':
					return jsonBeginObject;
					break;

				case '}':
					return jsonEndObject;
					break;

				case '[':
					return jsonBeginArray;
					break;

				case ']':
					return jsonEndArray;
					break;

				case ',':
					return jsonSeparator;
					break;

				case ':':
					return jsonAssigne;
					break;

				case '"':
					{
						if (withquotes != nil)
							*withquotes = true;
						do 
						{
							c = GetNextChar(eof);
							if (c != 0)
							{
								if (c == '\\')
								{
									c = GetNextChar(eof);
									switch(c)
									{
										case '\\':
										case '"':
										case '/':
											//  c is the same
											break;

										case 't':
											c = 9;
											break;

										case 'r':
											c = 13;
											break;

										case 'n':
											c = 10;
											break;

										case 'b':
											c = 8;
											break;

										case 'f':
											c = 12;
											break;
									}
									if (c != 0)
										outString.AppendUniChar(c);
								}
								else if (c == '"')
								{
									return jsonString;
								}
								else
								{
									outString.AppendUniChar(c);
								}
							}

							
						} while(!eof);
						return jsonString;
					}
					break;

				default:
					{
						outString.AppendUniChar(c);
						do 
						{
							const UniChar* oldpos = fCurChar;
							c = GetNextChar(eof);
							if (c <= 32)
							{
								return jsonString;
							}
							else
							{
								switch(c)
								{
									case '{':
									case '}':
									case '[':
									case ']':
									case ',':
									case ':':
									case '"':
										fCurChar = oldpos;
										return jsonString;
										break;

									default:
										outString.AppendUniChar(c);
										break;
								}
							}
						} while(!eof);
						return jsonString;
					}
					break;
			}
		}
	} while(!eof);
	
	return jsonNone;
}


VError RestTools::PutText(const VString& inText)
{
	VIndex lenToAdd = inText.GetLength();
	VIndex actualLen = fOutputBuffer.GetEnsuredSize();
	VIndex len = fOutputBuffer.GetLength();
	if ((lenToAdd + len) > actualLen)
	{
		VIndex newLen = actualLen * 2;
		if (newLen < (lenToAdd + len))
			newLen = newLen + (lenToAdd * 2);
		fOutputBuffer.EnsureSize(newLen);
	}
	fOutputBuffer.AppendString(inText);
	return VE_OK;
}

void RestTools::IncrementLevel()
{
	fLevelHasChanged = true;
	fCurlevel++;
}

void RestTools::DecrementLevel()
{
	fLevelHasChanged = true;
	fCurlevel--;
}

VError RestTools::PutTabLevel()
{
	if (fPrettyFormatting)
	{
		if (fLevelHasChanged)
		{
			fLevelHasChanged = false;
			fTabString.Clear();
			for (sLONG i = 0; i < fCurlevel; i++)
				fTabString.AppendUniChar(9);
		}
		return PutText(fTabString);
	}
	else
		return VE_OK;
}



const VString RestTools::kCR("\n");
const VString RestTools::kQuote("\"");
const VString RestTools::kColon(":");
const VString RestTools::kBeginObject("{");
const VString RestTools::kEndObject("}");
const VString RestTools::kBeginArray("[");
const VString RestTools::kEndArray("]");
const VString RestTools::kSeparator(",");
const VString RestTools::kPropNull("null");

const VString RestTools::kPropDeferred("__deferred");
const VString RestTools::kPropKey("__KEY");
const VString RestTools::kPropImage("image");
const VString RestTools::kPropQueryPath("__queryPath");
const VString RestTools::kPropQueryPlan("__queryPlan");
const VString RestTools::kPropEntityModel("__entityModel");
const VString RestTools::kPropError("__ERROR");
const VString RestTools::kPropStamp("__STAMP");
const VString RestTools::kPropTransformedSelection("__transformedSelection");
const VString RestTools::kPropENTITYSET("__ENTITYSET");
const VString RestTools::kPropCount("__COUNT");
const VString RestTools::kPropSent("__SENT");
const VString RestTools::kPropFirst("__FIRST");
const VString RestTools::kPropEntities("__ENTITIES");
const VString RestTools::kPropResult("result");

VError RestTools::NewLine()
{
	if (fPrettyFormatting)
	{
		return PutText(kCR);
	}
	else
		return VE_OK;

}


VError RestTools::PutJsonPropertyName(const VString& inPropName)
{
	VError err = VE_OK;
	PutTabLevel();
	if (!microsoft)
		err = PutText(kQuote);

	PutText(inPropName);

	if (!microsoft)
		err = PutText(kQuote);

	if (fPrettyFormatting)
		PutText(kColon);
	else
		PutText(kColon);

	return err;
}


VError RestTools::PutJsonBeginObject(bool newlinefirst)
{
	if (newlinefirst)
	{
		NewLine();
		PutTabLevel();
	}
	PutText(kBeginObject);
	NewLine();
	IncrementLevel();
	return VE_OK;
}

VError RestTools::PutJsonEndObject()
{
	NewLine();
	DecrementLevel();
	PutTabLevel();
	PutText(kEndObject);
	return VE_OK;
}


VError RestTools::PutJsonBeginArray(bool newlinefirst)
{
	if (newlinefirst)
	{
		NewLine();
		PutTabLevel();
	}
	PutText(kBeginArray);
	NewLine();
	IncrementLevel();
	return VE_OK;
}


VError RestTools::PutJsonEndArray()
{
	NewLine();
	DecrementLevel();
	PutTabLevel();
	PutText(kEndArray);
	return VE_OK;
}


VError RestTools::PutJsonSeparator()
{
	return PutText(kSeparator);
}


VError RestTools::AddFormatToURI(VString& outURI, bool dejaargument)
{
	if (fToXML)
	{
		if (dejaargument)
			outURI += L"&$format=xml";
		else
			outURI += L"?$format=xml";
	}
	return VE_OK;
}



VError RestTools::CalculateDataURI(VString& outURI, const EntityModel* em, const VString& additionnalPart, bool WithUri, bool withquotes)
{
	outURI.Clear();
	if (WithUri)
	{
		if (microsoft)
			outURI = L"dataURI:";
		else
			outURI = L"\"dataURI\":";
	}

	/*
	if (withquotes)
		outURI += L"\"http://";
	else
		outURI += L"http://";

	outURI += fHostName;
	*/

	if (withquotes)
		outURI.AppendUniChar('"');

	outURI += L"/"+VDBMgr::GetManager()->GetRestPrefix()+"/";

	VString entityname;
	em->GetName(entityname);
	outURI += entityname;
	outURI += additionnalPart;
	AddFormatToURI(outURI, !additionnalPart.IsEmpty());

	if (withquotes)
		outURI.AppendUniChar('"');
	return VE_OK;
}



VError RestTools::CalculateURI(VString& outURI, const EntityModel* em, const VString& additionnalPart, bool WithUri, bool withquotes)
{
	outURI.Clear();
	if (WithUri)
	{
		if (microsoft)
			outURI = L"uri:";
		else
			outURI = L"\"uri\":";
	}

	/*
	if (withquotes)
		outURI += L"\"http://";
	else
		outURI += L"http://";

	outURI += fHostName;
	*/

	if (withquotes)
		outURI.AppendUniChar('"');

	outURI += L"/"+VDBMgr::GetManager()->GetRestPrefix()+"/$catalog/";

	VString entityname;
	em->GetName(entityname);
	outURI += entityname;
	outURI += additionnalPart;
	AddFormatToURI(outURI, !additionnalPart.IsEmpty());

	if (withquotes)
		outURI.AppendUniChar('"');
	return VE_OK;
}



VError RestTools::CalculateURI(VString& outURI, EntityRecord* erec, const EntityAttribute* attribute, const VString& additionnalPart, bool WithUri, bool withquotes, bool withformat, bool forJSON)
{
	outURI.Clear();
	if (WithUri)
	{
		if (microsoft)
			outURI = L"uri:";
		else
			outURI = L"\"uri\":";
	}

	if (withquotes)
		outURI.AppendUniChar('"');

	outURI += L"/"+VDBMgr::GetManager()->GetRestPrefix()+"/";

	VString entityname;
	erec->GetOwner()->GetName(entityname);
	outURI += entityname;
	outURI.AppendUniChar('(');
	VString skey;
	erec->GetPrimKeyValue(skey, true);
	if (forJSON)
	{
		VString skeyj;
		skey.GetJSONString(skeyj);
		outURI += skeyj;
	}
	else
		outURI += skey;
	outURI.AppendUniChar(')');
	if (attribute != nil)
	{
		outURI.AppendUniChar('/');
		VString fieldname;
		attribute->GetName(fieldname);
		outURI += fieldname;
	}
	outURI += additionnalPart;
	bool dejaargument = !additionnalPart.IsEmpty();
	if (attribute != nil)
	{
		dejaargument = true;
		if (additionnalPart.IsEmpty())
			outURI+=L"?$expand=";
		else
			outURI+=L"&$expand=";
		outURI += attribute->GetName();
	}
	if (withformat)
		AddFormatToURI(outURI, dejaargument);

	if (withquotes)
		outURI.AppendUniChar('"');
	return VE_OK;
}



VError RestTools::CalculateURI(VString& outURI, const EntityModel* em, const EntityAttribute* attribute, const VString& additionnalPart, bool WithUri, bool withquotes, bool withformat, bool forJSON, const VString& skey)
{
	outURI.Clear();
	if (WithUri)
	{
		if (microsoft)
			outURI = L"uri:";
		else
			outURI = L"\"uri\":";
	}

	if (withquotes)
		outURI.AppendUniChar('"');

	outURI += L"/" + VDBMgr::GetManager()->GetRestPrefix() + "/";

	VString entityname;
	em->GetName(entityname);
	outURI += entityname;
	outURI.AppendUniChar('(');
	if (forJSON)
	{
		VString skeyj;
		skey.GetJSONString(skeyj);
		outURI += skeyj;
	}
	else
		outURI += skey;
	outURI.AppendUniChar(')');
	if (attribute != nil)
	{
		outURI.AppendUniChar('/');
		VString fieldname;
		attribute->GetName(fieldname);
		outURI += fieldname;
	}
	outURI += additionnalPart;
	bool dejaargument = !additionnalPart.IsEmpty();
	if (attribute != nil)
	{
		dejaargument = true;
		if (additionnalPart.IsEmpty())
			outURI += L"?$expand=";
		else
			outURI += L"&$expand=";
		outURI += attribute->GetName();
	}
	if (withformat)
		AddFormatToURI(outURI, dejaargument);

	if (withquotes)
		outURI.AppendUniChar('"');
	return VE_OK;
}


VError RestTools::CalculateURI(VString& outURI, DataSet* inDataSet, const EntityModel* em, const VString& additionnalPart, bool WithUri, bool withquotes)
{
	outURI.Clear();
	if (WithUri)
	{
		if (microsoft)
			outURI = L"uri:";
		else
			outURI = L"\"uri\":";
	}

	/*
	if (withquotes)
		outURI += L"\"http://";
	else
		outURI += L"http://";

	outURI += fHostName;
	*/

	if (withquotes)
		outURI.AppendUniChar('"');
	outURI += L"/"+VDBMgr::GetManager()->GetRestPrefix()+"/";

	VString tablename;
	em->GetName(tablename);
	outURI += tablename;
	outURI.AppendUniChar('/');

	VString datasetname;
	inDataSet->GetID().GetString(datasetname);

	outURI += L"$entityset/";
	outURI += datasetname;

	outURI = outURI + additionnalPart;
	AddFormatToURI(outURI, !additionnalPart.IsEmpty());
	if (withquotes)
		outURI.AppendUniChar('"');
	return VE_OK;
}


VError RestTools::PutJsonPropertyString(const VString& inPropVal)
{
	VString s2;
	inPropVal.GetJSONString(s2, JSON_WithQuotesIfNecessary);
	PutText(s2);
	return VE_OK;
}


VError RestTools::PutJsonPropertyLong(sLONG inPropVal)
{
	VString s2;
	s2.FromLong(inPropVal);
	return PutText(s2);
}


VError RestTools::PutJsonPropertyLong8(sLONG8 inPropVal)
{
	VString s2;
	s2.FromLong8(inPropVal);
	return PutText(s2);
}


VError RestTools::PutJsonPropertyValue(VValueSingle& inPropVal)
{
	if (inPropVal.IsNull())
	{
		PutText(kPropNull);
	}
	else
	{
		VStr255 s2;

		inPropVal.GetJSONString(s2, JSON_WithQuotesIfNecessary);
		PutText(s2);
	}
	return VE_OK;
}


VError RestTools::PutJsonProperty(const VString& inPropName, sLONG inPropVal, bool withSeparator)
{
	PutJsonPropertyName(inPropName);
	PutJsonPropertyLong(inPropVal);
	if (withSeparator)
		PutJsonSeparator();
	NewLine();
	return VE_OK;
}


VError RestTools::PutJsonPropertybool(const VString& inPropName, bool inPropVal, bool withSeparator)
{
	PutJsonPropertyName(inPropName);
	VBoolean vb(inPropVal);
	VString s;
	vb.GetJSONString(s);
	PutText(s);
	if (withSeparator)
		PutJsonSeparator();
	NewLine();
	return VE_OK;
}


VError RestTools::PutJsonProperty(const VString& inPropName, const VString& inPropVal, bool withSeparator)
{
	PutJsonPropertyName(inPropName);
	PutJsonPropertyString(inPropVal);
	if (withSeparator)
		PutJsonSeparator();
	NewLine();
	return VE_OK;
}


VError RestTools::PutJsonProperty(const VString& inPropName, const VValueBag* bag, bool withSeparator)
{
	PutJsonPropertyName(inPropName);
	VString ss;
	bag->GetJSONString(ss, fPrettyFormatting ? JSON_PrettyFormatting : JSON_Default);
	PutText(ss);
	if (withSeparator)
		PutJsonSeparator();
	NewLine();
	return VE_OK;
}


VError RestTools::PutDeferred(const VString& uri, bool ispict, VValueSingle* cv, VString* skey)
{
	PutJsonBeginObject(false);
	PutJsonPropertyName(kPropDeferred);
	PutJsonBeginObject(false);

	PutTabLevel();
	PutText(uri);
	if (skey != nil)
	{
		PutJsonSeparator();
		PutJsonProperty(kPropKey, *skey, false);
	}
	if (ispict)
	{
		PutJsonSeparator();
		NewLine();
		if (cv != nil)
		{
			VString mediatype;
			sLONG pictWidth = 0;
			sLONG pictHeight = 0;
			GetVPictureInfo(*cv, mediatype, &pictWidth, &pictHeight);
			PutJsonProperty(L"mediatype", mediatype, true);
			PutJsonProperty(L"height", pictHeight, true);
			PutJsonProperty(L"width", pictWidth, true);
		}
		PutJsonPropertybool(kPropImage, true, false);
	}
	PutJsonEndObject();
	NewLine();
	PutJsonEndObject();
	return VE_OK;
}


VValueBag* RestTools::PutDeferred(VError& outErr, const VString& uri, bool ispict, VValueSingle* cv, VString* skey)
{
	outErr = VE_OK;
	VValueBag* bag = new VValueBag();
	VValueBag* bag2 = new VValueBag();
	if (!fToXML)
	{
		bag->SetBool(____objectunic, true);
		bag2->SetBool(____objectunic, true);
	}
	bag2->SetString(d4::uri, uri);
	if (skey != nil)
		bag2->SetString(d4::__KEY, *skey);
	if (ispict)
	{
		if (cv != nil)
		{
			VString mediatype;
			sLONG pictWidth = 0;
			sLONG pictHeight = 0;
			GetVPictureInfo(*cv, mediatype, &pictWidth, &pictHeight);
			bag2->SetString(d4::mediatype, mediatype);
			bag2->SetLong(d4::height, pictHeight);
			bag2->SetLong(d4::width, pictWidth);
		}
		bag2->SetBool(d4::image,true);
	}
	bag->AddElement(d4::__deferred, bag2);
	bag2->Release();
	return bag;
}




VError RestTools::PutJsonQueryPlan()
{
	if (fWithQueryPath && fQueryPath != nil)
		PutJsonProperty(L"__queryPath", fQueryPath, true);
	if (fWithQueryPlan && fQueryPlan != nil)
		PutJsonProperty(L"__queryPlan", fQueryPlan, true);
	return VE_OK;
}


VError RestTools::EntityRecordToJSON(EntityRecord* erec, EntityAttributeSortedSelection& whichAttributes, EntityAttributeSelection* expandAttributes, EntityAttributeSortedSelection* sortingAttributes, bool withheader, bool nofields, sLONG* errToSend)
{
	VError err = VE_OK;
	VStream* output = fOutput;
	EntityModel* em = erec->GetOwner();

	if (whichAttributes.empty() && !nofields)
	{
		if (!em->GetAllSortedAttributes(whichAttributes, this))
			err = ThrowBaseError(memfull);
	}

	if (err == VE_OK)
	{
		PutJsonBeginObject();

		if (microsoft)
		{
			if (withheader)
			{
				PutTabLevel();
				output->PutText(L"\"d\":");
				PutJsonBeginObject();
			}

			PutJsonPropertyName(L"__metadata");
			PutJsonBeginObject(false);
			VString uri;
			CalculateURI(uri, erec, nil, L"", true, true, true, true);
			PutTabLevel();
			PutText(uri);
			PutJsonSeparator();
			NewLine();
			PutJsonPropertyName(L"type");

			VString entityname;
			erec->GetOwner()->GetName(entityname);
			PutJsonPropertyString(entityname);
			PutJsonEndObject();
			NewLine();
		}

		if (withheader)
			PutJsonProperty(kPropEntityModel, em->GetName(), true);

		if (errToSend != nil)
			PutJsonProperty(kPropError, *errToSend, true);

		if (!erec->IsNew())
		{
			VString skey;
			erec->GetPrimKeyValue(skey, true);
			PutJsonProperty(kPropKey, skey, true);
		}
		
		PutJsonProperty(kPropStamp, erec->GetModificationStamp(), !whichAttributes.empty());

		sLONG nb = (sLONG)whichAttributes.size(), i = 1;
		for (EntityAttributeSortedSelection::iterator cur = whichAttributes.begin(), end = whichAttributes.end(); cur != end && err == VE_OK; cur++, i++)
		{
			const EntityAttribute* attribute = cur->fAttribute;
			if (attribute != nil)
			{
				PutJsonPropertyName(attribute->GetName());

				bool restrictValue = false;
				EntityAttributeItem* eai = nil;
				if (expandAttributes != nil)
					eai = &(*expandAttributes)[attribute->GetPosInOwner()-1];
				if (eai == nil || eai->fAttribute == nil)
					restrictValue = true;

				EntityAttributeValue* val = nil;
				if (attribute->permissionMatch(DB4D_EM_Read_Perm, fContext))
				{
					val = erec->getAttributeValue(attribute, err, fContext, restrictValue);
				}
				/*
				else
					err = attribute->ThrowError(VE_DB4D_NO_PERM_TO_READ_ATTRIBUTE_VALUE);
					*/
				if (val == nil)
				{
					PutText(kPropNull);
				}
				else if (val == (EntityAttributeValue*)-2 || val == (EntityAttributeValue*)-3)
				{
					bool ispict = val == (EntityAttributeValue*)-2;
					VString uri;
					CalculateURI(uri, erec, attribute, ispict ? L"?$imageformat=best" : L"?$binary=true", true, true, ispict, true);
					PutDeferred(uri, ispict, nil);
				}
				else if (val == (EntityAttributeValue*)-4)
				{
					PrimKey* key = erec->RetainRelatedAttributeKey(attribute, err);
					if (key != nil)
					{
						EntityModel* relatedModel = attribute->GetSubEntityModel();
						if (testAssert(relatedModel != nil))
						{
							VString uri;
							VString keystr;
							key->GetAsString(keystr, true);
							CalculateURI(uri, relatedModel, nil, L"", true, true, true, true, keystr);
							PutDeferred(uri, false, nil, &keystr);
							key->Release();
						}
						else
							PutText(kPropNull);
					}
					else
						PutText(kPropNull);


				}
				else if (val == (EntityAttributeValue*)-5)
				{
					VString uri;
					CalculateURI(uri, erec, attribute, L"", true, true, true, true);
					PutDeferred(uri, false);
				}
				else
				{
					EntityAttributeValueKind kind = val->GetKind();
					switch(kind)
					{
					case eav_vvalue:
						{
							VValueSingle* cv = val->getVValue();
							if (cv == nil || cv->IsNull())
							{
								PutText(kPropNull);
							}
							else
							{
								ValueKind valuetype = cv->GetValueKind();
								if (valuetype == VK_BLOB_DB4D || valuetype == VK_IMAGE || valuetype == VK_BLOB)
								{
									bool isnull;

									if (attribute->GetKind() == eattr_computedField)
										isnull = false;
									else
										isnull = !erec->ContainsBlobData(attribute, fContext);

									if (isnull)
										PutText(kPropNull);
									else
									{
										VString uri;
										CalculateURI(uri, erec, attribute, (valuetype == VK_IMAGE) ? L"?$imageformat=best" : L"?$binary=true", true, true, (valuetype != VK_IMAGE), true);
										PutDeferred(uri, valuetype == VK_IMAGE, cv);
									}
								}
								else
								{
									if (attribute->isSimpleDate() && cv->GetValueKind() == VK_TIME)
									{
										VString dateString;
										((VTime*)cv)->toSimpleDateString(dateString);
										PutJsonPropertyValue(dateString);
									}
									else
										PutJsonPropertyValue(*cv);
								}
							}
						}
						break;

					case eav_subentity:
						{
							EntityModel* submodel = val->getRelatedEntityModel();
							EntityRecord* subrec = val->getRelatedEntity();
							if (subrec == nil || submodel == nil)
								PutText(kPropNull);
							else
							{
								EntityAttributeItem* eai = nil;
								if (expandAttributes != nil)
									eai = &(*expandAttributes)[attribute->GetPosInOwner()-1];
								if (eai != nil && eai->fAttribute != nil)
								{
									if (cur->fSousSelection == nil)
									{
										cur->fSousSelection = new EntityAttributeSortedSelection(submodel);
										submodel->GetAllSortedAttributes(*cur->fSousSelection, this);
									}
									EntityAttributeSortedSelection* subSortingAttributes = nil;
									if (sortingAttributes != nil)
										subSortingAttributes = sortingAttributes->FindSubSelection(attribute);
									err = EntityRecordToJSON(subrec, *cur->fSousSelection, eai->fSousSelection, subSortingAttributes, false, false, nil);
								}
								else
								{
									VString uri;
									CalculateURI(uri, subrec, nil, L"", true, true, true, true);
									VString subkey;
									subrec->GetPrimKeyValue(subkey, true);
									PutDeferred(uri, false, nil, &subkey);
								}
							}
						}
						break;

					case eav_selOfSubentity:
						{
							EntityModel* submodel = val->getRelatedEntityModel();
							EntityCollection* sel = val->getRelatedSelection();
							if (sel == nil || submodel == nil)
								PutText(kPropNull);
							else
							{
								EntityAttributeItem* eai = nil;
								if (expandAttributes != nil)
									eai = &(*expandAttributes)[attribute->GetPosInOwner()-1];
								if (eai != nil && eai->fAttribute != nil)
								{
									if (cur->fSousSelection == nil)
									{
										cur->fSousSelection = new EntityAttributeSortedSelection(submodel);
										submodel->GetAllSortedAttributes(*cur->fSousSelection, this);
									}
									EntityAttributeSortedSelection* subSortingAttributes = nil;
									if (sortingAttributes != nil)
										subSortingAttributes = sortingAttributes->FindSubSelection(attribute);
									sLONG countelem = eai->fCount;
									if (countelem == -1)
									{
										countelem = submodel->GetDefaultTopSizeInUse();
									}
									err = SelToJSON(false, submodel, sel, *cur->fSousSelection, eai->fSousSelection, subSortingAttributes, false, eai->fSkip, countelem);
								}
								else
								{
									VString uri;
									CalculateURI(uri, erec, attribute, L"", true, true, true, true);
									PutDeferred(uri, false);
								}
							}
						}
						break;

					case eav_composition:
						{
							EntityModel* submodel = val->getRelatedEntityModel();
							EntityCollection* sel = val->getRelatedSelection();
							if (sel == nil || submodel == nil)
								PutText(kPropNull);
							else
							{
								EntityAttributeItem* eai = nil;
								if (expandAttributes != nil)
									eai = &(*expandAttributes)[attribute->GetPosInOwner()-1];
								{
									if (cur->fSousSelection == nil)
									{
										cur->fSousSelection = new EntityAttributeSortedSelection(submodel);
										submodel->GetAllSortedAttributes(*cur->fSousSelection, this);
									}
									EntityAttributeSortedSelection* subSortingAttributes = nil;
									if (sortingAttributes != nil)
										subSortingAttributes = sortingAttributes->FindSubSelection(attribute);
									err = SelToJSON(false, submodel, sel, *cur->fSousSelection, eai == nil ? nil : eai->fSousSelection, subSortingAttributes, false, 0, -1, nil, false, true);
								}
							}
						}
						break;
					}
				}

				if (i != nb)
					PutJsonSeparator();
				NewLine();

			}
		}

		if (microsoft && withheader)
		{
			PutJsonEndObject();
			NewLine();
		}

		PutJsonEndObject();
	}

	return err;
}


VError RestTools::SelToJSON(bool firstLevel, EntityModel* em, EntityCollection* inSel, EntityAttributeSortedSelection& whichAttributes, EntityAttributeSelection* expandAttributes,
							EntityAttributeSortedSelection* sortingAttributes, bool withheader, sLONG from, sLONG count, DataSet* inDataSet, bool dejatriee, bool isAComposition)
{
	VError err = VE_OK;
	EntityCollection* trueSel = dejatriee ? RetainRefCountable(inSel) : inSel->SortSel(err, sortingAttributes, fContext, nil);
	if (err == VE_OK && trueSel != nil)
	{
		inSel = trueSel;
		VStream* output = fOutput;
		if (from < 0)
			from = 0;
		sLONG qt = inSel->GetLength(fContext);
		if (count < 0)
			count = qt;
		sLONG lastrow = from+count-1;
		if (lastrow >= qt)
			lastrow = qt-1;

		if (!isAComposition)
		{
			PutJsonBeginObject();

			if (withheader && microsoft)
			{
				PutTabLevel();
				output->PutText(L"\"d\":");
				PutJsonBeginObject();
			}

			if (inDataSet != nil)
			{
				if (newwafkeepselptr != nil)
				{
					PutJsonPropertyName(kPropTransformedSelection);
					
					PutJsonBeginObject();
					PutJsonProperty("mode", newwafkeepselptr->GetMode(), true);
					PutJsonPropertyName("rows");
					PutJsonBeginArray();
					bool first = true;
					for (vector<sLONG>::iterator cur = newwafkeepselptr->rows.begin(), end = newwafkeepselptr->rows.end(); cur != end; cur++)
					{
						if (first)
							first = false;
						else
							PutJsonSeparator();
						PutJsonPropertyLong(*cur);										
					}
					PutJsonEndArray();
					PutJsonSeparator();

					PutJsonPropertyName("ranges");
					PutJsonBeginArray();
					first = true;
					for (WafSelectionRangeVector::iterator cur = newwafkeepselptr->ranges.begin(), end = newwafkeepselptr->ranges.end(); cur != end; cur++)
					{
						if (first)
							first = false;
						else
							PutJsonSeparator();
						sLONG debut = cur->start;
						sLONG fin = cur->end;
						PutJsonBeginObject();
						PutJsonProperty("start", debut, true);
						PutJsonProperty("end", fin, false);
						PutJsonEndObject();
					}
					PutJsonEndArray();
					PutJsonSeparator();

					PutJsonPropertyName("butRows");
					PutJsonBeginArray();
					first = true;
					for (vector<sLONG>::iterator cur = newwafkeepselptr->butRows.begin(), end = newwafkeepselptr->butRows.end(); cur != end; cur++)
					{
						if (first)
							first = false;
						else
							PutJsonSeparator();
						PutJsonPropertyLong(*cur);				
					}
					PutJsonEndArray();

					PutJsonEndObject();
					PutJsonSeparator();
				}
				PutJsonPropertyName(kPropENTITYSET);
				VString uri;
				CalculateURI(uri, inDataSet, em, L"", false, true);
				PutText(uri);
				PutJsonSeparator();
				NewLine();	
			}

			PutJsonQueryPlan();
			if (withheader)
				PutJsonProperty(kPropEntityModel, em->GetName(), true);
			PutJsonProperty(kPropCount, inSel->GetLength(fContext), true);
			PutJsonProperty(kPropSent, lastrow - from + 1, true);
			PutJsonProperty(kPropFirst, from, true);
			PutJsonPropertyName(kPropEntities);
		}

		PutJsonBeginArray();

		for (sLONG i = from; i <= lastrow && err == VE_OK; ++i)
		{
			EntityRecord* erec = inSel->LoadEntity(i, fContext, err, DB4D_Do_Not_Lock);
			if (erec != nil)
			{
				err = EntityRecordToJSON(erec, whichAttributes, expandAttributes, sortingAttributes, false);
				if (i != lastrow && err == VE_OK)
					PutJsonSeparator();
				NewLine();
				erec->Release();
			}
			else
			{
				PutJsonBeginObject(true);
				PutJsonPropertyName(kPropStamp);
				PutJsonPropertyLong(0);
				PutJsonEndObject();
				if (i != lastrow && err == VE_OK)
					PutJsonSeparator();

			}
		}

		PutJsonEndArray();
		NewLine();

		if (firstLevel && err != VE_OK)
		{
			PutJsonSeparator();
			PutJsonPropertyName("__ERROR");
			GenereErrorReport(true);
		}

		if (!isAComposition)
		{
			if (microsoft && withheader)
			{
				PutJsonEndObject();
				NewLine();
			}

			PutJsonEndObject();
		}
	}

	QuickReleaseRefCountable(trueSel);
	return err;
}


VError RestTools::EntityRecordToBag(VValueBag& outBag, EntityRecord* erec, EntityAttributeSortedSelection& whichAttributes, EntityAttributeSelection* expandAttributes, EntityAttributeSortedSelection* sortingAttributes, bool withheader, bool nofields, sLONG* errToSend)
{
	VError err = VE_OK;
	EntityModel* em = erec->GetOwner();

	if (whichAttributes.empty() && !nofields)
	{
		if (!em->GetAllSortedAttributes(whichAttributes, this))
			err = ThrowBaseError(memfull);
	}

	if (err == VE_OK)
	{
		if (errToSend != nil)
			outBag.SetLong(L"__ERROR", *errToSend);
		if (!erec->IsNew())
		{
			VString skey;
			erec->GetPrimKeyValue(skey, true);
			outBag.SetString(d4::__KEY, skey);
			outBag.SetLong(d4::__STAMP, erec->GetModificationStamp());
		}

		sLONG nb = (sLONG)whichAttributes.size(), i = 1;
		for (EntityAttributeSortedSelection::iterator cur = whichAttributes.begin(), end = whichAttributes.end(); cur != end && err == VE_OK; cur++, i++)
		{
			const EntityAttribute* attribute = cur->fAttribute;
			if (attribute != nil)
			{
				bool restrictValue = false;
				EntityAttributeItem* eai = nil;
				if (expandAttributes != nil)
					eai = &(*expandAttributes)[attribute->GetPosInOwner()-1];
				if (eai == nil || eai->fAttribute == nil)
					restrictValue = true;

				EntityAttributeValue* val = nil;
				if (attribute->permissionMatch(DB4D_EM_Read_Perm, fContext))
				{
					val = erec->getAttributeValue(attribute, err, fContext, restrictValue);
				}
				if (val == (EntityAttributeValue*)-2 || val == (EntityAttributeValue*)-3)
				{
					bool ispict = val == (EntityAttributeValue*)-2;
					VString uri;
					CalculateURI(uri, erec, attribute, ispict ? L"?$imageformat=best" : L"?$binary=true", false, false, ispict, false);
					VValueBag* deferredbag = PutDeferred(err, uri, ispict, nil);
					outBag.AddElement(attribute->GetName(), deferredbag);
					deferredbag->Release();
				}
				else if (val == (EntityAttributeValue*)-4)
				{
					PrimKey* key = erec->RetainRelatedAttributeKey(attribute, err);
					if (key != nil)
					{
						EntityModel* relatedModel = attribute->GetSubEntityModel();
						if (testAssert(relatedModel != nil))
						{
							VString uri;
							VString keystr;
							key->GetAsString(keystr, true);

							CalculateURI(uri, relatedModel, nil, L"", false, false, true, false, keystr);
							VValueBag* deferredbag = PutDeferred(err, uri, false, nil, &keystr);
							if (!fToXML)
								deferredbag->SetBool(____objectunic, true);
							outBag.AddElement(attribute->GetName(), deferredbag);
							deferredbag->Release();

							key->Release();
						}
					}
				}
				else if (val == (EntityAttributeValue*)-5)
				{
					VString uri;
					CalculateURI(uri, erec, attribute, L"", false, false, true, false);
					VValueBag* deferredbag = PutDeferred(err, uri, false);
					outBag.AddElement(attribute->GetName(), deferredbag);
					deferredbag->Release();
				}
				else if (val != nil)
				{
					EntityAttributeValueKind kind = val->GetKind();
					switch(kind)
					{
					case eav_vvalue:
						{
							VValueSingle* cv = val->getVValue();
							if (cv == nil || cv->IsNull())
							{
								//PutText(L"null");
							}
							else
							{
								ValueKind valuetype = cv->GetValueKind();
								if (valuetype == VK_BLOB_DB4D || valuetype == VK_IMAGE || valuetype == VK_BLOB)
								{
									bool isnull;

									if (attribute->GetKind() == eattr_computedField)
										isnull = false;
									else
										isnull = !erec->ContainsBlobData(attribute, fContext);

									if (!isnull)
									{
										VString uri;
										CalculateURI(uri, erec, attribute, (valuetype == VK_IMAGE) ? L"?$imageformat=best" : L"?$binary=true", false, false, (valuetype != VK_IMAGE), false);
										VValueBag* deferredbag = PutDeferred(err, uri, valuetype == VK_IMAGE, cv);
										outBag.AddElement(attribute->GetName(), deferredbag);
										deferredbag->Release();
									}
								}
								else
								{
									if (attribute->isSimpleDate() && cv->GetValueKind() == VK_TIME)
									{
										VString dateString;
										((VTime*)cv)->toSimpleDateString(dateString);
										outBag.SetAttribute(attribute->GetName(), (VValueSingle*)dateString.Clone());
									}
									else
										outBag.SetAttribute(attribute->GetName(), (VValueSingle*)cv->Clone());
								}
							}
						}
						break;

					case eav_subentity:
						{
							EntityModel* submodel = val->getRelatedEntityModel();
							EntityRecord* subrec = val->getRelatedEntity();
							if (subrec != nil && submodel != nil)
							{
								EntityAttributeItem* eai = nil;
								if (expandAttributes != nil)
									eai = &(*expandAttributes)[attribute->GetPosInOwner()-1];
								if (eai != nil && eai->fAttribute != nil)
								{
									if (cur->fSousSelection == nil)
									{
										cur->fSousSelection = new EntityAttributeSortedSelection(submodel);
										submodel->GetAllSortedAttributes(*cur->fSousSelection, this);
									}

									EntityAttributeSortedSelection* subSortingAttributes = nil;
									if (sortingAttributes != nil)
										subSortingAttributes = sortingAttributes->FindSubSelection(attribute);

									VValueBag* entitybag = new VValueBag();
									if (!fToXML)
										entitybag->SetBool(____objectunic, true);
									err = EntityRecordToBag(*entitybag, subrec, *cur->fSousSelection, eai->fSousSelection, subSortingAttributes, false, false, nil);
									if (err == VE_OK)
										outBag.AddElement(attribute->GetName(), entitybag);
									entitybag->Release();
								}
								else
								{
									VString uri;
									CalculateURI(uri, subrec, nil, L"", false, false, true, false);
									VString subkey;
									subrec->GetPrimKeyValue(subkey, true);
									VValueBag* deferredbag = PutDeferred(err, uri, false, nil, &subkey);
									if (!fToXML)
										deferredbag->SetBool(____objectunic, true);
									outBag.AddElement(attribute->GetName(), deferredbag);
									deferredbag->Release();
								}
							}
						}
						break;

					case eav_selOfSubentity:
						{
							EntityModel* submodel = val->getRelatedEntityModel();
							EntityCollection* sel = val->getRelatedSelection();
							if (sel != nil && submodel != nil)
							{
								EntityAttributeItem* eai = nil;
								if (expandAttributes != nil)
									eai = &(*expandAttributes)[attribute->GetPosInOwner()-1];
								if (eai != nil && eai->fAttribute != nil)
								{
									if (cur->fSousSelection == nil)
									{
										cur->fSousSelection = new EntityAttributeSortedSelection(submodel);
										submodel->GetAllSortedAttributes(*cur->fSousSelection, this);
									}

									EntityAttributeSortedSelection* subSortingAttributes = nil;
									if (sortingAttributes != nil)
										subSortingAttributes = sortingAttributes->FindSubSelection(attribute);

									VValueBag* selbag = new VValueBag();
									sLONG countelem = eai->fCount;
									if (countelem == -1)
									{
										countelem = submodel->GetDefaultTopSizeInUse();
									}
									err = SelToBag(*selbag, submodel, sel, *cur->fSousSelection, eai->fSousSelection, subSortingAttributes, false, eai->fSkip, countelem);
									if (err == VE_OK)
										outBag.AddElement(attribute->GetName(), selbag);
									selbag->Release();
								}
								else
								{
									VString uri;
									CalculateURI(uri, erec, attribute, L"", false, false, true, false);
									VValueBag* deferredbag = PutDeferred(err, uri, false);
									outBag.AddElement(attribute->GetName(), deferredbag);
									deferredbag->Release();
								}
							}
						}
						break;

					case eav_composition:
						{
							EntityModel* submodel = val->getRelatedEntityModel();
							EntityCollection* sel = val->getRelatedSelection();
							if (sel != nil && submodel != nil)
							{
								EntityAttributeItem* eai = nil;
								if (expandAttributes != nil)
									eai = &(*expandAttributes)[attribute->GetPosInOwner()-1];
								{
									if (cur->fSousSelection == nil)
									{
										cur->fSousSelection = new EntityAttributeSortedSelection(submodel);
										submodel->GetAllSortedAttributes(*cur->fSousSelection, this);
									}

									EntityAttributeSortedSelection* subSortingAttributes = nil;
									if (sortingAttributes != nil)
										subSortingAttributes = sortingAttributes->FindSubSelection(attribute);

									VString attributeName;
									attribute->GetName(attributeName);

									VValueBag* selbag = new VValueBag();
									err = SelToBag(*selbag, submodel, sel, *cur->fSousSelection, eai == nil ? nil : eai->fSousSelection, subSortingAttributes, false, 0, -1, nil, true, true, &attributeName);
									if (err == VE_OK)
										outBag.AddElement(attribute->GetName(), selbag);
									selbag->Release();
								}
							}
						}
						break;
					}
				}
			}
		}

	}

	return err;
}


VError RestTools::SelToBag(VValueBag& outBag, EntityModel* em, EntityCollection* inSel, EntityAttributeSortedSelection& whichAttributes, EntityAttributeSelection* expandAttributes, 
						   EntityAttributeSortedSelection* sortingAttributes, bool withheader, sLONG from, sLONG count, DataSet* inDataSet, bool dejatriee, 
						   bool isAComposition, const VString* arrayname)
{
	VError err = VE_OK;
	EntityCollection* trueSel = dejatriee ? RetainRefCountable(inSel) : inSel->SortSel(err, sortingAttributes, fContext, nil);
	if (err == VE_OK && trueSel != nil)
	{
		inSel = trueSel;

		if (from < 0)
			from = 0;
		sLONG qt = inSel->GetLength(fContext);
		if (count < 0)
			count = qt;
		sLONG lastrow = from+count-1;
		if (lastrow >= qt)
			lastrow = qt-1;

		bool oldfWithQueryPath = fWithQueryPath;
		bool oldfWithQueryPlan = fWithQueryPlan;

		if (!isAComposition)
		{
			if (inDataSet != nil)
			{
				VString uri;
				CalculateURI(uri, inDataSet, em, L"", false, true);
				outBag.SetString(L"__ENTITYSET", uri);
			}

			if (fWithQueryPath && fQueryPath != nil)
				outBag.AddElement(L"__queryPath", fQueryPath);
			if (fWithQueryPlan && fQueryPlan != nil)
				outBag.AddElement(L"__queryPlan", fQueryPlan);
			fWithQueryPath = false;
			fWithQueryPlan = false;

			if (withheader)
				outBag.SetString(L"__entityModel", em->GetName());
			outBag.SetLong(L"__COUNT", inSel->GetLength(fContext));
			outBag.SetLong(L"__SENT", lastrow-from+1);
			outBag.SetLong(L"__FIRST", from);
		}
		else
		{
			fWithQueryPath = false;
			fWithQueryPlan = false;
		}

		for (sLONG i = from; i <= lastrow; ++i)
		{
			EntityRecord* erec = inSel->LoadEntity(i, fContext, err, DB4D_Do_Not_Lock);
			if (erec != nil)
			{
				VValueBag* oneEntityBag = new VValueBag();
				err = EntityRecordToBag(*oneEntityBag, erec, whichAttributes, expandAttributes, sortingAttributes, false);
				erec->Release();
				if (err == VE_OK)
				{
					if (isAComposition && arrayname != nil)
						outBag.AddElement(*arrayname, oneEntityBag);
					else
						outBag.AddElement(d4::__ENTITIES, oneEntityBag);
				}
				oneEntityBag->Release();
			}
			else
			{
				VValueBag* oneEntityBag = new VValueBag();
				oneEntityBag->SetLong("__STAMP", 0);
				if (isAComposition && arrayname != nil)
					outBag.AddElement(*arrayname, oneEntityBag);
				else
					outBag.AddElement(d4::__ENTITIES, oneEntityBag);
				oneEntityBag->Release();
			}
		}

		fWithQueryPath = oldfWithQueryPath;
		fWithQueryPlan = oldfWithQueryPlan;
	}

	QuickReleaseRefCountable(trueSel);
	return err;
}


void RestTools::StringToHTML( const VString& inString, VString& processed_string )
{
	processed_string.Clear();
	VIndex str_len = inString.GetLength();
	VIndex block_start = 0;
	VIndex block_size = 0;


	for (VIndex lap_index = 1;lap_index <= str_len; ++lap_index)
	{
		UniChar lap_char = inString.GetUniChar(lap_index);

		if ((lap_char==CHAR_QUOTATION_MARK) || (lap_char==CHAR_LESS_THAN_SIGN)
			|| (lap_char==CHAR_GREATER_THAN_SIGN) || (lap_char==CHAR_PERCENT_SIGN)
			|| (lap_char==CHAR_AMPERSAND) || (lap_char==CHAR_EQUALS_SIGN)
			|| (lap_char==CHAR_APOSTROPHE)
			/*|| ( htmlConvert && lap_char >= CHAR_TILDE )*/)
		{
			// first append previous byte
			if (block_size)
				processed_string.AppendUniChars(inString.GetCPointer()+block_start,block_size);
			block_start = lap_index;
			block_size = 0;
			switch (lap_char)
			{
			case CHAR_QUOTATION_MARK: processed_string.AppendCString("&quot;");		break;
			case CHAR_LESS_THAN_SIGN: processed_string.AppendCString("&lt;");		break;
			case CHAR_GREATER_THAN_SIGN: processed_string.AppendCString("&gt;");	break;
			case CHAR_PERCENT_SIGN: processed_string.AppendCString("&#37;");		break;
			case CHAR_AMPERSAND: processed_string.AppendCString("&amp;");			break;
			case CHAR_EQUALS_SIGN: processed_string.AppendCString("&#61;");			break;
			case CHAR_APOSTROPHE: processed_string.AppendCString("&#39;");			break;	// "&apos;" is not recognized by IE...
			default:
				processed_string.AppendUniChar( CHAR_AMPERSAND);
				processed_string.AppendUniChar( CHAR_NUMBER_SIGN);
				processed_string.AppendLong( (sLONG)lap_char);
				processed_string.AppendUniChar( CHAR_SEMICOLON);
				break;
			}
		}
		else
			++block_size;
	}

	// append the final block
	if (block_size > 0)
		processed_string.AppendUniChars(inString.GetCPointer()+block_start,block_size);	

}


VError RestTools::PutHTMLEntete()
{
	return PutText(L"<body>\n");
}


VError RestTools::PutHTMLFin()
{
	return PutText(L"</body>\n");
}


VError RestTools::PutHTMLTableBegin()
{
	return PutText(L"<table border=\"1\" cellpadding=\"2\" cellspacing=\"0\">\n");
}


VError RestTools::PutHTMLTableEnd()
{
	return PutText(L"</table>\n");
}


VError RestTools::PutHTMLTableRowBegin()
{
	return PutText(L"<tr>\n");
}


VError RestTools::PutHTMLTableRowEnd()
{
	return PutText(L"</tr>\n");
}


VError RestTools::PutHTMLTableColBegin()
{
	return PutText(L"<td>\n");
}


VError RestTools::PutHTMLTableColEnd()
{
	return PutText(L"</td>\n");
}


VError RestTools::PutHTMLString(const VString& inText)
{
	VString s;
	StringToHTML(inText, s);
	return PutText(s);
}


VError RestTools::SelToHTML(EntityModel* em, EntityCollection* inSel, EntityAttributeSortedSelection& whichFields, bool withheader, sLONG from, sLONG count, DataSet* inDataSet)
{
	VError err = VE_OK;
	return err;
}


VError RestTools::ImportEntitiesSel(EntityModel* em, const VValueBag& bagData, VValueBag& bagResult, EntityAttributeValue* parent, bool firstLevel)
{
	VError err = VE_OK;

	const VBagArray* records = bagData.GetElements(d4::__ENTITIES);

	if (records != nil)
	{
		for (VIndex i = 1, nbrecs = records->GetCount(); i <= nbrecs && err == VE_OK; i++)
		{
			BagElement outRecBag(bagResult, d4::__ENTITIES);
			const VValueBag* recbag = records->GetNth(i);
			err = ImportEntityRecord(em, *recbag, *outRecBag, parent, false, nil, nil, firstLevel);
		}
	}
	else
	{
		err = ImportEntityRecord(em, bagData, bagResult, parent, true, nil, nil, firstLevel);
	}

	return err;
}

VError RestTools::ImportEntityRecord(EntityModel* em, const VValueBag& bagData, VValueBag& bagResult, EntityAttributeValue* parent, bool onlyone, const EntityAttribute* relDestAtt, EntityAttributeValue* relValue, bool firstLevel)
{
	VError errglob = VE_OK;
	VError err = VE_OK;

	if (bagData.GetAttributesCount() !=0 || bagData.GetElementNamesCount() != 0)
	{
		StErrorContextInstaller errs(false);

		bool newrec = true;
		sLONG stamp = 0;
		EntityRecord* erec = nil;
		bool toutok = true, mustsave = true;

		EntityAttributeSortedSelection attributes(em);

		bagData.GetLong(d4::__STAMP, stamp);

		VString skey;
		if (bagData.GetString(d4::__KEY, skey) && !skey.IsEmpty())
		{
			newrec = false;
			erec = em->findEntityWithPrimKey(skey, fContext, err, DB4D_Do_Not_Lock);
		}

		if (newrec)
			erec = em->newEntity(fContext);

		if (erec != nil && err == VE_OK)
		{
			for (VIndex i = 1, nb = bagData.GetAttributesCount(); i <= nb; i++)
			{
				VString attname;
				const VValueSingle* cv = bagData.GetNthAttribute(i, &attname);
				if (cv != nil)
				{
					if (attname.GetLength() >= 2 && attname.GetUniChar(1) == '_' && attname.GetUniChar(2) == '_')
					{
						// c'est un mot reserve
					}
					else
					{
						EntityAttribute* att = em->getAttribute(attname);
						if (att == nil || att->getScope() != escope_public)
						{
							err = erec->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &attname);
							toutok = false;
						}
						else
						{
							if (!att->permissionMatch(newrec ? DB4D_EM_Create_Perm : DB4D_EM_Update_Perm, fContext))
							{
								err = att->ThrowError(VE_DB4D_NO_PERM_TO_UPDATE_ATTRIBUTE_VALUE);
								fContext->SetPermError();
								toutok = false;
							}
							else
							{
								attributes.AddAttribute(att, nil);
								if (!erec->equalAttributeValue(att,cv))
									err = erec->setAttributeValue(att, cv);
								if (err != VE_OK)
									toutok = false;
							}
						}
					}
				}
			}

			for (VIndex i = 1, nb = bagData.GetElementNamesCount(); i <= nb; i++)
			{
				VString attname;
				const VBagArray* subelems = bagData.GetNthElementName(i, &attname);
				if (subelems != nil)
				{
					if (attname.GetLength() >= 2 && attname.GetUniChar(1) == '_' && attname.GetUniChar(2) == '_')
					{
						// c'est un mot reserve
					}
					else
					{
						EntityAttribute* att = em->getAttribute(attname);
						if (att == nil)
						{
							err = erec->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &attname);
							toutok = false;
						}
						else
						{
							const VValueBag* elemBag = subelems->GetNth(1);
							EntityModel* subEM = att->GetSubEntityModel();
							if (!att->permissionMatch(newrec ? DB4D_EM_Create_Perm : DB4D_EM_Update_Perm, fContext))
							{
								err = att->ThrowError(VE_DB4D_NO_PERM_TO_UPDATE_ATTRIBUTE_VALUE);
								fContext->SetPermError();
								toutok = false;
							}
							else
							{
								switch (att->GetKind())
								{
									case eattr_relation_Nto1:
										{
											EntityAttributeValue* eVal = erec->getAttributeValue(att, err, fContext);
											const VBagArray* subrecords = elemBag->GetElements(d4::__ENTITIES);
											BagElement outRecBag(bagResult, attname);
											if (subrecords != nil)
											{
												const VValueBag* realElem = subrecords->GetNth(1);
												err = ImportEntityRecord(subEM, *realElem, *outRecBag, eVal, false, nil, nil, false);
											}
											else
												err = ImportEntityRecord(subEM, *elemBag, *outRecBag, eVal, true, nil, nil, false);

											if (err != VE_OK)
											{
												err = erec->ThrowError(VE_DB4D_CANNOT_SAVE_RELATED_ENTITY, &attname);
											}
										}
										break;

									case eattr_relation_1toN:
										break;

									case eattr_composition:
										{
											EntityAttributeValue* eVal = erec->getAttributeValue(att, err, fContext);
											if (eVal != nil)
											{
												EntityCollection* sel = eVal->getRelatedSelection();
												EntityModel* relEm = eVal->getRelatedEntityModel();
												if (sel != nil && relEm != nil)
												{
													err = sel->DropEntities(fContext, nil, nil);
													if (err == VE_OK)
													{
														const VBagArray* records = bagData.GetElements(attname);

														if (records != nil)
														{
															EntityRelation* rel = att->GetRelPath();
															const EntityAttribute* destatt = rel->GetDestAtt();
															const EntityAttribute* sourceatt = rel->GetSourceAtt();

															VError err2 = VE_OK;
															EntityAttributeValue* cv = erec->getAttributeValue(sourceatt, err, fContext);

															for (VIndex i = 1, nbrecs = records->GetCount(); i <= nbrecs && err == VE_OK; i++)
															{
																BagElement outRecBag(bagResult, d4::__ENTITIES);
																const VValueBag* recbag = records->GetNth(i);
																EntityRecord* subrec = nil;
																err = ImportEntityRecord(relEm, *recbag, *outRecBag, eVal, false, destatt, cv, false);
															}
														}

													}
													
												}
											}

										}
										break;

									default:
										attributes.AddAttribute(att, this);
										break;
								}
							}
						}
					}
				}
			}

			if (err == VE_OK)
			{
				if (relDestAtt != nil && relValue != nil)
				{
					err = erec->setAttributeValue(relDestAtt, relValue);
				}
			}

			if (!newrec)
			{
				if (!erec->IsModified())
					mustsave = false;
			}

			if (err == VE_OK && toutok && (mustsave || refreshOnly))
			{
				bool allowOverrideStamp = false;
				if (stamp < 0)
				{
					stamp = - stamp;
					allowOverrideStamp = em->allowOverrideStamp();
				}
				else
				{
					if (stamp == 0)
						stamp = 1;
				}
				err = erec->Save(stamp, allowOverrideStamp, refreshOnly);
			}

			if (err != VE_OK && !refreshOnly)
			{
				StErrorContextInstaller errs2(false);
				VError err2;
				EntityRecord* erec2 = nil;
				if (!newrec)
					erec2 = em->findEntityWithPrimKey(skey, fContext, err2, DB4D_Do_Not_Lock);
				if (erec2 != nil)
				{
					if (firstLevel)
					{
						EntityAttributeSortedSelection xattributes(em);
						EntityRecordToBag(bagResult, erec2, xattributes, expandattributes, nil, false, false, nil);
					}
					else
					{
						EntityRecordToBag(bagResult, erec2, attributes, nil, nil, false, true, nil);
					}
					erec2->Release();
				}
				else
				{
					VString skey;
					erec->GetPrimKeyValue(skey, true);
					if (!skey.IsEmpty())
					{

						VString skey;
						erec->GetPrimKeyValue(skey, true);
						bagResult.SetString(d4::__KEY, skey);
						bagResult.SetLong(d4::__STAMP, erec->GetModificationStamp());
						VString uri;
						CalculateURI(uri, erec, nil, L"", false, false, true, false);
						bagResult.SetString(d4::uri,uri);
					}
				}
			}
			else
			{
				if (parent != nil)
				{
					if (mustsave || !parent->equalRelatedEntity(erec))
						parent->SetRelatedEntity(erec, fContext);
				}
				VString skey;
				if (em->HasPrimKey())
				erec->GetPrimKeyValue(skey, true);
				if (!skey.IsEmpty())
				{
					bagResult.SetString(d4::__KEY, skey);
					bagResult.SetLong(d4::__STAMP, erec->GetModificationStamp());
					VString uri;
					CalculateURI(uri, erec, nil, L"", false, false, true, false);
					bagResult.SetString(d4::uri,uri);
				}

				
				{
					EntityAttributeSortedSelection outAtts(em);
					{
						if (firstLevel)
						{
							EntityRecordToBag(bagResult, erec, outAtts, expandattributes, nil, false, false, nil);
						}
						else
						{
							EntityRecordToBag(bagResult, erec, outAtts, nil, nil, false, false, nil);
						}
					}
				}

			}

		}
		else
			toutok = false;

		QuickReleaseRefCountable(erec);

		if (err != VE_OK || !toutok)
		{
			BuildErrorStack(bagResult);
			fImportNotFullySuccessfull = true;
			if (parent != nil)
				errglob = -1;
		}
		else
		{

		}

		if (onlyone && !fToXML)
		{
			bagResult.SetBool(____objectunic, true);
		}
	}

	return errglob;
}



VError RestTools::DropEntities(EntityModel* em, EntityCollection* sel)
{
	VError err = VE_OK;

	bool allsuccess = true;

	if (fImportAtomic)
		fContext->StartTransaction(err);


	err = sel->DropEntities(fContext, nil, nil);
	if (err != VE_OK)
		allsuccess = false;

	if (fImportAtomic)
	{
		if (!allsuccess)
			fContext->RollBackTransaction();
		else
			fContext->CommitTransaction();
	}

	return err;
}



VError RestTools::ImportEntities(EntityModel* em)
{
	VError err = VE_OK;

	fImportNotFullySuccessfull = false;

	if (fImportAtomic)
		fContext->StartTransaction(err);

	VValueBag bagData;
	if (fToXML)
	{
		ComputeInputString();
		err = LoadBagFromXML(fInputString, L"result", bagData);
	}
	else
	{
		VString s;
		ComputeInputString();
		JsonToken tok = GetNextJsonToken(s);
		if (tok == jsonBeginArray)
		{
			VString newinput(L"{ __ENTITIES: ");
			newinput+=fInputString;
			newinput+=L"}";
			err = bagData.FromJSONString(newinput);
		}
		else
			err = bagData.FromJSONString(fInputString);
	}

	if (err == VE_OK)
	{
		VValueBag resultBag;
		err = ImportEntitiesSel(em, bagData, resultBag, nil, true);
		if (err == VE_OK)
		{
			VString result;
			if (fToXML)
				SaveBagToXML(resultBag, L"result", result, fPrettyFormatting, nil, true);
			else
				SaveBagToJson(resultBag, result, fPrettyFormatting);
			PutText(result);
		}
	}

	if (fImportNotFullySuccessfull)
	{
		SetHTTPError(rest::http_internal_error);
		if (fContext->PermErrorHappened())
			SetHTTPError(rest::http_perm_error);
	}

	if (fImportAtomic)
	{
		if (fImportNotFullySuccessfull)
		{
			fContext->RollBackTransaction();
		}
		else
			fContext->CommitTransaction();
	}

	return err;

}


bool RestTools::CalculateHtmlBaseURI(VString& outUri)
{
	/*
	outUri = L"http://";

	outUri += fHostName;
	*/

	outUri += L"/"+VDBMgr::GetManager()->GetRestPrefix();
	fURL->SetCurPartPos(0);
	const VString* s = fURL->NextPart();
	while (s != nil)
	{
		outUri.AppendUniChar('/');
		outUri += *s;
		s = fURL->NextPart();
	}

	bool first = true;

	return first;
}


VError RestTools::ThrowError(VError err, const VString* param1, const VString* param2, const VString* param3)
{
	VErrorBase *ERR = new VErrorBase(err, 0);
	if (param1 != nil)
		ERR->GetBag()->SetString(Db4DError::Param1, *param1);
	if (param2 != nil)
		ERR->GetBag()->SetString(Db4DError::Param2, *param2);
	if (param3 != nil)
		ERR->GetBag()->SetString(Db4DError::Param3, *param3);
	VTask::GetCurrent()->PushRetainedError( ERR);

	return err;
}


void RestTools::BuildErrorStack(VValueBag& outBag)
{
	VErrorTaskContext *context = VTask::GetCurrent()->GetErrorContext( false);
	if (context)
	{
		VErrorContext combinedContext;
		context->GetErrors( combinedContext);

		const VErrorStack& errorstack = combinedContext.GetErrorStack();
		for( VErrorStack::const_iterator cur = errorstack.begin(), end = errorstack.end(); cur != end; cur++)
		{
			VErrorBase *err = *cur;
			BagElement errbag(outBag, d4::__ERROR);
			VString errdesc;
			err->DumpToString(errdesc);
			errbag->SetString(L"message", errdesc);
			VError xerr = err->GetError();
			sLONG errnum = ERRCODE_FROM_VERROR(xerr);
			OsType component = COMPONENT_FROM_VERROR(xerr);
			component = SwapLong(component);
			VString compname(&component, 4, VTC_US_ASCII);
			errbag->SetString(L"componentSignature", compname);
			errbag->SetLong(L"errCode", errnum);
		}
	}
	FlushErrors();
}


void RestTools::TransfertBufferToOutpout()
{
	if (!fOutputBuffer.IsEmpty())
		fOutput->PutText(fOutputBuffer);
}

void RestTools::GenerateInfoIfEmpty()
{
	if (fOutput->GetSize() == 0)
	{
		VValueBag bag;
		bag.SetBool(L"ok", true);
		VString resultString;
		if (fToXML)
			SaveBagToXML(bag, L"result", resultString, false, nil, true);
		else
			bag.GetJSONString(resultString);
		fOutput->PutText(resultString);
	}
}

void RestTools::GenereErrorReport(bool inAnotherObject)
{
	if (!dejaerror)
	{
		dejaerror = true;
		VValueBag errbag;
		BuildErrorStack(errbag);
		VString outErrStack;
		if (fToXML)
		{
			SaveBagToXML(errbag, L"errorStack", outErrStack, false, nil, true);
		}
		else
		{
			if (inAnotherObject)
			{
				VBagArray* sub = errbag.GetElements("__ERROR");
				if (sub != nil)
				{
					sLONG curlevel = 0;
					sub->_GetJSONString(outErrStack, curlevel, true, JSON_Default);
				}
				else
				{
					PutJsonBeginArray();
					PutJsonEndArray();
				}
			}
			else
				errbag.GetJSONString(outErrStack);
		}

		PutText(outErrStack);
	}
}


VError RestTools::SetCurrentUser(const VString& inUserName, const VString& inPassword)
{
	VError err = VE_OK;

	if (fUAGDirectory != nil && !inUserName.IsEmpty())
	{
		fUserSession = fUAGDirectory->OpenSession(inUserName, inPassword, &err);
		if (fUserSession != nil)
		{
			fUAGDirectory->GetUserID(inUserName, fUserID);
			fUserName = inUserName;
			fPassword = inPassword;
		}
		fContext->SetCurrentUser(fUserID, fUserSession);
	}
	
	return err;
}


VDB4DProgressIndicator* RestTools::GetProgressIndicator()
{
	if (fProgressIndicator == nil)
	{
		VString userinfo;
		if (!fURL->GetValue(rest::progressinfo, userinfo) || userinfo.IsEmpty())
		{
			userinfo = fURL->GetSource();
		}
		fProgressIndicator = new VProgressIndicator();
		fProgressIndicator->SetUserInfo(userinfo);
	}
	return fProgressIndicator;
}










