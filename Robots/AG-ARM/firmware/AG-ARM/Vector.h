#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

namespace std 
{

template<class T>
struct Node
{
	T data;
	Node<T> *next;
};

template <typename T>
class Vector{

protected:
	int _size;
	Node<T> *root;
	Node<T>	*last;

	Node<T> *lastNodeGot;
	int lastIndexGot;
	// isCached should be set to FALSE
	// everytime the list suffer changes
	bool isCached;

	Node<T>* getNode(int index);

	Node<T>* findEndOfSortedString(Node<T> *p, int (*cmp)(T &, T &));

public:
	Vector();
	Vector(int sizeIndex, T _t); //使用默认值初始化Vector
	virtual ~Vector();

	/*
		Vector大小
	*/
	virtual int size();
	/*
        添加一个元素,重新制定上下节点
		_size++
	*/
	virtual bool add(int index, T);
	/*
		末尾添加一个元素
		_size++
	*/
	virtual bool push_back(T);
	/*
		最前面添加一个元素
		_size++
	*/
	virtual bool push_front(T);
	/*
		替换元素
	*/
	virtual bool set(int index, T);
	/*
        删除在index处的元素,如果可以访问,则_size--,否则返回false
	*/
	virtual T remove(int index);
	/*
		删除最后一个袁术
	*/
	virtual T pop_back();
	/*
		删除第一个元素
	*/
	virtual T pop_front();
	/*
        获取在index处的元素,如果可以访问,则返回元素
	*/
	virtual T get(int index);

	/*
		清除
	*/
	virtual void clear();

	/*
		根据给定的函数参数,对列表进行排序
	*/
	virtual void sort(int (*cmp)(T &, T &));

