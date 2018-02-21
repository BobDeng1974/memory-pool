#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <string>
#include <iostream>
#include <cstdlib>

using std::string;
using std::ostream;

namespace MemoryPoolSpace
{
	typedef unsigned char BaseType;
	
	static const size_t DEFAULT_MEMORY_SIZE = 1000; //default size of memory pool
	static const size_t DEFAULT_NODE_SIZE = 100;    //default size of pool node
	static const int INITIAL_MEMORY_CONTENT = 0xff; //1111 1111
	
	/**
	 * a doubly linked list to memory the memroy
	 */
	typedef struct MemoryNode
	{
		MemoryNode *left;           //left node
		MemoryNode *right;          //rigit node
		MemoryNode *end;            //the end of memory;
		
		size_t data_size;           //size of node
		
		bool is_used;               //if the node is used
		bool is_begin;              //if the node is the begin of the list
		
		BaseType *data;             //point to the memory
		void *p_user;               //the owner of memory node
	}MemoryNode;
	
	class MemoryPool
	{
	public:
		MemoryPool(const size_t &pool_size = DEFAULT_MEMORY_SIZE, const size_t &node_size = DEFAULT_NODE_SIZE);
		~MemoryPool();
	
	private:
		//forbid copy constructor
		MemoryPool(MemoryPool *pool);
		MemoryPool &operator=(MemoryPool &pool);
	
	public:
		void *allocate(void **ptr, const size_t &size);      //allocate memory
		void free(void *p, const size_t &size);             //free memory
		void free_all();                                    //free all memory
		
		ostream& display_memory_status(ostream &os);                       //display the status of memory
		void get_error_info();                              //get the erro information
	private:
		void alloc_pool_memory();
		void calc_node_number();                            //calcuate how many node is needed
		void calc_memory_size();                            //calcuate how many memory should allocate actully
		void calc_need_num(const size_t &size);
		void reset_memory();                                //reset the memory after realloc
		void link_memory2node(BaseType *ptr);
		void update_link_node(MemoryNode *last);
		MemoryNode* find_memory_node(size_t size);
		MemoryNode* search_alloc_node(void *p);
		void free_all_memory();
		void free_all_node();
	private:
		MemoryNode *phead;                                  //head of list
		MemoryNode *pcur;                                   //current node
		MemoryNode *plast;                                  //last node
		
		BaseType *palloc;
		
		bool is_first;                                      //if the memory is the first time to allocate
		
		size_t no;                                          //number of node
		size_t all_no;                                      //number of pool
		size_t node_size;                                   //node size
		size_t pool_size;                                   //pool size
		size_t pool_all_size;                               //total size
		size_t pool_used_size;                              //used size
		size_t pool_free_size;                              //free size
		
		size_t use_object;
		
		string error_msg;
		
		void **puser;
	};
};


#endif //MEMORY_POOL_H
