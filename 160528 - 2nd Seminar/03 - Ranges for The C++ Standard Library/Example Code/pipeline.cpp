#include<iostream>
#include<algorithm>
#include<vector>
#include<range\v3\all.hpp>

using std::cout;
using std::cin;
using std::endl;
using std::vector;
using namespace ranges;

// �Ʒ��� ���� ������ ���ض�
//[0,1,2,3,4,0,1,2,3,4,0,1,2,3,4]
int main()
{
	// for������ �̿��� ���
	// �����̳ʸ� ���ϴ� ũ��� �����, �ݺ����� �̿��� ������ ���� ä���ִ´�. (����� �帧�� �ݴ�)
	vector<int> v1(15);
	for (int i = 0; i < v1.size(); ++i)
		v1[i] = i % 5;

	cout << view::all(v1) << endl;

	// std::generate() �� �̿��� ���
	// �����̳ʸ� ���ϴ� ũ��� �����, ������ ä������ ����� �����Ѵ�. (����� �帧�� �ݴ�)
	int n = 0;
	vector<int> v2(15);
	std::generate(v2.begin(), v2.end(), [&n] { 
		return n++ % 5;
	});

	cout << view::all(v2) << endl;

	// ranges �� �̿��� ���
	// ���ϴ� ������ �����, �� ������ �����̳ʸ� �ʱ�ȭ�Ѵ�. (����� �帧���)
	vector<int> v3 =
		view::iota(0, 5) |
		view::cycle |
		view::take(15);

	cout << view::all(v3) << endl;

	// ranges �� �̿��� ���2
	// ���ϴ� ������ �����, �� ������ �����̳ʸ� �ʱ�ȭ�Ѵ�.
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