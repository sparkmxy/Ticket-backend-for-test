#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.h"

#include <climits>
#include <cstddef>

namespace sjtu {
	/**
	 * a data container like std::vector
	 * store data in a successive memory and support random access.
	 */
	template<typename T>
	class vector {
		size_t maxsize;
		size_t len;
		T *head;
		void dele() {
			for (size_t i = 0; i < len; i++)
				(head + i)->~T();
			operator delete[]((void *)head, sizeof(T) * maxsize);
		}
		void extend() {
			T *temp = (T*)operator new[](sizeof(T) * maxsize * 2);
			for (size_t i = 0; i < len; i++) new(temp + i) T(*(head + i));
			operator delete[](head, sizeof(T) * maxsize);
			head = temp, maxsize *= 2;
		}
	public:
		class const_iterator;
		class iterator {
			friend vector;
			friend const_iterator;
		private:
			size_t pos;
			vector *it;
			iterator(size_t p, vector *o) :pos(p), it(o) {}
			void del() {
				(it->head + pos)->~T();
			}
			void ini(const T& o) {
				new(it->head + pos) T(o);
			}
		public:
			iterator() :pos(0), it(nullptr) {}
			iterator(const iterator &other) :pos(other.pos), it(other.it) {}
			iterator operator+(const int &n) const {
				if (pos + n > it->len) throw(runtime_error());
				return iterator(pos + n, it);
			}
			iterator operator-(const int &n) const {
				if (pos < n) throw(runtime_error());
				return iterator(pos - n, it);
			}
			// return th distance between two iterator,
			// if these two iterators points to different vectors, throw invaild_iterator.
			int operator-(const iterator &rhs) const {
				if (it != rhs.it) throw(invalid_iterator());
				return pos - rhs.pos;
			}
			iterator operator+=(const int &n) {
				if (pos + n >= it->len) throw(runtime_error());
				pos += n;
				return *this;
			}
			iterator operator-=(const int &n) {
				if (pos < n) throw(runtime_error());
				pos -= n;
				return *this;
			}

			iterator operator++(int) {
				size_t temp = pos;
				if (pos >= it->len) throw(runtime_error());
				pos++;
				return iterator(temp, it);
			}

			iterator& operator++() {
				if (pos >= it->len) throw(runtime_error());
				pos++;
				return *this;
			}

			iterator operator--(int) {
				size_t temp = pos;
				if (!pos) throw(runtime_error());
				pos--;
				return iterator(temp, it);
			}

			iterator& operator--() {
				if (!pos) throw(runtime_error());
				pos--;
				return *this;
			}

