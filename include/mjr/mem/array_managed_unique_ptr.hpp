#ifndef MJR_MEM_ARRAY_MANAGED_UNIQUE_PTR_HPP
#define MJR_MEM_ARRAY_MANAGED_UNIQUE_PTR_HPP

#include <utility>
#include <memory>
#include <type_traits>

namespace mjr::mem {

namespace del { // namespace mjr::mem::del

template <typename T>
concept erasable_container = requires(T container) {
	container.erase(container.begin());
};

class ArrayManagedDeleterBase {
protected:
	constexpr ArrayManagedDeleterBase() {}

public:
	using size_type = size_t;

	virtual ~ArrayManagedDeleterBase() noexcept = default;
	constexpr virtual void remove(size_type index) = 0;
};

template <erasable_container Container>
class ArrayManagedDeleter : public ArrayManagedDeleterBase {
private:
	Container& _container;

public:
	ArrayManagedDeleter(Container& container)
		: _container(container)
		{}
	~ArrayManagedDeleter() noexcept override {}

	constexpr void remove(size_type index) override {
		_container.erase(_container.begin() + index);
	}
};

} // namespace mjr::mem::del

template <typename Obj>
class ArrayManagedUniquePtr {
public:
	using size_type = size_t;
	using value_type = Obj;
	Obj* data;
	
protected:
	size_type _index;
	del::ArrayManagedDeleterBase* _deleter;

public:
	constexpr ArrayManagedUniquePtr(del::ArrayManagedDeleterBase* deleter, size_t index, Obj* obj) noexcept
		: _deleter(deleter), _index(index), data(obj)
		{}
	
	ArrayManagedUniquePtr(const ArrayManagedUniquePtr&) = delete;
	constexpr ArrayManagedUniquePtr(ArrayManagedUniquePtr&& mov) noexcept
		: _deleter(mov._deleter), _index(mov._index), data(mov.data)
	{
		mov._deleter = nullptr;
		mov.data = nullptr;
	}
	
	ArrayManagedUniquePtr& operator=(const ArrayManagedUniquePtr&) = delete;
	constexpr ArrayManagedUniquePtr& operator=(ArrayManagedUniquePtr&& mov) noexcept {
		// Index remains the same when the pointer is moved
		// this means that indices remain correct when a pointer is removed from an array
		_index = mov._index;

		delete _deleter;
		delete data;

		_deleter = mov._deleter;
		data = mov.data;
		mov._deleter = nullptr;
		mov.data = nullptr;
		
		return *this;
	}

	constexpr void remove() {
		_deleter->remove(_index);
	}

	~ArrayManagedUniquePtr() noexcept {
		delete _deleter;
		delete data;
	}

	[[nodiscard]]
	constexpr const Obj& operator*() const {
		return *data;
	}
	[[nodiscard]]
	constexpr Obj& operator*() {
		return *data;
	}
};

template <typename T>
concept managing_container = std::is_same_v<typename T::value_type, ArrayManagedUniquePtr<typename T::value_type::value_type>>;

template <managing_container Container, typename Obj = Container::value_type::value_type, typename... Args>
[[nodiscard]]
inline auto make_array_managed_unique_ptr(Container& container, typename ArrayManagedUniquePtr<Obj>::size_type index, Args&&... args)
	noexcept(	std::is_nothrow_constructible_v<del::ArrayManagedDeleter<Container>> &&
				std::is_nothrow_constructible_v<Obj, Args...>)
{
	return ArrayManagedUniquePtr<Obj>(
		static_cast<del::ArrayManagedDeleterBase*>(new del::ArrayManagedDeleter<Container>{container}),
		index,
		new Obj(std::forward<Args>(args)...)
	);
}

template <managing_container Container, typename Obj = Container::value_type::value_type, typename... Args>
inline constexpr void push_back_managed_unique_ptr(Container& container, Args&&... args) {
	container.push_back(make_array_managed_unique_ptr(container, container.size(), std::forward<Args>(args)...));
}

} // namespace mjr::mem

#endif