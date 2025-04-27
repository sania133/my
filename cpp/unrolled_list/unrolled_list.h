#ifndef UNROLLED_LIST_H
#define UNROLLED_LIST_H

#include "static_array.h"
#include <memory>
#include <iterator>
#include <utility>

template <typename T, std::size_t NodeMaxSize = 10, typename Alloc = std::allocator<T>>
class unrolled_list
{
private:
    struct Node
    {
        StaticArray<T, NodeMaxSize> elements;
        Node *next;
        Node *prev;

        Node() : next(nullptr), prev(nullptr) {}
    };

    using NodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    using NodeAllocTraits = std::allocator_traits<NodeAlloc>;

    Node *head;
    Node *tail;
    std::size_t list_size;
    NodeAlloc node_alloc;

    Node *allocate_node()
    {
        Node *node = NodeAllocTraits::allocate(node_alloc, 1);
        try
        {
            NodeAllocTraits::construct(node_alloc, node);
        }
        catch (...)
        {
            NodeAllocTraits::deallocate(node_alloc, node, 1);
            throw;
        }
        return node;
    }

    void deallocate_node(Node *node) noexcept
    {
        NodeAllocTraits::destroy(node_alloc, node);
        NodeAllocTraits::deallocate(node_alloc, node, 1);
    }

    Node *split_node(Node *node)
    {
        Node *new_node = allocate_node();
        size_t mid = node->elements.size() / 2;
        for (size_t i = mid; i < node->elements.size(); ++i)
        {
            new_node->elements.push_back(node->elements[i]);
        }
        while (node->elements.size() > mid)
        {
            node->elements.pop_back();
        }
        new_node->next = node->next;
        new_node->prev = node;
        if (node->next)
        {
            node->next->prev = new_node;
        }
        else
        {
            tail = new_node;
        }
        node->next = new_node;
        return new_node;
    }

public:
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T &;
    using const_reference = const T &;
    using pointer = typename std::allocator_traits<Alloc>::pointer;
    using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;

    class iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T *;
        using reference = T &;

        iterator(Node *n = nullptr, size_type idx = 0) : node(n), index(idx) {}
        reference operator*() const { return node->elements[index]; }
        pointer operator->() const { return &node->elements[index]; }

        iterator &operator++()
        {
            if (node)
            {
                if (index + 1 < node->elements.size())
                {
                    index++;
                }
                else
                {
                    if (node->next)
                    {
                        node = node->next;
                        index = 0;
                    }
                    else
                    {
                        node = nullptr;
                        index = 0;
                    }
                }
            }
            return *this;
        }

        iterator operator++(int)
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        iterator &operator--()
        {
            if (node && index > 0)
            {
                index--;
            }
            else if (node && node->prev)
            {
                node = node->prev;
                index = node->elements.size() - 1;
            }
            return *this;
        }

        iterator operator--(int)
        {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const iterator &other) const { return node == other.node && index == other.index; }
        bool operator!=(const iterator &other) const { return !(*this == other); }

