#include <stdio.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
// #include <asm/atomic.h>

using linuxSys::sp;
using linuxSys::RefBase;

class mytest : public RefBase
{
public:
	mytest() {
		printf("mytest init point :%#x\n",getStrongCount());
	}

	~mytest(){
		printf("mytest destroy point :%#x\n",getStrongCount());
	}
};

int main(int argc, char const *argv[])
{
	sp<mytest> mysp = new mytest();
	printf("count point :%#x\n",mysp->getStrongCount());
	{
		sp<mytest> mysp2 = mysp;
		printf("mysp count point :%#x\n",mysp->getStrongCount());
		printf("mysp2 count point :%#x\n",mysp2->getStrongCount());
	}
	printf("end count point :%#x\n",mysp->getStrongCount());
	return 0;
}

 