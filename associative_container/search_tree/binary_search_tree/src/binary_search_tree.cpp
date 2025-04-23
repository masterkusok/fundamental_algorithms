#include "../include/binary_search_tree.h"

namespace __detail
{
    template<typename tkey, typename tvalue, typename compare, typename tag>
    binary_search_tree<tkey, tvalue, compare, tag>::node* bst_impl<tkey, tvalue, compare, tag>::create_node(
        binary_search_tree<tkey, tvalue, compare, tag>& cont,
        Args&& ...args)
    {
        auto* new_node = cont._allocator.allocate(1);
        new (new_node) typename binary_search_tree<tkey, tvalue, compare, tag>::node(nullptr, std::forward<Args>(args)...);
        return new_node;
    }

    template<typename tkey, typename tvalue, typename compare, typename tag>
    void bst_impl<tkey, tvalue, compare, tag>::delete_node(
        binary_search_tree<tkey, tvalue, compare, tag>& cont,
        typename binary_search_tree<tkey, tvalue, compare, tag>::node* node)
    {
        node->~node();
        cont._allocator.deallocate(node, 1);
    }

    template<typename tkey, typename tvalue, typename compare, typename tag>
    void bst_impl<tkey, tvalue, compare, tag>::erase(
        binary_search_tree<tkey, tvalue, compare, tag>& cont,
        typename binary_search_tree<tkey, tvalue, compare, tag>::node** node)
    {
        if (*node == nullptr)
        {
            return;
        }

        if ((*node)->left_subtree == nullptr && (*node)->right_subtree == nullptr)
        {
            // Leaf node
            if ((*node)->parent != nullptr)
            {
                if ((*node)->parent->left_subtree == *node)
                {
                    (*node)->parent->left_subtree = nullptr;
                }
                else
                {
                    (*node)->parent->right_subtree = nullptr;
                }
            }
            else
            {
                cont._root = nullptr;
            }
            delete_node(cont, *node);
            *node = nullptr;
        }
        else if ((*node)->left_subtree == nullptr)
        {
            // Only right child
            auto* temp = *node;
            *node = (*node)->right_subtree;
            (*node)->parent = temp->parent;
            if (temp->parent != nullptr)
            {
                if (temp->parent->left_subtree == temp)
                {
                    temp->parent->left_subtree = *node;
                }
                else
                {
                    temp->parent->right_subtree = *node;
                }
            }
            else
            {
                cont._root = *node;
            }
            delete_node(cont, temp);
        }
        else if ((*node)->right_subtree == nullptr)
        {
            // Only left child
            auto* temp = *node;
            *node = (*node)->left_subtree;
            (*node)->parent = temp->parent;
            if (temp->parent != nullptr)
            {
                if (temp->parent->left_subtree == temp)
                {
                    temp->parent->left_subtree = *node;
                }
                else
                {
                    temp->parent->right_subtree = *node;
                }
            }
            else
            {
                cont._root = *node;
            }
            delete_node(cont, temp);
        }
        else
        {
            // Two children
            auto* successor = (*node)->right_subtree;
            while (successor->left_subtree != nullptr)
            {
                successor = successor->left_subtree;
            }

            // Swap data
            std::swap((*node)->data, successor->data);

            // Recursively delete the successor
            erase(cont, &successor);
        }
    }

    template<typename tkey, typename tvalue, typename compare, typename tag>
    void bst_impl<tkey, tvalue, compare, tag>::swap(
        binary_search_tree<tkey, tvalue, compare, tag>& lhs,
        binary_search_tree<tkey, tvalue, compare, tag>& rhs) noexcept
    {
        std::swap(lhs._root, rhs._root);
        std::swap(lhs._size, rhs._size);
        std::swap(lhs._logger, rhs._logger);
        std::swap(lhs._allocator, rhs._allocator);
    }
}

