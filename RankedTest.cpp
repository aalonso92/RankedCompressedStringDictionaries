/* RankedTest.cpp
 * Copyright (C) 2016, Álvaro Alonso
 * all rights reserved.
 *
 * Script for testing string dictionaries built with the library of Compressed
 * String Dictionaries (libCSD).
- *
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
 
#ifndef _RANKEDTEST_CPP
#define _RANKEDTEST_CPP

#include <fstream>
#include <iostream>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "src/RankedStringDictionary.h"

#define RUNS 5

void checkDict()
{
	cerr << endl;
	cerr << " ******************************************************************************** " << endl;
	cerr << " *** Checks the given file because it does not represent any valid dictionary *** " << endl;
	cerr << " ******************************************************************************** " << endl;
	cerr << endl;
}

void checkFile()
{
	cerr << endl;
	cerr << " ****************************************************************** " << endl;
	cerr << " *** Checks the given path because it does not contain any file *** " << endl;
	cerr << " ****************************************************************** " << endl;
	cerr << endl;
}


void useTest()
{
	cerr << endl;
	cerr << " ******************************************************************************** " << endl;
	cerr << " *** Test script for the library of Compressed String Dictionaries (libCSD). *** " << endl;
	cerr << " ******************************************************************************** " << endl;
	cerr << endl;
	cerr << " ----- test <mode> <opt> <in> <file>" << endl;
	cerr << endl;
	cerr << " <mode> r : Run the given test." << endl;
	cerr << "    <opt> l : LOCATE test." << endl;
	cerr << "    <opt> e : EXTRACT test." << endl;
	cerr << "    <opt> pl : LOCATE PREFIX test." << endl;
	cerr << "    <opt> pe : EXTRACT PREFIX test." << endl;
	cerr << "    <opt> pkl : TOP K LOCATE PREFIX test (K = 2, 4, 8, 16, 32, 64 & 128)." << endl;
	cerr << "    <opt> pke : TOP K EXTRACT PREFIX test (K = 2, 4, 8, 16, 32, 64 & 128)." << endl;
	cerr << "    <opt> i : TOP K EXTRACT INTERVAL test (K = 2, 4, 8, 16, 32, 64 & 128)." << endl;
	cerr << "    <opt> sl : LOCATE SUBSTRING test." << endl;
	cerr << "    <opt> se : EXTRACT SUBSTRING test." << endl;
	cerr << " <mode> g : Generate the basic testbed." << endl;
	cerr << "    <opt> number of patterns to be generated." << endl;
	cerr << " <mode> p : Generate the prefix testbed." << endl;
	cerr << "    <opt> mean string length." << endl;
	cerr << " <mode> i : Generate the interval testbed." << endl;
	cerr << "    <opt> interval length." << endl;
	cerr << " <mode> s : Generate the substring testbed." << endl;
	cerr << "    <opt> mean string length." << endl;
	cerr << " <in> : input file containing the compressed string dictionary." << endl;
	cerr << " <file> : file from which the patterns are loaded or in which are saved." << endl;
	cerr << endl;
}

void runLocate(RankedStringDictionary *dict, char* in)
{
	ifstream inStrings(in);

	vector<uchar*> strings;
	vector<uint> lengths;
	uint maxlength = dict->maxLength()+1;

	while (true)
	{
		uchar *str = new uchar[maxlength+1];
		inStrings.getline((char*)str, maxlength);
		uint len = strlen((char*)str);

		if (len == 0) { delete [] str; break; }

		strings.push_back(str);
		lengths.push_back(len);

	}

	inStrings.close();

	uint patterns = strings.size();
	double t0, t1, total=0;

	for (uint i=1; i<=RUNS; i++)
	{
		t0 = getTime ();
		for (uint j=0; j<patterns; j++)
			dict->locate(strings[j], lengths[j]);

		t1 = (getTime () - t0);
		//cout << (t1*SEC_TIME_DIVIDER) << " ";
		total += t1;

		sleep(5);
	}

	double avgrun = total/RUNS;
	double avgpattern = avgrun/patterns;

//	cout << dict->getSize() << ";";
//	cout << ";;;" << (total*SEC_TIME_DIVIDER);
//	cout << ";;;" << (avgrun*SEC_TIME_DIVIDER);
//	cout << (avgpattern*MCSEC_TIME_DIVIDER) << " " << MCSEC_TIME_UNIT << endl;
	cout << (avgpattern*MCSEC_TIME_DIVIDER) << endl;
	
	for (uint i=0; i<patterns; i++) delete [] strings[i];
}

void runExtract(RankedStringDictionary *dict, char* in)
{
	ifstream inIds(in);
	vector<uint> ids;
	char line[256];

	while (true)
	{
		inIds.getline(line, 256);
		size_t len = strlen(line);

		if (len == 0) break;

		ids.push_back(atoi(line));
	}

	uint patterns = ids.size();
	double t0, t1, total=0;
	uint strLen;

	for (uint i=1; i<=RUNS; i++)
	{
		t0 = getTime ();

		for (uint j=1; j<patterns; j++)
		{
			uchar *str = dict->extract(ids[j], &strLen);
			delete [] str;
		}

		t1 = (getTime () - t0);
		//cout << (t1*SEC_TIME_DIVIDER) << " ";
		total += t1;

		sleep(5);
	}

	double avgrun = total/RUNS;
	double avgpattern = avgrun/patterns;

	//cout << dict->getSize() << ";";
	//cout << ";;;" << (total*SEC_TIME_DIVIDER);
	//cout << ";;;" << (avgrun*SEC_TIME_DIVIDER);
	//cout << ";;;" << (avgpattern*MCSEC_TIME_DIVIDER) << " " << MCSEC_TIME_UNIT << endl;
	cout << avgpattern*MCSEC_TIME_DIVIDER << endl;
}

void runLocatePrefix(RankedStringDictionary *dict, char* in)
{
	ifstream inStrings(in);

	vector<uchar*> strings;
	vector<uint> lengths;
	uint maxlength = dict->maxLength();

	while (true)
	{
		uchar *str = new uchar[maxlength+1];
		inStrings.getline((char*)str, maxlength);
		uint len = strlen((char*)str);

		if (len == 0) { delete [] str; break; }

		strings.push_back(str);
		lengths.push_back(len);

	}

	inStrings.close();

	uint patterns = strings.size();
	double t0, t1, total=0;
	size_t located;

	for (uint i=1; i<=RUNS; i++)
	{
		located = 0;
		t0 = getTime ();

		for (uint j=0; j<patterns; j++)
		{
			IteratorRankedDictID *it = dict->locatePrefix(strings[j], lengths[j]);
			while (it->hasNext())
			{
				it->next();
				located++;
			}
			
			delete it;
		}

		t1 = (getTime () - t0);
		cout << (t1*SEC_TIME_DIVIDER) << " ";
		total += t1;

		sleep(5);
	}

	double avgrun = total/RUNS;
	double avgpattern = avgrun/located;

	cout << ";;;" << (total*SEC_TIME_DIVIDER);
	cout << ";;;" << (avgrun*SEC_TIME_DIVIDER);
	cout << ";;;" << (avgpattern*MCSEC_TIME_DIVIDER) << " " << MCSEC_TIME_UNIT << endl;
	cout << (avgpattern*MCSEC_TIME_DIVIDER) << "  " << located << endl;

	for (uint i=0; i<patterns; i++) delete [] strings[i];
}

void runExtractPrefix(RankedStringDictionary *dict, char* in)
{
	ifstream inStrings(in);

	vector<uchar*> strings;
	vector<uint> lengths;
	uint maxlength = dict->maxLength();
	while (true)
	{
		uchar *str = new uchar[maxlength+1];
		inStrings.getline((char*)str, maxlength);
		uint len = strlen((char*)str);

		if (len == 0) { delete [] str; break; }

		strings.push_back(str);
		lengths.push_back(len);

	}
	uint patterns = strings.size();
	double t0, t1, total=0;
	size_t extracted;
	for (uint i=1; i<=RUNS; i++)
	{
		extracted = 0;
		t0 = getTime ();

		for (uint j=0; j<patterns; j++)
		{
			IteratorRankedDictString *it = dict->extractPrefix(strings[j], lengths[j]);

			while (it->hasNext())
			{
				uint strLen;
				uchar *str = it->next(&strLen);
				if(str!=NULL) delete [] str;
				extracted++;
			}
			delete it;
		}

		t1 = (getTime () - t0);
		cout << (t1*SEC_TIME_DIVIDER) << " ";
		total += t1;

		sleep(5);
	}

	double avgrun = total/RUNS;
	double avgpattern = avgrun/extracted;

	cout << ";;;" << (total*SEC_TIME_DIVIDER);
	cout << ";;;" << (avgrun*SEC_TIME_DIVIDER);
	cout << ";;;" << (avgpattern*MCSEC_TIME_DIVIDER) << " " << MCSEC_TIME_UNIT << endl;
	cout << " " << (avgpattern*MCSEC_TIME_DIVIDER) << "  " << extracted << endl;

	for (uint i=0; i<patterns; i++) delete [] strings[i];
}

void runTopKLocatePrefix(RankedStringDictionary *dict, char* in)
{
	ifstream inStrings(in);

	vector<uchar*> strings;
	vector<uint> lengths;
	uint maxlength = dict->maxLength();

	while (true)
	{
		uchar *str = new uchar[maxlength+1];
		inStrings.getline((char*)str, maxlength);
		uint len = strlen((char*)str);

		if (len == 0) { delete [] str; break; }

		strings.push_back(str);
		lengths.push_back(len);

	}

	uint patterns = strings.size();
	double t0, t1, total=0;
	size_t located;

	//Repeat for k=2,4,8,16,32,64,128
	uint k=1;
	while(k<=128)
	{
		k*=2;
		for (uint i=1; i<=RUNS; i++)
		{
			located = 0;
			t0 = getTime ();

			for (uint j=0; j<patterns; j++)
			{
				IteratorRankedDictID *it = dict->locateRankedPrefix(strings[j], lengths[j], k);

				while (it->hasNext())
				{
					it->next();
					located++;
				}

				delete it;
			}

			t1 = (getTime () - t0);
			cout << (t1*SEC_TIME_DIVIDER) << " ";
			total += t1;

			sleep(5);
		}

		double avgrun = total/RUNS;
		double avgpattern = avgrun/located;

		cout << "K="<<k<<endl;
		cout << ";;;" << (total*SEC_TIME_DIVIDER);
		cout << ";;;" << (avgrun*SEC_TIME_DIVIDER);
		cout << ";;;" << (avgpattern*MCSEC_TIME_DIVIDER) << " " << MCSEC_TIME_UNIT << endl;
		cout << " " << (avgpattern*MCSEC_TIME_DIVIDER) << "  " << located << endl;
	}
	
	for (uint i=0; i<patterns; i++) delete [] strings[i];
}


void runTopKExtractPrefix(RankedStringDictionary *dict, char* in)
{
	ifstream inStrings(in);

	vector<uchar*> strings;
	vector<uint> lengths;
	uint maxlength = dict->maxLength();

	while (true)
	{
		uchar *str = new uchar[maxlength+1];
		inStrings.getline((char*)str, maxlength);
		uint len = strlen((char*)str);

		if (len == 0) { delete [] str; break; }

		strings.push_back(str);
		lengths.push_back(len);

	}

	uint patterns = strings.size();
	double t0, t1, total=0;
	size_t extracted;
	
	//Repeat for k=2,4,8,16,32,64,128
	uint k=1;
	while(k<128)
	{
		k*=2;
		for (uint i=1; i<=RUNS; i++)
		{
			extracted = 0;
			t0 = getTime ();

			for (uint j=0; j<patterns; j++)
			{

				IteratorRankedDictString *it = dict->extractRankedPrefix(strings[j], lengths[j], k);
			
				while (it->hasNext())
				{
					uint strLen;
					uchar *str = it->next(&strLen);
					delete [] str;
					extracted++;
				}
				
				delete it;
			}

			t1 = (getTime () - t0);
			//cout << (t1*SEC_TIME_DIVIDER) << " ";
			total += t1;

			sleep(5);
		}
		
		double avgrun = total/RUNS;
		double avgpattern = avgrun/extracted;

		cout << "K="<<k;
		//cout << ";;;" << (total*SEC_TIME_DIVIDER);
		//cout << ";;;" << (avgrun*SEC_TIME_DIVIDER);
		cout << ";;;" << (avgpattern*MCSEC_TIME_DIVIDER) << " ; " << MCSEC_TIME_UNIT << " ; extracted ; "<<extracted<<endl;
		//cout << " " << (avgpattern*MCSEC_TIME_DIVIDER) << "  " << extracted << endl<<endl;
	}
	
	for (uint i=0; i<patterns; i++) delete [] strings[i];
}


void runTopKExtractInterval(RankedStringDictionary *dict, char* in)
{
	ifstream inStrings(in);

	vector<uint> beginnings;
	vector<uint> lengths;
	uint maxlength = dict->maxLength();

	char *intervalLenStr = new char[maxlength+1];
	inStrings.getline((char*)intervalLenStr, maxlength);
	uint intervalLen = atoi(intervalLenStr);
	delete [] intervalLenStr;
	while (true)
	{
		char *str = new char[maxlength+1];
		inStrings.getline((char*)str, maxlength);
		uint len = strlen((char*)str);

		if (len == 0) { delete [] str; break; }

		beginnings.push_back(atoi(str));
		lengths.push_back(len);
		delete [] str;

	}

	uint patterns = beginnings.size();
	double t0, t1, total=0;
	size_t extracted;
	
	//Repeat for k=2,4,8,16,32,64,128
	uint k=1;
	while(k<128)
	{
		k*=2;
		for (uint i=1; i<=RUNS; i++)
		{
			extracted = 0;
			t0 = getTime ();

			for (uint j=0; j<patterns; j++)
			{

				IteratorRankedDictString *it = dict->extractRankedInterval(beginnings[j], intervalLen, k);
			
				while (it->hasNext())
				{
					uint strLen;
					uchar *str = it->next(&strLen);
					delete [] str;
					extracted++;
				}
				
				delete it;
			}

			t1 = (getTime () - t0);
			//cout <<(t1*SEC_TIME_DIVIDER) << " ";
			total += t1;

			sleep(5);
		}
		
		double avgrun = total/RUNS;
		double avgpattern = avgrun/extracted;
		
		
		cout <<k<<"\t"<<avgpattern*MCSEC_TIME_DIVIDER<<endl;
		//cout <<k<<"; extracted = "<<extracted<<"; "<<avgpattern*MCSEC_TIME_DIVIDER<<endl;
		//cout << ";;;" << (total*SEC_TIME_DIVIDER);
		//cout << ";;;" << (avgrun*SEC_TIME_DIVIDER);
		//cout << ";;;" << (avgpattern*MCSEC_TIME_DIVIDER) << " ; " << MCSEC_TIME_UNIT << " ; extracted ; "<<extracted<<endl;
		//cout << " " << (avgpattern*MCSEC_TIME_DIVIDER) << "  " << extracted << endl<<endl;
	}
}


void runLocateSubstring(RankedStringDictionary *dict, char* in)
{
	ifstream inStrings(in);

	vector<uchar*> strings;
	vector<uint> lengths;
	uint maxlength = dict->maxLength();

	while (true)
	{
		uchar *str = new uchar[maxlength+1];
		inStrings.getline((char*)str, maxlength);
		uint len = strlen((char*)str);

		if (len == 0) { delete [] str; break; }

		strings.push_back(str);
		lengths.push_back(len);

	}

	inStrings.close();

	uint patterns = strings.size();
	double t0, t1, total=0;
	size_t located;

	for (uint i=1; i<=RUNS; i++)
	{
		located = 0;
		t0 = getTime ();

		for (uint j=0; j<patterns; j++)
		{
			IteratorRankedDictID *it = dict->locateSubstr(strings[j], lengths[j]);

			while (it->hasNext())
			{
				it->next();
				located++;
			}

			delete it;
		}

		t1 = (getTime () - t0);
		cout << (t1*SEC_TIME_DIVIDER) << " ";
		total += t1;

		sleep(5);
	}


	cout << endl;
	double avgrun = total/RUNS;
	cout << dict->getSize() << ";" << avgrun << ";" << located << endl;

	for (uint i=0; i<patterns; i++) delete [] strings[i];
}

void runExtractSubstring(RankedStringDictionary *dict, char* in)
{
	ifstream inStrings(in);

	vector<uchar*> strings;
	vector<uint> lengths;
	uint maxlength = dict->maxLength();

	uchar *str = new uchar[maxlength+1];

	while (true)
	{
		inStrings.getline((char*)str, maxlength);
		uint len = strlen((char*)str);

		if (len == 0) { delete [] str; break; }

		uchar *tmp = new uchar[len+1];
		strcpy((char*)tmp, (char*)str);

		strings.push_back(tmp);
		lengths.push_back(len);
	}

	uint patterns = strings.size();
	double t0, t1, total=0;
	size_t extracted;

	for (uint i=1; i<=RUNS; i++)
	{
		extracted = 0;
		t0 = getTime ();

		for (uint j=0; j<patterns; j++)
		{
			IteratorRankedDictString *it = dict->extractSubstr(strings[j], lengths[j]);

			while (it->hasNext())
			{
				uint strLen;
				uchar *str = it->next(&strLen);
				delete [] str;
				extracted++;
			}

			delete it;
		}

		t1 = (getTime () - t0);
		cout << (t1*SEC_TIME_DIVIDER) << " ";
		total += t1;

		sleep(5);
	}

	cout << endl;
	double avgrun = total/RUNS;
	cout << dict->getSize() << ";" << avgrun << ";" << extracted << endl;

	for (uint i=0; i<patterns; i++) delete [] strings[i];
}

void generate(RankedStringDictionary *dict, uint patterns, char* out)
{
	srand (time(NULL));
	uint num = dict->numElements();

	vector<size_t> ids(patterns);

	for (uint i=0; i<patterns; i++) ids[i] = rand()%num+1;

	string name = string(out)+string(".ids");
	ofstream outIds((char*)name.c_str());

	name = string(out)+string(".strings");
	ofstream outStrings((char*)name.c_str());
	for (uint i=0; i<patterns; i++)
	{
		outIds << ids[i] << endl;

		uint strLen;
		uchar *str = dict->extract(ids[i], &strLen);
		outStrings << str << endl;

		delete [] str;
	}

	outIds.close();
	outStrings.close();
}

void generateP(RankedStringDictionary *dict, uint mean, char* out)
{
	srand (time(NULL));
	uint num = dict->numElements();

	uint PREFIXES = 100000;
	vector<size_t> ids(PREFIXES);
	for (uint i=0; i<PREFIXES; i++) ids[i] = rand()%num+1;

	vector<uint> lens;

	lens.push_back(0.4*mean);
	lens.push_back(0.5*mean);
	lens.push_back(0.6*mean);
	lens.push_back(0.7*mean);
	lens.push_back(0.8*mean);
	lens.push_back(0.9*mean);
	lens.push_back(1.0*mean);
	lens.push_back(1.1*mean);
	lens.push_back(1.2*mean);
	lens.push_back(1.3*mean);

	for (uint i=0; i<=9; i++)
	{
		char len[10];
		sprintf(len, "%d", lens[i]);
		string name = string(out)+string(".")+string(len)+string(".prefixes");

		ofstream outPrefixes((char*)name.c_str());

		for (uint j=0; j<PREFIXES; j++)
		{
			uint strLen;
			uchar *str;

			do
			{
				str = dict->extract(ids[j], &strLen);
				ids[j] = rand()%num+1;
			}
			while (strlen((char*)str) < lens[i]);

			str[lens[i]] = '\0';
			outPrefixes << str << endl;

			delete [] str;
		}

		outPrefixes.close();
	}
}


void generateI(RankedStringDictionary *dict, uint intervalLen, char* out)
{
	srand (time(NULL));
	uint num = dict->numElements();

	uint INTERVALS = 100000;
	vector<size_t> ids(INTERVALS);
	for (uint i=0; i<INTERVALS; i++) 
		ids[i] = rand()%(num-intervalLen)+1;

	char len[10];
	sprintf(len, "%d", intervalLen);
	string name = string(out)+string(".")+string(len)+string(".intervals");

	ofstream outPrefixes((char*)name.c_str());
	outPrefixes << intervalLen << endl;

	for (uint i=0; i<INTERVALS; i++)
		outPrefixes << ids[i] << endl;
	
	outPrefixes.close();
}

void generateS(RankedStringDictionary *dict, uint mean, char* out)
{
	srand (time(NULL));
	uint num = dict->numElements();

	uint SUBSTRINGS = 100000;
	vector<size_t> ids(SUBSTRINGS);
	vector<size_t> positions(SUBSTRINGS, 0);
	for (uint i=0; i<SUBSTRINGS; i++) ids[i] = (rand()%num)+1;

	vector<uint> lens;

	if (mean > 20) lens.push_back(0.05*mean);

	lens.push_back(0.1*mean);
	lens.push_back(0.15*mean);
	lens.push_back(0.2*mean);
	lens.push_back(0.25*mean);
	lens.push_back(0.3*mean);
	lens.push_back(0.35*mean);
	lens.push_back(0.4*mean);
	lens.push_back(0.45*mean);
	lens.push_back(0.5*mean);
	lens.push_back(0.55*mean);
	lens.push_back(0.6*mean);
	lens.push_back(0.65*mean);
	lens.push_back(0.7*mean);
	lens.push_back(0.75*mean);

	uint max = lens[lens.size()-1];

	for (uint i=0; i<lens.size(); i++)
	{
		char len[10];
		sprintf(len, "%d", lens[i]);
		string name = string(out)+string(".")+string(len)+string(".substrings");

		ofstream outSubstrings((char*)name.c_str());

		for (uint j=0; j<SUBSTRINGS; j++)
		{
			uint strLen;
			uchar *str;

			while (true)
			{
				str = dict->extract(ids[j], &strLen);

				if (strLen >= 3*(max+2)) break;
				else
				{
					delete [] str;
					ids[j] = rand()%num+1;
				}
			}

			if (positions[j] == 0) positions[j] = rand()%(strLen-(max+2));

			for (uint k=0; k<lens[i]; k++) outSubstrings << str[k+positions[j]];
			outSubstrings  << endl;
			delete [] str;
		}

		outSubstrings.close();
	}
}

int
main(int argc, char* argv[])
{
	if (argc >= 5)
	{
		ifstream in(argv[3]);
		if (in.good())
		{
			RankedStringDictionary *dict = RankedStringDictionary::load(in);
			in.close();
			if (dict == NULL) { checkDict(); exit(0); }

			char mode = argv[1][0];

			switch (mode)
			{
				case 'r':
				{
					uchar opt = argv[2][0];

					//cout << "RUNNING " << RUNS << " independent runs" << endl;

					switch (opt)
					{
						case 'l':
						{
							runLocate(dict, argv[4]);
							break;
						}

						case 'e':
						{
							runExtract(dict, argv[4]);
							break;
						}

						case 'p':
						{
							if (argv[2][1] == 'l')
								runLocatePrefix(dict, argv[4]);
							else if(argv[2][1] == 'e')
								runExtractPrefix(dict, argv[4]);
							else if(argv[2][2] == 'l')
								runTopKLocatePrefix(dict,argv[4]);
							else
								runTopKExtractPrefix(dict,argv[4]);

							break;
						}
						case 'i':
						{
							runTopKExtractInterval(dict, argv[4]);
							break;
						}

						case 's':
						{
							if (argv[2][1] == 'l')
								runLocateSubstring(dict, argv[4]);
							else
								runExtractSubstring(dict, argv[4]);

							break;
						}

						default:
						{
							useTest();
							break;
						}
					}

					break;
				}

				case 'g':
				{
					uint patterns = atoi(argv[2]);
					generate(dict, patterns, argv[4]);
					break;
				}

				case 'p':
				{
					uint mean = atoi(argv[2]);
					generateP(dict, mean, argv[4]);
					break;
				}
				
				case 'i':
				{
					uint intervLen = atoi(argv[2]);
					generateI(dict, intervLen, argv[4]);
					break;
				}

				case 's':
				{
					uint mean = atoi(argv[2]);
					generateS(dict, mean, argv[4]);
					break;
				}

				default:
				{
					useTest();
					break;
				}


			}

			delete dict;
		}
		else { checkFile(); exit(0); }
	}
	else useTest();
}

#endif  /* _TEST_CPP */

