CPP=g++
FLAGS=-O9 -Wall -DNDEBUG -I libcds/includes/ 
#FLAGS=-O0 -g3 -Wall -DNDEBUG -I libcds/includes/
LIB=libcds/lib/libcds.a

OBJECTS_CODER=src/utils/Coder/StatCoder.o src/utils/Coder/DecodingTableBuilder.o src/utils/Coder/DecodingTable.o src/utils/Coder/DecodingTree.o src/utils/Coder/BinaryNode.o
OBJECTS_UTILS=src/utils/VByte.o src/utils/LogSequence.o src/utils/DAC_VLS.o src/utils/DAC_BVLS.o $(OBJECTS_CODER) 
 
OBJECTS_HUTUCKER=src/HuTucker/HuTucker.o
OBJECTS_REPAIR=src/RePair/Coder/arrayg.o src/RePair/Coder/basics.o src/RePair/Coder/hash.o src/RePair/Coder/heap.o src/RePair/Coder/records.o src/RePair/Coder/dictionary.o src/RePair/Coder/IRePair.o src/RePair/Coder/CRePair.o src/RePair/RePair.o
OBJECTS_HASH=src/Hash/Hash.o src/Hash/HashDAC.o src/Hash/Hashdh.o src/Hash/HashBdh.o src/Hash/HashBBdh.o
OBJECTS_HUFFMAN=src/Huffman/huff.o src/Huffman/Huffman.o
OBJECTS_FMINDEX=src/FMIndex/SuffixArray.o src/FMIndex/SSA.o
OBJECTS_XBW=src/XBW/TrieNode.o src/XBW/XBW.o  
OBJECTS=$(OBJECTS_UTILS) $(OBJECTS_HUTUCKER) $(OBJECTS_HUFFMAN) $(OBJECTS_REPAIR) $(OBJECTS_HASH) $(OBJECTS_XBW) $(OBJECTS_FMINDEX) src/StringDictionary.o src/RankedStringDictionary.o src/StringDictionaryHASHHF.o src/StringDictionaryHASHRPF.o src/StringDictionaryHASHUFFDAC.o src/StringDictionaryHASHRPDAC.o src/StringDictionaryPFC.o src/StringDictionaryRPFC.o src/StringDictionaryHTFC.o src/StringDictionaryHHTFC.o src/StringDictionaryRPHTFC.o src/StringDictionaryRPDAC.o src/StringDictionaryXBW.o src/StringDictionaryFMINDEX.o src/RankedStringDictionarySimple.o src/RankedStringDictionaryRPDAC.o
EXES=Build.o Test.o Check.o RankedTest.o RankedBuild.o

BIN=Build Test Check RankedTest RankedBuild

%.o: %.cpp
	@echo " [C++] Compiling $<"
	@$(CPP) $(FLAGS) -c $< -o $@

all: clean $(OBJECTS) $(EXES) $(BIN) libCSD
	@echo " [FLG] $(FLAGS)"
	@echo " [MSG] Done compiling tests"
	

RankedTest:	
	$(CPP) $(FLAGS) -o RankedTest RankedTest.o $(OBJECTS) ${LIB}
	@rm RankedTest.o

Build:	
	$(CPP) $(FLAGS) -o Build Build.o $(OBJECTS) ${LIB}
	@rm Build.o
	
RankedBuild:	
	$(CPP) $(FLAGS) -o RankedBuild RankedBuild.o $(OBJECTS) ${LIB}
	@rm RankedBuild.o
	
Test:	
	$(CPP) $(FLAGS) -o Test Test.o $(OBJECTS) ${LIB}
	@rm Test.o

Check:	
	$(CPP) $(FLAGS) -o Check Check.o $(OBJECTS) ${LIB}
	@rm Check.o

clean:
	@echo " [CLN] Removing object files"
	@rm -f  $(BIN) $(OBJECTS) $(EXES) *~ src/*~ src/iterators/*~ src/FMIndex/*~ src/Hash/*~ src/Huffman/*~ src/RePair/*~ src/utils/*~ src/XBW/*~

libCSD:
	@echo " [LIB] Packing the object files"
	@rm -f lib/libCSD.a
	@ar rcs lib/libCSD.a $(OBJECTS) 

