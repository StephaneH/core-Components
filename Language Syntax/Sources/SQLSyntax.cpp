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
#include "LanguageSyntaxHeaders.h"
#include "SQLSyntax.h"

// I hate having to do this, but unfortunately, it's the easiest way to handle
// the fact that we cannot include SQLParser.tab.h.  For starters, it's in an
// entirely different project.  So there's no way for me to just extern the enumeration
// it defines and snag it that way.  But also, it's a C only file using constructs that
// C++ compilers don't always like.  Also, it declares variables with entirely unknown
// types due to a lack of forward declarations.  When you sum everything up, it's just
// not meant to be (at least for today).
//
// So!  If you ever find yourself generating a new version of SQLParser.y, then it's possible
// for the header file it generates to get out of sync with the values I've copy and pasted
// here.  The solution is simple: if you run bison on the .y file to generate a new .h/.c
// pair, then be sure to copy and paste the new defines in here.  Thankfully, we do not 
// generate those files on every compile.
#define NAME 258
#define INTNUM 259
#define APPROXNUM 260
#define STRING 261
#define BACK_TICKED_STRING 262
#define SQL_HEXA_BLOB 263
#define SQL_KWORD_AMMSC 264
#define INSERT 265
#define SQL_KWORD_CHAR_LENGTH 266
#define SQL_KWORD_LENGTH 267
#define SQL_KWORD_LOWER 268
#define SQL_KWORD_UPPER 269
#define SQL_KWORD_CONCAT 270
#define SQL_KWORD_CONCATENATE 271
#define SQL_KWORD_LOCATE 272
#define SQL_KWORD_POSITION 273
#define SQL_KWORD_LEFT 274
#define SQL_KWORD_RIGHT 275
#define SQL_FUNC_REPLACE 276
#define SQL_KWORD_SUBSTRING 277
#define SQL_KWORD_LTRIM 278
#define SQL_KWORD_RTRIM 279
#define SQL_KWORD_TRIM 280
#define SQL_KWORD_REPEAT 281
#define SQL_KWORD_SPACE 282
#define SQL_KWORD_ASCII 283
#define SQL_KWORD_TRANSLATE 284
#define SQL_KWORD_CHAR 285
#define SQL_KWORD_BIT_LENGTH 286
#define SQL_KWORD_OCTET_LENGTH 287
#define SQL_KWORD_DATE_TO_CHAR 288
#define SQL_KWORD_ACOS 289
#define SQL_KWORD_ASIN 290
#define SQL_KWORD_ATAN 291
#define SQL_KWORD_COS 292
#define SQL_KWORD_COT 293
#define SQL_KWORD_SIN 294
#define SQL_KWORD_TAN 295
#define SQL_KWORD_ABS 296
#define SQL_KWORD_DEGREES 297
#define SQL_KWORD_EXP 298
#define SQL_KWORD_LOG 299
#define SQL_KWORD_LOG10 300
#define SQL_KWORD_RADIANS 301
#define SQL_KWORD_SQRT 302
#define SQL_KWORD_CEILING 303
#define SQL_KWORD_FLOOR 304
#define SQL_KWORD_SIGN 305
#define SQL_KWORD_ROUND 306
#define SQL_KWORD_TRUNC 307
#define SQL_KWORD_TRUNCATE 308
#define SQL_KWORD_RAND 309
#define SQL_KWORD_ATAN2 310
#define SQL_KWORD_POWER 311
#define SQL_KWORD_MOD 312
#define PI_MATH_CONST 313
#define SQL_KWORD_CURRENT_DATE 314
#define SQL_KWORD_CURRENT_TIME 315
#define SQL_KWORD_CURRENT_TIMESTAMP 316
#define SQL_KWORD_CURDATE 317
#define SQL_KWORD_CURTIME 318
#define SQL_KWORD_DAYOFMONTH 319
#define SQL_KWORD_DAYOFWEEK 320
#define SQL_KWORD_DAYOFYEAR 321
#define SQL_KWORD_MILLISECOND 322
#define SQL_KWORD_SECOND 323
#define SQL_KWORD_MINUTE 324
#define SQL_KWORD_HOUR 325
#define SQL_KWORD_DAY 326
#define SQL_KWORD_WEEK 327
#define SQL_KWORD_MONTH 328
#define SQL_KWORD_QUARTER 329
#define SQL_KWORD_YEAR 330
#define SQL_KWORD_DAYNAME 331
#define SQL_KWORD_MONTHNAME 332
#define SQL_KWORD_EXTRACT 333
#define COALESCE 334
#define NULLIF 335
#define CAST 336
#define SQL_KWORD_ROW_ID 337
#define SQL_KWORD_ROW_STAMP 338
#define SQL_KWORD_ROW_ACTION 339
#define SQL_KWORD_DATABASE_PATH 340
#define SQL_KWORD_SUM 341
#define SQL_KWORD_AVG 342
#define SQL_AGG_FUNC_MIN 343
#define SQL_AGG_FUNC_MAX 344
#define SQL_KWORD_COUNT 345
#define TABLE_REF_PRIORITY 346
#define SQL_KWORD_FULL 347
#define SQL_KWORD_CROSS 348
#define SQL_KWORD_INNER 349
#define SQL_KWORD_JOIN 350
#define ON 351
#define OR 352
#define AND 353
#define NOT 354
#define COMPARISON 355
#define MINUS 356
#define PLUS 357
#define DIVIDE 358
#define SQL_MULTIPLY 359
#define UMINUS 360
#define ADD 361
#define ALL 362
#define ALTER 363
#define ANY 364
#define AS 365
#define ASC 366
#define SQL_KWORD_AUTO_CLOSE 367
#define SQL_KWORD_AUTO_GENERATE 368
#define AUTO_INCREMENT 369
#define BETWEEN 370
#define BOTH 371
#define BY 372
#define CASCADE 373
#define CASE 374
#define CHARACTER 375
#define CHECK 376
#define COMMIT 377
#define CONSTRAINT 378
#define SQL_KWORD_CONSTRAINTS 379
#define CREATE 380
#define DATABASE 381
#define DATA_FILE 382
#define SQL_KWORD_DEFAULT 383
#define SQL_KWORD_DELETE 384
#define DESC 385
#define SQL_KWORD_DISABLE 386
#define DISTINCT 387
#define DROP 388
#define EXCLUSIVE 389
#define EXECUTE 390
#define ELSE 391
#define SQL_KWORD_ENABLE 392
#define END 393
#define ESCAPE 394
#define EXISTS 395
#define SQL_KWORD_FALSE 396
#define FN 397
#define SQL_KWORD_FOR 398
#define FOREIGN 399
#define SQL_KWORD_FROM 400
#define SQL_KWORD_GRANT 401
#define SQL_KWORD_GROUP 402
#define HAVING 403
#define SQL_KWORD_IF 404
#define IMMEDIATE 405
#define SQL_KWORD_IN 406
#define SQL_KWORD_INF 407
#define INDEX 408
#define SQL_KWORD_INDEXES 409
#define SQL_KWORD_INFILE 410
#define INTEGER 411
#define INTO 412
#define IS 413
#define SQL_KW_KEY 414
#define SQL_KWORD_LATEST 415
#define LEADING 416
#define LIKE 417
#define LIMIT 418
#define LISTBOX 419
#define LOCK 420
#define SQL_KWORD_LOCAL 421
#define SQL_KWORD_MODIFY 422
#define SQL_KWORD_NATURAL 423
#define NULLX 424
#define OFFSET 425
#define ORDER 426
#define SQL_KWORD_OUTER 427
#define SQL_KWORD_OVER 428
#define PRIMARY 429
#define SQL_KWORD_READ 430
#define SQL_KWORD_READ_WRITE 431
#define SQL_KWORD_REMOTE 432
#define SQL_KWORD_RENAME 433
#define REFERENCES 434
#define SQL_KWORD_REPLICATE 435
#define RESTRICT 436
#define SQL_KWORD_REVOKE 437
#define ROLLBACK 438
#define SQL_KWORD_SCHEMA 439
#define SELECT 440
#define SET 441
#define SHARE 442
#define SOME 443
#define SQL_KWORD_SQL_INTERNAL 444
#define SQL_KWORD_STAMP 445
#define START 446
#define STRUCTURE_FILE 447
#define SQL_KWORD_SYNCHRONIZE 448
#define TABLE 449
#define THEN 450
#define SQL_KWORD_TO 451
#define TRAILING 452
#define TRANSACTION 453
#define SQL_KWORD_TRUE 454
#define SQL_KWORD_TS 455
#define SQL_KW_UNIQUE 456
#define UNLOCK 457
#define UPDATE 458
#define SQL_KWORD_USE 459
#define SQL_KWORD_UTF16 460
#define SQL_KWORD_UTF8 461
#define VALUES 462
#define VIEW 463
#define WHEN 464
#define WHERE 465
#define SQL_KWORD_WITH 466
#define ALPHA_NUMERIC 467
#define TEXT 468
#define TIME 469
#define DURATION 470
#define SQL_KWORD_BOOLEAN 471
#define SQL4D_BYTE 472
#define SQL_KWORD_INT16 473
#define SQL_KWORD_INT32 474
#define SQL_KWORD_INT64 475
#define SQL_KWORD_REAL 476
#define SQL_KWORD_FLOAT 477
#define SQL_KWORD_UUID 478
#define SQL_KWORD_BLOB 479
#define PICTURE 480
#define VARCHAR 481
#define SQL_KWORD_INT 482
#define SMALLINT 483
#define BIT 484
#define VARYING 485
#define NUMERIC 486
#define SQL_KWORD_DOUBLE 487
#define PRECISION 488
#define SQL_KWORD_DATE 489
#define TIMESTAMP 490
#define INTERVAL 491
#define CLOB 492
#define DEBUG 493
#define THREADING 494
#define MODE 495
#define DIRECT 496
#define SYNC 497
#define ASYNC 498
#define ODBC_OJ 499
#define COMMA 500
#define PERIOD 501
#define LEFT_PARENTHESIS 502
#define RIGHT_PARENTHESIS 503
#define SEMICOLON 504
#define LEFT_CURLY_BRACKET 505
#define RIGHT_CURLY_BRACKET 506
#define DOUBLE_LESS_THAN 507
#define DOUBLE_GREATER_THAN 508
#define QUESTION_MARK 509
#define COLON 510
#define DOLLAR_SIGN 511

using namespace std;

class TokenListIterator {
private:
	vector< ILexerToken * > fTokens;
	vector< ILexerToken * >::iterator fIterator;
	bool fStarted;

public:
	TokenListIterator( vector< ILexerToken * > tokens ) {
		fTokens = tokens;
		fStarted = false;
	}

	bool Next() {
		if (!fStarted) {
			// If there are no tokens, we are done before we even start
			if (fTokens.empty())	return false;
			fStarted = true;
			fIterator = fTokens.begin();
			return true;
		} else {
			// We have started, so we want to advance to the
			// next item in the iterator, assuming we're not
			// already at the end
			if (fIterator == fTokens.end())	return false;
			return (++fIterator != fTokens.end());
		}
	}

	ILexerToken *Current() {
		return *fIterator;
	}

	void PutBack() {
		if (fStarted)	--fIterator;
	}

};

void VSQLSyntax::PutBackToken()
{
	if (fTokenList)	fTokenList->PutBack();
}

ILexerToken *VSQLSyntax::GetNextToken()
{
	if (!fTokenList)	return NULL;
	return (fTokenList->Next()) ? fTokenList->Current() : NULL;
}

ILexerToken *VSQLSyntax::PeekNextToken()
{
	if (!fTokenList)	return NULL;
	ILexerToken *ret = NULL;
	if (fTokenList->Next()) {
		ret = fTokenList->Current();
		fTokenList->PutBack();
	}

	return ret;
}

bool VSQLSyntax::ParseTokenList( vector< ILexerToken * > inTokens, SuggestionList &outSuggestions )
{
	// Set up the token list iterator so that we can easily parse our
	// way through it
	if (fTokenList)	delete fTokenList;
	fTokenList = new TokenListIterator( inTokens );


	// Now that we've finished all the housekeeping, we can actually parse
	// the list of tokens that we've been given.
	bool ret = ParseTopLevel( outSuggestions );

	// Now that we're done, let's clean up
	delete fTokenList;
	fTokenList = NULL;

	return ret;
}

bool VSQLSyntax::ParseLockStatement( SuggestionList &outSuggestions )
{
	// We expect the next token to be TABLE
	ILexerToken *token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "TABLE", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != TABLE)	return false;

	// Now we expect a table, which is either a simple name,
	// string or back-tic'ed string.
	token = GetNextToken();
	if (!token) {
		ParseTable( outSuggestions, token );
		return true;
	} else if (token->GetValue() != NAME && token->GetType() != ILexerToken::TT_STRING) return false;

	// Next we expect the IN keyword
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "IN EXCLUSIVE MODE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "IN SHARE MODE", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != SQL_KWORD_IN)	return false;

	// Next we expect to have either the EXCLUSIVE or the SHARE keyword
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "EXCLUSIVE MODE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "SHARE MODE", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != EXCLUSIVE && token->GetValue() != SHARE)	return false;

	// Finally, we expect to see the MODE keyword
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "MODE", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != MODE)	return false;

	// We're done!  Anything past this is considered superfluous.
	return true;
}

