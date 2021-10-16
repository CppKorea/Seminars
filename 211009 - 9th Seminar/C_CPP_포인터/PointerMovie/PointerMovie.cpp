// PointerMovie.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <iomanip>
#include <string>
#include <Windows.h>

struct SPointerTest
{
	SPointerTest()
		: MemberPointer0(nullptr)
		, MemberPointer1(0)
		, MemberPointer2((void*)'\0')
	{

	}

	void* MemberPointer0;
	void* MemberPointer1;
	void* MemberPointer2;
};

class CPointerTest
{
public:
	CPointerTest()
		: MemberPointer0(nullptr)
		, MemberPointer1(0)
		, MemberPointer2((void*)'\0')
	{

	}

	void ClearPointer()
	{
		MemberPointer0 = nullptr;
		MemberPointer1 = nullptr;
		MemberPointer2 = nullptr;
	}

	void* MemberPointer0;
	void* MemberPointer1;
	void* MemberPointer2;
};

static void PrintMemory(std::string _Comment, void* _Target)
{
	std::cout << _Comment << std::hex << _Target << std::endl;
}

static void PrintValue(std::string _Comment, uint64_t _Target)
{
	std::cout << _Comment << std::hex << _Target << std::endl;
}

static void SimplePrintMemory(void* _Target)
{
	std::cout << "Pointer : " << std::hex << _Target << std::endl;
}

static void ForMemPrint(uint64_t* _Pointer, int _MaxCount)
{
	std::cout << "PrintData : " << std::endl;
	for (int Count = 0; _MaxCount > Count; ++Count)
	{
		std::cout << std::setfill('0') << std::setw(8) << std::right << std::hex << (int)_Pointer[Count];
		//if ((Count != 0) && ((Count+1) % 8) == 0)
		{
			std::cout << "|";
		}
	}
	std::cout << std::endl;
	std::cout << "Data End" << std::endl;
}

static void ForDataChunkPrint(uint64_t* _Pointer, int _MaxCount)
{
	std::cout << "Print Chunk Data : " << std::endl;
	for (int Count = 0; _MaxCount > Count; ++Count)
	{
		std::cout << std::setfill('0') << std::setw(8) << std::right << std::hex << (int)_Pointer[Count];
		//if ((Count != 0) && ((Count+1) % 8) == 0)
		{
			std::cout << "|";
		}
	}
	std::cout << std::endl;
	std::cout << "Data End" << std::endl;
}

static void DoingFunctionPointer(void* _Pointer, void* _Args)
{
	__try
	{
		((void(*)(void*))_Pointer)(_Args);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf_s("Error Occurr!! : NewData1's MemberPointer2 Is nullptr\n");
	}
}

int main()
{
	SPointerTest* StructPointer = 0;
	CPointerTest* ClassPointer = 0;

	std::cout << "========================== Start Example!! ==========================" << std::endl;

	int StackLocalData = 1;
	PrintMemory("Local Stack Data Address : ", &StackLocalData);

	char* DataAllocation = (char*)malloc(1024);
	ZeroMemory(DataAllocation, 1024);
	std::cout << "========================== Allocate Memory Chunk ==========================" << std::endl;
	ForMemPrint((uint64_t*)DataAllocation, 1024 / sizeof(uint64_t*));

	DataAllocation[0] = (char)1;
	DataAllocation[1] = (char)0;
	DataAllocation[2] = (char)0;
	DataAllocation[3] = (char)0;
	DataAllocation[4] = (char)0;
	DataAllocation[5] = (char)0;
	DataAllocation[6] = (char)0;
	DataAllocation[7] = (char)0;

	DataAllocation[8] = (char)1;
	DataAllocation[9] = (char)0;
	DataAllocation[10] = (char)0;
	DataAllocation[11] = (char)0;
	DataAllocation[12] = (char)0;
	DataAllocation[13] = (char)0;
	DataAllocation[14] = (char)0;
	DataAllocation[15] = (char)0;

	DataAllocation[16] = (char)4;
	DataAllocation[17] = (char)0;
	DataAllocation[18] = (char)0;
	DataAllocation[19] = (char)0;
	DataAllocation[20] = (char)0;
	DataAllocation[21] = (char)0;
	DataAllocation[22] = (char)0;
	DataAllocation[23] = (char)0;
	std::cout << "========================== Memory Setup Manually ==========================" << std::endl;
	ForDataChunkPrint((uint64_t*)DataAllocation, 1024 / sizeof(uint64_t*));

	CPointerTest* NewData0 = (CPointerTest*)&DataAllocation[0];
	std::cout << "========================== Class Manual Memory Setup ==========================" << std::endl;
	ForMemPrint((uint64_t*)NewData0, sizeof(CPointerTest));

	SPointerTest* NewData1 = (SPointerTest*)&DataAllocation[8];
	std::cout << "========================== Class Manual Memory Setup ==========================" << std::endl;
	ForMemPrint((uint64_t*)NewData1, sizeof(CPointerTest));

	void(*NewIntegerPointer)(void*) = &SimplePrintMemory;
	std::cout << "========================== Set Function Pointer ==========================" << std::endl;
	PrintMemory("SimplePrintMemory Function Address : %x", &SimplePrintMemory);
	PrintMemory("NewIntegerPointer Address : ", &NewIntegerPointer);
	PrintMemory("NewIntegerPointer Function Pointer Address : ", NewIntegerPointer);
	PrintValue("NewIntegerPointer Function Pointer Value : ", *(uint64_t*)NewIntegerPointer);

	uint64_t* GetPointer = (uint64_t*)&DataAllocation[16];
	std::cout << "========================== Get Memory Chunk Address ==========================" << std::endl;
	PrintMemory("GetPointer Address : ", &GetPointer);
	PrintMemory("GetPointer Function Address : ", GetPointer);
	PrintValue("GetPointer Value : ", *(uint64_t*)GetPointer);
	
	*GetPointer = (uint64_t)NewIntegerPointer;
	std::cout << "========================== Set Function Pointer On Memory Chunk ==========================" << std::endl;
	std::cout << "Class Manual Memory Check" << std::endl;
	ForMemPrint((uint64_t*)NewData0, sizeof(CPointerTest) / sizeof(uint64_t*));
	std::cout << "Struct Manual Memory Check" << std::endl;
	ForMemPrint((uint64_t*)NewData1, sizeof(CPointerTest) / sizeof(uint64_t*));
	ForDataChunkPrint((uint64_t*)DataAllocation, 1024 / sizeof(uint64_t*));
	
	void(CPointerTest::*Test)() = &CPointerTest::ClearPointer;
	std::cout << "========================== Set Member Function Pointer ==========================" << std::endl;
	PrintMemory("Test Member Function Pointer Address : ", &Test);
	PrintMemory("Test Member Function Pointer Address : ", (void*&)Test);

	std::cout << "========================== Use Function Pointer ==========================" << std::endl;
	std::cout << "Doing Function Pointer SimplePrintMemory" << std::endl;
	DoingFunctionPointer(NewData0->MemberPointer2, GetPointer);

	std::cout << "========================== Use Member Function Pointer ==========================" << std::endl;
	ForDataChunkPrint((uint64_t*)DataAllocation, 1024 / sizeof(uint64_t*));
	(NewData0->*Test)();
	ForDataChunkPrint((uint64_t*)DataAllocation, 1024 / sizeof(uint64_t*));

	std::cout << "========================== Null Function Pointer Error ==========================" << std::endl;
	DoingFunctionPointer(NewData0->MemberPointer2, GetPointer);

	free(DataAllocation);
}
