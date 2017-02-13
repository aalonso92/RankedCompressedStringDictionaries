/* RankedStringDictionary.h
 * Copyright (C) 2016, Álvaro Alonso
 * all rights reserved.
 *
 * Abstract class for implementing Compressed  Ranked String Dictionaries.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 * Contacting the authors:
 *   Álvaro Alonso:  	alvaro.alonso.isla@alumnos.uva.es
 */
 
#ifndef _RANKEDSTRINGDICTIONARY_H
#define _RANKEDSTRINGDICTIONARY_H

using namespace std;

#include <libcdsBasics.h>
using namespace cds_utils;


#include "iterators/IteratorRankedDictID.h"
#include "iterators/IteratorRankedDictString.h"
#include "utils/Utils.h"

struct dictItem
{
    uchar* itemText;
    uint itemId;
};



class RankedStringDictionary
{
	public:
		/** Retrieves the ID corresponding to the given string.
		    @param str: the string to be located.
		    @param strLen: the string length.
		    @returns the ID (or NORESULT if it is not in the dictionary).
		*/
		virtual uint locate(uchar *str, uint strLen)=0;

		/** Obtains the string associated with the given ID.
		    @param id: the ID to be extracted.
		    @param strLen: pointer to the extracted string length.
		    @returns the requested string (or NULL if it is not in the
		      dictionary).
		*/
		virtual uchar* extract(size_t id, uint *strLen)=0;

		/** Locates all IDs of those elements prefixed by the given
		    string.
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
		    @returns an iterator for direct scanning of all the IDs.
		*/
		virtual IteratorRankedDictID* locatePrefix(uchar *str, uint strLen)=0;

		/** Locates all IDs of those elements containing the given
		    substring.
		    @param str: the substring to be searched.
		    @param strLen: the substring length.
		    @returns an iterator for direct scanning of all the IDs.
		*/
		virtual IteratorRankedDictID* locateSubstr(uchar *str, uint strLen)=0;

		/** Retrieves the ID with rank k according to its alphabetical order.
		    @param rank: the alphabetical ranking.
		    @returns the ID.
		*/
		virtual uint locateRank(uint rank)=0;

		/** Extracts all elements prefixed by the given string.
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
		    @returns an iterator for direct scanning of all the strings.
		*/
		virtual IteratorRankedDictString* extractPrefix(uchar *str, uint strLen)=0;

		/** Extracts all elements containing by the given substring.
		    @param str: the substring to be searched.
		    @param strLen: the substring length.
		    @returns an iterator for direct scanning of all the strings.
		*/
		virtual IteratorRankedDictString* extractSubstr(uchar *str, uint strLen)=0;

		/** Obtains the string  with rank k according to its
		    alphabetical order.
		    @param id: the ID to be extracted.
		    @param strLen: pointer to the extracted string length.
		    @returns the requested string (or NULL if it is not in the
		      dictionary).
		*/
		virtual uchar* extractRank(uint rank, uint *strLen)=0;

		/** Extracts all strings in the dictionary sorted in
		    alphabetical order.
		    @returns an iterator for direct scanning of all the strings.
		*/
		virtual IteratorRankedDictString* extractTable()=0;

		/** Computes the size of the structure in bytes.
		    @returns the dictionary size in bytes.
		*/
		virtual size_t getSize()=0;

		/** Retrieves the length of the largest string in the
		    dictionary.
		    @returns the length.
		*/
		uint maxLength();

		/** Retrieves the number of elements in the dictionary.
		    @returns the number of elements.
		*/
		size_t numElements();

		/** Stores the dictionary into an ofstream.
		    @param out: the oftstream.
		*/
		virtual void save(ofstream &out)=0;

		/** Loads a dictionary from an ifstream.
		    @param in: the ifstream.
		    @returns the loaded dictionary.
		*/
		static RankedStringDictionary *load(ifstream &in);

		
		/** Locates the first k IDs of those elements prefixed by the given
		    string.
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
			@param k: the maximun number of elements to return
		    @returns an iterator for direct scanning of all the IDs.
		*/
		virtual IteratorRankedDictID* locateRankedPrefix(uchar *str, uint strLen, uint k)=0;
		
		/** Extracts the first (ranked) k strings of those elements prefixed by the given
		    string.
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
			@param k: the maximun number of elements to return
		    @returns an iterator for direct scanning of all the strings.
		*/
		virtual IteratorRankedDictString* extractRankedPrefix(uchar *str, uint strLen, uint k)=0;
		
		/** Extracts the first (ranked) k strings of those elements in the closed interval given
		    string.
			@param beginning: The first element of the interval
			@param len: The length of the interval
			@param k: the maximun number of elements to return
		    @returns an iterator for direct scanning of all the strings.
		*/
		virtual IteratorRankedDictString* extractRankedInterval(uint beginning, uint len, uint k)=0;


		
		/** Generic destructor. */
		virtual ~RankedStringDictionary() {};


	protected:
		uint32_t type;      //! Dictionary type.
		uint64_t elements;  //! Number of strings in the dictionary.
		uint32_t maxlength; //! Length of the largest string in the dictionary.
		
		
		bool static sorting(dictItem item1, dictItem item2)
        {
            	if (strcmp((char*)item1.itemText, (char*)item2.itemText) < 0)
                	return true;
            	return false;
        }
		
		
};

#include "RankedStringDictionarySimple.h"
#include "RankedStringDictionaryRPDAC.h"

#endif

