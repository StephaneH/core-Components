/*
* This file is part of Wakanda software, licensed by 4D under
*  (i)the GNU General Public License version 3 (GNU GPL v3), or
*  (ii)the Affero General Public License version 3 (AGPL v3) or
*  (iii)a commercial license.
* This file remains the exclusive property of 4D and / or its licensors
* and is protected by national and international legislations.
* In any event, Licensee's compliance with the terms and conditions
* of the applicable license constitutes a prerequisite to any use of this file.
* Except as otherwise expressly stated in the applicable license,
*such license does not include any other license or rights on this file,
*4D's and/or its licensors' trademarks and / or other proprietary rights.
* Consequently, no title, copyright or other proprietary rights
* other than those specified in the applicable license is granted.
*/


#ifndef __OBJATTRIBUTE__
#define __OBJATTRIBUTE__




class ObjectPath
{
	public:
		ObjectPath(const VString& inPath);
		ObjectPath();

		void FromVector(const VectorOfVString& from);
		void FromString(const VString& inPath);
		void GotoFirst();
		const VString* NextPart();

		bool IsEmpty() const
		{
			return fParts.empty();
		}

		void Clear()
		{
			fParts.clear();
			GotoFirst();
		}

		bool SamePath(const ObjectPath& other) const;

		void ToString(VString& outStr) const;

	protected:
		VectorOfVString fParts;
		VectorOfVString::iterator fCurrent;


};


class ObjectNode
{
	public:

		ObjectNode(const VJSONValue& inVal, const VString& inPath)
		{
			fVal = inVal;
			fPath = inPath;
		}

		inline VString* GetPath()
		{
			return &fPath;
		}

		inline VJSONValue& GetValue()
		{
			return fVal;
		}


	protected:
		VJSONValue fVal;
		VString fPath;
};


class ReducedObjectNode
{
	public:

		ReducedObjectNode(const VJSONValue& inVal, sLONG pathID)
		{
			fVal = inVal;
			fPathID = pathID;
		}

		inline sLONG GetPathID()
		{
			return fPathID;
		}

		inline VJSONValue& GetValue()
		{
			return fVal;
		}

	protected:
		VJSONValue fVal;
		sLONG fPathID;
};

typedef vector<ObjectNode> ObjectNodeCollection;
typedef vector<ReducedObjectNode> ReducedObjectNodeCollection;

class ObjectParser
{
	public:
		ObjectParser(VJSONValue& inVal, ObjectPath& path, VJSONValue& outVal);
		ObjectParser(const VJSONValue& inVal, ObjectNodeCollection& outNodes);

		void ParseBranches(const VJSONValue& inVal, ObjectNodeCollection& outNodes, const VString& currentPath);

};


// --------------------------------------------------


#endif
