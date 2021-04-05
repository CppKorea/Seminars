#include<iostream>
#include<vector>
#include<range\v3\all.hpp>

using namespace std;
using namespace ranges;

int main()
{
	std::vector<int> numbers = view::iota(0, 12);

	auto r = numbers | view::group_by([](int a, int b) {
		//cout << endl << "group_by { " << a << ", " << b << " }" << endl;
		return a / 3 == b / 3;
	}) | view::group_by([](const auto& r1, const auto& r2) {
		return front(r1) / 6 == front(r2) / 6;
	}) | view::remove_if([](const auto& r) {
		return front(front(r)) >= 6;
	});

	cout << typeid(r).name() << endl;

	auto r2 = r | view::join;	// r에 재배정 할 수 없다. 타입이 변하기 때문

	cout << typeid(r2).name() << endl;

	cout << r << endl;
	cout << r2 << endl;

	for (auto e : numbers)
	{
		cout << e << " ";
	}
}
