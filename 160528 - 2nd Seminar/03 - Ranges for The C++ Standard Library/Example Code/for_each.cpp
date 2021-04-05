#include<iostream>
#include<vector>
#include<range/v3/all.hpp>

using namespace std;
using namespace ranges;

int main()
{
	// view::for_each 는 Range<T> 객체를 리턴
	auto r = view::for_each(view::iota(0, 10), [](int i) {
		// view::for_each 는 기본적으로 각 루프의 결과를 join 하여 하나의 view<Rng>로 만드려 한다. 
		// 각 결과를 하위 범위로 유지하고 싶을 경우 yield() 를 사용한다. (한번 더 그룹해준다.)
		//return yield_from(view::repeat_n(i, i)); // yeild_from : 매개변수가 view<Rng>인지 확인하는 용도. 기본적으로 각 그룹이 조인해서 나옴.
		//return yield_if(i % 2 == 0, i); // yeild_if : 조건식이 true 일 때에만 리턴. 내부적으로 repeat_n 호출. 두번째 매개변수에 view<Rng>를 넣을 경우 yield_from 과는 다르게 그룹지어 나옴.
		return yield(i); // yield : 내부적으로 view::single() 호출. 그룹지어 나오게 함.
	});

	cout << typeid(r).name() << endl;

	cout << r << endl;

	// ranges::for_each 는 v.end() 를 리턴
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
	
	// std::for_each 는 세번째 인자로 넣었던 callableType을 리턴
	auto it2 = std::for_each(v.begin(), v.end(), lamda);
	cout << endl;
	cout << typeid(it2).name() << endl;
	cout << boolalpha << (it2 == lamda) << endl; // 복사된 functor기 때문에 서로 다른 객체지만 bitwise equal.
												 // 비교 가능 => 즉 같은 타입이라는 게 중요

	// 물론 view 도 begin(), end()로 순회 가능하기 때문에 range-based for loop 가능
	for (auto e : view::iota(0, 10))
	{
		cout << e << " ";
	}
	cout << endl;

	// begin(), end() 멤버변수도 가지고 있음 (view_facade 에 구현되어 있음)
	auto vRng = view::iota(0, 10);
	for (auto it = vRng.begin(); it != vRng.end(); ++it)
	{
		cout << *it << endl;
	}
}