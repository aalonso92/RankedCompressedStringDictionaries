/* Check.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Script for checking string dictionaries built with the library of Compressed
 * String Dictionaries (libCSD). These structures are based on the techniques 
 * presented in:
 *
 *   ==========================================================================
 *     "Compressed String Dictionaries"
 *     Nieves R. Brisaboa, Rodrigo Canovas, Francisco Claude, 
 *     Miguel A. Martinez-Prieto and Gonzalo Navarro.
 *     10th Symposium on Experimental Algorithms (SEA'2011), p.136-147, 2011.
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
 *   Francisco Claude:  	fclaude@cs.uwaterloo.ca
 *   Rodrigo Canovas:  		rcanovas@student.unimelb.edu.au
 *   Miguel A. Martinez-Prieto:	migumar2@infor.uva.es
 */

#ifndef _CHECK_CPP
#define _CHECK_CPP

#include <fstream>
#include <iostream>
using namespace std;

#include <time.h>

#include "src/StringDictionary.h"

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

void useCheck()
{
	cerr << endl;
	cerr << " ******************************************************************************** " << endl;
	cerr << " *** CHECK script for the library of Compressed String Dictionaries (libCSD). *** " << endl;
	cerr << " ******************************************************************************** " << endl;
	cerr << endl;
	cerr << " ----- check <mode> <in>" << endl;
	cerr << endl;
	cerr << " <mode> 1 : Check the LOCATE function." << endl;
	cerr << " <mode> 2 : Check the EXTRACT function." << endl;
	cerr << " <mode> 3 : Check the DECOMPRESSION function." << endl;
	cerr << " <mode> 4 : Check the PREFIX-based functions." << endl;
	cerr << " <mode> 5 : Check the SUBSTRING-based functions." << endl;
	cerr << endl;
	cerr << " <in> : input file containing the compressed string dictionary." << endl;
	cerr << endl;
}

void locate(StringDictionary *dict)
{
	{
		cerr << endl;
		cerr << " **************************************************** " << endl;
		cerr << " *** Locating all valid strings in the dictionary *** " << endl;
		cerr << " **************************************************** " << endl;

		size_t elements = dict->numElements();
		size_t valid = 0;
		size_t errors = 0;


		for (size_t i=1; i<=elements; i++)
		{
			uint strLen;
			uchar *str = dict->extract(i, &strLen);
			size_t id = dict->locate(str, strLen);

			if (id == i) valid++;
			else errors++;

			delete [] str;
		}

		cerr << "     - " << valid << " strings are located correctly." << endl;
		cerr << "     - " << errors << " strings are located with errors." << endl;
	}

	{
		cerr << endl;
		cerr << " **************************************************************** " << endl;
		cerr << " *** Locating non-valid strings preceding the first valid one *** " << endl;
		cerr << " **************************************************************** " << endl;

		size_t valid = 0;
		size_t errors = 0;

		uint strLen;
		uchar *str = dict->extract(1, &strLen);

		for (uint i=strLen; i>0; i--)
		{
			uchar *substr = new uchar[i+1];
			strncpy((char*)substr, (char*)str, i);
			substr[i-1]--; substr[i] = '\0';

			size_t id = dict->locate(substr, i);

			if (id == 0) valid++;
			else
			{
				uchar *substr2 = dict->extract(id, &strLen);

				// It is really an error (otherwise, we are looking for an existing string)
				if (strcmp((char*)substr,(char*)substr2) != 0) errors++;

				delete [] substr2;
			}

			delete [] substr;
		}

		delete [] str;

		cerr << "     - " << valid << " strings are located correctly." << endl;
		cerr << "     - " << errors << " strings are located as false positives." << endl;

	}

	{
		cerr << endl;
		cerr << " **************************************************************** " << endl;
		cerr << " *** Locating non-valid strings succeeding the last valid one *** " << endl;
		cerr << " **************************************************************** " << endl;

		size_t valid = 0;
		size_t errors = 0;

		uint strLen;
		uchar *str = dict->extract(dict->numElements(), &strLen);

		for (uint i=strLen; i>0; i--)
		{
			uchar *substr = new uchar[i+1];
			strncpy((char*)substr, (char*)str, i);
			substr[i-1]++; substr[i] = '\0';

			size_t id = dict->locate(substr, i);

			if (id == 0) valid++;
			else
			{
				uchar *substr2 = dict->extract(id, &strLen);

				// It is really an error (otherwise, we are looking for an existing string)
				if (strncmp((char*)substr,(char*)substr2, strLen) != 0) errors++;

				delete [] substr2;
			}


			delete [] substr;
		}

		delete [] str;

		cerr << "     - " << valid << " strings are located correctly." << endl;
		cerr << "     - " << errors << " strings are located as false positives." << endl;
	}

	{
		cerr << endl;
		cerr << " ********************************************* " << endl;
		cerr << " *** Locating non-valid 'internal' strings *** " << endl;
		cerr << " ********************************************* " << endl;

		size_t valid = 0;
		size_t errors = 0;

		srand (time(NULL));

		for (uint i=0; i<100; i++)
		{
			uint id = 1+(rand() % dict->numElements());

			uint strLen;
			uchar *str = dict->extract(id, &strLen);

			for (uint j=strLen; j>0; j--)
			{
				uchar *substr = new uchar[j+1];
				strncpy((char*)substr, (char*)str, j);
				substr[j-1]++; substr[j] = '\0';

				size_t id = dict->locate(substr, j);

				if (id == 0) valid++;
				else
				{
					uchar *substr2 = dict->extract(id, &strLen);

					// It is really an error (otherwise, we are looking for an existing string)
					if (strncmp((char*)substr,(char*)substr2, strLen) != 0) errors++;

					delete [] substr2;
				}

				delete [] substr;
			}

			delete [] str;
		}

		cerr << "     - " << valid << " strings are located correctly." << endl;
		cerr << "     - " << errors << " strings are located as false positives." << endl;
	}
}

