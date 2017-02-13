/* RankedStringDictionaryRPDAC.cpp
 * Copyright (C) 2016, Álvaro Alonso
 * all rights reserved.
 *
 * This class implements a Compressed Ranked String Dictionary based on RPDAC.
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



#include "RankedStringDictionaryRPDAC.h"



RankedStringDictionaryRPDAC::RankedStringDictionaryRPDAC()
{
	this->type = RDRPDAC;
	this->elements = 0;
	this->maxlength = 0;
	this->bucketsize = 0;
	this->numBuckets = 0;
	this->rp=NULL;
	this->H=NULL;
	this->P=NULL;
}

RankedStringDictionaryRPDAC::RankedStringDictionaryRPDAC(IteratorDictString *it, uint bucketsize)
{
	this->type = RDRPDAC;
	this->maxlength = 0;
	this->rp = NULL;
	this->H = NULL;
	this->P = NULL;
	this->bucketsize = bucketsize;
	this->numBuckets = 0;

	//Create a struct list containing the text and the ID of each element of the dictionary
	vector<dictItem> itemsList;

	//Insert the items in the list using the iterator
	uchar* strCurrent=NULL;
	uint lenCurrent=0;
	uint id=0;
	uint mBytes=it->size();
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

	
	// Create P
	uint * bucketNumber = new uint[elements];
	uint pos = 0;
	for (std::vector<dictItem>::iterator it = itemsList.begin() ; it != itemsList.end(); it++)
    {
        uint rankedPos = it->itemId; //get the ID of the element
		bucketNumber[rankedPos-1] = floor(pos/bucketsize)+1; //same as ceil((pos+1)/bucketsize)
        pos++;
    }

	P = new WaveletTreeNoptrs(bucketNumber, elements, new BitSequenceBuilder375(), new MapperNone());
	//P = new WaveletTreeNoptrs(bucketNumber, elements, new BitSequenceBuilderRG(20), new MapperNone());
	
	delete [] bucketNumber;
	

	this->numBuckets = floor((elements-1)/bucketsize)+1; //same as ceil(elements/bucketsize)

	//Reorder the inside the buckets and get H 
	/**MEJOR CON UN SORT**/
	size_t * headers = new size_t[numBuckets];
	uint * finalOrder = new uint[elements]; //for saving the permutation between the lexicographically position and the final one
	for(uint i=0; i<numBuckets-1; i++) //for each bucket (except the last one)
	{
		for(uint j=0; j<bucketsize; j++) //for each element of the bucket
		{
			uint aux = 0;
			for(uint k=0; k<bucketsize; k++) //for each element of the bucket
			{
				//compare the IDs (the smallest one will be first)
				if(itemsList[i*bucketsize+j].itemId > itemsList[i*bucketsize+k].itemId)
					aux++;
			}
			//insert the actual element in the correct position
			finalOrder[i*bucketsize+aux] = i*bucketsize+j;
			//if it is the first element (lexicographically) store the position in the header array
			if(j==0)
				headers[i] = aux+1;
		}
	}
	//reorder the last bucket
	for(uint position = (numBuckets-1)*bucketsize; position<elements; position++) //for each element of the bucket
	{
		uint aux = 0;
		for(uint k=(numBuckets-1)*bucketsize; k<elements; k++) //for each element of the bucket
		{
			if(itemsList[position].itemId > itemsList[k].itemId)
				aux++;
		}
		finalOrder[(numBuckets-1)*bucketsize+aux] = position;
		if(position == (numBuckets-1)*bucketsize)
			headers[numBuckets-1] = aux+1;
	}
	
	
	//Make the logsecuence H
	vector<size_t> headersList(headers, headers+numBuckets);
	H = new LogSequence(&headersList, bits(numBuckets));
	delete [] headers;
	
	
	//Create the RPDAC
	uchar *current=NULL;
	uint currentLen=0;
	size_t processed = 0;
	int *dict = new int[mBytes];
	for(uint i=0; i<elements; i++)
	{
		current = itemsList[finalOrder[i]].itemText;
		currentLen = strlen((char*)current);
		
		for (uint j=0; j<=currentLen; j++) //for each character of the string
			dict[processed+j] = current[j];
		processed += currentLen+1;
	}
	
	delete [] finalOrder;
	rp = new RePair(dict, processed, 0);
	
	
	// Compacting the sequence (a -i value is inserted after the i-th string).
	int *cdict = new int[processed];
	uint io = 0, ic = 0, strings = 0;
	uint maxseq = 0, currentseq = 0;

	while (io<processed)
	{
		if (dict[io] >= 0)
		{
			if (dict[io] == 0)
			{
				if (currentseq > maxseq) maxseq = currentseq;

				strings++;

				cdict[ic] = -strings;
				io++; ic++;
				currentseq = 0;
			}
			else
			{
				cdict[ic] = dict[io];
				io++; ic++;
				currentseq++;
			}
		}
		else
		{
			if (io < processed) io = -(dict[io]+1);
		}
	}

	// Building the array for the sequence
	rp->Cdac = new DAC_VLS(cdict, ic-2, bits(rp->rules+rp->terminals), maxseq);

	delete [] cdict;
	delete [] dict;
}



uint RankedStringDictionaryRPDAC::locate(uchar *str, uint strLen)
{
	/**binary search for finding the bucket in which the element is**/
	
	//initialize the limits of the actual part of the search
	uint left = 0;
	uint right = numBuckets-1;
	uint center = floor(numBuckets/2);
	//binary search
	while(left!=right)
	{
		uint headerPosition = bucketsize*center + H->getField(center); //(bucketsize*center) gets the element inmidiatly before the center bucket
		int compare = rp->extractStringAndCompareDAC(headerPosition, str, strLen);
		
		if(compare > 0) //the searched string is to the left
		{
			right=center-1;
		}
		else
		{
			if(compare < 0) //the searched string is to the right (or in the same bucket)
			{
				left=center;
			}
			else //the header is the string searched (the id of the element searched is P->select(bucketNumber,innerBucketPosition)
				return P->select(center+1,H->getField(center))+1; // +1 because P starts from 0
		}
		center = left + floor((right-left+1)/2);
	}
	
	//the searched string is in the bucket center (starting from 0)
	//search in the bucket for the string (if not found at the end return -1)
	for(uint i=1; i<=bucketsize; i++)
	{
		uint elemPosition = bucketsize*center + i;
		int comp = rp->extractStringAndCompareDAC(elemPosition, str, strLen);
		if(comp == 0) //if we found it return the position
			return P->select(center+1,i)+1; //(the id of the element searched is P->select(bucketNumber,innerBucketPosition)+1, because P starts from 0
	}
	
	//if we reach this point we have not found the string, so return -1
	return -1;
}



