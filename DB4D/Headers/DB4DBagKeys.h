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
#ifndef __DB4DBAGKEYS__ 
#define __DB4DBAGKEYS__

#include "Kernel/VKernel.h"

#pragma pack( push, 8 )


CREATE_BAGKEY (____objectunic);


namespace d4
{
	CREATE_BAGKEY(name);
	CREATE_BAGKEY(type);
	CREATE_BAGKEY(kind);
	CREATE_BAGKEY(dbInfo);

	CREATE_BAGKEY(relationship);
	CREATE_BAGKEY(relationPath);
	CREATE_BAGKEY(sourceTable);
	CREATE_BAGKEY(sourceColumn);
	CREATE_BAGKEY(destinationTable);
	CREATE_BAGKEY(destinationColumn);

	CREATE_BAGKEY(columnName);
	CREATE_BAGKEY(fieldPos);
	CREATE_BAGKEY(readOnly);
	CREATE_BAGKEY(script);
	CREATE_BAGKEY(scriptKind);
	CREATE_BAGKEY(path);
	CREATE_BAGKEY(reversePath);
	CREATE_BAGKEY(primKey);
	CREATE_BAGKEY(indexKind);
	CREATE_BAGKEY(indexed);
	CREATE_BAGKEY(identifying);
	CREATE_BAGKEY(multiLine);
	CREATE_BAGKEY(simpleDate);

	CREATE_BAGKEY(onGet);
	CREATE_BAGKEY(onSet);
	CREATE_BAGKEY(onQuery);
	CREATE_BAGKEY(onSort);

	CREATE_BAGKEY(dataClasses);
	CREATE_BAGKEY(dataSource);
	CREATE_BAGKEY(extends);
	CREATE_BAGKEY(attributes);
	CREATE_BAGKEY(defaultTopSize);
	CREATE_BAGKEY(identifyingAttribute);
	CREATE_BAGKEY(key);
	CREATE_BAGKEY(optionnal);
	CREATE_BAGKEY(uuid);
	CREATE_BAGKEY(tablePos);
	CREATE_BAGKEY(singleEntityName);
	CREATE_BAGKEY(className);
	CREATE_BAGKEY(collectionName);
	CREATE_BAGKEY(matchTable);
	CREATE_BAGKEY(matchColumn);
	CREATE_BAGKEY(noEdit);
	CREATE_BAGKEY(noSave);
	CREATE_BAGKEY(allow);
	CREATE_BAGKEY(publishAsJSGlobal);
	CREATE_BAGKEY(allowOverrideStamp);

	CREATE_BAGKEY(queryStatement);
	CREATE_BAGKEY(applyToModel);
	CREATE_BAGKEY(restrictingQuery);
	CREATE_BAGKEY(top);
	CREATE_BAGKEY(orderBy);

	CREATE_BAGKEY(methods);
	CREATE_BAGKEY(source);
	CREATE_BAGKEY(from);
	CREATE_BAGKEY(applyTo);
	CREATE_BAGKEY(parameter);
	CREATE_BAGKEY(returnType);
	CREATE_BAGKEY(scope);
	CREATE_BAGKEY(serverOnly);
	CREATE_BAGKEY(userDefined);

	CREATE_BAGKEY(enumeration);
	CREATE_BAGKEY(item);
	CREATE_BAGKEY(value);

	CREATE_BAGKEY(minValue);
	CREATE_BAGKEY(maxValue);
	CREATE_BAGKEY(defaultValue);
	CREATE_BAGKEY(pattern);
	CREATE_BAGKEY(fixedLength);
	CREATE_BAGKEY(minLength);
	CREATE_BAGKEY(maxLength);
	CREATE_BAGKEY(autoComplete);
	CREATE_BAGKEY(cacheDuration);


	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( textAsBlob, XBOX::VBoolean, bool, false);

	CREATE_BAGKEY(defaultFormat);
	CREATE_BAGKEY(locale);
	CREATE_BAGKEY(format);
	CREATE_BAGKEY(presentation);
	CREATE_BAGKEY(sliderMin);
	CREATE_BAGKEY(sliderMax);
	CREATE_BAGKEY(sliderInc);

	CREATE_BAGKEY(And);
	CREATE_BAGKEY(Or);
	CREATE_BAGKEY(Not);
	CREATE_BAGKEY(subquery);

	CREATE_BAGKEY(__KEY);
	CREATE_BAGKEY(__ENTITIES);
	CREATE_BAGKEY(__RECID);
	CREATE_BAGKEY(__STAMP);
	CREATE_BAGKEY(__ERROR);