void extract(StringDictionary *dict)
{
	{
		cerr << endl;
		cerr << " *************************************************** " << endl;
		cerr << " *** Extracting all valid IDs in the dictionary *** " << endl;
		cerr << " *************************************************** " << endl;
	
		size_t elements = dict->numElements();
		size_t valid = 0;
		size_t errors = 0;

		for (size_t i=1; i<=elements; i++)
		{
			uint strLen;
			uchar *str = dict->extract(i, &strLen);
			size_t id = dict->locate(str, strLen);

			if (id == i) valid++;
			else errors++;

			delete [] str;

		}

		cerr << "     - " << valid << " IDs are extracted correctly." << endl;
		cerr << "     - " << errors << " IDs are extracted with errors." << endl;
	}

	{
		cerr << endl;
		cerr << " ******************************** " << endl;
		cerr << " *** Extracting non-valid IDs *** " << endl;
		cerr << " ******************************** " << endl;

		size_t valid = 0;
		size_t errors = 0;

		{
			uint strLen;
			uchar *str = dict->extract(0, &strLen);

			if ((str == NULL) && (strLen ==0)) valid++;
			else errors++;

			delete [] str;
		}

		{
			uint strLen;
			uchar *str = dict->extract(dict->numElements()+1, &strLen);

			if ((str == NULL) && (strLen == 0)) valid++;
			else errors++;

			delete [] str;
		}

		cerr << "     - " << valid << " IDs are extracted correctly." << endl;
		cerr << "     - " << errors << " IDs are extracted with errors." << endl;
	}
}