uchar* RankedStringDictionaryRPDAC::extract(size_t id, uint *strLen)
{
    //if the id its higher than the number of elements return 0
    if(id>elements || id==0)
    {
		uchar *s = new uchar[1];
		s[0]='\0';
		return s;
	}
    
	id--; //the positions of the sctructures starts from 0
	//Firstly obtain the bucketNumber and the position inside the bucket
	size_t bucketPos;
	uint bucketNumber = P->access(id, bucketPos);
	//get the position of the element in the RPDAC
	uint rpPos = bucketsize*(bucketNumber-1) + bucketPos;
	
	//extract the string from the RPDAC
	uint *rules;
	uint len = rp->Cdac->access(rpPos, &rules);
	uchar *s = new uchar[maxlength+1];
			
	*strLen = 0;
			
	for (uint i=0; i<len; i++)
	{
		if (rules[i] >= rp->terminals) 
			(*strLen) += rp->expandRule(rules[i]-rp->terminals, (s+(*strLen)));
		else
		{
			s[*strLen] = (uchar)rules[i];
			(*strLen)++;
		}
	}
			
	s[*strLen] = (uchar)'\0';
	delete [] rules;
			
	return s;
}



IteratorRankedDictID* RankedStringDictionaryRPDAC::locatePrefix(uchar *str, uint strLen)
{	
	size_t numLocated;
	//obtain the positions of the strings with the prefix in the RPDAC
	IteratorRankedDictID * prefIt = findPrefix(str, strLen, &numLocated);
	
	
	//get the ids from the positions
	size_t * ids = new size_t[numLocated];
	for(uint i=0; i<numLocated; i++)
	{	
		uint position = prefIt->next();
		uint bucket = floor((position-1) / bucketsize);
		uint innerPos = (position % bucketsize);
		if(innerPos==0) innerPos=bucketsize;
		ids[i] = P->select(bucket+1,innerPos)+1;
		
	}

	/*Create a non contiguous ID iterator*/
	IteratorRankedDictID* it = new IteratorRankedDictIDNoContiguous(ids, numLocated);
	
	return it;
}



IteratorRankedDictID* RankedStringDictionaryRPDAC::locateSubstr(uchar *str, uint strLen)
{
    cerr << "This dictionary does not provide substring location" << endl;
	return NULL;
}




uint RankedStringDictionaryRPDAC::locateRank(uint rank)
{
    return rank;
}



IteratorRankedDictString* RankedStringDictionaryRPDAC::extractPrefix(uchar *str, uint strLen)
{
	size_t numLocated;
	//obtain the positions of the strings with the prefix in the RPDAC
	IteratorRankedDictID * prefIt = findPrefix(str, strLen, &numLocated);
	 
	IteratorRankedDictString* iter = new IteratorRankedDictStringRPDAC(rp,prefIt,numLocated);
    
	return iter;
}




IteratorRankedDictString* RankedStringDictionaryRPDAC::extractSubstr(uchar *str, uint strLen)
{
	cerr << "This dictionary does not provide substring extraction" << endl;
    return NULL;
}





uchar* RankedStringDictionaryRPDAC::extractRank(uint rank, uint *strLen)
{
    return extract(rank, strLen);
}




IteratorRankedDictString* RankedStringDictionaryRPDAC::extractTable()
{
cerr << "This dictionary does not provide table extraction" << endl;
    return NULL;
}




size_t RankedStringDictionaryRPDAC::getSize()
{
    return rp->getSize() + P->getSize() + H->getSize() + sizeof(RankedStringDictionaryRPDAC);
}




uint RankedStringDictionaryRPDAC::maxLength()
{
    return maxlength;
}



size_t RankedStringDictionaryRPDAC::numElements()
{
    return elements;
}



void RankedStringDictionaryRPDAC::save(ofstream &out)
{
	//save type, elements and maxlength (common for every RankedDictionary)
    saveValue<uint32_t>(out, type);
    saveValue<uint64_t>(out, elements);
    saveValue<uint32_t>(out, maxlength);
	
	//save bucketsize, number of buckets, RPDAC, P and H (specific of this dictionary)
    saveValue<uint32_t>(out, bucketsize);
	saveValue<uint32_t>(out, numBuckets);
	rp->save(out, RPDAC);
	P->save(out);
	H->save(out);
	if(rp==NULL) cout<<"NULLrp"<<endl;
	if(P==NULL) cout<<"NULLP"<<endl;
	if(H==NULL) cout<<"NULLH"<<endl;
}




RankedStringDictionary * RankedStringDictionaryRPDAC::load(ifstream &in)
{
    RankedStringDictionaryRPDAC * dict = new RankedStringDictionaryRPDAC();
	//load elements and maxlength (common for every RankedDictionary)
    dict->type = RDRPDAC;
    dict->elements = loadValue<uint64_t>(in);
    dict->maxlength = loadValue<uint32_t>(in);
	
	//load bucketsize, number of buckets, RPDAC, P and H (specific of this dictionary)
	dict->bucketsize = loadValue<uint32_t>(in);
	dict->numBuckets = loadValue<uint32_t>(in);
    dict->rp = RePair::load(in);
	dict->P = WaveletTreeNoptrs::load(in);
	dict->H = new LogSequence(in); //the LogSequence doesn't have load, it has a constructor with the in file
	if(dict->rp==NULL) cout<<"NULLrp"<<endl;
	if(dict->P==NULL) cout<<"NULLP"<<endl;
	if(dict->H==NULL) cout<<"NULLH"<<endl;
	return dict;
}


IteratorRankedDictID* RankedStringDictionaryRPDAC::locateRankedPrefix(uchar *str, uint strLen, uint k)
{
	//obtain the ids
	size_t numLocated=0;
	size_t ** ids = topKprefix(str, strLen, k, &numLocated);
	
	size_t * finalIds = new size_t[numLocated];
	for(uint i=0; i<numLocated;i++)
		finalIds[i]=ids[1][i];
	
	delete [] ids[0];
	delete [] ids[1];
	delete [] ids;
	
	/*Create a non contiguous ID iterator*/
	IteratorRankedDictID* it = new IteratorRankedDictIDNoContiguous(finalIds, numLocated);
	
	
    return it;
	
}