bool VSQLSyntax::ParseDebugStatement( SuggestionList &outSuggestions )
{
	// There's only one legal path to take for the DEBUG statement, so
	// we won't bother breaking it down into individual ParseFoo methods.
	// Instead, we will handle the entire statement here directly.
	
	// The next expected token is SET
	ILexerToken *token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "SET THREADING MODE", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != SET)	return false;

	// Now we expect THREADING
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "THREADING MODE", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != THREADING)	return false;

	// Now we expect MODE
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "MODE", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != MODE)	return false;

	// Now we expect SYNC, ASYNC or DIRECT
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "DIRECT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "SYNC", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "ASYNC", SuggestionInfo::eKeyword );
		return true;
	}

	// If we got here, then that means the user has already put in some sort of token for this
	// statement, and there's nothing else the user can legally enter.  So we return false to let
	// the caller know something's wrong
	return false;

}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseTable( SuggestionList &outSuggestions, ILexerToken * &outToken, VString *outTableName, bool inIncludeTables, bool inIncludeViews )
{
	// We are expected to parse a table -- the return value reflects whether we parsed a valid
	// table or not.  The outToken will always point to whatever the last token we parsed was.
	// This function is responsible for filling out any suggestions that may be needed.
	outToken = GetNextToken();

	if (!outToken) {
		// We expect to see a table here
		if ( inIncludeTables )
			outSuggestions.SuggestTables();
		if ( inIncludeViews )
			outSuggestions.SuggestViews();

		return eNothingLeftToParse;
	} else {
		// What we parsed needs to be a string (back tic'ed or otherwise), or a name.
		if (outToken->GetType() != ILexerToken::TT_STRING && outToken->GetValue() != NAME) {
			// Whatever this is: it's not a table
			return eIllegalValueParsed;
		} else {
			// We found a table and are good to go
			if (outTableName)	*outTableName = outToken->GetText();
			return eParseSucceeded;
		}
	}
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseColumnRef( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// We are expected to parse a column ref -- the return value reflects whether we parsed a valid
	// column ref or not.  The outToken will always point to whatever the last token we parsed was.
	// This function is responsible for filling out any suggestions that may be needed.
	outToken = GetNextToken();

	if (!outToken) {
		// Everything we expect here is either the name of a table (because the user is going to
		// be specifying a TABLE.COLUMN, or the name of a column directly.
		outSuggestions.SuggestColumns();
		ParseTable( outSuggestions, outToken );
		return eNothingLeftToParse;
	}

	switch (outToken->GetValue()) {
		case STRING: {
			// The only legal way to have a string as a columm reference is by following it with
			// a period and another string.  Anything else is illegal
			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			if (outToken->GetValue() != PERIOD)	return eIllegalValueParsed;

			// We expect a string next
			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			if (outToken->GetValue() != STRING)	return eIllegalValueParsed;

			return eParseSucceeded;
		} break;

		case NAME: {
			// A name can be by itself, or it can be followed by a period and another name.  Get the
			// name of the first token in case we find a period.
			VString tokenText = outToken->GetText();
			ILexerToken *tempToken = PeekNextToken();
			if (!tempToken)	return eParseSucceeded;	// A name by itself is legal
			if (tempToken->GetValue() != PERIOD)	return eParseSucceeded;		// A name by itself is legal
			// Consume the period
			GetNextToken();

			// Now we expect another name, that's the only legal thing allowed.  The first name was a 
			// table, so we want to find that table token in our symbol table and use it for showing
			// the user a list of columns.
			outToken = GetNextToken();
			if (!outToken) {
				IAutoCompleteSymbolTable::IAutoCompleteSymbol *sym = fSymTable->GetSymbolByName( tokenText );
				if (sym) {
					vector< IAutoCompleteSymbolTable::IAutoCompleteSymbol * > syms;
					sym->GetSubSymbols( IAutoCompleteSymbolTable::IAutoCompleteSymbol::eColumn, syms );
					for (vector< IAutoCompleteSymbolTable::IAutoCompleteSymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter) {
						(*iter)->GetCompletionText( tokenText );
						outSuggestions.Suggest( tokenText, SuggestionInfo::eName );
					}
				} else {
					outSuggestions.SuggestColumns();
				}
				return eNothingLeftToParse;
			}
			if (outToken->GetValue() != NAME)	return eIllegalValueParsed;

			return eParseSucceeded;
		} break;

		case BACK_TICKED_STRING: {
			// A back-tic'ed string can be by itself, or it can be followed by a period and another back-tic'ed string
			ILexerToken *tempToken = PeekNextToken();
			if (!tempToken)	return eParseSucceeded;	// A name by itself is legal
			if (tempToken->GetValue() != PERIOD)	return eParseSucceeded;		// A name by itself is legal
			// Consume the period
			GetNextToken();
		
			// Now we expect another back-tic'ed string, that's the only legal thing allowed
			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			if (outToken->GetValue() != BACK_TICKED_STRING)	return eIllegalValueParsed;

			return eParseSucceeded;
		} break;
	}

	return eIllegalValueParsed;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseColumnRefList( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// This is a list of one or more column refs, separated by commas.
	while (true) {
		// Parse the column
		ParserReturnValue ret = ParseColumnRef( outSuggestions, outToken );
		if (ret == eNothingLeftToParse || ret == eIllegalValueParsed)	return ret;

		// See if we've gotten a comma or not
		ILexerToken *tempToken = PeekNextToken();
		if (!tempToken)	return eParseSucceeded;
		if (tempToken->GetValue() != COMMA)	return eParseSucceeded;

		// We did get a comma, so consume it
		outToken = GetNextToken();
	}
}

bool VSQLSyntax::ParseCreateIndexStatement( SuggestionList &outSuggestions )
{
	// We've already determined that we're creating an INDEX, so the assumption is that the
	// next token will be the name of the index to create
	ILexerToken *token = GetNextToken();
	
	if (!token) {
		// There's nothing here to suggest
		return true;
	} else if (token->GetValue() != NAME)	return false;

	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "ON", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != ON)		return false;

	// Now that we know the name and have parsed the ON keyword, the user is expected to enter
	// a table
	ParserReturnValue helperRet = ParseTable( outSuggestions, token );
	if (eNothingLeftToParse == helperRet)		return true;
	else if (eIllegalValueParsed == helperRet)	return false;

	// We've parsed the table, so now we expect a left paren
	token = GetNextToken();
	if (!token)										return true;
	else if (token->GetValue() != LEFT_PARENTHESIS)	return false;

	// Now we expect a comma delimited list of column references
	helperRet = ParseColumnRefList( outSuggestions, token );
	if (eNothingLeftToParse == helperRet)		return true;
	else if (eIllegalValueParsed == helperRet)	return false;

	// Now we expect a right paren to close the statement
	token = GetNextToken();
	if (!token)											return true;
	else if (token->GetValue() != RIGHT_PARENTHESIS)	return false;

	// We're done!
	return true;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParsePrimaryOrForeignKeyDef( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// We're parsing the common portion of a PRIMARY or FOREIGN key definition.  We may have an optional
	// constraint that we need to parse before anything else.
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "CONSTRAINT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "PRIMARY KEY", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "FOREIGN KEY", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	}

	// Check to see if we have the optional constraint token
	if (outToken->GetValue() == CONSTRAINT) {
		// We expect to have a NAME token next
		outToken = GetNextToken();
		if (!outToken)	return eNothingLeftToParse;
		if (outToken->GetValue() != NAME)	return eIllegalValueParsed;

		// Get another token for whether it's a primary or foreign key
		outToken = GetNextToken();
		if (!outToken) {
			outSuggestions.Suggest( "PRIMARY KEY", SuggestionInfo::eKeyword );
			outSuggestions.Suggest( "FOREIGN KEY", SuggestionInfo::eKeyword );
			return eNothingLeftToParse;
		}
	}

	// Check to see whether we're a PRIMARY or a FOREIGN key
	bool bIsForeignKey = false;
	if (outToken->GetValue() == FOREIGN) {
		bIsForeignKey = true;
	} else if (outToken->GetValue() == PRIMARY) {
		// Do nothing -- we're good
	} else {
		return eIllegalValueParsed;
	}
	
	// We expect to see a KEY statement
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "KEY", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != SQL_KW_KEY)	return eIllegalValueParsed;

	// Now we expect a left paren
	outToken = GetNextToken();
	if (!outToken)	return eNothingLeftToParse;
	else if (outToken->GetValue() != LEFT_PARENTHESIS)	return eIllegalValueParsed;

	// Next comes a comma-delimited list of columns
	ParserReturnValue helperRet = ParseColumnRefList( outSuggestions, outToken );
	if (helperRet != eParseSucceeded)	return helperRet;

	// Finally, we expect the closing paren
	outToken = GetNextToken();
	if (!outToken)	return eNothingLeftToParse;
	else if (outToken->GetValue() != RIGHT_PARENTHESIS)	return eIllegalValueParsed;

	// If we're a foreign key, there may be more left to parse
	if (bIsForeignKey)	return ParseForeignKeyReferences( outSuggestions, outToken );
	else				return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseOnClause( SuggestionList &outSuggestions, class ILexerToken * &outToken, OnClause inAllowed, OnClause &outFound )
{
	// We expect to get either UPDATE or DELETE.  Either is fine, depending on what the user passes for
	// the inAllowed parameter.
	outToken = GetNextToken();
	if (!outToken) {
		if (inAllowed & eDelete) {
			outSuggestions.Suggest( "DELETE RESTRICT", SuggestionInfo::eKeyword );
			outSuggestions.Suggest( "DELETE CASCADE", SuggestionInfo::eKeyword );
		}

		if (inAllowed & eUpdate) {
			outSuggestions.Suggest( "UPDATE RESTRICT", SuggestionInfo::eKeyword );
			outSuggestions.Suggest( "UPDATE CASCADE", SuggestionInfo::eKeyword );
		}

		return eNothingLeftToParse;
	}

	// Check to see what kind we got
	if (outToken->GetValue() == SQL_KWORD_DELETE) {
		outFound = eDelete;
	} else if (outToken->GetValue() == UPDATE) {
		outFound = eUpdate;
	} else {
		// Whatever we have, it's not legal
		return eIllegalValueParsed;
	}

	// Check to see if we were allowed whatever it was that we found
	if ((inAllowed & outFound) == 0)	return eIllegalValueParsed;

	// The next keyword we expect is a RESTRICT or CASCADE to finish the clause
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "RESTRICT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "CASCADE", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != RESTRICT && outToken->GetValue() != CASCADE)	return eIllegalValueParsed;

	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseForeignKeyReferences( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// We need to parse out the references for a foreign key.  The first keyword we expect to
	// see is the REFERENCES
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "REFERENCES", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != REFERENCES)	return eIllegalValueParsed;

	// Now we expect a table that the user is referencing
	ParserReturnValue retHelper = ParseTable( outSuggestions, outToken );
	if (retHelper != eParseSucceeded)	return retHelper;

	// Now it gets a bit tricky.  We can have an optional comma-delimited column list, which is
	// optionally followed by either ON DELETE, ON UPDATE or both (in any order).  If the next token
	// is a left paren, then we have the optional column list.  If it's ON, then we've got an ON DELETE
	// or ON UPDATE clause.  But since all of these clauses are optional -- we rarely fail from this point onward.
	outToken = PeekNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "ON DELETE RESTRICT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "ON DELETE CASCADE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "ON UPDATE RESTRICT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "ON UPDATE CASCADE", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	}

	if (outToken->GetValue() == LEFT_PARENTHESIS) {
		GetNextToken();	// Eat it
		retHelper = ParseColumnRefList( outSuggestions, outToken );
		if (retHelper != eParseSucceeded)	return retHelper;

		// The next token needs to be a right paren
		outToken = GetNextToken();
		if (!outToken)	return eNothingLeftToParse;
		else if (outToken->GetValue() != RIGHT_PARENTHESIS)	return eIllegalValueParsed;

		// Peek at another token to see if we have one of the ON clauses
		outToken = PeekNextToken();
		if (!outToken)	return eNothingLeftToParse;
	}

	// Check to see if we have an ON clause
	if (outToken->GetValue() == ON) {
		GetNextToken();	// Eat it
		// Here's the crazy part -- the user can have two ON clauses, but they must be distinct.  So
		// we will parse the first one, then test to see if there's a second ON clause or not.
		OnClause found = eAll;
		retHelper = ParseOnClause( outSuggestions, outToken, eAll, found );
		if (retHelper != eParseSucceeded)	return retHelper;

		// Now check to see whether we have another ON clause or not
		outToken = PeekNextToken();
		if (!outToken) {
			// It's gotta be the other one!
			if (found == eDelete) {
				outSuggestions.Suggest( "ON UPDATE", SuggestionInfo::eKeyword );
			} else {
				outSuggestions.Suggest( "ON DELETE", SuggestionInfo::eKeyword );
			}
			return eNothingLeftToParse;
		} else if (outToken->GetValue() != ON)	return eParseSucceeded;
		
		GetNextToken();	// Consume the one we peeked at

		// We do!  So whatever we found last time is now restricted so that we can only parse the
		// remaining clause.
		return ParseOnClause( outSuggestions, outToken, (found == eDelete)?(eUpdate):(eDelete), found );
	} else {
		return eParseSucceeded;
	}
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseAlterTableAdd( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// We know we've got some sort of ADD statement, but there are several to choose from.  We
	// can tell whether we're dealing with a primary or forgein key definition by looking for either
	// the CONSTRAINT, PRIMARY or FOREIGN keywords.  It's a column definition if it's a NAME token. If
	// it's not, then we assume it's a foreign or primary key.
	outToken = PeekNextToken();
	if (!outToken) {
		// It could be either, so let them both take a crack at it
		ParseColumnDef( outSuggestions, outToken );
		ParsePrimaryOrForeignKeyDef( outSuggestions, outToken );
		return eNothingLeftToParse;
	}

	if (outToken->GetValue() == NAME) {
		return ParseColumnDef( outSuggestions, outToken );
	} else {
		return ParsePrimaryOrForeignKeyDef( outSuggestions, outToken );
	}
}

bool VSQLSyntax::ParseAlterTableStatement( SuggestionList &outSuggestions )
{
	// The first thing we expect is the table clause for the table being altered
	ILexerToken *token;
	VString tableName;
	ParserReturnValue helperRet = ParseTable( outSuggestions, token, &tableName );

	if (eNothingLeftToParse == helperRet)		return true;
	else if(eIllegalValueParsed == helperRet)	return false;

	// Now comes the action of how we want to alter the table
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "ADD PRIMARY KEY", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "ADD CONSTRAINT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "ADD FOREIGN KEY", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DISABLE REPLICATE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DISABLE STAMP", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DROP", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DROP PRIMARY KEY", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DROP CONSTRAINT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "ENABLE REPLICATE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "ENABLE STAMP", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "SET SCHEMA", SuggestionInfo::eKeyword );
		// The ENABLE STAMP and ENABLE REPLICATE format are preferred over the more archaic form
		// outSuggestions.Suggest( "REPLICATE DISABLE", SuggestionInfo::eKeyword );
		// outSuggestions.Suggest( "REPLICATE ENABLE", SuggestionInfo::eKeyword );
		// outSuggestions.Suggest( "STAMP ENABLE", SuggestionInfo::eKeyword );
		// outSuggestions.Suggest( "STAMP DISABLE", SuggestionInfo::eKeyword );
		return true;
	}

	// There are a lot of different ways you can alter a table, and the next few keywords will tell
	// us what manner the user is after.
	switch (token->GetValue()) {
		case SQL_KWORD_ENABLE:
		case SQL_KWORD_DISABLE: {
			// Now we expect to find either a STAMP or REPLICATE clause
			token = GetNextToken();
			if (!token) {
				outSuggestions.Suggest( "REPLICATE", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "STAMP", SuggestionInfo::eKeyword );
				return true;
			} else if (token->GetValue() != SQL_KWORD_STAMP &&
						token->GetValue() != SQL_KWORD_REPLICATE)	return false;
		} break;
		case SQL_KWORD_STAMP:
		case SQL_KWORD_REPLICATE: {
			// Now we expect to see an ENABLE or DISABLE clause
			token = GetNextToken();
			if (!token) {
				outSuggestions.Suggest( "DISABLE", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "ENABLE", SuggestionInfo::eKeyword );
				return true;
			} else if (token->GetValue() != SQL_KWORD_ENABLE && 
						token->GetValue() != SQL_KWORD_DISABLE)	return false;
		} break;

		case SET: {
			// This is the easiest of the cases -- we expect to see SCHEMA and the name
			token = GetNextToken();
			if (!token) {
				outSuggestions.Suggest( "SCHEMA", SuggestionInfo::eKeyword );
				return true;
			} else if (token->GetValue() != SQL_KWORD_SCHEMA)	return false;

			token = GetNextToken();
			if (!token) {
				outSuggestions.SuggestSchemas();
				return true;
			} else if (token->GetValue() != NAME)	return false;
		} break;

		case DROP: {
			// There are three forms of DROP: NAME, CONSTRAINT and PRIMARY, so we need another
			// token to tell which the user is after
			token = GetNextToken();
			if (!token) {
				outSuggestions.Suggest( "CONSTRAINT", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "PRIMARY KEY", SuggestionInfo::eKeyword );
				// It could be that the user is dropping a column from this table, so we want to suggest
				// the columns from the table.
				IAutoCompleteSymbolTable::IAutoCompleteSymbol *sym = fSymTable->GetSymbolByName( tableName );
				if (sym) {
					vector< IAutoCompleteSymbolTable::IAutoCompleteSymbol * > syms;
					sym->GetSubSymbols( IAutoCompleteSymbolTable::IAutoCompleteSymbol::eColumn, syms );
					for (vector< IAutoCompleteSymbolTable::IAutoCompleteSymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter) {
						VString name;
						(*iter)->GetCompletionText( name );
						outSuggestions.Suggest( name, SuggestionInfo::eName );
					}
				}
				return true;
			}

			if (token->GetValue() == NAME) {
				// We're done!  The user wants to drop the name
				return true;
			} else if (token->GetValue() == CONSTRAINT) {
				// We expect another token: the name of the constraint to drop
				token = GetNextToken();
				return (!token || token->GetValue() == NAME);
			} else if (token->GetValue() == PRIMARY) {
				// We expect another token: KEY
				token = GetNextToken();
				if (!token) {
					outSuggestions.Suggest( "KEY", SuggestionInfo::eKeyword );
					return true;
				} else if (token->GetValue() != SQL_KW_KEY)	return false;
			} else {
				return false;
			}
		} break;

		case ADD: {
			// ADD is the hardest case for us to parse, as the user could be adding
			// a foreign key, a primary key or a column definition.
			helperRet = ParseAlterTableAdd( outSuggestions, token );
			if (eNothingLeftToParse == helperRet)	return true;
			if (eIllegalValueParsed == helperRet)	return false;
			return true;
		} break;
		
		default: {
			return false;
		} break;
	}

	return true;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::Parse4DReference( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// We expect to get a 4D language reference token
	outToken = GetNextToken();
	if (!outToken)	return eNothingLeftToParse;
	
	if (outToken->GetValue() == DOUBLE_LESS_THAN) {
		// There are three forms of this command: <<NAME>>, <<$NAME>>, <<NAME NAME>>
		outToken = GetNextToken();
		if (!outToken) {
			// The <<NAME NAME>> form of this allows the user to reference 4D columns, where
			// the first name is a TABLE and the second name is a COLUMN.
			ParseTable( outSuggestions, outToken );
			outSuggestions.Suggest4DProcessVariables();
			outSuggestions.Suggest4DInterProcessVariables();
			return eNothingLeftToParse;
		}

		if (outToken->GetValue() == DOLLAR_SIGN) {
			// This is the easy case -- next comes a name, then the closing greater than pair
			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest4DLocalVariables();
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != NAME)			return eIllegalValueParsed;

			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			if (outToken->GetValue() != DOUBLE_GREATER_THAN)	return eIllegalValueParsed;
		} else if (outToken->GetValue() == NAME) {
			// This is a bit trickier as the next outToken could either be another name, or
			// a closing greater than pair.  If we get another NAME, then it's the name of a
			// column from the given table.
			VString tokenText = outToken->GetText();
			outToken = GetNextToken();
			if (!outToken) {
				// Suggest column names from the passed table, if it's a table at all
				IAutoCompleteSymbolTable::IAutoCompleteSymbol *sym = fSymTable->GetSymbolByName( tokenText );
				if (sym) {
					vector< IAutoCompleteSymbolTable::IAutoCompleteSymbol * > syms;
					sym->GetSubSymbols( IAutoCompleteSymbolTable::IAutoCompleteSymbol::eColumn, syms );
					for (vector< IAutoCompleteSymbolTable::IAutoCompleteSymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter) {
						VString name;
						(*iter)->GetCompletionText( name );
						outSuggestions.Suggest( name, SuggestionInfo::eName );
					}
				}
				return eNothingLeftToParse;
			}
			
			if (outToken->GetValue() == NAME) {
				// It was a name, so now we expect the next outToken to be a greater than closer
				outToken = GetNextToken();
				if (!outToken)	return eNothingLeftToParse;
			}

			if (outToken->GetValue() != DOUBLE_GREATER_THAN)	return eIllegalValueParsed;
		} else {
			return eIllegalValueParsed;
		}
	} else if (outToken->GetValue() == COLON) {
		// There are three forms of this syntax: :NAME, :NAME.NAME, :$NAME.  The
		// :NAME and :$NAME forms are for referencing 4D methods from SQL.  The
		// :NAME.NAME is the way to reference 4D columns from SQL.
		outToken = GetNextToken();
		if (!outToken)	{
			// If there's no outToken, then it's possible the user is attempting to
			// reference one of two things: either a 4D method name, or a column reference
			outSuggestions.Suggest4DMethods();
			outSuggestions.Suggest4DProcessVariables();
			outSuggestions.Suggest4DInterProcessVariables();
			ParseTable( outSuggestions, outToken );
			return eNothingLeftToParse;
		}

		if (outToken->GetValue() == DOLLAR_SIGN) {
			// We expect the next outToken to be the name of a 4D method
			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest4DMethods();
				outSuggestions.Suggest4DLocalVariables();
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != NAME)	return eIllegalValueParsed;
		} else if (outToken->GetValue() == NAME) {
			// There are two forms of this -- NAME.NAME, or just a regular name. So peek at the 
			// next outToken to see if it's a period or not.  But we need to pay attention to the
			// token text since it may be a column reference as well.
			VString tokenText = outToken->GetText();
			outToken = PeekNextToken();
			if (!outToken)	return eParseSucceeded;	// We can't count this as a failure because it's a legal parsing

			if (outToken->GetValue() == PERIOD) {
				// Consume that outToken!  We now know that the user is trying for a column
				// reference instead of a 4D method.
				GetNextToken();

				// Now make sure the next outToken is a NAME
				outToken = GetNextToken();
				if (!outToken) {
					if (!outToken) {
						// We have one name, which may be the name of a table -- so make suggestions about
						// columns if we did get a table reference.
						IAutoCompleteSymbolTable::IAutoCompleteSymbol *sym = fSymTable->GetSymbolByName( tokenText );
						if (sym) {
							vector< IAutoCompleteSymbolTable::IAutoCompleteSymbol * > syms;
							sym->GetSubSymbols( IAutoCompleteSymbolTable::IAutoCompleteSymbol::eColumn, syms );
							for (vector< IAutoCompleteSymbolTable::IAutoCompleteSymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter) {
								VString name;
								(*iter)->GetCompletionText( name );
								outSuggestions.Suggest( name, SuggestionInfo::eName );
							}
						}
						return eNothingLeftToParse;
					}
				}
				if (outToken->GetValue() != NAME)	return eIllegalValueParsed;
			}
		} else {
			return eIllegalValueParsed;
		}
	} else {
		return eIllegalValueParsed;
	}

	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::Parse4DReferenceList( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// The 4D method reference list is a series of one or more 4D methods, separated by
	// a comma.  Note that this requires at least one method.
	while (true) {
		// Parse the method
		ParserReturnValue ret = Parse4DReference( outSuggestions, outToken );
		if (ret == eNothingLeftToParse || ret == eIllegalValueParsed)	return ret;

		// See if we've gotten a comma or not
		ILexerToken *tempToken = PeekNextToken();
		if (!tempToken)	return eParseSucceeded;
		if (tempToken->GetValue() != COMMA)	return eParseSucceeded;

		// We did get a comma, so consume it
		outToken = GetNextToken();
	}
}

bool VSQLSyntax::ParseExecuteImmediateStatement( SuggestionList &outSuggestions )
{
	// The next token we expect is the IMMEDIATE token, followed by a 4D language starter
	ILexerToken *token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "IMMEDIATE", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != IMMEDIATE)	return false;

	// Now that we've gotten the EXECUTE IMMEDIATE out of the way, the next token will either
	// be a double less than sign, or a colon.
	ParserReturnValue helperRet = Parse4DReference( outSuggestions, token );
	if (eNothingLeftToParse == helperRet)		return true;
	else if(eIllegalValueParsed == helperRet)	return false;

	return true;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseReplicateItem( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// A replicate item is either a column reference, or one of the following keywords:
	// __ROW_ACTION or __ROW_STAMP

	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		outSuggestions.Suggest( "__ROW_ACTION", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "__ROW_STAMP", SuggestionInfo::eKeyword );

		// We also want to parse the column reference, knowing full-well that it won't find any tokens.
		// But this will add any suggestions to the list from that possible avenue as well.
		ParseColumnRef( outSuggestions, outToken );
		return eNothingLeftToParse;
	}

	// We've got a token, so consume it, and then check to see what type it is
	switch (temp->GetValue()) {
		case SQL_KWORD_ROW_STAMP:
		case SQL_KWORD_ROW_ACTION: {
			// We're done!
			outToken = GetNextToken();
			return eParseSucceeded;
		} break;
	}

	// It must be a column reference, so parse that instead, without consuming the token
	// we peeked at previously
	return ParseColumnRef( outSuggestions, outToken );
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseReplicateList( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// The replicate list is a series of one or more replicate items, separated by
	// a comma.  Note that this requires at least one replicate item.
	while (true) {
		// Parse the replicate
		ParserReturnValue ret = ParseReplicateItem( outSuggestions, outToken );
		if (ret == eNothingLeftToParse || ret == eIllegalValueParsed)	return ret;

		// See if we've gotten a comma or not
		ILexerToken *tempToken = PeekNextToken();
		if (!tempToken)	return eParseSucceeded;
		if (tempToken->GetValue() != COMMA)	return eParseSucceeded;

		// We did get a comma, so consume it
		outToken = GetNextToken();
	}
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseOptionalReplicationMergeMode( class SuggestionList &outSuggestions, class ILexerToken *&outToken )
{
	// This can either be REMOTE OVER LOCAL, or LOCAL OVER REMOTE.  However, it
	// is a purely optional clause as well.
	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		outSuggestions.Suggest( "LOCAL OVER REMOTE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "REMOTE OVER LOCAL", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	}

	bool bExpectSourceKeyword = false;
	if (temp->GetValue() == SQL_KWORD_REMOTE) {
		bExpectSourceKeyword = false;
	} else if (temp->GetValue() == SQL_KWORD_LOCAL) {
		bExpectSourceKeyword = true;
	} else {
		// This isn't an error because this is a purely optional clause
		return eUnexpectedTokenParsed;
	}

	// If we got here, we need to consume whichever token we found previously
	GetNextToken();

	// Now we expect the OVER keyword followed by either SOURCE or DESTINATION, depending
	// on which keyword the user previously entered.
	outToken = GetNextToken();
	if (!outToken) {
		if (bExpectSourceKeyword) {
			outSuggestions.Suggest( "OVER REMOTE", SuggestionInfo::eKeyword );
		} else {
			outSuggestions.Suggest( "OVER LOCAL", SuggestionInfo::eKeyword );
		}
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != SQL_KWORD_OVER)	return eIllegalValueParsed;

	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( (bExpectSourceKeyword)?("REMOTE"):("LOCAL"), SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	}

	if (outToken->GetValue() != ((bExpectSourceKeyword)?(SQL_KWORD_REMOTE):(SQL_KWORD_LOCAL))) {
		return eIllegalValueParsed;
	}

	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseForStampClause( SuggestionList &outSuggestions, ILexerToken *&outToken )
{
	// Now we expect to see the FOR keyword
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "FOR REMOTE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "FOR REMOTE STAMP", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != SQL_KWORD_FOR)	return eIllegalValueParsed;

	// We require the SOURCE keyword before seeing the next clause
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "REMOTE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "REMOTE STAMP", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != SQL_KWORD_REMOTE)	return eIllegalValueParsed;

	// It is possible that we see an optional STAMP keyword following the SOURCE keyword, so
	// peek at the next token to see if we got STAMP.  If we did, consume it.
	outToken = PeekNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "STAMP", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (outToken->GetValue() == SQL_KWORD_STAMP) {
		// We got the stamp, so consume it
		GetNextToken();
	}

	// Now we expect either an INTNUM, a SQL command parameter (?), or a 4D language reference, so peek
	// at the next token to see what we might have.
	outToken = PeekNextToken();
	if (!outToken)	return eNothingLeftToParse;
	else if (outToken->GetValue() == INTNUM) {
		// We have an INTNUM, so eat the token we just read and we can move on
		GetNextToken();
	} else {
		ParserReturnValue helperRet = ParseCommandParameter( outSuggestions, outToken );
		if (eNothingLeftToParse == helperRet || eIllegalValueParsed == helperRet)	return helperRet;
	}

	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseOptionalDestStampClause( SuggestionList &outSuggestions, ILexerToken *&outToken )
{
	// If this clause exists, it is going to be preceeded by a comma
	ILexerToken *token = PeekNextToken();
	if (!token)	return eNothingLeftToParse;
	else if (token->GetValue() != COMMA)	return eUnexpectedTokenParsed;

	// This means we found the COMMA and have a DESTINATION clause to deal with, so eat
	// the COMMA
	GetNextToken();	// Eat!

	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "LOCAL", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "LOCAL STAMP", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != SQL_KWORD_LOCAL)	return eIllegalValueParsed;

	// It is possible that we see an optional STAMP keyword following the DESTINATION keyword, so
	// peek at the next token to see if we got STAMP.  If we did, consume it.
	token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "STAMP", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (token->GetValue() == SQL_KWORD_STAMP) {
		// We got the stamp, so consume it
		outToken = GetNextToken();
	}

	// Now we expect either an INTNUM, a SQL command parameter (?), or a 4D language reference, so peek
	// at the next token to see what we might have.
	token = PeekNextToken();
	if (!token)	return eNothingLeftToParse;
	else if (token->GetValue() == INTNUM) {
		// We have an INTNUM, so eat the token we just read and we can move on
		outToken = GetNextToken();
	} else {
		ParserReturnValue helperRet = ParseCommandParameter( outSuggestions, outToken );
		if (eNothingLeftToParse == helperRet || eIllegalValueParsed == helperRet)	return helperRet;
	}

	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseOptionalLatestStampClause( SuggestionList &outSuggestions, ILexerToken *&outToken )
{
	// The LATEST clause is actually optional, so if we don't have the LATEST keyword,
	// we can just bail out.
	ILexerToken *token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "LATEST REMOTE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "LATEST REMOTE STAMP", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (token->GetValue() != SQL_KWORD_LATEST)	return eUnexpectedTokenParsed;

	// This means we found the LATEST keyword and have a to parse this clause, so eat
	// the keyword
	GetNextToken();	// Eat!

	// Now we expect to see the REMOTE clause
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "REMOTE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "REMOTE STAMP", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != SQL_KWORD_REMOTE)	return eIllegalValueParsed;

	// Again, there may be an optional STAMP keyword that we need to handle
	token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "STAMP", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (token->GetValue() == SQL_KWORD_STAMP) {
		// We got the stamp, so consume it
		outToken = GetNextToken();
	}

	// We now expect an INTNUM, SQL command or 4D method, just like we did for the FOR keyword
	token = PeekNextToken();
	if (!token)	return eNothingLeftToParse;
	else if (token->GetValue() == INTNUM) {
		// We have an INTNUM, so eat the token we just read and we can move on
		outToken = GetNextToken();
	} else {
		ParserReturnValue helperRet = ParseCommandParameter( outSuggestions, token );
		if (eNothingLeftToParse == helperRet || eIllegalValueParsed == helperRet)	return helperRet;
	}

	// Now there can be an optional LOCAL clause for us to handle.  If it exists, there
	// will be a COMMA for us to handle
	token = PeekNextToken();
	if (!token)	return eNothingLeftToParse;
	else if (token->GetValue() != COMMA)	return eUnexpectedTokenParsed;

	// This means we found the COMMA and have a LOCAL clause to deal with, so eat
	// the COMMA
	GetNextToken();	// Eat!

	// Next, the destination is expected
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "LATEST LOCAL", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "LATEST LOCAL STAMP", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != SQL_KWORD_LATEST)	return eIllegalValueParsed;

	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "LOCAL", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "LOCAL STAMP", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != SQL_KWORD_LOCAL)	return eIllegalValueParsed;

	// It is possible that we see an optional STAMP keyword following the LOCAL keyword, so
	// peek at the next token to see if we got STAMP.  If we did, consume it.
	token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "STAMP", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (token->GetValue() == SQL_KWORD_STAMP) {
		// We got the stamp, so consume it
		outToken = GetNextToken();
	}

	// Now we expect either an INTNUM, a SQL command parameter (?), or a 4D language reference, so peek
	// at the next token to see what we might have.
	token = PeekNextToken();
	if (!token)	return eNothingLeftToParse;
	else if (token->GetValue() == INTNUM) {
		// We have an INTNUM, so eat the token we just read and we can move on
		outToken = GetNextToken();
	} else {
		ParserReturnValue helperRet = ParseCommandParameter( outSuggestions, outToken );
		if (eNothingLeftToParse == helperRet || eIllegalValueParsed == helperRet)	return helperRet;
	}

	return eParseSucceeded;
}

bool VSQLSyntax::ParseReplicateStatement( SuggestionList &outSuggestions )
{
	// We have gotten a REPLICATE token from the user, so we want to parse the replicate statement.
	// The first thing we expect to find is the replicate list
	ILexerToken *token = NULL;
	ParserReturnValue helperRet = ParseReplicateList( outSuggestions, token );
	if (eNothingLeftToParse == helperRet)		return true;
	else if(eIllegalValueParsed == helperRet)	return false;

	// Now we expect to see the FROM keyword
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "FROM", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != SQL_KWORD_FROM)	return false;

	// The next thing we should see is a table which is being replicated from
	helperRet = ParseTable( outSuggestions, token );
	if (eNothingLeftToParse == helperRet)		return true;
	else if(eIllegalValueParsed == helperRet)	return false;

	// Now we can have an optional WHERE clause
	token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "WHERE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "LIMIT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "OFFSET", SuggestionInfo::eKeyword );
		ParseForStampClause( outSuggestions, token );
		return true;
	} else if (token->GetValue() == WHERE) {
		// We do have a WHERE clause, so consume the token and handle the search condition
		GetNextToken();	// Eat the WHERE token
		helperRet = ParseSearchCondition( outSuggestions, token );
		if (helperRet == eNothingLeftToParse)		return true;
		else if (helperRet == eIllegalValueParsed)	return false;

		token = PeekNextToken();
	}

	// Next, we can have an optional LIMIT clause
	if (!token) {
		outSuggestions.Suggest( "LIMIT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "OFFSET", SuggestionInfo::eKeyword );
		ParseForStampClause( outSuggestions, token );
		return true;
	} else if (token->GetValue() == LIMIT) {
		GetNextToken();		// Eat the LIMIT token
		helperRet = ParseLimitClause( outSuggestions, token );
		if (helperRet == eNothingLeftToParse)		return true;
		else if (helperRet == eIllegalValueParsed)	return false;

		token = PeekNextToken();
	}

	// And an optional OFFSET clause
	if (!token) {
		outSuggestions.Suggest( "OFFSET", SuggestionInfo::eKeyword );
		ParseForStampClause( outSuggestions, token );
		return true;
	} else if (token->GetValue() == OFFSET) {
		GetNextToken();		// Eat the OFFSET token
		helperRet = ParseOffsetClause( outSuggestions, token );
		if (helperRet == eNothingLeftToParse)		return true;
		else if (helperRet == eIllegalValueParsed)	return false;
	}

	// Then comes the mandatory FOR STAMP clause
	helperRet = ParseForStampClause( outSuggestions, token );
	if (eNothingLeftToParse == helperRet)		return true;
	else if(eIllegalValueParsed == helperRet)	return false;

	// Next, we have an optional DESTINATION clause.  
	helperRet = ParseOptionalDestStampClause( outSuggestions, token );
	if (eNothingLeftToParse == helperRet)		return true;
	else if(eIllegalValueParsed == helperRet)	return false;

	// There can be an optional replication merge mode clause that we may wish to handle.  If
	// we have no more tokens before trying to parse the merge mode, we also want to show the
	// LATEST SOURCE clause because the merge mode is optional.
	if (!PeekNextToken()) {
		ParseOptionalReplicationMergeMode( outSuggestions, token );
		ParseOptionalLatestStampClause( outSuggestions, token );
		return true;
	} else {
		helperRet = ParseOptionalReplicationMergeMode( outSuggestions, token );
		if (eNothingLeftToParse == helperRet)			return true;
		else if (eIllegalValueParsed == helperRet)		return false;
	}

	// Now there's an optional LATEST clause
	helperRet = ParseOptionalLatestStampClause( outSuggestions, token );
	if (eNothingLeftToParse == helperRet)		return true;
	else if(eIllegalValueParsed == helperRet)	return false;

	// Finally, we expect the replicate into clause, which is either a list of 4D methods, or
	// a table and column list.  Either way, the next keyword should be INTO
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "INTO", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != INTO)	return false;

	// Peek at the next token.  If it's a << or a :, then we've got the 4D language reference
	// list.  Otherwise we should have a table.  Those are our only two choices.
	token = PeekNextToken();
	if (!token) {
		// It could be a table, so we'll let the table parser add some suggestions
		ParseTable( outSuggestions, token );
		return eNothingLeftToParse;
	} else if (token->GetValue() == DOUBLE_LESS_THAN || token->GetValue() == COLON) {
		helperRet = Parse4DReferenceList( outSuggestions, token );
		if (eNothingLeftToParse == helperRet)		return true;
		else if(eIllegalValueParsed == helperRet)	return false;
		else										return true;
	}

	// If we got here, then we expect to parse a table
	helperRet = ParseTable( outSuggestions, token );
	if (eNothingLeftToParse == helperRet)		return true;
	else if(eIllegalValueParsed == helperRet)	return false;

	// Now we require an opening parenthesis
	token = GetNextToken();
	if (!token)	return true;
	else if (token->GetValue() != LEFT_PARENTHESIS)	return false;

	// Next comes the column list
	helperRet = ParseColumnRefList( outSuggestions, token );
	if (eNothingLeftToParse == helperRet)		return true;
	else if(eIllegalValueParsed == helperRet)	return false;

	// Finally, we require a closing parenthesis
	token = GetNextToken();
	return (!token || token->GetValue() == RIGHT_PARENTHESIS);
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseSynchronizeMergeMode( class SuggestionList &outSuggestions )
{
	// REMOTE OVER LOCAL or LOCAL OVER REMOTE are the only two merge modes
	// we can expect from the user.
	ILexerToken *token = GetNextToken();
	bool bExpectRemote = false;
	if (!token) {
		outSuggestions.Suggest( "LOCAL OVER REMOTE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "REMOTE OVER LOCAL", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (token->GetValue() == SQL_KWORD_LOCAL) {
		bExpectRemote = true;
	} else if (token->GetValue() != SQL_KWORD_REMOTE)	return eIllegalValueParsed;

	// Now we expect the OVER keyword
	token = GetNextToken();
	if (!token) {
		if (bExpectRemote) {
			outSuggestions.Suggest( "OVER REMOTE", SuggestionInfo::eKeyword );
		} else {
			outSuggestions.Suggest( "OVER LOCAL", SuggestionInfo::eKeyword );
		}
		return eNothingLeftToParse;
	} else if (token->GetValue() != SQL_KWORD_OVER)		return eIllegalValueParsed;

	// Now we expect either the LOCAL or the REMOTE keyword, depending on what
	// we've seen before.
	token = GetNextToken();
	if (!token) {
		if (bExpectRemote) {
			outSuggestions.Suggest( "REMOTE", SuggestionInfo::eKeyword );
		} else {
			outSuggestions.Suggest( "LOCAL", SuggestionInfo::eKeyword );
		}
		return eNothingLeftToParse;
	} else {
		if (bExpectRemote) {
			if (token->GetValue() != SQL_KWORD_REMOTE)		return eIllegalValueParsed;
		} else if (token->GetValue() != SQL_KWORD_LOCAL)	return eIllegalValueParsed;
	}

	return eParseSucceeded;
}

bool VSQLSyntax::ParseSynchronizeStatement( class SuggestionList &outSuggestions )
{
	// We start with an optional LOCAL keyword
	ILexerToken *token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "LOCAL", SuggestionInfo::eKeyword );
		ParseTable( outSuggestions, token );
		return eNothingLeftToParse;
	} else if (token->GetValue() == SQL_KWORD_LOCAL) {
		// We want to eat the optional token
		GetNextToken();
	}

	// Now we expect there to be a table
	ParserReturnValue helperRet = ParseTable( outSuggestions, token );
	if (eNothingLeftToParse == helperRet)		return true;
	else if (eIllegalValueParsed == helperRet)	return false;

	// Now comes a column list surrounded by parens
	token = GetNextToken();
	if (!token)		return true;
	else if (token->GetValue() != LEFT_PARENTHESIS)	return false;
	
	helperRet = ParseColumnRefList( outSuggestions, token );
	if (eNothingLeftToParse == helperRet)		return true;
	else if (eIllegalValueParsed == helperRet)	return false;

	token = GetNextToken();
	if (!token)		return true;
	else if (token->GetValue() != RIGHT_PARENTHESIS)	return false;

	// Now we expect the WITH keyword
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "WITH", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "WITH REMOTE", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != SQL_KWORD_WITH)	return false;

	// Optionally, we can have the remote keyword
	token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "REMOTE", SuggestionInfo::eKeyword );
		ParseTable( outSuggestions, token );
		return true;
	} else if (token->GetValue() == SQL_KWORD_REMOTE) {
		// Eat the optional token
		GetNextToken();
	}

	// We expect another table reference here
	helperRet = ParseTable( outSuggestions, token );
	if (eNothingLeftToParse == helperRet)		return true;
	else if (eIllegalValueParsed == helperRet)	return false;

	// Now comes a column list surrounded by parens
	token = GetNextToken();
	if (!token)		return true;
	else if (token->GetValue() != LEFT_PARENTHESIS)	return false;
	
	helperRet = ParseColumnRefList( outSuggestions, token );
	if (eNothingLeftToParse == helperRet)		return true;
	else if (eIllegalValueParsed == helperRet)	return false;

	token = GetNextToken();
	if (!token)		return true;
	else if (token->GetValue() != RIGHT_PARENTHESIS)	return false;

	// We expect to see a FOR keyword
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "FOR", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != SQL_KWORD_FOR)	return false;

	// Now we require a REMOTE keyword
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "REMOTE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "REMOTE STAMP", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != SQL_KWORD_REMOTE)	return false;

	// Optionally, there is a STAMP clause
	token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "STAMP", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() == SQL_KWORD_STAMP) {
		// Eat the optional token
		GetNextToken();
	}

	// Now we require an integer or a command parameter
	token = PeekNextToken();
	if (!token) {
		ParseCommandParameter( outSuggestions, token );
		return true;
	} else if (token->GetValue() == INTNUM) {
		// We have an INTNUM, so eat the token we just read and we can move on
		GetNextToken();
	} else {
		helperRet = ParseCommandParameter( outSuggestions, token );
		if (eNothingLeftToParse == helperRet )		return true;
		else if (eIllegalValueParsed == helperRet)	return false;
	}

	// There may be a comma for us to read
	token = PeekNextToken();
	if (!token)	return true;
	else if (token->GetValue() == COMMA) {
		// Eat the token
		GetNextToken();
	}

	// Now we require a LOCAL keyword
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "LOCAL", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "LOCAL STAMP", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != SQL_KWORD_LOCAL)	return false;

	// Optionally, there is a STAMP clause
	token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "STAMP", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() == SQL_KWORD_STAMP) {
		// Eat the optional token
		GetNextToken();
	}

	// Now we require an integer or a command parameter
	token = PeekNextToken();
	if (!token) {
		ParseCommandParameter( outSuggestions, token );
		return true;
	} else if (token->GetValue() == INTNUM) {
		// We have an INTNUM, so eat the token we just read and we can move on
		GetNextToken();
	} else {
		helperRet = ParseCommandParameter( outSuggestions, token );
		if (eNothingLeftToParse == helperRet )		return true;
		else if (eIllegalValueParsed == helperRet)	return false;
	}

	// Next comes the merge mode
	helperRet = ParseSynchronizeMergeMode( outSuggestions );
	if (eNothingLeftToParse == helperRet )		return true;
	else if (eIllegalValueParsed == helperRet)	return false;

	// Now we expect the LATEST REMOTE clause
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "LATEST REMOTE", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != SQL_KWORD_LATEST)	return false;

	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "REMOTE", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != SQL_KWORD_REMOTE)	return false;

	// Optionally, there is a STAMP clause
	token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "STAMP", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() == SQL_KWORD_STAMP) {
		// Eat the optional token
		GetNextToken();
	}

	// Now we require an integer or a command parameter
	token = PeekNextToken();
	if (!token) {
		ParseCommandParameter( outSuggestions, token );
		return true;
	} else if (token->GetValue() == INTNUM) {
		// We have an INTNUM, so eat the token we just read and we can move on
		GetNextToken();
	} else {
		helperRet = ParseCommandParameter( outSuggestions, token );
		if (eNothingLeftToParse == helperRet )		return true;
		else if (eIllegalValueParsed == helperRet)	return false;
	}

	// There may be a comma for us to read
	token = PeekNextToken();
	if (!token)	return true;
	else if (token->GetValue() == COMMA) {
		// Eat the token
		GetNextToken();
	}

	// Now we expect the LATEST LOCAL clause
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "LATEST LOCAL", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != SQL_KWORD_LATEST)	return false;

	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "LOCAL", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != SQL_KWORD_LOCAL)	return false;

	// Optionally, there is a STAMP clause
	token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "STAMP", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() == SQL_KWORD_STAMP) {
		// Eat the optional token
		GetNextToken();
	}

	// Now we require an integer or a command parameter
	token = PeekNextToken();
	if (!token) {
		ParseCommandParameter( outSuggestions, token );
		return true;
	} else if (token->GetValue() == INTNUM) {
		// We have an INTNUM, so eat the token we just read and we can move on
		GetNextToken();
	} else {
		helperRet = ParseCommandParameter( outSuggestions, token );
		if (eNothingLeftToParse == helperRet )		return true;
		else if (eIllegalValueParsed == helperRet)	return false;
	}

	return true;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseDataType( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// We want to parse one of the datatypes
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "ALPHA_NUMERIC", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "VARCHAR", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "TEXT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "CLOB", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "TIMESTAMP", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "INTERVAL", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DURATION", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "BOOLEAN", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "BIT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "BYTE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "SMALLINT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "INT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "INT16", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "INT32", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "INT64", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "NUMERIC", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "REAL", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "FLOAT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DOUBLE PRECISION", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "BLOB", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "BIT VARYING", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "PICTURE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "UUID", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	}

	// There are only two datatypes which span multiple tokens: BIT VARYING and DOUBLE PRECISION,
	// so we will have to handle those specially.  Also, this means that we cannot handle BIT 
	// as an easy case since it may have the VARYING token that follows it.
	switch (outToken->GetValue()) {
		case ALPHA_NUMERIC:
		case VARCHAR:
		case TEXT:
		case CLOB:
		case TIMESTAMP:
		case INTERVAL:
		case DURATION:
		case SQL_KWORD_BOOLEAN:
		case SQL4D_BYTE:
		case SMALLINT:
		case SQL_KWORD_INT16:
		case SQL_KWORD_INT32:
		case SQL_KWORD_INT64:
		case SQL_KWORD_INT:
		case NUMERIC:
		case SQL_KWORD_REAL:
		case SQL_KWORD_FLOAT:
		case SQL_KWORD_BLOB:
		case PICTURE:
		case SQL_KWORD_UUID: {
			// We're all set!
			return eParseSucceeded;
		} break;

		case SQL_KWORD_DOUBLE: {
			// The DOUBLE keyword is required to have PRECISION following it
			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest( "PRECISION", SuggestionInfo::eKeyword );
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != PRECISION)	return eIllegalValueParsed;
			return eParseSucceeded;
		} break;
		case BIT: {
			// The BIT keyword can optionally be followed by the VARYING keyword,
			// or it can be used by itself.  Peek at the next token to see whether
			// it is part of the datatype or not
			ILexerToken *temp = PeekNextToken();
			if (!temp) {
				outSuggestions.Suggest( "VARYING", SuggestionInfo::eKeyword );
				return eNothingLeftToParse;
			} else if (temp->GetValue() == VARYING) {
				// We found the VARYING keyword, so consume the token
				outToken = GetNextToken();
				return eParseSucceeded;
			}
			
			// Anything else is success since the BIT keyword is a datatype in itself
			return eParseSucceeded;
		} break;
	}

	// If we got here, we didn't parse anything we expected
	return eIllegalValueParsed;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseDataTypeEncoding( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// Data type encodings can be optional, or they can be UTF8/16.
	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		outSuggestions.Suggest( "UTF8", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "UTF16", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	}

	// If the token is one of our expected values, then consume it and we're done.  If it's
	// not, then we're still done since this is an optional clause.
	switch (temp->GetValue()) {
		case SQL_KWORD_UTF8:
		case SQL_KWORD_UTF16: {
			// Consume the token
			outToken = GetNextToken();
		} break;
	}
	
	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseOptionalColumnDefOption( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	ILexerToken *temp = PeekNextToken();

	if (!temp) {
		outSuggestions.Suggest( "NOT NULL", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "NOT NULL UNIQUE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "AUTO_INCREMENT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "CONSTRAINT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "PRIMARY KEY", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "AUTO_GENERATE", SuggestionInfo::eKeyword );

		return eNothingLeftToParse;
	}

	// We have some sort of token, so classify it and if it's one of our expected
	// optional values, then consume it
	if (temp->GetValue() == AUTO_INCREMENT ||
		temp->GetValue() == SQL_KWORD_AUTO_GENERATE) {
		// Ta da!  It worked
		outToken = GetNextToken();
		return eParseSucceeded;
	} else if (temp->GetValue() == NOT) {
		// We expect to see a NULL token, and possibly a UNIQUE token as well
		GetNextToken();  // Consume it!
		outToken = GetNextToken();
		if (!outToken) {
			outSuggestions.Suggest( "NULL", SuggestionInfo::eKeyword );
			outSuggestions.Suggest( "NULL UNIQUE", SuggestionInfo::eKeyword );
			return eNothingLeftToParse;
		} else if (outToken->GetValue() != NULLX)	return eIllegalValueParsed;

		// If we see a UNIQUE token, that's fine
		temp = PeekNextToken();
		if (!temp) {
			outSuggestions.Suggest( "UNIQUE", SuggestionInfo::eKeyword );

			// It's also possible that there are other column definition options that we may want
			// to suggest.  However, that assumes we want a list of these options.  I am making the
			// assumption that we're always going to be parsing these as part of a list, and so recursing
			// to get the next options for the suggestion list.
			ParseOptionalColumnDefOption( outSuggestions, outToken );

			return eNothingLeftToParse;
		} else if (temp->GetValue() == SQL_KW_UNIQUE) {
			// We need to consume this token
			GetNextToken();
		}

		// We're good!
		return eParseSucceeded;
	}

	// It is also possible that we will get a CONSTRAINT or PRIMARY keyword here.  If we see CONSTRAINT
	// then we will expect the PRIMARY KEY as well.
	bool bRequirePrimaryKey = false;
	if (temp->GetValue() == CONSTRAINT) {
		// Consume the CONSTRAINT token
		GetNextToken();

		// Make sure the next token is the name of the constraint
		outToken = GetNextToken();
		if (!outToken)	return eNothingLeftToParse;
		else if (outToken->GetValue() != NAME)	return eIllegalValueParsed;

		// We expect the next token to be the PRIMARY token
		temp = PeekNextToken();
		if (!temp) {
			outSuggestions.Suggest( "PRIMARY KEY", SuggestionInfo::eKeyword );
			return eNothingLeftToParse;
		}

		bRequirePrimaryKey = true;
	}

	if (temp->GetValue() == PRIMARY) {
		// Now we expect the KEY token to be next.  But first, consume the PRIMARY token
		GetNextToken();
		outToken = GetNextToken();
		if (!outToken) {
			outSuggestions.Suggest( "KEY", SuggestionInfo::eKeyword );
			return eNothingLeftToParse;
		} else if (outToken->GetValue() != SQL_KW_KEY)	return eIllegalValueParsed;

		// Now we're done
		return eParseSucceeded;
	} else if (bRequirePrimaryKey)	return eIllegalValueParsed;

	return eUnexpectedTokenParsed;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseOptionalColumnDefOptionsList( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// This is a purely optional list of column definition options.  There are no
	// separators between the items in the list.  We do not care how frequently the
	// user wants to complete the same items from this list (though that might be a
	// nice touch to add some day).
	while (true) {
		// Parse the optional column definition option
		ParserReturnValue ret = ParseOptionalColumnDefOption( outSuggestions, outToken );
		if (ret == eNothingLeftToParse || ret == eIllegalValueParsed)	return ret;
		// Since this is optional, an unexpected token means we're still good
		if (ret == eUnexpectedTokenParsed)	return eParseSucceeded;
	}
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseColumnDef( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// A column definition always starts with a name, and is then followed by either
	// a datatype, or a VARCHAR(INTNUM)
	outToken = GetNextToken();
	if (!outToken)	return eNothingLeftToParse;
	else if (outToken->GetValue() != NAME)	return eIllegalValueParsed;

	// Peek at the next token.  If it's a VARCHAR, we have a special code path to take.  
	// Otherwise, we expect to see a datatype
	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		// Any datatype will do, including VARCHAR
		ParseDataType( outSuggestions, outToken );
		return eNothingLeftToParse;
	}

	if (temp->GetValue() == VARCHAR) {
		// The VARCHAR production has two rules, though they're spaced pretty far apart in the grammar.  It
		// can either be VARCHAR by itself, or it can have an optional (INTNUM) clause.  First,
		// consume the token we peeked at since we no longer need it.
		GetNextToken();

		// Now we see if we have an open paren
		outToken = PeekNextToken();
		if (!outToken)	return eNothingLeftToParse;
		else if (outToken->GetValue() == LEFT_PARENTHESIS) {
			// We do have one, so let's eat it and parse the rest of the production
			GetNextToken();

			// Next we want to see an INTNUM
			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != INTNUM)	return eIllegalValueParsed;
			
			// Finally, we expect a closing paren
			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != RIGHT_PARENTHESIS)	return eIllegalValueParsed;
		}
	} else {
		// We expect to get a regular datatype definition
		ParserReturnValue helperRet = ParseDataType( outSuggestions, outToken );
		if (eParseSucceeded != helperRet)	return helperRet;
	}

	// Next we need to handle the optional datatype encoding clause
	ParserReturnValue helperRet = ParseDataTypeEncoding( outSuggestions, outToken );
	// If the return value is that there's nothing left to parse, we also want to include
	// the list of options the user can pick from
	if (eNothingLeftToParse == helperRet)	ParseOptionalColumnDefOptionsList( outSuggestions, outToken );
	if (eParseSucceeded != helperRet)	return helperRet;

	// Finally, there is an optional list of column definition options.  This list
	// does not have any separators to it.
	return ParseOptionalColumnDefOptionsList( outSuggestions, outToken );
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseOptionalTableProperty( class SuggestionList &outSuggestions, class ILexerToken *&outToken )
{
	// Check to see if there are any optional table properties that we need to parse.
	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		// The ENABLE STAMP and ENABLE REPLICATE format are preferred over the more archaic form
		// outSuggestions.Suggest( "REPLICATE DISABLE", SuggestionInfo::eKeyword );
		// outSuggestions.Suggest( "REPLICATE ENABLE", SuggestionInfo::eKeyword );
		// outSuggestions.Suggest( "STAMP ENABLE", SuggestionInfo::eKeyword );
		// outSuggestions.Suggest( "STAMP DISABLE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "ENABLE STAMP", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DISABLE STAMP", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "ENABLE REPLICATE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DISABLE REPLICATE", SuggestionInfo::eKeyword );

		return eNothingLeftToParse;
	}

	switch (temp->GetValue()) {
		case SQL_KWORD_ENABLE:
		case SQL_KWORD_DISABLE: {
			GetNextToken();  // Consume!

			// We expect to see a STAMP or REPLICATE token
			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest( "STAMP", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "REPLICATE", SuggestionInfo::eKeyword );
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != SQL_KWORD_STAMP &&
						outToken->GetValue() != SQL_KWORD_REPLICATE)	return eIllegalValueParsed;

			return eParseSucceeded;
		} break;
		case SQL_KWORD_REPLICATE:
		case SQL_KWORD_STAMP: {
			GetNextToken();  // Consume!

			// We expect to see an ENABLE or DISABLE token
			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest( "ENABLE", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "DISABLE", SuggestionInfo::eKeyword );
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != SQL_KWORD_ENABLE &&
						outToken->GetValue() != SQL_KWORD_DISABLE)	return eIllegalValueParsed;

			return eParseSucceeded;
		} break;

		default: {
			// If we got something other than one of the expected values, that's ok because
			// this is an entirely optional clause
			return eUnexpectedTokenParsed;
		} break;
	}

	return eIllegalValueParsed;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseOptionalTablePropertyList( class SuggestionList &outSuggestions, class ILexerToken *&outToken )
{
	// This is a purely optional list of table properties.  There are no separators between the items in the list.  
	// We do not care how frequently the user wants to complete the same items from this list (though that might be a
	// nice touch to add some day).
	while (true) {
		// Parse the optional column definition option
		ParserReturnValue ret = ParseOptionalTableProperty( outSuggestions, outToken );
		if (ret == eNothingLeftToParse || ret == eIllegalValueParsed)	return ret;
		// Since this is optional, an unexpected token means we're still good
		if (ret == eUnexpectedTokenParsed)	return eParseSucceeded;
	}
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseBaseTableElement( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// A base table element is either a column definition, or a primary/foreign key.  To tell
	// the difference between which is expected here, we will peek at the first token.  If it's
	// a NAME, then we know it's a column definition.  If it's not, we will assume it is a primary
	// or foreign key definition.
	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		// Let them both fill out some suggestions
		ParseColumnDef( outSuggestions, outToken );
		ParsePrimaryOrForeignKeyDef( outSuggestions, outToken );
		return eNothingLeftToParse;
	}

	if (temp->GetValue() == NAME) {
		// It's a column definition
		return ParseColumnDef( outSuggestions, outToken );
	} else {
		// It's a foreign or primary key definition
		return ParsePrimaryOrForeignKeyDef( outSuggestions, outToken );
	}
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseBaseTableElementList( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// The base table list is a series of one or more base table items, separated by
	// a comma.  Note that this requires at least one base table item.
	while (true) {
		// Parse the base table definition
		ParserReturnValue ret = ParseBaseTableElement( outSuggestions, outToken );
		if (ret == eNothingLeftToParse || ret == eIllegalValueParsed)	return ret;

		// See if we've gotten a comma or not
		ILexerToken *tempToken = PeekNextToken();
		if (!tempToken)	return eParseSucceeded;
		if (tempToken->GetValue() != COMMA)	return eParseSucceeded;

		// We did get a comma, so consume it
		outToken = GetNextToken();
	}
}

bool VSQLSyntax::ParseCreateTableStatement( SuggestionList &outSuggestions )
{
	// We know that we're creating a table, and we've parsed the TABLE token already.  So 
	// the first thing to check for is the optional IF NOT EXISTS clause.
	ILexerToken *token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "IF NOT EXISTS", SuggestionInfo::eKeyword );
		return true;
	}

	// If we got a token and it's an IF, we require the NOT EXISTS clause as well
	if (token->GetValue() == SQL_KWORD_IF) {
		// Consume the IF token
		GetNextToken();

		// Now we expect to get the NOT EXISTS clause
		token = GetNextToken();
		if (!token) {
			outSuggestions.Suggest( "NOT EXISTS", SuggestionInfo::eKeyword );
			return true;
		} else if (token->GetValue() != NOT)	return false;

		token = GetNextToken();
		if (!token) {
			outSuggestions.Suggest( "EXISTS", SuggestionInfo::eKeyword );
			return true;
		} else if (token->GetValue() != EXISTS)	return false;
	}

	// Now we expect a NAME, optionally followed by a .NAME
	token = GetNextToken();
	if (!token)	return true;
	if (token->GetValue() != NAME)	return false;

	// Check to see if we've gotten a period for the optional clause
	token = GetNextToken();
	if (!token)	return true;
	if (token->GetValue() == PERIOD) {
		// Check for the extra NAME
		token = GetNextToken();
		if (!token)	return true;
		else if (token->GetValue() != NAME)	return false;

		// Now get another token
		token = GetNextToken();
		if (!token)	return true;
	}

	// We expect a left paren
	if (token->GetValue() != LEFT_PARENTHESIS)	return false;

	// Now we're looking for a base table element list
	ParserReturnValue helperRet = ParseBaseTableElementList( outSuggestions, token );
	if (helperRet == eNothingLeftToParse)		return true;
	else if (helperRet == eIllegalValueParsed)	return false;

	// Followed by a right paren
	token = GetNextToken();
	if (!token)	return true;
	else if (token->GetValue() != RIGHT_PARENTHESIS)	return false;

	// Optionally, we may have some table properties defined
	helperRet = ParseOptionalTablePropertyList( outSuggestions, token );
	if (helperRet == eNothingLeftToParse)		return true;
	else if (helperRet == eIllegalValueParsed)	return false;

	return true;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseTerminal( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// We need to parse a terminal, which could, in theory parse another expression (such as
	// is the case with the () operators).  The assumption is that we will only be parsing 
	// something which isn't a part of a binary operator like +, -, etc.  This algorithm is 
	// based off the Shunting Yard Algorithm.
	ILexerToken *temp = PeekNextToken();

	if (!temp) {
		ParseLiteral( outSuggestions, outToken );
		ParseCaseExpression( outSuggestions, outToken );
		Parse4DReference( outSuggestions, outToken );
		ParseColumnRef( outSuggestions, outToken );
		ParseScalarFunction( outSuggestions, outToken );
		return eNothingLeftToParse;
	}

	// Check to see what the token's value to determine how to handle it.  It may be a terminal, or
	// it may be a recusive call.  We want to handle terminals first to avoid possible infinite loops.
	switch (temp->GetValue()) {
		case LEFT_CURLY_BRACKET: {
			// We have to peek at the next token, because this could either be the start of a function,
			// or a literal in the form { TS STRING }, { d STRING } or { t STRING }.  Consume the current token since we're handling
			// one of these two cases.
			GetNextToken();  // Consume!
			ILexerToken *temp = PeekNextToken();
			if (!temp) {
				outSuggestions.Suggest( "TS", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "D", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "T", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "FN", SuggestionInfo::eKeyword );
				return eNothingLeftToParse;
			}

			if (temp->GetValue() == SQL_KWORD_TS || temp->GetText()=="D" || temp-> GetText()=="T") {
				// Put the curly brace back
				PutBackToken();
				return ParseLiteral( outSuggestions, outToken );
			} else if (temp->GetValue() == FN) {
				// Put the curly brace back
				PutBackToken();
				return ParseScalarFunction( outSuggestions, outToken );
			} else {
				return eIllegalValueParsed;
			}
		} break;
		case SQL_KWORD_TRUE:
		case SQL_KWORD_FALSE:
		case INTNUM:
		case APPROXNUM: {
			// These are literals and so they've terminated the expression.  Consume the token and we're done
			return ParseLiteral( outSuggestions, outToken );
		} break;
		case PLUS:
		case MINUS: {
			// These expect a terminal to follow immediately after.  So consume the token and parse again
			outToken = GetNextToken();
			return ParseTerminal( outSuggestions, outToken );
		} break;
		case LEFT_PARENTHESIS: {
			// This expects another scalar expression followed by a right paren, so consume the token and continue;
			outToken = GetNextToken();
			ParserReturnValue helperRet = ParseScalarExpression( outSuggestions, outToken );
			if (helperRet != eParseSucceeded)	return helperRet;
			outToken = GetNextToken();
			if (!outToken)										return eNothingLeftToParse;
			else if (outToken->GetValue() != RIGHT_PARENTHESIS)	return eIllegalValueParsed;
			else												return eParseSucceeded;
		} break;
		case QUESTION_MARK: {
			// This is a SQL command parameter, and we're done.  Consume the token and get out of here
			outToken = GetNextToken();
			return eParseSucceeded;
		} break;
		case DOUBLE_LESS_THAN:
		case COLON: {
			// This is a 4D reference, so parse it and we're done
			return Parse4DReference( outSuggestions, outToken );
		} break;
		case NAME:
		case BACK_TICKED_STRING: {
			// This is a column reference, so parse it and we're done
			return ParseColumnRef( outSuggestions, outToken );
		} break;
		case STRING: {
			// Strings are fun... if it's followed by a PERIOD token, then
			// it is a column reference.  If it's not, then it's a literal
			// string token.  However, this requires an additional token of
			// lookahead, which isn't available to us without consuming the
			// string token we've just peeked at.  So we cannot use the helper
			// method for this -- we just have to parse the column reference
			// or literal ourselves.
			outToken = GetNextToken();	// Consume

			// Check to see if we have a PERIOD.  If we do, this is a column
			// reference.  If not, we're done
			temp = PeekNextToken();
			if (!temp)	return eParseSucceeded;
			if (temp->GetValue() == PERIOD) {
				// This is a column reference, so consume the token and move on
				GetNextToken();	// Consume
				outToken = GetNextToken();
				if (!outToken)								return eNothingLeftToParse;
				else if (outToken->GetValue() != STRING)	return eIllegalValueParsed;
				else										return eParseSucceeded;
			} else {
				return eParseSucceeded;
			}
		} break;
		case CASE: {
			return ParseCaseExpression( outSuggestions, outToken );
		} break;

		default: {
			// The only other thing we could possibly be is a scalar function, so
			// attempt to parse as that
			return ParseScalarFunction( outSuggestions, outToken );
		} break;
	}

	return eIllegalValueParsed;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseScalarFunction( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// The grammar hard-codes all of the scalar functions, as well as their parameter lists. 
	// However, I'm not going to try to monkey with that.  Instead, I'm going to test to see
	// whether the next token is the name of a scalar function.  If it's not, we're done and
	// something illegal has happened.  If it is, then I am going to parse an arbitrary number
	// of parameters enclosed in parens.  Of couse, as with anything in SQL, there are some 
	// exceptions to the rule.  Some scalar functions do not use commas to separate the parameters,
	// but instead use keywords like AS and FROM.  Also, there are 4D functions which begin with
	// the { FN pattern.  So we will perform the special case checks before falling back on the
	// more general purpose function parsing.  But it should be noted that this will consider it
	// legal to pass too many parameters to a given function (or not enough).  That being said, the
	// user can deal with it.  All of the parameters are expected to be expressions, and the parens
	// are required.
	//
	// Get the first token
	outToken = GetNextToken();
	if (!outToken) {
		for ( vector< VString * >::iterator iter = fFunctions.begin(); iter != fFunctions.end(); ++iter) {
			outSuggestions.Suggest( **iter, SuggestionInfo::eScalarMethod );
		}
		return eNothingLeftToParse;
	}

	// Handle that token!
	ParserReturnValue helperRet;
	bool bIsCountFunction = false;
	switch (outToken->GetValue()) {
		case SQL_KWORD_ROW_STAMP:
		case SQL_KWORD_ROW_ID: {
			// This function takes no parameters, nor does it use parens
			return eParseSucceeded;
		} break;
		case NAME: {
			// We expect a period token, followed by either __ROW_STAMP or __ROW_ID
			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != PERIOD)	return eIllegalValueParsed;

			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest( "__ROW_STAMP", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "__ROW_ID", SuggestionInfo::eKeyword );
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != SQL_KWORD_ROW_STAMP &&
						outToken->GetValue() != SQL_KWORD_ROW_ID)	return eIllegalValueParsed;

			return eParseSucceeded;
		} break;
		case SQL_KWORD_POSITION: {
			// ( exp IN exp )
			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != LEFT_PARENTHESIS)	return eIllegalValueParsed;

			helperRet = ParseScalarExpression( outSuggestions, outToken );
			if (helperRet != eParseSucceeded)	return helperRet;

			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest( "IN", SuggestionInfo::eKeyword );
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != SQL_KWORD_IN)	return eIllegalValueParsed;

			helperRet = ParseScalarExpression( outSuggestions, outToken );
			if (helperRet != eParseSucceeded)	return helperRet;

			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != RIGHT_PARENTHESIS)	return eIllegalValueParsed;
			return eParseSucceeded;
		} break;
		case SQL_KWORD_TRIM: {
			// ( trim_type opt_scalar_exp FROM scalar_exp )
			// ( scalar_exp FROM scalar_exp )
			// ( scalar_exp )

			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != LEFT_PARENTHESIS)	return eIllegalValueParsed;

			// Peek at the next token to see whether it's the trim_type
			outToken = PeekNextToken();
			if (!outToken) {
				outSuggestions.Suggest( "LEADING", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "TRAILING", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "BOTH", SuggestionInfo::eKeyword );
				ParseScalarExpression( outSuggestions, outToken );
				return eNothingLeftToParse;
			}

			ParserReturnValue helperRet;
			if (outToken->GetValue() == LEADING || 
				outToken->GetValue() == TRAILING ||
				outToken->GetValue() == BOTH) {
				// Consume that token
				GetNextToken();

				// Now we have an optional scalar expression, so check to see if we can
				// find the FROM keyword.  If we can, then we know the user omitted the
				// scalar expression.
				outToken = PeekNextToken();
				if (!outToken) {
					outSuggestions.Suggest( "FROM", SuggestionInfo::eKeyword );
					ParseScalarExpression( outSuggestions, outToken );
					return eNothingLeftToParse;
				} else if (outToken->GetValue() != SQL_KWORD_FROM) {
					// We have a scalar expression to parse
					helperRet = ParseScalarExpression( outSuggestions, outToken );
					if (helperRet != eParseSucceeded)	return helperRet;
				}

				// Now we expect to see the FROM clause no matter what
				outToken = GetNextToken();
				if (!outToken) {
					outSuggestions.Suggest( "FROM", SuggestionInfo::eKeyword );
					return eNothingLeftToParse;
				} else if (outToken->GetValue() != SQL_KWORD_FROM)	return eIllegalValueParsed;

				// And we expect the other scalar expression no matter waht
				helperRet = ParseScalarExpression( outSuggestions, outToken );
				if (helperRet != eParseSucceeded)	return helperRet;
			} else {
				// We didn't have a trim type, so that means we certainly have to get the scalar
				// expression.  There may also be a FROM clause to handle.
				helperRet = ParseScalarExpression( outSuggestions, outToken );
				if (helperRet != eParseSucceeded)	return helperRet;

				// Look to see if there's a FROM clause
				outToken = PeekNextToken();
				if (!outToken) {
					outSuggestions.Suggest( "FROM", SuggestionInfo::eKeyword );
					return eNothingLeftToParse;
				} else if (outToken->GetValue() == SQL_KWORD_FROM) {
					// There was a FROM clause, so consume it and get the remaining scalar expression
					GetNextToken();
					helperRet = ParseScalarExpression( outSuggestions, outToken );
					if (helperRet != eParseSucceeded)	return helperRet;
				}
			}

			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != RIGHT_PARENTHESIS)	return eIllegalValueParsed;
			return eParseSucceeded;
		} break;
		case SQL_KWORD_EXTRACT: {
			// ( extract_field_type FROM scalar_exp )
			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != LEFT_PARENTHESIS)	return eIllegalValueParsed;

			// The next token needs to be one of the extract field types
			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest( "YEAR", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "MONTH", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "DAY", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "HOUR", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "MINUTE", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "SECOND", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "MILLISECOND", SuggestionInfo::eKeyword );
			}

			switch (outToken->GetValue()) {
				case SQL_KWORD_YEAR:
				case SQL_KWORD_MONTH:
				case SQL_KWORD_DAY:
				case SQL_KWORD_HOUR:
				case SQL_KWORD_MINUTE:
				case SQL_KWORD_SECOND:
				case SQL_KWORD_MILLISECOND: {
					// We're great!
				} break;

				default: {
					// Failure!
					return eIllegalValueParsed;
				} break;
			}

			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest( "FROM", SuggestionInfo::eKeyword );
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != SQL_KWORD_FROM)	return eIllegalValueParsed;

			helperRet = ParseScalarExpression( outSuggestions, outToken );
			if (helperRet != eParseSucceeded)	return helperRet;

			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != RIGHT_PARENTHESIS)	return eIllegalValueParsed;
			return eParseSucceeded;
		} break;
		case CAST: {
			// ( scalar_exp AS data_type )
			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != LEFT_PARENTHESIS)	return eIllegalValueParsed;

			helperRet = ParseScalarExpression( outSuggestions, outToken );
			if (helperRet != eParseSucceeded)	return helperRet;

			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest( "AS", SuggestionInfo::eKeyword );
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != AS)	return eIllegalValueParsed;

			helperRet = ParseDataType( outSuggestions, outToken );
			if (helperRet != eParseSucceeded)	return helperRet;

			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != RIGHT_PARENTHESIS)	return eIllegalValueParsed;
			return eParseSucceeded;
		} break;
		case SQL_KWORD_COUNT:
			// The count fucntion is handled in a slightly special case
			bIsCountFunction = true;
			// We *want* to fall through here!!!
		case SQL_KWORD_SUM:
		case SQL_KWORD_AVG: {
			// ( opt_all_distinct scalar_exp )
			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != LEFT_PARENTHESIS)	return eIllegalValueParsed;

			// If we're the count function, there are two forms: one uses the *, and the
			// other uses the all/disctinct scalar expression.  So we need to check for that
			outToken = PeekNextToken();
			if (!outToken) {
				if (bIsCountFunction) {
					outSuggestions.Suggest( "*", SuggestionInfo::eKeyword );
				}
				outSuggestions.Suggest( "ALL", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "DISTINCT", SuggestionInfo::eKeyword );
				ParseScalarExpression( outSuggestions, outToken );

				return eNothingLeftToParse;
			} else if ((outToken->GetValue() == ALL || outToken->GetValue() == DISTINCT)) {
				// Consume this token
				GetNextToken();
			}

			if (bIsCountFunction && outToken->GetValue() == SQL_MULTIPLY) {
				// We can just consume this token
				GetNextToken();
			} else {
				helperRet = ParseScalarExpression( outSuggestions, outToken );
				if (helperRet != eParseSucceeded)	return helperRet;
			}

			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != RIGHT_PARENTHESIS)	return eIllegalValueParsed;
			return eParseSucceeded;
		} break;
		case LEFT_CURLY_BRACKET: {
			// FN NAME ( opt_scalar_exp_list ) AS data_type }
			
			// First, grab the FN and NAME tokens
			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest( "FN", SuggestionInfo::eKeyword );
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != FN)	return eIllegalValueParsed;

			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest4DMethods();
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != NAME)	return eIllegalValueParsed;

			// Now we want to get the open paren
			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != LEFT_PARENTHESIS)	return eIllegalValueParsed;

			// Next comes the comma list.  The end of this list (which could have zero items) in it
			// is actually a right paren.
			bool bParsedAParam = false;
			do {
				// Peek to see whether we've found the end of the list, or perhaps a comma if we've
				// already parsed at least one parameter.
				outToken = PeekNextToken();
				if (!outToken)	return eNothingLeftToParse;
				else if (outToken->GetValue() == RIGHT_PARENTHESIS) {
					GetNextToken();
					break;
				} else if (bParsedAParam) {
					// Once we've parsed a parameter, we expect there to be a comma if there's any
					// other parameters left
					if (outToken->GetValue() == COMMA) {
						GetNextToken();
					} else {
						return eIllegalValueParsed;
					}
				}

				// If it's not the end of the list, it needs to be a scalar expression of some sort
				ParserReturnValue helperRet = ParseScalarExpression( outSuggestions, outToken );
				if (helperRet != eParseSucceeded)	return helperRet;
				bParsedAParam = true;
			} while (true);

			// Next we expect to find the AS data_type clause
			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest( "AS", SuggestionInfo::eKeyword );
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != AS)	return eIllegalValueParsed;

			helperRet = ParseDataType( outSuggestions, outToken );
			if (helperRet != eParseSucceeded)	return helperRet;

			// Finally, we expect the closing bracket to complete the expression
			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != RIGHT_CURLY_BRACKET)	return eIllegalValueParsed;
			return eParseSucceeded;
		} break;
		default: {
			// Check to make sure the text of the keyword we've just gotten actually exists within
			// the vector of functions we expect.  Since this list is small, we can just loop over
			// it rather quickly.  If this turns out to be a performance bottleneck, we can always
			// disable it, or modify it to use a map instead of a vector (though that may require
			// some work since we're dealing with VString * instead of VString).

			bool bFound = false;
			for ( vector< VString * >::iterator iter = fFunctions.begin(); iter != fFunctions.end() && !bFound; ++iter) {
				bFound = ((**iter) == outToken->GetText());
			}

			if (!bFound)	return eIllegalValueParsed;
			
			// Now that we know the user hasn't screwed up the function name, we want to parse the
			// open paren, followed by zero or more comma separate parameters, and a closing paren.
			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != LEFT_PARENTHESIS)	return eIllegalValueParsed;

			// Next comes the comma list.  The end of this list (which could have zero items) in it
			// is actually a right paren.
			bool bParsedAParam = false;
			do {
				// Peek to see whether we've found the end of the list, or perhaps a comma if we've
				// already parsed at least one parameter.
				outToken = PeekNextToken();
				if (!outToken)	{
					ParseTerminal( outSuggestions, outToken );
					return eNothingLeftToParse;
				}
				else if (outToken->GetValue() == RIGHT_PARENTHESIS) {
					GetNextToken();
					return eParseSucceeded;
				} else if (bParsedAParam) {
					// Once we've parsed a parameter, we expect there to be a comma if there's any
					// other parameters left
					if (outToken->GetValue() == COMMA) {
						GetNextToken();
					} else {
						return eIllegalValueParsed;
					}
				}

				// If it's not the end of the list, it needs to be a scalar expression of some sort
				ParserReturnValue helperRet = ParseScalarExpression( outSuggestions, outToken );
				if (helperRet != eParseSucceeded)	return helperRet;
				bParsedAParam = true;
			} while (true);
		} break;
	}

	return eIllegalValueParsed;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseThenExpression( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// We expect to see a THEN token followed by a scalar expression
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "THEN", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != THEN) return eIllegalValueParsed;

	// Now we expect a scalar expression
	return ParseScalarExpression( outSuggestions, outToken );
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseSelection( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// A selection is either a scalar expression list separated by commas, or it's
	// the * token.
	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		outSuggestions.Suggest( "*", SuggestionInfo::eKeyword );
		ParseScalarExpression( outSuggestions, outToken );
		return eNothingLeftToParse;
	} else if (temp->GetValue() == SQL_MULTIPLY) {
		outToken = GetNextToken(); // Consume!
		return eParseSucceeded;
	} else {
		// Parse the comma-separated scalar expression list.  This list can also 
		// contain ALIAS clauses for each of the scalar expressions.
		while (true) {
			// Parse a scalar expression
			ParserReturnValue helperRet = ParseScalarExpression( outSuggestions, outToken );
			if (helperRet != eParseSucceeded)	return helperRet;

			// Check to see whether we have an optional ALIAS clause we need to
			// handle.
			temp = PeekNextToken();
			if (!temp) {
				outSuggestions.Suggest( "AS", SuggestionInfo::eKeyword );
				return eParseSucceeded;
			} else if (temp->GetValue() == AS) {
				// Consume the AS token, and find out what we're aliasing it as
				GetNextToken();

				// We expect one more token for the alias type
				outToken = GetNextToken();
				if (!outToken)	return eNothingLeftToParse;
				if (outToken->GetValue() != NAME &&
					outToken->GetValue() != STRING &&
					outToken->GetValue() != BACK_TICKED_STRING)	return eIllegalValueParsed;

				// We're aliasing this as a column, similar to how we do it for tables when looking at
				// table ref clauses.  So let's add a temporary column reference to the symbol table.  Columns
				// are a bit strange in that they may or may not really exist.  For instance, you could do 
				// something like this: SELECT (1+2) as Foo, and that's perfectly legal to do.  We would need
				// to add a column named Foo to our temporary list, but it doesn't reference a real column.
				// I am going to skimp a bit because columns do not contain any sub-symbols.  So I am not going
				// to see if I can find a named column from the scalar expression (since it's possible the user
				// can do goofy things, like (SomeColumn)).
				if (fSymTable) {
					// Strip the backtic's or quotes around the string if we have to
					VString newName = outToken->GetText();
					if (outToken->GetValue() == STRING || outToken->GetValue() == BACK_TICKED_STRING)	newName.SubString( 2, newName.GetLength() - 2 );
					fSymTable->AddSymbolToTable( IAutoCompleteSymbolTable::VColumnSymbol::TemporaryFrom( newName ) );
				}

				// Peek at another token so we can see if there's a COMMA
				temp = PeekNextToken();
				if (!temp)	return eNothingLeftToParse;
			}

			if (temp->GetValue() != COMMA)	return eParseSucceeded;
			// Consume the COMMA
			GetNextToken();
		}
	}
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseTableRefTerminal( SuggestionList &outSuggestions, class ILexerToken * &outToken, bool inIncludeViews )
{
	// A terminal table ref is either a table by itself, or it's a table that's been aliased.  Tables are
	// just a NAME, STRING, or BACK_TICKED_STRING.
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.SuggestTables();
		if ( inIncludeViews )
			outSuggestions.SuggestViews();

		return eNothingLeftToParse;
	} 
	
	if (outToken->GetValue() == LEFT_PARENTHESIS) {
		ParserReturnValue helperRet = ParseTableRef( outSuggestions, outToken, inIncludeViews );
		if (helperRet != eParseSucceeded)	return helperRet;
		outToken = GetNextToken();
		if (!outToken)										return eNothingLeftToParse;
		else if (outToken->GetValue() != RIGHT_PARENTHESIS)	return eIllegalValueParsed;
		else												return eParseSucceeded;
	} else if (outToken->GetValue() != NAME && outToken->GetValue() != STRING && outToken->GetValue() != BACK_TICKED_STRING) return eIllegalValueParsed;

	VString tableText = outToken->GetText();

	// Next comes the optional AS clause
	ILexerToken *temp = PeekNextToken();
	bool bFoundAsClause = false;
	if (!temp) {
		outSuggestions.Suggest( "AS", SuggestionInfo::eKeyword );
		return eParseSucceeded;
	} else if (temp->GetValue() == AS) {
		// Consume the AS token
		GetNextToken();
		bFoundAsClause = true;
	}

	// If we have an AS clause, we expect an alias.  However, it is possible
	// that the user left the AS clause out and we still have an alias.  So
	// we need to peek at the next token to determine what to do.
	outToken = PeekNextToken();
	if (!outToken) {
		return eNothingLeftToParse;
	} 
	
	if (outToken->GetValue() != NAME && 
		outToken->GetValue() != STRING && 
		outToken->GetValue() != BACK_TICKED_STRING) {

		if (bFoundAsClause)		return eIllegalValueParsed;
		else					return eParseSucceeded;
	}

	// We found a the alias name, so we want to consume it
	GetNextToken();

	// Now that we've seen an alias, we want to add it temporarily to the symbol
	// table so that it can be used in future completions.  Technically, this should be
	// the job of the declaration parser.  However, there are so few declarations to deal
	// with in SQL that we are just going to do it inline with the statement completion.
	// These temporary table aliases will be removed once the statement completion is
	// attempted.
	if (fSymTable) {
		// See if we can find a table symbol with the proper name
		IAutoCompleteSymbolTable::IAutoCompleteSymbol *table = fSymTable->GetSymbolByName( tableText );
		if (table) {
			// Strip the backtic's or quotes around the string if we have to
			VString newName = outToken->GetText();
			if (outToken->GetValue() == STRING || outToken->GetValue() == BACK_TICKED_STRING)	newName.SubString( 2, newName.GetLength() - 2 );
			fSymTable->AddSymbolToTable( IAutoCompleteSymbolTable::VTableSymbol::TemporaryFrom( table, newName ) );
		}
	}

	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseTableRef( SuggestionList &outSuggestions, ILexerToken* &outToken, bool inIncludeViews )
{
	// Due to join clauses, table references are left-recursive, so we will handle them in much 
	// the same way we do for scalar expressions: by using shunting yard.  For more information,
	// see the comments at the start of the ParseScalarExpression method.
	ParserReturnValue helperRet = ParseTableRefTerminal( outSuggestions, outToken, inIncludeViews );
	if (helperRet != eParseSucceeded)	return helperRet;

	// Now peek to see whether we've got a join table clause that we need to care about
	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		// While the parse is technically done, it's still possible that the caller has
		// some choices they might want to make.
		outSuggestions.Suggest( "LEFT OUTER JOIN", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "RIGHT OUTER JOIN", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "FULL OUTER JOIN", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "INNER JOIN", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "CROSS JOIN", SuggestionInfo::eKeyword );
		return eParseSucceeded;
	}

	// Now we expect to see the join type if there is one.  It's perfectly fine to not
	// get a join type clause though, so it's ok to bail out.
	enum JoinSpecRequirement {
		kRequired,
		kIllegal,
		kOptional,
	};
	JoinSpecRequirement joinSpecRequired = kRequired;
	if (temp->GetValue() == SQL_KWORD_LEFT ||
		temp->GetValue() == SQL_KWORD_RIGHT || 
		temp->GetValue() == SQL_KWORD_FULL) {
		GetNextToken();	// Eat the token
		// We expect the next keyword to be OUTER
		outToken = GetNextToken();
		if (!outToken) {
			outSuggestions.Suggest( "OUTER JOIN", SuggestionInfo::eKeyword );
			return eNothingLeftToParse;
		} else if (outToken->GetValue() != SQL_KWORD_OUTER)	return eIllegalValueParsed;
	} else if (temp->GetValue() == SQL_KWORD_INNER) {
		GetNextToken();	// Eat the token
		joinSpecRequired = kOptional;
	} else if (temp->GetValue() == SQL_KWORD_CROSS) {
		GetNextToken(); // Eat the token
		joinSpecRequired = kIllegal;
	} else if (temp->GetValue() == SQL_KWORD_JOIN) {
		// The join type is actually optional -- when no type is specified, it's actually
		// an INNER join.  We don't explicitly give this option to the user because we want
		// them to be explicit whenever possible.  But we don't want to disallow it either.
		// We just want to fall through and not eat the token, because we'll eat the join 
		// in the next step anyways.
		joinSpecRequired = kOptional;
	} else {
		// We don't have a join clause, which is fine
		return eParseSucceeded;
	}

	// Now we expect to see the JOIN clause itself
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "JOIN", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != SQL_KWORD_JOIN)	return eIllegalValueParsed;

	// Now we expect to recieve another table reference.
	helperRet = ParseTableRef( outSuggestions, outToken, inIncludeViews );
	if (helperRet != eParseSucceeded)	return helperRet;

	// Now that we have both the left and right table references, we can parse the search condition, which
	// starts with the ON clause.  However, the specification is sometimes optional, sometimes required and
	// sometimes illegal.  So we'll test whether we want to see it or not.
	outToken = PeekNextToken();
	if (!outToken) {
		if (joinSpecRequired != kIllegal) {
			outSuggestions.Suggest( "ON", SuggestionInfo::eKeyword );
		}
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != ON) {
		// If we require the join spec, then we claim this is an illegal token.  But
		// if the spec is optional, then we can't make such a claim
		if (joinSpecRequired == kRequired)	return eIllegalValueParsed;
		if (joinSpecRequired == kOptional)	return eParseSucceeded;
		if (joinSpecRequired == kIllegal)	return eParseSucceeded;
	} else if (joinSpecRequired == kIllegal) {
		// We cannot have a spec specified, but the user put in an ON token
		return eIllegalValueParsed;
	}

	// At this point, the only thing left to parse is the search condition itself since
	// we already handled the case where a search condition is illegal.  We need to eat
	// the ON token before we proceed.
	GetNextToken();
	helperRet = ParseSearchCondition( outSuggestions, outToken );
	if (helperRet != eParseSucceeded)	return helperRet;

	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseTableRefList( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// Parse the comma-separated table reference list.
	while (true) {
		// Parse a table reference
		ParserReturnValue helperRet = ParseTableRef( outSuggestions, outToken, true );
		if (helperRet != eParseSucceeded)	return helperRet;

		// Check to see whether we have a COMMA
		ILexerToken *temp = PeekNextToken();
		if (!temp) {
			return eParseSucceeded;
		} else if (temp->GetValue() == COMMA) {
			// Consume the COMMA
			GetNextToken();
		} else {
			return eParseSucceeded;
		}
	}
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseOrderingSpecification( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// This starts with either an INTNUM or a column reference.  It is optionally followed
	// by an ascending or descending clause.
	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		ParseColumnRef( outSuggestions, outToken );
		return eNothingLeftToParse;
	}

	if (temp->GetValue() == INTNUM) {
		// Just consume the token
		outToken = GetNextToken();
	} else {
		// Parse a column reference
		ParserReturnValue helperRet = ParseColumnRef( outSuggestions, outToken );
		if (helperRet != eParseSucceeded)	return helperRet;
	}

	// Now we may have an ascending/descending clause
	temp = PeekNextToken();
	if (!temp) {
		outSuggestions.Suggest( "ASC", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DESC", SuggestionInfo::eKeyword );
		return eParseSucceeded;
	} else if (temp->GetValue() == ASC || temp->GetValue() == DESC) {
		// Consume the token!
		outToken = GetNextToken();
	}

	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseOrderingSpecificationList( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// Parse the comma-separated ordering specification list.
	while (true) {
		// Parse an ordering specification
		ParserReturnValue helperRet = ParseOrderingSpecification( outSuggestions, outToken );
		if (helperRet != eParseSucceeded)	return helperRet;

		// Check to see whether we have a COMMA
		ILexerToken *temp = PeekNextToken();
		if (!temp) {
			return eParseSucceeded;
		} else if (temp->GetValue() == COMMA) {
			// Consume the COMMA
			GetNextToken();
		} else {
			return eParseSucceeded;
		}
	}
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseLiteral( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// The caller expects to get a literal, so get the next token to handle it
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "FALSE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "TRUE", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	}

	switch (outToken->GetValue()) {
		case SQL_KWORD_TRUE:
		case SQL_KWORD_FALSE:
		case STRING:
		case INTNUM:
		case APPROXNUM:	return eParseSucceeded;
		case LEFT_CURLY_BRACKET: {
			// This is a bit of a strange case in that it's not just a single token for the literal.
			// We expect to see { TS STRING }
			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest( "TS", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "D", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "T", SuggestionInfo::eKeyword );
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != SQL_KWORD_TS && outToken->GetText() != "D" && outToken->GetText() != "T")	return eIllegalValueParsed;

			outToken = GetNextToken();
			if (!outToken)								return eNothingLeftToParse;
			else if (outToken->GetValue() != STRING)	return eIllegalValueParsed;

			outToken = GetNextToken();
			if (!outToken)											return eNothingLeftToParse;
			else if (outToken->GetValue() != RIGHT_CURLY_BRACKET)	return eIllegalValueParsed;

			return eParseSucceeded;
		} break;
	}

	return eIllegalValueParsed;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseFrom( SuggestionList &outSuggestions, ILexerToken * &outToken )
{
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "FROM", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != SQL_KWORD_FROM)	return eIllegalValueParsed;

	// Next comes a comma-separated list of table references
	return ParseTableRefList( outSuggestions, outToken );
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseLimitClause( class SuggestionList &outSuggestions, class ILexerToken *&outToken )
{
	// This clause must be followed by one of the following: 
	// INTNUM, <<NAME>>, <<$NAME>>, :NAME, :$NAME, ?, ALL
	outToken = GetNextToken();
	if (!outToken) return eNothingLeftToParse;

	switch (outToken->GetValue()) {
		case QUESTION_MARK:
		case ALL:
		case INTNUM:	break;		// We're done!
		case DOUBLE_LESS_THAN: {
			// We're may see a $ before seeing the NAME and the closing bracket
			outToken = PeekNextToken();
			bool bForLocalVariables = false;
			if (!outToken) {
				outSuggestions.Suggest4DProcessVariables();
				outSuggestions.Suggest4DInterProcessVariables();
				return eNothingLeftToParse;
			} else if (outToken->GetValue() == DOLLAR_SIGN)	{
				bForLocalVariables = true;
				GetNextToken();	// Consume the $
			}

			// Now we expect to see a NAME followed by >>
			outToken = GetNextToken();
			if (!outToken) {
				if (bForLocalVariables) {
					outSuggestions.Suggest4DLocalVariables();
				} else {
					outSuggestions.Suggest4DProcessVariables();
					outSuggestions.Suggest4DInterProcessVariables();
				}
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != NAME)	return eIllegalValueParsed;

			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != DOUBLE_GREATER_THAN)	return eIllegalValueParsed;
		} break;
		case COLON: {
			// We're may see a $ before seeing the NAME
			outToken = PeekNextToken();
			bool bForLocalVariables = false;
			if (!outToken) {
				outSuggestions.Suggest4DMethods();
				outSuggestions.Suggest4DInterProcessVariables();
				outSuggestions.Suggest4DProcessVariables();
				return eNothingLeftToParse;
			} else if (outToken->GetValue() == DOLLAR_SIGN) {
				bForLocalVariables = true;
				GetNextToken();	// Consume the $
			}

			// Now we expect to see the NAME
			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest4DMethods();
				if (bForLocalVariables) {
					outSuggestions.Suggest4DLocalVariables();
				} else {
					outSuggestions.Suggest4DInterProcessVariables();
					outSuggestions.Suggest4DProcessVariables();
				}
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != NAME)	return eIllegalValueParsed;
		} break;
	}

	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseOffsetClause( class SuggestionList &outSuggestions, class ILexerToken *&outToken )
{
	// This clause must be followed by one of the following: 
	// INTNUM, <<NAME>>, <<$NAME>>, :NAME, :$NAME, ?
	outToken = GetNextToken();
	if (!outToken) return eNothingLeftToParse;

	switch (outToken->GetValue()) {
		case QUESTION_MARK:
		case INTNUM:	break;		// We're done!
		case DOUBLE_LESS_THAN: {
			// We're may see a $ before seeing the NAME and the closing bracket
			outToken = PeekNextToken();
			bool bForLocalVariables = false;
			if (!outToken) {
				outSuggestions.Suggest4DProcessVariables();
				outSuggestions.Suggest4DInterProcessVariables();
				return eNothingLeftToParse;
			} else if (outToken->GetValue() == DOLLAR_SIGN)	{
				bForLocalVariables = true;
				GetNextToken();	// Consume the $
			}

			// Now we expect to see a NAME followed by >>
			outToken = GetNextToken();
			if (!outToken) {
				if (bForLocalVariables) {
					outSuggestions.Suggest4DLocalVariables();
				} else {
					outSuggestions.Suggest4DProcessVariables();
					outSuggestions.Suggest4DInterProcessVariables();
				}
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != NAME)	return eIllegalValueParsed;

			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != DOUBLE_GREATER_THAN)	return eIllegalValueParsed;
		} break;
		case COLON: {
			// We're may see a $ before seeing the NAME
			outToken = PeekNextToken();
			bool bForLocalVariables = false;
			if (!outToken) {
				outSuggestions.Suggest4DMethods();
				outSuggestions.Suggest4DInterProcessVariables();
				outSuggestions.Suggest4DProcessVariables();
				return eNothingLeftToParse;
			} else if (outToken->GetValue() == DOLLAR_SIGN) {
				bForLocalVariables = true;
				GetNextToken();	// Consume the $
			}

			// Now we expect to see the NAME
			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest4DMethods();
				if (bForLocalVariables) {
					outSuggestions.Suggest4DLocalVariables();
				} else {
					outSuggestions.Suggest4DInterProcessVariables();
					outSuggestions.Suggest4DProcessVariables();
				}
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != NAME)	return eIllegalValueParsed;
		} break;
	}

	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseSelectStatement( SuggestionList &outSuggestions, class ILexerToken * &outToken, bool inForSubquery )
{
	// SELECT statements are used all over the place in the grammar, each with slightly
	// different syntax.  So we differentiate the syntax expected based on parameter
	// values that are passed in.  But the basic form of this statement is:
	// SELECT [ALL|DISTINCT] selection FROM table_list [bunch of optional stuff that depends on statement type]

	// First, we want to parse the SELECT statement if we're doing a subquery -- if we're not, then
	// the SELECT has already been parsed.  This is a bit confusing, but would require a lot of work
	// to refactor.
	if (inForSubquery) {
		outToken = GetNextToken();
		if (!outToken) {
			outSuggestions.Suggest( "SELECT", SuggestionInfo::eKeyword );
			return eNothingLeftToParse;
		} else if (outToken->GetValue() != SELECT)	return eIllegalValueParsed;
	}

	// Next comes the optional ALL or DISTINCT clause
	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		outSuggestions.Suggest( "ALL", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DISTINCT", SuggestionInfo::eKeyword );
		ParseSelection( outSuggestions, outToken );
		return eNothingLeftToParse;
	} else if (temp->GetValue() == ALL || temp->GetValue() == DISTINCT) {
		// Consume the token and move on
		outToken = GetNextToken();
	}

	// Now we expect to get the selection
	ParserReturnValue helperRet = ParseSelection( outSuggestions, outToken );
	if (helperRet != eParseSucceeded)	return helperRet;

	// Next comes the FROM clause
	helperRet = ParseFrom( outSuggestions, outToken );
	if (helperRet != eParseSucceeded)	return helperRet;

	// Now comes a whole bunch of optional clauses.  Peek at the next token to see
	// what sort of clause we might have.
	temp = PeekNextToken();
	if (!temp) {
		outSuggestions.Suggest( "WHERE", SuggestionInfo::eKeyword );
		if (!inForSubquery) {
			outSuggestions.Suggest( "ORDER BY", SuggestionInfo::eKeyword );
			outSuggestions.Suggest( "INTO", SuggestionInfo::eKeyword );
			outSuggestions.Suggest( "FOR UPDATE", SuggestionInfo::eKeyword );
		}
		outSuggestions.Suggest( "GROUP BY", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "HAVING", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "LIMIT", SuggestionInfo::eKeyword );
		return eParseSucceeded;
	}

	// Check to see if we have a WHERE clause
	if (temp->GetValue() == WHERE) {
		// Consume the token
		GetNextToken();
		// Handle the search condition
		helperRet = ParseSearchCondition( outSuggestions, outToken );
		if (helperRet != eParseSucceeded)	return helperRet;

		// Get the next token so we can test for the remaining clauses
		temp = PeekNextToken();
		if (!temp) {
			if (!inForSubquery) {
				outSuggestions.Suggest( "ORDER BY", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "INTO", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "FOR UPDATE", SuggestionInfo::eKeyword );
			}
			outSuggestions.Suggest( "GROUP BY", SuggestionInfo::eKeyword );
			outSuggestions.Suggest( "HAVING", SuggestionInfo::eKeyword );
			outSuggestions.Suggest( "LIMIT", SuggestionInfo::eKeyword );
			return eParseSucceeded;
		}
	}

	// If we're not a subquery, check to see if we've got an ORDER BY clause
	bool bSeenOrderBy = false;
	if (!inForSubquery && temp->GetValue() == ORDER) {
		bSeenOrderBy = true;

		// Consume the token
		GetNextToken();

		// We expect to see a BY clause
		outToken = GetNextToken();
		if (!outToken) {
			outSuggestions.Suggest( "BY", SuggestionInfo::eKeyword );
			return eNothingLeftToParse;
		} else if (outToken->GetValue() != BY)	return eIllegalValueParsed;

		// Now we expect the ordering specification.
		helperRet = ParseOrderingSpecificationList( outSuggestions, outToken );
		if (helperRet != eParseSucceeded)	return helperRet;

		// Get the next token so we can test for the remaining clauses
		temp = PeekNextToken();
		if (!temp) {
			if (!inForSubquery) {
				outSuggestions.Suggest( "INTO", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "FOR UPDATE", SuggestionInfo::eKeyword );
			}
			outSuggestions.Suggest( "GROUP BY", SuggestionInfo::eKeyword );
			outSuggestions.Suggest( "HAVING", SuggestionInfo::eKeyword );
			outSuggestions.Suggest( "LIMIT", SuggestionInfo::eKeyword );
			return eParseSucceeded;
		}
	}
	
	// Check to see if we have a GROUP BY clause
	if (temp->GetValue() == SQL_KWORD_GROUP) {
		// Consume the token
		GetNextToken();

		// We expect to see a BY clause
		outToken = GetNextToken();
		if (!outToken) {
			outSuggestions.Suggest( "BY", SuggestionInfo::eKeyword );
			return eNothingLeftToParse;
		} else if (outToken->GetValue() != BY)	return eIllegalValueParsed;

		// Now we expect the ordering specification.
		helperRet = ParseOrderingSpecificationList( outSuggestions, outToken );
		if (helperRet != eParseSucceeded)	return helperRet;

		// Get the next token so we can test for the remaining clauses
		temp = PeekNextToken();
		if (!temp) {
			if (!inForSubquery) {
				outSuggestions.Suggest( "INTO", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "FOR UPDATE", SuggestionInfo::eKeyword );
			}
			outSuggestions.Suggest( "HAVING", SuggestionInfo::eKeyword );
			outSuggestions.Suggest( "LIMIT", SuggestionInfo::eKeyword );
			return eParseSucceeded;
		}
	}

	// Check to see if we have a HAVING clause
	if (temp->GetValue() == HAVING) {
		// Consume the token
		GetNextToken();

		// Now we expect the search condition
		helperRet = ParseSearchCondition( outSuggestions, outToken );
		if (helperRet != eParseSucceeded)	return helperRet;

		// Get the next token so we can test for the remaining clauses
		temp = PeekNextToken();
		if (!temp) {
			if (!inForSubquery) {
				outSuggestions.Suggest( "INTO", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "FOR UPDATE", SuggestionInfo::eKeyword );
				if (!bSeenOrderBy) {
					outSuggestions.Suggest( "ORDER BY", SuggestionInfo::eKeyword );
				}
			}
			outSuggestions.Suggest( "LIMIT", SuggestionInfo::eKeyword );
			return eParseSucceeded;
		}
	}

	// If we've not already seen an ORDER BY clause, and we're not parsing for a subquery,
	// then we might have the other ORDER BY clause by now.
	if (!bSeenOrderBy && !inForSubquery && temp->GetValue() == ORDER) {
		// Consume the token
		GetNextToken();

		// We expect to see a BY clause
		outToken = GetNextToken();
		if (!outToken) {
			outSuggestions.Suggest( "BY", SuggestionInfo::eKeyword );
			return eNothingLeftToParse;
		} else if (outToken->GetValue() != BY)	return eIllegalValueParsed;

		// Now we expect the ordering specification.

		// Get the next token so we can test for the remaining clauses
		temp = PeekNextToken();
		if (!temp) {
			if (!inForSubquery) {
				outSuggestions.Suggest( "INTO", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "FOR UPDATE", SuggestionInfo::eKeyword );
			}
			outSuggestions.Suggest( "LIMIT", SuggestionInfo::eKeyword );
			return eParseSucceeded;
		}
	}

	// Check to see if we have a LIMIT clause
	if (temp->GetValue() == LIMIT) {
		// Consume the token
		GetNextToken();
		helperRet = ParseLimitClause( outSuggestions, outToken );
		if (helperRet != eParseSucceeded)	return helperRet;

		// LIMIT may also be followed by an optional OFFSET clause, so peek at the next token
		// to see if its an OFFSET clause
		temp = PeekNextToken();
		if (!temp) {
			outSuggestions.Suggest( "OFFSET", SuggestionInfo::eKeyword );
			if (!inForSubquery) {
				outSuggestions.Suggest( "INTO", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "FOR UPDATE", SuggestionInfo::eKeyword );
			}
			return eParseSucceeded;
		} else if (temp->GetValue() == OFFSET) {
			// We do have an OFFSET clause, so consume this token and move on
			GetNextToken();

			helperRet = ParseOffsetClause( outSuggestions, outToken );
			if (helperRet != eParseSucceeded)	return helperRet;

			// Get the next token so we can test for the remaining clauses
			temp = PeekNextToken();
			if (!temp) {
				if (!inForSubquery) {
					outSuggestions.Suggest( "INTO", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "FOR UPDATE", SuggestionInfo::eKeyword );
				}
				return eParseSucceeded;
			}
		}
	}

	// If we're not parsing for a subquery, we may get an INTO clause
	if (!inForSubquery && temp->GetValue() == INTO) {
		// Consume the token
		outToken = GetNextToken();

		// We're either selecting into a 4D reference list, or a LISTBOX, so peek at the next
		// token to see which it is.
		temp = PeekNextToken();
		if (!temp) {
			outSuggestions.Suggest( "LISTBOX", SuggestionInfo::eKeyword );
			Parse4DReference( outSuggestions, outToken );
			return eNothingLeftToParse;
		} else if (temp->GetValue() == LISTBOX) {
			GetNextToken(); // Consume the token
			// Parse a single 4D reference
			helperRet = Parse4DReference( outSuggestions, outToken );
			if (helperRet != eParseSucceeded)	return helperRet;
		} else {
			// Parse a 4D reference list
			helperRet = Parse4DReferenceList( outSuggestions, outToken );
			if (helperRet != eParseSucceeded)	return helperRet;
		}

		// Get the next token so we can test for the remaining clauses
		temp = PeekNextToken();
		if (!temp) {
			outSuggestions.Suggest( "FOR UPDATE", SuggestionInfo::eKeyword );
			return eParseSucceeded;
		}
	}

	// If we're not parsing for a subquery, we may get an FOR UPDATE
	if (!inForSubquery && temp->GetValue() == SQL_KWORD_FOR) {
		// Consume the token
		GetNextToken();

		// We require the UPDATE clause
		outToken = GetNextToken();
		if (!outToken) {
			outSuggestions.Suggest( "UPDATE", SuggestionInfo::eKeyword );
			return eNothingLeftToParse;
		} else if (outToken->GetValue() != UPDATE)	return eIllegalValueParsed;
	}

	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseSubquery( SuggestionList &outSuggestions, class ILexerToken * &outToken, bool inJustBody )
{
	// This is generally a tricky function is that it is often found in a production that
	// expects either a scalar expression or a subquery.  However, both begin with a left
	// parenthesis.  The difference is that a subquery's left parenthesis is followed by
	// a SELECT clause and a scalar expression's is not.  So if we read in an opening paren
	// that's not followed by SELECT, then we need to tell the caller that we hit an unexpected
	// token instead of an illegal value.  But the fact is that we've still parsed the the
	// open paren, which needs to be pushed back onto the stream.  This is a disgusting thing
	// to do, because it basically means we're using two tokens of lookahead.  However, in the
	// interests of keeping the parser reasonably easy to understand, we're just going to have
	// to turn a blind eye to this little mess.
	//
	// Get the opening paren that we expect if we're expecting to do more than just the body
	if (!inJustBody) {
		outToken = PeekNextToken();
		if (!outToken) {
			return eNothingLeftToParse;
		} else if (outToken->GetValue() != LEFT_PARENTHESIS)	return eUnexpectedTokenParsed;
		GetNextToken();	// Consume the token we just read
	}

	// Peek at the next token to ensure that it's a SELECT statement
	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		outSuggestions.Suggest( "SELECT", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (temp->GetValue() != SELECT) {
		// This is that messy, nasty case I was talking about previously.  We're not really a
		// subquery after all.  So put the left paren token back onto the stream where it came
		// from, and tell the user we found something unexpected;
		PutBackToken();
		return eUnexpectedTokenParsed;
	}

	// We now know that we're part of the subquery body, so parse that next
	ParserReturnValue helperRet = ParseSelectStatement( outSuggestions, outToken, true );
	if (helperRet != eParseSucceeded)	return helperRet;

	if (!inJustBody) {
		// Now we expect the closing parens
		outToken = GetNextToken();
		if (!outToken)	return eNothingLeftToParse;
		else if (outToken->GetValue() != RIGHT_PARENTHESIS)	return eIllegalValueParsed;
	}

	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParsePredicate( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// There are a fair number of predicates to test for, most of which involve a scalar
	// expression as their first clause.  In fact, the only cases which don't follow this
	// pattern are the existence_test (which uses EXISTS as its first token), and one part
	// of the comparison_predicate (which uses a subquery which always starts with "( SELECT").
	// This is a bit tricky however... we can peek at the first token, and see if it's EXISTS
	// to know that we're doing an existence_test.  But if we see a LEFT_PARENTHESIS, then it
	// could be the start of a scalar expression, or it could be the start of a subquery.  We
	// would have to be able to peek at the next token to tell whether it's a SELECT or not.
	// However, that requires us to have two tokens of lookahead.
	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		outSuggestions.Suggest( "EXISTS", SuggestionInfo::eKeyword );
		ParseScalarExpression( outSuggestions, outToken );
		return eNothingLeftToParse;
	}

	switch (temp->GetValue()) {
		case EXISTS: {
			outToken = GetNextToken();  // Consume the token
			// This is an existence_test, which requires us to parse a subquery next.
			return ParseSubquery( outSuggestions, outToken );
		} break;
		case LEFT_PARENTHESIS: {
			// This could be part of a comparison_predicate or just the start of a 
			// scalar expression.  We will try to parse a subquery, but it that fails
			// we want to fall through to the default because it needs to be a scalar
			// expression.
			ParserReturnValue helperRet = ParseSubquery( outSuggestions, outToken );
			if (helperRet == eNothingLeftToParse || helperRet == eIllegalValueParsed)	return helperRet;
			if (helperRet == eParseSucceeded)	break;
		}	// We *do not* want to break here!  This is on purpose!
		default: {
			// The rest of the predicates all start with a scalar expression.  So parse it.
			ParserReturnValue helperRet = ParseScalarExpression( outSuggestions, outToken );
			if (helperRet != eParseSucceeded)	return helperRet;
		} break;
	}

	// At this point, we need to grab the next token to see if we can narrow down what type of predicate
	// we're really dealing with.  We may be able to determine exactly what's expected, but we may need
	// to continue parsing more tokens before we know for sure.
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "BETWEEN", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "LIKE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "IS NOT NULL", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "IS NULL", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "NOT BETWEEN", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "NOT LIKE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "IN", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "NOT IN", SuggestionInfo::eKeyword );
		ParseScalarExpression( outSuggestions, outToken );
		return eNothingLeftToParse;
	}

	enum EPredicateType {
		eUnknown = -1,
		eComparison,
		eBetween,
		eLike,
		eTestForNull,
		eIn,
		eAllOrAny,
	};

	EPredicateType predicateType = eUnknown;
	switch (outToken->GetValue()) {
		// All of these cases are unambiguous
		case BETWEEN:		predicateType = eBetween; break;
		case LIKE:			predicateType = eLike; break;
		case IS:			predicateType = eTestForNull; break;
		case SQL_KWORD_IN:	predicateType = eIn; break;
		
		// These cases require further evaluation to determine the
		// clause's type
		case COMPARISON: {
			// We need to peek at the next token to see whether we're an any/all clause
			// or just a simple comparison.
			temp = PeekNextToken();
			if (!temp) {
				outSuggestions.Suggest( "ANY", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "ALL", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "SOME", SuggestionInfo::eKeyword );
				ParseScalarExpression( outSuggestions, outToken );
				ParseSubquery( outSuggestions, outToken );
				return eNothingLeftToParse;
			} else if (temp->GetValue() == ANY || temp->GetValue() == ALL || temp->GetValue() == SOME) {
				// Consume the token
				outToken = GetNextToken();
				predicateType = eAllOrAny;
			} else {
				predicateType = eComparison;
			}
		} break;
		case NOT: {
			// Consume the next token to see what kind of clause
			// we really have
			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest( "BETWEEN", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "LIKE", SuggestionInfo::eKeyword );
				outSuggestions.Suggest( "IN", SuggestionInfo::eKeyword );
				return eNothingLeftToParse;
			}

			if (outToken->GetValue() == BETWEEN)			predicateType = eBetween;
			else if (outToken->GetValue() == LIKE)			predicateType = eLike;
			else if (outToken->GetValue() == SQL_KWORD_IN)	predicateType = eIn;
			else return eIllegalValueParsed;
		} break;
	}

	// The assumption is that by now we've figured out exactly what type of predicate
	// we are dealing with, and we are now required to finish parsing that particular
	// predicate in whatever fashion makes sense.
	if (eUnknown == predicateType)	return eIllegalValueParsed;
	if (eBetween == predicateType) {
		// We expect to see scalar_exp AND scalar_exp
		ParserReturnValue helperRet = ParseScalarExpression( outSuggestions, outToken );
		if (helperRet != eParseSucceeded)	return helperRet;

		outToken = GetNextToken();
		if (!outToken) {
			outSuggestions.Suggest( "AND", SuggestionInfo::eKeyword );
			return eNothingLeftToParse;
		} else if (outToken->GetValue() != AND)	return eIllegalValueParsed;

		return ParseScalarExpression( outSuggestions, outToken );
	} else if (eLike == predicateType) {
		// We expect to see a scalar_exp followed by an optional escape clause
		ParserReturnValue helperRet = ParseScalarExpression( outSuggestions, outToken );
		if (helperRet != eParseSucceeded)	return helperRet;

		temp = PeekNextToken();
		if (!temp) {
			outSuggestions.Suggest( "ESCAPE", SuggestionInfo::eKeyword );
			return eParseSucceeded;
		} else if (temp->GetValue() == ESCAPE) {
			// Consume the token we found, and parse the STRING token next
			GetNextToken();
			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != STRING)	return eIllegalValueParsed;
			else return eParseSucceeded;
		} else {
			return eParseSucceeded;
		}
	} else if (eTestForNull == predicateType) {
		// We expect to see either NOT NULL or NULL, and that's it
		outToken = GetNextToken();
		if (!outToken) {
			outSuggestions.Suggest( "NOT NULL", SuggestionInfo::eKeyword );
			outSuggestions.Suggest( "NULL", SuggestionInfo::eKeyword );
			return eNothingLeftToParse;
		} else if (outToken->GetValue() == NULLX) {
			return eParseSucceeded;
		} else if (outToken->GetValue() == NOT) {
			// We expect to find the NULL token next
			outToken = GetNextToken();
			if (!outToken) {
				outSuggestions.Suggest( "NULL", SuggestionInfo::eKeyword );
				return eNothingLeftToParse;
			} else if (outToken->GetValue() != NULLX)	return eIllegalValueParsed;
			return eParseSucceeded;
		} else {
			return eIllegalValueParsed;
		}
	} else if (eAllOrAny == predicateType) {
		// We expect to get a subquery of some type
		ParserReturnValue helperRet = ParseSubquery( outSuggestions, outToken );
		if (helperRet == eUnexpectedTokenParsed)	helperRet = eIllegalValueParsed;
		return helperRet;
	} else if (eComparison == predicateType) {
		// We expect either a subquery or a scalar expression, so try to parse the subquery, and
		// if that fails, fall back on the scalar expression parser.  This isn't entirely correct
		// in that we should only suggest a subquery or a scalar expression depending on what the
		// left hand side of the COMPARISON is.  Might be a nice thing to support some day
		ParserReturnValue helperRet = ParseSubquery( outSuggestions, outToken );
		if (helperRet == eUnexpectedTokenParsed) {
			helperRet = ParseScalarExpression( outSuggestions, outToken );
		}
		return helperRet;
	} else if (eIn == predicateType) {
		// We're either going to be dealing with a single subquery, or a list of scalar expressions,
		// so try the subquery first.  If that fails, fall back on the list of scalar expressions.
		ParserReturnValue helperRet = ParseSubquery( outSuggestions, outToken );
		if (helperRet == eUnexpectedTokenParsed) {
			// We expect to see a list of comma-separated scalar expressions bounded by parens
			// Now we want to get the open paren
			outToken = GetNextToken();
			if (!outToken)	return eNothingLeftToParse;
			else if (outToken->GetValue() != LEFT_PARENTHESIS)	return eIllegalValueParsed;

			// Next comes the comma list.  The end of this list (which could have zero items) in it
			// is actually a right paren.
			bool bParsedAParam = false;
			do {
				// Peek to see whether we've found the end of the list, or perhaps a comma if we've
				// already parsed at least one parameter.
				outToken = PeekNextToken();
				if (!outToken)	return eNothingLeftToParse;
				else if (outToken->GetValue() == RIGHT_PARENTHESIS) {
					GetNextToken();
					return eParseSucceeded;
				} else if (bParsedAParam) {
					// Once we've parsed a parameter, we expect there to be a comma if there's any
					// other parameters left
					if (outToken->GetValue() == COMMA) {
						GetNextToken();
					} else {
						return eIllegalValueParsed;
					}
				}

				// If it's not the end of the list, it needs to be a scalar expression of some sort
				ParserReturnValue helperRet = ParseScalarExpression( outSuggestions, outToken );
				if (helperRet != eParseSucceeded)	return helperRet;
				bParsedAParam = true;
			} while (true);
		}
		return helperRet;		
	}

	return eIllegalValueParsed;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseSearchConditionTerminal( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	outToken = PeekNextToken();

	if (!outToken) {
		outSuggestions.Suggest( "NOT", SuggestionInfo::eKeyword );
		ParsePredicate( outSuggestions, outToken );
		return eNothingLeftToParse;
	}

	switch (outToken->GetValue()) {
		case NOT: {
			// We expect a terminal to follow immediately after.
			GetNextToken();
			return ParseSearchConditionTerminal( outSuggestions, outToken );
		} break;
		case LEFT_PARENTHESIS: {
			GetNextToken();
			// This expects another search condition followed by a right paren
			ParserReturnValue helperRet = ParseSearchCondition( outSuggestions, outToken );
			if (helperRet != eParseSucceeded)	return helperRet;
			outToken = GetNextToken();
			if (!outToken)										return eNothingLeftToParse;
			else if (outToken->GetValue() != RIGHT_PARENTHESIS)	return eIllegalValueParsed;
			else												return eParseSucceeded;
		} break;
		default: {
			// The only other thing we could possibly be is a predicate
			return ParsePredicate( outSuggestions, outToken );
		} break;
	}

	return eIllegalValueParsed;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseSearchCondition( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// Search conditions are left-recursive in the same manner as scalar expressions, so
	// we will handle them in much the same way: by using shunting yard.  For more information,
	// see the comments at the start of the ParseScalarExpression method.
	ParserReturnValue helperRet = ParseSearchConditionTerminal( outSuggestions, outToken );
	if (helperRet != eParseSucceeded)	return helperRet;

	bool done = true;
	do {
		// Now peek to see whether we've got a binary operator that we need to care about
		ILexerToken *temp = PeekNextToken();
		if (!temp) {
			// While the parse is technically done, it's still possible that the caller has
			// some choices they might want to make.
			outSuggestions.Suggest( "OR", SuggestionInfo::eKeyword );
			outSuggestions.Suggest( "AND", SuggestionInfo::eKeyword );
			return eParseSucceeded;
		}

		switch (temp->GetValue()) {
			case OR:
			case AND: {
				// We need to consume this token, and then parse the other terminal
				outToken = GetNextToken();
				helperRet = ParseSearchConditionTerminal( outSuggestions, outToken );
				if (helperRet != eParseSucceeded)	return helperRet;
				done = false;
			} break;
			default: {
				done = true;
			} break;
		}
	} while (!done);

	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseWhenConditionThenExpression( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// We expect to see a WHEN condition followed by a THEN expression.
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "WHEN", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != WHEN)	return eIllegalValueParsed;

	// Now we expect to see a search condition
	ParserReturnValue helperRet = ParseSearchCondition( outSuggestions, outToken );
	if (helperRet != eParseSucceeded)	return helperRet;

	// Finally, we expect a THEN expression
	return ParseThenExpression( outSuggestions, outToken );
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseWhenConditionThenExpressionList( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// This is a series of at least one WHEN conditions followed by THEN expressions, 
	// possibly more.  However, there's no separator between the list parts.  We simply
	// need to check for the WHEN clause to determine whether there's another item in the list.
	while (true) {
		// Parse the WHEN condition THEN expression clause
		ParserReturnValue ret = ParseWhenConditionThenExpression( outSuggestions, outToken );
		if (ret != eParseSucceeded)	return ret;

		// Now check to see if there's another WHEN clause.  If there is, then we've got
		// another item in the list we want to process.  If there's not one, then we're done
		ILexerToken *temp = PeekNextToken();
		if (!temp) {
			outSuggestions.Suggest( "WHEN", SuggestionInfo::eKeyword );
			return eParseSucceeded;
		} else if (temp->GetValue() != WHEN)	return eParseSucceeded;
	}
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseOptionalElseExpression( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// Peek at the next token to see if it's an ELSE.  If it is, then we have an ELSE
	// expression and are followed by a scalar expression.
	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		outSuggestions.Suggest( "ELSE", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (temp->GetValue() != ELSE)	return eParseSucceeded;

	// Consume the token
	outToken = GetNextToken();

	// Parse the scalar expression
	return ParseScalarExpression( outSuggestions, outToken );
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseCaseExpression( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "CASE", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	}

	// There are two productions for the base CASE expression.  One expects a scalar expression
	// to follow the CASE token, the other expects a WHEN clause.  We'll peek at the next token
	// to see which of these cases we're after.
	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		outSuggestions.Suggest( "WHEN", SuggestionInfo::eKeyword );
		ParseScalarExpression( outSuggestions, outToken );
		return eNothingLeftToParse;
	}

	ParserReturnValue helperRet;
	if (temp->GetValue() == WHEN) {
		// We're expecting a list of WHEN conditions followed by a THEN expressions.
		helperRet = ParseWhenConditionThenExpressionList( outSuggestions, outToken );
		if (helperRet != eParseSucceeded)	return helperRet;

		// Next we're expecting an optional else expression
		helperRet = ParseOptionalElseExpression( outSuggestions, outToken );
		if (helperRet != eParseSucceeded)	return helperRet;
	} else {

	}

	// Now we expect an END token
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.Suggest( "END", SuggestionInfo::eKeyword );
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != END)	return eIllegalValueParsed;

	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseScalarExpression( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// We need to parse a terminal, and then check to see if we have any of the binary
	// operators which require a second pass.  Because of the left-recursion inherent in the
	// grammar, we are going to use the Shunting Yard Algorithm to do the parsing -- this 
	// removes the possibility for infinite recusion by reordering the the grammar in a way
	// that is equivalent, but more obtuse.  For more information on this implementation, see
	// <http://www.engr.mun.ca/~theo/Misc/exp_parsing.htm>
	ParserReturnValue helperRet = ParseTerminal( outSuggestions, outToken );
	if (helperRet != eParseSucceeded)	return helperRet;

	bool done = true;
	do {
		// Now peek to see whether we've got a binary operator that we need to care about
		ILexerToken *temp = PeekNextToken();
		if (!temp)	return eParseSucceeded;

		switch (temp->GetValue()) {
			case PLUS:
			case MINUS:
			case SQL_MULTIPLY:
			case DIVIDE: {
				// We need to consume this token, and then parse the other terminal
				outToken = GetNextToken();
				helperRet = ParseTerminal( outSuggestions, outToken );
				if (helperRet != eParseSucceeded)	return helperRet;
				done = false;
			} break;
			default: {
				done = true;
			} break;
		}
	} while (!done);

	return eParseSucceeded;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseAssignment( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// We want to parse a column, followed by some form of comparison expression.
	outToken = GetNextToken();
	if (!outToken) {
		outSuggestions.SuggestColumns();
		return eNothingLeftToParse;
	} else if (outToken->GetValue() != NAME)	return eIllegalValueParsed;

	// Next we expect a comparison
	outToken = GetNextToken();
	if (!outToken)	return eNothingLeftToParse;
	else if (outToken->GetValue() != COMPARISON)	return eIllegalValueParsed;

	// Finally, we expect to see either a NULL token, or a scalar expression
	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		outSuggestions.Suggest( "NULL", SuggestionInfo::eKeyword );
		ParseScalarExpression( outSuggestions, outToken );
		return eNothingLeftToParse;
	} else if (temp->GetValue() == NULLX) {
		// We found the NULL token, so consume it and we're done
		outToken = GetNextToken();
		return eParseSucceeded;
	} else {
		return ParseScalarExpression( outSuggestions, outToken );
	}
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseAssignmentList( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// Parse the comma-separated assignment list.
	while (true) {
		// Parse an assignment
		ParserReturnValue helperRet = ParseAssignment( outSuggestions, outToken );
		if (helperRet != eParseSucceeded)	return helperRet;

		// Check to see whether we have a COMMA
		ILexerToken *temp = PeekNextToken();
		if (!temp) {
			return eParseSucceeded;
		} else if (temp->GetValue() == COMMA) {
			// Consume the COMMA
			GetNextToken();
		} else {
			return eParseSucceeded;
		}
	}	
}

bool VSQLSyntax::ParseUpdateStatement( SuggestionList &outSuggestions )
{
	// We've just parsed the UPDATE clause, so the next thing we expect is the table
	// to be updated.
	ILexerToken *token;
	ParserReturnValue helperRet = ParseTable( outSuggestions, token );
	if (helperRet == eNothingLeftToParse)		return true;
	else if (helperRet == eIllegalValueParsed)	return false;

	// We expect to see the SET keyword next
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "SET", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != SET)		return false;

	// Now we're going to parse the list of assignments
	helperRet = ParseAssignmentList( outSuggestions, token );
	if (helperRet == eNothingLeftToParse)		return true;
	else if (helperRet == eIllegalValueParsed)	return false;

	// Now comes an optional WHERE clause, so peek at the next token to see if
	// we need to parse further or not
	token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "WHERE", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != WHERE)	return true;

	// We do have a WHERE clause, so consume the token and handle the search condition
	GetNextToken();
	helperRet = ParseSearchCondition( outSuggestions, token );
	if (helperRet == eNothingLeftToParse)		return true;
	else if (helperRet == eIllegalValueParsed)	return false;

	return true;
}

bool VSQLSyntax::ParseDeleteStatement( SuggestionList &outSuggestions )
{
	// We've read the DELETE token, and now we expect to see the FROM
	ILexerToken *token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "FROM", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != SQL_KWORD_FROM)	return false;

	// Now get the table from which we're deleting
	ParserReturnValue helperRet = ParseTable( outSuggestions, token );
	if (helperRet == eNothingLeftToParse)		return true;
	else if (helperRet == eIllegalValueParsed)	return false;

	// Now comes an optional WHERE clause, so peek at the next token to see if
	// we need to parse further or not
	token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "WHERE", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != WHERE)	return true;

	// We do have a WHERE clause, so consume the token and handle the search condition
	GetNextToken();
	helperRet = ParseSearchCondition( outSuggestions, token );
	if (helperRet == eNothingLeftToParse)		return true;
	else if (helperRet == eIllegalValueParsed)	return false;

	return true;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseAon( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// We expect either NULL, INFILE, INF, -INF or a scalar expression.  Thanks to the -INF,
	// we have a slight problem.  The MINUS token could be the start of a scalar expression, so
	// we are going to use two tokens of lookahead to keep the parser design clean.
	ILexerToken *temp = PeekNextToken();
	if (!temp) {
		outSuggestions.Suggest( "INFILE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "INF", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "-INF", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "NULL", SuggestionInfo::eKeyword );
		ParseScalarExpression( outSuggestions, outToken );
		return eNothingLeftToParse;
	}

	switch (temp->GetValue()) {
		case NULLX:
		case SQL_KWORD_INF: {
			// Just consume the token -- we're done
			outToken = GetNextToken();
			return eParseSucceeded;
		} break;
		case MINUS: {
			// This is a bit of a mess -- we need to consume the MINUS and peek at the next
			// token to see if the user is doing a -INF or a scalar expression.
			GetNextToken(); // Consume

			// Peek to see if there's an INF
			temp = PeekNextToken();
			if (!temp) {
				outSuggestions.Suggest( "INF", SuggestionInfo::eKeyword );
				ParseScalarExpression( outSuggestions, outToken );
				return eNothingLeftToParse;
			} else if (temp->GetValue() == SQL_KWORD_INF) {
				// Aha!  The user typed -INF, so consume this token and we're done
				outToken = GetNextToken();
				return eParseSucceeded;
			} else {
				// Must be a scalar expression -- put back the minus and parse that way
				PutBackToken();
				return ParseScalarExpression( outSuggestions, outToken );
			}
		} break;
		case SQL_KWORD_INFILE: {
			// Consume the token, and we expect a scalar expression to follow
			GetNextToken();
			return ParseScalarExpression( outSuggestions, outToken );
		} break;
		default: {
			// This must be a scalar expression
			return ParseScalarExpression( outSuggestions, outToken );
		} break;
	}
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseListOfAonLists( class SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// Parse the comma-separated aon list.
	while (true) {
		// Parse an aon list
		ParserReturnValue helperRet = ParseAonList( outSuggestions, outToken );
		if (helperRet != eParseSucceeded)	return helperRet;

		// Check to see whether we have a COMMA
		ILexerToken *temp = PeekNextToken();
		if (!temp) {
			return eParseSucceeded;
		} else if (temp->GetValue() == COMMA) {
			// Consume the COMMA
			GetNextToken();
		} else {
			return eParseSucceeded;
		}
	}
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseAonList( SuggestionList &outSuggestions, class ILexerToken * &outToken )
{
	// First, we expect to see a left paren
	outToken = GetNextToken();
	if (!outToken)										return eNothingLeftToParse;
	else if (outToken->GetValue() != LEFT_PARENTHESIS)	return eIllegalValueParsed;

	// Parse the comma-separated aon list.
	while (true) {
		// Parse an aon
		ParserReturnValue helperRet = ParseAon( outSuggestions, outToken );
		if (helperRet != eParseSucceeded)	return helperRet;

		// Check to see whether we have a COMMA
		ILexerToken *temp = PeekNextToken();
		if (!temp) {
			return eParseSucceeded;
		} else if (temp->GetValue() == COMMA) {
			// Consume the COMMA
			GetNextToken();
		} else {
			break;
		}
	}

	// Get the closing paren
	outToken = GetNextToken();
	if (!outToken)										return eNothingLeftToParse;
	else if (outToken->GetValue() != RIGHT_PARENTHESIS)	return eIllegalValueParsed;
	
	return eParseSucceeded;
}

bool VSQLSyntax::ParseInsertStatement( SuggestionList &outSuggestions )
{
	// We've read the INSERT token, and now we want to parse the rest of the insert
	// statement.  We always expect to see an INTO table clause first.
	ILexerToken *token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "INTO", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != INTO)	return eIllegalValueParsed;

	ParserReturnValue helperRet = ParseTable( outSuggestions, token );
	if (helperRet == eNothingLeftToParse)		return true;
	else if (helperRet == eIllegalValueParsed)	return false;

	// The next bit gets very tricky because we can expect one of a few different
	// clauses.  Either there's a left paren followed by a column list, or a left
	// paren followed by a subquery, or a VALUES statement, or even a subquery body
	// which starts with a SELECT.  So it's a bit of a mess.
	//
	// We're going to cheat a little bit and use multiple tokens of lookahead.  This
	// is disgusting, but makes life a lot easier.  If we see a ( SELECT then we know
	// that we're parsing the subquery version, otherwise, if we see at least the (, we
	// know there's a column list and can go from there.
	token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "VALUES", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "SELECT", SuggestionInfo::eKeyword );
		return true;
	}

	// Check to see whether we have a left paren or not
	bool hasLeftParen = (token->GetValue() == LEFT_PARENTHESIS);
	bool isSubquery = false;

	// If there's a left paren, check to see whether the next token is a SELECT
	// statement or not.
	if (hasLeftParen) {
		// Consume the paren token
		GetNextToken();

		// See if we've got a SELECT
		token = PeekNextToken();
		if (!token) {
			outSuggestions.Suggest( "SELECT", SuggestionInfo::eKeyword );
			ParseColumnRefList( outSuggestions, token );
			return true;
		} else if (token->GetValue() == SELECT) {
			// We've got a subquery!
			isSubquery = true;
		}

		// Put the paren back
		PutBackToken();
	}

	// There are three ways this production can go, and we are now able to tell
	// the difference between them.  Either we're expecting only a subquery, or
	// starting with a column list, or expecting a values/query specification.
	if (isSubquery) {
		// We expect to only see a subquery and then we're done
		helperRet = ParseSubquery( outSuggestions, token );
		if (helperRet == eNothingLeftToParse)		return true;
		else if (helperRet == eIllegalValueParsed)	return false;
		else										return true;
	}

	// If we have an opening paren, then we know we need to parse the column 
	// list.
	if (hasLeftParen) {
		// Consume the left paren
		GetNextToken();
		helperRet = ParseColumnRefList( outSuggestions, token );
		if (helperRet == eNothingLeftToParse)		return true;
		else if (helperRet == eIllegalValueParsed)	return false;

		// Now we need to consume the right paren
		token = GetNextToken();
		if (!token)	return true;
		else if (token->GetValue() != RIGHT_PARENTHESIS)	return false;
	}

	// Finally, we expect to see either a VALUES clause, a ( or a SELECT
	// statement.
	token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "VALUES", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "SELECT", SuggestionInfo::eKeyword );
		return true;
	}

	if (token->GetValue() == VALUES) {
		// We've gotten a VALUE clause, so we now expect to see a list of
		// "aon"s in parens.  Consume the VALUES token
		GetNextToken();

		helperRet = ParseListOfAonLists( outSuggestions, token );
		if (helperRet == eNothingLeftToParse)		return true;
		else if (helperRet == eIllegalValueParsed)	return false;
		else										return true;
	} else if (token->GetValue() == SELECT) {
		// We expect just the subquery body here 
		helperRet = ParseSubquery( outSuggestions, token, true );
		if (helperRet == eNothingLeftToParse)		return true;
		else if (helperRet == eIllegalValueParsed)	return false;
		else										return true;
	} else if (token->GetValue() == LEFT_PARENTHESIS) {
		// We expect a qubquery here and then we're done
		helperRet = ParseSubquery( outSuggestions, token );
		if (helperRet == eNothingLeftToParse)		return true;
		else if (helperRet == eIllegalValueParsed)	return false;
		else										return true;
	}

	return false;
}

