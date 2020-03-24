#include "definitions.h"

#include <iostream>

namespace PathORAM
{
	using namespace std;

	class AbsPositionMapAdapter
	{
		public:
		virtual ulong get(ulong block)			  = 0;
		virtual void set(ulong block, ulong leaf) = 0;
		virtual ~AbsPositionMapAdapter()						  = 0;
	};

	class InMemoryPositionMapAdapter : public AbsPositionMapAdapter
	{
		private:
		ulong *map;
		ulong capacity;

		void checkCapacity(ulong block);

		public:
		InMemoryPositionMapAdapter(ulong capacity);
		~InMemoryPositionMapAdapter() final;
		ulong get(ulong block) final;
		void set(ulong block, ulong leaf) final;
	};
}