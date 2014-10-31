#include <memory>

struct A
{

};
void main()
{
	std::weak_ptr<A> w;
	void* p = 0;
	{
		std::shared_ptr<A> s(new A);
		if (s)
		{
			int a = 0;
			a++;
		}

		s.reset();
		if (s)
		{
			int a = 0;
			a++;
		}
		w = s;
		p = w.lock().get();

	}

	p = w.lock().get();
	
	
	int a = 0;
	a++;
}