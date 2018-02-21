#include "MemoryPool.h"
#include <cmath>
#include <iostream>
#include <iomanip>

#define DEBUG

namespace MemoryPoolSpace
{
	/**
	 * constructor
	 * @param pool_size	size of pool
	 * @param node_size node size
	 * @return
	 */
	MemoryPool::MemoryPool(const size_t &pool_size, const size_t &node_size)
	{
		//initialize all parameter
		this->pool_size = pool_size;
		this->node_size = node_size;
		this->pool_free_size = 0;
		this->pool_used_size = 0;
		
		this->pcur = nullptr;
		this->plast = nullptr;
		
		this->no = 0;
		this->all_no = 0;
		this->pool_all_size = 0;
		this->use_object = 0;
		
		this->phead = new MemoryNode();
		
		this->phead->left = nullptr;
		this->phead->right = nullptr;
		this->phead->data = nullptr;
		
		this->is_first = true;
		
		int ret = this->alloc_pool_memory();
		if(ret == -1)
		{
			this->log(std::cout,this->error_msg);
		}

		this->debug_log(std::cout,"constructor initalized!");
	}
	
	/**
	 * destructor
	 * free all memroy
	 */
	MemoryPool::~MemoryPool()
	{
		this->free_all();
		this->debug_log(std::cout,"destructor end!");
	}
	
	/**
	 * calcuate how many node you need
	 */
	void MemoryPool::calc_node_number()
	{
		float value = this->pool_size / this->node_size;
		this->no = ceil(value);
	}
	
	/**
	 * rejust the memory size according to node number
	 */
	void MemoryPool::calc_memory_size()
	{
		this->pool_size = (size_t)this->no * this->node_size;
	}
	
	/**
	 * rejust memory while last function realloc memory
	 */
	void MemoryPool::reset_memory()
	{
		MemoryNode *ptmp = this->phead->right;
		for(int i = 0; ptmp != nullptr; ptmp = ptmp->right, i ++)
		{
			ptmp->data = &this->palloc[i * this->node_size];
			if(nullptr != ptmp->p_user)
			{
				*ptmp->p_user = ptmp->data;
			}
		}
		
		this->debug_log(std::cout,"reset memory end!");
	}
	
	/**
	 * mapping node and memory
	 * @param ptr pool memory pointer
	 */
	void MemoryPool::link_memory2node(BaseType *ptr)
	{
		MemoryNode *pnode = nullptr;
		for(size_t i = 0;i < this->no;i ++)
		{
			pnode = new MemoryNode();
			if(nullptr == this->plast)
			{
				pnode->right = this->phead->right;
				this->phead->right = pnode;
				pnode->left = this->phead;
				this->plast = pnode;
			}
			else
			{
				pnode->right = this->plast->right;
				this->plast->right = pnode;
				pnode->left = this->plast;
				this->plast = pnode;
			}
			
			this->plast->is_used = false;
			this->plast->data_size = this->pool_size - i * this->node_size;
			this->plast->data = &ptr[i * this->node_size];
			this->plast->is_begin = false;
			this->plast->p_user = nullptr;
			if(true == this->is_first && 0 == i)
			{
				this->plast->is_begin = true;
				this->is_first = false;
			}
		}
		
		this->update_link_node(this->plast);
		this->debug_log(std::cout, "link node to memory end!");
	}
	
	/**
	 * merge the free memory
	 * @param last the begin where you want to merge
	 */
	void MemoryPool::update_link_node(MemoryNode *last)
	{
		MemoryNode *pnode = nullptr;
		pnode = last->right;
		int size = 0;
		if(nullptr != pnode && false == pnode->is_used)
		{
			size = pnode->data_size;
		}
		
		for(int i = 0; last != this->phead && false == last->is_used ;i ++, last = last->left)
		{
			last->data_size = i * this->node_size + size;
		}
		
		this->pcur = last->right;
		this->debug_log(std::cout, "refresh memory and node end!");
	}
	
