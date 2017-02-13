/* StringDictionary.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Abstract class for implementing Compressed String Dictionaries.
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
 *   Francisco Claude:  	fclaude@recoded.cl
 *   Rodrigo Canovas:  		rcanovas@student.unimelb.edu.au
 *   Miguel A. Martinez-Prieto:	migumar2@infor.uva.es
 */

#ifndef _STRINGDICTIONARY_H
#define _STRINGDICTIONARY_H

using namespace std;

#include <libcdsBasics.h>
using namespace cds_utils;

#include "iterators/IteratorDictID.h"
#include "iterators/IteratorDictString.h"
#include "utils/Utils.h"


class StringDictionary
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
		virtual IteratorDictID* locatePrefix(uchar *str, uint strLen)=0;

		/** Locates all IDs of those elements containing the given
		    substring.
		    @param str: the substring to be searched.
		    @param strLen: the substring length.
		    @returns an iterator for direct scanning of all the IDs.
		*/
		virtual IteratorDictID* locateSubstr(uchar *str, uint strLen)=0;

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
		virtual IteratorDictString* extractPrefix(uchar *str, uint strLen)=0;

		/** Extracts all elements containing by the given substring.
		    @param str: the substring to be searched.
		    @param strLen: the substring length.
		    @returns an iterator for direct scanning of all the strings.
		*/
		virtual IteratorDictString* extractSubstr(uchar *str, uint strLen)=0;

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
		virtual IteratorDictString* extractTable()=0;

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
		static StringDictionary *load(ifstream &in);

		/** Generic destructor. */
		virtual ~StringDictionary() {};


	protected:
		uint32_t type;      //! Dictionary type.
		uint64_t elements;  //! Number of strings in the dictionary.
		uint32_t maxlength; //! Length of the largest string in the dictionary.
};

#include "StringDictionaryHASHHF.h"
#include "StringDictionaryHASHUFFDAC.h"
#include "StringDictionaryHASHRPF.h"
#include "StringDictionaryHASHRPDAC.h"

#include "StringDictionaryPFC.h"
#include "StringDictionaryRPFC.h"
#include "StringDictionaryHTFC.h"
#include "StringDictionaryHHTFC.h"
#include "StringDictionaryRPHTFC.h"

#include "StringDictionaryRPDAC.h"
#include "StringDictionaryFMINDEX.h"
#include "StringDictionaryXBW.h"

#endif

