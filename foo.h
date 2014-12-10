#include <sys/types.h>
#include <stdint.h>




class foo
{
public:
	foo();
	~foo();
	void print();
private:
	uint8_t bytes[8];
};
