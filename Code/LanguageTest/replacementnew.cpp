#include <stdlib.h>
#include <new>

class Base
{
public:
	Base(){
		a = 0;
	}
	virtual ~Base()
	{
		a = 0;
	}

private:
	int a;
};

class Derived : public Base
{
public:
	Derived(){
		k = 0;

	}
	~Derived()
	{
		k = 0;
	}

private:
	int k;
};

void main()
{
	Base* b = (Base*)malloc(sizeof(Derived));
	
	b = new (b) Derived;

	delete (void*)b;
}