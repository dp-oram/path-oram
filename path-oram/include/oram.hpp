#include "definitions.h"
#include "position-map-adapter.hpp"
#include "stash-adapter.hpp"
#include "storage-adapter.hpp"

// #include <gtest/gtest_prod.h>

#include <iostream>
#include <unordered_map>

namespace PathORAM
{
	using namespace std;

	class ORAM
	{
		private:
		AbsStorageAdapter *storage;
		AbsPositionMapAdapter *map;
		AbsStashAdapter *stash;

		ulong dataSize;
		ulong Z;

		ulong height;
		ulong buckets;
		ulong blocks;

		bytes access(bool read, ulong block, bytes data);
		void readPath(ulong leaf);
		void writePath(ulong leaf);

		bool canInclude(ulong pathLeaf, ulong blockPosition, ulong level);
		ulong bucketForLevelLeaf(ulong level, ulong leaf);

		void checkConsistency();

		friend class ORAMTest_BucketFromLevelLeaf_Test;
		friend class ORAMTest_CanInclude_Test;
		friend class ORAMTest_ReadPath_Test;
		friend class ORAMTest_ConsistencyCheck_Test;

		public:
		ORAM(ulong logCapacity, ulong blockSize, ulong Z, AbsStorageAdapter *storage, AbsPositionMapAdapter *map, AbsStashAdapter *stash);
		~ORAM();

		bytes get(ulong block);
		void put(ulong block, bytes data);
	};
}