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



/* 

model = new DataStoreCatalog();

var myclass = model.addClass("Sample", "Samples");


var myclass2 = model.addClass("ExtendedSample", "ExtendedSamples", "Sample");

myclass2.addEventListener("onValidate", function);

var myatt = myclass.addAttribute("id", "storage", "long", "key");
var myatt = myclass.addAttribute("ID", "storage", "long", "key auto");

var myatt = myclass.addAttribute("att1", "storage", "string", "btree");
var myatt = myclass.addAttribute("att2", "storage", "long", "cluster");
var myatt = myclass.addAttribute("att3", "storage", "number");


var myatt = myclass.addAttribute("relAtt4", "relatedEntity", "class1", "class1");
var myatt = class1.addAttribute("relOther", "relatedEntities", "myclass", "relAtt4", {reversePath:true});

var myatt = myclass.addAttribute("relAtt5", "relatedEntity", "class3", "relAtt4.relAtt1.relAtt2");
var myatt = class3.addAttribute("relOther2", "relatedEntities", "myclass", "relAtt5", {reversePath:true});

var myatt = myclass.addAttribute("aliasAtt", "alias", "string", "relAtt4.att1");


var myatt = myclass.addAttribute("calcAtt", "calculated", "string");
myatt.onGet = function() { };
myatt.onSet = function(value) {};
myatt.onQuery = function(compOper, valueToCompare) {};
myatt.onSort = function(ascending) {};


myatt.addEventListener("onSave", function);


var myfunc = myclass.AddMethod("method1", "entity", function, "public");
var myfunc = myclass.AddMethod("method2", "dataClass", function, "publicOnServer");
var myfunc = myclass.AddMethod("method3", "entityCollection", function);



model.addSQLCatalog("unnom", { 
	hostname: "194.98.194.72",
	user: "wakandaqa",
	password: "wakandaqa",
	database: "benchdb",
	port: 3306,
	ssl: false
});

*/

function prepareDataStoreCatalog(cat){
	cat.mergeOutsideCatalog = DataStoreCatalog.prototype.mergeOutsideCatalog;
	cat.mergeSQLCatalog = DataStoreCatalog.prototype.mergeSQLCatalog;
	cat.addOutsideCatalog = DataStoreCatalog.prototype.mergeOutsideCatalog;
	cat.addSQLCatalog = DataStoreCatalog.prototype.mergeSQLCatalog;
	cat.getClass = DataStoreCatalog.prototype.getClass;
	cat.addClass = DataStoreCatalog.prototype.addClass;
	cat._outsideCatalogs = [];
	cat._outsideSQLCatalogs = [];
}

function DataStoreCatalog(catalogName, options) {
	var newcat = DynamicObj();
	prepareDataStoreCatalog(newcat);
	if (catalogName != null && options != null)
	{
		newcat.catalogName = catalogName;
		for (e in options)
		{
			newcat[e] = options[e];
		}
	}
    //return this;
    return newcat;
}

DataClass = DataStoreCatalog.prototype.DataClass = function(collectionName, scope, extendClass, otherProperties) {
    //var dataClass = this;
    var dataClass = DynamicObj();

    dataClass.properties = {};
    dataClass.entityMethods = {};
    dataClass.collectionMethods = {};
    dataClass.methods = {};
    dataClass.events = {};

    dataClass.setRestrictingQuery = DataClass.prototype.setRestrictingQuery;
    dataClass.setProperties = DataClass.prototype.setProperties;
    dataClass.Attribute = DataClass.prototype.Attribute;
    dataClass.addRelatedEntity = DataClass.prototype.addRelatedEntity;
    dataClass.addRelatedEntities = DataClass.prototype.addRelatedEntities;
    dataClass.addAttribute = DataClass.prototype.addAttribute;
    dataClass.addMethod = DataClass.prototype.addMethod;
    dataClass.addEventListener = DataClass.prototype.addEventListener;
    dataClass.removeAttribute = DataClass.prototype.removeAttribute;


    var props = dataClass.properties;
    //props.className = className;
    if (collectionName != null)
        props.collectionName = collectionName;
    if (extendClass != null) {
        //props.extendClass = extendClass;
        props["extends"] = extendClass;
    }

    scope = scope || "public";
    props.scope = scope;

    if (otherProperties != null && typeof otherProperties === "object") {
        for (var e in otherProperties) {
            props[e] = otherProperties[e];
        }
    }

    return dataClass;
}


