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

Object.prototype.addClass = function(className, collectionName, extendClass, scope, otherProperties)
{
	var dataClass = {
		properties: {}
	};
	
	if (className == "" || className == null)
		throw({error: 12001, errorMessage:"DataClass name is invalid."});
	
	if (collectionName == null || collectionName =="")
		collectionName = className+"Collection";
		
	var props = dataClass.properties;
	props.className = className;
	props.collectionName = collectionName;
	if (extendClass != null)
		props.extendClass = extendClass;
		
	scope = scope || "public";
	props.scope = scope;
	
	if (otherProperties != null && typeof otherProperties === "object")
	{
		for (var e in otherProperties)
		{
			props[e] = otherProperties[e];
		}
	}
	
	this[className] = dataClass;
};


Object.prototype.addAttribute = function(attributeName, kind, type, param4, otherProperties)
{
	var attribute = {};
	var props = attribute;

	if (attributeName == "" || attributeName == null)
		throw({error: 12002, errorMessage:"Attribute name is invalid."});
		
	kind = kind || "storage";
	type = type || "string";
	
	props.name = attributeName;
	props.kind = kind;
	props.type = type;
	
	var relationPath = null;
	var indexType= null;
	
	if (param4 != null)
	{
		if (kind === "storage")
			indexType = param4;
		else
			relationPath = param4;
	}
	
	if (relationPath != null)
		props.path = relationPath;
	
	if (indexType != null)
	{
		if (indexType === "key" || indexType === "key auto")
		{
			props.primKey = true;
			if (indexType === "key auto")
			{
				if (type === "uuid")
					props.autogenerate = true;
				else
					props.autosequence = true;
			}
		}
		else
			props.indexType = indexType;
	}
	
	if (otherProperties != null && typeof otherProperties === "object")
	{
		for (var e in otherProperties)
		{
			props[e] = otherProperties[e];
		}
	}
	this[attributeName] = attribute;
};
