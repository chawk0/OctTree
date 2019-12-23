/*
	yaaaaay linkedlist
*/

#pragma once

#include <iostream>
#include <cstdlib>

//using namespace std;

template <class T>
class DualLinkedList
{
public:
	DualLinkedList():
		_head(NULL),
		_tail(NULL),
		_size(0)
	{ }

	~DualLinkedList()
	{
		free();
	}

	void append(T val);
	void prepend(T val);
	void insert(T val, int index);

	void erase(int index);
	void free();
	int size() const;
	int find(T val)const;
	T& operator[](int index);

	ostream& printTo(ostream& os) const;

private:
	struct Node
	{
		T val;
		Node* prev;
		Node* next;
	};

	Node* _head;
	Node* _tail;
	int _size;
};

template <class T>
void DualLinkedList<T>::append(T val)
{
	if (_size > 0)
	{
		// link both ways
		_tail->next = new Node;
		_tail->next->prev = _tail;
		_tail->next->next = NULL;

		// move tail
		_tail = _tail->next;

		// set val
		_tail->val = val;
	}
	else
	{
		// when there's only 1 element, the head and tail point to the same spot
		_head = new Node;
		_head->val = val;
		_tail = _head;
	}

	_size++;
}

template <class T>
void DualLinkedList<T>::prepend(T val)
{
	if (_size > 0)
	{
		// link both ways
		_head->prev = new Node;
		_head->prev->next = _head;
		_head->prev->prev = NULL;

		// move head
		_head = _head->prev;

		// set val
		_head->val = val;
	}
	else
	{
		// when there's only 1 element, the head and tail point to the same spot
		_head = new Node;
		_head->val = val;
		_tail = _head;
	}

	_size++;
}

template <class T>
void DualLinkedList<T>::insert(T val, int index)
{
	if (index == 0)
		prepend(val);
	else if (index == _size)
		append(val);
	// loop through, starting from one end
	else if (index > 0 && index < _size)
	{
		Node* ptr;
		Node* newNode;
		int direction;
		int count;
			
		// start at the closer end to the desired index
		if (index <= _size / 2)
		{
			direction = 1;
			ptr = _head;
			count = 0;
		}
		else
		{
			direction = -1;
			ptr = _tail;
			count = _size - 1;
		}

		// loop until found
		while (count != index)
		{
			if (direction == 1)
			{
				ptr = ptr->next;
				count++;
			}
			else
			{
				ptr = ptr->prev;
				count--;
			}
		}

		newNode = new Node;
		// link prev node to the new node
		ptr->prev->next = newNode;
		newNode->prev = ptr->prev;

		// link next node to the new node
		newNode->next = ptr;
		ptr->prev = newNode;

		// set val
		newNode->val = val;
	}
	else
		cout << "BAD insert!" << endl;
}

template <class T>
void DualLinkedList<T>::erase(int index)
{
	cout << "ERROR: DualLinkedList::erase not implemented!" << endl;
}

template <class T>
void DualLinkedList<T>::free()
{
	if (_size > 0)
	{
		Node* current = _head;
		Node* next = current->next;

		// if there's more than 1 node, this should iterate and delete
		while (next != NULL)
		{
			current = next;
			next = current->next;
			delete current;
		}

		_size = 0;
	}
}

template <class T>
int DualLinkedList<T>::size() const
{
	return _size;
}

template <class T>
int DualLinkedList<T>::find(T val) const
{
	cout << "ERROR: DualLinkedList::find not implemented!" << endl;
	return 0;
}

template <class T>
T& DualLinkedList<T>::operator[](int index)
{
	cout << "ERROR: DualLinkedList::operator[] not implemented!" << endl;
	return _head->val;
}

template <class T>
ostream& DualLinkedList<T>::printTo(ostream& os) const
{
	os << "(";
	if (_size > 0)
	{
		Node* current = _head;

		while (1)
		{
			os << current->val;
			if (current->next != NULL)
			{
				os << ", ";
				current = current->next;
			}
			else
				break;
		}
	}
	os << ")";

	return os;
}

template <class T>
ostream& operator<<(ostream& os, const DualLinkedList<T>* list)
{
	return list->printTo(os);
}