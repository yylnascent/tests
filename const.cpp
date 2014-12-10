#include <iostream>

int main(void)
{
	int i = 5;
	const int *ip = &i;
	const int **ipp = &ip;

	//ip++;
	//ipp++;
	i += 10;

	std::cout << "i = " << i << std::endl;
	std::cout << "*ip = " << *ip << std::endl;
	std::cout << "**ipp = " << **ipp << std::endl;
	return 0;
}
