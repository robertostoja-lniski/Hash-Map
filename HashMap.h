#ifndef AISDI_MAPS_HASHMAP_H
#define AISDI_MAPS_HASHMAP_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <list>
#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>

#define TABLE_SIZE 1000000

namespace aisdi
{

template <typename KeyType, typename ValueType>
class HashMap
{
      public:
        using key_type = KeyType;
        using mapped_type = ValueType;
        using value_type = std::pair<key_type, mapped_type>;
        using size_type = std::size_t;
        using reference = value_type &;
        using const_reference = const value_type &;
        using list = std::list<value_type>;
        using list_iterator = typename list::const_iterator;
        using vector = std::vector<list>;

        class ConstIterator;

        friend class ConstIterator;

        class Iterator;

        using iterator = Iterator;
        using const_iterator = ConstIterator;

      private:
        size_type beginIndex;
        size_type endIndex;
        size_type size;
        vector table;

      public:
        HashMap() : beginIndex(TABLE_SIZE), endIndex(0), size(0), table(TABLE_SIZE) {}

        HashMap(std::initializer_list<value_type> list) : beginIndex(TABLE_SIZE), endIndex(0), size(0), table(TABLE_SIZE)
        {

                for (auto item : list)
                {
                        key_type key = item.first;
                        mapped_type value = item.second;

                        size_type index = getHash(key);
                        (*this)[key] = value;

                        beginIndex = std::min(beginIndex, index);
                        endIndex = std::max(endIndex, index);
                }
        }

        HashMap(const HashMap &other) : beginIndex(TABLE_SIZE), endIndex(0), size(0), table(TABLE_SIZE)
        {
                *this = other;
        }

        HashMap(HashMap &&other)
        {
                *this = std::move(other);
        }

        HashMap &operator=(const HashMap &other)
        {
                if (*this == other)
                {
                        return *this;
                }
                beginIndex = other.beginIndex;
                endIndex = other.endIndex;
                size = 0;

                for (auto item : other)
                {
                        key_type key = item.first;
                        mapped_type value = item.second;

                        (*this)[key] = value;
                }
                return *this;
        }

        HashMap &operator=(HashMap &&other) noexcept
        {
                if (*this == other)
                {
                        return *this;
                }

                beginIndex = other.beginIndex;
                endIndex = other.endIndex;

                size = other.size;
                table = std::move(other.table);

                return *this;
        }

        bool isEmpty() const
        {
                return !getSize();
        }
        size_type getBeginIndex() const
        {
                return beginIndex;
        }
        size_type getEndIndex() const
        {
                return endIndex;
        }
        mapped_type &operator[](const key_type &key)
        {
                auto found = find(key);

                if (found != end())
                {
                        return found->second;
                }
                else
                {
                        size++;
                        size_type index = getHash(key);
                        table[index].push_back({key, mapped_type{}});

                        beginIndex = std::min(beginIndex, index);
                        endIndex = std::max(endIndex, index);

                        return table[index].back().second;
                }
        }

        const mapped_type &valueOf(const key_type &key) const
        {
                auto found = find(key);

                if (find(key) != end())
                {
                        return found->second;
                }
                else
                {
                        throw std::out_of_range("Element does no exist");
                }
        }

        mapped_type &valueOf(const key_type &key)
        {
                return const_cast<mapped_type &>((static_cast<const HashMap *>(this))->valueOf(key));
        }

        const_iterator find(const key_type &key) const
        {
                size_type index = getHash(key);

                auto const &searchedList = table[index];

                for (auto it = searchedList.begin(); it != searchedList.end(); it++)
                {
                        if (it->first == key)
                                return ConstIterator(this, index, it);
                }
                return end();
        }

        iterator find(const key_type &key)
        {
                return (static_cast<const HashMap *>(this)->find(key));
        }

        void remove(const key_type &key)
        {
                auto found = find(key);
                if (found == end())
                {
                        throw std::out_of_range("Tried to remove undefined element");
                }

                remove(found);
        }

        void remove(const const_iterator &it)
        {
                if (it == end())
                {

                        throw std::out_of_range("Tried to remove end!");
                }
                auto index = it.index;
                auto listIteratorEqualToIt = it.listIterator;
                --size;

                table[index].erase(listIteratorEqualToIt);
        }

        size_type getSize() const
        {
                return size;
        }

        bool operator==(const HashMap &other) const
        {

                if (getSize() != other.getSize())
                {
                        return false;
                }

                auto otherIt = other.begin();

                for (auto thisIt = begin(); thisIt != end(); ++thisIt)
                {

                        if (thisIt->second != otherIt->second)
                        {

                                return false;
                        }

                        ++otherIt;
                }

                return true;
        }

        bool operator!=(const HashMap &other) const
        {
                return !(*this == other);
        }

