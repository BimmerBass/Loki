#ifndef HELPERS_H
#define HELPERS_H
#include <array>
#include <random>
#include <iomanip>
#include <fstream>



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

// Adding this here because it is needed for calculating the loss function
// Two different loss functions can be used:
enum class LOSS_F :int {
	MSE = 0,	/* Mean squared error (1/n * sum((a[i] - y[i])^2))*/
	AAE = 1		/* Average absolute error (1/n * sum(|a[i] - y[i]|))*/
};

/*

Calculate the loss of a set of outputs/expected outputs.

*/
template<LOSS_F F>
double compute_loss(const std::vector<double>& a, const std::vector<double>& y) {
	assert(a.size() == y.size());
	int64_t sum = 0;

	for (size_t i = 0; i < a.size(); i++) {
		
		if constexpr (F == LOSS_F::MSE) { // Mean squared error
			sum += std::pow(a[i] - y[i], 2.0);
		}
		else { // Average absolute
			sum += abs(a[i] - y[i]);
		}
	}

	// Divide by the amount of samples.
	return static_cast<double>(sum) / static_cast<double>(a.size());
}


/*

Generate a random number in the interval [min; max]
NOTE: A random seed should be made before running this method in order to assure "true" randomness

*/
inline double random_num(int min, int max) {
	double f = static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);

	return static_cast<double>(min) + f * (static_cast<double>(max) - static_cast<double>(min));
}


/*

Combine a vector of vectors into one vector.

*/
template<typename T>
std::vector<T> combine_vectors(const std::vector<std::vector<T>>& _Src) {
	std::vector<T> out;

	for (size_t i = 0; i < _Src.size(); i++) {
		for (size_t j = 0; j < _Src[i].size(); j++) {
			out.push_back(_Src[i][j]);
		}
	}
	
	return out;
}


/*

Get the date and time, and return it in std::string format.

*/
inline std::string getDateTime()
{
	auto time = std::time(nullptr);
	std::stringstream ss;

#if (defined(_WIN32) || defined(_WIN64))
	tm ltm;
	localtime_s(&ltm, &time);
	ss << std::put_time(&ltm, "%F_%T"); // ISO 8601 without timezone information.
#else
	ss << std::put_time(std::localtime(&time), "%F_%T");
#endif
	auto s = ss.str();
	std::replace(s.begin(), s.end(), ':', '-');
	return s;
}



// Write an array to a csv file object
template<typename T, size_t SIZE>
void write_array(std::ofstream& file, const std::array<T, SIZE>& v) {

	file << std::to_string(v[0]);

	for (int i = 1; i < SIZE; i++) {
		file << ";" << std::to_string(v[i]);
	}
	file << "\n";
}

// Write multiple arrays to a csv file object
template<typename T, size_t SIZE, size_t NEXT_SIZE>
void write_multiple_arrays(std::ofstream& file, const std::array<std::array<T, SIZE>, NEXT_SIZE>& v) {

	for (size_t i = 0; i < SIZE; i++) {
		
		file << std::to_string(v[0][i]);

		for (size_t j = 1; j < NEXT_SIZE; j++) {
			file << ";" << std::to_string(v[j][i]);
		}

		file << "\n";
	}

}


#endif