template<typename tkey, typename tvalue, typename compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::node::node(
    node* parent,
    Args&& ...args) :
    parent(parent),
    left_subtree(nullptr),
    right_subtree(nullptr),
    data(std::forward<Args>(args)...)
{}

template<typename tkey, typename tvalue, typename compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::compare_keys(
    const tkey& lhs,
    const tkey& rhs) const
{
    return _compare(lhs, rhs);
}

template<typename tkey, typename tvalue, typename compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::compare_pairs(
    const value_type& lhs,
    const value_type& rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, typename compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(
    const compare& comp,
    pp_allocator<value_type> alloc,
    logger* logger) :
    _root(nullptr),
    _logger(logger),
    _size(0),
    _allocator(alloc),
    _compare(comp)
{}

template<typename tkey, typename tvalue, typename compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(
    pp_allocator<value_type> alloc,
    const compare& comp,
    logger* logger) :
    _root(nullptr),
    _logger(logger),
    _size(0),
    _allocator(alloc),
    _compare(comp)
{}

template<typename tkey, typename tvalue, typename compare, typename tag>
template<input_iterator_for_pair<tkey, tvalue> iterator>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(
    iterator begin,
    iterator end,
    const compare& cmp,
    pp_allocator<value_type> alloc,
    logger* logger) :
    _root(nullptr),
    _logger(logger),
    _size(0),
    _allocator(alloc),
    _compare(cmp)
{
    insert(begin, end);
}

template<typename tkey, typename tvalue, typename compare, typename tag>
template<std::ranges::input_range Range>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(
    Range&& range,
    const compare& cmp,
    pp_allocator<value_type> alloc,
    logger* logger) :
    _root(nullptr),
    _logger(logger),
    _size(0),
    _allocator(alloc),
    _compare(cmp)
{
    insert_range(std::forward<Range>(range));
}

template<typename tkey, typename tvalue, typename compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(
    std::initializer_list<std::pair<tkey, tvalue>> data,
    const compare& cmp,
    pp_allocator<value_type> alloc,
    logger* logger) :
    _root(nullptr),
    _logger(logger),
    _size(0),
    _allocator(alloc),
    _compare(cmp)
{
    insert(data.begin(), data.end());
}

template<typename tkey, typename tvalue, typename compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(
    const binary_search_tree& other) :
    _root(nullptr),
    _logger(other._logger),
    _size(0),
    _allocator(other._allocator),
    _compare(other._compare)
{
    if (other._root != nullptr)
    {
        _root = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, other._root->data);
        copy_subtree(_root, other._root);
    }
}

template<typename tkey, typename tvalue, typename compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(
    binary_search_tree&& other) noexcept :
    _root(other._root),
    _logger(other._logger),
    _size(other._size),
    _allocator(std::move(other._allocator)),
    _compare(std::move(other._compare))
{
    other._root = nullptr;
    other._size = 0;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>& binary_search_tree<tkey, tvalue, compare, tag>::operator=(
    const binary_search_tree& other)
{
    if (this != &other)
    {
        clear();
        _logger = other._logger;
        _allocator = other._allocator;
        _compare = other._compare;
        if (other._root != nullptr)
        {
            _root = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, other._root->data);
            copy_subtree(_root, other._root);
        }
    }
    return *this;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>& binary_search_tree<tkey, tvalue, compare, tag>::operator=(
    binary_search_tree&& other) noexcept
{
    if (this != &other)
    {
        clear();
        _root = other._root;
        _logger = other._logger;
        _size = other._size;
        _allocator = std::move(other._allocator);
        _compare = std::move(other._compare);
        other._root = nullptr;
        other._size = 0;
    }
    return *this;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::~binary_search_tree()
{
    clear();
}

template<typename tkey, typename tvalue, typename compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::copy_subtree(
    node* dest,
    const node* src)
{
    if (src->left_subtree != nullptr)
    {
        dest->left_subtree = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, src->left_subtree->data);
        dest->left_subtree->parent = dest;
        copy_subtree(dest->left_subtree, src->left_subtree);
    }
    if (src->right_subtree != nullptr)
    {
        dest->right_subtree = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, src->right_subtree->data);
        dest->right_subtree->parent = dest;
        copy_subtree(dest->right_subtree, src->right_subtree);
    }
}