	inline T& operator[](int index); 
	inline T& operator[](size_t& i) { return this->get(i); }
  	inline const T& operator[](const size_t& i) const { return this->get(i); }

};

// Initialize Vector with false values
template<typename T>
Vector<T>::Vector()
{
	root=NULL;
	last=NULL;
	_size=0;

	lastNodeGot = root;
	lastIndexGot = 0;
	isCached = false;
}

//析构
template<typename T>
Vector<T>::~Vector()
{
	Node<T>* tmp;
	while(root!=NULL)
	{
		tmp=root;
		root=root->next;
		delete tmp;
	}
	last = NULL;
	_size=0;
	isCached = false;
}


template<typename T>
Node<T>* Vector<T>::getNode(int index){

	int _pos = 0;
	Node<T>* current = root;

	// Check if the node trying to get is
	// immediatly AFTER the previous got one
	if(isCached && lastIndexGot <= index){
		_pos = lastIndexGot;
		current = lastNodeGot;
	}

	while(_pos < index && current){
		current = current->next;

		_pos++;
	}

	// Check if the object index got is the same as the required
	if(_pos == index){
		isCached = true;
		lastIndexGot = index;
		lastNodeGot = current;

		return current;
	}

	return NULL;
}

template<typename T>
int Vector<T>::size(){
	return _size;
}

template<typename T>
Vector<T>::Vector(int sizeIndex, T _t){
	for (int i = 0; i < sizeIndex; i++){
		push_back(_t);
	}
}

template<typename T>
bool Vector<T>::add(int index, T _t){

	if(index >= _size)
		return push_back(_t);

	if(index == 0)
		return push_front(_t);

	Node<T> *tmp = new Node<T>(),
				 *_prev = getNode(index-1);
	tmp->data = _t;
	tmp->next = _prev->next;
	_prev->next = tmp;

	_size++;
	isCached = false;

	return true;
}

template<typename T>
bool Vector<T>::push_back(T _t){

	Node<T> *tmp = new Node<T>();
	tmp->data = _t;
	tmp->next = NULL;
	
	if(root){
		// Already have elements inserted
		last->next = tmp;
		last = tmp;
	}else{
		// First element being inserted
		root = tmp;
		last = tmp;
	}

	_size++;
	isCached = false;

	return true;
}

template<typename T>
bool Vector<T>::push_front(T _t){

	if(_size == 0)
		return push_back(_t);

	Node<T> *tmp = new Node<T>();
	tmp->next = root;
	tmp->data = _t;
	root = tmp;
	
	_size++;
	isCached = false;
	
	return true;
}


template<typename T>
T& Vector<T>::operator[](int index) {
	return getNode(index)->data;
}

template<typename T>
bool Vector<T>::set(int index, T _t){
	// Check if index position is in bounds
	if(index < 0 || index >= _size)
		return false;

	getNode(index)->data = _t;
	return true;
}

template<typename T>
T Vector<T>::pop_back(){
	if(_size <= 0)
		return T();
	
	isCached = false;

	if(_size >= 2){
		Node<T> *tmp = getNode(_size - 2);
		T ret = tmp->next->data;
		delete(tmp->next);
		tmp->next = NULL;
		last = tmp;
		_size--;
		return ret;
	}else{
		// Only one element left on the list
		T ret = root->data;
		delete(root);
		root = NULL;
		last = NULL;
		_size = 0;
		return ret;
	}
}

template<typename T>
T Vector<T>::pop_front(){
	if(_size <= 0)
		return T();

	if(_size > 1){
		Node<T> *_next = root->next;
		T ret = root->data;
		delete(root);
		root = _next;
		_size --;
		isCached = false;

		return ret;
	}else{
		// Only one left, then pop_back()
		return pop_back();
	}

}

template<typename T>
T Vector<T>::remove(int index){
	if (index < 0 || index >= _size)
	{
		return T();
	}

	if(index == 0)
		return pop_front();
	
	if (index == _size-1)
	{
		return pop_back();
	}

	Node<T> *tmp = getNode(index - 1);
	Node<T> *toDelete = tmp->next;
	T ret = toDelete->data;
	tmp->next = tmp->next->next;
	delete(toDelete);
	_size--;
	isCached = false;
	return ret;
}


template<typename T>
T Vector<T>::get(int index){
	Node<T> *tmp = getNode(index);

	return (tmp ? tmp->data : T());
}

template<typename T>
void Vector<T>::clear(){
	while(size() > 0)
		pop_front();
}

template<typename T>
void Vector<T>::sort(int (*cmp)(T &, T &)){
	if(_size < 2) return; // trivial case;

	for(;;) {	

		Node<T> **joinPoint = &root;

		while(*joinPoint) {
			Node<T> *a = *joinPoint;
			Node<T> *a_end = findEndOfSortedString(a, cmp);
	
			if(!a_end->next	) {
				if(joinPoint == &root) {
					last = a_end;
					isCached = false;
					return;
				}
				else {
					break;
				}
			}

			Node<T> *b = a_end->next;
			Node<T> *b_end = findEndOfSortedString(b, cmp);

			Node<T> *tail = b_end->next;

			a_end->next = NULL;
			b_end->next = NULL;

			while(a && b) {
				if(cmp(a->data, b->data) <= 0) {
					*joinPoint = a;
					joinPoint = &a->next;
					a = a->next;	
				}
				else {
					*joinPoint = b;
					joinPoint = &b->next;
					b = b->next;	
				}
			}

			if(a) {
				*joinPoint = a;
				while(a->next) a = a->next;
				a->next = tail;
				joinPoint = &a->next;
			}
			else {
				*joinPoint = b;
				while(b->next) b = b->next;
				b->next = tail;
				joinPoint = &b->next;
			}
		}
	}
}

template<typename T>
Node<T>* Vector<T>::findEndOfSortedString(Node<T> *p, int (*cmp)(T &, T &)) {
	while(p->next && cmp(p->data, p->next->data) <= 0) {
		p = p->next;
	}
	
	return p;
}

}

#endif