DataStoreCatalog.prototype.mergeOutsideCatalog = function(catalogName, catalogPath, username, password) {
	if (this._outsideCatalogs == null)
		this._outsideCatalogs = [];
		
	if (typeof catalogPath === 'object')
	{
		var outcat = new DataStoreCatalog(catalogName, catalogPath);
	}
	else
	{
		var outcat = new DataStoreCatalog(catalogName, {
			name: catalogName || "",
			hostname: catalogPath,
			user: username || "",
			password: password || ""
		});
	}
	this._outsideCatalogs.push(outcat);
	return outcat;
}

DataStoreCatalog.prototype.mergeSQLCatalog = function(catalogName, outcat) {
	if (this._outsideSQLCatalogs == null)
		this._outsideSQLCatalogs = [];
	outcat = new DataStoreCatalog(catalogName, outcat);
	this._outsideSQLCatalogs.push(outcat);
	return outcat;
}

DataStoreCatalog.prototype.addOutsideCatalog = DataStoreCatalog.prototype.mergeOutsideCatalog;
DataStoreCatalog.prototype.addSQLCatalog = DataStoreCatalog.prototype.mergeSQLCatalog;



DataStoreCatalog.prototype.getClass = function(className)
{
    if (className == "" || className == null)
        throw ({ error: 12001, errorMessage: "DataClass name is invalid." });
    var result = this[className];
    if (result == null)
		result = this.addClass(className);
	return result;
}


DataStoreCatalog.prototype.addClass = function(className, collectionName, scope, extendClass, otherProperties) {
    //var dataClass = new this.DataClass();

    if (className == "" || className == null)
        throw ({ error: 12001, errorMessage: "DataClass name is invalid." });

	
    if (collectionName == null || collectionName == "")
        collectionName = className + "Collection";
        
        /*

    var props = dataClass.properties;
    props.className = className;
    props.collectionName = collectionName;
    if (extendClass != null) {
        //props.extendClass = extendClass;
        props["extends"] = extendClass;
    }

    scope = scope || "public";
    props.scope = scope;

    if (otherProperties != null && typeof otherProperties === "object") {
        for (var e in otherProperties) {
            props[e] = otherProperties[e];
        }
    }
    */
    
    var dataClass = new DataClass(collectionName || null, scope || null, extendClass || null, otherProperties || null);

    this[className] = dataClass;
    return dataClass;
};


DataStoreCatalog.prototype.DataClass.prototype.setRestrictingQuery = function(queryString)
{
	this.properties.restrictingQuery = { queryStatement: queryString };
}

DataStoreCatalog.prototype.DataClass.prototype.setProperties = function(newprops)
{
	var props = this.properties;
	for (var e in newprops) {
            props[e] = newprops[e];
        }
}


Attribute = DataStoreCatalog.prototype.DataClass.prototype.Attribute = function(kind, type, param4, otherProperties) {
    this.events = {};
 
    var attribute = this;
    var props = attribute;
   
    kind = kind || "storage";
    type = type || "string";

   // props.name = attributeName;
    props.kind = kind;
    props.type = type;

    var relationPath = null;
    var indexType = null;

	if (typeof param4 === "object" && param4 != null)
	{
		if (otherProperties == null)
			otherProperties = param4;
		param4 = null;
	}
	
    if (param4 != null) {
        if (kind === "storage")
            indexType = param4;
        else {
            relationPath = param4;
            if (kind === "relatedEntity" && relationPath == null)
                relationPath = type;
        }
    }

    if (relationPath != null)
        props.path = relationPath;

    if (indexType != null) {
        if (indexType === "key" || indexType === "key auto") {
            props.primKey = true;
            if (indexType === "key auto") {
                if (type === "uuid")
                    props.autogenerate = true;
                else
                    props.autosequence = true;
            }
        }
        else
            props.indexKind = indexType;
    }

    if (otherProperties != null && typeof otherProperties === "object") {
        for (var e in otherProperties) {
            props[e] = otherProperties[e];
        }
    }
    
    return this;
}


