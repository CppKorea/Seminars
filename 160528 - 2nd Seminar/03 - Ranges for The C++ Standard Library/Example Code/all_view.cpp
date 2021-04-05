#include<iostream>
#include<vector>
#include<range/v3/all.hpp>

using namespace ranges;
using std::cout;
using std::endl;

int main()
{
	std::vector<int> v = {1, 3, 5, 6, 7};
	auto all = view::all(v);	// container '같은 걸' view 로 전환할 때
	cout << all << endl;
	cout << typeid(all).name() << endl;
	// iterator_range<vector_begin<int>(), vector_end<int>())

	// all 의 각 요소는 iterator 가 아니라 v의 원소.
	for (auto e : all)
		cout << e << " ";
	cout << endl;

	// 반면 iota(begin(), end()) 는 각 원소가 iterator
	auto iota = view::iota(v.begin(), v.end());
	auto deref = iota | view::transform([](const auto iter) {return *iter; });
	cout << deref << endl;
	
	for (const auto iter : iota)
		cout << *iter << " ";
	cout << endl;
}