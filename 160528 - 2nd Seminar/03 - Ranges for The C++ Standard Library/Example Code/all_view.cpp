#include<iostream>
#include<vector>
#include<range/v3/all.hpp>

using namespace ranges;
using std::cout;
using std::endl;

int main()
{
	std::vector<int> v = {1, 3, 5, 6, 7};
	auto all = view::all(v);	// container '���� ��' view �� ��ȯ�� ��
	cout << all << endl;
	cout << typeid(all).name() << endl;
	// iterator_range<vector_begin<int>(), vector_end<int>())

	// all �� �� ��Ҵ� iterator �� �ƴ϶� v�� ����.
	for (auto e : all)
		cout << e << " ";
	cout << endl;

	// �ݸ� iota(begin(), end()) �� �� ���Ұ� iterator
	auto iota = view::iota(v.begin(), v.end());
	auto deref = iota | view::transform([](const auto iter) {return *iter; });
	cout << deref << endl;
	
	for (const auto iter : iota)
		cout << *iter << " ";
	cout << endl;
}