#include <initializer_list>
#include <stdexcept>
#include <array>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <ranges>

template< class R, class T >
concept container_compatible_range =
    std::ranges::input_range<R> &&
    std::convertible_to<std::ranges::range_reference_t<R>, T>;

template<typename T, std::size_t _max_size>
class inplace_vector{
public:
//Member types
  typedef T value_type;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef T* iterator;
  typedef const T* const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

//Member Functions
  constexpr inplace_vector() noexcept : _size{} {}
  template<typename... Args>
  constexpr inplace_vector(Args&&... args) : _data{std::forward<Args>(args)...}, _size{sizeof...(Args)} {};
  
  //Element Access
  constexpr reference at(size_type index){
    if(index < _size)[[likely]]{
      return _data[index];
    }
    throw std::out_of_range{};
  }
  constexpr const_reference at(size_type index) const {
    if(index < _size)[[likely]]{
      return _data[index];
    }
    throw std::out_of_range{};
  }
  constexpr reference operator[](size_type index){ return _data[index]; }
  constexpr const_reference operator[](size_type index) const { return _data[index]; }
  constexpr reference front(){ return _data.front(); }
  constexpr const_reference front() const { return _data.front(); }
  constexpr reference back(){ return _data.back(); }
  constexpr const_reference back() const { return _data.back(); }
  constexpr pointer data() noexcept { return _data.data(); }
  constexpr const_pointer data() const noexcept { return _data.data(); }

  //Iterators
  constexpr iterator begin() noexcept { return _data.data(); }
  constexpr const_iterator begin() const noexcept { return _data.data(); }
  constexpr const_iterator cbegin() const noexcept { return _data.data(); }
  constexpr iterator end() noexcept { return _data.data() + _size; }
  constexpr const_iterator end() const noexcept{ return _data.data() + _size; }
  constexpr iterator cend() const noexcept { return _data.data() + _size; }
  constexpr reverse_iterator rbegin() noexcept { return _data.rbegin() + (_max_size - _size); } 
  constexpr const_reverse_iterator rbegin() const noexcept { return _data.crbegin() + (_max_size - _size); } 
  constexpr const_reverse_iterator crbegin() const noexcept { return _data.crbegin() + (_max_size - _size); }
  constexpr reverse_iterator rend() noexcept { return _data.rend(); }
  constexpr const_reverse_iterator rend() const noexcept { return _data.crend(); }
  constexpr const_reverse_iterator crend() const noexcept { return _data.crend(); }

  //Size and Capacity
  constexpr bool empty() const noexcept { return _size == 0; }
  constexpr auto size() const { return _size; } //could be noexcept?
  constexpr auto static max_size() noexcept { return _max_size; }
  constexpr auto static capacity() noexcept { return _max_size; }
  constexpr void resize(std::size_t count, const T& val = {}){
    if(count > _max_size) [[unlikely]] throw std::bad_alloc{};
    if(count > _size){
      while(_size < count){
        _data[_size] = val;
        _size++;
      }
    }
    else{
      while(_size != count){
        _data[_size].~T();
        _size--;
      }
    }
  }
  constexpr void static reserve(size_type count){ if(count >= _max_size) [[unlikely]] throw std::bad_alloc{}; }
  constexpr void static shrink_to_fit() noexcept {}

