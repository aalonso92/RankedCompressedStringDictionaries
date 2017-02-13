/* RankedStringDictionarySimple.h
 * Copyright (C) 2016, Álvaro Alonso
 * all rights reserved.
 *
 * This class implements a Compressed Ranked String Dictionary based on another dictionary and a permutation.
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



#ifndef _RANKEDSTRINGDICTIONARYBASIC_H
#define _RANKEDSTRINGDICTIONARYBASIC_H

#include <iostream>
#include "RankedStringDictionary.h"
#include "StringDictionary.h"
#include <PermutationBuilderMRRR.h>
#include <BitSequenceBuilder.h>
#include <libcdsBasics.h>
#include <vector>
#include "iterators/IteratorDictString.h"

using namespace std;
using namespace cds_utils;





class RankedStringDictionarySimple : public RankedStringDictionary
{
    static const size_t PERM_EPS = 4;

	public:
		/** Generic Constructor. */
		RankedStringDictionarySimple();

		/** Class Constructor.
		    @param it: iterator scanning the original set of strings.
		    @param bucketsize: number of strings represented per bucket.
		*/
		RankedStringDictionarySimple(IteratorDictString *it, uint bucketsize, uint dictionaryType, char compress);

		/** Retrieves the ID corresponding to the given string.
		    @param str: the string to be searched.
		    @param strLen: the string length.
		    @returns the ID (or NORESULT if it is not in the bucket).
		*/
		uint locate(uchar *str, uint strLen);

		/** Obtains the string associated with the given ID.
		    @param id: the ID to be extracted.
		    @param strLen: pointer to the extracted string length.
		    @returns the requested string (or NULL if it is not in the
		      dictionary).
		 */
		uchar* extract(size_t id, uint *strLen);

		/** Locates all IDs of those elements prefixed by the given
		    string.
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
		    @returns an iterator for direct scanning of all the IDs.
		*/
		IteratorRankedDictID* locatePrefix(uchar *str, uint strLen);

		/** Locates all IDs of those elements containing the given
		    substring.
		    @param str: the substring to be searched.
		    @param strLen: the substring length.
		    @returns an iterator for direct scanning of all the IDs.
		*/
		IteratorRankedDictID* locateSubstr(uchar *str, uint strLen);

		/** Retrieves the ID with rank k according to its alphabetical order.
		    @param rank: the alphabetical ranking.
		    @returns the ID.
		*/
		uint locateRank(uint rank);

		/** Extracts all elements prefixed by the given string.
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
		    @returns an iterator for direct scanning of all the strings.
		*/
		IteratorRankedDictString* extractPrefix(uchar *str, uint strLen);

		/** Extracts all elements containing by the given substring.
		    @param str: the substring to be searched.
		    @param strLen: the substring length.
		    @returns an iterator for direct scanning of all the strings.
		*/
		IteratorRankedDictString* extractSubstr(uchar *str, uint strLen);

		/** Obtains the string  with rank k according to its
		    alphabetical order.
		    @param id: the ID to be extracted.
		    @param strLen: pointer to the extracted string length.
		    @returns the requested string (or NULL if it is not in the
		      dictionary).
		*/
		uchar* extractRank(uint rank, uint *strLen);

		/** Extracts all strings in the dictionary sorted in
		    alphabetical order.
		    @returns an iterator for direct scanning of all the strings.
		*/
		IteratorRankedDictString* extractTable();

		/** Computes the size of the structure in bytes.
		    @returns the dictionary size in bytes.
		*/
		size_t getSize();

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
		void save(ofstream &out);

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
		IteratorRankedDictID* locateRankedPrefix(uchar *str, uint strLen, uint k);
		
		/** Extracts the first k strings of those elements prefixed by the given
		    string.
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
			@param k: the maximun number of elements to return
		    @returns an iterator for direct scanning of all the strings.
		*/
		IteratorRankedDictString* extractRankedPrefix(uchar *str, uint strLen, uint k);
		
		/** Extracts the first (ranked) k strings of those elements in the closed interval given
		    string.
			@param beginning: The first element of the interval
			@param len: The length of the interval
			@param k: the maximun number of elements to return
		    @returns an iterator for direct scanning of all the strings.
		*/
		IteratorRankedDictString* extractRankedInterval(uint beginning, uint len, uint k);
		
		/** Generic destructor. */
		~RankedStringDictionarySimple();

	protected:
        Permutation * permutation;
        StringDictionary * mDictionary ;

		inline size_t* insertionSort(uint* allIds, uint n, uint k);

};

#endif

