/* $Id: fqueue.hpp,v 1.1 2002/03/02 23:50:30 martin Exp $
  
   (C) Martin Ankerl
   Permission to copy, use, modify, sell and distribute this software
   is granted provided this copyright notice appears in all copies.
   This software is provided "as is" without express or implied
   warranty, and with no claim as to its suitability for any purpose.
  
   This is an implementation of a queue with a fixed maximum size. I
   tried to make the queue as fast as possible. As in std::queue, the
   caller has to ensure that the queue has not 0 elements when he
   pop()'s, or more elements than max_size when he uses push().
  
   Example usage:

   #include <iostream>
   #include "fqueue.hpp"
   using namespace std;   
   int main(int argc, char* argv[]) {
     FQueue<int, 1234> q;          // creates a queue with max. 2047 elements
     cout << q.max_size() << endl;  // output: 2047
     q.push(22);             
     q.push(11);
     cout << q.front() << endl;    // output: 22
     cout << q.back() << endl;     // output: 11
     cout << q.size() << endl;     // output: 2
     q.pop();
     cout << q.back() << endl;     // output: 11
     return(0);
   }

   One should keep the size of the FQueue as close to the real maximum
   as possible, this will make it even faster (e.g. do not create an
   FQueue<int, 1000> when you only need 10 elements)

   Diagram:
           ^           ^
           |back()     |front()
           v           v
    push()+-------------+ pop()
     ---->|   fqueue    |---->
          +-------------+

*/
#ifndef FQUEUE_HPP
#define FQUEUE_HPP
#include <cstddef>
using namespace std;

// tricky calculation of the size of the queue: everything done by the
// compiler
template<size_t N> struct calcSize {
    enum { result = calcSize<(N >> 1)>::result << 1 };
};

template<> struct calcSize<0> {
    enum { result = 1 };
};

template<class T, size_t N>
class FQueue 
{
public:
    //// Type Definitions /////////////////////////////////////////////////////
    // The type of the elements
    typedef T value_type;	
    // the unsigned integral type of size values
    typedef size_t size_type;

    //// Operations ///////////////////////////////////////////////////////////
    // default constructor: create an empty queue
    FQueue()
        : sentinel(elems+maxSize1),
	  head(elems),
	  tail(elems)
    {}

    // return the current number of elements
    size_type size() const
    {
	// this one looks pretty obscure, but it is correct :-)
	return ((head-tail) & maxSize);
    }

    // max_size is constant
    size_type max_size() const
    {
	return(N);
    }

    // return whether the queue is empty
    bool empty() const
    {
	return(!size());
    }

    // Remove the next element from the queue. It has no return value.
    // The caller has to ensure that the queue contains an element,
    // otherwise, the behaviour is undefined.
    void push(const value_type& x)
    {
	*head=x;
	if (++head==sentinel) head=elems;
    }

    // for those who can not have it fast enough, instead of using
    // pop() and than push(...), you can use popAndPush(...), it saves
    // about 2 MOV's :-)
    void popAndPush(const value_type& x)
    {
 	*head=x;
        if (++tail==sentinel) tail=elems;
	if (++head==sentinel) head=elems;
    }

    // only add value when queue is not full
    void savePush(const value_type& x)
    {
	if (size()!=N) push(x);
    }
	
    // return next element of the queue. The caller has to ensure that
    // the queue contains an element, otherwise, the behaviour is
    // undefined.
    value_type& front()
    {
	return(*tail);
    }
    const value_type& front() const
    {
	return(*tail);
    }

    // return last element of the queue. The caller has to ensure that
    // the queue contains an element, otherwise, the behaviour is
    // undefined.
    value_type& back()
    {
	return(*head);
    }
    const value_type& back() const
    {
	return(*head);
    }

    // Remove the next element from the queue. It has no return value.
    // The caller has to ensure that the queue contains an element,
    // otherwise, the behaviour is undefined.
    void pop()
    {
        // no delete: elements are copied when inserted into the queue
	if (++tail==sentinel) tail=elems;
    }

    // Clear: empties the queue. Elements are not destroyed.
    void clear()
    {
        tail = head;
    }

private:
    // The queue should take at least 16 elements to be able to work
    // at full speed
    enum { maxSize = calcSize<(N > 16 ? N : 16)>::result - 1 };
    enum { maxSize1 = maxSize + 1 };
    T elems[maxSize1]; // fixed-size array of elements of type T
    T const * const sentinel;
    T* head;
    T* tail;
};

#endif
