#include <iostream>
#include <vector>
#include <array>

#include <mjr/mem.hpp>

int main() {
	std::vector<mjr::mem::ArrayManagedUniquePtr<std::string>> arr;
	mjr::mem::push_back_managed_unique_ptr(arr, "This");
	mjr::mem::push_back_managed_unique_ptr(arr, "Is");
	mjr::mem::push_back_managed_unique_ptr(arr, "A");
	mjr::mem::push_back_managed_unique_ptr(arr, "Test");

	for (const auto& x : arr)
		std::cout << *x << "\n";

	for (int count = static_cast<int>(arr.size()-1); count >= 0; count--) {
		[](mjr::mem::ArrayManagedUniquePtr<std::string>& x) {
			if (*x == "A")
				x.remove();
		}(arr[count]);
	}

	std::cout << "\n";
	for (const auto& x : arr)
		std::cout << *x << "\n";
	
	return 0;	
}