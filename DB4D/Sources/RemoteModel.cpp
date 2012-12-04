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



VError EntityModelCatalogRemote::GetJSON(VJSONValue& outVal, const VString& inURL, const VString* inBodyPart, const VString* inCommand, const VJSONValue* inURLQueryPart)
{
	VError err = VE_OK;
	XMLHttpRequest xhr;
	VString url = fBaseURL+inURL;
	if (inURLQueryPart != nil)
	{
		const VJSONObject* parts = inURLQueryPart->GetObject();
		if (parts != nil)
		{
			VString added;
			for (VJSONPropertyConstIterator cur(parts); cur.IsValid(); ++cur)
			{
				const VString& propName = cur.GetName();
				const VJSONValue* val = &(cur.GetValue());
				VString s;
				if (val->IsObject())
				{
					val->Stringify(s, JSON_WithQuotesIfNecessary);
				}
				else
				{
					val->GetString(s);
				}
				if (added.GetLength() == 0)
					added.AppendUniChar('?');
				else
					added.AppendUniChar('&');
				added += propName;
				added.AppendUniChar('=');
				added += s;
			}
			url += added;
		}
	}
	VString command;
	if (inCommand != nil)
		command = *inCommand;
	else
	{
		if (inBodyPart != nil)
			command = "POST";
		else
			command = "GET";
	}
	err = xhr.Open(command, url, false);
	if (err == VE_OK)
	{
		VString body;
		if (inBodyPart != nil)
			body = *inBodyPart;
		VError err2 = xhr.Send(body, &err);
		if (err == VE_OK)
			err = err2;
	}

	VString response;
	if (err == VE_OK)
	{
		err = xhr.GetResponseText(&response);
	}

	if (err == VE_OK)
	{
		VJSONImporter json(response);
		err = json.Parse(outVal);
	}
	
	if (err != VE_OK)
	{
		VJSONObject* obj = new VJSONObject;
		VJSONValue errval;
		errval.SetNumber(0);
		obj->SetProperty("error", errval); 
		outVal.SetObject(obj);
	}
	else
	{
		if (outVal.IsObject())
		{
			VJSONValue errval = outVal.GetProperty("error");
			if (errval.IsNumber())
				err = -1;
		}
	}

	return err;
}




VError EntityModelCatalogRemote::GetCatalog()
{
	VJSONValue catVal;
	VError err = GetJSON(catVal, "$catalog/$all");
	if (err == VE_OK)
	{
		VJSONValue classesVal = catVal.GetProperty("dataClasses");
		if (classesVal.IsObject())
		{
			for (VJSONPropertyIterator curclass(classesVal.GetObject()); curclass.IsValid(); ++curclass)
			{
				VString dataClassName = curclass.GetName();
				VJSONObject* dataClass = curclass.GetValue().GetObject();
				if (dataClass != nil)
				{
					VString collectionName;
//					dataClass->GetPropertyAsString("collectionName", collectionName);
					EntityModelRemote* model = new EntityModelRemote(dataClassName, collectionName, this);
					fEntityModels[dataClassName] = model;
					fEntityModelsByCollectionName[collectionName] = model;
				}
			}
		}
	}

	return err;
}


