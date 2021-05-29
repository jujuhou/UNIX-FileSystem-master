#include "head.h"
# include <vector>
int main()
{
	try {
		Init();
		vector<unsigned int> m;
		for (int i = 0; i < 200; i++) {
			unsigned int a;
			Allocate_Block(a);
			cout << a << endl;
			m.push_back(a);
		}
		cout << "-------------------\n";
		for (auto& it : m) {
			cout << it << endl;
		}
		cout << "-------------------\n";
		for (int i = 0; i < 200; i++) {
			Free_Block(*(m.end() - 1));
			m.pop_back();
		}
		cout << "-------------------\n";
		for (auto& it : m) {
			cout << it << endl;
		}
		cout << "-------------------\n";
		for (int i = 0; i < 200; i++) {
			unsigned int a;
			Allocate_Block(a);
			cout << a << endl;
			m.push_back(a);
		}
		cout << "-------------------\n";
		for (auto& it : m) {
			cout << it << endl;
		}
		cout << "-------------------\n";
	}
	catch (int& r) {
		cout << r;
	}
	

	return 0;
}