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
ReportingDB4D = {};


ReportingDB4D.report = function(col, atts, groupBy){
	var xatts = atts.split(',');
	var xgroupBy = groupBy.split(',');
	atts = [];
	groupBy = [];
	
	var dataClass = col.getDataClass();

	xatts.forEach(function (att) {
	    var attname = att.trim();
	    if (dataClass.allowAttribute(attname, "read"))
		    atts.push(attname);
	});
	
	xgroupBy.forEach(function(att){
	    var attname = att.trim();
	    if (dataClass.allowAttribute(attname, "read"))
	        groupBy.push(att.trim());
	});
	
	var fullres = {
		rows: {},
		__orderByAttributes: groupBy,
		toArray: ReportingDB4D.asArray
	};
	
	if (col != null) {
		col.forEach(function(e){
			var subres;
			
			function subCompute(partialres){
				atts.forEach(function(att){
					var val = e[att];
					if (val != null) {
						var attres = partialres[att];
						if (attres == null) {
							attres = {
								count: 0,
								sum: 0
							};
							partialres[att] = attres;
						}
						attres.count++;
						if (attres.min == null || attres.min > val) 
							attres.min = val;
						if (attres.max == null || attres.max < val) 
							attres.max = val;
						
						if (typeof val === "number") {
							attres.sum += val;
							attres.average = attres.sum / attres.count;
						}
						
					}
				});
			}
			
			var parentres = fullres.rows;
			subCompute(fullres);
			
			groupBy.forEach(function(groupAtt){
				var attval = e[groupAtt] == null ? "__null__null" : e[groupAtt];
				var subres = parentres[attval];
				if (subres == null) {
					subres = {
						rows: {}
					};
					parentres[attval] = subres;
				}
				
				subCompute(subres);
				
				parentres = subres.rows;
			});
		});
	}
	
	
	return fullres;
}


ReportingDB4D.asArray = function(destproperties){
	var arr = [];
	destproperties = destproperties || {};
	var report = this;
	var orderBy = report.__orderByAttributes;
	
	function copyResult(val, res, level){
		var rows = res.rows;
		if (rows != null) {
			for (r in rows) {
				copyResult(r, rows[r], level+1);
			}
		}
		
		var elem = {};
		if (level != -1 && val != null) {
			var groupAtt = orderBy[level];
			elem[groupAtt] = val;
		}
		for (prop in res) {
			if (prop !== "rows"){
				var destPropObj = destproperties[prop];
				if (destPropObj == null)
					elem[prop] = res[prop].sum;
				else
				{
					for (destprop in destPropObj)
					{
						elem[prop+"_"+destprop] = res[prop][destprop];
					}
				}
			}
		}
		
		arr.push(elem);
			
	}
	
	copyResult(null, report,-1);
	
	return arr;
}

