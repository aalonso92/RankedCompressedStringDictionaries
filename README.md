# RankedCompressedStringDictionaries

libCSD is a C++ library providing some different techniques for managing
string dictionaries in compressed space. These approaches are inspired on
the papers:

==========================================================================
  "Compressed String Dictionaries"
  Nieves R. Brisaboa, Rodrigo Cánovas, Francisco Claude, 
  Miguel A. Martínez-Prieto, and Gonzalo Navarro
  10th Symposium on Experimental Algorithms (SEA'2011), p.136-147, 2011.

  "Compression of Ranked String Dictionaries"
  Álvaro Alonso, Miguel A. Martínez-Prieto, and Gonzalo Navarro
  16th Symposium on Experimental Algorithms (SEA'2017), p.xxx-xxx, 2017.
==========================================================================

The current release is a beta version providing full functionality for
string location and ID extraction. Besides, some techniques support queries
by prefix and substrings. The set of techniques is described as follows:

- "Hash":  implements four different techniques which combines hashing and 
	   Huffman / Re-Pair compression.
- "FC":    implements two different techniques based on Front-Coding 
	   compression. PFC is a straightforward byte-oriented implementation,
	   and HTFC performs Hu-Tucker compression over the headers while
	   supports Huffman / Re-Pair compression for the internal strings.
- "RPDAC": uses Re-Pair for compressing the strings and.
- "FMI"  : self-indexes the dictionary and provides all types of queries using
	   the FM-Index facilities.
- "XBW"  : obtains a compressed trie of the dictionary and transforms it to
	   support all types of queries in highly compressed space.

The techniques support different parameterizations to achieve varied and
competitive space/time tradeoffs. 

Compiling the library
====================
1) For building the libcds library, type "make" within the libcds folder.
2) Once built libcds, type "make" within the root folder and libCSD is ready.


Building a dictionary
=====================
The library provides a simple command-line script for building dictionaries. There are two possibilities, ranked (RankedBuild) or normal dictionaries (Build):

./Build <type> <parameters> <in> <out>
  - The first parameter chooses the <type> of dictionary to be built. 
  - The set of <parameters> provide specific configuration values.
  - The <in> parameter locates the original dictionary file.
  - The <out> parameter tells the destination path.
  
./RankedBuild <type> <parameters> <in> <out>
  - The first parameter chooses the <type> of dictionary to be built. 
  - The set of <parameters> provide specific configuration values.
  - The <in> parameter locates the original dictionary file.
  - The <out> parameter tells the destination path.

It is worth noting that when using Build (normal dictionaries), "in" dictionary file must be lexicographically sorted;
and in both dictionary types strings must be ended with the '\0' ASCII char.

Examples:
=========
./Build 1 h 10 geonames dicts/geo.10

  Builds a HASH dictionary for "geonames" and stores it as "dicts/geo.10". The 
  dictionary uses a hash table which hash 10% more cells than strings in 
  "geonames" and compresses them using Huffman (h).


  ./Build 4 r 16 geonames dicts/geo.16

  Builds a HTFC (Hu-Tucker Front-Coding) dictionary for "geonames" and stores 
  it as "dicts/geo.16". The dictionary uses buckets of 16 strings and 
  compresses them using Re-Pair.

  
./RankedBuild 5 16 geonames dicts/geo.16

  Builds a wtRCSD dictionary for "geonames" and stores 
  it as "dicts/geo.16". The dictionary uses buckets of 16 strings.  
  

  
Testing a dictionary
====================
The library also provides a command-line script for testing purposes RankedTest for ranked dictionaries and Test for normal ones:

./Test <mode> <opt> <in> <file>

- This script supports four different <modes>:

  - 'r' is used for running the test chosen in <opt>:
	 - 'l' (for testing locate), 'e' (extract).
	 - 'pl' (prefix location), 'pe' (prefix extraction).
	 - 'sl' (substring location), 'pe' (substring extraction). 
  - 'g' is used for generating a basic testbed comprising <opt> valid strings
	for locate and <opt> valid IDs for extract.
  - 'p' is used for generating a prefix testbed comprising 100,000 query 
	patterns of some different lengths. The <opt> parameter tells the
	a reference string length, and the testbed obtain different files
	with patterns longer and shorter than this length.
  - 's' is used for generating a substring testbed with the features described
	above.

	
./RankedTest <mode> <opt> <in> <file>

- This script supports four different <modes>:

  - 'r' is used for running the test chosen in <opt>:
	 - 'l' (for testing locate), 'e' (extract).
	 - 'pl' (prefix location), 'pe' (prefix extraction).
	 - 'sl' (substring location), 'pe' (substring extraction). 
	 - 'pkl' (topk prefix location), 'pke' (topk prefix extraction).
	 - 'i' (topk interval extraction).
  - 'g' is used for generating a basic testbed comprising <opt> valid strings
	for locate and <opt> valid IDs for extract.
  - 'p' is used for generating a prefix testbed comprising 100,000 query 
	patterns of some different lengths. The <opt> parameter tells the
	a reference string length, and the testbed obtain different files
	with patterns longer and shorter than this length.
  - 's' is used for generating a substring testbed with the features described
	above.
  - 'i' is used for generating a interval testbed comprising 100,000 query 
	patterns of some lengths. The <opt> parameter tells the
	interval length, and the testbed obtain one file
	with patterns longer and shorter than this length.

	
- The parameter <in> locates the file storing the compressed string dictionary.
- The last <file> parameter locates the file comprising the testbed (for 'r'
  mode) or the destination path for saving the corresponding testbed (modes 
  'g', 'p', or 's').


Examples:
=========
./Test r l dicts/geo.10 tests/geo.strings

  Uses the testbed "tests/geo.strings" for analyzing how the dictionary stored 
  at "dicts/geo.10" behaves for 'locate'.

./RankedTest r i dicts/geo.10 tests/geo.intervals 

  Uses the testbed "tests/geo.intervals" for analyzing how the dictionary stored 
  at "dicts/geo.10" behaves for topk with intervals.
  
./Test g 100000 dicts/geo.10 tests/geo

  Uses the dictionary stored at "dicts/geo.10" for generating a basic testbed 
  of 100,000 strings and IDs which are stored as "tests/geo".

./Test p 16 dicts/geo.10 tests/geo

  Uses the dictionary stored at "dicts/geo.10" for generating a prefix testbed
  comprising patterns of lengths longer and shorter than 16 chars. The 
  resulting pattern set are stored at "tests/geo".
    


If you find bugs or have any issue with library, please ask us. Enjoy the 
library and if you find it useful for your research, please cite our papers:

@inproceeding{BCCMPN11,
  author    = "N. Brisaboa and R. C{\'a}novas and F. Claude 
                and M.A. Mart{\'{\i}}nez-Prieto and G. Navarro",
  title     = "Compressed String Dictionaries",
  booktitle = "Proc. 10th International Symposium on Experimental 
	        Algorithms (SEA)",
  year      = "2011",
  series    = "LNCS 6630",
  pages     = "136--147"
}

@inproceeding{XXXXXXXX,
  author    = "{\'A}lvaro Alonso and M.A. Mart{\'{\i}}nez-Prieto and G. Navarro",
  title     = "Compression of Ranked String Dictionaries",
  booktitle = "Proc. 16th International Symposium on Experimental 
	        Algorithms (SEA)",
  year      = "2017",
  series    = "XXXX XXXX",
  pages     = "XXX--XXX"
}

