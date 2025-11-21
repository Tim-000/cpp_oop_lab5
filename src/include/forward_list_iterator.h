#pragma once
#include <iterator>
#include <cstddef>

template <typename T>
struct Forward_ListIteratorNode;

template <typename T>
class Forward_ListIterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T *;
    using reference = T &;

    Forward_ListIterator(Forward_ListIteratorNode<T> *n = nullptr) : node_(n) {}

    reference operator*() const { return node_->value; }
    pointer operator->() const { return &node_->value; }

    Forward_ListIterator &operator++()
    {
        node_ = node_->next;
        return *this;
    }
    Forward_ListIterator operator++(int)
    {
        Forward_ListIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const Forward_ListIterator &other) const { return node_ == other.node_; }
    bool operator!=(const Forward_ListIterator &other) const { return node_ != other.node_; }

private:
    Forward_ListIteratorNode<T> *node_;
};
