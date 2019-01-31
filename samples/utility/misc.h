#pragma once

template <typename T, size_t N>
size_t array_length(const T (&)[N]){
	return N;
}
