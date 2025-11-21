#pragma once
#include <memory_resource>
#include <list>
#include <cstddef>
#include <cstdint>
#include <new>
#include <algorithm>
#include <type_traits>

class FixedPoolResource : public std::pmr::memory_resource
{
public:
    explicit FixedPoolResource(std::size_t bytes)
    {
        total_size_ = align_up(bytes, alignof(std::max_align_t));
        buffer_ = static_cast<std::byte *>(::operator new(total_size_));
        base_addr_ = reinterpret_cast<std::uintptr_t>(buffer_);
        blocks_.push_back(Block{0, total_size_, false});
    }

    ~FixedPoolResource() override
    {
        ::operator delete(buffer_);
    }

    FixedPoolResource(const FixedPoolResource &) = delete;
    FixedPoolResource &operator=(const FixedPoolResource &) = delete;

private:
    struct Block
    {
        std::size_t offset;
        std::size_t size;
        bool used;
    };

    static constexpr std::size_t align_up(std::size_t n, std::size_t a) noexcept
    {
        return (n + a - 1) & ~(a - 1);
    }

    void *do_allocate(std::size_t bytes, std::size_t alignment) override
    {
        if (bytes == 0)
            bytes = 1;
        alignment = std::max<std::size_t>(alignment, alignof(std::max_align_t));

        for (auto it = blocks_.begin(); it != blocks_.end(); ++it)
        {
            if (it->used)
                continue;
            std::uintptr_t seg_addr = base_addr_ + it->offset;
            std::uintptr_t aligned_addr = align_up_uintptr(seg_addr, alignment);
            std::size_t padding = static_cast<std::size_t>(aligned_addr - seg_addr);
            if (padding + bytes <= it->size)
            {
                std::size_t result_offset = it->offset + padding;

                std::size_t prefix = padding;
                std::size_t used_size = bytes;
                std::size_t suffix = it->size - prefix - used_size;

                std::size_t cur_offset = it->offset;
                auto insert_pos = it;
                if (prefix > 0)
                {
                    it->size = prefix;
                    it->used = false;

                    ++it;
                    it = blocks_.insert(it, Block{cur_offset + prefix, used_size, true});
                }
                else
                {

                    it->used = true;
                    it->size = used_size;
                }

                if (suffix > 0)
                {

                    auto after_used = std::next(it);
                    blocks_.insert(after_used, Block{result_offset + used_size, suffix, false});
                }

                void *result_ptr = reinterpret_cast<void *>(base_addr_ + result_offset);
                return result_ptr;
            }
        }
        throw std::bad_alloc();
    }

    void do_deallocate(void *p, std::size_t, std::size_t) noexcept override
    {
        if (!p)
            return;
        std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(p);
        if (addr < base_addr_ || addr >= base_addr_ + total_size_)
        {

            return;
        }
        std::size_t offset = static_cast<std::size_t>(addr - base_addr_);
        for (auto it = blocks_.begin(); it != blocks_.end(); ++it)
        {
            if (it->offset == offset && it->used)
            {
                it->used = false;
                if (it != blocks_.begin())
                {
                    auto prev = std::prev(it);
                    if (!prev->used && prev->offset + prev->size == it->offset)
                    {
                        prev->size += it->size;
                        it = blocks_.erase(it);
                        it = prev;
                    }
                }
                auto next = std::next(it);
                if (next != blocks_.end() && !next->used && it->offset + it->size == next->offset)
                {
                    it->size += next->size;
                    blocks_.erase(next);
                }
                return;
            }
        }
    }

    bool do_is_equal(const std::pmr::memory_resource &other) const noexcept override
    {
        return this == &other;
    }

private:
    static std::uintptr_t align_up_uintptr(std::uintptr_t v, std::size_t a) noexcept
    {
        std::uintptr_t mask = static_cast<std::uintptr_t>(a - 1);
        return (v + mask) & ~mask;
    }

private:
    std::byte *buffer_{nullptr};
    std::uintptr_t base_addr_{0};
    std::size_t total_size_{0};
    std::list<Block> blocks_;
};
