#include "MemoryPool.h"
#include <cmath>
#include <iostream>


namespace MemoryPoolSpace
{
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
		
		this->alloc_pool_memory();
	}
	
	MemoryPool::~MemoryPool()
	{
		this->free_all();
	}
	
	void MemoryPool::calc_node_number()
	{
		float value = this->pool_size / this->node_size;
		this->no = ceil(value);
	}
	
	void MemoryPool::calc_memory_size()
	{
		this->pool_size = (size_t)this->no * this->node_size;
	}
	
	void MemoryPool::reset_memory()
	{
		MemoryNode *ptmp = this->phead->right;
		for(int i = 0; ptmp != nullptr; ptmp = ptmp->right, i ++)
		{
			ptmp->data = &this->palloc[i * this->node_size];
			if(nullptr != ptmp->p_user)
			{
				ptmp->p_user = ptmp->data;
			}
		}
	}
	
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
	}
	
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
	}
	
	void MemoryPool::alloc_pool_memory()
	{
		this->calc_node_number();
		this->calc_memory_size();
		
		this->all_no += this->no;
		this->pool_all_size += this->pool_size;
		this->pool_free_size += this->pool_size;
		
		this->palloc = (BaseType *)realloc(this->palloc,this->pool_all_size * sizeof(BaseType));
		//error ocurr
		if(this->palloc == nullptr)
		{
			this->error_msg = "allocate memory failed!";
			return;
		}
		
		if(false == this->is_first)
		{
			this->reset_memory();
		}
		
		this->link_memory2node(&this->palloc[this->pool_all_size - this->pool_size]);
	}
	
	void MemoryPool::calc_need_num(const size_t &size)
	{
		float value = size / this->node_size;
		this->no = ceil(value);
		if(this->no == 0)
		{
			this->no = 1;
		}
	}
	
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
	
	void* MemoryPool::allocate(void **ptr, const size_t &size)
	{
		this->pool_free_size -= size;
		this->pool_used_size += size;
		this->puser = ptr;
		this->use_object++;
		void *pfind = find_memory_node(size);
		if(nullptr != pfind)
		{
			return pfind;
		}
		
		MemoryNode *ptmp = this->pcur;
		this->pcur = this->pcur->right;
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
		
		this->pool_size = size;
		this->alloc_pool_memory();
		return find_memory_node(size);
	}
	
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
	
	void MemoryPool::free(void *p, const size_t &size)
	{
		this->use_object--;
		this->pool_used_size -= size;
		this->pool_free_size += size;
		MemoryNode *ptmp = this->search_alloc_node(p);
		if(nullptr == ptmp)
		{
			this->error_msg = "can not find the allocated pointer!";
			return;
		}
		
		this->calc_need_num(size);
		for(size_t i = 0;i < this->no;i ++)
		{
			ptmp->is_used = false;
			ptmp = ptmp->right;
		}
		
		this->update_link_node(ptmp->left);
	}
	
	void MemoryPool::free_all_memory()
	{
		for(MemoryNode *ptmp = this->phead->right; nullptr != ptmp;ptmp = ptmp->right)
		{
			if(ptmp->is_begin && nullptr != ptmp->data)
			{
				delete []ptmp->data;
				return;
			}
		}
	}
	
	void MemoryPool::free_all_node()
	{
		for(MemoryNode *ptmp = this->phead->right; nullptr != ptmp;)
		{
			MemoryNode* ptr = ptmp;
			ptmp = ptmp->right;
			delete ptr;
			
		}
		
		if(nullptr != this->phead)
		{
			delete this->phead;
		}
	}
	
	void MemoryPool::free_all()
	{
		if(0 != this->use_object)
		{
			this->error_msg = "there still are some object are useing the memory!";
			return;
		}
		
		this->free_all_memory();
		this->free_all_node();
	}
	
	ostream &MemoryPool::display_memory_status(ostream &os)
	{
		os<<"\n\t\t\tthe information of memory pool\t\t\t\n"
	      <<"\t\tpool total size:"<<this->pool_all_size<<"\n"
	      <<"\t\tused size:"<<this->pool_used_size<<"\n"
	      <<"\t\tfree size:"<<this->pool_free_size<<"\n";
		
		MemoryNode *ptmp = this->phead->right;
		for(int i = 1;ptmp != nullptr;i ++,ptmp = ptmp->right)
		{
			os<<"\npool no:"<<i<<"\t\tsize:"<<ptmp->data_size<<"\t\tused:"<<ptmp->is_used<<"\n";
		}
		
		return os;
	}
	
	
};