bool VSQLSyntax::ParseCreateViewStatement( SuggestionList &outSuggestions )
{
	// We've parsed a CREATE VIEW clause, so now we expect the name of the view to
	// create, possibly followed by an optional list of other names.
	ILexerToken *token = GetNextToken();
	if (!token)	return true;
	else if (token->GetValue() != NAME)	return false;

	// Now we can have an optional '.NAME'
	token = PeekNextToken();
	if (token && token->GetValue() == PERIOD) {
		GetNextToken();
		token = PeekNextToken();
		if (!token) return true;

		if (token->GetValue() != NAME) return false;
		GetNextToken();
	}

	// Now we have an optional list "( NAME, NAME, ... )" of other names separated by commas.  To make the loop
	// a bit easier to handle, we'll peek to see whether there's a token or not.  If there's
	// not a token, then we expect the AS clause.  If there is a token, we'll check to see
	// whether it's a NAME or not.  Regardless, the next clause handler will take care of
	// adding the suggestion for the AS clause.
	if (PeekNextToken()) {
		token = PeekNextToken();
		if (token->GetValue() == LEFT_PARENTHESIS)
		{
			// Consume '('
			GetNextToken();

			while (true) {
				// See if we have a NAME token
				token = PeekNextToken();
				if (!token) return true;
				else if (token->GetValue() != NAME) break;	// We're done with the list

				// Consume the NAME
				GetNextToken();

				// Check to see whether we have a comma or a right parenthesis, and if not, we're done
				token = PeekNextToken();
				if ( token && token->GetValue() == RIGHT_PARENTHESIS ) {
					GetNextToken();

					break;
				}

				if (!token || token->GetValue() != COMMA)	return false;	// We're done with the list

				// Consume the COMMA
				GetNextToken();
			}
		}
	}

	// Now we expect the AS clause
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "AS", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != AS)	return false;

	// Now we expect something close to a SELECT statement, but significantly more restricted
	token = GetNextToken();
	if (!token) {
		outSuggestions.Suggest( "SELECT", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != SELECT)	return false;

	// We expect the selection to be here
	ParserReturnValue helperRet = ParseSelection( outSuggestions, token );
	if (helperRet == eNothingLeftToParse)		return true;
	else if (helperRet == eIllegalValueParsed)	return false;

	// Next comes the FROM clause
	helperRet = ParseFrom( outSuggestions, token );
	if (helperRet == eNothingLeftToParse)		return true;
	else if (helperRet == eIllegalValueParsed)	return false;

	// The only optional clause CREATE VIEW supports is the optional WHERE clause
	token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "WHERE", SuggestionInfo::eKeyword );
		return true;
	} else if (token->GetValue() != WHERE)	return true;

	// Consume the WHERE token, and get the search condition
	GetNextToken();
	helperRet = ParseSearchCondition( outSuggestions, token );
	if (helperRet == eNothingLeftToParse)		return true;
	else if (helperRet == eIllegalValueParsed)	return false;

	// We're done!
	return true;
}

