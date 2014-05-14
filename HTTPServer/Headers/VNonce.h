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
#ifndef __NONCE_INCLUDED__
#define __NONCE_INCLUDED__


extern const sLONG8 CONST_NONCE_TIMEOUT;


class VNonce : public XBOX::VObject, public XBOX::IRefCountable
{
public:
									VNonce (const XBOX::VString& inNonceValue, const sLONG8 inNonceCreationTime);
	virtual							~VNonce();

	bool							IsStillValid();

	const XBOX::VString&			GetNonceValue() const { return fNonce; }
	sLONG8							GetNonceCreationTime() const { return fNonceCreationTime; }

	void							SetNonceValue (const XBOX::VString& inNonceValue);
	void							SetNonceCreationTime (const sLONG8 inCreationTime) { fNonceCreationTime = inCreationTime; }

	typedef std::list<XBOX::VRefPtr<VNonce> >	VNoncePile;

	static void						GetNewNonceValue (XBOX::VString& outNonceValue);
	static bool						ValidNonceAndCleanPile (const XBOX::VString& inNonceValue);
	static bool						ParseNonceValue (const XBOX::VString& inNonceValue, XBOX::VString& outNonceTime, XBOX::VString& outNonceRandomValue);

private:
	XBOX::VString					fNonce;
	sLONG8							fNonceCreationTime;
	static VNoncePile				fNoncePile;
	static XBOX::VCriticalSection	fNoncePileCriticalSection;
};


#endif	// __NONCE_INCLUDED__