IteratorRankedDictString* RankedStringDictionaryRPDAC::extractRankedPrefix(uchar *str, uint strL, uint k)
{
	//obtain the ids
	size_t numLocated=0;
	size_t ** ids = topKprefix(str, strL, k, &numLocated);
	
	//for each id obtain the string
	vector<uchar*> strings;
	for(uint i=0; i<numLocated; i++)
	{
		//extract the string from the RPDAC
		uint *rules;
		uint len = rp->Cdac->access(ids[0][i], &rules);
		uchar *s = new uchar[maxlength+1];
			
		uint strLen = 0;
			
		for (uint i=0; i<len; i++)
		{
			if (rules[i] >= rp->terminals) 
				strLen += rp->expandRule(rules[i]-rp->terminals, (s+strLen));
			else
			{
				s[strLen] = (uchar)rules[i];
				strLen++;
			}
		}
			
		s[strLen] = (uchar)'\0';
		delete [] rules;
			
		strings.push_back(s);
	}
	if(ids!=NULL)
	{
		delete [] ids[0];
		delete [] ids[1];
		delete [] ids;
	}
	
	IteratorRankedDictString* it = new IteratorRankedDictStringVector(&strings, numLocated);
	
	return it;
}



IteratorRankedDictString* RankedStringDictionaryRPDAC::extractRankedInterval(uint beginning, uint len, uint k)
{
	//obtain the ids
	size_t ** ids = topKInterval(beginning, len, k);
	
	if(len<k)
		k=len;
	
	//for each id obtain the string
	vector<uchar*> strings;
	for(uint i=0; i<k; i++)
	{
		//extract the string from the RPDAC
		uint *rules;
		uint len = rp->Cdac->access(ids[0][i], &rules);
		uchar *s = new uchar[maxlength+1];
			
		uint strLen = 0;
			
		for (uint i=0; i<len; i++)
		{
			if (rules[i] >= rp->terminals) 
				strLen += rp->expandRule(rules[i]-rp->terminals, (s+strLen));
			else
			{
				s[strLen] = (uchar)rules[i];
				strLen++;
			}
		}
			
		s[strLen] = (uchar)'\0';
		delete [] rules;
			
		strings.push_back(s);
	}
	if(ids!=NULL)
	{
		delete [] ids[0];
		delete [] ids[1];
		delete [] ids;
	}
	
	IteratorRankedDictString* it = new IteratorRankedDictStringVector(&strings, k);
	
	return it;
}


RankedStringDictionaryRPDAC::~RankedStringDictionaryRPDAC()
{
	if(rp!=NULL) delete rp;
	if(P!=NULL) delete P;
	if(H!=NULL) delete H;
}





IteratorRankedDictID* RankedStringDictionaryRPDAC::findPrefix(uchar *str, uint strLen, size_t* numLocated)
{
	/*Search for the first and last buckets containing the prefix*/
	
	//find any bucket containing the prefix (using binary search)
	//initialize the limits of the actual part of the search
	uint left = 0;
	uint right = numBuckets-1;
	uint center = floor(numBuckets/2);
	bool found = false;
	//start the binary search
	while(!found && left!=right)
	{
		uint headerPosition = bucketsize*center + H->getField(center); //(bucketsize*center-1) gets the element inmidiatly before the center bucket
		int compare = rp->extractPrefixAndCompareDAC(headerPosition, str, strLen);
		
		if(compare > 0) //the searched prefix is to the left
		{
			right=center-1;
			center = left + floor((right-left+1)/2);
		}
		else
		{
			if(compare < 0) //the searched prefix is to the right (or in the same bucket)
			{
				left=center;
				center = left + floor((right-left+1)/2);
			}
			else //the header match the prefix
				found=true;
		}
	}
	size_t* ids; //array for storing the position in the RPDAC
	*numLocated = 0; //number of strings located with the prefix
	if(!found) //not found the prefix... Can be some or none strings with the prefix inside the bucket
	{
		ids = new size_t[bucketsize];
		
		//go through the bucket
		uint actBucketSize;
		if(center==numBuckets-1) actBucketSize = elements%bucketsize; //the actual is the last bucket
		else actBucketSize=bucketsize;
		for(uint i=1; i<=actBucketSize; i++)
		{
			uint actPos = bucketsize*center + i;
			uint comp = rp->extractPrefixAndCompareDAC(actPos, str, strLen);
			if(comp==0) //found an element with the prefix, so store its ID
			{
				ids[*numLocated] = actPos;
				*numLocated=*numLocated+1;
			}
		}

		if(*numLocated==0) //no string found
			return NULL;
	}
	else//found a header with the prefix
	{
		
		/*starting from the found bucket search left and right for the first and last bucket*/
		uint first = 0;
		uint last = numBuckets-1;
		
		ids = new size_t[(last-first+1)*bucketsize];
		

		//search the first bucket
		uint L = 0;
		uint R = center-1;
		uint C = floor((R-L+1)/2);
		while(L<R)
		{
			uint headerPosition = bucketsize*C + H->getField(C);
			int compare = rp->extractPrefixAndCompareDAC(headerPosition, str, strLen);
			if(compare == 0) //the first bucket is to the left
				R=C-1;
			else //the first bucket is to the right
			{
				L=C;
			}
			C=floor((R-L+1)/2)+L;
		}
		first = C;
		
		//search the last bucket
		L=center;
		R=numBuckets;
		C=floor((R-L+1)/2)+L;
		while(L<R)
		{
			uint headerPosition = bucketsize*C + H->getField(C);
			int compare = rp->extractPrefixAndCompareDAC(headerPosition, str, strLen);
			if(compare == 0) //the last bucket is to the right (or in the actual)
				L=C;
			else //the last bucket is to the left
				R=C-1;
				
			C=floor((R-L+1)/2)+L;
		}
		last = C;


		/*go element by element getting their position in the RPDAC (for the first and last bucket is necesary to check if the elements have the prefix)*/
		
		//first bucket
		for(uint i=1; i<=bucketsize; i++)
		{
			uint actPos = bucketsize*first + i;
			uint comp = rp->extractPrefixAndCompareDAC(actPos, str, strLen);
			if(comp==0) //found an element with the prefix, so store its ID
			{
				ids[*numLocated] = actPos;
				*numLocated=*numLocated+1;
			}
		}

		//the buckets between first and last (in this ones it's not necesary to compare if the strings have the prefix)
		for(uint actBucket=first+1; actBucket<last; actBucket++) //for each bucket
		{
			for(uint i=1; i<=bucketsize; i++) //for each element of the bucket
			{
				ids[*numLocated] = actBucket*bucketsize + i;
				*numLocated=*numLocated+1;
			}
		}

		//last bucket
		for(uint i=1; i<=bucketsize; i++)
		{
			uint actPos = bucketsize*last + i;
			uint comp = rp->extractPrefixAndCompareDAC(actPos, str, strLen);
			if(comp==0) //found an element with the prefix, so store its ID
			{
				ids[*numLocated] = actPos;
				*numLocated=*numLocated+1;
			}
		}
		
	}
	
	/*Create a non contiguous ID iterator*/
	IteratorRankedDictID* it = new IteratorRankedDictIDNoContiguous(ids, *numLocated);
	
    return it;
}