void prefix(StringDictionary *dict)
{
	// Las operaciones infructuosas de prefijos son equivalentes a hacer
	// "locate" de algo que no existe...

	// OJO! Para la XBW no se puede hacer comprobación del previo y el sucesor de la forma general porque el orden
	// de los ids es el de las hojas en el trie.

	{
		cerr << endl;
		cerr << " ***************************************************************************************** " << endl;
		cerr << " *** Locating ID ranges for all prefixes in the first valid string and extracting them *** " << endl;
		cerr << " ***************************************************************************************** " << endl;

		size_t valid = 0;
		size_t errorsL = 0, errorsR = 0, errorsI = 0, errorsE = 0;

		uint strLen;
		uchar *str = dict->extract(1, &strLen);

		for (uint i=strLen; i>4; i--)
		{
			uchar *substr = new uchar[i+1];

			strncpy((char*)substr, (char*)str, i);
			substr[i] = '\0';

			IteratorDictID* itI = dict->locatePrefix(substr, i);
			IteratorDictString* itS = dict->extractPrefix(substr, i);

			if (itI->hasNext())
			{
				size_t current = itI->next();

				if ((current-1) > 0)
				{
					// Checking if the preceding ID is not prefixed
					uint prefLen;
					uchar *prefixed = dict->extract(current-1, &prefLen);

					if (strncmp((char*)substr, (char*)prefixed, i) == 0) errorsL++;
					delete [] prefixed;
				}

				{
					// Checking the internal IDs
					while (true)
					{
						uint prefLen;
						uchar *prefixed = dict->extract(current, &prefLen);

						if (strncmp((char*)substr, (char*)prefixed, i) == 0) valid++;
						else errorsI++;

						{
							uint prefLen;
							uchar *prefixedS = itS->next(&prefLen);

							if (strcmp((char*)prefixed, (char*)prefixedS) != 0) errorsE++;
							delete [] prefixedS;
						}

						delete [] prefixed;

						if (itI->hasNext()) current = itI->next();
						else break;
					}
				}

				if ((current+1) < dict->numElements())
				{
					// Checking if the succeding ID is not prefixed
					uint prefLen;
					uchar *prefixed = dict->extract(current+1, &prefLen);

					if (strncmp((char*)substr, (char*)prefixed, i) == 0) errorsR++;
					delete [] prefixed;
				}

			}
			else
			{
				// The range is empty
				errorsL++; errorsR++;
			}

			delete itI; delete itS;			
			delete [] substr;
		}

		delete [] str;

		cerr << "     - " << valid << " IDs are located and extracted correctly in their corresponding ranges." << endl;
		cerr << "     - " << errorsI << " IDs are located with errors in their corresponding ranges." << endl;
		cerr << "     - " << errorsE << " IDs are extracted with errors in their corresponding ranges." << endl;
		cerr << "     - " << errorsL << " left limits are no correct." << endl;
		cerr << "     - " << errorsR << " right limits are no correct." << endl;
	}


	{
		cerr << endl;
		cerr << " **************************************************************************************** " << endl;
		cerr << " *** Locating ID ranges for all prefixes in the last valid string and extracting them *** " << endl;
		cerr << " **************************************************************************************** " << endl;

		size_t valid = 0;
		size_t errorsL = 0, errorsR = 0, errorsI = 0, errorsE = 0;

		uint strLen;
		uchar *str = dict->extract(dict->numElements()-1, &strLen);

		for (uint i=strLen; i>4; i--)
		{
			uchar *substr = new uchar[i+1];
			strncpy((char*)substr, (char*)str, i);
			substr[i] = '\0';

			IteratorDictID* itI = dict->locatePrefix(substr, i);
			IteratorDictString* itS = dict->extractPrefix(substr, i);

			if (itI->hasNext())
			{
				size_t current = itI->next();

				if ((current-1) > 0)
				{
					// Checking if the preceding ID is not prefixed
					uint prefLen;
					uchar *prefixed = dict->extract(current-1, &prefLen);

					if (strncmp((char*)substr, (char*)prefixed, i) == 0) errorsL++;

					delete [] prefixed;
				}

				{
					// Checking the internal IDs
					while (true)
					{
						uint prefLen;
						uchar *prefixed = dict->extract(current, &prefLen);

						if (strncmp((char*)substr, (char*)prefixed, i) == 0) valid++;
						else
						{
							cout << substr << endl << prefixed << endl;
							errorsI++;
						}

						{
							uint prefLen;
							uchar *prefixedS = itS->next(&prefLen);

							if (strcmp((char*)prefixed, (char*)prefixedS) != 0)
							{
								cout << prefixed << endl << prefixedS << endl;
								errorsE++;
							}
							delete [] prefixedS;
						}

						delete [] prefixed;

						if (itI->hasNext()) current = itI->next();
						else break;
					}
				}

				if ((current+1) < dict->numElements())
				{
					// Checking if the succeding ID is not prefixed
					uint prefLen;
					uchar *prefixed = dict->extract(current+1, &prefLen);

					if (strncmp((char*)substr, (char*)prefixed, i) == 0) errorsR++;

					delete [] prefixed;
				}
			}
			else
			{
				// The range is empty
				errorsL++; errorsR++;
			}

			delete itI; delete itS;	
			delete [] substr;
		}

		delete [] str;

		cerr << "     - " << valid << " IDs are located and extracted correctly in their corresponding ranges." << endl;
		cerr << "     - " << errorsI << " IDs are located with errors in their corresponding ranges." << endl;
		cerr << "     - " << errorsE << " IDs are extracted with errors in their corresponding ranges." << endl;
		cerr << "     - " << errorsL << " left limits are no correct." << endl;
		cerr << "     - " << errorsR << " right limits are no correct." << endl;
	}

	{
		cerr << endl;
		cerr << " ************************************************************************************************ " << endl;
		cerr << " *** Locating ID ranges for all prefixes in some 'internal' valid strings and extracting them *** " << endl;
		cerr << " ************************************************************************************************ " << endl;

		size_t valid = 0;
		size_t errorsL = 0, errorsR = 0, errorsI = 0, errorsE = 0;

		srand (time(NULL));

		for (uint z=0; z<10; z++)
		{
			uint id = 1+(rand() % dict->numElements());

			uint strLen;
			uchar *str = dict->extract(id, &strLen);

			for (uint i=strLen; i>2; i--)
			{
				uchar *substr = new uchar[i+1];
				strncpy((char*)substr, (char*)str, i);
				substr[i] = '\0';

				IteratorDictID* itI = dict->locatePrefix(substr, i);
				IteratorDictString* itS = dict->extractPrefix(substr, i);

				if (itI->hasNext())
				{
					size_t current = itI->next();

					if ((current-1) > 0)
					{
						// Checking if the preceding ID is not prefixed
						uint prefLen;
						uchar *prefixed = dict->extract(current-1, &prefLen);

						if (strncmp((char*)substr, (char*)prefixed, i) == 0) errorsL++;

						delete [] prefixed;
					}

					{
						// Checking the internal IDs
						while (true)
						{
							uint prefLen;
							uchar *prefixed = dict->extract(current, &prefLen);

							if (strncmp((char*)substr, (char*)prefixed, i) == 0) valid++;
							else errorsI++;

							{
								uint prefLen;
								uchar *prefixedS = itS->next(&prefLen);

								if (strcmp((char*)prefixed, (char*)prefixedS) != 0) errorsE++;

								delete [] prefixedS;
							}

							delete [] prefixed;

							if (itI->hasNext()) current = itI->next();
							else break;
						}
					}

					if ((current+1) < dict->numElements())
					{
						// Checking if the succeding ID is not prefixed
						uint prefLen;
						uchar *prefixed = dict->extract(current+1, &prefLen);

						if (strncmp((char*)substr, (char*)prefixed, i) == 0) errorsR++;

						delete [] prefixed;
					}

				}
				else
				{
					// The range is empty
					errorsL++; errorsR++;
				}

				delete itI; delete itS;
				delete [] substr;
			}

			delete [] str;
		}

		cerr << "     - " << valid << " IDs are located and extracted correctly in their corresponding ranges." << endl;
		cerr << "     - " << errorsI << " IDs are located with errors in their corresponding ranges." << endl;
		cerr << "     - " << errorsE << " IDs are extracted with errors in their corresponding ranges." << endl;
		cerr << "     - " << errorsL << " left limits are no correct." << endl;
		cerr << "     - " << errorsR << " right limits are no correct." << endl;
	}
}

