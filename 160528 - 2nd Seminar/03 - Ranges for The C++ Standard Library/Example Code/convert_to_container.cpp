#include<iostream>
#include<vector>
#include<set>
#include<unordered_map>
#include<range/v3/all.hpp>

using namespace ranges;
using std::cout;
using std::endl;

int main()
{
	std::set<int> s = view::iota(0, 10);
	std::vector<int> v = view::iota(0, 10);

	// pair형 범위는 view::zip() 으로 
	auto rng = view::zip(view::iota(0), view::iota(10)) | view::take(5);
	std::unordered_map<int, int> m = rng;

	for (auto e : m)
	{
		cout << e.first << ", " << e.second << endl;
	}
}