RankedPrefixNode ** RankedStringDictionaryRPDAC::createAuxiliarTree(uint first, uint last, uint *differentBits, uint *numNodes)
{
	first++; //in the tree buckets starts from 1
	last++; //in the tree buckets starts from 1
	
	//Find the common bits prefix of the first and last symbols, and get the number of different bits
	uint auxFirst=first;
	uint auxLast=last;
	*differentBits=0;
	while(auxFirst!=auxLast) //Find the not common bits
	{
		auxFirst >>= 1;
		auxLast >>= 1;
		*differentBits+=1;
	}
	
	//Obtain the levels array with the number of elements of each level (level 0 refers to the leaves)
	uint * levels = new uint[*differentBits]; 
	auxFirst=first;
	auxLast=last;
	*numNodes=0;
	for(uint i=0; i<*differentBits; i++)
	{
		levels[i]=auxLast-auxFirst+1;
		auxFirst=auxFirst>>1;
		auxLast=auxLast>>1;
		*numNodes+=levels[i];
	}
	
	//Create an array for storing the auxiliar tree
	RankedPrefixNode ** auxTree = new RankedPrefixNode*[*numNodes+1]; //the tree also stores the pivot node

	/*CREATE AND INITIALIZE THE TREE*/
	int father = levels[0];
	int pos=0;
	
	/*initialize the leaves*/
	uint symbol=first;
	auxTree[father]= new RankedPrefixNode(); //create an empty node for the first father
	for(uint i=0; i<levels[0]; i++)
	{
		//create the leaf node
		size_t ptr=1;
		auxTree[pos] = new RankedPrefixNode(ptr,father); //create the leaf node
		if((1 & symbol) !=0 ) //is a rightChild
		{
			auxTree[father]->setRightson(pos);
			//set right
			size_t right = P->selectInLevel( symbol, auxTree[pos]->getPtr(), 1 );
			auxTree[father]->setRight(right);
			
			if(i<levels[0]-1)//if it is not the last node of the level
			{
				father++;
				auxTree[father]= new RankedPrefixNode(); //create an empty node for the next father
			}	
		}
		else //is a left child
		{
			auxTree[father]->setLeftson(pos);
			//set left
			size_t left = P->selectInLevel( symbol, auxTree[pos]->getPtr(), 1 );
			auxTree[father]->setLeft(left);
		}
		
		pos++;
		symbol+=1;
	}
	
	
	/*initialize the internal nodes*/
	uint mask=1;
	for(uint i=1; i<*differentBits; i++) //for each level starting from down
	{		
		//create an empty node for the father of the first node
		father++;
		auxTree[father] = new RankedPrefixNode();
		
		symbol=first;
		mask <<= 1;
				
		for(uint j=0; j<levels[i]; j++) //for each element of the level
		{
			//it's not necesary to create the node (was created by the son)
			
			//set father
			auxTree[pos]->setFather(father);

			//set ptr
			size_t left = auxTree[pos]->getLeft();
			size_t right = auxTree[pos]->getRight(); 
			if(left==0) //there is no left son
				auxTree[pos]->setPtr(right);
			else if(right==0) //there is no right son
				auxTree[pos]->setPtr(left);
			else //there are both sons
			{
				if(left<right)
					auxTree[pos]->setPtr(left);
				else
					auxTree[pos]->setPtr(right);
			}
			
			
			if((mask & symbol) != 0) //is a right child
			{
				auxTree[father]->setRightson(pos); //point the father to the node
				//set right
				size_t right = P->selectInLevel( symbol, auxTree[pos]->getPtr(), i+1 );
				auxTree[father]->setRight(right);
				
				if(j<levels[i]-1) //if it is not the last node of the level
				{
					father++;
					auxTree[father]= new RankedPrefixNode(); //create an empty node for the next father 
				}
			}
			else //is a left child
			{
				auxTree[father]->setLeftson(pos);
				//set left
				size_t left = P->selectInLevel( symbol, auxTree[pos]->getPtr(), i+1 );
				auxTree[father]->setLeft(left);
			}
			
			symbol+= mask;
			pos++;
		}
	}
		
	/*set ptr in pivot node*/
	size_t left = auxTree[pos]->getLeft();
	size_t right = auxTree[pos]->getRight(); 
	if(left==0) //there is no left son
		auxTree[pos]->setPtr(right);
	else if(right==0) //there is no right son
		auxTree[pos]->setPtr(left);
	else //there are both sons
	{
		if(left<=right)
			auxTree[pos]->setPtr(left);
		else
			auxTree[pos]->setPtr(right);
	}
	delete [] levels;
	return auxTree;	
}




uint RankedStringDictionaryRPDAC::nextMidBucket(uint first, uint last, RankedPrefixNode ** auxTree, uint *inPos, uint numNodes)
{
	first++; //in the tree buckets starts from 1
	last++;	//in the tree buckets starts from 1
	
	//obtain the common bit prefix of first and last buckets
	uint auxFirst=first;
	uint auxLast=last;
	while(auxFirst!=auxLast) //Find the not common bits
	{
		auxFirst >>= 1;
		auxLast >>= 1;
	}
	uint bucket=auxFirst;
	uint bucketPos = numNodes; //initialize the bucket to the pivot node
	bool ended=false;
	while(!ended)
	{
		if(auxTree[bucketPos]->getLeft()==0 && auxTree[bucketPos]->getRight()==0) //if it has no sons is a leaf and we have ended
			if(auxTree[bucketPos]->getLeftSon()==-1 && auxTree[bucketPos]->getRightSon()==-1) //is a leaf
				ended=true;
			else //it is not a leaf, so does not exist more elements
				return 0;
		else //is an internal node
		{
			bucket<<=1;
			if(auxTree[bucketPos]->getLeft()==0) //it has no left son
			{
				bucketPos=auxTree[bucketPos]->getRightSon();
				bucket++;
			}
			else if(auxTree[bucketPos]->getRight()==0) //it has no right son
			{
				bucketPos=auxTree[bucketPos]->getLeftSon();
			}
			else //it has both sons
			{
				if(auxTree[bucketPos]->getLeft() > auxTree[bucketPos]->getRight())
				{
					bucketPos=auxTree[bucketPos]->getRightSon();
					bucket++;
				}
				else
				{
					bucketPos=auxTree[bucketPos]->getLeftSon();
				}
			}
		}
	}

	//get the inner position inside the bucket
	*inPos=auxTree[bucketPos]->getPtr();
	
	//update the node ptr
	if(*inPos<bucketsize)
		auxTree[bucketPos]->setPtr(*inPos+1);
	else
		auxTree[bucketPos]->setPtr(0);
	
	/*update the tree*/
	uint mask=1;
	int father;
	uint level=1;
	while(bucketPos < numNodes)
	{
		uint ptr = auxTree[bucketPos]->getPtr();
		father = auxTree[bucketPos]->getFather();
		if( (mask & bucket) != 0) //is a right son
		{
			uint right = P->selectInLevel(bucket, ptr, level );
			auxTree[father]->setRight(right);
			
			//update father ptr
			uint left = auxTree[father]->getLeft();
			if(left==0) //father has no left son
				auxTree[father]->setPtr(right);
			else //father has left son
			{
				if(left<=right)
					auxTree[father]->setPtr(left);
				else
					auxTree[father]->setPtr(right);
			}
		}
		else //is a left son
		{
			uint left = P->selectInLevel(bucket, ptr, level );
			auxTree[father]->setLeft(left);
			
			//update father ptr
			uint right = auxTree[father]->getRight();
			if(right==0) //father has no right son
				auxTree[father]->setPtr(left);
			else //father has right son
			{
				if(right<left)
					auxTree[father]->setPtr(right);
				else
					auxTree[father]->setPtr(left);
			}
		}
		
		bucketPos=father;
		mask <<= 1;
		level++;
	}

	
	return bucket;
}



