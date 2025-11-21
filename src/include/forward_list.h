#pragma once
#include <memory_resource>
#include <utility>
#include <cstddef>
#include "memory_resource.h"
#include "forward_list_iterator.h"

template <typename T>
struct Forward_ListIteratorNode
{
    T value;
    Forward_ListIteratorNode *next = nullptr;
    template <class... Args>
    Forward_ListIteratorNode(Args &&...args) : value(std::forward<Args>(args)...) {}
};

template <typename T>
class Forward_List
{
public:
    using iterator = Forward_ListIterator<T>;
    using allocator_type = std::pmr::polymorphic_allocator<Forward_ListIteratorNode<T>>;

    explicit Forward_List(std::pmr::memory_resource *mr = std::pmr::new_delete_resource())
        : resource_(mr), node_alloc_(mr) {}

    ~Forward_List() { clear(); }

    void push_front(const T &v) { emplace_front(v); }
    void push_front(T &&v) { emplace_front(std::move(v)); }

    template <class... Args>
    void emplace_front(Args &&...args)
    {
        auto *new_node = node_alloc_.allocate(1);
        try
        {
            std::construct_at(new_node, std::forward<Args>(args)...);
        }
        catch (...)
        {
            node_alloc_.deallocate(new_node, 1);
            throw;
        }
        new_node->next = head_;
        head_ = new_node;
        ++sz_;
    }

    void pop_front()
    {
        if (!head_)
            return;
        auto *old = head_;
        head_ = head_->next;
        old->value.~T();
        node_alloc_.deallocate(old, 1);
        --sz_;
    }

    T &front() { return head_->value; }
    const T &front() const { return head_->value; }

    iterator begin() { return iterator(head_); }
    iterator end() { return iterator(); }

    bool empty() const { return head_ == nullptr; }
    std::size_t size() const { return sz_; }

    void clear()
    {
        while (head_)
            pop_front();
    }

private:
    Forward_ListIteratorNode<T> *head_ = nullptr;
    std::size_t sz_ = 0;
    std::pmr::memory_resource *resource_;
    allocator_type node_alloc_;
};
