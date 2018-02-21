#include "MemoryPool.h"

using MemoryPoolSpace::MemoryPool;
int main(int argc,char **argv)
{
	MemoryPool *pool =  new MemoryPool();

	char *p1 = nullptr;
	char *p2 = nullptr;
	char *p3 = nullptr;
	p1 = (char *)pool->allocate((void**)&p1,700);
	pool->display_memory_status(std::cout);
	p2 = (char *)pool->allocate((void**)&p2,40);
	pool->display_memory_status(std::cout);
	p3 = (char *)pool->allocate((void**)&p3,100);
	pool->display_memory_status(std::cout);

	pool->free(p1,700);
	pool->display_memory_status(std::cout);
	pool->free(p1,40);
	pool->display_memory_status(std::cout);
	pool->free(p1,100);
	pool->display_memory_status(std::cout);
	
	
	p1 = (char *)pool->allocate((void **)&p1, 1400);
	pool->display_memory_status(std::cout);
	for(int i = 0;i < 1000;i ++)
	{
		p1[i] = 'a';
	}
	
	pool->display_memory_status(std::cout);
	pool->free(p1,1400);
	pool->display_memory_status(std::cout);
	
	delete pool;
	
}