uint RankedStringDictionaryRPDAC::nextIdFirstLastBucket(uchar *str, uint strLen, uint bucket, uint *startPoint)
{
	uint id=elements+1;
	bool found=false;
	while(*startPoint<bucketsize && !found)
	{
		uint pos = bucket*bucketsize+*startPoint+1;
		uint comp = rp->extractPrefixAndCompareDAC(pos, str, strLen);
		if(comp==0) //found element with the prefix
		{
			found = true;
			id = P->select(bucket+1, *startPoint+1)+1;
		}
		*startPoint+=1;
	}
	return id;
}




size_t ** RankedStringDictionaryRPDAC::topKprefix(uchar *str, uint strLen, uint n, size_t *numLocated)
{
	/*FIND FIRST AND LAST BUCKET*/
	//find any bucket containing the prefix (using binary search)
	//initialize the limits of the actual part of the search
	uint left = 0;
	uint right = numBuckets-1;
	uint center = floor(numBuckets/2);
	bool found = false;
	//start the binary search
	while(!found && left!=right)
	{
		uint headerPosition = bucketsize*center + H->getField(center); //(bucketsize*center-1) gets the element inmidiatly before the center bucket
		int compare = rp->extractPrefixAndCompareDAC(headerPosition, str, strLen);

		if(compare > 0) //the searched prefix is to the left
		{
			right=center-1;
			center = left + floor((right-left+1)/2);
		}
		else if(compare < 0) //the searched prefix is to the right (or in the same bucket)
		{
			left=center;
			center = left + floor((right-left+1)/2);
		}
		else //the header match the prefix
		{
			found=true;
		}

	}
	size_t** ids; //array for storing the position in the RPDAC (first row) and the id (second row)
	*numLocated = 0; //number of strings located with the prefix
	if(!found) //first and last bucket are the same
	{
		uint arrSize=min(bucketsize,n);
		ids = new size_t*[2];
		ids[0] = new size_t[arrSize];
		ids[1] = new size_t[arrSize];
		
		for(uint i=0;i<arrSize;i++) 
		{
			ids[0][i]=0;
			ids[1][i]=0;
		}
		
		//go through the bucket
		uint actBucketSize;
		if(center==numBuckets-1) //the actual is the last bucket
			actBucketSize = elements%bucketsize;
		else actBucketSize=bucketsize;
		uint i=1;
		while(*numLocated<n && i<=actBucketSize)
		{
			uint actPos = bucketsize*center + i;
			uint comp = rp->extractPrefixAndCompareDAC(actPos, str, strLen);
			if(comp==0) //found an element with the prefix, so store its ID
			{
				ids[0][*numLocated] = actPos;
				ids[1][*numLocated] = P->select(center+1,i)+1;
				*numLocated=*numLocated+1;
			}
			i++;
		}

		if(*numLocated==0) //no string found
			return NULL;
	}
	else//first and last buckets are different
	{
		/*starting from the found bucket search (binary search) left and right for the first and last bucket*/
		uint first = 0;
		uint last = numBuckets-1;
		
		ids = new size_t*[2];
		ids[0] = new size_t[n];
		ids[1] = new size_t[n];
		
		for(uint i=0;i<n;i++) 
		{
			ids[0][i]=0;
			ids[1][i]=0;
		}		

		//search the first bucket
		uint L = 0;
		uint R = center-1;
		uint C = floor((R-L+1)/2);
		while(L<R)
		{
			uint headerPosition = bucketsize*C + H->getField(C);
			int compare = rp->extractPrefixAndCompareDAC(headerPosition, str, strLen);
			if(compare == 0) //the first bucket is to the left
				R=C-1;
			else //the first bucket is to the right
			{
				L=C;
			}
			C=floor((R-L+1)/2)+L;
		}
		first = C;
		
		//search the last bucket
		L=center;
		R=numBuckets;
		C=floor((R-L+1)/2)+L;
		while(L<R)
		{
			uint headerPosition = bucketsize*C + H->getField(C);
			int compare = rp->extractPrefixAndCompareDAC(headerPosition, str, strLen);
			if(compare == 0) //the last bucket is to the right (or in the actual)
				L=C;
			else //the last bucket is to the left
				R=C-1;
				
			C=floor((R-L+1)/2)+L;
		}
		last = C;

		/*HAVING FIRST AND LAST BUCKET COMPARE THE FIRST OF FIRST BUCKET, LAST BUCKET AND MIDDLE BUCKETS*/

		uint firstBucketId, lastBucketId;
		if(first+2<last) /*there are more than one mid bucket*/
		{
			uint x;
			uint numNodes;
			RankedPrefixNode ** auxTree = createAuxiliarTree(first+1,last-1, &x, &numNodes);
			
			lowestIdBucket * bucketsAndIds = new lowestIdBucket[3];
						
			*numLocated=0;
			bool existElements=true;
			bucketsAndIds[0].startPoint=0;
			bucketsAndIds[0].id = nextIdFirstLastBucket(str, strLen, first, &bucketsAndIds[0].startPoint);
			bucketsAndIds[0].bucket=first;
			bucketsAndIds[0].position = first*bucketsize+bucketsAndIds[0].startPoint-1;
			bucketsAndIds[1].startPoint=0;
			bucketsAndIds[1].id = nextIdFirstLastBucket(str, strLen, last, &bucketsAndIds[1].startPoint);
			bucketsAndIds[1].bucket=last;
			bucketsAndIds[1].position = last*bucketsize+bucketsAndIds[1].startPoint-1;
			bucketsAndIds[2].startPoint=0;
			uint midBucket = nextMidBucket(first+1,last-1, auxTree, &bucketsAndIds[2].startPoint, numNodes);
			bucketsAndIds[2].id = P->select(midBucket,bucketsAndIds[2].startPoint)+1;
			bucketsAndIds[2].bucket=numBuckets+1;
			bucketsAndIds[2].position = (midBucket-1)*bucketsize+bucketsAndIds[2].startPoint;
			
				
			reorderLowestIdBucket(bucketsAndIds);
			
			while(*numLocated<n && existElements)
			{
				if(bucketsAndIds[0].id==elements+1 && bucketsAndIds[1].id==elements+1 && bucketsAndIds[2].id==elements+1) //no more elements
				{
					existElements=false;
				}
				else
				{
					ids[0][*numLocated] = bucketsAndIds[0].position;
					ids[1][*numLocated] = bucketsAndIds[0].id;
					if(bucketsAndIds[0].bucket==numBuckets+1)//the ID is from the middle buckets
					{
						midBucket = nextMidBucket(first+1,last-1, auxTree, &bucketsAndIds[0].startPoint, numNodes);
						if(midBucket>0)
						{
							bucketsAndIds[0].id = P->select(midBucket,bucketsAndIds[0].startPoint)+1;
							bucketsAndIds[0].position = (midBucket-1)*bucketsize+bucketsAndIds[0].startPoint;
						}
						else
							bucketsAndIds[0].id = elements+1;
					}
					else//the ID is from the first or last bucket
					{
						bucketsAndIds[0].id = nextIdFirstLastBucket(str, strLen, bucketsAndIds[0].bucket, &bucketsAndIds[0].startPoint);
						bucketsAndIds[0].position = bucketsAndIds[0].bucket*bucketsize+bucketsAndIds[0].startPoint-1;
					}
					*numLocated+=1;
					
					if(bucketsAndIds[1].id < bucketsAndIds[0].id && *numLocated<n) //the new id obtained is not the smallest
					{
						ids[0][*numLocated] = bucketsAndIds[1].position;
						ids[1][*numLocated] = bucketsAndIds[1].id;
						if(bucketsAndIds[1].bucket==numBuckets+1)//the ID is from the middle buckets
						{
							midBucket = nextMidBucket(first+1,last-1, auxTree, &bucketsAndIds[1].startPoint, numNodes);
							if(midBucket>0)
							{
								bucketsAndIds[1].id = P->select(midBucket,bucketsAndIds[1].startPoint)+1;
								bucketsAndIds[1].position = (midBucket-1)*bucketsize+bucketsAndIds[1].startPoint;
							}
							else
								bucketsAndIds[1].id = elements+1;
						}
						else//the ID is from the first or last bucket
						{
							bucketsAndIds[1].id = nextIdFirstLastBucket(str, strLen, bucketsAndIds[1].bucket, &bucketsAndIds[1].startPoint);
							bucketsAndIds[1].position = bucketsAndIds[1].bucket*bucketsize+bucketsAndIds[1].startPoint-1;
						}
						*numLocated+=1;
						reorderLowestIdBucket(bucketsAndIds);
					}
				}
			}
			for(uint i=0; i<=numNodes;i++)
				delete auxTree[i];
			delete [] auxTree;
			delete [] bucketsAndIds;
		}
		
		else if(first+1==last) /*there are no mid buckets*/
		{
			*numLocated=0;
			bool existElements=true;
			uint startPointFirst=0;
			uint startPointLast=0;
			firstBucketId = nextIdFirstLastBucket(str, strLen, first, &startPointFirst);
			lastBucketId = nextIdFirstLastBucket(str, strLen, last, &startPointLast);
			while(*numLocated<n && existElements)
			{
				if(firstBucketId==elements+1 && lastBucketId==elements+1) //no more elements with the prefix
				{
					existElements=false;
				}
				else
				{
					if(firstBucketId<=lastBucketId)
					{
						ids[0][*numLocated]=first*bucketsize+startPointFirst-1;
						ids[1][*numLocated]=firstBucketId;
						firstBucketId = nextIdFirstLastBucket(str, strLen, first, &startPointFirst);
					}
					else
					{
						ids[0][*numLocated]=last*bucketsize+startPointLast-1;
						ids[1][*numLocated]=lastBucketId;
						lastBucketId = nextIdFirstLastBucket(str, strLen, first, &startPointLast);
					}
					*numLocated+=1;
				}
			}
		}
		
		else /*there is only one mid bucket*/
		{
			
			lowestIdBucket * bucketsAndIds = new lowestIdBucket[3];
			
			uint midBucket = first+1;
			
			*numLocated=0;
			bool existElements=true;
			bucketsAndIds[0].startPoint=0;
			bucketsAndIds[0].id = nextIdFirstLastBucket(str, strLen, first, &bucketsAndIds[0].startPoint);
			bucketsAndIds[0].bucket=first;
			bucketsAndIds[0].position = first*bucketsize+bucketsAndIds[0].startPoint-1;
			bucketsAndIds[1].startPoint=0;
			bucketsAndIds[1].id = nextIdFirstLastBucket(str, strLen, last, &bucketsAndIds[1].startPoint);
			bucketsAndIds[1].bucket=last;
			bucketsAndIds[1].position = last*bucketsize+bucketsAndIds[1].startPoint-1;
			bucketsAndIds[2].startPoint=1;
			bucketsAndIds[2].id = P->select(midBucket+1,bucketsAndIds[2].startPoint)+1;
			bucketsAndIds[2].bucket=numBuckets+1;
			bucketsAndIds[2].position = (midBucket-1)*bucketsize+bucketsAndIds[2].startPoint;
			
			reorderLowestIdBucket(bucketsAndIds);
			
			while(*numLocated<n && existElements)
			{
				if(bucketsAndIds[0].id==elements+1 && bucketsAndIds[1].id==elements+1 && bucketsAndIds[2].id==elements+1) //no more elements
				{
					existElements=false;
				}
				else
				{
					ids[0][*numLocated]=bucketsAndIds[0].position;
					ids[1][*numLocated]=bucketsAndIds[0].id;
					if(bucketsAndIds[0].bucket==numBuckets+1)//the ID is from the middle buckets
					{
						if(bucketsAndIds[0].startPoint<=bucketsize)
						{
							bucketsAndIds[0].id = P->select(midBucket+1,bucketsAndIds[0].startPoint)+1;
							bucketsAndIds[0].position = (midBucket-1)*bucketsize+bucketsAndIds[0].startPoint;
						}
						else
							bucketsAndIds[0].id = elements+1;
					}
					else//the ID is from the first or last bucket
					{
						bucketsAndIds[0].id = nextIdFirstLastBucket(str, strLen, bucketsAndIds[0].bucket, &bucketsAndIds[0].startPoint);
						bucketsAndIds[0].position = bucketsAndIds[0].bucket*bucketsize+bucketsAndIds[0].startPoint-1;
					}
					*numLocated+=1;
					
					if(bucketsAndIds[1].id < bucketsAndIds[0].id && *numLocated<n) //the new id obtained is not the smallest
					{
						ids[0][*numLocated]=bucketsAndIds[1].position;
						ids[1][*numLocated]=bucketsAndIds[1].id;
						if(bucketsAndIds[1].bucket==numBuckets+1)//the ID is from the middle buckets
						{
							if(bucketsAndIds[0].startPoint<=bucketsize)
							{
								bucketsAndIds[1].id = P->select(midBucket+1,bucketsAndIds[1].startPoint)+1;
								bucketsAndIds[1].position = (midBucket-1)*bucketsize+bucketsAndIds[1].startPoint;
							}
							else
								bucketsAndIds[1].id = elements+1;
						}
						else//the ID is from the first or last bucket
						{
							bucketsAndIds[1].id = nextIdFirstLastBucket(str, strLen, bucketsAndIds[1].bucket, &bucketsAndIds[1].startPoint);
							bucketsAndIds[1].position = bucketsAndIds[1].bucket*bucketsize+bucketsAndIds[1].startPoint-1;
						}
						*numLocated+=1;
						reorderLowestIdBucket(bucketsAndIds);
					}
				}
			}
			delete [] bucketsAndIds;
		}

		
	}
	return ids;
}




