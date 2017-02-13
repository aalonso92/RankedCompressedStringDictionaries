/* IteratorDictID.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning streams of IDs.
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

#ifndef _ITERATORDICTID_H
#define _ITERATORDICTID_H


#include <iostream>
using namespace std;

class IteratorDictID
{
	public:
		/** Checks for non-processed IDs in the stream.
		    @returns if remains non-processed IDs.
		*/
	    	bool hasNext() { return processed<scanneable; };

		/** Extracts the next ID in the stream.
		    @returns the next ID.
		*/
	    	virtual size_t next()=0;

		/** Returns the remaining ids to be retrieved */
		uint size() { return scanneable-processed; } //OJOOOOOOOOOOOOOOOOOOOOOOOOOOOOO

		/** Generic destructor */
		virtual ~IteratorDictID() {} ;

	protected:
		size_t processed;	// Number of processed IDs
		size_t scanneable;	// Upper limit of the stream
};

#include "IteratorDictIDContiguous.h"
#include "IteratorDictIDNoContiguous.h"
#include "IteratorDictIDDuplicates.h"
#include "IteratorDictIDXBW.h"
#include "IteratorDictIDXBWDuplicates.h"

#endif
