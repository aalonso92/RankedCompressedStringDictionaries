/* IteratorRankedDictString.h
 * Copyright (C) 2016, Álvaro Alonso
 * all rights reserved.
 *
 * Iterator class for scanning streams of strings.
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



#ifndef _ITERATORRANKEDDICTSTRING_H
#define _ITERATORRANKEDDICTSTRING_H

#include <string.h>

#include <cassert>
#include <iostream>
using namespace std;


class IteratorRankedDictString
{
	public:
		/** Checks for non-processed strings in the stream.
		    @returns if remains non-processed strings.
		*/
		virtual bool hasNext()=0;

		/** Extracts the next string in the stream.
		    @param strLen pointer to the string length.
		    @returns the next string.
		*/
	   	virtual unsigned char* next(uint *str_length)=0;

		/** Generic destructor. */
		virtual ~IteratorRankedDictString() {} ;

		/** Returns the remaining strings to be retrieved */
		uint size() { return scanneable-processed; }


	protected:
		size_t processed;	// Number of processed strings
		size_t scanneable;	// Upper limit of the stream
		uint maxlength;		// Largest string length
};


#include "IteratorRankedDictString1.h"
#include "IteratorRankedDictStringVector.h"
#include "IteratorRankedDictStringRPDAC.h"


#endif