			T& operator*() const {
				return *(it->head + pos);
			}
			/**
			 * a operator to check whether two iterators are same (pointing to the same memory).
			 */
			bool operator==(const iterator &rhs) const {
				return (it == rhs.it && pos == rhs.pos);
			}
			bool operator==(const const_iterator &rhs) const {
				return (it == rhs.it && pos == rhs.pos);
			}
			/**
			 * some other operator for iterator.
			 */
			bool operator!=(const iterator &rhs) const {
				return (it != rhs.it || pos != rhs.pos);
			}
			bool operator!=(const const_iterator &rhs) const {
				return (it != rhs.it || pos != rhs.pos);
			}
		};
		class const_iterator {
			friend vector;
			friend iterator;
		private:
			size_t pos;
			const vector *it;
			const_iterator(size_t p, const vector *o) :pos(p), it(o) {}
		public:
			const_iterator() :pos(0), it(nullptr) {}
			const_iterator(const const_iterator &other) :pos(other.pos), it(other.it) {}
			iterator operator+=(const int &n) {
				if (pos + n >= it->len) throw(runtime_error());
				pos += n;
				return *this;
			}
			const_iterator operator-=(const int &n) {
				if (pos < n) throw(runtime_error());
				pos -= n;
				return *this;
			}
			const_iterator operator++(int) {
				size_t temp = pos;
				if (pos >= it->len) throw(runtime_error());
				pos++;
				return const_iterator(temp, it);
			}
			const_iterator& operator++() {
				if (pos >= it->len) throw(runtime_error());
				pos++;
				return *this;
			}
			const_iterator operator--(int) {
				size_t temp = pos;
				if (!pos) throw(runtime_error());
				pos--;
				return const_iterator(temp, it);
			}
			const_iterator& operator--() {
				if (!pos) throw(runtime_error());
				pos--;
				return *this;
			}
			int operator-(const const_iterator &rhs) const {
				if (it != rhs.it) throw(invalid_iterator());
				return pos - rhs.pos;
			}
			const T& operator*() const {
				return *(it->head + pos);
			}
			bool operator==(const const_iterator &rhs) const {
				return (it == rhs.it && pos == rhs.pos);
			}
			bool operator==(const iterator &rhs) const {
				return (it == rhs.it && pos == rhs.pos);
			}
			bool operator!=(const iterator &rhs) const {
				return (it != rhs.it || pos != rhs.pos);
			}
			bool operator!=(const const_iterator &rhs) const {
				return (it != rhs.it || pos != rhs.pos);
			}
		};
		vector() :maxsize(256), len(0) {
			head = (T*)operator new[](sizeof(T) * maxsize);
		}
		vector(const vector &other) :maxsize(other.maxsize), len(other.len) {
			head = (T*)operator new[](sizeof(T) * maxsize);
			for (size_t i = 0; i < len; i++)
				*(head + i) = *(other.head + i);
		}
		vector(vector &&other) :maxsize(other.maxsize), len(other.len) {
			head = other.head;
			other.head = nullptr;
			other.len = 0;
		}
		~vector() {
			for (size_t i = 0; i < len; i++)
				(head + i)->~T();
			operator delete[](head, sizeof(T) * maxsize);
		}
		vector &operator=(const vector &other) {
			if (head == other.head) return *this;
			dele();
			maxsize = other.maxsize, len = other.len;
			head = (T*)operator new[](sizeof(T) * maxsize);
			for (size_t i = 0; i < len; i++) *(head + i) = *(other.head + i);
			return *this;
		}
		/**
		 * assigns specified element with bounds checking
		 * throw index_out_of_bound if pos is not in [0, size)
		 */
		T & at(const size_t &pos) {
			if (pos >= len) throw(index_out_of_bound());
			return *(head + pos);
		}
		const T & at(const size_t &pos) const {
			if (pos >= len) throw(index_out_of_bound());
			return *(head + pos);
		}
		/**
		 * assigns specified element with bounds checking
		 * throw index_out_of_bound if pos is not in [0, size)
		 * !!! Pay attentions
		 *   In STL this operator does not check the boundary but I want you to do.
		 */
		T & operator[](const size_t &pos) {
			if (pos >= len) throw(index_out_of_bound());
			return *(head + pos);
		}
		const T & operator[](const size_t &pos) const {
			if (pos >= len) throw(index_out_of_bound());
			return *(head + pos);
		}
		/**
		 * access the first element.
		 * throw container_is_empty if size == 0
		 */
		const T & front() const {
			if (len == 0) throw(container_is_empty());
			return *head;
		}
		/**
		 * access the last element.
		 * throw container_is_empty if size == 0
		 */
		const T & back() const {
			if (len == 0) throw(container_is_empty());
			return *(head + len - 1);
		}
		/**
		 * returns an iterator to the beginning.
		 */
		iterator begin() {
			return iterator(0, this);
		}
		const_iterator cbegin() const {
			return const_iterator(0, this);
		}
		/**
		 * returns an iterator to the end.
		 */
		iterator end() {
			return iterator(len, this);
		}
		const_iterator cend() const {
			return const_iterator(len, this);
		}
		/**
		 * checks whether the container is empty
		 */
		bool empty() const {
			return !len;
		}
		/**
		 * returns the number of elements
		 */
		size_t size() const {
			return len;
		}
		/**
		 * returns the number of elements that can be held in currently allocated storage.
		 */
		size_t capacity() const {
			return maxsize;
		}
		/**
		 * clears the contents
		 */
		void clear() {
			for (size_t i = 0; i < len; i++)
				(head + i)->~T();
			operator delete[]((void *)head, sizeof(T) * maxsize);
			head = operator new[](sizeof(T) * maxsize);
			len = 0;
		}
		/**
		 * inserts value before pos
		 * returns an iterator pointing to the inserted value.
		 */
		iterator insert(iterator pos, const T &value) {
			if (len + 1 >= maxsize) extend();
			len++;
			(this->end() - 1).ini(*(this->end() - 2));
			for (auto i = (this->end() - 1); i != pos; i--) *i = *(i - 1);
			*pos = value;
			return pos;
		}
		/**
		 * inserts value at index ind.
		 * after inserting, this->at(ind) == value is true
		 * returns an iterator pointing to the inserted value.
		 * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
		 */
		iterator insert(const size_t &ind, const T &value) {
			if (len + 1 >= maxsize) extend();
			len++;
			new(head + len - 1) T(*(head + len - 2));
			for (size_t i = len - 2; i > ind; i--) *(head + i) = *(head + i - 1);
			new(head + ind) T(value);
			return iterator(ind, this);
		}
		/**
		 * removes the element at pos.
		 * return an iterator pointing to the following element.
		 * If the iterator pos refers the last element, the end() iterator is returned.
		 */
		iterator erase(iterator pos) {
			len--;
			for (auto i = pos; i != this->end(); i++) *i = *(i + 1);
			this->end().del();
			return pos;
		}
		/**
		 * removes the element with index ind.
		 * return an iterator pointing to the following element.
		 * throw index_out_of_bound if ind >= size
		 */
		iterator erase(const size_t &ind) {
			len--;
			for (auto i = ind; i < len; i++) *(head + i) = *(head + i + 1);
			(head + len)->~T();
			return iterator(ind, this);
		}
		/**
		 * adds an element to the end.
		 */
		void push_back(const T &value) {
			if (len + 1 >= maxsize) extend();
			new(head + len) T(value);
			len++;
		}
		/**
		 * remove the last element from the end.
		 * throw container_is_empty if size() == 0
		 */
		void pop_back() {
			len--;
			(head + len)->~T();
		}
	};


}

#endif