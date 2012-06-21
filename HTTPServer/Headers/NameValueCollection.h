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
#ifndef __NAME_VALUE_COLLECTION__INCLUDED__
#define __NAME_VALUE_COLLECTION__INCLUDED__


#include <map>


class NameValueCollection
{
public:
								NameValueCollection();
								NameValueCollection (const NameValueCollection& inNameValueCollection);
	virtual						~NameValueCollection();

	struct ILT
	{
		bool operator() (const XBOX::VString& inString1, const XBOX::VString& inString2) const
		{
			return (inString1.CompareToString (inString2) == XBOX::CR_SMALLER);
		}
	};

	typedef std::multimap<XBOX::VString, XBOX::VString, ILT> NameValueMap;
	typedef NameValueMap::iterator Iterator;
	typedef NameValueMap::const_iterator ConstIterator;

	NameValueCollection&		operator = (const NameValueCollection& inNameValueCollection);
	void						swap (NameValueCollection& ioNameValueCollection);
	const XBOX::VString&		operator [] (const XBOX::VString& inName) const;
	void						set (const XBOX::VString& inName, const XBOX::VString& inValue);	
	void						add (const XBOX::VString& inName, const XBOX::VString& inValue);
	const XBOX::VString&		get (const XBOX::VString& inName) const;
	const XBOX::VString&		get (const XBOX::VString& inName, const XBOX::VString& defaultValue) const;
	bool						has (const XBOX::VString& inName) const;

	ConstIterator				find (const XBOX::VString& inName) const;

	ConstIterator				begin() const;
	Iterator					begin();
	ConstIterator				end() const;
	Iterator					end();
	bool						empty() const;
	int							size() const;
	void						erase (const XBOX::VString& inName);
	void						erase (Iterator& inIter);
	void						clear();

private:
	NameValueMap				fMap;
};


inline
void swap (NameValueCollection& ioNVC1, NameValueCollection& ioNVC2)
{
	ioNVC1.swap (ioNVC2);
}


#endif	// __NAME_VALUE_COLLECTION__INCLUDED__
