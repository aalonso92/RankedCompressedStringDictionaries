/* IteratorRankedDictString.h
 * Copyright (C) 2016, Álvaro Alonso
 * all rights reserved.
 *
 * Iterator class for scanning streams of strings for RankedStringDictionarySimple.
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



#ifndef _ITERATORRANKEDDICTSTRING1_H
#define _ITERATORRANKEDDICTSTRING1_H

#include <Permutation.h>
#include "../StringDictionary.h"
#include "IteratorDictID.h"


using namespace std;

class IteratorRankedDictString1 : public IteratorRankedDictString
{
	public:
		/** Constructor for the Ranked Iterator:
		    @dict: the dictionary with the lexicographically ordered strings.
		    @perm: permutation used to know the ranked order.
		    @iter: The iterator with the positions of the lexicographically ordered dictionary
		*/
        IteratorRankedDictString1(StringDictionary * dict, Permutation * perm, IteratorDictID * iter)
		{
			this->scanneable = iter->size();
			this->processed = 0;

			this->iter = iter;
			this->mDict = dict;
			this->perm = perm;
		}




		/** Extracts the next string in the stream.
		    @param strLen pointer to the string length.
		    @returns the next string.
		*/
		unsigned char* next(uint *str_length)
		{
            processed++;

            uint aux;

            //get the position in the ordered iterator (the lexicographically position
            uint orderedId = iter->next()-1;

            //get the string from the dictionary taking the ranked ordered element (using the permutation)
            uchar* result = mDict->extract(perm->pi(orderedId), &aux);

			*str_length = strlen((char*)(result));

			return result;
		}




		/** Checks for non-processed strings in the stream.
		    @returns if remains non-processed strings.
		*/
	   	bool hasNext()
		{
			return processed<scanneable;
		}






    protected:
        Permutation * perm; //The permutation saving the ranked order
        StringDictionary * mDict; //Dictionary holding the strings
        IteratorDictID * iter; //The lexicographically ordered ids
};

#endif
