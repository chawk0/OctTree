/*
	a wee lad
*/

#pragma once

#include <cstdlib>

template <class T>
class Array
{
public:
	Array<T>();
	Array(const Array& a);
	~Array<T>();	

	// index operator
	T& operator[](uint i);
	// assignment operator
	const Array& operator=(const Array& a);

	void alloc(uint64 size);
	void free();
	void clear();
	void set(T val);
	void test(Array* original);
	uint64 size() const;

private:
	T* _buffer;
	uint64 _size;
};

template <class T>
Array<T>::Array(): _buffer(NULL), _size(0)
{
	//
}

template <class T>
Array<T>::Array(const Array& a)
{
	if (a.size() > 0)
	{
		_buffer = new T[a._size];
		memcpy(_buffer, a._buffer, a.size() * sizeof(T));
		_size = a.size();
	}
	else
	{
		_buffer = NULL;
		_size = 0;
	}
}

template <class T>
Array<T>::~Array()
{
	free();
}

template <class T>
void Array<T>::alloc(uint64 size)
{
	if (size > 0)
	{
		if (_buffer != NULL)
			delete [] _buffer;
		_buffer = new T[size];
		_size = size;
	}
}

template <class T>
void Array<T>::free()
{
	if (_buffer != NULL)
	{
		delete [] _buffer;
		_buffer = NULL;
		_size = 0;
	}
}

template <class T>
uint64 Array<T>::size() const
{
	return _size;
}

template <class T>
void Array<T>::clear()
{
	if (_buffer != NULL)
		memset(_buffer, 0, _size * sizeof(T));
}

template <class T>
void Array<T>::set(T val)
{
	if (_buffer != NULL)
		memset(_buffer, val, _size * sizeof(T));
}

template <class T>
void Array<T>::test(Array* original)
{
	cout << original->_size << endl;
}

template <class T>
T& Array<T>::operator[](uint i)
{
	return _buffer[i];
}

template <class T>
const Array<T>& Array<T>::operator=(const Array& a)
{
	uint64 size = a.size();
	if (size > 0)
	{
		if (_buffer != NULL)
			delete [] _buffer;
		_buffer = new T[size];
		_size = size;
		memcpy(_buffer, a._buffer, _size * sizeof(T));
	}

	return *this;
}