void substring(StringDictionary *dict)
{
	{
		cerr << endl;
		cerr << " ****************************************************************************************** " << endl;
		cerr << " *** Locating ID ranges for some substrings within any valid string and extracting them *** " << endl;
		cerr << " ****************************************************************************************** " << endl;

		size_t valid = 0;
		size_t errors = 0;
		size_t errorsE = 0;

		srand (time(NULL));

		for (uint z=0; z<10; z++)
		{
			uint id = 1+(rand() % dict->numElements());
			uint strLen;
			uchar *str = dict->extract(id, &strLen);

			for (uint i=strLen; i>2; i--)
			{
				uchar *substr = new uchar[i+1];
				strncpy((char*)substr, (char*)str, i);
				substr[i] = '\0';

				IteratorDictID* itI = dict->locateSubstr(substr, i);
				IteratorDictString* itS = dict->extractSubstr(substr, i);

				while (itI->hasNext())
				{
					uint next = itI->next();
					uint nextLen;
					uchar *nextStr = dict->extract(next, &nextLen);

					uint nextLen2;
					uchar *nextStr2 = itS->next(&nextLen2);
					if (strcmp((char*)nextStr, (char*)nextStr2) != 0) errorsE++;

					string s((char*)nextStr);
					std::size_t found = s.find((char*)substr);

					if (found != std::string::npos) valid++;
					else errors++;

					delete [] nextStr;
					delete [] nextStr2;
				}

				delete itI; delete itS;
				delete [] substr;
			}

			delete [] str;
		}

		cerr << "     - " << valid << " IDs are located and extracted correctly for the corresponding substring." << endl;
		cerr << "     - " << errors << " IDs are located with errors for the corresponding substring." << endl;
		cerr << "     - " << errors << " strings are extracted with errors for the corresponding substring." << endl;
	}
}


