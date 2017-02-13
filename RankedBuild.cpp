/* RankedBuild.cpp
 * Copyright (C) 2016, Álvaro Alonso
 * all rights reserved.
 *
 * Script for building indexed ranked string dictionaries in compressed space.
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

 #ifndef _RANKEDBUILD_CPP
#define _RANKEDBUILD_CPP

#include <fstream>
#include <iostream>
using namespace std;

#include "src/RankedStringDictionary.h"
#include "src/iterators/IteratorDictStringPlain.h"
#include "src/utils/LogSequence.h"


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

void useBuild()
{

	cerr << endl;
	cerr << " ************************************************************************** " << endl;
	cerr << " *** BUILD script for indexing ranked string dictionaries in compressed space. *** " << endl;
	cerr << " ************************************************************************** " << endl;
	cerr << endl;
	cerr << " ----- ./RankedBuild <type> <parameters> <in> <out>" << endl;

	cerr << " type: 1 => Build SIMPLE RANKED PFC dictionary" << endl;
	cerr << " \t <compress> : 'p' for plain (uncompressed) representation; 'r' for RePair compression" << endl;
	cerr << " \t <bucketsize> : number of strings per bucket." << endl;
	cerr << " \t <in> : input file containing the set of '\\0'-delimited strings." << endl;
	cerr << " \t <out> : output file for storing the dictionary." << endl;
	cerr << endl;

	cerr << " type: 2 => Build SIMPLE RANKED HTFC dictionary" << endl;
	cerr << " \t <compress> : tecnique used for internal string compression." << endl;
	cerr << " \t              't' for HuTucker; 'h' for Huffman; 'r' for RePair compression." << endl;
	cerr << " \t <bucketsize> : number of strings per bucket." << endl;
	cerr << " \t <in> : input file containing the set of '\\0'-delimited strings." << endl;
	cerr << " \t <out> : output file for storing the dictionary." << endl;
	cerr << endl;

	cerr << " type: 3 => Build SIMPLE RANKED RPDAC dictionary" << endl;
	cerr << " \t <in> : input file containing the set of '\\0'-delimited strings." << endl;
	cerr << " \t <out> : output file for storing the dictionary." << endl;
	cerr << endl;
	
	cerr << " type: 4 => Build RANKED HASRPDAC dictionary" << endl;
	cerr << " \t <overhead> : hash table overhead (in percentage)." << endl;
	cerr << " \t <in> : input file containing the set of '\\0'-delimited strings." << endl;
	cerr << " \t <out> : output file for storing the dictionary." << endl;
	cerr << endl;
	
	
	cerr << " type: 5 => Build RANKED RPDAC dictionary" << endl;
	cerr << " \t <bucketsize> : number of strings per bucket." << endl;
	cerr << " \t <in> : input file containing the set of '\\0'-delimited strings." << endl;
	cerr << " \t <out> : output file for storing the dictionary." << endl;
	cerr << endl;
}


int
main(int argc, char* argv[])
{
	if (argc > 1)
	{
		int type = atoi(argv[1]);

		switch (type)
		{
			case 0:
			{
				cerr << "[TEST] Obtaining a friendly dictionary representation..." << endl;

				ifstream in(argv[2]);
				if (in.good())
				{
					in.seekg(0,ios_base::end);

					uint size = in.tellg()/sizeof(uchar);
					in.seekg(0,ios_base::beg);

					uchar* dict = loadValue<uchar>(in, size);
					in.close();

					cout << "File of: " << size << " bytes loaded" << endl;

					for (size_t i=0; i<size; i++)
						if (dict[i] == '\n') dict[i] = '\0';

					string filename = string(argv[2])+string(".strdic");
					ofstream out((char*)filename.c_str());
					saveValue(out, dict, size);
					out.close();

					delete [] dict;
				}
				else checkFile();

				break;
			}

			case 1: //simplePFC
			{
				if (argc != 6) { useBuild(); break; }

				ifstream in(argv[4]);
				if (in.good())
				{
					in.seekg(0,ios_base::end);
					uint lenStr = in.tellg()/sizeof(uchar);
					in.seekg(0,ios_base::beg);

					uint bucketsize = atoi(argv[3]);

					uchar *str = loadValue<uchar>(in, lenStr);
					in.close();

					IteratorDictString *it = new IteratorDictStringPlain(str, lenStr);
					RankedStringDictionary *dict = NULL;
					string filename = string(argv[5]);

                    dict = new RankedStringDictionarySimple(it, bucketsize, 1, argv[2][0]);
                    filename += string(".RDS");
					ofstream out((char*)filename.c_str());
					dict->save(out);
					out.close();
					delete dict;
					delete it;
				}
				else checkFile();

				break;
			}

			case 2: //simpleHTFC
			{
				if (argc != 6) { useBuild(); break; }

				ifstream in(argv[4]);
				if (in.good())
				{
					in.seekg(0,ios_base::end);
					uint lenStr = in.tellg()/sizeof(uchar);
					in.seekg(0,ios_base::beg);

					uint bucketsize = atoi(argv[3]);

					uchar *str = loadValue<uchar>(in, lenStr);
					in.close();

					IteratorDictString *it = new IteratorDictStringPlain(str, lenStr);
					RankedStringDictionary *dict = NULL;
					string filename = string(argv[5]);

                    dict = new RankedStringDictionarySimple(it, bucketsize, 2, argv[2][0]);
                    filename += string(".RDS");

					ofstream out((char*)filename.c_str());
					dict->save(out);
					out.close();
					delete dict;
					delete it;
				}
				else checkFile();

				break;
			}

			case 3: //simpleRPDAC
			{
				if (argc != 4) { useBuild(); break; }

				ifstream in(argv[2]);
				if (in.good())
				{
					in.seekg(0,ios_base::end);
					uint lenStr = in.tellg()/sizeof(uchar);
					in.seekg(0,ios_base::beg);

					uchar *str = loadValue<uchar>(in, lenStr);
					in.close();

					IteratorDictString *it = new IteratorDictStringPlain(str, lenStr);
					RankedStringDictionary *dict = NULL;
					string filename = string(argv[3]);

					uchar aux = ' ';
					uint auxbucketsize = 0;
                    dict = new RankedStringDictionarySimple(it, auxbucketsize, 3, aux);
                    filename += string(".RDS");

					ofstream out((char*)filename.c_str());
					dict->save(out);
					out.close();
					delete dict;
					delete it;
				}
				else checkFile();

				break;
			}
			
			case 4: //HASHRPDAC
			{
				if (argc != 5) { useBuild(); break; }
				ifstream in(argv[3]);
				if (in.good())
				{
					in.seekg(0,ios_base::end);
					uint lenStr = in.tellg()/sizeof(uchar);
					in.seekg(0,ios_base::beg);
					uchar *str = loadValue<uchar>(in, lenStr);
					in.close();
					IteratorDictString *it = new IteratorDictStringPlain(str, lenStr);
					RankedStringDictionary *dict = NULL;
					string filename = string(argv[4]);
					uint overhead = atoi(argv[2]);

					uchar aux = ' ';
                    dict = new RankedStringDictionarySimple(it, overhead, 4, aux);
                    filename += string(".RDS");
					ofstream out((char*)filename.c_str());
					dict->save(out);
					out.close();
					delete dict;
					delete it;
				}
				else checkFile();

				break;
			}
			
			case 5: //RPDAC
			{
				if (argc != 5) { useBuild(); break; }
				ifstream in(argv[3]);
				if (in.good())
				{
					in.seekg(0,ios_base::end);
					uint lenStr = in.tellg()/sizeof(uchar);
					in.seekg(0,ios_base::beg);
					uchar *str = loadValue<uchar>(in, lenStr);
					in.close();
					IteratorDictString *it = new IteratorDictStringPlain(str, lenStr);
					RankedStringDictionary *dict = NULL;
					string filename = string(argv[4]);
					uint bucketsize = atoi(argv[2]);

                    dict = new RankedStringDictionaryRPDAC(it, bucketsize);
                    filename += string(".RDRPDAC");
					ofstream out((char*)filename.c_str());
					dict->save(out);
					out.close();
					delete dict;
					delete it;
				}
				else checkFile();

				break;
			}

			default:
			{
				useBuild();
				break;
			}
		}
	}
	else
	{
		useBuild();
	}
}

#endif  /* _RANKEDBUILD_CPP */

