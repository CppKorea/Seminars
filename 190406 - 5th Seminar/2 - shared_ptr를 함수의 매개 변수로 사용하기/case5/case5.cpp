// case1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>


using namespace std;
using namespace chrono;
using namespace chrono_literals;

volatile int g_moo = 10;
const int TotalWorkCount = 300000000;

void Func2(const shared_ptr<int64_t>& a, const shared_ptr<int64_t>& b);

void Func1(const shared_ptr<int64_t>& a, const shared_ptr<int64_t>& b)
{
	(*a)++;
	(*b)++;
	Func2(a, b);
}

void Func2(const shared_ptr<int64_t>& a, const shared_ptr<int64_t>& b)
{
	(*a) += g_moo;
	(*b) += g_moo;
}

#pragma optimize("",off) // 이것이 없으면 아래 스레드 함수가 debug break 안됨

int main()
{
	shared_ptr<thread> workerThread;
	auto t0 = high_resolution_clock::now();

	{
		shared_ptr<int64_t> a = make_shared<int64_t>(0);
		shared_ptr<int64_t> b = make_shared<int64_t>(100);

		// 스레드 시작
		workerThread = make_shared<thread>([t0, a, b]()
		{
			auto a2 = a; 
			auto b2 = b;
			for (int i = 0; i < TotalWorkCount; i++)
			{
				Func1(a2, b2); // #TODO 5. 역시, 아래 thread.join 후, 여기다 debug break 후 refcount를 봅시다.
			}

			auto t1 = high_resolution_clock::now();

			auto elapsedMs = duration_cast<milliseconds>(t1 - t0).count();

			std::cout << "Work took " << elapsedMs << " ms.\n";
			std::cout << "a=" << *a << ", b=" << *b << endl;
		});

		// 즉시 a,b 소멸. 아직 workerThread는 작동중.
	}
	cout << "a and b are destroyed.\n";

	// workerThread의 소멸을 기다린다.
	workerThread->join();
}
