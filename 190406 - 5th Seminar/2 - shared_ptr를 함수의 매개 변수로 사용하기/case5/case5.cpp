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

#pragma optimize("",off) // �̰��� ������ �Ʒ� ������ �Լ��� debug break �ȵ�

int main()
{
	shared_ptr<thread> workerThread;
	auto t0 = high_resolution_clock::now();

	{
		shared_ptr<int64_t> a = make_shared<int64_t>(0);
		shared_ptr<int64_t> b = make_shared<int64_t>(100);

		// ������ ����
		workerThread = make_shared<thread>([t0, a, b]()
		{
			auto a2 = a; 
			auto b2 = b;
			for (int i = 0; i < TotalWorkCount; i++)
			{
				Func1(a2, b2); // #TODO 5. ����, �Ʒ� thread.join ��, ����� debug break �� refcount�� ���ô�.
			}

			auto t1 = high_resolution_clock::now();

			auto elapsedMs = duration_cast<milliseconds>(t1 - t0).count();

			std::cout << "Work took " << elapsedMs << " ms.\n";
			std::cout << "a=" << *a << ", b=" << *b << endl;
		});

		// ��� a,b �Ҹ�. ���� workerThread�� �۵���.
	}
	cout << "a and b are destroyed.\n";

	// workerThread�� �Ҹ��� ��ٸ���.
	workerThread->join();
}
