#ifndef CCNSL_ARRAY
#define CCNSL_ARRAY
#include<iostream>
#include<string> 
#include<cmath>
#include<limits>
#include<iomanip>
#include<fstream>
#include<vector>
#include<stdexcept>
#include<sstream>
#include<memory>
#include<algorithm>
#include<limits> 
#include<thread>    
#include<chrono>    
namespace SyLib{

template <typename T>
class arraylt {
private:
    T* data;
    size_t m_size;
    size_t m_capacity;

public:
    // 构造函数
    arraylt() : data(nullptr), m_size(0), m_capacity(0) {}

    explicit arraylt(size_t size) : data(new T[size]), m_size(size), m_capacity(size) {
        for (size_t i = 0; i < size; ++i) {
            data[i] = T();
        }
    }

    arraylt(size_t size, const T& value) : data(new T[size]), m_size(size), m_capacity(size) {
        for (size_t i = 0; i < size; ++i) {
            data[i] = value;
        }
    }

    // 拷贝构造函数
    arraylt(const arraylt& other) : m_size(other.m_size), m_capacity(other.m_capacity) {
        data = new T[m_capacity];
        for (size_t i = 0; i < m_size; ++i) {
            data[i] = other.data[i];
        }
    }

    // 移动构造函数
    arraylt(arraylt&& other) noexcept : data(other.data), m_size(other.m_size), m_capacity(other.m_capacity) {
        other.data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }

    // 析构函数
    ~arraylt() {
        delete[] data;
    }

    // 赋值运算符
    arraylt& operator=(const arraylt& other) {
        if (this != &other) {
            delete[] data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            data = new T[m_capacity];
            for (size_t i = 0; i < m_size; ++i) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    // 移动赋值运算符
    arraylt& operator=(arraylt&& other) noexcept {
        if (this != &other) {
            delete[] data;
            data = other.data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            other.data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    // 迭代器
    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() { return data; }
    const_iterator begin() const { return data; }
    iterator end() { return data + m_size; }
    const_iterator end() const { return data + m_size; }

    // 容量相关
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    bool empty() const { return m_size == 0; }

    void reserve(size_t new_capacity) {
        if (new_capacity > m_capacity) {
            T* new_data = new T[new_capacity];
            for (size_t i = 0; i < m_size; ++i) {
                new_data[i] = std::move(data[i]);
            }
            delete[] data;
            data = new_data;
            m_capacity = new_capacity;
        }
    }

    void resize(size_t new_size, const T& value = T()) {
        if (new_size > m_capacity) {
            reserve(std::max(new_size, m_capacity * 2));
        }
        if (new_size > m_size) {
            for (size_t i = m_size; i < new_size; ++i) {
                data[i] = value;
            }
        }
        m_size = new_size;
    }

    void shrink_to_fit() {
        if (m_size < m_capacity) {
            T* new_data = new T[m_size];
            for (size_t i = 0; i < m_size; ++i) {
                new_data[i] = data[i];
            }
            delete[] data;
            data = new_data;
            m_capacity = m_size;
        }
    }

    // 元素访问
    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }

    T& at(size_t index) {
        if (index >= m_size) {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }

    const T& at(size_t index) const {
        if (index >= m_size) {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }

    T& front() { return data[0]; }
    const T& front() const { return data[0]; }
    T& back() { return data[m_size - 1]; }
    const T& back() const { return data[m_size - 1]; }

    // 修改器
    void push_back(const T& value) {
        if (m_size >= m_capacity) {
            reserve(m_capacity == 0 ? 1 : m_capacity * 2);
        }
        data[m_size++] = value;
    }

    void pop_back() {
        if (m_size > 0) {
            --m_size;
        }
    }
	template<typename... Args>
	void emplace_back(Args&&... args) {
    	if (m_size >= m_capacity) {
    	    reserve(m_capacity == 0 ? 1 : m_capacity * 2);
    	}
    	// 直接在内存中构造元素
    	new (data + m_size) T(std::forward<Args>(args)...);
	    ++m_size;
	}
    // 插入元素
    iterator insert(iterator pos, const T& value) {
        size_t index = pos - begin();
        if (m_size >= m_capacity) {
            size_t new_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
            T* new_data = new T[new_capacity];
            
            // 复制前半部分
            for (size_t i = 0; i < index; ++i) {
                new_data[i] = data[i];
            }
            
            // 插入新元素
            new_data[index] = value;
            
            // 复制后半部分
            for (size_t i = index; i < m_size; ++i) {
                new_data[i + 1] = data[i];
            }
            
            delete[] data;
            data = new_data;
            m_capacity = new_capacity;
        } else {
            // 移动元素为插入腾出位置
            for (size_t i = m_size; i > index; --i) {
                data[i] = data[i - 1];
            }
            data[index] = value;
        }
        ++m_size;
        return begin() + index;
    }

    // 删除元素
    iterator erase(iterator pos) {
        size_t index = pos - begin();
        if (index >= m_size) {
            return end();
        }
        
        // 移动元素覆盖要删除的元素
        for (size_t i = index; i < m_size - 1; ++i) {
            data[i] = data[i + 1];
        }
        --m_size;
        return begin() + index;
    }

    iterator erase(iterator first, iterator last) {
        if (first == last) {
            return first;
        }
        
        size_t start = first - begin();
        size_t count = last - first;
        size_t new_size = m_size - count;
        
        // 移动元素覆盖要删除的范围
        for (size_t i = start; i < new_size; ++i) {
            data[i] = data[i + count];
        }
        
        m_size = new_size;
        return begin() + start;
    }

    void clear() {
    	for (size_t i = 0; i < m_size; ++i) {
    	    data[i].~T();
    	}
    	m_size = 0;
	}
};
}
#endif