VSQLSyntax::ParserReturnValue VSQLSyntax::ParseCommandParameter( class SuggestionList &outSuggestions, class ILexerToken *&outToken )
{
	// We expect either a ? or a 4D language reference
	ILexerToken *token = PeekNextToken();
	if (!token) {
		outSuggestions.Suggest( "?", SuggestionInfo::eKeyword );
		Parse4DReference( outSuggestions, outToken );
		return eNothingLeftToParse;
	}

	if (token->GetValue() == QUESTION_MARK) {
		outToken = GetNextToken();
		return eParseSucceeded;
	} else {
		return Parse4DReference( outSuggestions, outToken );
	}
}

bool VSQLSyntax::ParseTopLevel( SuggestionList &outSuggestions )
{
	// Get the next token, and see whether we're in suggestion mode, or continue
	// parsing mode.
	ILexerToken *token = GetNextToken();
	bool ret = false;

	if (!token) {
		// Fill out the suggestions box and we're done
		outSuggestions.Suggest( "SELECT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DROP INDEX", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DROP TABLE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DROP DATABASE DATAFILE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DROP TABLE IF EXISTS", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DROP VIEW IF EXISTS", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DROP SCHEMA", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "CREATE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "START", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "START TRANSACTION", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "COMMIT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "COMMIT TRANSACTION", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "ROLLBACK", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "ROLLBACK TRANSACTION", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "UPDATE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "CREATE UNIQUE INDEX", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "CREATE INDEX", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "CREATE TABLE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "CREATE VIEW", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "CREATE OR REPLACE VIEW", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "CREATE SCHEMA", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "CREATE DATABASE DATAFILE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "CREATE DATABASE IF NOT EXISTS DATAFILE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "DELETE FROM", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "INSERT INTO", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "ALTER TABLE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "ALTER DATABASE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "ALTER SCHEMA", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "EXECUTE IMMEDIATE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "LOCK TABLE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "UNLOCK TABLE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "GRANT", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "GRANT READ ON", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "GRANT READ_WRITE ON", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "GRANT ALL ON", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "REVOKE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "REVOKE READ ON", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "REVOKE READ_WRITE ON", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "REVOKE ALL ON", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "REPLICATE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "TRUNCATE TABLE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "USE DATABASE DATAFILE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "SYNCHRONIZE", SuggestionInfo::eKeyword );
		outSuggestions.Suggest( "SYNCHRONIZE LOCAL", SuggestionInfo::eKeyword );

		// There are actually two forms of this statement.  One uses DEFAULT, and the other uses 
		// SQL_INTERNAL.  However, these are just synonyms for the same action.  We want to steer 
		// users towards using DEFAULT.  So for the top-level form of the statement, this is the only
		// completion we will show them.  However, once the user has decided to manually enter the USE
		// keyword, we will start suggesting the SQL_INTERNAL form as well, just so they realize that
		// it is around and legal.  Also, the user could specify LOCAL or REMOTE, but we default to
		// LOCAL and want to steer them towards that.
		outSuggestions.Suggest( "USE DATABASE DEFAULT", SuggestionInfo::eKeyword );

		// CREATE VIEW statements are not for public consumption
		// outSuggestions.Suggest( "CREATE VIEW", SuggestionInfo::eKeyword );
		// DEBUG statements are not for public consumption
		// outSuggestions.Suggest( "DEBUG SET THREADING MODE", SuggestionInfo::eKeyword );

		ret = true;
	} else {
		// Find out what sort of statement we're looking at
		switch (token->GetValue()) {
			case SQL_KWORD_SYNCHRONIZE: {
				ret = ParseSynchronizeStatement( outSuggestions );
			} break;
			case SELECT: {
				ParserReturnValue helperRet = ParseSelectStatement( outSuggestions, token, false );
				ret = (helperRet == eNothingLeftToParse || helperRet == eParseSucceeded);
			} break;

			case SQL_KWORD_TRUNCATE: {
				// The TABLE keyword is optional
				token = PeekNextToken();
				if (!token) {
					outSuggestions.Suggest( "TABLE", SuggestionInfo::eKeyword );
					ParseTableRef( outSuggestions, token );
					ret = true;
					break;
				} else if (token->GetValue() == TABLE)	GetNextToken();

				// Finally, we expect a table reference here
				ParserReturnValue helperRet = ParseTableRef( outSuggestions, token );
				ret = (helperRet == eNothingLeftToParse || helperRet == eParseSucceeded);
			} break;

			case SQL_KWORD_USE: {
				// Optionally, we can have a LOCAL or REMOTE keyword here.
				token = PeekNextToken();
				if (!token) {
					outSuggestions.Suggest( "LOCAL DATABASE DATAFILE", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "LOCAL DATABASE DEFAULT", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "LOCAL DATABASE SQL_INTERNAL", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "REMOTE DATABASE DATAFILE", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "REMOTE DATABASE DEFAULT", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "REMOTE DATABASE SQL_INTERNAL", SuggestionInfo::eKeyword );
					ret = true;
					break;
				} else if (token->GetValue() == SQL_KWORD_LOCAL ||
						token->GetValue() == SQL_KWORD_REMOTE) {
					// Eat the token
					GetNextToken();
				}

				// The next token should be DATABASE
				token = GetNextToken();
				if (!token) {
					outSuggestions.Suggest( "DATABASE DATAFILE", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "DATABASE DEFAULT", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "DATABASE SQL_INTERNAL", SuggestionInfo::eKeyword );
					ret = true;
					break;
				} else if (token->GetValue() != DATABASE) {
					ret = false;
					break;
				}

				// The next token should be DATAFILE or DEFAULT/SQL_INTERNAL.  If it's DEFAULT, then we're done with the
				// statement.  But if it's DATAFILE or SQL_INTERNAL, then we expect to see the file name as a string,
				token = GetNextToken();
				if (!token) {
					outSuggestions.Suggest( "DATAFILE", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "DEFAULT", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "SQL_INTERNAL", SuggestionInfo::eKeyword );
					ret = true;
					break;
				} else if (token->GetValue() == SQL_KWORD_DEFAULT || token->GetValue() == SQL_KWORD_SQL_INTERNAL) {
					// DEFAULT and SQL_INTERNAL are synonyms, so either keyword is acceptable here.
					ret = true;
					break;
				} else if (token->GetValue() != DATA_FILE) {
					ret = false;
					break;
				}

				// Now we expect to get either a STRING token, or a command parameter.
				token = PeekNextToken();
				if (!token) {
					ParseCommandParameter( outSuggestions, token );
					ret = true;
					break;
				} else if (token->GetValue() == STRING) {
					GetNextToken();
					ret = true;
				} else {
					ParserReturnValue helperRet = ParseCommandParameter( outSuggestions, token );
					if (eNothingLeftToParse == helperRet)		ret = true;
					else if(eIllegalValueParsed == helperRet)	ret = false;
					else										ret = true;
				}

				// If we've already fail, then bail out
				if (!ret)	break;

				// Now there's an optional AUTO_CLOSE clause that we can use
				token = PeekNextToken();
				if (!token) {
					outSuggestions.Suggest( "AUTO_CLOSE", SuggestionInfo::eKeyword );
					break;
				} else if (token->GetValue() == SQL_KWORD_AUTO_CLOSE) {
					GetNextToken();
				}
			} break;

			case DROP: {
				// We have a DROP statement, however, we need to figure out which one we have before we
				// can properly parse it.  It could be an index, table, or a schema -- it all depends on
				// the next keyword.
				token = GetNextToken();
				if (!token) {
					outSuggestions.Suggest( "TABLE", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "TABLE IF EXISTS", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "VIEW", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "VIEW IF EXISTS", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "INDEX", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "SCHEMA", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "DATABASE DATAFILE", SuggestionInfo::eKeyword );
					ret = true;
				} else {
					// This token should tell us what sort of statement we're looking at
					switch (token->GetValue()) {
						case DATABASE: {
							// The next token should be DATAFILE
							token = GetNextToken();
							if (!token) {
								outSuggestions.Suggest( "DATAFILE", SuggestionInfo::eKeyword );
								ret = true;
								break;
							} else if (token->GetValue() != DATA_FILE) {
								ret = false;
								break;
							}

							// Finally, we expect a string for the datafile to be dropping
							token = GetNextToken();
							if (!token || token->GetValue() == STRING) {
								ret = true;
							} else {
								ret = false;
							}
						} break;
						case VIEW: {
						} // Fall through to TABLE:
						case TABLE: {
							int			nType = token->GetValue();
							// DROP TABLE has an optional "if exists" clause.  The final token in the statement
							// is the table name itself
							token = PeekNextToken();
							if (!token) {
								outSuggestions.Suggest( "IF EXISTS", SuggestionInfo::eKeyword );
								ParseTable( outSuggestions, token, NULL, ( nType == TABLE ), ( nType == VIEW ) );
								ret = true;
								break;
							}
							
							if (token->GetValue() == SQL_KWORD_IF) {
								// We have an IF clause, so now we need to check for the EXISTS clause.  At this point
								// it's no longer optional
								GetNextToken();		// Consume the IF
								token = GetNextToken();
								if (!token) {
									outSuggestions.Suggest( "EXISTS", SuggestionInfo::eKeyword );
									ret = true;
									break;
								} else if (token->GetValue() != EXISTS)	ret = false;

								// Peek at the next token, if it doesn't exist then we want the user to enter a table name
								token = PeekNextToken();
								if (!token) {
									ParseTable( outSuggestions, token, NULL, ( nType == TABLE ), ( nType == VIEW ) );
									ret = true;
									break;
								}
							}
							
							if (token->GetValue() == NAME) {
								GetNextToken();  // Consume the token
								// We have the table name, so we're done
								ret = true;
							}
						} break;
						case SQL_KWORD_SCHEMA: {
							// The only thing we expect here is a name
							token = GetNextToken();
							if (!token) {
								outSuggestions.SuggestSchemas();
								ret = true;
							} else if (token->GetValue() == NAME)	ret = true;
						} break;
						case INDEX: {
							// The only thing we expect here is a name
							token = GetNextToken();
							if (!token) {
								outSuggestions.SuggestIndexes();
								ret = true;
							} else if (token->GetValue() == NAME)	ret = true;
						} break;
						default: {
							// This is some kind of DROP statement we don't support, so bail out
							ret = false;
						} break;
					}
				}
			} break;

			case CREATE: {
				// The next keyword will tell us what it is the user wants to create.
				token = GetNextToken();
				if (!token) {
					outSuggestions.Suggest( "TABLE", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "TABLE IF NOT EXISTS", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "VIEW", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "OR REPLACE VIEW", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "INDEX", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "UNIQUE INDEX", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "SCHEMA", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "DATABASE DATAFILE", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "DATABASE IF NOT EXISTS DATAFILE", SuggestionInfo::eKeyword );
					ret = true;
				} else {
					// We've gotten a token, so let's see what the user wants to create
					switch (token->GetValue()) {
						case TABLE: {
							ret = ParseCreateTableStatement( outSuggestions );
						} break;
						case VIEW: {
							ret = ParseCreateViewStatement( outSuggestions );
						} break;
						case OR: {
							// We expect to see REPLACE VIEW now

							token = GetNextToken();
							if (!token) {
								outSuggestions.Suggest( "REPLACE VIEW", SuggestionInfo::eKeyword );
								ret = true;
								break;
							} else if (token->GetValue() == SQL_FUNC_REPLACE) {
								token = GetNextToken();
								if (!token) {
									outSuggestions.Suggest( "VIEW", SuggestionInfo::eKeyword );
									ret = true;
									break;
								} else if (token->GetValue() != VIEW) {
									ret = false;
									break;
								}
							}

							ret = ParseCreateViewStatement( outSuggestions );
						} break;
						case SQL_KWORD_SCHEMA: {
							// The only thing we expect to see here is a name
							token = GetNextToken();
							if (!token) {
								// However, there is nothing to suggest -- the user is to create a new one!
								ret = true;
							} else if (token->GetValue() == NAME)	ret = true;
						} break;
						case DATABASE: {
							// We expect to see a data file followed by a string (not a back-tic'ed one, either!) However,
							// in either case, there's nothing we can really suggest to the user.  So we just check types
							// to make sure they're on the right track.  There's also an optional IF NOT EXISTS clause that
							// the user can specify before the DATAFILE keyword.
							token = GetNextToken();
							if (!token) {
								outSuggestions.Suggest( "IF NOT EXISTS DATAFILE", SuggestionInfo::eKeyword );
								outSuggestions.Suggest( "DATAFILE", SuggestionInfo::eKeyword );
								ret = true;
								break;
							} else if (token->GetValue() == SQL_KWORD_IF) {
								// Parse out the rest of the IF NOT EXISTS clause
								token = GetNextToken();
								if (!token) {
									outSuggestions.Suggest( "NOT EXISTS DATAFILE", SuggestionInfo::eKeyword );
									ret = true;
									break;
								} else if (token->GetValue() != NOT) {
									ret = false;
									break;
								}

								token = GetNextToken();
								if (!token) {
									outSuggestions.Suggest( "EXISTS DATAFILE", SuggestionInfo::eKeyword );
									ret = true;
									break;
								} else if (token->GetValue() != EXISTS) {
									ret = false;
									break;
								}

								// Get the next token, which we hope is DATAFILE
								token = GetNextToken();
								if (!token) {
									outSuggestions.Suggest( "DATAFILE", SuggestionInfo::eKeyword );
									ret = true;
									break;
								}
							}

							// Now we expect to get the DATAFILE keyword, which can be followed by either a
							// STRING token, or a command parameter.
							if (token->GetValue() == DATA_FILE) {
								token = PeekNextToken();
								if (!token) {
									ParseCommandParameter( outSuggestions, token );
									ret = true;
								} else if (token->GetValue() == STRING) {
									GetNextToken();
									ret = true;
								} else {
									ParserReturnValue helperRet = ParseCommandParameter( outSuggestions, token );
									if (eNothingLeftToParse == helperRet)		ret = true;
									else if(eIllegalValueParsed == helperRet)	ret = false;
									else										ret = true;
								}
							} else {
								ret = false;
							}
						} break;
						case SQL_KW_UNIQUE:
							// We expect the next token to be INDEX -- this is mandatory at this point
							token = GetNextToken();
							if (!token) {
								outSuggestions.Suggest( "INDEX", SuggestionInfo::eKeyword );
								ret = true;
								break;
							} else if (token->GetValue() != INDEX) {
								ret = false;
								break;
							}
							// We *want* to fall through!  That's why there's no break for this case!
						case INDEX: {
							ret = ParseCreateIndexStatement( outSuggestions );
						} break;
					}
				}
			} break;

			case START:
			case COMMIT:
			case ROLLBACK: {
				// It's possible that we have an optional keyword for TRANSACTION.  If we do, then
				// great.  If not, no worries.
				token = GetNextToken();
				if (!token) {
					outSuggestions.Suggest( "TRANSACTION", SuggestionInfo::eKeyword );
					ret = true;
				} else if (token->GetValue() == TRANSACTION)	ret = true;
			} break;

			case UPDATE: {
				ret = ParseUpdateStatement( outSuggestions );
			} break;

			case SQL_KWORD_DELETE: {
				ret = ParseDeleteStatement( outSuggestions );
			} break;

			case INSERT: {
				ret = ParseInsertStatement( outSuggestions );
			} break;

			case ALTER: {
				// There are three types of alters -- one for tables, databases, and schemas.  The next token tells
				// us what type of alter the user wants
				token = GetNextToken();
				if (!token) {
					outSuggestions.Suggest( "TABLE", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "SCHEMA", SuggestionInfo::eKeyword );
					ret = true;
				} else if (token->GetValue() == TABLE) {
					// This is an ALTER TABLE statement
					ret = ParseAlterTableStatement( outSuggestions );
				} else if (token->GetValue() == DATABASE) {
					// The caller can either enable or disable one of two things: properties or constraints.  So
					// we expect to see the state token first, and then which state to alter next.
					token = GetNextToken();
					if (!token) {
						outSuggestions.Suggest( "ENABLE", SuggestionInfo::eKeyword );
						outSuggestions.Suggest( "DISABLE", SuggestionInfo::eKeyword );
						ret = true;
						break;
					} else if (token->GetValue() != SQL_KWORD_ENABLE && token->GetValue() != SQL_KWORD_DISABLE) {
						ret = false;
						break;
					}

					// Now we expect to see one of the states being altered
					token = GetNextToken();
					if (!token) {
						outSuggestions.Suggest( "INDEXES", SuggestionInfo::eKeyword );
						outSuggestions.Suggest( "CONSTRAINTS", SuggestionInfo::eKeyword );
						ret = true;
						break;
					}

					ret = (token->GetValue() == SQL_KWORD_INDEXES || token->GetValue() == SQL_KWORD_CONSTRAINTS);
				} else if (token->GetValue() == SQL_KWORD_SCHEMA) {
					// This is an ALTER SCHEMA statement.  This is pretty easy, since you're renaming the
					// schema from one name to another.
					token = GetNextToken();
					if (!token) {
						outSuggestions.SuggestSchemas();
						ret = true;
						break;
					} else if (token->GetValue() != NAME) {
						ret = false;
						break;
					}

					// After the name we expect to see a RENAME keyword for the RENAME TO clause
					token = GetNextToken();
					if (!token) {
						outSuggestions.Suggest( "RENAME TO", SuggestionInfo::eKeyword );
						ret = true;
						break;
					} else if (token->GetValue() != SQL_KWORD_RENAME) {
						ret = false;
						break;
					}

					// Now we expect the TO keyword
					token = GetNextToken();
					if (!token) {
						outSuggestions.Suggest( "TO", SuggestionInfo::eKeyword );
						ret = true;
						break;
					} else if (token->GetValue() != SQL_KWORD_TO) {
						ret = false;
						break;
					}

					// Finally, we expect a NAME token
					token = GetNextToken();
					ret = (!token || (token && token->GetValue() == NAME));
				} else {
					ret = false;
				}
			} break;

			case EXECUTE: {
				ret = ParseExecuteImmediateStatement( outSuggestions );
			} break;

			case LOCK: {
				ret = ParseLockStatement( outSuggestions );
			} break;

			case UNLOCK: {
				// Unlock is another easy case -- we expect to see UNLOCK TABLE table
				token = GetNextToken();
				if (!token) {
					outSuggestions.Suggest( "TABLE", SuggestionInfo::eKeyword );
					ret = true;
				} else if (token->GetValue() != TABLE)	ret = false;

				if (token) {	// Don't check for another token if we've already made our suggestions
					// Check to see if we have a table token -- if not, we need to suggest one
					token = GetNextToken();
					if (!token) {
						outSuggestions.SuggestTables();
						ret = true;
					} else if (token->GetValue() != NAME && token->GetType() != ILexerToken::TT_STRING) {
						ret = false;
					} else {
						ret = true;
					}
				}
			} break;

			case SQL_KWORD_GRANT: {
				// The user wants to grant some set of rights, so we just need to figure out what
				// rights and where they go.
				token = GetNextToken();
				if (!token) {
					outSuggestions.Suggest( "READ ON", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "READ_WRITE ON", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "ALL ON", SuggestionInfo::eKeyword );
					ret = true;
					break;
				} else if (token->GetValue() != SQL_KWORD_READ && 
							token->GetValue() != SQL_KWORD_READ_WRITE &&
							token->GetValue() != ALL) {
					ret = false;
					break;
				}

				// Next, we always expect the ON keyword
				token = GetNextToken();
				if (!token) {
					outSuggestions.Suggest( "ON", SuggestionInfo::eKeyword );
					ret = true;
					break;
				} else if (token->GetValue() != ON) {
					ret = false;
					break;
				}

				// Now we expect to get the NAME of the schema
				token = GetNextToken();
				if (!token) {
					outSuggestions.SuggestSchemas();
					ret = true;
					break;
				} else if (token->GetValue() != NAME) {
					ret = false;
					break;
				}

				// Now comes the TO keyword, followed by the name of the group we're granting rights to
				token = GetNextToken();
				if (!token) {
					outSuggestions.Suggest( "TO", SuggestionInfo::eKeyword );
					ret = true;
					break;
				} else if (token->GetValue() != SQL_KWORD_TO) {
					ret = false;
					break;
				}

				token = GetNextToken();
				if (!token) {
					// We expect a group name here
					outSuggestions.SuggestGroups();
					ret = true;
				} else if (token->GetValue() != NAME) {	
					ret = false;
				} else {
					ret = true;
				}
			} break;

			case SQL_KWORD_REVOKE: {
				// The user wants to revoke some set of rights, so we just need to figure out what rights
				token = GetNextToken();
				if (!token) {
					outSuggestions.Suggest( "READ ON", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "READ_WRITE ON", SuggestionInfo::eKeyword );
					outSuggestions.Suggest( "ALL ON", SuggestionInfo::eKeyword );
					ret = true;
					break;
				} else if (token->GetValue() != SQL_KWORD_READ && 
							token->GetValue() != SQL_KWORD_READ_WRITE &&
							token->GetValue() != ALL) {
					ret = false;
					break;
				}

				// Next, we always expect the ON keyword
				token = GetNextToken();
				if (!token) {
					outSuggestions.Suggest( "ON", SuggestionInfo::eKeyword );
					ret = true;
					break;
				} else if (token->GetValue() != ON) {
					ret = false;
					break;
				}

				// Now we expect to get the NAME of the schema
				token = GetNextToken();
				if (!token) {
					outSuggestions.SuggestSchemas();
					ret = true;
					break;
				} else if (token->GetValue() != NAME) {
					ret = false;
					break;
				} else {
					ret = true;
				}
			} break;

			case DEBUG: {
				// DEBUG statements are not for public consumption, so we don't want to complete them
				// ret = ParseDebugStatement( outSuggestions );
				ret = false;
			} break;

			case SQL_KWORD_REPLICATE: {
				ret = ParseReplicateStatement( outSuggestions );
			} break;

			case END: {
				// There's only one keyword possible after end, which is SQL (to 
				// terminate the SQL block).  So we will test that out here.
				token = GetNextToken();
				if (!token) {
					outSuggestions.Suggest( "SQL", SuggestionInfo::eKeyword );
					ret = true;
				}

				// If there is a token, then something is wrong, and there's no point
				// to continuing.
				ret = false;
			} break;

			default: {
				// This is some other keyword that's starting the sentence that we do
				// not understand.  So bail out
				ret = false;
			} break;

		}
	}

	// Assuming that the top-level statement is complete, we expect the next token
	// to be a semi-colon.  If there isn't a next token, that's fine -- it just means
	// the statement hasn't been finished.  But if there is a token and it's not a semi-
	// colon, then we've got a problem.
	if (ret) {
		token = GetNextToken();
		ret = (!token || token->GetValue() == SEMICOLON);
	}

	return ret;
}

