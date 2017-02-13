/* IteratorRankedDictID1.h
 * Copyright (C) 2016, Álvaro Alonso
 * all rights reserved.
 *
 * Iterator class for scanning streams of IDs in the RankedStringDictionarySimple.
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




#ifndef _ITERATORRANKEDDICTID1_H
#define _ITERATORRANKEDDICTID1_H

#include "IteratorDictID.h"
#include <iostream>
using namespace std;

class IteratorRankedDictID1 : public IteratorRankedDictID
{
	public:
		/** ID Iterator Constructor for streams of ranked elements.
		    @param perm: The permutation holding the ranked order
		    @param iter: The lexicographically ordered IDs
		*/
		IteratorRankedDictID1(Permutation *permutation, IteratorDictID *iter)
		{
			this->perm=permutation;
			this->iter=iter;

			this->scanneable = iter->size();
			this->processed = 0;
		}

		/** Extracts the next ID in the stream.
		    @returns the next ID.
		*/
	    	size_t next()
	    	{
                processed++;
                return perm->pi(iter->next()-1);

	    	}

		/** Generic destructor */
		~IteratorRankedDictID1() {}

	protected:
		Permutation * perm;
		IteratorDictID * iter;
};




#endif
