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





ObjectPath::ObjectPath(const VString& inPath)
{
	FromString(inPath);
}

ObjectPath::ObjectPath()
{
	GotoFirst();
}

void ObjectPath::FromVector(const VectorOfVString& from)
{
	fParts = from;
	GotoFirst();
}

void ObjectPath::FromString(const VString& inPath)
{
	//inPath.GetSubStrings('.', fParts, false, false);

	Wordizer ww(inPath);
	VString s;
	while (ww.GetNextWord(s, '.', true, false))
	{
		VString s2;
		Wordizer ww2(s);
		while (ww2.GetNextWord(s2, '[', true, false))
		{
			sLONG len = s2.GetLength();
			if (len > 0 && s2[len - 1] == ']')
			{
				s2.Truncate(len - 1);
			}
			fParts.push_back(s2);
		}
	}

	GotoFirst();
}


void ObjectPath::GotoFirst()
{
	fCurrent = fParts.begin();
}


const VString* ObjectPath::NextPart()
{
	const VString* result = nil;
	if (fParts.end() != fCurrent)
	{
		result = &(*fCurrent);
		++fCurrent;
	}
	return result;
}


bool ObjectPath::SamePath(const ObjectPath& other) const
{
	bool same = fParts.size() == other.fParts.size();
	if (same)
	{
		for (VectorOfVString::const_iterator cur = fParts.begin(), end = fParts.end(), curother = other.fParts.begin(); cur != end; ++cur)
		{
			if (!cur->EqualToString(*curother, true))
			{
				same = false;
				break;
			}
		}
	}

	return same;
}


void ObjectPath::ToString(VString& outStr) const
{
	outStr.Clear();
	bool first = true;
	for (VectorOfVString::const_iterator cur = fParts.begin(), end = fParts.end(); cur != end; ++cur)
	{
		if (first)
			first = false;
		else
			outStr += ".";
		outStr += *cur;
	}
}




// ---------------------------------------------------------------------------------------------------


ObjectParser::ObjectParser(VJSONValue& inVal, ObjectPath& path, VJSONValue& outVal)
{
	path.GotoFirst();
	outVal.SetNull();
	const VString* curpart = path.NextPart();
	while (inVal.IsObject() && curpart != nil)
	{
		VJSONValue subval = inVal.GetObject()->GetProperty(*curpart);
		curpart = path.NextPart();
		if (curpart == nil)
		{
			outVal = subval;
		}
		else
		{
			inVal = subval;
		}
	}
}


ObjectParser::ObjectParser(const VJSONValue& inVal, ObjectNodeCollection& outNodes)
{
	VString startingPath;
	ParseBranches(inVal, outNodes, startingPath);
}


void ObjectParser::ParseBranches(const VJSONValue& inVal, ObjectNodeCollection& outNodes, const VString& currentPath)
{
	if (!currentPath.IsEmpty())
		outNodes.push_back(ObjectNode(inVal, currentPath));

	if (inVal.IsArray())
	{
		VJSONArray* arr = inVal.GetArray();

		VString newpath;
		if (currentPath.IsEmpty())
			newpath = "length";
		else
			newpath = currentPath + "." + "length";
		outNodes.push_back(ObjectNode(VJSONValue(arr->GetCount()), newpath));

		for (VSize i = 0, nbelem = arr->GetCount(); i < nbelem; ++i)
		{
			VString newpath;
			if (currentPath.IsEmpty())
				newpath = ToString(i);
			else
				newpath = currentPath + "." + ToString(i);
			ParseBranches((*arr)[i], outNodes, newpath);
		}
	}
	else if (inVal.IsObject())
	{
		VJSONObject* obj = inVal.GetObject();
		VJSONPropertyConstIterator iter(obj);
		while (iter.IsValid())
		{
			VString newpath;
			if (currentPath.IsEmpty())
				newpath = iter.GetName();
			else
				newpath = currentPath + "." + iter.GetName();
			ParseBranches(iter.GetValue(), outNodes, newpath);
			++iter;
		}
	}
}



