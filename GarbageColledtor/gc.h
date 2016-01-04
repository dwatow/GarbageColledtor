#include <iostream>
#include <list>
#include <typeinfo>
#include <cstdlib>

using namespace std;

class OutOfRangeException {};

template<class T>
class Iter
{
	T *ptr;
	T *end;
	T *begin;
	unsigned length;
public:
	Iter()
	{
		ptr = end = begin = 0;
		length = 0;
	}

	Iter(T *p, T *first, T *last)
	{
		ptr = p;
		begin = first;
		end = last;
		length = last - first;
	}

	unsigned size()
	{
		return length;
	}

	T &operator*()
	{
		if ( (ptr >= end) || (ptr < begin))
			throw OutOfRangeException();
		return ptr;
	}

	Iter operator++()
	{
		ptr++;
		return *this;
	}

	Iter operator--()
	{
		ptr--;
		return *this;
	}

	Iter operator++(int notused)
	{
		T *tmp = ptr;
		ptr++;
		return Iter<T>(tmp, begin, end);
	}

	Iter operator--(int notused)
	{
		T *tmp = ptr;
		ptr--;
		return Iter<T>(tmp, begin, end);
	}

	T &operator[](int i)
	{
		if ( (i < 0) || (i >= (end - begin)) )
			throw OutOfRangeException();
		return ptr[i];
	}

	bool operator==(Iter op2)
	{
		return ptr == op2.ptr;
	}

	bool operator!=(Iter op2)
	{
		return ptr != op2.ptr;
	}

	bool operator<=(Iter op2)
	{
		return ptr <= op2.ptr;
	}

	bool operator<(Iter op2)
	{
		return ptr < op2.ptr;
	}

	bool operator>(Iter op2)
	{
		return ptr > op2.ptr;
	}

	bool operator>=(Iter op2)
	{
		return ptr >= op2.ptr;
	}

	Iter operator-(int i)
	{
		ptr -= n;
		return *this;
	}

	Iter operator+(int i)
	{
		ptr += i;
		return *this;
	}

	int operator-(Iter<T> &itr2)
	{
		return ptr - itr2.ptr;
	}
};

template<class T>
class GCInfo
{
public:
	unsigned reference_count;
	T *memory_ptr;

	bool is_array;

	unsigned array_size;

	GCInfo(T *mPtr, unsigned size = 0):
	reference_count(1), memory_ptr(mPtr), array_size(size)
	{
		if (size != 0)
			is_array = true;
		else
			is_array = false;
	}
};

//override operator for std::list
template<class T>
bool operator==(const GCInfo<T> &ob1, const GCInfo<T> &ob2)
{
	return (ob1.memory_ptr == ob2.memory_ptr);
}

template <class T, int size = 0>
class GCPtr
{
	static list<GCInfo<T> > gclist;

	T *addr;

	bool is_array;
	unsigned array_size;
	static bool first;

	typename list<GCInfo<T> >::iterator findPtrInfo(T *ptr);
public:

	typedef Iter<T> GCiterator;

	GCPtr(T *t = 0):
	first(false), addr(t), array_size(size)
	{
		if (first)
			atexit(shutdown);

		list<GCInfo<T> >::iterator p;
		p = findPtrInfo(t);

		if (p != gclist.end())
			p->reference_count++;
		else
		{
			GCInfo<T> gcObj(t, size);
			gclist.push_back(gcObj);
		}

		if (size > 0)
			is_array = true;
		else
			is_array = false;

#ifdef DISPLAY
		cout << "Countstructing GCPtr. ";
		if (is_array)
			cout << " Size is " << array_size << endl;
		else
			cout << endl;
#endif
	}

	//copy constructor
	GCPtr(const GCPtr &ob):
	addr(ob.addr), array_size(ob.array_size)
	{
		list<GCInfo<T> >::iterator p;
		p = findPtrInfo(ob.addr);

		p->reference_count++;

		if (array_size > 0)
			is_array = true;
		else
			is_array = false;

#ifdef DISPLAY
		cout << "Countstructing copy. ";
		if (is_array)
			cout << " Size is " << array_size << endl;
		else
			cout << endl;
#endif
	}

	~GCPtr();

	static bool collect();

	T *operator=(T *t);

	GCPtr &operator=(GCPtr &rv);

	T &operator*()
	{
		return *addr;
	}

	T *operator->()
	{
		return addr;
	}

	T &operator[](int i)
	{
		return addr[i];
	}

	operator T*()
	{
		return addr;
	}

