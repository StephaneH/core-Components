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
RestImpExpAccess = {};

// errors

RestImpExpAccess.errWrongDataClass = { error : 1, errorMessage: "DataClass is missing or unknown" };
RestImpExpAccess.errWrongExportType = { error: 2, errorMessage: "Import/Export type is missing or unknown" };


// import/export types

RestImpExpAccess.exportType = {
	"csv": 1,
	"json": 2,
	"xml": 3,
	"tab": 4
}



// functions

RestImpExpAccess.setError = function(errnum, errorMessage, result)
{
	result.error = { error: errnum, errorMessage: errorMessage };
}

RestImpExpAccess.setError.scope = "private";

RestImpExpAccess.getExportType = function(type)
{
	var result = null;
	if (type != null)
		result = RestImpExpAccess.exportType[type.toLowerCase()];
	if (result == null)
		result = 0;
	return result;
}

RestImpExpAccess.getExportType.scope = "private";


RestImpExpAccess.allocateTempFile = function(dataStore, rootName, ext)
{
	var temp = dataStore.getTempFolder();
	
	var mutex = Mutex("tempFileAllocate");
	mutex.lock();
	var i = 1;
	var file = null;
	while (file == null)
	{
		file = File(temp, rootName+i+ext);
		if (file.exists)
		{
			file = null;
			++i;
		}
		else
			file.create();
	}
	mutex.unlock();
	return file;
}

RestImpExpAccess.allocateTempFile.scope = "private";


// public fonctions



/* options:
	dataClassName : string
	exportType : "csv", "xml", etc...
	attributes : array of string or null
	collectionRef: collection reference
	withHeaders: if CSV, first line is filled with columns names
	lineSeparator : string (default CRLF)
	columnSeparator: string (default comma for CSV or TAB for tab separated)
	quoteChar: string (default ")
	
*/

RestImpExpAccess.exportData = function(options)
{
	var result = {};
	options = options || {};
	var dataClassName = options.dataClassName;
	var dataClass = null;
	
	if (dataClassName != null)
		dataClass = ds[dataClassName];
	
	if (dataClass == null)
	{
		RestImpExpAccess.setError(RestImpExpAccess.errWrongDataClass, result);
	}
	else
	{
		var exportType = RestImpExpAccess.getExportType(options.exportType || null);
		if (exportType == 0)
		{
			RestImpExpAccess.setError(RestImpExpAccess.errWrongExportType, result);
		}
		else
		{
			options.lineSeparator = options.lineSeparator || "\n";
			if (exportType == 1)
			{
				options.columnSeparator = options.columnSeparator || ",";
				options.quoteChar = '"';
				options.withHeaders = true;
			}
			else
			{
				options.columnSeparator = options.columnSeparator || "\t";
				options.quoteChar = '';
				options.withHeaders = false;
			}
				
			var attributes = [];
			var atts = options.attributes;
			if (atts != null)
			{
				atts.forEach(function(attName)
				{
					var att = dataClass.attributes[attName];
					if (att != null && att.kind !== "relatedEntities")
					{
					    if (dataClass.allowAttribute(e, "read"))
						    attributes.push(att);
					}
				});
			}
			else
			{
				for (e in dataClass.attributes)
				{
					var att = dataClass.attributes[e];
					if (att.kind === "storage" || att.kind === "relatedEntity")
					{
					    if (dataClass.allowAttribute(e, "read"))
						    attributes.push(att);
					}
				}
			}
						
			var collection;
			var colref = options.collectionRef;
			if (colref == null)
				collection = dataClass.all();
			else
			{
				// build collection from collection reference
			}
			
			var ext = "";
			switch (exportType)
			{
				case 1:
					ext = ".csv";
					break;
				case 2:
					ext = ".json";
					break;
				case 3:
					ext = ".xml";
					break;
				case 4:
					ext = ".txt";
					break;
			}
			
			var tempfile = RestImpExpAccess.allocateTempFile(ds, "exportData", ext);
			var stream = new TextStream(tempfile, "write");
			options.stream = stream;
			
			if (options.progressInfo != null)
			{
				var progress = ProgressIndicator(collection.length, "Exporting Data", true, "", options.progressInfo);
				options.progress = progress;
			}
			
			var subresult;
			
			switch (exportType)
			{
				case 1:
					subresult = RestImpExpAccess.exportDataAsCSV(dataClass, attributes, collection, options);
					break;
					
				case 2:
					subresult = RestImpExpAccess.exportDataAsJSON(dataClass, attributes, collection, options);
					break;
					
				case 3:
					subresult = RestImpExpAccess.exportDataAsXML(dataClass, attributes, collection, options);
					break;
					
				case 4:
					subresult = RestImpExpAccess.exportDataAsCSV(dataClass, attributes, collection, options);
					break;
			}

			subresult = subresult || {};
			if (subresult.error != null)
				result.error = subresult.error;
		}
	}
	
	if (result.error == null)
	{
	    result.headers = result.headers || {};
	    result.headers['Content-disposition'] = "attachment; filename = exportData"+ext;
	    result.headers['Cache-Control'] = "no-cache, must-revalidate";
	    result.headers['Pragma'] = "no-cache";
	    result.headers['Content-Type'] = "application/force-download";
	    result.HTTPStream = stream;
	}
	return result;
}


RestImpExpAccess.exportDataAsCSV = function(dataClass, attributes, collection, options) 
{
	var result = {};
    if (options.withHeaders)
    {
		attributes.forEach(function(att) {
			options.stream.write(options.quoteChar+att.name+options.quoteChar+options.columnSeparator);
		});
		options.stream.write(options.lineSeparator);
    }
    
    collection.forEach(function(entity) {
		attributes.forEach(function(att) {
			var kind = att.kind;
			var type = att.type;
			var attVal = entity[att.name];
			var attResString = "";
			if (attVal == null)
			{
			}
			else
			{
				if (kind === "storage" || kind === "alias" || kind === "calculated")
				{
					if (type === "date")
						attResString = attVal.toISOString();
					else
						attResString = attVal.toString();
					
				}
				else if (kind === "relatedEntity")
				{
					var key = attVal.getKey();
					attResString = key.toString();
				}
				
				var res = "";
				for (var i = 0, nbchar = attResString.length; i < nbchar; ++i)
				{
					var c = attResString[i];
					if (c === options.quoteChar)
						res += (c+c);
					else
						res += c;
				}
				
				attResString = options.quoteChar + res + options.quoteChar;
			}
			options.stream.write(attResString+options.columnSeparator);
		});
        options.stream.write(options.lineSeparator);
    });
    
    return result;
}

