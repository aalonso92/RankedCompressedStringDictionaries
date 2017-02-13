/* DAC_BVLS.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class takes the features of Directly Addresable Codes to perform a
 * a symbol reordering on sequences of bytes:
 *
 *   ==========================================================================
 *     "DACs: Bringing Direct Access to Variable-Length Codes"
 *     Nieves Brisaboa, Susana Ladra, and Gonzalo Navarro.
 *     Information Processing and Management 49(1):392-404, 2013.
 *   ==========================================================================
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

#ifndef _DAC_BVLS
#define _DAC_BVLS

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <libcdsTrees.h>
#include <BitSequence.h>

#define BYTE 8

using namespace cds_static;

class DAC_BVLS
{
	public:
			
		DAC_BVLS(uchar *list, uint l_Length, uint max_seq_length);
		
		DAC_BVLS(uint tamCode, uint nLevels, vector<uint> * levelsIndex, vector<uint> * rankLevels, uchar *levels, BitString *bS);

		/*Return the length of the sequence that start at pos (pos start from 1 to n) and in seq store the sequence*/
		virtual uint access(uint pos, uint **seq) const;
			
		/*Return the value stored in position pos in the level l. It also store in pos the position of the 
		 * next value in the next level (if not exist next value return (uint)-1 in pos*/
		virtual uint access_next(uint l, uint *pos) const;
			
		/*Return the size, in bytes, used for DAC_BVLS structure*/
		virtual uint getSize() const;
			
		virtual void save(ofstream & fp) const;
			
		static DAC_BVLS* load(ifstream & fp);
			
		virtual ~DAC_BVLS();

	private:
		DAC_BVLS();

		uint tamCode;
		uint nLevels; 
		uint * levelsIndex;
		uchar * levels;
		BitSequence * bS;
		uint * rankLevels; 
};

#endif
