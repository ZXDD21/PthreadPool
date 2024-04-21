#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;
const int n = 10, m = 10;
int w = 0;
void funa() {

}
int main() {
	const int n = 20000;
	std::thread ar[n] = {};
	int i = 0;
	while (i < n) {
		try {
			ar[i++] = std::move(thread(funa));
			cout << i << endl;
		}
		catch (std::system_error& e) {
			cout << e.what() << endl;
			break;
		}
	}
	cout << i << endl;
	for (int j = 0; j < i; ++j) {
		ar[j].join();
	}
	return 0;
}
#if 0
void funaa(char ch) {
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < m; ++j) {
			printf("%c", ch);
		}
		//printf("\n");
	}
	//printf("\n");
}
void add(int a, int b) {
	int c = a + b;
	printf("%d\n", c);
}
void show() {
	w++;
	cout << w << endl;
	while(1){}
}
void zx(int &it) {
	it = 100;
	printf("113221313313211213213132132321132213");
}
int main() {
	std::cout << "main begin" << endl;
	thread tha(funa, 'A');
	thread thb(funa, 'B');
	thread thc(funa, 'C');
	tha.join();
	thb.join();
	thc.join();
	cout << "main endl";
}
#endif