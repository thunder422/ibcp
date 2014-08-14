#include <iostream>

int main()
{
	// test initializers
	int i {1};
	double d {2.0};

	// test initializer list
	int a[] {1, 2, 3, 4};

	// test nullptr
	int *pi {nullptr};

	pi = &i;

	std::cout << d << a[1] << *pi << std::endl;
}
