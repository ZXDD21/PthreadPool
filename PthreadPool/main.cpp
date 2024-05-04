#include "my_unique_ptr.h"
#include "my_threadpool.h"
#include "my_shared_ptr.h"
using namespace std;
class Int {
private:
	int data;
public:
	explicit Int(int x = 0) :data(x) {};//明确关键字，不允许重载
	~Int() { cout << "delete" << endl; };
	void Print()const {
		cout << data << endl;
	}
};
void func() {
	my_shared_ptr<Int>pa(new Int(5));
	my_shared_ptr<Int>pb(new Int(5));
	my_shared_ptr<Int>pc=pb;
	my_shared_ptr<Int>pd=std::move(pb);
	pa->Print();
	//pb->Print();
	pc->Print();
	pd->Print();
	{
		my_shared_ptr<Int>pe(pd);
		pc->Print();
	}
	//pb->Print();
}
int main() {
	func();
	return 0;
}

/*int main() {
	//my_unique_ptr<int>ptr(new int);
	//my_unique_ptr<int[]>par(new int[10]);
	Int a(10);
	//Int b = 20;
	Int c{ 30 };
	Int d;
	std::unique_ptr<Int>pa(new Int(10));
	//std::unique_ptr<Int>pb = new Int(10);
	std::unique_ptr<Int>pc = std::make_unique <Int>(20);
	return 0;
}*/