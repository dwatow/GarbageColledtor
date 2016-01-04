#include <iostream>
#include "gc.h"

using namespace std;

int main()
{
	int *p, *q;
	p = new int(99);
	cout << *p << endl;
	cout << "-------" << endl;
	q = p;
	p = new int(100);
	cout << *p << endl;
	cout << *q << endl;
	return 0;
}