	CREATE_BAGKEY(__deferred);
	CREATE_BAGKEY(image);
	CREATE_BAGKEY(uri);
	CREATE_BAGKEY(dataURI);
	CREATE_BAGKEY(mediatype);
	CREATE_BAGKEY(height);
	CREATE_BAGKEY(width);

	CREATE_BAGKEY(permissions);
	CREATE_BAGKEY(action);
	CREATE_BAGKEY(group);
	CREATE_BAGKEY(groupID);
	CREATE_BAGKEY(resource);
	CREATE_BAGKEY(temporaryForcePermissions);
	CREATE_BAGKEY(entityModelPerm);
	CREATE_BAGKEY(read);
	CREATE_BAGKEY(update);
	CREATE_BAGKEY_2(xdelete, L"delete");
	CREATE_BAGKEY(create);
	CREATE_BAGKEY(execute);
	CREATE_BAGKEY(write);
	
	CREATE_BAGKEY(extraProperties);

	CREATE_BAGKEY(events);

	CREATE_BAGKEY(outsideCatalogs);
	CREATE_BAGKEY(user);
	CREATE_BAGKEY(password);

};


namespace DB4DBagKeys
{
	CREATE_BAGKEY(missing_datatable);
	CREATE_BAGKEY(matching_datatable);
	CREATE_BAGKEY(datatable_num);
	CREATE_BAGKEY(table_num);
	CREATE_BAGKEY(value);
	CREATE_BAGKEY(record);
	CREATE_BAGKEY(datatable);
	CREATE_BAGKEY(records_count);
	CREATE_BAGKEY(table_name);


	CREATE_BAGKEY_WITH_DEFAULT( name, XBOX::VString, "");
	CREATE_BAGKEY_WITH_DEFAULT( uuid, XBOX::VUUID, XBOX::VUUID::sNullUUID);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( id, XBOX::VLong, sLONG, 0);
	CREATE_BAGKEY( kind);
	CREATE_BAGKEY( type);
	CREATE_BAGKEY( state);

	CREATE_BAGKEY( base);
	CREATE_BAGKEY( base_extra);
	CREATE_BAGKEY( editor_base_info);

	CREATE_BAGKEY( relation);
	CREATE_BAGKEY( relation_extra);
	CREATE_BAGKEY( editor_relation_info);

	CREATE_BAGKEY( schema);

	CREATE_BAGKEY( table);
	CREATE_BAGKEY( table_extra);
	CREATE_BAGKEY( editor_table_info);

	CREATE_BAGKEY( field);
	CREATE_BAGKEY( field_extra);
	CREATE_BAGKEY( editor_field_info);

	CREATE_BAGKEY( journal_file);
	CREATE_BAGKEY( journal_file_enabled);

