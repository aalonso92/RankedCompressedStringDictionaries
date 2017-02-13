/* StringDictionaryHHTFC.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a compressed StringDictionaryHHTFC combining Huffman
 * and Hu-Tucker compression over a dictionary of strings encoded with 
 * FrontCoding.
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


#include "StringDictionaryHHTFC.h"

StringDictionaryHHTFC::StringDictionaryHHTFC()
{
	this->type = HHTFC;
	this->elements = 0;
	this->maxlength = 0;
	this->maxcomplength = 0;

	this->buckets = 0;
	this->bucketsize = 0;
	this->bytesStrings = 0;

	this->textStrings = NULL;
	this->blStrings  = NULL;
	this->coderHT  = NULL;
	this->coderHU = NULL;
	this->codewordsHT = NULL;
	this->codewordsHU = NULL;
	this->tableHT = NULL;
	this->tableHU = NULL;
}

StringDictionaryHHTFC::StringDictionaryHHTFC(IteratorDictString *it, uint bucketsize)
{
	this->type = HHTFC;

	if (bucketsize < 2)
	{
		cerr << "[WARNING] The bucketsize value must be greater than 1. ";
		cerr << "The dictionary is built using buckets of size 2" << endl;
		this->bucketsize = 2;
	}
	else this->bucketsize = bucketsize;

	// 1) Bulding the Front-Coding representation
	StringDictionaryPFC *dict = new StringDictionaryPFC(it, this->bucketsize);
	this->maxlength = dict->maxlength;
	this->elements = dict->elements;
	this->buckets = dict->buckets;
	this->maxcomplength = 0;

	// 2) Obtaining the char frequencies and the corresponding Hu-Tucker and Huffman
	//    trees for the headers and the internal strings respectively
	uint *freqsHT = new uint[256];
	uint *freqsHU = new uint[256];

	// Initializing counters
	for (uint i=0; i<256; i++) { freqsHT[i]=1; freqsHU[i]=1; }

	size_t pbeg = 0, pend = 0;
	uint bucket = 1;

	while (bucket <= dict->buckets)
	{
		// Recollecting statistics for the headers
		pbeg = dict->blStrings->getField(bucket);
		bucket++;
		pend = dict->blStrings->getField(bucket);

		for (; dict->textStrings[pbeg] != 0; pbeg++) freqsHT[(int)(dict->textStrings[pbeg])]++;
		freqsHT[0]++; pbeg++;

		// Recollecting statistics for the internal strings
		for (; pbeg < pend; pbeg++) freqsHU[(int)(dict->textStrings[pbeg])]++;
	}

	// Obtaining the codes
	HuTucker *ht = new HuTucker(freqsHT);
	Huffman *hu = new Huffman(freqsHU);

	// Initializing the HuTucker builder
	DecodingTableBuilder *builderHT = new DecodingTableBuilder();
	builderHT->initializeFromHuTucker(ht);
	codewordsHT = builderHT->getCodewords();
	delete [] freqsHT; delete ht;

	// Initializing the Huffman builder
	DecodingTableBuilder *builderHU = new DecodingTableBuilder();
	builderHU->initializeFromHuffman(hu);
	codewordsHU = builderHU->getCodewords();
	delete [] freqsHU; delete hu;

	// 3) Compressing the dictionary and building the decoding table
	{
		vector<size_t> xblStrings;
		size_t ptr = 0; uint offset = 0, bytes = 0;
		uchar *tmp = new uchar[4*maxlength];

		size_t reservedStrings = MEMALLOC*bucketsize;
		textStrings = new uchar[reservedStrings];
		bytesStrings = 0; textStrings[bytesStrings] = 0;

		xblStrings.push_back(bytesStrings);	

		// Auxiliar variables for managing the substrings indexed in 
		// the Decoding Table.
		vector<uchar> textSubstr; vector<ushort> lenSubstr;
		ushort ptrSubstr=0; uint codeSubstr=0;

		coderHT = new StatCoder(codewordsHT);
		coderHU = new StatCoder(codewordsHU);

		for (uint current=1; current<=elements; current++)
		{
			// Checking the available space in textStrings and 
			// realloc if required
			while ((bytesStrings+(2*maxlength)) > reservedStrings)
				reservedStrings = Reallocate(&textStrings, reservedStrings);

			if (((current-1)%bucketsize) == 0)
			{
				// NEW BLOCK: updating counters and storing the
				// pointer to the current bucket
				bytes = 0; tmp[bytes] = 0; offset = 0;
				xblStrings.push_back(bytesStrings);

				do
				{
					// Encoding the header in tmp
					uchar symbol = (dict->textStrings[ptr]);
					bytes += coderHT->encodeSymbol(symbol, &(tmp[bytes]), &offset);
					ptr++;

					builderHT->insertDecodeableSubstr(symbol, &codeSubstr, &ptrSubstr, &textSubstr, &lenSubstr);

					if (bytes > maxcomplength) maxcomplength = bytes;
				}
				while (dict->textStrings[ptr-1] != '\0');

				{
					// Encoding the compressed header length
					if (offset > 0) bytes++;

					memcpy(textStrings+bytesStrings, tmp, bytes);
					bytesStrings += bytes;

					if (bytes > maxcomplength) maxcomplength = bytes;
				}


				// Adding an ending decodeable string  (if required)
				if (textSubstr.size() > 0)
				{
					// #######################
					// It is necessary to read up to TABLEBITSO bits for indexing the
					// substring in the DecodingTable
					if (offset > 0)
					{
						// The substring is also padded
						codeSubstr = (codeSubstr << (8-offset));
						ptrSubstr += (8-offset); offset = 0;
					}

					if (ptrSubstr > TABLEBITSO)
					{
						codeSubstr = codeSubstr >> (ptrSubstr - TABLEBITSO);
						ptrSubstr = TABLEBITSO;
					}
					else
					{
						if (current == elements)
						{
							// The last element is directly padded
							codeSubstr = (codeSubstr << (TABLEBITSO-ptrSubstr));
							ptrSubstr = TABLEBITSO;
							break;
						}

						uint read = 0;
						uint nextst = current+1;

						while (TABLEBITSO > ptrSubstr)
						{
							{
								// The VByte value is independently encoded to avoid confusions due to '\0' bytes
								uint value = 0;
								size_t xptr = VByte::decode(&value, &(dict->textStrings[ptr]));

								for (uint i=0; i<xptr; i++)
								{
									uint symbol = dict->textStrings[ptr+read]; read++;
									uint bits = codewordsHU[(int)symbol].bits;
									uint codeword = codewordsHU[(int)symbol].codeword;

									if ((bits+ptrSubstr) <= TABLEBITSO)
									{
										codeSubstr = (codeSubstr << bits) | codeword;
										ptrSubstr += bits;
										offset = (offset+bits) % 8;
									}
									else
									{
										uint remaining = TABLEBITSO-ptrSubstr;

										codeSubstr = (codeSubstr << remaining) | (codeword >> (bits-remaining));
										ptrSubstr = TABLEBITSO;

										break;
									}
								}
							}

							while (TABLEBITSO > ptrSubstr)
							{
								uint symbol = dict->textStrings[ptr+read]; read++;
								uint bits = codewordsHU[(int)symbol].bits;
								uint codeword = codewordsHU[(int)symbol].codeword;

								if ((bits+ptrSubstr) <= TABLEBITSO)
								{
									codeSubstr = (codeSubstr << bits) | codeword;
									ptrSubstr += bits;
									offset += bits;
									if (offset > 8) offset -= 8;

									// The next string has fully read!
									if (symbol == 0)
									{
										if ((nextst % bucketsize) == 0)
										{
											if (((ptrSubstr+(8-offset)) > TABLEBITSO))
											{
												offset = offset % 8;

												// The padding bits are enough...
												codeSubstr = (codeSubstr << (TABLEBITSO-ptrSubstr));
												ptrSubstr = TABLEBITSO;
											}
											else
											{
												while (true)
												{
													uint symbol = dict->textStrings[ptr+read]; read++;
													uint bits = codewordsHT[(int)symbol].bits;
													uint codeword = codewordsHT[(int)symbol].codeword;

													if ((bits+ptrSubstr) <= TABLEBITSO)
													{
														codeSubstr = (codeSubstr << bits) | codeword;
														ptrSubstr += bits;
														offset += bits;
														if (offset > 8) offset -= 8;
													}
													else
													{
														uint remaining = TABLEBITSO-ptrSubstr;

														codeSubstr = (codeSubstr << remaining) | (codeword >> (bits-remaining));
														ptrSubstr = TABLEBITSO;

														break;
													}
												}
											}
										}
									}
								}
								else
								{
									uint remaining = TABLEBITSO-ptrSubstr;

									codeSubstr = (codeSubstr << remaining) | (codeword >> (bits-remaining));
									ptrSubstr = TABLEBITSO;

									break;
								}
							}

							nextst++;
						}
					}

					builderHT->insertEndingSubstr(&codeSubstr, &ptrSubstr, &textSubstr, &lenSubstr);
				}

				offset = 0; textStrings[bytesStrings] = 0; 

				// Clearing decodeable substrings
				textSubstr.clear(); lenSubstr.clear();
				ptrSubstr=0, codeSubstr=0;
			}
			else
			{
				// The VByte is independently encoded to avoid that any
				// possible 0 to be confused with the end of a string
				uint value = 0;
				size_t xptr = VByte::decode(&value, &(dict->textStrings[ptr]));

				for (uint i=0; i<xptr; i++)
				{
					uchar code = (dict->textStrings[ptr]);
					bytesStrings += coderHU->encodeSymbol(code, &(textStrings[bytesStrings]), &offset);
					ptr++;

					builderHU->insertDecodeableSubstr(code, &codeSubstr, &ptrSubstr, &textSubstr, &lenSubstr);
				}	

				do
				{
					// Encoding the string
					uchar symbol = (dict->textStrings[ptr]);
					bytesStrings += coderHU->encodeSymbol(symbol, &(textStrings[bytesStrings]), &offset);
					ptr++;

					builderHU->insertDecodeableSubstr(symbol, &codeSubstr, &ptrSubstr, &textSubstr, &lenSubstr);
				}
				while (dict->textStrings[ptr-1] != '\0');

				if ((current%bucketsize) == 0)
				{
					// Ending the block
					if (offset > 0) { offset=0; bytesStrings++; textStrings[bytesStrings] = 0; }

					// Adding an ending decodeable string (if required)
					if (textSubstr.size() > 0)
						builderHU->insertEndingSubstr(&codeSubstr, &ptrSubstr, &textSubstr, &lenSubstr);

					// Clearing decodeable substrings
					textSubstr.clear(); lenSubstr.clear();
					ptrSubstr=0, codeSubstr=0;
				}
			}
		}

		delete [] tmp;

		bytesStrings++;
		xblStrings.push_back(bytesStrings);
		blStrings = new LogSequence(&xblStrings, bits(bytesStrings));

		maxcomplength += 4; // The value is increased because advanced readings in decoding...
	}


	delete dict;

	tableHT = builderHT->getTable(); delete builderHT;
	tableHU = builderHU->getTable(); delete builderHU;
}

uint 
StringDictionaryHHTFC::locate(uchar *str, uint strLen)
{
	uint id = NORESULT;
	// Encoding the string
	uint encLen, offset;
	uchar *encoded = coderHT->encodeString(str, strLen+1, &encLen, &offset);

	// Locating the candidate bucket for the string
	size_t idbucket;
	bool cmp = locateBucket(encoded, encLen, &idbucket);
	delete [] encoded;

	// The string is the header of the bucket
	if (cmp) id = ((idbucket-1)*bucketsize)+1;
	else
	{
		// The string is previous to any other one in the dictionary
		if (idbucket != NORESULT)
		{
			// The bucket is sequentially scanned to find the string
			ChunkScan c = decodeHeader(idbucket);
			resetScan(&c, idbucket);

			uint scanneable = bucketsize;
			if ((idbucket == buckets) && ((elements%bucketsize) != 0)) scanneable = (elements%bucketsize);

			if (scanneable > 1)
			{
				uint sharedCurr=0, sharedPrev=0;
				int cmp=0;

				// Processing the first internal string
				sharedPrev = coderHU->decodeString(&c);
				cmp=longestCommonPrefix(c.str+sharedCurr, str+sharedCurr, c.strLen-sharedCurr, &sharedCurr);

				if (cmp != 0)
				{
					for (uint i=2; i<scanneable; i++)
					{
						sharedPrev = coderHU->decodeString(&c);
						if  (sharedPrev < sharedCurr) break;

						cmp=longestCommonPrefix(c.str+sharedCurr, str+sharedCurr, c.strLen-sharedCurr, &sharedCurr);

						if (cmp==0)
						{
							id = ((idbucket-1)*bucketsize)+i+1;
							break;
						}
						else if (cmp > 0) break;
					}
				}
				else id = ((idbucket-1)*bucketsize)+2;
			}

			delete [] c.str;
		}
    	}

	return id;
}

uchar *
StringDictionaryHHTFC::extract(size_t id, uint *strLen)
{
	if ((id > 0) && (id <= elements))
	{
		uint idbucket = 1+((id-1)/bucketsize);
		uint pos = ((id-1)%bucketsize);

		ChunkScan c = decodeHeader(idbucket);

		if (pos > 0)
		{
			resetScan(&c, idbucket);
			for (uint i=1; i<=pos; i++) coderHU->decodeString(&c);
		}

		*strLen = c.strLen-1;
		return c.str;
	}
	else
	{
		*strLen = 0;
		return NULL;
	}
}

IteratorDictID*
StringDictionaryHHTFC::locatePrefix(uchar *str, uint strLen)
{
	// Encoding the string
	uint encLen, offset;
	uchar *encoded = coderHT->encodeString(str, strLen, &encLen, &offset);

	size_t leftBucket = 1, rightBucket = buckets;
	size_t leftID = 0, rightID = 0;

	// Locating the candidate buckets for the prefix
	locateBoundaryBuckets(encoded, encLen, offset, &leftBucket, &rightBucket);

	IteratorDictIDContiguous *itResult;

	if (leftBucket > NORESULT)
	{
		ChunkScan c = decodeHeader(leftBucket);
		resetScan(&c, leftBucket);

		uint scanneable = bucketsize;
		if ((leftBucket == buckets) && ((elements%bucketsize) != 0)) scanneable = (elements%bucketsize);

		if (leftBucket == rightBucket)
		{
			// All candidate results are in the same bucket
			leftID = searchPrefix(&c, scanneable, str, strLen);

			// No strings use the required prefix
			if (leftID == NORESULT) itResult = new IteratorDictIDContiguous(NORESULT, NORESULT);
			else
			{
				rightID = leftID+searchDistinctPrefix(&c, scanneable-leftID+1, str, strLen)-1;

				leftID += (leftBucket-1)*bucketsize;
				rightID += (rightBucket-1)*bucketsize;
			}
		}
		else
		{
			// All prefixes exceeds (possibly) a single bucket
			{
				// Searching the left limit
				leftID = searchPrefix(&c, scanneable, str, strLen);

				// The first prefix is the next bucket header
				if (leftID == NORESULT) leftID = leftBucket*bucketsize+1;
				// The first prefix is an internal string of the leftBucket
				else leftID += (leftBucket-1)*bucketsize;

				delete [] c.str;
			}

			{
				// Searching the right limit
				c = decodeHeader(rightBucket);
				resetScan(&c, rightBucket);

				scanneable = bucketsize;
				if ((rightBucket == buckets) && ((elements%bucketsize) != 0)) scanneable = (elements%bucketsize);

				rightID = searchDistinctPrefix(&c, scanneable, str, strLen);
				rightID += (rightBucket-1)*bucketsize;
			}
		}

		itResult = new IteratorDictIDContiguous(leftID, rightID);
		delete [] c.str;
	}
	else
	{
		// No strings use the required prefix
		itResult = new IteratorDictIDContiguous(NORESULT, NORESULT);
	}

	delete [] encoded;
	return itResult;
}

IteratorDictID*
StringDictionaryHHTFC::locateSubstr(uchar *str, uint strLen)
{
	cerr << "This dictionary does not provide substring location" << endl;
	return NULL;
}

uint 
StringDictionaryHHTFC::locateRank(uint rank)
{
	return rank;
}

IteratorDictString*
StringDictionaryHHTFC::extractPrefix(uchar *str, uint strLen)
{
	IteratorDictIDContiguous *it = (IteratorDictIDContiguous*)locatePrefix(str, strLen);
	size_t left = it->getLeftLimit();

	if (left != NORESULT)
	{
		// Positioning the LEFT Limit
		uint leftbucket = 1+((left-1)/bucketsize);
		uint leftpos = ((left-1)%bucketsize);

		// Positioning the RIGHT Limit
		size_t right = it->getRightLimit();
		delete it;

		return new IteratorDictStringHHTFC(tableHT, tableHU, codewordsHT, textStrings, blStrings, leftbucket, leftpos, bucketsize, right-left+1, maxlength, maxcomplength);
	}
	else return NULL;
}

IteratorDictString*
StringDictionaryHHTFC::extractSubstr(uchar *str, uint strLen)
{
	cerr << "This dictionary does not provide substring extraction" << endl;
	return 0; 
}

uchar *
StringDictionaryHHTFC::extractRank(uint rank, uint *strLen)
{
	return extract(rank, strLen);
}

IteratorDictString*
StringDictionaryHHTFC::extractTable()
{
	return new IteratorDictStringHHTFC(tableHT, tableHU, codewordsHT, textStrings, blStrings, 1, 0, bucketsize, elements, maxlength, maxcomplength);
}

size_t 
StringDictionaryHHTFC::getSize()
{
	cout << tableHT->getSize() << " bytes" << endl;
	cout << tableHU->getSize() << " bytes" << endl;
	return bytesStrings*sizeof(uchar)+blStrings->getSize()+256*2*sizeof(Codeword)+tableHT->getSize()+tableHU->getSize()+sizeof(StringDictionaryHHTFC);
}

void 
StringDictionaryHHTFC::save(ofstream &out)
{
	saveValue<uint32_t>(out, type);
	saveValue<uint64_t>(out, elements);
	saveValue<uint32_t>(out, maxlength);
	saveValue<uint32_t>(out, maxcomplength);
	saveValue<uint32_t>(out, buckets);
	saveValue<uint32_t>(out, bucketsize);

	saveValue<uint64_t>(out, bytesStrings);
	saveValue<uchar>(out, textStrings, bytesStrings);
	blStrings->save(out);	

	saveValue<Codeword>(out, codewordsHT, 256);
	tableHT->save(out);
	saveValue<Codeword>(out, codewordsHU, 256);
	tableHU->save(out);
}

StringDictionary*
StringDictionaryHHTFC::load(ifstream &in)
{
	StringDictionaryHHTFC *dict = new StringDictionaryHHTFC();

	dict->type = HHTFC;
	dict->elements = loadValue<uint64_t>(in);
	dict->maxlength = loadValue<uint32_t>(in);
	dict->maxcomplength = loadValue<uint32_t>(in);
	dict->buckets = loadValue<uint32_t>(in);
	dict->bucketsize = loadValue<uint32_t>(in);

	dict->bytesStrings = loadValue<uint64_t>(in);
	dict->textStrings = loadValue<uchar>(in, dict->bytesStrings);
	dict->blStrings = new LogSequence(in);

	dict->codewordsHT = loadValue<Codeword>(in, 256);
	dict->tableHT = DecodingTable::load(in);
	dict->coderHT = new StatCoder(dict->tableHT, dict->codewordsHT);

	dict->codewordsHU = loadValue<Codeword>(in, 256);
	dict->tableHU = DecodingTable::load(in);
	dict->coderHU = new StatCoder(dict->tableHU, dict->codewordsHU);

	return dict;
}

uchar*
StringDictionaryHHTFC::getHeader(size_t idbucket)
{
	size_t ptrH = blStrings->getField(idbucket);
	uchar *header = textStrings+ptrH;

	return header;
}

ChunkScan
StringDictionaryHHTFC::decodeHeader(size_t idbucket)
{
	uchar* ptr = textStrings+blStrings->getField(idbucket);
	ChunkScan chunk = {0, 0, ptr, maxcomplength, new uchar[4*maxlength+tableHT->getK()], 0, 0, 1};

	// Variables used for adjusting purposes
	uint plen = 0;
	uint pvalid = 0;
	uchar *pptr = chunk.b_ptr;

	while (true)
	{
		if (tableHT->processChunk(&chunk)) break;

		plen = chunk.strLen;
		pvalid = chunk.c_valid;
		pptr = chunk.b_ptr;
	}

	uint bits = 0;

	for (uint i=1; i<=chunk.strLen-plen; i++)
	{
		uchar c = chunk.str[chunk.strLen-i];
		bits += codewordsHT[c].bits;
	}

	chunk.c_valid = 8*(chunk.b_ptr-pptr) - bits + pvalid;
	chunk.b_ptr = chunk.b_ptr - (chunk.c_valid/8);

	return chunk;
}

bool 
StringDictionaryHHTFC::locateBucket(uchar *str, uint strLen, size_t *idbucket)
{
	size_t left = 1, right = buckets, center = 0;
    int cmp = 0;

	uchar *header;

	while (left <= right)
    {
		center = (left+right)/2;
		header = getHeader(center);

       		cmp = memcmp(header, str, strLen);

		// The string is in any preceding bucket
		if (cmp > 0) right = center-1;
		// The string is in any subsequent bucket
        	else if (cmp < 0) left = center+1;
		else 
		{ 
			// The string is the first one in the c-th bucket
			*idbucket = center;
			return true;
		}
	}

	// c is the candidate bucket for the string
	if (cmp < 0) *idbucket = center;
	// c-1 is the candidate bucket for the string
	else *idbucket = center-1;

	return false;
}

void
StringDictionaryHHTFC::locateBoundaryBuckets(uchar *str, uint strLen, uint offset, size_t *left, size_t *right)
{
	size_t center = 0;
	int cmp = 0;

	uchar *header = new uchar[4*strLen];

	uchar cmask = (uchar)(~(mask(8) >> offset));

	while (*left <= *right)
	{
		center = (*left+*right)/2;

		memcpy(header, getHeader(center), strLen);
		if (offset != 0) header[strLen-1] = header[strLen-1] & cmask;
       		cmp = memcmp(header, str, strLen);

		if (cmp > 0) *right = center-1;
		else if (cmp < 0) *left = center+1;
		else break;
	}

	if (cmp != 0)
	{
		// All prefixes are in the same block
        	if (cmp < 0) { *left = center; *right = center;}
	    	else { *left = center-1; *right = center-1; }

		delete [] header;
		return;
	}

	if (center > 1)
	{
		// Looking for the left boundary
		uint ll = *left, lr = center-1, lc;

		while (ll <= lr)
		{
			lc = (ll+lr)/2;

			memcpy(header, getHeader(lc), strLen);
			if (offset != 0) header[strLen-1] = header[strLen-1] & cmask;
		    cmp = memcmp(header, str, strLen);

			if (cmp == 0) lr = lc-1;
			else ll = lc+1;
		}	

		if (lr > NORESULT) *left = lr;
		else *left = 1;
	}

	if (center < buckets)
	{
		// Looking for the right boundary
		uint rl = center, rr = *right+1, rc;

		while (rl < (rr-1))
		{
			rc = (rl+rr)/2;

			memcpy(header, getHeader(rc), strLen);
			if (offset != 0) header[strLen-1] = header[strLen-1] & cmask;
		    cmp = memcmp(header, str, strLen);

			if (cmp == 0) rl = rc;
		    else rr = rc;
		}

		*right = rl;
	}

	delete [] header;
}

uint 
StringDictionaryHHTFC::searchPrefix(ChunkScan* c, uint scanneable, uchar *str, uint strLen)
{
	uint id = NORESULT;

	uint sharedCurr=0, sharedPrev=0;
	int cmp=0;

	uint i=1;
	while (true)
	{
		cmp=longestCommonPrefix(c->str+sharedCurr, str+sharedCurr, c->strLen-sharedCurr-1, &sharedCurr);

		if (sharedCurr==strLen) { id = i; break; }
		else
		{
			if ((cmp > 0) || (i == scanneable)) break;

			sharedPrev = coderHU->decodeString(c);
			i++;

			if  (sharedPrev < sharedCurr) break;	
		}
	}

	return id;
}


uint
StringDictionaryHHTFC::searchDistinctPrefix(ChunkScan* c, uint scanneable, uchar *str, uint strLen)
{
	uint id = 1;

	for (id=1; id<scanneable; id++) 
	{
		if (coderHU->decodeString(c) < strLen) break;
	}

	return id;
}

void
StringDictionaryHHTFC::resetScan(ChunkScan *c, size_t idbucket)
{
	c->c_chunk = 0;
	c->c_valid = 0;
	c->b_remain = textStrings+blStrings->getField(idbucket+1)-c->b_ptr;
	c->advanced = 0;	

}

StringDictionaryHHTFC::~StringDictionaryHHTFC()
{
	if (textStrings != NULL) delete [] textStrings;
	if (blStrings != NULL) delete blStrings;
	if (coderHT != NULL) delete coderHT;
	if (coderHU != NULL) delete coderHU;
	if (codewordsHT != NULL) delete [] codewordsHT;
	if (codewordsHU != NULL) delete [] codewordsHU;
	if (tableHT != NULL) delete tableHT;
	if (tableHU != NULL) delete tableHU;
}
