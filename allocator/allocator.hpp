#ifndef _ALLOCATOR_
#define _ALLOCATOR_

#include<cstddef>
#include<type_traits>
#include<algorithm>
#include<cmath>
template <typename T>
class Allocator {
public:
	typedef T          value_type;
	typedef T*          pointer;
	typedef const T*    const_pointer;
	typedef void *      void_pointer;
	typedef T&          reference;
	typedef const T&    const_reference;
	typedef size_t      size_type;
	typedef ptrdiff_t   difference_type;

    template <typename U> 
    struct rebind {
		typedef Allocator<U> others;
    };
	Allocator(){};
	template <class U>
	Allocator(const Allocator<U>& c) {}

    pointer allocate(size_type);
    void deallocate(pointer, size_type);
	void construct(pointer p) {
		new(p) T();
	}
	pointer address(reference x) const noexcept
	{
		return &x;
	}
	const_pointer address(const_reference x) const noexcept
	{
		return &x;
	}
	void construct(pointer p, const T& value) {
		new(p) T(value);
	}
	void destroy(pointer p) {
		p->~T();
	}

private:
    static const int MAX_BYTES = 240000;/*max size in freelist*/
    static const int BASE = 8;/*the difference between two adjacent blocks*/
    static const int NUM = MAX_BYTES / BASE;/*the number of different blocks*/
    static char *head, *tail;
    static size_type heap_size;
	struct Block {
	Block* next;
	};/*Link list structure pointing to the next address*/
	static Block *free[NUM];/*freelist*/
    void add(size_type, size_type);/*increase the number of some kind of blocks*/
    void_pointer mempool(size_type, size_type &);/*increase the memory of memorypool*/
};

/*initialize the variable of allocator class */
template <typename T>
char *Allocator<T>::tail = nullptr;

template <typename T>
char *Allocator<T>::head = nullptr;

template <typename T>
typename Allocator<T>::size_type Allocator<T>::heap_size = 0;

template <typename T>
typename Allocator<T>::Block * Allocator<T>::free[NUM];

template <typename T>
typename Allocator<T>::pointer Allocator<T>::allocate(size_type n) {
	size_type size = sizeof(T) * n;/*calculate the total size of memory needed*/
	/*if the total memory is too large,then just new*/
	int index=std::ceil((double)size / BASE)-1;/*calculate the index in the freelist*/
	if (free[index] == nullptr)/*if there is no spare block*/
		add(index, 5);
	Block *last = free[index];/*pick one block from the freelist to allocate*/
	free[index] = free[index]->next;
	return reinterpret_cast<pointer>(last);
}

template <typename T>
void Allocator<T>::deallocate(pointer p, size_type n) {
	if (p == nullptr) return;
	size_type size = sizeof(T) * n;
	/*if the total memory is too large,then just delete*/
	int index = std::ceil((double)size / BASE) - 1;;
	Block *block = reinterpret_cast<Block *>(p);/*allocate the block to the freelist*/
	block->next = free[index];
	free[index] = block;
}

template <typename T>
void Allocator<T>::add(size_type list_index, size_type total) {
	size_type block_size = BASE * (list_index + 1);/*calculate the block size*/
	Block* start = reinterpret_cast<Block *>(mempool(block_size, total));/*get the start address*/
	//std::cout << total << std::endl;
	for (int i = 0; i <= (int)(total)-2; i++) {
		Block* now = reinterpret_cast<Block *>(reinterpret_cast<char *>(start) + i * block_size);/*get the i-th block address*/
		Block* next = reinterpret_cast<Block *>(reinterpret_cast<char *>(start) + (i + 1) * block_size);/*get the i+1-th block address*/
		now->next = next;/*i-th pointer point to the next address*/
	}
	Block* last = reinterpret_cast<Block *>(reinterpret_cast<char *>(start) + (total - 1) * block_size);
	last->next = nullptr;
	free[list_index] = reinterpret_cast<Block *>(reinterpret_cast<char *>(start));
}


template <typename T>
typename Allocator<T>::void_pointer Allocator<T>::mempool(size_type block_size, size_type &total) {
	/*The current remaining memory is enough for a block size*/
	if (tail-head>= block_size) {
		total = std::min(total, (tail - head) / block_size);
		auto result = head;
		head += total * block_size;
		return result;
	}
	else {
		/*increase the size of memorypool*/
		auto size = block_size * total;
		size_type new_size = (std::ceil(((size*2) + ((double)heap_size/4)) / BASE)-1) * BASE;/*calculate the number of new blocks*/
		head = new(char[new_size]);/*get the start address*/
		tail = head + new_size;/*the total memory that this time increases*/
		heap_size += new_size;
		auto last = head;
		head += size;/*some momory is used*/
		return last;
	}
}
#endif 