template<typename tkey, typename tvalue, typename compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::clear() noexcept
{
    while (_root != nullptr)
    {
        __detail::bst_impl<tkey, tvalue, compare, tag>::erase(*this, &_root);
    }
    _size = 0;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::empty() const noexcept
{
    return _size == 0;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
tvalue& binary_search_tree<tkey, tvalue, compare, tag>::at(const tkey& key)
{
    auto* node = find_node(key);
    if (node == nullptr)
    {
        throw std::out_of_range("Key not found");
    }
    return node->data.second;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
const tvalue& binary_search_tree<tkey, tvalue, compare, tag>::at(const tkey& key) const
{
    auto* node = find_node(key);
    if (node == nullptr)
    {
        throw std::out_of_range("Key not found");
    }
    return node->data.second;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
tvalue& binary_search_tree<tkey, tvalue, compare, tag>::operator[](const tkey& key)
{
    auto* node = find_node(key);
    if (node == nullptr)
    {
        node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, key, tvalue());
        insert_node(node);
    }
    return node->data.second;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
tvalue& binary_search_tree<tkey, tvalue, compare, tag>::operator[](tkey&& key)
{
    auto* node = find_node(key);
    if (node == nullptr)
    {
        node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, std::move(key), tvalue());
        insert_node(node);
    }
    return node->data.second;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::node* binary_search_tree<tkey, tvalue, compare, tag>::find_node(
    const tkey& key) const
{
    auto* current = _root;
    while (current != nullptr)
    {
        if (compare_keys(key, current->data.first))
        {
            current = current->left_subtree;
        }
        else if (compare_keys(current->data.first, key))
        {
            current = current->right_subtree;
        }
        else
        {
            return current;
        }
    }
    return nullptr;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::insert_node(node* new_node)
{
    if (_root == nullptr)
    {
        _root = new_node;
        ++_size;
        return;
    }

    auto* current = _root;
    while (true)
    {
        if (compare_keys(new_node->data.first, current->data.first))
        {
            if (current->left_subtree == nullptr)
            {
                current->left_subtree = new_node;
                new_node->parent = current;
                ++_size;
                return;
            }
            current = current->left_subtree;
        }
        else
        {
            if (current->right_subtree == nullptr)
            {
                current->right_subtree = new_node;
                new_node->parent = current;
                ++_size;
                return;
            }
            current = current->right_subtree;
        }
    }
}

template<typename tkey, typename tvalue, typename compare, typename tag>
std::pair<typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator, bool> binary_search_tree<tkey, tvalue, compare, tag>::insert(
    const value_type& value)
{
    auto* node = find_node(value.first);
    if (node != nullptr)
    {
        return {infix_iterator(node), false};
    }

    node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, value);
    insert_node(node);
    return {infix_iterator(node), true};
}

template<typename tkey, typename tvalue, typename compare, typename tag>
std::pair<typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator, bool> binary_search_tree<tkey, tvalue, compare, tag>::insert(
    value_type&& value)
{
    auto* node = find_node(value.first);
    if (node != nullptr)
    {
        return {infix_iterator(node), false};
    }

    node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, std::move(value));
    insert_node(node);
    return {infix_iterator(node), true};
}

template<typename tkey, typename tvalue, typename compare, typename tag>
template<std::input_iterator InputIt>
void binary_search_tree<tkey, tvalue, compare, tag>::insert(InputIt first, InputIt last)
{
    for (; first != last; ++first)
    {
        insert(*first);
    }
}

template<typename tkey, typename tvalue, typename compare, typename tag>
template<std::ranges::input_range R>
void binary_search_tree<tkey, tvalue, compare, tag>::insert_range(R&& rg)
{
    for (const auto& value : rg)
    {
        insert(value);
    }
}

template<typename tkey, typename tvalue, typename compare, typename tag>
template<class ...Args>
std::pair<typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator, bool> binary_search_tree<tkey, tvalue, compare, tag>::emplace(
    Args&&... args)
{
    value_type value(std::forward<Args>(args)...);
    return insert(std::move(value));
}

template<typename tkey, typename tvalue, typename compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator binary_search_tree<tkey, tvalue, compare, tag>::insert_or_assign(
    const value_type& value)
{
    auto* node = find_node(value.first);
    if (node != nullptr)
    {
        node->data.second = value.second;
        return infix_iterator(node);
    }

    node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, value);
    insert_node(node);
    return infix_iterator(node);
}

template<typename tkey, typename tvalue, typename compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator binary_search_tree<tkey, tvalue, compare, tag>::insert_or_assign(
    value_type&& value)
{
    auto* node = find_node(value.first);
    if (node != nullptr)
    {
        node->data.second = std::move(value.second);
        return infix_iterator(node);
    }

    node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, std::move(value));
    insert_node(node);
    return infix_iterator(node);
}

template<typename tkey, typename tvalue, typename compare, typename tag>
template<std::input_iterator InputIt>
void binary_search_tree<tkey, tvalue, compare, tag>::insert_or_assign(InputIt first, InputIt last)
{
    for (; first != last; ++first)
    {
        insert_or_assign(*first);
    }
}

template<typename tkey, typename tvalue, typename compare, typename tag>
template<class ...Args>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator binary_search_tree<tkey, tvalue, compare, tag>::emplace_or_assign(
    Args&&... args)
{
    value_type value(std::forward<Args>(args)...);
    return insert_or_assign(std::move(value));
}

template<typename tkey, typename tvalue, typename compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::swap(binary_search_tree& other) noexcept
{
    __detail::bst_impl<tkey, tvalue, compare, tag>::swap(*this, other);
}

template<typename tkey, typename tvalue, typename compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::contains(const tkey& key) const
{
    return find_node(key) != nullptr;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator binary_search_tree<tkey, tvalue, compare, tag>::find(const tkey& key)
{
    return infix_iterator(find_node(key));
}

template<typename tkey, typename tvalue, typename compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator binary_search_tree<tkey, tvalue, compare, tag>::find(
    const tkey& key) const
{
    return infix_const_iterator(find_node(key));
}

template<typename tkey, typename tvalue, typename compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator binary_search_tree<tkey, tvalue, compare, tag>::lower_bound(const tkey& key)
{
    auto* current = _root;
    node* result = nullptr;
    while (current != nullptr)
    {
        if (!compare_keys(current->data.first, key))
        {
            result = current;
            current = current->left_subtree;
        }
        else
        {
            current = current->right_subtree;
        }
    }
    return infix_iterator(result);
}

template<typename tkey, typename tvalue, typename compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator binary_search_tree<tkey, tvalue, compare, tag>::lower_bound(
    const tkey& key) const
{
    auto* current = _root;
    const node* result = nullptr;
    while (current != nullptr)
    {
        if (!compare_keys(current->data.first, key))
        {
            result = current;
            current = current->left_subtree;
        }
        else
        {
            current = current->right_subtree;
        }
    }
    return infix_const_iterator(result);
}

template<typename tkey, typename tvalue, typename compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator binary_search_tree<tkey, tvalue, compare, tag>::upper_bound(const tkey& key)
{
    auto* current = _root;
    node* result = nullptr;
    while (current != nullptr)
    {
        if (compare_keys(key, current->data.first))
        {
            result = current;
            current = current->left_subtree;
        }
        else
        {
            current = current->right_subtree;
        }
    }
    return infix_iterator(result);
}

template<typename tkey, typename tvalue, typename compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator binary_search_tree<tkey, tvalue, compare, tag>::upper_bound(
    const tkey& key) const
{
    auto* current = _root;
    const node* result = nullptr;
    while (current != nullptr)
    {
        if (compare_keys(key, current->data.first))
        {
            result = current;
            current = current->left_subtree;
        }
        else
        {
            current = current->right_subtree;
        }
    }
    return infix_const_iterator(result);
}

template<typename tkey, typename tvalue, typename compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator binary_search_tree<tkey, tvalue, compare, tag>::erase(infix_iterator pos)
{
    if (pos == end())
    {
        return pos;
    }

    auto* node = pos._data;
    auto next = pos;
    ++next;

    __detail::bst_impl<tkey, tvalue, compare, tag>::erase(*this, &node);
    --_size;

    return next;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator binary_search_tree<tkey, tvalue, compare, tag>::erase(
    infix_const_iterator pos)
{
    return erase(infix_iterator(const_cast<node*>(pos._data)));
}

template<typename tkey, typename tvalue, typename compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator binary_search_tree<tkey, tvalue, compare, tag>::erase(
    infix_iterator first,
    infix_iterator last)
{
    while (first != last)
    {
        first = erase(first);
    }
    return first;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator binary_search_tree<tkey, tvalue, compare, tag>::erase(
    infix_const_iterator first,
    infix_const_iterator last)
{
    return erase(infix_iterator(const_cast<node*>(first._data)), infix_iterator(const_cast<node*>(last._data)));
}

template<typename tkey, typename tvalue, typename compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::erase(const tkey& key)
{
    auto* node = find_node(key);
    if (node == nullptr)
    {
        return 0;
    }

    __detail::bst_impl<tkey, tvalue, compare, tag>::erase(*this, &node);
    --_size;
    return 1;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::small_left_rotation(node*& subtree_root) noexcept
{
    auto* right = subtree_root->right_subtree;
    subtree_root->right_subtree = right->left_subtree;
    if (right->left_subtree != nullptr)
    {
        right->left_subtree->parent = subtree_root;
    }
    right->parent = subtree_root->parent;
    if (subtree_root->parent == nullptr)
    {
        _root = right;
    }
    else if (subtree_root == subtree_root->parent->left_subtree)
    {
        subtree_root->parent->left_subtree = right;
    }
    else
    {
        subtree_root->parent->right_subtree = right;
    }
    right->left_subtree = subtree_root;
    subtree_root->parent = right;
    subtree_root = right;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::small_right_rotation(node*& subtree_root) noexcept
{
    auto* left = subtree_root->left_subtree;
    subtree_root->left_subtree = left->right_subtree;
    if (left->right_subtree != nullptr)
    {
        left->right_subtree->parent = subtree_root;
    }
    left->parent = subtree_root->parent;
    if (subtree_root->parent == nullptr)
    {
        _root = left;
    }
    else if (subtree_root == subtree_root->parent->right_subtree)
    {
        subtree_root->parent->right_subtree = left;
    }
    else
    {
        subtree_root->parent->left_subtree = left;
    }
    left->right_subtree = subtree_root;
    subtree_root->parent = left;
    subtree_root = left;
}

template<typename tkey, typename tvalue, typename compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::big_left_rotation(node*& subtree_root) noexcept
{
    small_right_rotation(subtree_root->right_subtree);
    small_left_rotation(subtree_root);
}

template<typename tkey, typename tvalue, typename compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::big_right_rotation(node*& subtree_root) noexcept
{
    small_left_rotation(subtree_root->left_subtree);
    small_right_rotation(subtree_root);
}

template<typename tkey, typename tvalue, typename compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::double_left_rotation(node*& subtree_root) noexcept
{
    small_left_rotation(subtree_root);
    small_left_rotation(subtree_root);
}

template<typename tkey, typename tvalue, typename compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::double_right_rotation(node*& subtree_root) noexcept
{
    small_right_rotation(subtree_root);
    small_right_rotation(subtree_root);
}
``` 