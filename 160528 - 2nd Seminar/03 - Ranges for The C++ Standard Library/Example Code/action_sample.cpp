#include<iostream>
#include<vector>
#include<random>
#include<range/v3/all.hpp>

using namespace ranges;
using std::cout;
using std::endl;

int main()
{
	std::default_random_engine gen;
	std::vector<int> v = view::iota(0, 10);

	action::shuffle(v, gen);
	// �Ʒ� �ڵ�鵵 ���� ���� �ǹ�
	//v | view::all | action::shuffle(gen);
	//v |= action::shuffle(gen);

	cout << view::all(v) << endl;
	//[2,7,8,4,0,6,1,9,3,5]

	auto v2 = v | ranges::copy | action::sort;
	cout << view::all(v2) << endl;
	//[0,1,2,3,4,5,6,7,8,9]

	// ¦���� ����
	v = view::ints(0, 10);
	v | view::stride(2) | action::shuffle(gen);
	cout << view::all(v) << endl;
	//[6,1,8,3,2,5,0,7,4,9]
}