VSQLSyntax::VSQLSyntax()
{
	fTabWidth = 4;

	fTokenizeFuncPtr = NULL;
	fTokenList = NULL;
	fSymTable = NULL;
}


VSQLSyntax::~VSQLSyntax()
{
	if (fTokenList)	delete fTokenList;
	if (fSymTable) fSymTable->Release();
}

void VSQLSyntax::SetSQLTokenizer ( SQLTokenizeFuncPtr inPtr, const std::vector< XBOX::VString * >& vctrSQLKeywords, const std::vector< XBOX::VString * >& vctrSQLFunctions )
{
	fTokenizeFuncPtr = inPtr;

	fKeywords. clear ( );
	std::vector< XBOX::VString * >::const_iterator		citer = vctrSQLKeywords. begin ( );
	while ( citer != vctrSQLKeywords. end ( ) )
	{
		if ( testAssert ( *citer != NULL ) )
		{
			fKeywords. push_back ( ( *citer )-> Clone ( ) );
		}
		citer++;
	}

	fFunctions. clear ( );
	citer = vctrSQLFunctions. begin ( );
	while ( citer != vctrSQLFunctions. end ( ) )
	{
		if ( testAssert ( *citer != NULL ) )
		{
			fFunctions. push_back ( ( *citer )-> Clone ( ) );
		}
		citer++;
	}

	if ( fTokenizeFuncPtr != 0 )
	{
		// This is a unit test of sorts -- we want to make sure that developers do not break
		// this component when they update the grammar file to add new tokens to the SQL parser.
		// When they do that and fail to update this component, they make bugs!  So if that happens,
		// then we bark at them to deal with the problem they caused.
		std::vector< ILexerToken * > tokens;
		VString text = "?";
		( *fTokenizeFuncPtr ) ( text, tokens, false );
		xbox_assert( tokens.size() == 1 );
		if (tokens[ 0 ]->GetValue() != QUESTION_MARK ) {
			/////////////////////// BIG HAIRY NOTE //////////////////
			//
			// You broke the SQL Syntax Engine by adding a new token value to the SQL Server grammar. 
			//
			// Scroll up to the top of this file.  There you will find a list of #define token values, which
			// is what needs to be updated.  Open up SQLParser.tab.h from the SQL Server component, copy
			// the #define list from there, and paste it into this file.  Yes, copy and paste sucks.  However,
			// there is no reasonable solution which allows us to share the enumeration between components so
			// we're stuck with this for now.
			//
			/////////////////////////////////////////////////////////
			XBOX_BREAK_INLINE;	// Look up a comment, it'll be worth it.
		}
	}
}

