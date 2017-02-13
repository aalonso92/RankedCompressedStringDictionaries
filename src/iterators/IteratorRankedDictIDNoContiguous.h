/* IteratorRankedDictIDNoContiguous.h
 * Copyright (C) 2016, Álvaro Alonso
 * all rights reserved.
 *
 * Iterator class for scanning streams of no contiguous IDs.
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




#ifndef _ITERATORRANKEDDICTIDNOCONTIGUOUS_H
#define _ITERATORRANKEDDICTIDNOCONTIGUOUS_H


#include <iostream>
using namespace std;

class IteratorRankedDictIDNoContiguous : public IteratorRankedDictID
{
	public:
		/** ID Iterator Constructor for streams of contiguous elements.
		    @param ids: the array of ids
		    @param scanneable: the number of ids in the array
		*/
		IteratorRankedDictIDNoContiguous(size_t *ids, size_t scanneable)
		{
			this->ids = ids;
			this->scanneable = scanneable;

			this->processed = 0;
		}

		/** Extracts the next ID in the stream. 
		    @returns the next ID.
		*/
	        size_t next() { return ids[processed++]; }

		/** Generic destructor */
		~IteratorRankedDictIDNoContiguous() { delete [] ids; }

	protected:
		size_t *ids;	// Array of ids
};

#endif  
