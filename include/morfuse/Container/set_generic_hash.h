#pragma once

#include <cstdint>
#include <functional>

namespace mfuse
{
template<typename T>
struct Hash
{
	intptr_t operator()(const T& key) const;
};

//
// include standard hashes
//

template<>
inline intptr_t Hash<void*>::operator()(void* const& key) const
{
	return (intptr_t)key;
}

template<>
inline intptr_t Hash<const void*>::operator()(const void* const& key) const
{
	return (intptr_t)key;
}

template<>
inline intptr_t Hash<std::nullptr_t>::operator()(const std::nullptr_t&) const
{
	return 0;
}

template<>
inline intptr_t Hash<bool>::operator()(const bool& key) const
{
	return key;
}

template<>
inline intptr_t Hash<char>::operator()(const char& key) const
{
	return key;
}

template<>
inline intptr_t Hash<unsigned char>::operator()(const unsigned char& key) const
{
	return key;
}

template<>
inline intptr_t Hash<char16_t>::operator()(const char16_t& key) const
{
	return key;
}

template<>
inline intptr_t Hash<char32_t>::operator()(const char32_t& key) const
{
	return key;
}

template<>
inline intptr_t Hash<wchar_t>::operator()(const wchar_t& key) const
{
	return key;
}

template<typename T>
inline intptr_t HashCharArray(T key)
{
	intptr_t hash = 0;

	for (T p = key; *p; p++)
	{
		hash = hash * 31 + *p;
	}

	return hash;
}

template<>
inline intptr_t Hash<const char*>::operator()(const char* const& key) const
{
	return HashCharArray(key);
}


template<>
inline intptr_t Hash<const unsigned char*>::operator()(const unsigned char* const& key) const
{
	return HashCharArray(key);
}
template<>
inline intptr_t Hash<const char16_t*>::operator()(const char16_t* const& key) const
{
	return HashCharArray(key);
}

template<>
inline intptr_t Hash<const char32_t*>::operator()(const char32_t* const& key) const
{
	return HashCharArray(key);
}

template<>
inline intptr_t Hash<const wchar_t*>::operator()(const wchar_t* const& key) const
{
	return HashCharArray(key);
}

template<>
inline intptr_t Hash<short>::operator()(const short& key) const
{
	return key;
}

template<>
inline intptr_t Hash<unsigned short>::operator()(const unsigned short& key) const
{
	return key;
}

template<>
inline intptr_t Hash<int>::operator()(const int& key) const
{
	return key;
}

template<>
inline intptr_t Hash<unsigned int>::operator()(const unsigned int& key) const
{
	return key;
}

template<>
inline intptr_t Hash<long>::operator()(const long& key) const
{
	return key;
}

template<>
inline intptr_t Hash<unsigned long>::operator()(const unsigned long& key) const
{
	return key;
}

template<>
inline intptr_t Hash<long long>::operator()(const long long& key) const
{
	return key;
}

template<>
inline intptr_t Hash<unsigned long long>::operator()(const unsigned long long& key) const
{
	return key;
}

template<>
inline intptr_t Hash<float>::operator()(const float& key) const
{
	return *(intptr_t*)&key;
}

template<>
inline intptr_t Hash<double>::operator()(const double& key) const
{
	return *(intptr_t*)&key;
}

template<>
inline intptr_t Hash<long double>::operator()(const long double& key) const
{
	return *(intptr_t*)&key;
}
}