	Iter<T> begin()
	{
		int size;

		if (is_array)
			size = array_size; 
		else
			size = 1;

		return Iter<T>(addr, addr , addr + size);
	}

	Iter<T> end()
	{
		int size;
		 if (is_array)
			 size = array_size;
		 else
			 size = 1;
		 
		 return Iter<T>(addr + size, addr, addr + size);
	}

	static int gclistSize()
	{
		return gclist.size();
	}

	static void showlist();

	static void shutdown();
};

//static member value
template <class T, int size>
 list <GCInfo<T> > GCPtr<T, size>::gclist;

 //static member value
template <class T, int size>
bool GCPtr<T, size>::first = true;

template <class T, int size>
GCPtr<T, size>::~GCPtr()
{
	list<GCInfo<T> >::iterator p;
	 p = findPtrInfo(addr);
	 if (p->reference_count)
		 p->reference_count--;

#ifdef DISPLAY
	 cout << "GCPtr going out of scope.\n";
#endif

	 collect();
}

//static member function
template <class T, int size>
bool GCPtr<T, size>::collect()
{
	bool memory_freed = false;
#ifdef DISPLAY
	cout << "Before garbage collection for ";
	showlist();
#endif

	list<GCInfo<T> >::iterator p;
	do 
	{
		for (p = gclist.begin(); p != gclist.end(); ++p)
		{
			if (p->reference_count > 0)
				continue;
			memory_freed = true;

			gclist.remove(*p);

			if (p->memory_ptr)
			{
				if (p->is_array)
				{
#ifdef DISPLAY
					cout << "Deletin array of size "
						 << p->array_size << endl;
#endif
					delete [] p->memory_ptr;
				}
				else
				{
#ifdef DISPLAY
					cout << "Deletin: "
						 << *(T*) p->memory_ptr << endl;
#endif
					delete p->memory_ptr;
				}
			}
			break;
		}
	} while (p != gclist.end());

#ifdef DISPLAY
	cout << "After:garbage collection for ";
	showlist();
#endif
	return memory_freed;
}
//static member function
template <class T, int size>
void GCPtr<T, size>::showlist()
{
	list<GCInfo<T> >::iterator p;

	cout << "gclist<" << typeid(T).name() << ", "
		<< size << ">:" << endl;
	cout << "memory ptr reference count value " << endl;

	if (gclist.begin() == gclist.end())
	{
		cout << " -- empty --" << endl << endl;
		return;
	}

	for (p = gclist.begin(); p != gclist.end(); ++p)
	{
		cout << "[" << (void*)p->memory_ptr << "]"
			<< "    " << p->reference_count << "    ";
		if (p->memory_ptr)
			cout << "   ---" << *p->memory_ptr;
		else
			cout << "   ---";
		cout << endl;
	}

	cout << endl;
}

//static member function
template <class T, int size>
typename list<GCInfo<T> >::iterator GCPtr<T, size>::findPtrInfo(T *ptr)
{
	list<GCInfo<T> >::iterator p;

	for (p = gclist.begin(); p != gclist.end(); ++p)
		if (p->memory_ptr)
			return p;

	return p;
}

//static member function
template <class T, int size>
void GCPtr<T, size>::shutdown()
{
	if (gclistSize() == 0)
		return;

	list<GCInfo<T> >::iterator p;

	for (p = gclist.begin(); p != gclist.end(); ++p)
	{
		p->reference_count = 0;
	}

#ifdef DISPLAY
	cout << "Before collecting for shutdown () for 
		<< typeid(T).name << endl;
#endif
	collect();

#ifdef DISPLAY
	cout << "After collecting for shutdown () for 
		<< typeid(T).name << endl;
#endif
}
//free operator function
template <class T, int size>
T *GCPtr<T, size>::operator=(T *t)
{
	list<GCInfo<T> >::iterator p;

	p = findPtrInfo(addr);
	p->reference_count--;

	p = findPtrInfo(t);
	if (p != gclist.end())
		p->reference_count++;
	else
	{
		GCInfo<T> gcObj(t, size);
		gclist.push_back(gcObj);
	}

	addr = t;
	return t;
}

template <class T, int size>
GCPtr<T, size> & GCPtr<T, size>::operator=(GCPtr &rv)
{
	list<GCInfo<T> >::iterator p;

	p = findPtrInfo(addr);
	p->reference_count--;

	p = findPtrInfo(rv.addr);
	p->reference_count++;

	addr = rv.addr;
	return rv;
}