    private:
        friend class unrolled_list;
        Node *node;
        size_type index;
    };

    class const_iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T *;
        using reference = const T &;

        const_iterator(Node *n = nullptr, size_type idx = 0) : node(n), index(idx) {}
        const_iterator(const iterator &it) : node(it.node), index(it.index) {}

        reference operator*() const { return node->elements[index]; }
        pointer operator->() const { return &node->elements[index]; }

        const_iterator &operator++()
        {
            if (node)
            {
                if (index + 1 < node->elements.size())
                {
                    index++;
                }
                else
                {
                    if (node->next)
                    {
                        node = node->next;
                        index = 0;
                    }
                    else
                    {
                        node = nullptr;
                        index = 0;
                    }
                }
            }
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        const_iterator &operator--()
        {
            if (node && index > 0)
            {
                index--;
            }
            else if (node && node->prev)
            {
                node = node->prev;
                index = node->elements.size() - 1;
            }
            return *this;
        }

        const_iterator operator--(int)
        {
            const_iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const const_iterator &other) const { return node == other.node && index == other.index; }
        bool operator!=(const const_iterator &other) const { return !(*this == other); }

    private:
        friend class unrolled_list;
        Node *node;
        size_type index;
    };

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    unrolled_list() noexcept : head(nullptr), tail(nullptr), list_size(0), node_alloc(Alloc()) {}
    explicit unrolled_list(const Alloc &alloc) noexcept : head(nullptr), tail(nullptr), list_size(0), node_alloc(alloc) {}

    unrolled_list(size_type n, const T &value, const Alloc &alloc = Alloc())
        : head(nullptr), tail(nullptr), list_size(0), node_alloc(alloc)
    {
        try
        {
            for (size_type i = 0; i < n; ++i)
            {
                push_back(value);
            }
        }
        catch (...)
        {
            clear();
            throw;
        }
    }

    template <typename InputIt>
    unrolled_list(InputIt first, InputIt last, const Alloc &alloc = Alloc())
        : head(nullptr), tail(nullptr), list_size(0), node_alloc(alloc)
    {
        Node *current_node = nullptr;
        try
        {
            while (first != last)
            {
                if (!tail || tail->elements.full())
                {
                    Node *new_node = allocate_node();
                    if (!head)
                        head = tail = new_node;
                    else
                    {
                        tail->next = new_node;
                        new_node->prev = tail;
                        tail = new_node;
                    }
                    current_node = new_node;
                }

                try
                {
                    tail->elements.push_back(*first);
                    ++list_size;
                }
                catch (...)
                {
                    if (current_node && current_node->elements.empty())
                    {
                        if (head == tail)
                            head = tail = nullptr;
                        else
                        {
                            tail = tail->prev;
                            tail->next = nullptr;
                        }
                        deallocate_node(current_node);
                    }
                    throw;
                }
                ++first;
            }
        }
        catch (...)
        {
            clear();
            throw;
        }
    }

    unrolled_list(const unrolled_list &other)
        : head(nullptr), tail(nullptr), list_size(0),
          node_alloc(NodeAllocTraits::select_on_container_copy_construction(other.node_alloc))
    {
        Node *current_node = nullptr;
        try
        {
            for (const auto &item : other)
            {
                if (!tail || tail->elements.full())
                {
                    Node *new_node = allocate_node();
                    if (!head)
                        head = tail = new_node;
                    else
                    {
                        tail->next = new_node;
                        new_node->prev = tail;
                        tail = new_node;
                    }
                    current_node = new_node;
                }

                try
                {
                    tail->elements.push_back(item);
                    ++list_size;
                }
                catch (...)
                {
                    if (current_node && current_node->elements.empty())
                    {
                        if (head == tail)
                            head = tail = nullptr;
                        else
                        {
                            tail = tail->prev;
                            tail->next = nullptr;
                        }
                        deallocate_node(current_node);
                    }
                    throw;
                }
            }
        }
        catch (...)
        {
            clear();
            throw;
        }
    }

    unrolled_list(const unrolled_list &other, const Alloc &alloc)
        : head(nullptr), tail(nullptr), list_size(0), node_alloc(alloc)
    {
        Node *current_node = nullptr;
        try
        {
            for (const auto &item : other)
            {
                if (!tail || tail->elements.full())
                {
                    Node *new_node = allocate_node();
                    if (!head)
                        head = tail = new_node;
                    else
                    {
                        tail->next = new_node;
                        new_node->prev = tail;
                        tail = new_node;
                    }
                    current_node = new_node;
                }

                try
                {
                    tail->elements.push_back(item);
                    ++list_size;
                }
                catch (...)
                {
                    if (current_node && current_node->elements.empty())
                    {
                        if (head == tail)
                            head = tail = nullptr;
                        else
                        {
                            tail = tail->prev;
                            tail->next = nullptr;
                        }
                        deallocate_node(current_node);
                    }
                    throw;
                }
            }
        }
        catch (...)
        {
            clear();
            throw;
        }
    }

    unrolled_list &operator=(const unrolled_list &other)
    {
        if (this != &other)
        {
            clear();
            if (NodeAllocTraits::propagate_on_container_copy_assignment::value)
            {
                node_alloc = other.node_alloc;
            }
            for (const auto &elem : other)
            {
                push_back(elem);
            }
        }
        return *this;
    }

    reference front() { return head->elements.front(); }
    const_reference front() const { return head->elements.front(); }
    reference back() { return tail->elements.back(); }
    const_reference back() const { return tail->elements.back(); }

    iterator begin() noexcept { return head ? iterator(head, 0) : iterator(); }
    iterator end() noexcept { return iterator(); }
    const_iterator begin() const noexcept { return cbegin(); }
    const_iterator end() const noexcept { return cend(); }
    const_iterator cbegin() const noexcept { return head ? const_iterator(head, 0) : const_iterator(); }
    const_iterator cend() const noexcept { return const_iterator(); }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(cend()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(cbegin()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

    size_type size() const noexcept { return list_size; }
    size_type max_size() const noexcept { return std::numeric_limits<size_type>::max(); }
    bool empty() const noexcept { return list_size == 0; }

    void push_back(const T &value)
    {
        Node *new_node = nullptr;
        bool node_allocated = false;

        try
        {
            if (!tail || tail->elements.full())
            {
                new_node = allocate_node();
                node_allocated = true;
                if (!head)
                    head = tail = new_node;
                else
                {
                    new_node->prev = tail;
                    tail->next = new_node;
                    tail = new_node;
                }
            }

            tail->elements.push_back(value);
            ++list_size;
        }
        catch (...)
        {
            if (node_allocated)
            {
                if (tail == new_node)
                {
                    if (head == tail)
                        head = tail = nullptr;
                    else
                    {
                        tail = tail->prev;
                        tail->next = nullptr;
                    }
                }
                deallocate_node(new_node);
            }
            throw;
        }
    }

    void clear() noexcept
    {
        Node *current = head;
        while (current)
        {
            Node *next = current->next;
            current->elements.clear();
            deallocate_node(current);
            current = next;
        }
        head = tail = nullptr;
        list_size = 0;
    }

    void push_front(const T &value)
    {
        if (!head || head->elements.full())
        {
            Node *new_node = allocate_node();
            if (!head)
            {
                head = tail = new_node;
            }
            else
            {
                new_node->next = head;
                head->prev = new_node;
                head = new_node;
            }
        }
        try
        {
            head->elements.insert(0, value);
            ++list_size;
        }
        catch (...)
        {
            if (head->elements.empty() && head != tail)
            {
                head->next->prev = nullptr;
                head = head->next;
                deallocate_node(head->prev);
            }
            throw;
        }
    }

    void pop_back() noexcept
    {
        if (!tail)
            return;
        tail->elements.pop_back();
        --list_size;
        if (tail->elements.empty() && head != tail)
        {
            Node *to_delete = tail;
            tail = tail->prev;
            tail->next = nullptr;
            deallocate_node(to_delete);
        }
    }

    void pop_front() noexcept
    {
        if (!head)
            return;
        head->elements.erase(0);
        --list_size;
        if (head->elements.empty() && head != tail)
        {
            Node *to_delete = head;
            head = head->next;
            head->prev = nullptr;
            deallocate_node(to_delete);
        }
    }

    iterator insert(const_iterator pos, const T &value)
    {
        if (pos == cend())
        {
            push_back(value);
            return iterator(tail, tail->elements.size() - 1);
        }

        Node *node = pos.node;
        if (!node)
        {
            push_front(value);
            return iterator(head, 0);
        }

        if (node->elements.full())
        {
            Node *new_node = split_node(node);
            if (pos.index > node->elements.size())
            {
                node = new_node;
                pos = const_iterator(new_node, pos.index - node->elements.size());
            }
        }

        try
        {
            node->elements.insert(pos.index, value);
            ++list_size;
            return iterator(node, pos.index);
        }
        catch (...)
        {
            throw;
        }
    }

    iterator insert(const_iterator pos, size_type count, const T &value)
    {
        iterator result;
        if (count == 0)
            return pos.node ? iterator(pos.node, pos.index) : end();

        if (pos == cend())
        {
            for (size_type i = 0; i < count; ++i)
            {
                push_back(value);
            }
            return iterator(tail, tail->elements.size() - count);
        }

        Node *node = pos.node;
        if (!node)
        {
            for (size_type i = 0; i < count; ++i)
            {
                push_front(value);
            }
            return iterator(head, 0);
        }

        while (node->elements.size() + count > NodeMaxSize)
        {
            node = split_node(node);
            if (pos.index > node->elements.size())
            {
                pos.index -= node->elements.size();
                node = node->next;
            }
        }

        try
        {
            for (size_type i = 0; i < count; ++i)
            {
                node->elements.insert(pos.index + i, value);
            }
            list_size += count;
            return iterator(node, pos.index);
        }
        catch (...)
        {
            throw;
        }
    }
    template <typename InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last)
    {
        iterator result;
        if (pos == cend())
        {
            for (; first != last; ++first)
            {
                push_back(*first);
            }
            return iterator(tail, tail->elements.size() - std::distance(first, last));
        }

        Node *node = pos.node;
        if (!node)
        {
            for (; first != last; ++first)
            {
                push_front(*first);
            }
            return iterator(head, 0);
        }

        size_t count = std::distance(first, last);
        while (node->elements.size() + count > NodeMaxSize)
        {
            node = split_node(node);
        }

        try
        {
            for (; first != last; ++first)
            {
                node->elements.insert(pos.index++, *first);
                ++list_size;
            }
            return iterator(node, pos.index - count);
        }
        catch (...)
        {
            throw;
        }
    }

    iterator erase(const_iterator pos) noexcept
    {
        if (pos == cend() || !pos.node)
            return end();

        Node *node = pos.node;
        node->elements.erase(pos.index);
        --list_size;

        iterator next_it(node, pos.index);
        if (node->elements.empty() && head != tail)
        {
            if (node == head)
            {
                head = head->next;
                head->prev = nullptr;
            }
            else if (node == tail)
            {
                tail = tail->prev;
                tail->next = nullptr;
            }
            else
            {
                node->prev->next = node->next;
                node->next->prev = node->prev;
            }
            deallocate_node(node);
            if (next_it.node->next)
            {
                next_it = iterator(next_it.node->next, 0);
            }
        }
        else if (next_it.index >= node->elements.size() && node->next)
        {
            next_it = iterator(node->next, 0);
        }

        return next_it;
    }

    iterator erase(const_iterator first, const_iterator last) noexcept
    {
        if (first == last)
            return iterator(first.node, first.index);

        Node *node = first.node;
        size_t count = 0;
        while (first != last)
        {
            node->elements.erase(first.index);
            ++first;
            ++count;
        }
        list_size -= count;

        iterator next_it(node, first.index);
        if (node->elements.empty() && head != tail)
        {
            if (node == head)
            {
                head = head->next;
                head->prev = nullptr;
            }
            else if (node == tail)
            {
                tail = tail->prev;
                tail->next = nullptr;
            }
            else
            {
                node->prev->next = node->next;
                node->next->prev = node->prev;
            }
            deallocate_node(node);
            if (next_it.node->next)
            {
                next_it = iterator(next_it.node->next, 0);
            }
        }
        else if (next_it.index >= node->elements.size() && node->next)
        {
            next_it = iterator(node->next, 0);
        }

        return next_it;
    }

    void swap(unrolled_list &other) noexcept
    {
        Node *tmp_head = head;
        Node *tmp_tail = tail;
        std::size_t tmp_size = list_size;
        NodeAlloc tmp_alloc = node_alloc;

        head = other.head;
        tail = other.tail;
        list_size = other.list_size;
        if (NodeAllocTraits::propagate_on_container_swap::value)
        {
            node_alloc = other.node_alloc;
        }

        other.head = tmp_head;
        other.tail = tmp_tail;
        other.list_size = tmp_size;
        if (NodeAllocTraits::propagate_on_container_swap::value)
        {
            other.node_alloc = tmp_alloc;
        }
    }

    bool operator==(const unrolled_list &other) const
    {
        if (size() != other.size())
            return false;
        auto it1 = begin();
        auto it2 = other.begin();
        for (; it1 != end(); ++it1, ++it2)
        {
            if (*it1 != *it2)
                return false;
        }
        return true;
    }

    bool operator!=(const unrolled_list &other) const { return !(*this == other); }
    allocator_type get_allocator() const noexcept { return node_alloc; }
};

template <typename T, std::size_t N, typename Alloc>
void swap(unrolled_list<T, N, Alloc> &lhs, unrolled_list<T, N, Alloc> &rhs) noexcept
{
    lhs.swap(rhs);
}

#endif