        iterator begin()
        {
                return Iterator(cbegin());
        }

        iterator end()
        {
                return Iterator(cend());
        }

        const_iterator cbegin() const
        {
                if (isEmpty())
                {
                        return cend();
                }

                return ConstIterator(this, beginIndex, table[beginIndex].begin());
        }

        const_iterator cend() const
        {
                return ConstIterator(this, endIndex, table[endIndex].end());
        }

        const_iterator begin() const
        {
                return cbegin();
        }

        const_iterator end() const
        {
                return cend();
        }

      private:
        size_type getHash(const key_type &key) const
        {
                return (key) % (TABLE_SIZE);
        };
};

template <typename KeyType, typename ValueType>
class HashMap<KeyType, ValueType>::ConstIterator
{
      public:
        using reference = typename HashMap<KeyType, ValueType>::const_reference;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename HashMap<KeyType, ValueType>::value_type;
        using pointer = const typename HashMap<KeyType, ValueType>::value_type *;
        using size_type = typename HashMap<KeyType, ValueType>::size_type;
        using list = typename HashMap<KeyType, ValueType>::list;
        using list_iterator = typename HashMap<KeyType, ValueType>::list_iterator;
        using vector = typename HashMap<KeyType, ValueType>::vector;

        friend class HashMap;

      private:
        const HashMap<KeyType, ValueType> *map;
        // when index == table.capacity then it is HashMap.end()
        size_type index;
        list_iterator listIterator;

      public:
        explicit ConstIterator(const HashMap<KeyType, ValueType> *map, size_type index, list_iterator it = {}) : map(map), index(index), listIterator(it) {}

        ConstIterator(const ConstIterator &other) : map(other.map), index(other.index), listIterator(other.listIterator) {}

        ConstIterator &operator++()
        {

                if (*this == map->end())
                {
                        throw std::out_of_range("Tried to increment collection end");
                }

                ++listIterator;

                if (listIterator != getListAtIndexReference(index).end())
                {
                        return *this;
                }

                size_type endIndex = map->getEndIndex();

                while (getListAtIndexReference(++index).empty())
                {
                        if (index > endIndex)
                        {
                                *this = map->end();
                                return *this;
                        }
                }

                listIterator = getListAtIndexReference(index).begin();
                return *this;
        }

        ConstIterator operator++(int)
        {
                auto result = *this;
                ++(*this);
                return result;
        }

        ConstIterator &operator--()
        {
                if (*this == map->begin())
                {
                        throw std::out_of_range("Tried to decrement collection begin");
                }

                if (listIterator != getListAtIndexReference(index).begin())
                {

                        --listIterator;
                        return *this;
                }

                size_type beginIndex = map->getBeginIndex();

                while (getListAtIndexReference(--index).empty())
                {
                        if (index < beginIndex)
                        {
                                *this = map->begin();
                                return *this;
                        }
                }

                listIterator = getListAtIndexReference(index).end();
                return *this;
        }

        ConstIterator operator--(int)
        {
                auto result = *this;
                --(*this);
                return result;
        }

        reference operator*() const
        {
                if (*this == map->end())
                {
                        throw std::out_of_range("Dereferencing map end");
                }
                return *listIterator;
        }

        pointer operator->() const
        {
                return &this->operator*();
        }

        size_type getIndex()
        {
                return index;
        }
        list_iterator getListIterator()
        {
                return listIterator;
        }
        bool operator==(const ConstIterator &other) const
        {
                return index == other.index && listIterator == other.listIterator && map == other.map;
        }

        bool operator!=(const ConstIterator &other) const
        {
                return !(*this == other);
        }

      private:
        const list &getListAtIndexReference(size_type index)
        {
                return map->table[index];
        }
};

template <typename KeyType, typename ValueType>
class HashMap<KeyType, ValueType>::Iterator : public HashMap<KeyType, ValueType>::ConstIterator
{
      public:
        using reference = typename HashMap::reference;
        using pointer = typename HashMap::value_type *;

        Iterator() = delete;

        Iterator(const ConstIterator &other)
            : ConstIterator(other) {}

        Iterator &operator++()
        {
                ConstIterator::operator++();
                return *this;
        }

        Iterator operator++(int)
        {
                auto result = *this;
                ConstIterator::operator++();
                return result;
        }

        Iterator &operator--()
        {
                ConstIterator::operator--();
                return *this;
        }

        Iterator operator--(int)
        {
                auto result = *this;
                ConstIterator::operator--();
                return result;
        }

        pointer operator->() const
        {
                return &(this->operator*());
        }

        reference operator*() const
        {
                // ugly cast, yet reduces code duplication.
                return const_cast<reference>(ConstIterator::operator*());
        }
};
} // namespace aisdi

#endif /* AISDI_MAPS_HASHMAP_H */