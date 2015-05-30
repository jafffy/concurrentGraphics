#ifndef ATOMIC_VECTOR_H_
#define ATOMIC_VECTOR_H_

#include <atomic>
#include <vector>

// http://stackoverflow.com/questions/13193484/how-to-declare-a-vector-of-atomic-in-c
template <typename T>
struct atomwrapper
{
	std::atomic<T> _a;

	atomwrapper()
		: _a()
	{}

	atomwrapper(const T& data)
		: _a(data)
	{}

	atomwrapper(const std::atomic<T> &a)
		: _a(a.load())
	{}

	atomwrapper(const atomwrapper &other)
		: _a(other._a.load())
	{}

	atomwrapper &operator=(const atomwrapper &other)
	{
		_a.store(other._a.load());
	}

	T get() { return _a.load(); }
	const T get() const { return _a.load(); }
};

template <typename T>
using atomic_vector = std::vector < atomwrapper<T> > ;

#endif // ATOMIC_VECTOR_H_