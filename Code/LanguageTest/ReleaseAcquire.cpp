#include <thread>
#include <atomic>
#include <cassert>
#include <string>
#include <iostream>

std::atomic<std::string*> ptr;
int data;

void producer()
{
	std::string* p = new std::string("Hello");
	data = 42;
	ptr.store(p, std::memory_order_release);
}

void consumer()
{
	std::string* p2;
	while (!(p2 = ptr.load(std::memory_order_acquire)))
		;
	assert(*p2 == "Hello"); // never fires: *p2 carries dependency from ptr
	assert(data == 42); // may or may not fire: data does not carry dependency from ptr
	std::cout << "thread id = " << std::this_thread::get_id() << std::endl;
}

int main()
{
	std::thread t1(producer);
	std::thread t2(consumer);
	std::this_thread::sleep_for(std::chrono::seconds(5));
	std::thread t3(consumer);
	t1.join(); t2.join(); t3.join();
	system("PAUSE");
}