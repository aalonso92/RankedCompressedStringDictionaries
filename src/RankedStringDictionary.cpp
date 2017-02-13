/* RankedStringDictionary.cpp
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

#include "RankedStringDictionary.h"

RankedStringDictionary* RankedStringDictionary::load(ifstream & fp)
{
	size_t r = loadValue<uint32_t>(fp);

	switch(r)
	{
		case RDS: return RankedStringDictionarySimple::load(fp);
		case RDRPDAC: return RankedStringDictionaryRPDAC::load(fp);
	}

	return NULL;
}

uint RankedStringDictionary::maxLength()
{
	return maxlength;
}

size_t RankedStringDictionary::numElements()
{
	return elements;
}

