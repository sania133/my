#ifndef STATIC_ARRAY_H
#define STATIC_ARRAY_H

#include <iterator>
#include <stdexcept>
#include <new>
#include <memory>

template <typename T, size_t NodeMaxSize>
class StaticArray
{
private:
    alignas(alignof(T)) std::byte elements[NodeMaxSize * sizeof(T)];
    size_t count;

    T *element_ptr(size_t index) noexcept
    {
        return std::launder(reinterpret_cast<T *>(&elements[index * sizeof(T)]));
    }

    const T *element_ptr(size_t index) const noexcept
    {
        return std::launder(reinterpret_cast<const T *>(&elements[index * sizeof(T)]));
    }

public:
    class iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T *;
        using reference = T &;

        iterator(StaticArray *array, size_t ind) : array_(array), ind_(ind) {}

        reference operator*() const { return *array_->element_ptr(ind_); }
        pointer operator->() const { return array_->element_ptr(ind_); }

        iterator &operator++()
        {
            ++ind_;
            return *this;
        }
        iterator operator++(int)
        {
            iterator temp = *this;
            ++ind_;
            return temp;
        }
        iterator &operator--()
        {
            --ind_;
            return *this;
        }
        iterator operator--(int)
        {
            iterator temp = *this;
            --ind_;
            return temp;
        }

        bool operator==(const iterator &other) const { return array_ == other.array_ && ind_ == other.ind_; }
        bool operator!=(const iterator &other) const { return !(*this == other); }
        difference_type operator-(const iterator &other) const { return ind_ - other.ind_; }

    private:
        StaticArray *array_;
        size_t ind_;
        friend class StaticArray;
    };

    class const_iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T *;
        using reference = const T &;

        const_iterator(const StaticArray *array, size_t idx) : array_(array), ind_(idx) {}
        const_iterator(const iterator &it) : array_(it.array_), ind_(it.ind_) {}

        reference operator*() const { return *array_->element_ptr(ind_); }
        pointer operator->() const { return array_->element_ptr(ind_); }

        const_iterator &operator++()
        {
            ++ind_;
            return *this;
        }
        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++ind_;
            return tmp;
        }
        const_iterator &operator--()
        {
            --ind_;
            return *this;
        }
        const_iterator operator--(int)
        {
            const_iterator tmp = *this;
            --ind_;
            return tmp;
        }

        bool operator==(const const_iterator &other) const { return array_ == other.array_ && ind_ == other.ind_; }
        bool operator!=(const const_iterator &other) const { return !(*this == other); }
        difference_type operator-(const const_iterator &other) const { return ind_ - other.ind_; }

    private:
        const StaticArray *array_;
        size_t ind_;
        friend class StaticArray;
    };

    StaticArray() : count(0)
    {
        static_assert(NodeMaxSize > 0, "StaticArray capacity must be greater than 0");
    }

    ~StaticArray()
    {
        clear();
    }

    void push_back(const T &value)
    {
        if (count >= NodeMaxSize)
        {
            throw std::out_of_range("StaticArray capacity exceeded");
        }
        new (element_ptr(count)) T(value);
        ++count;
    }

    void erase(size_t index)
    {
        if (index >= count)
        {
            throw std::out_of_range("Index out of range");
        }
        element_ptr(index)->~T();
        for (size_t i = index; i < count - 1; ++i)
        {
            new (element_ptr(i)) T(*element_ptr(i + 1));
            element_ptr(i + 1)->~T();
        }
        --count;
    }

    void insert(size_t index, const T &value)
    {
        if (count >= NodeMaxSize)
        {
            throw std::out_of_range("StaticArray capacity exceeded");
        }
        if (index > count)
        {
            throw std::out_of_range("Index out of range");
        }
        for (size_t i = count; i > index; --i)
        {
            new (element_ptr(i)) T(*element_ptr(i - 1));
            element_ptr(i - 1)->~T();
        }
        new (element_ptr(index)) T(value);
        ++count;
    }

    T &front()
    {
        if (count == 0)
        {
            throw std::out_of_range("StaticArray is empty");
        }
        return *element_ptr(0);
    }

    const T &front() const
    {
        if (count == 0)
        {
            throw std::out_of_range("StaticArray is empty");
        }
        return *element_ptr(0);
    }

    T &back()
    {
        if (count == 0)
        {
            throw std::out_of_range("StaticArray is empty");
        }
        return *element_ptr(count - 1);
    }

    const T &back() const
    {
        if (count == 0)
        {
            throw std::out_of_range("StaticArray is empty");
        }
        return *element_ptr(count - 1);
    }

    void pop_back()
    {
        if (count == 0)
        {
            throw std::out_of_range("StaticArray is empty");
        }
        element_ptr(count - 1)->~T();
        --count;
    }

    void clear() noexcept
    {
        for (size_t i = 0; i < count; ++i)
        {
            element_ptr(i)->~T();
        }
        count = 0;
    }

    size_t size() const noexcept { return count; }
    size_t capacity() const noexcept { return NodeMaxSize; }
    bool empty() const noexcept { return count == 0; }
    bool full() const noexcept { return count == NodeMaxSize; }

    iterator begin() noexcept { return iterator(this, 0); }
    iterator end() noexcept { return iterator(this, count); }
    const_iterator begin() const noexcept { return const_iterator(this, 0); }
    const_iterator end() const noexcept { return const_iterator(this, count); }
    const_iterator cbegin() const noexcept { return const_iterator(this, 0); }
    const_iterator cend() const noexcept { return const_iterator(this, count); }
};

#endif