void VSQLSyntax::Init( ICodeEditorDocument* inDocument )
{

}

void VSQLSyntax::Load( ICodeEditorDocument* inDocument, VString& ioContent )
{

}

void VSQLSyntax::Save( ICodeEditorDocument* inDocument, VString& ioContent )
{
	
}

void VSQLSyntax::SaveLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, VString& ioContent )
{
	
}

void VSQLSyntax::Close( ICodeEditorDocument* inDocument )
{
}

std::map< int, bool >* VSQLSyntax::GetCommentMap( ICodeEditorDocument* inDocument )
{
	VSQLSyntaxParams* params = dynamic_cast< VSQLSyntaxParams* >( inDocument->GetSyntaxParams( 1 ) );

	/* Sergiy - April 20, 2009 - Crash fix. */
	if (NULL == params) {
		params = new VSQLSyntaxParams();
		inDocument->SetSyntaxParams( params , 1 );
	}
	return params->GetCommentMap();
}

void VSQLSyntax::CleanTokens( std::vector< class ILexerToken * > &inTokens )
{
	while (!inTokens.empty()) {
		inTokens.back()->SelfDelete();
		inTokens.pop_back();
	}
}

bool VSQLSyntax::DetermineMorphemeBoundary( ICodeEditorDocument *inDocument, sLONG inLineIndex, sLONG inOffset, sLONG &outLeftBoundary, sLONG &outLength, bool inAlternateKey )
{
	VString xstr;
	inDocument->GetLine( inLineIndex, xstr );

	// Tokenize that line
	TokenList tokens;
	xbox_assert ( fTokenizeFuncPtr != NULL );
	( *fTokenizeFuncPtr ) ( xstr, tokens, false );
	
	// Now that we have the tokens, look for the one containing the offset
	ILexerToken *morpheme = NULL;
	for (TokenList::iterator iter = tokens.begin(); iter != tokens.end(); ++iter) {
		ILexerToken *token = *iter;
		if (token->GetPosition() <= inOffset && token->GetPosition() + token->GetLength() >= inOffset) {
			// This token has semantic meaning, so we're already set
			morpheme = token;
			break;
		}
	}

	// Sanity check -- this really shouldn't happen, but if it does, we can't do much about it.
	if (!morpheme)	return true;

	// Now that we have the morpheme, we're set
	outLeftBoundary = morpheme->GetPosition();
	outLength = morpheme->GetLength();

	return true;
}

