#include <iostream>
#include <string.h>
#include <libcdsBasics.h>
#include <Permutation.h>
#include <vector>
#include <fstream>
#include "src/RankedStringDictionaryRPDAC.h"

using namespace std;
using namespace cds_utils;

int main(int argc, char* argv[])
{

    //ifstream in("/home/alvaro/diccionarios/geonames/geonamesRPDAC_4.RDRPDAC");
    ifstream in("/home/alvaro/diccionarios/geonames/geonamesPFC.RDS");


    if (in.good())
    {

        //create the dictionary
        in.seekg(0,ios_base::end);
        in.seekg(0,ios_base::beg);

        RankedStringDictionary *dict = RankedStringDictionary::load(in);
        in.close();


		// LOCATE/EXTRACT
		uchar* x;
		uint len;
		for(uint i=1;i<11;i++)
		{
			x = dict->extract(i,&len);
			cout<<dict->locate(x,len)<<" ----- "<<x<<endl;
			delete x;
		}
		
		
		//TOP-K
		IteratorRankedDictString * stringIter = dict->extractRankedInterval(2,14,20);
		
		while(stringIter->hasNext())
		{
			uint aux;
			uchar* asd = stringIter->next(&aux);
			cout<<dict->locate(asd,aux)<<" ----------------------------------- "<<asd<<endl;
		}
		
		cout<<endl<<endl<<endl<<" - "<<endl<<endl<<endl;
		
		
		delete stringIter;
		delete dict;
    }
    return 0;
}
