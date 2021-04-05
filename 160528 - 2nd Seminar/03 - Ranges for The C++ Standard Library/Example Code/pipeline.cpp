#include<iostream>
#include<algorithm>
#include<vector>
#include<range\v3\all.hpp>

using std::cout;
using std::cin;
using std::endl;
using std::vector;
using namespace ranges;

// 아래와 같은 수열을 구해라
//[0,1,2,3,4,0,1,2,3,4,0,1,2,3,4]
int main()
{
	// for루프를 이용한 방식
	// 컨테이너를 원하는 크기로 만들고, 반복문을 이용해 내용을 직접 채워넣는다. (사고의 흐름의 반대)
	vector<int> v1(15);
	for (int i = 0; i < v1.size(); ++i)
		v1[i] = i % 5;

	cout << view::all(v1) << endl;

	// std::generate() 를 이용한 방식
	// 컨테이너를 원하는 크기로 만들고, 내용을 채워넣을 방법을 지정한다. (사고의 흐름의 반대)
	int n = 0;
	vector<int> v2(15);
	std::generate(v2.begin(), v2.end(), [&n] { 
		return n++ % 5;
	});

	cout << view::all(v2) << endl;

	// ranges 를 이용한 방식
	// 원하는 수열을 만들고, 그 수열로 컨테이너를 초기화한다. (사고의 흐름대로)
	vector<int> v3 =
		view::iota(0, 5) |
		view::cycle |
		view::take(15);

	cout << view::all(v3) << endl;

	// ranges 를 이용한 방식2
	// 원하는 수열을 만들고, 그 수열로 컨테이너를 초기화한다.
	auto quotient = [](int a, int b) {
		return a / 5 == b / 5; 
	};
	
	auto sum = [](auto rng) { 
		return ranges::accumulate(rng, 0); 
	};

	vector<int> v4 = view::iota(0) |
					view::group_by(quotient) |
					view::transform(sum) |
					view::take(6);

	cout << view::all(v4) << endl;
	//[10, 35, 60, 85, 110, 135]
}