void VSQLSyntax::SetLine( ICodeEditorDocument* inDocument, sLONG inLineNumber, bool inLoading )
{
	// Get the state of the previous line (assuming we're not the first line in the 
	// document) to see whether it ended with an unfinished comment.  If it does, then
	// we want to tell the lexer about it so that it can resume lexing the unfinish
	// comment instead of assuming these are non-comment tokens.
	bool previousLineEndsWithComment = false;
	if (inLineNumber > 0) {
		std::map< int, bool > *lineMap = GetCommentMap( inDocument );
		if (lineMap) {
			previousLineEndsWithComment = (*lineMap)[ inLineNumber - 1 ];
		}
	}

	// Ask the document to give us the text for the given line
	VString xstr;
	inDocument->GetLine(inLineNumber,xstr);

	// Initialize the document line
	inDocument->SetLineStyle( inLineNumber, 0, xstr.GetLength(), eNormal );

	// Tokenize that line
	vector< ILexerToken * > tokens;
	xbox_assert ( fTokenizeFuncPtr != NULL );
	VError err = ( *fTokenizeFuncPtr )( xstr, tokens, previousLineEndsWithComment );

	// Iterate over the tokens, and ask the document to highlight 
	// them as needed.
	for (vector< ILexerToken *>::iterator iter = tokens.begin(); 
						iter != tokens.end(); iter++) {
		ILexerToken *current = *iter;
		sBYTE style = eNormal;
		switch (current->GetType()) {
			case ILexerToken::TT_OPEN_COMMENT:			style = eComment; break;
			case ILexerToken::TT_COMMENT:				style = eComment; break;
			case ILexerToken::TT_NUMBER:				style = eNumber; break;
			case ILexerToken::TT_OPEN_STRING:			style = eString; break;
			case ILexerToken::TT_STRING:				style = eString; break;
			case ILexerToken::TT_SQL_KEYWORD:			style = eKeyword; break;
			case ILexerToken::TT_SQL_COLUMN_FUNCTION:
			case ILexerToken::TT_SQL_SCALAR_FUNCTION:	style = eFunctionKeyword; break;
			case ILexerToken::TT_COMPARISON:			style = eComparison; break;
			case ILexerToken::TT_SQL_DEBUG:				style = eDebug; break;
			case ILexerToken::TT_OPEN_NAME:				style = eName; break;
			case ILexerToken::TT_NAME: {
				// Names are a bit special in that we want to see if we can locate the name within the
				// symbol table.  If we can, we can find out what type of object this is and color it specially.
				IAutoCompleteSymbolTable::IAutoCompleteSymbol *sym = NULL;
				if (fSymTable)	sym = fSymTable->GetSymbolByName( current->GetText() );
				if (sym) {
					if (IAutoCompleteSymbolTable::IAutoCompleteSymbol::eColumn == sym->GetType()) {
						style = eTableName;
					} else if (IAutoCompleteSymbolTable::IAutoCompleteSymbol::eTable == sym->GetType()) {
						style = eColumnName;
					}
				}

				// If the style is still normal, then we've not set the name up properly -- fall back on the name color
				if (eNormal == style)	style = eName;
			} break;
		}
		// Set the style for that token
		inDocument->SetLineStyle( inLineNumber, current->GetPosition(), 
			current->GetPosition() + current->GetLength(), style );
	}

	// Check to see if the last token on the line is an open comment.  This is a
	// special case that we need to track to support multi-line comments.  When
	// tokenizing a line, we will look at the previous line's state to see if it
	// ends in an opened comment.  If it does, then we will alert the lexer that
	// this token stream is assumed to start with a comment so that it can continue
	// to lex appropriately.
	//
	// Note that if we have no tokens on this line, we automatically assume the state 
	// of the line before this one.
	bool endsWithOpenComment = (!tokens.empty()) ? (tokens.back()->GetType() == ILexerToken::TT_OPEN_COMMENT) : (previousLineEndsWithComment);
	std::map< int, bool > *lineMap = GetCommentMap( inDocument );
	bool previousOpenCommentState = (*lineMap)[ inLineNumber ];
	(*lineMap)[ inLineNumber ] = endsWithOpenComment;

	CleanTokens( tokens );

	// There are two cases we really need to care about.  If the line now ends in
	// an open comment (and didn't used to), we want to colorize down the document.
	// Also, if the line no longer ends in an open comment (but used to), we want to
	// colorize down the document.  In either case, we want to keep colorizing subsequent
	// lines until the comment is ended or the end of the document is reached.
	if ((!previousOpenCommentState && endsWithOpenComment ||		// Now ends with open comment, didn't used to
		previousOpenCommentState && !endsWithOpenComment) &&		// Used to end with an open comment, but no longer does
		inLineNumber + 1 < inDocument->GetNbLines()) {
		if (inDocument->GetLineKind( inLineNumber + 1 ) != 38) {	// 38 is the endSql line kind from legacy_language_types.h...
			SetLine( inDocument, inLineNumber + 1, inLoading );
		}
	}
}

bool VSQLSyntax::CheckFolding( ICodeEditorDocument* inDocument, sLONG inLineNumber )
{
	// Get the line of text
	VString xstr;
	inDocument->GetLine( inLineNumber, xstr );

	// Tokenize that line
	vector< ILexerToken * > tokens;
	xbox_assert ( fTokenizeFuncPtr != NULL );
	( *fTokenizeFuncPtr ) ( xstr, tokens, false );

	// If the line starts with an open comment, then we need to check the folding
	bool ret = false;
	for( vector< ILexerToken * >::iterator iter = tokens.begin(); iter != tokens.end(); ++iter) {
		if ((*iter)->GetType() == ILexerToken::TT_OPEN_COMMENT)	{ ret = true; break; }
		else if ((*iter)->GetType() == ILexerToken::TT_WHITESPACE) continue;
		else break;
	}

	CleanTokens( tokens );

	return ret;
}

void VSQLSyntax::ComputeFolding( ICodeEditorDocument *inDocument, sLONG inStartIndex, sLONG inLastIndex )
{
	// Code folding is very difficult given the line-oriented nature of the code editor interface, so
	// I am only going to support a limited amount of folding.  Currently, that's just going to be
	// multiline comments which exist as the only tokens on a line.  This neatly skirts around issues
	// involving tokens which can appear as the start of a statement as well as within the statement, and
	// multiple-statements per line, etc.
	const sLONG kNoBlockStarted = -1;
	sLONG commentBlockStart = kNoBlockStarted;
	for (sLONG i = inStartIndex; i <= inLastIndex; i++) {
		// Tokenize the line of text that we're interested in, paying attention to whether
		// it's part of a comment block or not
		bool previousLineEndsWithComment = false;
		if (i > 0) {
			std::map< int, bool > *lineMap = GetCommentMap( inDocument );
			if (lineMap) {
				previousLineEndsWithComment = (*lineMap)[ i - 1 ];
			}
		}

		// Get the line of text
		VString xstr;
		inDocument->GetLine( i, xstr );

		// Tokenize that line
		vector< ILexerToken * > tokens;
		xbox_assert ( fTokenizeFuncPtr != NULL );
		( *fTokenizeFuncPtr ) ( xstr, tokens, previousLineEndsWithComment );

		// Remove any whitespace, but not comment tokens
		for (vector< ILexerToken * >::reverse_iterator iter = tokens.rbegin(); iter != tokens.rend(); iter++) {
			// Check to see if we have a token we want to remove from the list, and remove it.  
			// Whoever said STL was easy to read should be smacked.  We have to do the (++iter).base()
			// dance because erase expects an iterator, not a reverse iterator.  But if we do forward
			// iteration, we invalidate the iterator every time we do an erase.  This gets around it
			// by doing a reverse iteration, but still calls erase by converting the reverse_iterator
			// into a regular iterator.  Yay!
			if ((*iter)->GetType() == ILexerToken::TT_WHITESPACE) {
				vector< ILexerToken * >::reverse_iterator temp = iter;
				(*iter)->SelfDelete();
				tokens.erase( (++temp).base() );
			}
		}

		// The first time we come across a line that contains only an open comment, we will keep track of
		// the line index since it may be the start of a foldable block.  Any lines after this open comment
		// which are also open comments are simply ignored.  However, when we close the comment, we have a 
		// bit of work to do.  We will look to see if the closed comment is the only token on the line, and if
		// it is, then we will mark the entire block as foldable.
		//
		// Working example:
		// /* This is a multi-line comment
		//		that is perfectly legal
		//		and will fold properly. */
		//
		// Non-working example:
		// SELECT * FROM Movies /* This example
		//		will not fold because the comment
		//		is not the only token on the line. */
		//
		// /* This example will fail for the
		//		same reason. */ LOCK TABLE Movies;

		if (kNoBlockStarted == commentBlockStart && 
			tokens.size() == 1 && 
			tokens.front()->GetType() == ILexerToken::TT_OPEN_COMMENT) {
			// If the only token on the line is an open comment, we've got the start (or the continuation) of a comment
			// block that we care about.
			commentBlockStart = i;
		} else if (kNoBlockStarted != commentBlockStart &&
			tokens.size() == 1 &&
			tokens.front()->GetType() == ILexerToken::TT_COMMENT) {
			// This is the only token on a line, and it's a closed comment.  This means we've closed off the previous 
			// open comment, and this is a foldable block that we care about.
			inDocument->SetFoldable( commentBlockStart, true );
			inDocument->SetNbFoldedLines( commentBlockStart, i - commentBlockStart );
		}

		// If we have found an open comment already, and there's a closed comment on the line, then we know we're done 
		// with the open comment block.  So search the tokens for a comment to clear the flag as needed.
		if (kNoBlockStarted != commentBlockStart) {
			for( vector< ILexerToken * >::iterator iter = tokens.begin(); iter != tokens.end(); ++iter) {
				if ((*iter)->GetType() == ILexerToken::TT_COMMENT) {
					commentBlockStart = kNoBlockStarted;
					break;
				}
			}
		}

		CleanTokens( tokens );
	}
}

void VSQLSyntax::ComputeFolding( ICodeEditorDocument* inDocument )
{
	// Find all of the blocks of SQL code in this document so that we can handle them as individual code blocks.
	sLONG count = inDocument->GetNbLines();
	sLONG startIndex = -1;
	for (sLONG i = 0; i < count; i++) {
		// Check to see if we have a BEGIN SQL block
		if (startIndex == -1 && inDocument->GetLineKind( i ) == 37) {		// This is a beginSQL line kind
			startIndex = i + 1;
		}

		// Check to see if we have an END SQL block
		if (startIndex != -1 && inDocument->GetLineKind( i ) == 38) {		// This is an endSQL line kind
			// Now we have the start and end of the SQL block, so we can process it as a whole
			ComputeFolding( inDocument, startIndex, i - 1 );
			startIndex = -1;
		}
	}
}

bool VSQLSyntax::CheckOutline( ICodeEditorDocument* inInterface, sLONG inLineNumber )
{
	return false;
}

void VSQLSyntax::ComputeOutline( ICodeEditorDocument* inInterface )
{
}


void VSQLSyntax::TokenizeLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, std::vector<XBOX::VString>& ioLines )
{
}

void VSQLSyntax::GetTip( ICodeEditorDocument* inDocument, sLONG inLine, sLONG inPos, VString& outText, Boolean& outRtfText )
{
}

static bool LineHasSemanticMeaning( vector< ILexerToken * > &tokens )
{
	// Loop over all the tokens in the list and see if any of them have semantic meaning
	for (vector< ILexerToken * >::iterator iter = tokens.begin(); iter != tokens.end(); iter++) {
		// Get the token's value -- this loop will end once we find a token with semantic meaning
		if (-1 != (*iter)->GetValue())	return true;
	}
	return false;
}

bool VSQLSyntax::FindStatementStart( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG &outStart )
{
	// Starting at the index we've been passed in, walk up the document looking
	// for the line that starts the statement.  Each statement starts with a mostly 
	// unique keyword, so we will walk back up the document until we reach a line 
	// that starts with a statement token.
	assert( inDocument->GetLineKind( inLineIndex ) == 39 );
	sLONG lastSQLLineParsed = inLineIndex;
	for (sLONG i = inLineIndex; i >= 0; --i) {
		// If the line isn't a SQL line then we're done looking
		if (inDocument->GetLineKind( i ) != 39) {
			outStart = lastSQLLineParsed;
			return true;
		}

		// Tokenize the line of text that we're interested in, paying attention to whether
		// it's part of a comment block or not
		bool previousLineEndsWithComment = false;
		if (i > 0) {
			std::map< int, bool > *lineMap = GetCommentMap( inDocument );
			if (lineMap) {
				previousLineEndsWithComment = (*lineMap)[ i - 1 ];
			}
		}

		// Get the line of text
		VString xstr;
		inDocument->GetLine( i, xstr );

		// Tokenize that line
		vector< ILexerToken * > tokens;
		xbox_assert ( fTokenizeFuncPtr != NULL );
		( *fTokenizeFuncPtr ) ( xstr, tokens, previousLineEndsWithComment );

		// Grab the first non-whitespace token so that we can inspect its value.  Also grab the last non-whitespace
		// token so we can see if it's a semicolon.
		int firstToken = -1, lastToken = -1;
		for (vector< ILexerToken * >::iterator iter = tokens.begin(); iter != tokens.end() && firstToken == -1; iter++) {
			// Get the token's value -- this loop will end once we find a token with semantic meaning
			firstToken = (*iter)->GetValue();
		}

		for (vector< ILexerToken * >::reverse_iterator iter = tokens.rbegin(); iter != tokens.rend() && lastToken == -1; iter++) {
			// Get the token's value -- this loop will end once we find a token with semantic meaning
			lastToken = (*iter)->GetValue();
		}

		// If the last token is a semi-colon, then the last line we parsed might be the statement opener.  However, that
		// is not the case if the semi-colon we've found is a part of the current statement.  If we are parsing the line
		// that the caller gave us, we have to do a bit more work.
		if (lastToken == SEMICOLON && inLineIndex != i) {
			outStart = lastSQLLineParsed;
			CleanTokens( tokens );
			return true;
		}

		// If we have a token, check to see whether it's one that starts a statement or not.  Some tokens can
		// only ever appear on the start of a statement line.  Other tokens can actually appear as part of a legit
		// statement, though not as the start.  For instance, COMMIT can only ever be at the start of a statement, but
		// SELECT can appear as part of a subquery.  So we are also keeping track of the last token on a line so we can
		// see if it's a semicolon.  If it is, then we know that one statement has ended.
		switch (firstToken) {
			// Definitive statement openers
			case DEBUG:
			case CREATE:
			case START:
			case COMMIT:
			case ROLLBACK:
			case ALTER:
			case EXECUTE:
			case LOCK:
			case UNLOCK:
			case SQL_KWORD_USE:
			case SQL_KWORD_DELETE:
			case SQL_KWORD_GRANT:
			case SQL_KWORD_REVOKE:
			case SQL_KWORD_REPLICATE: {
				// This is a keyword which starts a statement, so we're done
				outStart = i;
				CleanTokens( tokens );
				return true;
			} break;
	
			// Contextual statement openers
			case DROP:		// DROP can appear as part of an ALTER TABLE action
			case SELECT:	// SELECT can appear as part of a subquery
			case UPDATE:	// UPDATE can appear as part of a constraint
			case INSERT: {	// INSERT can appear as a function reference
				// We're going to determine whether this is a statement opener in a way that will work
				// regardless of keyword location.  Basically, we're going to walk *up* the document, looking
				// to see if the previous line ends with a semi-colon.  If the line contains only non-semantic
				// information (whitespace, comments, empty, etc) then we will skip it.  But if the line contains
				// semantic information, we can see whether it ends with a semi-colon or not.  If it does, then
				// this keyword starts a new statement.  If it doesn't, then this keyword appears in the middle of
				// a statement and we need to resume normal searching.
				for (sLONG j = i - 1; j >= 0; --j) {
					// If we're not looking at a SQL line, we're done
					if (inDocument->GetLineKind( j ) != 39)	break;

					// Tokenize the line of text that we're interested in, paying attention to whether
					// it's part of a comment block or not
					bool previousLineEndsWithComment = false;
					if (j > 0) {
						std::map< int, bool > *lineMap = GetCommentMap( inDocument );
						if (lineMap) {
							previousLineEndsWithComment = (*lineMap)[ j - 1 ];
						}
					}

					// Get the line of text
					VString xstr;
					inDocument->GetLine( j, xstr );

					// Tokenize that line
					vector< ILexerToken * > otherTokens;
					xbox_assert ( fTokenizeFuncPtr != NULL );
					( *fTokenizeFuncPtr ) ( xstr, otherTokens, previousLineEndsWithComment );

					// If the line has no semantic meaning, then we want to walk up a step to the next line
					if (!LineHasSemanticMeaning( otherTokens )) {
						CleanTokens( otherTokens );
						CleanTokens( tokens );
						continue;
					}

					// Get last semantic token on the line to see if it's a semi-colon
					for (vector< ILexerToken * >::reverse_iterator iter = otherTokens.rbegin(); iter != otherTokens.rend(); iter++) {
						// Get the token's value -- this loop will end once we find a token with semantic meaning
						int	token = (*iter)->GetValue();
						if (-1 != token) {
							if (token == SEMICOLON) {
								outStart = i;
								CleanTokens( tokens );
								CleanTokens( otherTokens );
								return true;
							}
							break;
						}
					}

					// We're done with looking up for the next semantic line because we've found one and it does not end
					// with a semi-colon.  So resume searching as normal
					CleanTokens( otherTokens );
					break;
				}
			} break;
		}

		CleanTokens( tokens );
		lastSQLLineParsed = i;
	}

	return false;
}

