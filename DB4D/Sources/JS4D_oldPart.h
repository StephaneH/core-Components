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

class DB4DSelectionIterator
{
public:
	DB4DSelectionIterator(CDB4DSelection* inSel, bool inReadOnly, bool inAutoSave, CDB4DBaseContext* inContext);
	~DB4DSelectionIterator();

	void First(CDB4DBaseContext* inContext);
	void Next(CDB4DBaseContext* inContext);

	void ReleaseCurCurec();

	CDB4DRecord* GetCurRec(CDB4DBaseContext* inContext);

	CDB4DTable* GetTable() const
	{
		return fTable;
	}

	sLONG GetCurPos() const
	{
		return fCurPos;
	}

	sLONG GetCurRecID();

	XBOX::VError ReLoadCurRec(CDB4DBaseContext* inContext, bool readonly);

protected:
	CDB4DTable* fTable;
	CDB4DSelection* fSel;
	CDB4DRecord* fCurRec;
	sLONG fCurPos, fSelSize;
	bool fReadOnly, fAutoSave;
};



//======================================================


class VJSRecord : public XBOX::VJSClass<VJSRecord, CDB4DRecord>
{
public:
	typedef VJSClass<VJSRecord, CDB4DRecord>	inherited;

	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, CDB4DRecord* inRecord);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, CDB4DRecord* inRecord);
	static	void			GetProperty( XBOX::VJSParms_getProperty& ioParms, CDB4DRecord* inRecord);
	static	bool			SetProperty( XBOX::VJSParms_setProperty& ioParms, CDB4DRecord* inRecord);
	static	void			GetPropertyNames( XBOX::VJSParms_getPropertyNames& ioParms, CDB4DRecord* inRecord);
	static	void			GetDefinition( ClassDefinition& outDefinition);

	static void _Save(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DRecord* inRecord);  // Save()
	static void _GetID(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DRecord* inRecord);  // Number : GetID()
	static void _IsModified(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DRecord* inRecord);  // bool : IsModified()
	static void _IsNew(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DRecord* inRecord);  // bool : IsNew()
	static void _Drop(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DRecord* inRecord);  // Drop()
	static void _getTimeStamp(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DRecord* inRecord);  // Date : _getTimeStamp()

};



//======================================================


class VJSSelection : public XBOX::VJSClass<VJSSelection, CDB4DSelection>
{
public:
	typedef VJSClass<VJSSelection, CDB4DSelection>	inherited;

	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, CDB4DSelection* inSelection);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, CDB4DSelection* inSelection);
	static	void			GetProperty( XBOX::VJSParms_getProperty& ioParms, CDB4DSelection* inSelection);
	static	void			GetDefinition( ClassDefinition& outDefinition);

	static void _First(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection);  // SelectionIterator : First({bool ReadOnly})
	static void _SetReadOnly(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection);  // _SetReadOnly(bool)
	static void _Count(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection);  // Number : _Count()
	static void _OrderBy(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection);  // _OrderBy({Field : column, bool : ascent})
	static void _Add(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection);  // _Add(Selection | Record | Number)
	static void _ExportAsSQL(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection);  // _ExportAsSQL(Folder | StringURL, number : nbBlobsPerLevel, number : maxExportFileSize)

};



//======================================================


class VJSSelectionIterator : public XBOX::VJSClass<VJSSelectionIterator, DB4DSelectionIterator>
{
public:
	typedef VJSClass<VJSSelectionIterator, DB4DSelectionIterator>	inherited;

	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, DB4DSelectionIterator* inSelIter);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, DB4DSelectionIterator* inSelIter);
	static	void			GetProperty( XBOX::VJSParms_getProperty& ioParms, DB4DSelectionIterator* inSelIter);
	static	bool			SetProperty( VJSParms_setProperty& ioParms, DB4DSelectionIterator* inSelIter);
	static	void			GetPropertyNames( XBOX::VJSParms_getPropertyNames& ioParms, DB4DSelectionIterator* inSelIter);
	static	void			GetDefinition( ClassDefinition& outDefinition);

	static void _Next(XBOX::VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter);  // bool : Next()
	static void _Valid(XBOX::VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter);  // bool : Valid()
	static void _Loaded(XBOX::VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter);  // bool : Loaded()
	static void _Save(XBOX::VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter);  // Save()
	static void _GetID(XBOX::VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter);  // Number : GetID()
	static void _Reload(XBOX::VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter);  // Reload(bool | string : "ReadOnly")
	static void _IsModified(XBOX::VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter);  // bool : IsModified()
	static void _IsNew(XBOX::VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter);  // bool : IsNew()
	static void _Drop(XBOX::VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter);  // Drop()
	static void _Release(XBOX::VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter);  // Release()
	static void _getTimeStamp(XBOX::VJSParms_callStaticFunction& ioParms, DB4DSelectionIterator* inSelIter);   // Date : _getTimeStamp()

};



//======================================================

