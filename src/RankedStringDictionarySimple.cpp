/* RankedStringDictionarySimple.cpp
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



#include "RankedStringDictionarySimple.h"



RankedStringDictionarySimple::RankedStringDictionarySimple()
{
	this->type = RDS;
	this->elements = 0;
	this->maxlength = 0;
}

RankedStringDictionarySimple::RankedStringDictionarySimple(IteratorDictString *it, uint bucketsize, uint dictionaryType, char compress)
{
	this->type = RDS;
	this->maxlength = 0;
	this->elements = 0;
	if(dictionaryType==4) //is HASHRPDAC
	{
		vector<uchar*> strings;
		while(it->hasNext())
		{
			uint strLen;
			strings.push_back(it->next(&strLen));
			elements++;
		}
		((IteratorDictStringPlain*)it)->restart();
		
		//create the hash dictionary
		mDictionary = new StringDictionaryHASHRPDAC(it, elements, bucketsize); //for hash the bucketsize represents the overhead
		

		//create the array for the permutation
		uint* elems = new uint[elements];
		for (uint i=0; i<elements; i++) elems[i] = 0;
		for (uint i=0; i<elements; i++)
		{
			uint strLen = strlen((char*)strings.at(i));
			uint pos = mDictionary->locate(strings.at(i), strLen);
			if(pos!=0)
				elems[i] = pos;
			if(strLen>maxlength) maxlength=strLen;
		}
		//set the needed bits for the permutation and each element
		uint bitsperm = bits(elements+1);
		uint sizeperm = (int)ceil(double(bitsperm*(elements+1))/W);
		
		//create the permutation (the permutation goes from 0 to elements-1, the position elements it's a 0)
		uint *perm = new uint[sizeperm];
		for (uint i=0; i<sizeperm; i++) perm[i] = 0;
		for (uint i=0; i<elements; i++) set_field(perm, bitsperm, i, elems[i]);
		
		delete [] elems;
		PermutationBuilder *pb = new PermutationBuilderMRRR(PERM_EPS, new BitSequenceBuilderRG(20));
		permutation = pb->build(perm, elements+1);
		delete pb;
	}
	else
	{
		//Create a struct list containing the text and the ID of each element of the dictionary
		vector<dictItem> itemsList;

		//Insert the items in the list using the iterator
		uchar* strCurrent=NULL;
		uint lenCurrent=0;
		uint id=0;
		while(it->hasNext())
		{
			id++;
			strCurrent = it->next(&lenCurrent);
			if (lenCurrent >= maxlength) maxlength = lenCurrent+1; //set the maxlength
			dictItem item;
			item.itemId=id;
			item.itemText = strCurrent;

			itemsList.push_back(item);
		}

		this->elements=itemsList.size();


		//order the list using a class
		sort(itemsList.begin(), itemsList.end(), sorting);



		//create the array for the permutation
		uint* elems = new uint[itemsList.size()];
		uint pos=0;
		for (std::vector<dictItem>::iterator it = itemsList.begin() ; it != itemsList.end(); ++it)
		{
			elems[it->itemId-1] = pos+1;
			pos++;
		}
		//set the needed bits for the permutation and each element
		uint bitsperm = bits(elements+1);
		uint sizeperm = (int)ceil(double(bitsperm*(elements+1))/W);

		//create the permutation (the permutation goes from 0 to elements-1, the position elements it's a 0)
		uint *perm = new uint[sizeperm];
		for (uint i=0; i<sizeperm; i++) perm[i] = 0;
		for (uint i=0; i<elements; i++) set_field(perm, bitsperm, i, elems[i]);
		delete [] elems;

		PermutationBuilder *pb = new PermutationBuilderMRRR(PERM_EPS, new BitSequenceBuilderRG(20));
		permutation = pb->build(perm, elements+1);
		delete pb;


		//create the array for the dictionary
		vector<uchar*> elemsStr;
		for (uint i=0; i<itemsList.size();i++)
		{
			elemsStr.push_back(itemsList[i].itemText);
		}


		//create the String Dictionary with the strings ordered
		IteratorDictString *iter = new IteratorDictStringVector(&elemsStr, elements); //create the iterator

		//create the dictionary
		switch(dictionaryType)
		{
			case 1: //PFC Ranked dictionary
			{
				if(compress == 'p')
				{
					//plain compression
					cout << "PFC" << endl;
					mDictionary = new StringDictionaryPFC(iter, bucketsize);
				}
				else if (compress == 'r')
				{
					//Re-Pair compression
					cout << "RPCF" << endl;
					mDictionary = new StringDictionaryRPFC(iter, bucketsize);
				}
				else
					cerr << "wrong parameters" << endl;
				break;
			}
			case 2: //HTFC Ranked dictionary
			{
				if (compress == 't')
				{
					// HuTucker compression
                    cout << "HTCF" << endl;
                    mDictionary = new StringDictionaryHTFC(iter, bucketsize);
				}
				else  if(compress == 'r')
				{
					// RePair compression
                    cout << "RPHTFC" << endl;
                    mDictionary = new StringDictionaryRPHTFC(iter, bucketsize);
				}
				else if(compress == 'h')
				{
					// Huffman compression
                    cout << "HHTFC" << endl;
                    mDictionary = new StringDictionaryHHTFC(iter, bucketsize);
				}
				else
                    cerr << "wrong parameters" << endl;
				break;
			}
				case 3: //RPDAC Ranked dictionary
			{
				cout << "RPDAC" << endl;
				mDictionary = new StringDictionaryRPDAC(iter);
				break;
			}
		}
	}
}







uint RankedStringDictionarySimple::locate(uchar *str, uint strLen)
{
    uint orderedPos = mDictionary->locate(str,strLen);
    if(orderedPos == NORESULT) //if the string it's not in the dictionary orderedPos=0
        return NORESULT;

	uint elemId = permutation->revpi(orderedPos)+1; //+1 because the permutation starts with 0
    return elemId;
}





uchar* RankedStringDictionarySimple::extract(size_t id, uint *strLen)
{
    //if the id its higher than the number of elements return 0
    if(id>elements)
        return 0;
    uint orderedPos = permutation->pi(id-1); //-1 because the permutation starts with 0 and we start the ids with 1
    uchar* s = mDictionary->extract(orderedPos, strLen);
	return s;
}





IteratorRankedDictID* RankedStringDictionarySimple::locatePrefix(uchar *str, uint strLen)
{

    IteratorDictID * orderedIds =  mDictionary->locatePrefix(str, strLen); //get the positions in the ordered dictionary

    //if the iterator is null return null
    if(!orderedIds->hasNext())
    {
        delete orderedIds;
        return NULL;
    }

    //create a ranked id iterator with the permutation and the ordered iterator
    IteratorRankedDictID * rankedIds = new IteratorRankedDictID1(permutation,orderedIds);
    return rankedIds;
}






IteratorRankedDictID* RankedStringDictionarySimple::locateSubstr(uchar *str, uint strLen)
{
    cerr << "This dictionary does not provide substring location" << endl;
	return NULL;
}






uint RankedStringDictionarySimple::locateRank(uint rank)
{
    return rank;
}






IteratorRankedDictString* RankedStringDictionarySimple::extractPrefix(uchar *str, uint strLen)
{
    IteratorDictID * orderedIds =  mDictionary->locatePrefix(str, strLen); //get the positions in the ordered dictionary

    //if the iterator is null return null
    if(!orderedIds->hasNext())
    {
        delete orderedIds;
        return NULL;
    }

    //create a ranked string iterator with the permutation, the dictionary and the ordered iterator
    IteratorRankedDictString * rankedStrings = new IteratorRankedDictString1(mDictionary, permutation, orderedIds);
    return rankedStrings;
}






IteratorRankedDictString* RankedStringDictionarySimple::extractSubstr(uchar *str, uint strLen)
{
	cerr << "This dictionary does not provide substring extraction" << endl;
    return NULL;
}






uchar* RankedStringDictionarySimple::extractRank(uint rank, uint *strLen)
{
    return extract(rank, strLen);
}






IteratorRankedDictString* RankedStringDictionarySimple::extractTable()
{
cerr << "This dictionary does not provide table extraction" << endl;
    return NULL;
}







size_t RankedStringDictionarySimple::getSize()
{
    return permutation->getSize() + mDictionary->getSize() + sizeof(RankedStringDictionarySimple);
}






uint RankedStringDictionarySimple::maxLength()
{
    return mDictionary->maxLength();
}






size_t RankedStringDictionarySimple::numElements()
{
    return elements;
}






void RankedStringDictionarySimple::save(ofstream &out)
{
    saveValue<uint32_t>(out, type);
    saveValue<uint64_t>(out, elements);
	saveValue<uint32_t>(out, maxlength);
	permutation->save(out);
	mDictionary->save(out);
}






RankedStringDictionary * RankedStringDictionarySimple::load(ifstream &in)
{
    RankedStringDictionarySimple * dict = new RankedStringDictionarySimple();
    dict->type = RDS;
    dict->elements = loadValue<uint64_t>(in);
    dict->maxlength = loadValue<uint32_t>(in);
    dict->permutation = Permutation::load(in);
    uint32_t dictType = loadValue<uint32_t>(in);
    switch(dictType) //check the inside dictionary type
    {
        case PFC:
        {
            dict->mDictionary = StringDictionaryPFC::load(in);
            break;
        }
        case RPFC:
        {
            dict->mDictionary = StringDictionaryRPFC::load(in);
            break;
        }
        case HTFC:
        {
            dict->mDictionary = StringDictionaryHTFC::load(in);
            break;
        }
        case HHTFC:
        {
            dict->mDictionary = StringDictionaryHHTFC::load(in);
            break;
        }
        case RPHTFC:
        {
            dict->mDictionary = StringDictionaryRPHTFC::load(in);
            break;
        }
        case RPDAC:
        {
            dict->mDictionary = StringDictionaryRPDAC::load(in);
            break;
        }
		case HASHRPDAC:
		{
			dict->mDictionary = StringDictionaryHASHRPDAC::load(in);
			break;
		}
		default:
        {
            return NULL;
        }
    }
	
    return dict;
}


IteratorRankedDictID* RankedStringDictionarySimple::locateRankedPrefix(uchar *str, uint strLen, uint k)
{
	IteratorDictID * orderedIds =  mDictionary->locatePrefix(str, strLen); //get the positions in the ordered dictionary

    //if the iterator is null return null
    if(!orderedIds->hasNext())
    {
        delete orderedIds;
        return NULL;
    }
	
	
	uint* allIds = new uint[orderedIds->size()];
	uint x=0;
	
	while(orderedIds->hasNext()) //obtain the real IDs with the permutation
	{
		allIds[x] = permutation->revpi(orderedIds->next())+1;
		x++;
	}
	if(x<k)
		k=x;
	//get the first k with an insertion sort
	size_t* topk = insertionSort(allIds,x,k);
	
	
	return new IteratorRankedDictIDNoContiguous(topk,k);
}

IteratorRankedDictString* RankedStringDictionarySimple::extractRankedPrefix(uchar *str, uint strLen, uint k)
{
	IteratorDictID * orderedIds =  mDictionary->locatePrefix(str, strLen); //get the positions in the ordered dictionary

    //if the iterator is null return null
    if(!orderedIds->hasNext())
    {
        delete orderedIds;
        return NULL;
    }
	
	
	uint* allIds = new uint[orderedIds->size()];
	uint x=0;
	
	while(orderedIds->hasNext()) //obtain the real IDs with the permutation
	{
		allIds[x] = permutation->revpi(orderedIds->next())+1;
		x++;
	}
	if(x<k)
		k=x;
	//get the first k with an insertion sort
	size_t* topk = insertionSort(allIds,x,k);
	delete [] allIds;
	//extract the strings for the ids
	vector<uchar*> strings;
	uint strl;
	for(uint i=0; i<k; i++)
		strings.push_back(this->extract(topk[i],&strl));
	
	IteratorRankedDictString* it = new IteratorRankedDictStringVector(&strings, k);
	
	return it;
}


IteratorRankedDictString* RankedStringDictionarySimple::extractRankedInterval(uint beginning, uint len, uint k)
{
	if(len<k) //if the interval is less than k, set k to the interval length
		k=len;
		
	uint* allIds = new uint[len];
	
	for(uint i=0; i<len; i++) //obtain the real IDs of the interval with the permutation
	{
		allIds[i] = permutation->revpi(beginning+i)+1;
	}
	
	//get the first k with an insertion sort
	size_t* topk = insertionSort(allIds,len,k);
	//extract the strings for the ids
	vector<uchar*> strings;
	uint strl;
	for(uint i=0; i<k; i++)
		strings.push_back(this->extract(topk[i],&strl));
	
	delete [] topk;
	IteratorRankedDictString* it = new IteratorRankedDictStringVector(&strings, k);
	
	return it;
}


RankedStringDictionarySimple::~RankedStringDictionarySimple()
{
    delete mDictionary;
    delete permutation;
}




size_t* RankedStringDictionarySimple::insertionSort(uint* allIds, uint n, uint k)
{
	size_t* topk = new size_t[k];
	
	//initialize the array to a big number
	for(uint i=0; i<k; i++)
		topk[i]=this->elements+1;
	
	int actualPos; //the position to be compared
	uint actualId; //the id of the topk array to be compared
	//only one run to the array to sort
	for(uint i=0; i<n; i++)
	{
		actualPos=k-1;
		actualId=topk[actualPos];
		if(actualId>allIds[i]) //the new id is smaller
		{
			topk[actualPos]=allIds[i];
			actualPos--;
			actualId=topk[actualPos];
			while(actualId>allIds[i] && actualPos>=0) //advance the new ID while it is smaller
			{
				topk[actualPos]=allIds[i];
				topk[actualPos+1]=actualId;
				actualPos--;
				if(actualPos>=0)
					actualId=topk[actualPos];
			}
		}
	}

	return topk;
}