	/**
	 * allocate memory for pool according to need
	 * @return
	 */
	int MemoryPool::alloc_pool_memory()
	{
		this->calc_node_number();
		this->calc_memory_size();
		
		this->all_no += this->no;
		this->pool_all_size += this->pool_size;
		this->pool_free_size += this->pool_size;
		
		//allocate memory
		this->palloc = (BaseType *)realloc(this->palloc,this->pool_all_size * sizeof(BaseType));
		//error ocurr
		if(this->palloc == nullptr)
		{
			this->error_msg = "allocate memory failed!";
			return -1;
		}
		
		if(false == this->is_first)
		{
			this->reset_memory();
		}
		
		this->link_memory2node(&this->palloc[this->pool_all_size - this->pool_size]);
		
		this->debug_log(std::cout, "allocate pool memory end!");
		return 0;
	}
	
	/**
	 * output log information
	 * @param os where you want to output
	 * @param msg message
	 * @return
	 */
	std::ostream &MemoryPool::log(std::ostream &os,const string &msg)
	{
		return os<<__FILE__<<" "<<__func__<<" "<<__LINE__<<": something wrong "
				"occour!detail:"<<msg<<std::endl;
	}
	
	/**
	 * calcuate how many node do you need according to size
	 * @param size memory size
	 */
	void MemoryPool::calc_need_num(const size_t &size)
	{
		float value = size / this->node_size;
		this->no = ceil(value);
		if(this->no == 0)
		{
			this->no = 1;
		}
	}
	
	/**
	 * find porper memory block
	 * @param size target size
	 * @return the porper pointer
	 */
	MemoryNode* MemoryPool::find_memory_node(size_t size)
	{
		if(this->pcur->data_size >= size && false == this->pcur->is_used)
		{
			this->calc_need_num(size);
			MemoryNode *ptmp = this->pcur;
			for(size_t i = 0;i < this->no;i ++)
			{
				this->pcur->is_used = true;
				this->pcur = this->pcur->right;
			}
			
			ptmp->p_user = this->puser;
			ptmp->end = this->pcur->right;
			return (MemoryNode*)ptmp->data;
		}
		else
		{
			return nullptr;
		}
	}
	
	/**
	 * allocate memory for user
	 * @param ptr target pointer
	 * @param size size
	 * @return void
	 */
	void* MemoryPool::allocate(void **ptr, const size_t &size)
	{
		this->pool_free_size -= size;
		this->pool_used_size += size;
		this->puser = ptr;                      //user pointer
		this->use_object++;                     //current user number
		void *pfind = find_memory_node(size);   //find
		if(nullptr != pfind)
		{
			return pfind;
		}
		
		MemoryNode *ptmp = this->pcur;
		this->pcur = this->pcur->right;
		//allocate memory by traversal all node
		for(;ptmp != this->pcur;this->pcur = this->pcur->right)
		{
			if(nullptr == this->pcur)
			{
				this->pcur = this->phead->right;
			}
			
			if(true == this->pcur->is_used)
			{
				this->pcur = this->pcur->end;
			}
			
			pfind = this->find_memory_node(size);
			if(nullptr != pfind)
			{
				return pfind;
			}
			
			if(ptmp == this->pcur)
			{
				break;
			}
		}
		
		//the memory is not enouth,allocate more
		this->pool_size = size;
		this->alloc_pool_memory();
		this->debug_log(std::cout, "allocate memory for user end!");
		return find_memory_node(size);
	}
	
	/**
	 * find the node you want to free
	 * @param p memory pointer
	 * @return target node
	 */
	MemoryNode* MemoryPool::search_alloc_node(void *p)
	{
		for(MemoryNode *ptmp = this->phead->right;nullptr != ptmp;ptmp = ptmp->right)
		{
			if(ptmp->data == (BaseType*)p)
			{
				ptmp->p_user = nullptr;
				return ptmp;
			}
		}
		
		return nullptr;
	}
	