	CREATE_BAGKEY_WITH_DEFAULT( name_Nto1, XBOX::VString, "");
	CREATE_BAGKEY_WITH_DEFAULT( name_1toN, XBOX::VString, "");

	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( entry_autofill, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( entry_wildchar, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( entry_create, XBOX::VBoolean, bool, false);

	CREATE_BAGKEY_WITH_DEFAULT( integrity, XBOX::VString, "none");
	CREATE_BAGKEY( tip);
	CREATE_BAGKEY( comment);
	CREATE_BAGKEY( format);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( auto_load_Nto1, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( auto_load_1toN, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( foreign_key, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( choice_field, XBOX::VLong, sLONG, 0);
	CREATE_BAGKEY( related_field);
	CREATE_BAGKEY( index);
	CREATE_BAGKEY( field_ref);
	CREATE_BAGKEY( table_ref);
	CREATE_BAGKEY( index_ref);
	CREATE_BAGKEY( primary_key);
	CREATE_BAGKEY( field_name);
	CREATE_BAGKEY( field_uuid);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( prevent_journaling, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( hide_in_REST, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( styled_text, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( outside_blob, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( enterable, XBOX::VBoolean, bool, true);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( modifiable, XBOX::VBoolean, bool, true);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( mandatory, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( enumeration_id, XBOX::VLong, sLONG, -1);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( visible, XBOX::VBoolean, bool, true);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( trashed, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( never_null, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( not_null, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( unique, XBOX::VBoolean, bool, false);	// for field
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( unique_keys, XBOX::VBoolean, bool, false);	// for index
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( autosequence, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( autogenerate, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( store_as_utf8, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( store_as_UUID, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( limiting_length, XBOX::VLong, sLONG, 0);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( text_switch_size, XBOX::VLong, sLONG, 0);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( blob_switch_size, XBOX::VLong, sLONG, 0);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( compressed, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT( multi_line, XBOX::VString, "default");
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( reused_count, XBOX::VLong, sLONG, 0);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( position, XBOX::VLong, sLONG, 0);
//	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( fully_delete_records, XBOX::VBoolean, bool, false);		// sc 19/02/2008, doublon de "leave_tag_on_delete"
	CREATE_BAGKEY( coordinates);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( trigger_load, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( trigger_delete, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( trigger_update, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( trigger_insert, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( fields_ordering, XBOX::VLong, sLONG, 0);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( leave_tag_on_delete, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( keep_record_stamps, XBOX::VBoolean, bool, true);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( keep_record_sync_info, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( sql_schema_id, XBOX::VLong, sLONG, 1);
	CREATE_BAGKEY_WITH_DEFAULT(sql_schema_name, XBOX::VString, "DEFAULT_SCHEMA" );
	CREATE_BAGKEY( color);
	CREATE_BAGKEY( qt_spatial_settings);
	CREATE_BAGKEY( wedd);	// wedd dans les extra properties du data et la structure
	CREATE_BAGKEY( data_file_path);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( structure_opener, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( resman_stamp, XBOX::VLong, sLONG, 0);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( resman_marker, XBOX::VLong8, sLONG8, 0);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( source_code_available, XBOX::VBoolean, bool, true);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( source_code_stamp, XBOX::VLong, uLONG, 0);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( intel_code_stamp, XBOX::VLong, uLONG, 0);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( intel64_code_stamp, XBOX::VLong, uLONG, 0);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( last_opening_mode, XBOX::VLong, sLONG, 0);
	CREATE_BAGKEY_WITH_DEFAULT( package_name, XBOX::VString, "");
	CREATE_BAGKEY_WITH_DEFAULT( structure_file_name, XBOX::VString, "");
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( picture_format, XBOX::VLong, sLONG, 1);
	CREATE_BAGKEY_WITH_DEFAULT( picture_name, XBOX::VString, "");
	CREATE_BAGKEY_WITH_DEFAULT( font_name, XBOX::VString, "Arial");
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( font_size, XBOX::VLong, sLONG, 10);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( collapsed, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( displayable_fields_count, XBOX::VLong, sLONG, 0);
	CREATE_BAGKEY( collation_locale);	// locale id for VCollator
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( collator_ignores_middle_wildchar, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( consider_only_dead_chars_for_keywords, XBOX::VBoolean, bool, false);	//COL_ConsiderOnlyDeadCharsForKeywords
	CREATE_BAGKEY( temp_folder);
	CREATE_BAGKEY_WITH_DEFAULT( folder_selector, XBOX::VString, "data");	// data | structure | system | custom
	CREATE_BAGKEY_WITH_DEFAULT( path, XBOX::VString, "");	// file system path
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( open_data_one_version_more_recent_mode, XBOX::VLong, sLONG, 0);	// tells if one can open a data one major version more recent.
	
	// for extra data in CDBBaseContext
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( task_id, XBOX::VLong, sLONG, 0);
	CREATE_BAGKEY_WITH_DEFAULT( user_name, XBOX::VString, "");
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( user4d_id, XBOX::VLong, sLONG, 0);
	CREATE_BAGKEY_WITH_DEFAULT( host_name, XBOX::VString, "");
	CREATE_BAGKEY_WITH_DEFAULT( task_name, XBOX::VString, "");
	CREATE_BAGKEY_WITH_DEFAULT( client_uid, XBOX::VUUID, XBOX::VUUID::sNullUUID);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( is_remote_context, XBOX::VBoolean, bool,false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( client_version, XBOX::VLong, sLONG, 0);
	
	const XBOX::VValueBag::StBagElementsOrdering	ordering[] = 
	{
			{ base, {table,relation,index,base_extra}}
		,	{ base_extra, {temp_folder,journal_file,editor_base_info}}
		,	{ editor_base_info, {color}}
		,	{ schema, {} }
		,	{ table, {field,table_extra}}
		,	{ table_extra, {comment,editor_table_info}}
		,	{ editor_table_info, {color,coordinates}}
		,	{ field, {index_ref,field_extra}}
		,	{ field_extra, {qt_spatial_settings,tip,comment,editor_field_info}}
		,	{ editor_field_info, {color}}
		,	{ relation, {related_field, relation_extra}}
		,	{ related_field, {field_ref}}
		,	{0,{}}
	};
}

namespace LogFileBagKey
{
	CREATE_BAGKEY( journal_file );
	CREATE_BAGKEY( datalink );
	CREATE_BAGKEY( filepath );
	CREATE_BAGKEY( next_filepath );
	CREATE_BAGKEY( sequence_number );
};

namespace keys
{
};


#pragma pack( pop )

#endif
