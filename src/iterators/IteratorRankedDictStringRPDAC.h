/* IteratorRankedDictStringRPDAC.h
 * Copyright (C) 2016, Álvaro Alonso
 * all rights reserved.
 *
 * Iterator class for scanning strings in a Re-Pair+DAC representation. *
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



#ifndef _ITERATORRANKEDDICTSTRINGRPDAC_H
#define _ITERATORRANKEDDICTSTRINGRPDAC_H

#include <Permutation.h>
#include "IteratorDictID.h"


using namespace std;


class IteratorRankedDictStringRPDAC : public IteratorRankedDictString
{
	public:
		/** Constructor for the Ranked Iterator:
		    @rp: all the strings of the dictionary.
		    @Ids: a struct containing the position in the rp and the real ID (this last is not used).
		*/
        IteratorRankedDictStringRPDAC(RePair *rp, IteratorRankedDictID * positions, uint elements)
		{
			this->scanneable = elements;
			this->processed = 0;

			this->rp = rp;
			this->positions = positions;
		}




		/** Extracts the next string in the stream.
		    @param strLen pointer to the string length.
		    @returns the next string.
		*/
		unsigned char* next(uint *str_length)
		{	
			//extract the string from the RPDAC
			uint *rules;
			uint len = rp->Cdac->access(positions->next(), &rules);
			uchar *s = new uchar[maxlength+1];
			
			*str_length = 0;
			
			for (uint i=0; i<len; i++)
			{
				if (rules[i] >= rp->terminals) 
					(*str_length) += rp->expandRule(rules[i]-rp->terminals, (s+(*str_length)));
				else
				{
					s[*str_length] = (uchar)rules[i];
					(*str_length)++;
				}
			}
			
			s[*str_length] = (uchar)'\0';
			delete [] rules;
						
			processed++;
			
			return s;
		}




		/** Checks for non-processed strings in the stream.
		    @returns if remains non-processed strings.
		*/
	   	bool hasNext()
		{
			return processed<scanneable;
		}



		~IteratorRankedDictStringRPDAC()
		{
			delete positions;
		}


    protected:
        RePair *rp;
		IteratorRankedDictID* positions;
};

#endif