DataStoreCatalog.prototype.DataClass.prototype.addRelatedEntity = function(attributeName, type, param4, otherProperties) {
	return this.addAttribute(attributeName, "relatedEntity", type, otherProperties);
}


DataStoreCatalog.prototype.DataClass.prototype.addRelatedEntities = function(attributeName,type,param4,otherProperties) {
	return this.addAttribute(attributeName,"relatedEntities",type,otherProperties);
}


DataStoreCatalog.prototype.DataClass.prototype.addAttribute = function(attributeName, kind, type, param4, otherProperties) {
    //var attribute = new this.Attribute();
    //var props = attribute;

    if (attributeName == "" || attributeName == null)
        throw ({ error: 12002, errorMessage: "Attribute name is invalid." });

/*
    kind = kind || "storage";
    type = type || "string";

    props.name = attributeName;
    props.kind = kind;
    props.type = type;

    var relationPath = null;
    var indexType = null;

	if (typeof param4 === "object" && param4 != null)
	{
		if (otherProperties == null)
			otherProperties = param4;
		param4 = null;
	}
	
    if (param4 != null) {
        if (kind === "storage")
            indexType = param4;
        else {
            relationPath = param4;
            if (kind === "relatedEntity" && relationPath == null)
                relationPath = type;
        }
    }

    if (relationPath != null)
        props.path = relationPath;

    if (indexType != null) {
        if (indexType === "key" || indexType === "key auto") {
            props.primKey = true;
            if (indexType === "key auto") {
                if (type === "uuid")
                    props.autogenerate = true;
                else
                    props.autosequence = true;
            }
        }
        else
            props.indexKind = indexType;
    }

    if (otherProperties != null && typeof otherProperties === "object") {
        for (var e in otherProperties) {
            props[e] = otherProperties[e];
        }
    }
*/
	/*
    if (this.attributes == null)
        this.attributes = {};
    this.attributes[attributeName] = attribute;
        */
        
    var attribute = new Attribute(kind || null, type || null, param4 || null, otherProperties || null);

    this[attributeName] = attribute;
    return attribute;
};


DataStoreCatalog.prototype.DataClass.prototype.addMethod = function(funcName, kind, func, scope, otherProperties) {

    var meth;
    kind = kind || "dataClass";
    switch (kind.toLowerCase()) {
        case "dataclass":
            meth = this.methods;
            break;
        case "entitycollection":
            meth = this.collectionMethods;
            break;
        case "entity":
            meth = this.entityMethods;
            break;
        default:
            throw ({ error: 12004, errorMessage: "Method kind is invalid." });
            break;
    }
    if (funcName == "" || funcName == null)
        throw ({ error: 12003, errorMessage: "Method's name is invalid." });
    if (scope != null) {
        func.scope = scope;
    }

    meth[funcName] = func;

    return this;
}


DataStoreCatalog.prototype.DataClass.prototype.addEventListener = function(eventKind, func)
{
    var ok = false;
    switch(eventKind) {
        case "onLoad":
        case "onSet":
        case "onInit":
        case "onRemove":
        case "onValidate":
        case "onSave":
        case "onRestrictingQuery":
            ok = true;
    }
    if (ok)
    {
        this.events[eventKind] = func;
        return this;
    }
    else
        throw ({ error: 12005, errorMessage: "Event's kind is invalid." });
}


DataStoreCatalog.prototype.DataClass.prototype.removeAttribute = function(attname) {
    this[attname] = { kind: "removed", type:"string", events:{} };
}




DataStoreCatalog.prototype.DataClass.prototype.Attribute.prototype.addEventListener = DataStoreCatalog.prototype.DataClass.prototype.addEventListener;

DataStoreCatalog.prototype.DataClass.prototype.Attribute.prototype.setProperties  = function(newprops)
{
	var props = this;
	for (var e in newprops) {
            props[e] = newprops[e];
        }
}

model = DynamicObj();
prepareDataStoreCatalog(model);
