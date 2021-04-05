#include<iostream>
#include<vector>
#include<range/v3/all.hpp>

using namespace ranges;
using std::cout;
using std::endl;

template <class T>
struct TD;

int main()
{
	// 1. iota | take
	auto rng0 =	view::iota(0) | 
				view::take(10);

	cout << typeid(rng0).name() << endl;
	//struct ranges::v3::detail::take_exactly_view_<struct ranges::v3::iota_view<int,void>,0>

	// 2. iota | cycle | take
	const auto rng1 =	view::iota(0, 5) |
				view::cycle |
				view::take(15);

	cout << typeid(rng1).name() << endl;
	// take_exactly_view_<cycled_view<take_exactly_view_<iota_view<int, void>, 1> >, 0>

	// 3. iota | group_by | transform
	auto quotient = [](int a, int	 b) {
		return a / 5 == b / 5;
	};

	auto sum = [](auto rng) {
		return ranges::accumulate(rng, 0);
	};

	auto rng2 =	view::iota(0, 15) |
				view::group_by(quotient) |
				view::transform(sum);

	// ���ٰ� ���ø� �Ķ���ͷ� ���� Ÿ�Կ��� typeid() ����
	//cout << typeid(rng2).name() << endl;
	//?AU?$transform_view@U?$group_by_view@U?$take_exactly_view_@U?$iota_view@HX@v3@ranges@@$00@detail@v3@ranges@@V<lambda_0>@?main@@9@@v3@ranges@@V<lambda_1>@?main@@9@@v3@ranges@@

	// ��� TypeDisplayer �� Ÿ�� Ȯ��
	//TD<decltype(rng2)>();
	// error : implicit instantiation of undefined template
	// 'TD<transform_view<group_by_view<take_exactly_view_<iota_view<int, void>, true>, 
	//  (lambda at D:\����\type_of_range.cpp:32:18)>, (lambda at D:\����\type_of_range.cpp:36:13)> >'
}	