bool VSQLSyntax::IsSQLSafeName( VString inName )
{
	// A "safe" SQL name is one that starts with an underscore or a latin character and contains nothing 
	// but latin characters (not spaces), underscores, and digits.

	// Sanity check
	if (!testAssert( inName.GetLength() != 0 ))	return false;

	// Check the first character to make sure it's an underscore or ASCII character
	if (inName[ 0 ] == (UniChar)'_' || 
		(inName[ 0 ] >= (UniChar)'a' && inName[ 0 ] <= (UniChar)'z') ||
		(inName[ 0 ] >= (UniChar)'A' && inName[ 0 ] <= (UniChar)'Z')) {
		// It started with a valid character and we're good to continue processing
	} else {
		// It started with an illegal character, so we're done
		return false;
	}

	// Now we want to loop over the rest of the string to make sure it's a latin
	// character, underscore, or digit
	for (VIndex i = 1; i < inName.GetLength(); i++) {
		if (inName[ i ] == (UniChar)'_' || 
			(inName[ i ] >= (UniChar)'0' && inName[ i ] <= (UniChar)'9') ||
			(inName[ i ] >= (UniChar)'a' && inName[ i ] <= (UniChar)'z') ||
			(inName[ i ] >= (UniChar)'A' && inName[ i ] <= (UniChar)'Z')) {
			// This is a valid character for the name
		} else {
			// This is an illegal character and we're done searching
			return false;
		}
	}

	// We got to the end of the string without finding anything wrong with it, so
	// it is SQL compliant.
	return true;
}

void VSQLSyntax::MakeSQLSafeName( VString &ioName )
{
	// A SQL compliant name is one which is bracketed by [ and ], and any ] characters within the original
	// name are escaped by doubling it up.  For instance, given a name like "Test ] Name" would be converted
	// into "[Test ]] Name]"

	// Loop over the contents of the string looking for any ']' characters -- if we find one, insert a ]
	// to escape it and continue searching.  Note that we want to insert *before* the current position in
	// the string so that we catch the case where the user has doubled up the bracket.  For instance:
	// "Test ]] Name" would become "Test ]]]] Name"
	for (VIndex i = 0; i < ioName.GetLength(); i++) {
		if (ioName[ i ] == (UniChar)']') {
			// Insert is 1-based, but our loop is 0-based!!
			ioName.Insert( (UniChar)']', i + 1 );
			i++;
		}
	}

	// Now bracket the name with []
	ioName = "[" + ioName + "]";
}
	
void VSQLSyntax::GetSuggestions( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inPos, ITipInfoArray *outSuggestions, sLONG& outStartOffset, bool inAll )
{
	// Get the text corresponding to the line we've been asked after.  We will
	// truncate the line at the given line position because this is the location
	// where the completion will happen.  However, we do care about the text past
	// the completion point since it may be used as a filter for the generated list
	// of completions.  For instance, if the user has DELETE ^ FROM Baz, where ^ is the
	// insertion position, we wouldn't want to suggest FROM to the user since that is
	// the text immediately following the cursor.  Similarly, if the user
	VString xstr;
	inDocument->GetLine( inLineIndex, xstr );
	xstr.Truncate( inPos );

	// We need to make sure we're lexing complete statements.  However, statements
	// can span multiple lines.  That means we need to walk back up the document lines 
	// until we get to the start of the statement.  What's more, there can also be multiple 
	// statements on a single line.  If we spot a semi-colon, we will split the line into 
	// multiple parts and only deal with the part containing inPos.  If we don't spot any 
	// semi-colons, then we will have to locate the start of the statement by hand.
	VectorOfVString substrs;
	if (xstr.GetSubStrings( ";", substrs, true )) {
		// We found at least one semi-colon, and we know that anything past inPos no longer
		// exists as part of this sentence, which means the last substring is the statement
		// we want to complete.
		xstr = substrs.back();
	} else {
		sLONG startIndex = -1;
		if (FindStatementStart( inDocument, inLineIndex, startIndex )) {
			// Compose a final statement for us to pass off to the lexer.  We've
			// already modified the last line, so we don't need to include it in our
			// composition.
			VString lastLine = xstr;
			xstr = "";
			for (sLONG i = startIndex; i < inLineIndex; i++) {
				VString temp;
				inDocument->GetLine( i, temp );
				xstr += (temp + " "); // Add some whitespace just to be sure it's there.  It's harmless
			}
			xstr += lastLine;
		}
	}

	// Now we will lex our string into a bunch of tokens.  This allows us to determine
	// what the statement type is (SELECT, DROP, etc).  It also allows us to tell what
	// the immediate preceeding token is.  Then we can take this information, along with
	// the partial text from the final token, and use it to filter the list of possible
	// results.  For instance, if we have a SET keyword, the only possible item that's
	// legal is the DEBUG keyword.
	vector< ILexerToken * > tokens;
	xbox_assert ( fTokenizeFuncPtr != NULL );
	( *fTokenizeFuncPtr ) ( xstr, tokens, false );

	// We need to determine whether the last token is whitespace before we start stripping
	// whitespace tokens from the list.  This is used to determine whether our completion
	// string is empty or not.
	bool bLastTokenIsWhitespace = (tokens.size())?(tokens.back()->GetValue() == -1):(false);
	
	// Now that we have the tokens from the sentence, we're going to remove any tokens
	// which we don't care about.  That includes whitespace, comments, and the likes.
	// We will remove them based on the fact that there's no token value associated with them.
	for (vector< ILexerToken * >::reverse_iterator iter = tokens.rbegin(); iter != tokens.rend(); iter++) {
		// Check to see if we have a token we want to remove from the list, and remove it.  
		// Whoever said STL was easy to read should be smacked.  We have to do the (++iter).base()
		// dance because erase expects an iterator, not a reverse iterator.  But if we do forward
		// iteration, we invalidate the iterator every time we do an erase.  This gets around it
		// by doing a reverse iteration, but still calls erase by converting the reverse_iterator
		// into a regular iterator.  Yay!
		if (-1 == (*iter)->GetValue()) {
			vector< ILexerToken * >::reverse_iterator temp = iter;
			(*iter)->SelfDelete();
			tokens.erase( (++temp).base() );
		}
	}

	// Now that we've got the list of tokens, we can pull out the tokens of interest
	// to us.  There are potentially three pieces of the sentence that are interesting:
	// the first token tells us what kind of a statement we should expect.  The last
	// token is the partial string we're trying to complete.  The token preceeding that
	// allows us to filter our final list based on what is expected to follow it.
	//
	// We need to determine what the completion token is, if any.  In most cases, the
	// user is typing something which needs to be completed, and so the last toke is
	// the item that needs to be completed (whether the tokenizer considers it to be a name
	// or not is generally something that can be disregarded).  However, under certain
	// circumstances, the user may not have entered any text for the final token yet.  
	// Consider the case where the user types EXECUTE IMMEDIATE : and hits tab to see a
	// list of the 4D methods they can complete.  In this case, the last token is the 
	// COLON, but if we were to pop that off the list and use it as our completion text,
	// we would never get any matches (since method names don't start with colons).  To
	// make matters worse, it is possible that the user may not have any identifying mark
	// like the COLON -- they may just want to complete after a keyword.  Consider the case
	// where the user types ALTER TABLE and hits tab to see the list of tables.  In this case,
	// the last token is a keyword.  What appears to be the deciding factor between whether the
	// last token is to be completed (and hence, popped off the token stack) is whether there are
	// any spaces or punctuation at the end of the token list.  So, if the last token is a 
	// whitespace, then we assume the completion is blank and no tokens are to be popped.  If the 
	// last token is not whitespace or punctuation or a number, then we assume it's text to be completed.
	if (bLastTokenIsWhitespace || tokens.empty() || 
		tokens.back()->GetType() == ILexerToken::TT_COMPARISON ||
		tokens.back()->GetType() == ILexerToken::TT_PUNCTUATION || 
		tokens.back()->GetType() == ILexerToken::TT_NUMBER ) {
		// There is no text to complete, so we do not want to modify the token list
		xstr = "";
	} else {
		// The last token is text that we want to try to complete, so use it and remove the
		// last token from our list.
		xstr = tokens.back()->GetText();
		tokens.back()->SelfDelete();
		tokens.pop_back();
	}

	bool bShowSemiHiddenContent = (xstr.GetLength() != 0);

	// Add on the wildcard character so that we can do a LIKE search
	xstr.AppendChar( VTask::GetWildChar() );

	std::vector< Completion > completions;
	SuggestionList suggestions;
	if (ParseTokenList( tokens, suggestions ) ) {
		map< SuggestionInfo::Type, IAutoCompleteSymbolTable::IAutoCompleteSymbol::SymbolType > typeMap;
		map< SuggestionInfo::Type, bool > seenMap;

		// We only want to add entries to the list based on their type.  If a table is expected,
		// then we should push tables onto the list.  If a schema is expected, then push schemas
		// onto the list, and so on.  Since suggestions and symbols are not required to have types
		// values which match (for instance, symbols may pertain to other languages than SQL), we need
		// to map from one to another.  Furthermore, we only want to add symbols to the list one time.  So
		// we are going to make a few STL maps to clean this up a bit.
		typeMap[ SuggestionInfo::eTable ] = IAutoCompleteSymbolTable::IAutoCompleteSymbol::eTable;
		typeMap[ SuggestionInfo::eView ] = IAutoCompleteSymbolTable::IAutoCompleteSymbol::eView;
		typeMap[ SuggestionInfo::eColumn ] = IAutoCompleteSymbolTable::IAutoCompleteSymbol::eColumn;
		typeMap[ SuggestionInfo::eSchema ] = IAutoCompleteSymbolTable::IAutoCompleteSymbol::eSchema;
		typeMap[ SuggestionInfo::e4DMethod ] = IAutoCompleteSymbolTable::IAutoCompleteSymbol::e4DMethod;
		typeMap[ SuggestionInfo::eIndex ] = IAutoCompleteSymbolTable::IAutoCompleteSymbol::eIndex;
		typeMap[ SuggestionInfo::eGroup ] = IAutoCompleteSymbolTable::IAutoCompleteSymbol::eGroup;
		typeMap[ SuggestionInfo::e4DLocalVariable ] = IAutoCompleteSymbolTable::IAutoCompleteSymbol::e4DLocalVariable;
		typeMap[ SuggestionInfo::e4DInterProcessVariable ] = IAutoCompleteSymbolTable::IAutoCompleteSymbol::e4DInterProcessVariable;
		typeMap[ SuggestionInfo::e4DProcessVariable ] = IAutoCompleteSymbolTable::IAutoCompleteSymbol::e4DProcessVariable;

		for (SuggestionList::iterator iter = suggestions.begin(); iter != suggestions.end(); ++iter) {
			SuggestionInfo *suggestion = *iter;

			switch (suggestion->fType) {
				case SuggestionInfo::eScalarMethod: {
					completions.push_back( Completion( suggestion->fText, eFunctionKeyword ) );
				} break;
				case SuggestionInfo::eName: {
					// If the name is already SQL safe, then add it to the list.  But also add an encoded name
					// to the list as well.
					if (IsSQLSafeName( suggestion->fText )) {
						completions.push_back( Completion( suggestion->fText, eName ) );
					} else {
						MakeSQLSafeName( suggestion->fText );
						completions.push_back( Completion( suggestion->fText, eName ) );
					}
				} break;
				case SuggestionInfo::eKeyword: {
					completions.push_back( Completion( suggestion->fText, eKeyword ) );
				} break;
				default: {
					if (!seenMap[ suggestion->fType ]) {
						seenMap[ suggestion->fType ] = true;
						for (size_t i = 0; i < fSymTable->GetSymbolCount(); i++) {
							IAutoCompleteSymbolTable::IAutoCompleteSymbol *sym = fSymTable->GetSymbol( i );
							if (sym->GetType() == typeMap[ suggestion->fType ]) {
								VString name;
								sym->GetCompletionText( name );

								if (bShowSemiHiddenContent || !sym->IsSemiHidden()) {
									if (IsSQLSafeName( name )) {
										// If the name is already SQL safe, then add it to the list.
										completions.push_back( Completion( name, eName ) );
									} else {
										// If the name isn't SQL safe, then make it so and add it to the list
										MakeSQLSafeName( name );
										completions.push_back( Completion( name, eName ) );
									}
								}
							}
						}
					}				
				} break;
			}
		}
	} else {
		// This is just a fallback in the worst case scenario.  It has no intelligence behind 
		// it aside from doing a simple text search and suggests anything, anywhere.  The user
		// should only hit this in the case where the statement completer fails (usually due to
		// the user entering in something syntactically incorrect).
		//
		// We're going to start off with a cheap and dirty list of keywords that the
		// user can make use of.  Unfortunately, this list comes to us as a list of 
		// VString *, which we need to convert down to a list of VString
		for (vector< VString * >::iterator iter = fKeywords.begin(); iter != fKeywords.end(); ++iter) {
			completions.push_back( Completion( **iter, eKeyword ) );
		}

		// Hack in a few extra things we want to auto-complete that aren't actually
		// SQL keywords.
		completions.push_back( Completion( VString( "SQL" ), eKeyword ) );

		// If we have a symbol table, then push all of the symbols into the list
		if (fSymTable) {
			for (size_t i = 0; i < fSymTable->GetSymbolCount(); i++) {
				VString name;
				fSymTable->GetSymbol( i )->GetCompletionText( name );

				if (IsSQLSafeName( name )) {
					// If the name is already SQL safe, then add it to the list.
					completions.push_back( Completion( name, eName ) );
				} else {
					// If the name isn't SQL safe, then make it so and add it to the list
					MakeSQLSafeName( name );
					completions.push_back( Completion( name, eName ) );
				}
			}
		}
	}

	// Loop over all of the items in the keyword list and see if we can find any matches
	VCollator *collator = VIntlMgr::GetDefaultMgr()->GetCollator();
	for (vector< Completion >::iterator iter = completions.begin(); 
						iter != completions.end(); iter++) {
		Completion comp = *iter;
		if (collator->EqualString_Like( comp.fText.GetCPointer(), comp.fText.GetLength(), xstr.GetCPointer(), xstr.GetLength(), false))
		{
			// As a sanity check, let's make sure that none of the suggestions we're giving the
			// caller are empty.
			assert( comp.fText.GetLength() != 0 );

			// If this is a duplicate entry, then we do not want to add it to the list.  Duplicates
			// are normal, but fairly rare.  For instance, several tables may have a column named ID
			// and the completer is suggesting all columns.  Duplicates can be safely removed, because
			// at this point, we're just dealing with textual suggestions instead of semantic morphemes.
			if (!outSuggestions->Contains( comp.fText )) {
				outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, comp.fText, comp.fStyle ) );
				outStartOffset = inPos - xstr.GetLength() + 1;
			}
		}
	}

	// Now we need to be sure to remove any temporaries which were created during this parsing
	if (fSymTable) fSymTable->RemoveTemporaries();

	CleanTokens( tokens );
}

sLONG VSQLSyntax::GetIndentWidth()
{
	return 0;
}
bool VSQLSyntax::UseTab() 
{
	return true;
}

void VSQLSyntax::InsertChar( ICodeEditorDocument* inDocument, UniChar inUnichar, sLONG inLineIndex, sLONG inPosition, ITipInfoArray *outSuggestions, sLONG& outStartOffset )
{
}

void VSQLSyntax::UpdateBreakPointsLineNumbers( ICodeEditorDocument* inDocument, const std::vector<sWORD>& inBreakIDs, const std::vector<sLONG>& inLineNumbers )
{
}

void VSQLSyntax::AddBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber, sWORD& outID, bool& outDisabled )
{
	outID = 0;
}

bool VSQLSyntax::EditBreakPoint( ICodeEditorDocument* inDocument, sWORD inBreakID, bool& outDisabled )
{
	return false;
}

void VSQLSyntax::RemoveBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber )
{
}

bool VSQLSyntax::GetBreakPoints( ICodeEditorDocument* inDocument, std::vector<sLONG>& outLineNumbers, std::vector<sWORD>& outIDs, std::vector<bool>& outDisabled )
{
	return false;
}


void VSQLSyntax::UpdateCompileErrors( ICodeEditorDocument *inDocument, sLONG inStartIndex, sLONG inLastIndex )
{
	// Now that we know the entire block of SQL code to complete, start walking over it a single
	// line at a time, updating the compile errors as we go.
	sLONG anchor = inStartIndex;
	bool bInErrorMode = false;
	for (sLONG i = inStartIndex; i <= inLastIndex; i++) {
		// Grab the data from our anchor to the current line we're trying to process
		VString code;
		for (sLONG j = anchor; j <= i; j++) {
			VString temp;
			inDocument->GetLine( j, temp );
			code += (temp + " ");
		}

		// Now that we have the code, we want to tokenize it and parse it like always
		vector< ILexerToken * > tokens;
		xbox_assert ( fTokenizeFuncPtr != NULL );
		( *fTokenizeFuncPtr ) ( code, tokens, false );

		for (vector< ILexerToken * >::reverse_iterator iter = tokens.rbegin(); iter != tokens.rend(); iter++) {
			if (-1 == (*iter)->GetValue()) {
				vector< ILexerToken * >::reverse_iterator temp = iter;
				(*iter)->SelfDelete();
				tokens.erase( (++temp).base() );
			}
		}

		// Now that we finally have just a list of the semantic tokens that are interesting
		// to us, let's parse the code and see if we can spot any errors with it.
		SuggestionList suggestions;
		if (!bInErrorMode && !tokens.empty() && !ParseTokenList( tokens, suggestions )) {
			// Since we are not doing incredibly complex error checking, right now all errors are
			// just being reported as syntax errors.  This makes me feel dirty, but it's a reasonable
			// solution since this is a one-off parser instead of the actual language parser.
			VString aLocalizedString;
			extern VLanguageSyntaxComponent* gLanguageSyntax;
			gLanguageSyntax->GetLocalizationMgr()->LocalizeStringWithKey( CVSTR( "SyntaxError" ), aLocalizedString );

			// This line had an error, so we should report it!
			inDocument->SetLineParsingError( i, aLocalizedString );

			// Once we have gotten a syntax error on a statement's line, we do not want to report any
			// more syntax errors for that same statement (since every subsequent line in the statement
			// will report errors as well.
			bInErrorMode = true;
		}

		// If the last token on the line is actually a semi-colon, then it's time to set 
		// a new anchor point and clear our error mode.
		if (!tokens.empty() && tokens.back()->GetValue() == SEMICOLON) {
			anchor = i + 1;
			bInErrorMode = false;
		}

		CleanTokens( tokens );
	}
}

void VSQLSyntax::UpdateCompilerErrors( ICodeEditorDocument* inDocument )
{

}

void VSQLSyntax::PerformIdleProcessing( ICodeEditorDocument *inDocument )
{
	// We need to walk over the entire contents of the document to tell it where there are
	// compile errors with the SQL statements.  However, the document does not contain just
	// SQL statements -- it can also contain 4D statements, amongst other things.  So the first
	// order of business is to narrow the document down to just the lines which contain SQL
	// code.  The document interface does not require you to give a location for the error, only
	// a line.  So we will walk over every line of SQL code, and try to parse them out to determine
	// whether that line has an error or not.  However, since SQL statements can span multiple
	// lines, we have to be a bit sneaky in our approach.  Starting with the first line of SQL 
	// statements, we will determine whether the parse had completed the statement or not (by seeing
	// if the last semantic token on the line is a semi-colon or not).  If the statement has not been
	// completed, then we will assume this line is an anchor point.  When we parse the next line down,
	// we will use this anchor point to restart the parsing.  By doing this, we can make some handy 
	// assumptions.  When parsing a line, we can assume it's clean based on the previous line's state.
	// If the previous line is clean and we parse an error -- we know that error must exist on this line.
	// If the previous line has an error, then we don't have to parse this line at all -- we know it is
	// in error because it's part of the same statement.  Once we parse a semi-colon, then we know the
	// statement has ended, and we can reset our anchor point.  The edge case here is when there are
	// multiple statements on a single line.  For instance:
	// 
	//	DROP TABLE
	//		SomeTable; DROP
	//		SCHEMA Bar;
	//
	// In this case, we have multiple statements spanning multiple lines.  Line one contains solely a 
	// portion of the DROP TABLE statement, while line two contains the completion of the DROP TABLE
	// as well as the start of the DROP SCHEMA statement.  The third line contains the completion of the
	// DROP SCHEMA statement.  Consider if there was an error on the second line.  That error could belong
	// to the DROP TABLE statement or the DROP SCHEMA statement, depending on its location.  I am going to
	// assume that this particular edge case is not important -- the error will be reported on the second line
	// which is correct.  If it cascades to further lines, that's not the end of the world even if it's not 
	// 100% correct either.

	// The first step is to locate a SQL block in the document.  Note that documents may contain multiple
	// SQL blocks and we need to support all of them.  So we look for the BEGIN SQL, take note of its line
	// index, and then continue scanning for the END SQL.  Once we have it, we can parse that complete block
	// as a whole before continuing to look for another BEGIN SQL block (which may or may not exist).  Note that
	// we do not support nested blocks.
	sLONG count = inDocument->GetNbLines();
	sLONG startIndex = -1;
	for (sLONG i = 0; i < count; i++) {
		// Check to see if we have a BEGIN SQL block
		if (startIndex == -1 && inDocument->GetLineKind( i ) == 37) {		// This is a beginSQL line kind
			startIndex = i + 1;
		}

		// Check to see if we have an END SQL block
		if (startIndex != -1 && inDocument->GetLineKind( i ) == 38) {		// This is an endSQL line kind
			// Before we start to update compile errors, we have to tell the document
			// that there are no more compile errors on any line.  We cannot rely on SetLine
			// to do this work for us because it is possible the user modified some code on
			// line X which affected the ability to compile line X + 10.  SetLine would clear
			// the non-existent error on line X, but not clear the one on X + 10 -- so we will
			// clear *all* of the lines manually to ensure the document get a fresh start.
			for (VIndex j = startIndex; j <= i; ++j) {
				inDocument->SetLineParsingError( j, CVSTR( "" ) );
			}


			// Now we have the start and end of the SQL block, so we can process it as a whole
			UpdateCompileErrors( inDocument, startIndex, i - 1 );
			startIndex = -1;
		}
	}
}

void VSQLSyntax::GotoNextMethodError( ICodeEditorDocument* inDocument, bool inNext )
{
}

void VSQLSyntax::SwapAssignments( ICodeEditorDocument* inDocument, VString& ioString )
{
}

bool VSQLSyntax::IsComment( ICodeEditorDocument* inDocument, const VString& inString )
{
	// We will hand the string off to the lexer, and if a single token comes back that 
	// says "this is a comment", then we're set.  If multiple tokens come back that are
	// non-white space, we know it's not a comment.
	vector< ILexerToken * > tokens;
	xbox_assert ( fTokenizeFuncPtr != NULL );
	( *fTokenizeFuncPtr ) ( (VString&)inString, tokens, false );

	// Loop over the tokens and see what we've got
	bool isComment = false;

	for (vector< ILexerToken *>::iterator iter = tokens.begin(); 
						iter != tokens.end(); iter++) {
		ILexerToken *current = *iter;

		// If we have a comment token, then we still need to keep looking to make sure
		// there is not some other token on the same line.
		if (current->GetType() == ILexerToken::TT_COMMENT) {
			isComment = true;
		} else if (current->GetType() != ILexerToken::TT_WHITESPACE) {
			// Whitespace can be safely ignored as not important.  But if we have something 
			// that's not whitespace, nor is it a comment, then we know this line cannot
			// contain only a comment and so we can bail out.
			isComment = false;
			break;
		}
	}

	CleanTokens( tokens );

	return isComment;
}

void VSQLSyntax::SwapComment( ICodeEditorDocument* inDocument, VString& ioString, bool inComment )
{
	// We've been given a string that we either need to comment, or uncomment, depending
	// on the state of the "inComment" parameter.  We will do a simple validation if the caller
	// is asking us to uncomment something which isn't commented.
	if (inComment) {
		// This is the easy case -- we just need to take the string and wrap it with the appropriate
		// comment characters. 
		ioString = VString( "/*" ) + ioString + VString( "*/" );
	} else {
		// Make sure that what we're dealing with really is commented before we start making 
		// assumptions about the format of it.
		if (!IsComment( inDocument, ioString ))	return;

		// Now that we know it's commented, we need to strip off the leading and trailing 
		// comment markers.  We don't know whether there's whitespace in front or behind those
		// markers, so we cannot simply chop based on character positions.  So we will loop from 
		// the beginning of the string until we find the first non-whitespace which we know to be
		// the open comment marker, and remove those two characters.  We will then start from the
		// end of the string and work backwards to do the same for the close comment marker.
		
		// Handle the open comment
		for (VIndex i = 0; i < ioString.GetLength(); ++i) {
			// We know the first character we will happen upon will be the "/" since we've already
			// validated that this is a comment.
			if (ioString[ i ] == (UniChar)'/') {
				// Remove this character and the one that follows it.  Except that i need to be
				// base one for this call
				ioString.Remove( i + 1, 2 );
				// We're done with the opening comment indicator
				break;
			}
		}

		// Handle the close comment
		for (VIndex i = ioString.GetLength() - 1; i >= 0; --i) {
			// We know the first character we will happen upon will also be the "/"
			if (ioString[ i ] == (UniChar)'/') {
				// Remove this character and the one preceeding it.  Except that i is
				// actually base 1 in this case.
				ioString.Remove( i, 2 );
				// We're done with the closing comment indicator
				break;
			}
		}
	}
}


void VSQLSyntax::CallHelp( ICodeEditorDocument* inDocument )
{
}


bool VSQLSyntax::IsMatchingCharacter( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, UniChar inChar )
{
	switch( inChar )
	{
		case '(':
		case ')':
		case '{':
		case '}':
		case '[':
		case ']':
			return true;
		default:
			return false;
	}
}


bool VSQLSyntax::ShouldValidateTipWindow( UniChar inChar )
{
	return inChar == '(' || inChar == ':' || inChar == '=' || inChar == ';' || 
		   inChar == '<' || inChar == '>' || inChar == '+' || inChar == '-' || 
		   inChar == '{' || inChar == '/' || inChar == '#' || inChar == '[';
}


bool VSQLSyntax::DoubleClick( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, bool inAlternateKey )
{
	sLONG start, length;
	DetermineMorphemeBoundary( inDocument, inLineIndex, inOffset, start, length, inAlternateKey );

	sLONG line = inDocument->GetVisibleLine( inLineIndex );
	inDocument->Select( start, start + length, line, line );

	return true;
}

void VSQLSyntax::GetTokensForTesting( VString& inSourceCode, std::vector<VString>& outTokens )
{

}