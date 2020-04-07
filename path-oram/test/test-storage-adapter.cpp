#include "definitions.h"
#include "storage-adapter.hpp"
#include "utility.hpp"

#include "gtest/gtest.h"
#include <boost/format.hpp>
#include <fstream>
#include <openssl/aes.h>

using namespace std;

namespace PathORAM
{
	enum TestingStorageAdapterType
	{
		StorageAdapterTypeInMemory,
		StorageAdapterTypeFileSystem
	};

	class StorageAdapterTest : public testing::TestWithParam<TestingStorageAdapterType>
	{
		public:
		inline static const number CAPACITY	  = 10;
		inline static const number BLOCK_SIZE = 32;
		inline static const string FILE_NAME  = "storage.bin";

		protected:
		unique_ptr<AbsStorageAdapter> adapter;

		StorageAdapterTest()
		{
			auto type = GetParam();
			switch (type)
			{
				case StorageAdapterTypeInMemory:
					adapter = make_unique<InMemoryStorageAdapter>(CAPACITY, BLOCK_SIZE, bytes());
					break;
				case StorageAdapterTypeFileSystem:
					adapter = make_unique<FileSystemStorageAdapter>(CAPACITY, BLOCK_SIZE, bytes(), FILE_NAME, true);
					break;
				default:
					throw Exception(boost::format("TestingStorageAdapterType %1% is not implemented") % type);
			}
		}

		~StorageAdapterTest() override
		{
			remove(FILE_NAME.c_str());
		}
	};

	TEST_P(StorageAdapterTest, Initialization)
	{
		SUCCEED();
	}

	TEST_P(StorageAdapterTest, NoOverrideFile)
	{
		if (GetParam() == StorageAdapterTypeFileSystem)
		{
			auto data		= fromText("hello", BLOCK_SIZE);
			auto key		= getRandomBlock(KEYSIZE);
			string filename = "tmp.bin";

			auto storage = make_unique<FileSystemStorageAdapter>(CAPACITY, BLOCK_SIZE, key, filename, true);
			storage->set(CAPACITY - 1, {5, data});
			ASSERT_EQ(data, storage->get(CAPACITY - 1).second);
			storage.reset();

			storage = make_unique<FileSystemStorageAdapter>(CAPACITY, BLOCK_SIZE, key, filename, false);

			ASSERT_EQ(data, storage->get(CAPACITY - 1).second);

			remove(filename.c_str());
		}
		else
		{
			SUCCEED();
		}
	}

	TEST_P(StorageAdapterTest, CannotOpenFile)
	{
		if (GetParam() == StorageAdapterTypeFileSystem)
		{
			ASSERT_ANY_THROW(make_unique<FileSystemStorageAdapter>(CAPACITY, BLOCK_SIZE, bytes(), "tmp.bin", false));
		}
		else
		{
			SUCCEED();
		}
	}

	TEST_P(StorageAdapterTest, InputsCheck)
	{
		ASSERT_ANY_THROW(make_unique<InMemoryStorageAdapter>(CAPACITY, AES_BLOCK_SIZE, bytes()));
		ASSERT_ANY_THROW(make_unique<InMemoryStorageAdapter>(CAPACITY, AES_BLOCK_SIZE * 3 - 1, bytes()));
	}

	TEST_P(StorageAdapterTest, ReadWriteNoCrash)
	{
		EXPECT_NO_THROW({
			adapter->set(CAPACITY - 1, {5uLL, bytes()});
			adapter->get(CAPACITY - 2);
		});
	}

	TEST_P(StorageAdapterTest, ReadEmpty)
	{
		auto data = adapter->get(CAPACITY - 2).second;
		ASSERT_EQ(BLOCK_SIZE, data.size());
	}

	TEST_P(StorageAdapterTest, IdOutOfBounds)
	{
		ASSERT_ANY_THROW(adapter->get(CAPACITY + 1));
		ASSERT_ANY_THROW(adapter->set(CAPACITY + 1, {5uLL, bytes()}));
	}

	TEST_P(StorageAdapterTest, DataTooBig)
	{
		ASSERT_ANY_THROW(adapter->set(CAPACITY - 1, {5uLL, bytes(BLOCK_SIZE + 1, 0x08)}));
	}

	TEST_P(StorageAdapterTest, ReadWhatWasWritten)
	{
		auto data = bytes{0xa8};
		auto id	  = 5uLL;

		adapter->set(CAPACITY - 1, {id, data});
		auto [returnedId, returnedData] = adapter->get(CAPACITY - 1);

		data.resize(BLOCK_SIZE, 0x00);

		ASSERT_EQ(id, returnedId);
		ASSERT_EQ(data, returnedData);
	}

	TEST_P(StorageAdapterTest, OverrideData)
	{
		auto id	  = 5;
		auto data = bytes{0xa8};
		data.resize(BLOCK_SIZE, 0x00);

		adapter->set(CAPACITY - 1, {id, data});
		data[0] = 0x56;
		id		= 6;

		adapter->set(CAPACITY - 1, {id, data});

		auto [returnedId, returnedData] = adapter->get(CAPACITY - 1);

		ASSERT_EQ(id, returnedId);
		ASSERT_EQ(data, returnedData);
	}

	TEST_P(StorageAdapterTest, InitializeToEmpty)
	{
		for (number i = 0; i < CAPACITY; i++)
		{
			auto expected = bytes();
			expected.resize(BLOCK_SIZE);
			ASSERT_EQ(ULONG_MAX, adapter->get(i).first);
			ASSERT_EQ(expected, adapter->get(i).second);
		}
	}

	string printTestName(testing::TestParamInfo<TestingStorageAdapterType> input)
	{
		switch (input.param)
		{
			case StorageAdapterTypeInMemory:
				return "InMemory";
			case StorageAdapterTypeFileSystem:
				return "FileSystem";
			default:
				throw Exception(boost::format("TestingStorageAdapterType %1% is not implemented") % input.param);
		}
	}

	INSTANTIATE_TEST_SUITE_P(StorageAdapterSuite, StorageAdapterTest, testing::Values(StorageAdapterTypeInMemory, StorageAdapterTypeFileSystem), printTestName);
}

int main(int argc, char** argv)
{
	srand(TEST_SEED);

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
