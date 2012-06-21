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
#ifndef __JOURNAL_PARSER__
#define __JOURNAL_PARSER__

class DB4DJournalHeader : public IStreamable
{
	public:
							DB4DJournalHeader();
							~DB4DJournalHeader();

				bool		IsValid();
				bool		IsValid( const VUUID &inDataLink );
				void		Init( VUUID &inDataLink );

		virtual VError		ReadFromStream (VStream* ioStream, sLONG inParam = 0);
		virtual VError		WriteToStream (VStream* ioStream, sLONG inParam = 0) const;

	protected:
		uLONG				fFileSignature1;
		uLONG				fFileSignature2;
		VUUID				fDataLink;
};

class DB4DJournalParser : public VObject
{
	public:
							DB4DJournalParser(VFile* inJournalFile);
							~DB4DJournalParser();

				VError		Init( uLONG8 &outTotalOperationCount, VDB4DProgressIndicator *inProgressIndicator );
				VError		DeInit();

				VError		SetCurrentOperation( uLONG8 inOperation, CDB4DJournalData **outJournalData );
				VError		NextOperation( uLONG8 &outOperation, uLONG8 *outGlobalOperation, CDB4DJournalData **outJournalData );
							
				VError		SetEndOfJournal( uLONG8 inOperation ); /* after the call of SetEndOfJournal() you have to re-call the Init() method */
				uLONG8		CountOperations() const { return fTotalOperationCount; };

				bool		IsValid( const VUUID &inDataLink ) { return fFileHeader.IsValid(inDataLink); }
				
				uLONG8		GetFirstOperation() const	{ return fFirstOperation;}
				uLONG8		GetLastOperation() const	{ return fLastOperation;}
	
	protected:
				VError				_CheckFile( VDB4DProgressIndicator *inProgressIndicator );
				const VValueBag*	GetContextExtraData( uLONG8 inContextID) const;

		VFile				*fLogFile;
		VFileStream			*fFileStream;
		uLONG8				fTotalOperationCount;
		uLONG8				fCurrentOperation;
		uLONG8				fFirstOperation;
		uLONG8				fLastOperation;
		DB4DJournalHeader	fFileHeader;
		CDB4DJournalData	*fCurrentData;
		
		typedef std::map<uLONG8,VRefPtr<const VValueBag> >		MapOfExtraByID;
		MapOfExtraByID		fContextExtraByID;

		VArrayLong8			fModuloDataOffset;
};

#endif






