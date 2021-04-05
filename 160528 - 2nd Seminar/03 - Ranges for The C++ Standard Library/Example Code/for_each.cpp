#include<iostream>
#include<vector>
#include<range/v3/all.hpp>

using namespace std;
using namespace ranges;

int main()
{
	// view::for_each �� Range<T> ��ü�� ����
	auto r = view::for_each(view::iota(0, 10), [](int i) {
		// view::for_each �� �⺻������ �� ������ ����� join �Ͽ� �ϳ��� view<Rng>�� ����� �Ѵ�. 
		// �� ����� ���� ������ �����ϰ� ���� ��� yield() �� ����Ѵ�. (�ѹ� �� �׷����ش�.)
		//return yield_from(view::repeat_n(i, i)); // yeild_from : �Ű������� view<Rng>���� Ȯ���ϴ� �뵵. �⺻������ �� �׷��� �����ؼ� ����.
		//return yield_if(i % 2 == 0, i); // yeild_if : ���ǽ��� true �� ������ ����. ���������� repeat_n ȣ��. �ι�° �Ű������� view<Rng>�� ���� ��� yield_from ���� �ٸ��� �׷����� ����.
		return yield(i); // yield : ���������� view::single() ȣ��. �׷����� ������ ��.
	});

	cout << typeid(r).name() << endl;

	cout << r << endl;

	// ranges::for_each �� v.end() �� ����
	vector<int> v = view::iota(0, 10);
	auto it = ranges::for_each(v, [](int i) {
		cout << i << " ";
	});
	cout << endl;
	cout << typeid(it).name() << endl;
	cout << boolalpha << (it == v.end()) <<endl;

	auto lamda = [](int i) {
		cout << i << " ";
	};
	
	// std::for_each �� ����° ���ڷ� �־��� callableType�� ����
	auto it2 = std::for_each(v.begin(), v.end(), lamda);
	cout << endl;
	cout << typeid(it2).name() << endl;
	cout << boolalpha << (it2 == lamda) << endl; // ����� functor�� ������ ���� �ٸ� ��ü���� bitwise equal.
												 // �� ���� => �� ���� Ÿ���̶�� �� �߿�

	// ���� view �� begin(), end()�� ��ȸ �����ϱ� ������ range-based for loop ����
	for (auto e : view::iota(0, 10))
	{
		cout << e << " ";
	}
	cout << endl;

	// begin(), end() ��������� ������ ���� (view_facade �� �����Ǿ� ����)
	auto vRng = view::iota(0, 10);
	for (auto it = vRng.begin(); it != vRng.end(); ++it)
	{
		cout << *it << endl;
	}
}