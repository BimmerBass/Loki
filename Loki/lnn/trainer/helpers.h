#ifndef HELPERS_H
#define HELPERS_H
#include <array>



/*

Take the dot product of two vectors (dot(v1, v2) = sum(v1[i]*v2[i]))

*/
template<typename T, size_t SIZE>
void dot_product(const std::array<T, SIZE>& v1, const std::array<T, SIZE>& v2, T& out) {
	out = T(0);

	for (size_t i = 0; i < SIZE; i++) {
		out += v1[i] * v2[i];
	}
}

/*

Apply a relu activation function on each element in a vector

*/
template<typename T, size_t SIZE>
std::array<T, SIZE> apply_ReLU(const std::array<T, SIZE>& v) {
	std::array<T, SIZE> out;

	for (int i = 0; i < SIZE; i++) {
		out[i] = std::max(T(0), v[i]);
	}

	return out;
}

/*

Divide each element in an array with the same number

*/
template<typename T, size_t SIZE>
void divide_array(std::array<T, SIZE>& _Dst, T x) {
	for (size_t i = 0; i < SIZE; i++) {
		_Dst[i] /= x;
	}
}


/*

Add one array to another

*/
template<typename T, size_t SIZE>
void add_array(std::array<T, SIZE>& _Dst, const std::array<T, SIZE>& v) {
	for (int i = 0; i < SIZE; i++) {
		_Dst[i] += v[i];
	}
}



#endif