void decompress(StringDictionary *dict, char *name, char *type)
{
	IteratorDictString *it = dict->extractTable();
	string filename = (string(name)+string(".")+string(type)+string(".dec"));
	ofstream output(filename.c_str());

	while (it->hasNext())
	{
		uint strLen;
		uchar *str = it->next(&strLen);
		output << str << '\0';
		delete [] str;
	}

	output.close();

	delete it;
}
				
int 
main(int argc, char* argv[])
{
	if (argc == 3)
	{
		ifstream in(argv[2]);

		cout << "CHECKING -> " << argv[2] << endl;

		if (in.good())
		{

			int check = atoi(argv[1]);

			switch (check)
			{
				case 1:
				{
					StringDictionary *dict = StringDictionary::load(in);

					cout << "SIZE -> " << dict->getSize() << endl;

					in.close();
					locate(dict);

					delete dict;
					break;
				}

				case 2:
				{
					StringDictionary *dict = StringDictionary::load(in);

					cout << "SIZE -> " << dict->getSize() << endl;

					in.close();
					extract(dict);

					delete dict;
					break;
				}

				case 3:
				{
					cout << endl << "### Checking FULL DECOMPRESSION" << endl << endl;

					StringDictionary *dict = StringDictionary::load(in);

					cout << "SIZE -> " << dict->getSize() << endl;

					in.close();
					decompress(dict, argv[2], (char*)"a");

					delete dict;
					break;
				}

				case 4:
				{
					StringDictionary *dict = StringDictionary::load(in);
					in.close();
					prefix(dict);

					delete dict;
					break;
				}

				case 5:
				{
					StringDictionary *dict = StringDictionary::load(in);
					in.close();
					substring(dict);

					delete dict;
					break;
				}

				default:
					useCheck();
					break;
			}
		}
		else { checkFile(); exit(0); }
	}
	else useCheck();
}

#endif  /* _CHECK_CPP */