	/**
	 * free p with size length memory,here just reback the memory to pool,do not free it
	 * @param p the target you want to free
	 * @param size size
	 * @return -1 error
	 * 			0 normal
	 */
	int MemoryPool::free(void *p, const size_t &size)
	{
		this->use_object--;
		this->pool_used_size -= size;
		this->pool_free_size += size;
		MemoryNode *ptmp = this->search_alloc_node(p);
		if(nullptr == ptmp)
		{
			this->error_msg = "can not find the allocated pointer!";
			return -1;
		}
		
		this->calc_need_num(size);
		for(size_t i = 0;i < this->no;i ++)
		{
			ptmp->is_used = false;
			ptmp = ptmp->right;
		}
		
		//rejust memory
		this->update_link_node(ptmp->left);
		this->debug_log(std::cout, "free memory end!");
		return 0;
	}
	
	/**
	 * free all memory,reback all memory to os
	 * @return
	 */
	int MemoryPool::free_all_memory()
	{
		for(MemoryNode *ptmp = this->phead->right; nullptr != ptmp;ptmp = ptmp->right)
		{
			if(ptmp->is_begin && nullptr != ptmp->data)
			{
				delete []ptmp->data;
				if(ptmp->data != nullptr)
				{
					this->error_msg = "free memory error!";
					this->debug_log(std::cout, "free pool memory error!");
					return -1;
				}
				
				this->debug_log(std::cout, "free pool memory end!");
				return 0;
			}
		}
		
		this->debug_log(std::cout, "free pool memory error!");
		return -1;
	}
	
	/**
	 * safe delete
	 * @param p target you want to delete
	 */
	void MemoryPool::safe_delete(void *p)
	{
		if(nullptr != p)
		{
			delete p;
			p = nullptr;
		}
	}
	
	/**
	 * free all node in the pool
	 */
	void MemoryPool::free_all_node()
	{
		for(MemoryNode *ptmp = this->phead->right; nullptr != ptmp;)
		{
			MemoryNode* ptr = ptmp;
			ptmp = ptmp->right;
			this->safe_delete(ptr);
		}
		
		this->safe_delete(this->phead);
		this->debug_log(std::cout, "free node end!");
	}
	
	/**
	 * free all memory
	 * @return -1 error
	 * 			0 corrent
	 */
	int MemoryPool::free_all()
	{
		if(0 != this->use_object)
		{
			this->error_msg = "there still are some object are useing the memory!";
			this->debug_log(std::cout, "free pool memory error!");
			return - 1;
		}
		
		this->free_all_memory();
		this->free_all_node();
		return 0;
	}
	
	/**
	 * output the information of memory
	 * @param os
	 * @return
	 */
	ostream &MemoryPool::display_memory_status(ostream &os)
	{
		os<<"\n\t\t\tthe information of memory pool\t\t\t\n"
	      <<"pool total size:"<<this->pool_all_size
	      <<"\t\tused size:"<<this->pool_used_size
	      <<"\t\tfree size:"<<this->pool_free_size<<"\n\n";
		
		MemoryNode *ptmp = this->phead->right;
		os.flags(std::cout.left);
		os<<std::setw(4)<<"no";
		os<<std::setw(8)<<"size";
		os<<std::setw(6)<<"used";
		os<<std::endl;
		for(int i = 1;ptmp != nullptr;i ++,ptmp = ptmp->right)
		{
			os<<std::setw(4)<<i<<std::setw(8)<<ptmp->data_size<<std::setw(6)<<ptmp->is_used<<"\n";
		}
		
		return os;
	}

	/**
	 * output debug information
	 * @param os
	 * @param msg message
	 */
	void MemoryPool::debug_log(std::ostream &os,const string &msg)
	{
#ifndef DEBUG
		this->log(os,msg);
#endif
	}
};