  //Modifiers
  constexpr iterator insert(iterator pos, const T& val){
    if(_size == _max_size) [[unlikely]] throw std::bad_alloc{};
    _size++;
    std::move_backward(pos, end(), end() + 1);
    *pos = val;
    return pos;
  }
  constexpr iterator insert(iterator pos, T&& val){
    if(_size == _max_size) [[unlikely]] throw std::bad_alloc{};
    _size++;
    std::move_backward(pos, end(), end() + 1);
    *pos = std::move(val);
    return pos;
  }
  constexpr iterator insert(iterator pos, size_type count, const T& val){
    if(_size + count >= _max_size) [[unlikely]] throw std::bad_alloc{};
    _size += count;
    std::move_backward(pos, end(), end() + count);
    while(count != 0){
      *(pos + count - 1) = val;
      count--;
    }
    return pos;
  }
  template<std::input_iterator InputIt>
  constexpr iterator insert(iterator pos, InputIt first, InputIt last){
    auto count = std::distance(first, last);
    if(_size + count >= _max_size) [[unlikely]] throw std::bad_alloc{};
    _size += count;
    std::move_backward(pos, end(), end() + count);
    count = 0;
    while(first != last){
      *(pos + count) = *first;
      count++;
      first++;
    }
    return pos;
  }
  constexpr iterator insert(iterator pos, std::initializer_list<T> ilist){ return insert(pos, ilist.begin(), ilist.end()); }
  template<container_compatible_range<T> R>
  constexpr iterator insert_range(iterator pos, R&& rg) { return insert(pos, std::ranges::begin(rg), std::ranges::end(rg)); }
  template<class... Args>
  constexpr iterator emplace(iterator pos, Args&&... args){
    if(_size == _max_size) [[unlikely]] throw std::bad_alloc{};
    _size++;
    std::move_backward(pos, end(), end() + 1);
    *pos = T{std::forward<Args>(args)...};
    return pos;
  }
  template<class... Args>
  constexpr reference emplace_back(Args&&... args){
    if(_size == _max_size) [[unlikely]] throw std::bad_alloc{};
    *end() = T{std::forward<Args>(args)...};
    _size++;
    return back();
  }
  template<class... Args>
  constexpr pointer try_emplace_back(Args&&... args){
    if(_size != _max_size) [[likely]]{
      *end() = T{std::forward<Args>(args)...};
      _size++;
      return &back();
    }
    return nullptr;
  }
  template<class... Args>
  constexpr reference unchecked_emplace_back(Args&&... args){
    *end() = T{std::forward<Args>(args)...};
    _size++;
    return back();
  }
  constexpr reference push_back(const T& val){
    if(_size == _max_size) [[unlikely]] throw std::bad_alloc{};
    *end() = val;
    _size++;
    return back();
  }
  constexpr reference push_back(T&& val){
    if(_size == _max_size) [[unlikely]] throw std::bad_alloc{};
    *end() = std::move(val);
    _size++;
    return back();
  }
  constexpr pointer try_push_back(const T& val){
    if(_size != _max_size) [[likely]]{
      *end() = val;
      _size++;
      return &back();
    }
    return nullptr;
  }
  constexpr pointer try_push_back(T&& val){
    if(_size != _max_size) [[likely]]{
      *end() = std::move(val);
      _size++;
      return &back();
    }
    return nullptr;
  }
  constexpr reference unchecked_push_back(const T& val){
    *end() = val;
    _size++;
    return back();
  }
  constexpr reference unchecked_push_back(T&& val){
    *end() = std::move(val);
    _size++;
    return back();
  }
  constexpr void pop_back() noexcept(std::is_nothrow_destructible_v<T>) { 
    _size--; 
    _data[_size].~T();
  }
  template<container_compatible_range<T> R>
  constexpr void append_range(R&& rg){
    if(std::ranges::size(rg) + _size >= _max_size) [[unlikely]] throw std::bad_alloc{};
    for(auto& i: rg){
      _data[_size] = i;
      _size++;
    }
  }
  template<container_compatible_range<T> R>
  constexpr std::ranges::borrowed_iterator_t<R> try_append_range(R&& rg){
    auto it = std::ranges::begin(rg);
    while(_size < _max_size && it != std::ranges::end(rg)){
      _data[_size] = *it;
      _size++;
      it++;
    }
    return it;
  }
  constexpr void clear() noexcept{
    while(_size != 0){
      _data[_size - 1].~T();
      _size--;
    }
  }
  constexpr iterator erase(iterator pos){
    pos->~T();
    std::move(pos + 1, end(), pos);
    _size--;
    if(pos == end())[[unlikely]]{
      return end();
    }
    return pos;
  }
  constexpr iterator erase(iterator first, iterator last){
    if(first == last) [[unlikely]] return last;
    if(last == end()){
      std::for_each(first, last, [](T& i){ i.~T(); });
      _size -= last - first;
      return end();
    }
    else{
      std::for_each(first, last, [](T& i){ i.~T(); });
      std::move(last, end(), first);
      _size -= last - first;
      return last;
    }
  }
  constexpr void swap(inplace_vector& other) noexcept(_max_size == 0 || (std::is_nothrow_swappable_v<T> && std::is_nothrow_move_constructible_v<T>)){
    for(std::size_t i = 0; i < _size; i++){
      std::swap(_data[i], other[i]);
    }
  }
private:
  std::array<T, _max_size> _data;
  std::size_t _size;
};