size_t ** RankedStringDictionaryRPDAC::topKInterval(uint beginning, uint len, uint k)
{
	if(len<k) //if len<k we can only return len elements -> k=len
		k=len;
	
	size_t** ids = new size_t*[2];; //array for storing the position in the RPDAC (first row) and the id (second row)
	ids[0] = new size_t[k];
	ids[1] = new size_t[k];
//	for(uint i=0;i<k;i++) 
//	{
//		ids[0][i]=0;
//		ids[1][i]=0;
//	}
	
	//obtain the first and last buckets
	uint first = (beginning-1)/bucketsize;
	uint last = (beginning+len-2)/bucketsize;
	
	if(first==last) //first and last bucket are the same
	{	//just go through the bucket from the beginning until reach the k elements
		for(uint i=0; i<k; i++)
		{
			ids[0][i]=beginning+i;
		}
	}
	else//first and last buckets are different
	{
		/*COMPARE THE FIRST RANKED ELEMENT OF FIRST BUCKET, LAST BUCKET AND MIDDLE BUCKETS*/

		uint firstBucketId, lastBucketId;
		if(first+2<last) /*there are more than one mid bucket*/
		{
			uint x;
			uint numNodes;
			RankedPrefixNode ** auxTree = createAuxiliarTree(first+1,last-1, &x, &numNodes);
			
			lowestIdBucket * bucketsAndIds = new lowestIdBucket[3];
			
			uint lastBucketEnd = (beginning+len-2)%bucketsize+1;
			
			uint numLocated=0;
			bucketsAndIds[0].startPoint = (beginning-1)%bucketsize+1;
			bucketsAndIds[0].id = P->select(first+1, bucketsAndIds[0].startPoint)+1;
			bucketsAndIds[0].bucket=first+1;
			bucketsAndIds[0].position = beginning;
			
			
			bucketsAndIds[1].startPoint=1;
			bucketsAndIds[1].id = P->select(last+1, bucketsAndIds[1].startPoint)+1;
			bucketsAndIds[1].bucket=last+1;
			bucketsAndIds[1].position = last*bucketsize+1;
			
			bucketsAndIds[2].startPoint=0;
			uint midBucket = nextMidBucket(first+1,last-1, auxTree, &bucketsAndIds[2].startPoint, numNodes);
			bucketsAndIds[2].id = P->select(midBucket,bucketsAndIds[2].startPoint)+1;
			bucketsAndIds[2].bucket=numBuckets+1;
			bucketsAndIds[2].position = (midBucket-1)*bucketsize+bucketsAndIds[2].startPoint;
			
				
			reorderLowestIdBucket(bucketsAndIds);
			
			while(numLocated<k)
			{
				
				ids[0][numLocated] = bucketsAndIds[0].position;
				ids[1][numLocated] = bucketsAndIds[0].id;
				
				if(bucketsAndIds[0].bucket==numBuckets+1)//the ID is from the middle buckets
				{
					midBucket = nextMidBucket(first+1,last-1, auxTree, &bucketsAndIds[0].startPoint, numNodes);
					if(midBucket>0)
					{
						bucketsAndIds[0].id = P->select(midBucket,bucketsAndIds[0].startPoint)+1;
						bucketsAndIds[0].position = (midBucket-1)*bucketsize+bucketsAndIds[0].startPoint;
					}
					else
						bucketsAndIds[0].id = elements+1;
				}
				else//the ID is from the first or last bucket
				{
					if(bucketsAndIds[0].bucket==last+1) //last bucket
					{
						if(bucketsAndIds[0].startPoint>=lastBucketEnd)
							bucketsAndIds[0].id = elements+1;
						else
						{
							bucketsAndIds[0].startPoint++;
							bucketsAndIds[0].id = P->select(bucketsAndIds[0].bucket,bucketsAndIds[0].startPoint)+1;
							bucketsAndIds[0].position = (bucketsAndIds[0].bucket-1)*bucketsize + bucketsAndIds[0].startPoint;
						}
					}
					else if(bucketsAndIds[0].startPoint<bucketsize) //first bucket
					{
						bucketsAndIds[0].startPoint++;
						bucketsAndIds[0].id = P->select(bucketsAndIds[0].bucket,bucketsAndIds[0].startPoint)+1;
						bucketsAndIds[0].position = (bucketsAndIds[0].bucket-1)*bucketsize + bucketsAndIds[0].startPoint;
					}
					else
						bucketsAndIds[0].id = elements+1;
				}
				numLocated+=1;
				
				if(bucketsAndIds[1].id < bucketsAndIds[0].id && numLocated<k) //the new id obtained is not the smallest
				{
					ids[0][numLocated] = bucketsAndIds[1].position;
					ids[1][numLocated] = bucketsAndIds[1].id;
					if(bucketsAndIds[1].bucket==numBuckets+1)//the ID is from the middle buckets
					{
						midBucket = nextMidBucket(first+1,last-1, auxTree, &bucketsAndIds[1].startPoint, numNodes);
						if(midBucket>0)
						{
							bucketsAndIds[1].id = P->select(midBucket,bucketsAndIds[1].startPoint)+1;
							bucketsAndIds[1].position = (midBucket-1)*bucketsize+bucketsAndIds[1].startPoint;
						}
						else
							bucketsAndIds[1].id = elements+1;
					}
					else//the ID is from the first or last bucket
					{
						if(bucketsAndIds[1].bucket==last+1) //last bucket
						{
							if(bucketsAndIds[1].startPoint>=lastBucketEnd)
								bucketsAndIds[1].id = elements+1;
							else
							{
								bucketsAndIds[1].startPoint++;
								bucketsAndIds[1].id = P->select(bucketsAndIds[1].bucket,bucketsAndIds[1].startPoint)+1;
								bucketsAndIds[1].position = (bucketsAndIds[1].bucket-1)*bucketsize + bucketsAndIds[1].startPoint;
							}
						}
						else if(bucketsAndIds[1].startPoint<bucketsize) //first bucket
						{
							bucketsAndIds[1].startPoint++;
							bucketsAndIds[1].id = P->select(bucketsAndIds[1].bucket,bucketsAndIds[1].startPoint)+1;
							bucketsAndIds[1].position = (bucketsAndIds[1].bucket-1)*bucketsize + bucketsAndIds[1].startPoint;
						}
						else
							bucketsAndIds[1].id = elements+1;
					}
					numLocated++;
					reorderLowestIdBucket(bucketsAndIds);
				}

			}
			for(uint i=0; i<=numNodes;i++)
				delete auxTree[i];
			delete [] auxTree;
			delete [] bucketsAndIds;
		}
		
		else if(first+1==last) /*there are no mid buckets*/
		{
			uint lastBucketEnd = (beginning+len-2)%bucketsize+1;
			
			uint startPointFirst=(beginning-1)%bucketsize+1;
			uint startPointLast=1;
			
			firstBucketId = P->select(first+1, startPointFirst)+1;
			lastBucketId = P->select(last+1, startPointLast)+1;
			
			for(uint i=0; i<k; i++)
			{
				if(firstBucketId<=lastBucketId) //recover element from first bucket
				{
					ids[0][i]=first*bucketsize+startPointFirst-1;
					ids[1][i]=firstBucketId;
					startPointFirst++;
					if(startPointFirst<=bucketsize)
						firstBucketId = P->select(first+1, startPointFirst)+1;
					else
						firstBucketId = elements+1;
				}
				else  //recover element from last bucket
				{
					ids[0][i]=last*bucketsize+startPointLast;
					ids[1][i]=lastBucketId;
					startPointLast++;
					if(startPointLast<lastBucketEnd)
						lastBucketId = P->select(last+1, startPointLast)+1;
					else
						lastBucketId = elements+1;
				}
			}
		}
		
		else /*there is only one mid bucket*/
		{
			uint lastBucketEnd = (beginning+len-2)%bucketsize+1;
			lowestIdBucket * bucketsAndIds = new lowestIdBucket[3];
			uint midBucket = first+1;
			
			uint numLocated=0;
			bucketsAndIds[0].startPoint = (beginning-1)%bucketsize+1;
			bucketsAndIds[0].id = P->select(first+1, bucketsAndIds[0].startPoint)+1;
			bucketsAndIds[0].bucket=first+1;
			bucketsAndIds[0].position = beginning;
			
			bucketsAndIds[1].startPoint=1;
			bucketsAndIds[1].id = P->select(last+1, bucketsAndIds[1].startPoint)+1;
			bucketsAndIds[1].bucket=last+1;
			bucketsAndIds[1].position = last*bucketsize+1;
			
			bucketsAndIds[2].startPoint=1;
			bucketsAndIds[2].id = P->select(midBucket+1,bucketsAndIds[2].startPoint)+1;
			bucketsAndIds[2].bucket = midBucket+1;
			bucketsAndIds[2].position = midBucket*bucketsize+bucketsAndIds[2].startPoint;
			
			reorderLowestIdBucket(bucketsAndIds);
			
			
			while(numLocated<k) //while not have recovered the k elements
			{

				ids[0][numLocated] = bucketsAndIds[0].position;
				ids[1][numLocated] = bucketsAndIds[0].id;
				
				//update the recovered element
				if(bucketsAndIds[0].bucket==last+1 && bucketsAndIds[0].startPoint>=lastBucketEnd)
					bucketsAndIds[0].id = elements+1;
				else if(bucketsAndIds[0].startPoint<bucketsize)
				{
					bucketsAndIds[0].startPoint++;
					bucketsAndIds[0].id = P->select(bucketsAndIds[0].bucket,bucketsAndIds[0].startPoint)+1;
					bucketsAndIds[0].position = (bucketsAndIds[0].bucket-1)*bucketsize + bucketsAndIds[0].startPoint;
				}
				else
					bucketsAndIds[0].id = elements+1;
				
				numLocated++;
				
				if(bucketsAndIds[1].id < bucketsAndIds[0].id && numLocated<k) //the new id obtained is not the smallest
				{
					ids[0][numLocated] = bucketsAndIds[1].position;
					ids[1][numLocated] = bucketsAndIds[1].id;
				
					//update the recovered element
					if(bucketsAndIds[1].bucket==last+1 && bucketsAndIds[1].startPoint>lastBucketEnd)
						bucketsAndIds[1].id = elements+1;
					else if(bucketsAndIds[1].startPoint<bucketsize)
					{
						bucketsAndIds[1].startPoint++;
						bucketsAndIds[1].id = P->select(bucketsAndIds[1].bucket,bucketsAndIds[1].startPoint)+1;
						bucketsAndIds[1].position = (bucketsAndIds[1].bucket-1)*bucketsize + bucketsAndIds[1].startPoint;
					}
					else
						bucketsAndIds[1].id = elements+1;
				
					numLocated++;
					reorderLowestIdBucket(bucketsAndIds);
				}
			}
			delete [] bucketsAndIds;
		}

		
	}
	return ids;
}


void RankedStringDictionaryRPDAC::reorderLowestIdBucket(lowestIdBucket* array)
{
	lowestIdBucket aux;
	if(array[1].id < array[0].id)
	{
		aux=array[1];
		array[1]=array[0];
		array[0]=aux;
		if(array[2].id < array[1].id)
		{
			aux=array[2];
			array[2]=array[1];
			array[1]=aux;
			if(array[1].id < array[0].id)
			{
				aux=array[1];
				array[1]=array[0];
				array[0]=aux;
			}
		}
	}
	else if(array[2].id < array[1].id)
	{
		aux=array[2];
		array[2]=array[1];
		array[1]=aux;
		if(array[1].id < array[0].id)
		{
			aux=array[1];
			array[1]=array[0];
			array[0]=aux;
		}
	}
}










