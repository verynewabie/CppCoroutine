#include <initializer_list>
#include <iostream>
using namespace std;
void Handle() {
	int b = 0;
	cout << &b << endl;
	int c = 0;
	cout << &c << endl;
	auto tmp = {1,2,3,4,5};
	cout << tmp.begin() << endl;
	int d = 0;
	cout << &d << endl;
}