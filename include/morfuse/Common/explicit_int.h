#pragma once

#include <cstdint>

namespace mfuse
{
template<typename Type, typename ImplT>
class explicit_int
{
public:
    constexpr explicit_int() {};
    constexpr explicit_int(explicit_int&& other) { value = other.value; }
    constexpr explicit_int(const explicit_int& other) { value = other.value; }
    constexpr explicit_int& operator=(explicit_int&& other) { value = other.value; return *this; }
    constexpr explicit_int& operator=(const explicit_int& other) { value = other.value; return *this; }
    constexpr explicit_int(char valueValue) : value((Type)valueValue) {}
    constexpr explicit_int(unsigned char valueValue) : value((Type)valueValue) {}
    constexpr explicit_int(short valueValue) : value((Type)valueValue) {}
    constexpr explicit_int(unsigned short valueValue) : value((Type)valueValue) {}
    constexpr explicit_int(int valueValue) : value((Type)valueValue) {}
    constexpr explicit_int(unsigned int valueValue) : value((Type)valueValue) {}
    constexpr explicit_int(long valueValue) : value((Type)valueValue) {}
    constexpr explicit_int(unsigned long valueValue) : value((Type)valueValue) {}
    constexpr explicit_int(long long valueValue) : value((Type)valueValue) {}
    constexpr explicit_int(unsigned long long valueValue) : value((Type)valueValue) {}

    constexpr operator bool() const { return value != 0; }
    constexpr operator signed char() const { return (signed char)value; }
    constexpr operator unsigned char() const { return (unsigned char)value; }
    constexpr operator signed short() const { return (signed short)value; }
    constexpr operator unsigned short() const { return (unsigned short)value; }
    constexpr operator signed int() const { return (signed int)value; }
    constexpr operator unsigned int() const { return (unsigned int)value; }
    constexpr operator signed long() const { return (signed long)value; }
    constexpr operator unsigned long() const { return (unsigned long)value; }
    constexpr operator signed long long() const { return (long)value; }
    constexpr operator unsigned long long() const { return (unsigned long long)value; }

    friend constexpr bool operator==(ImplT lhs, ImplT rhs) { return lhs.value == rhs.value; };
    friend constexpr bool operator!=(ImplT lhs, ImplT rhs) { return lhs.value != rhs.value; };
    template<typename U> friend constexpr bool operator==(ImplT lhs, const U& rhs) { return lhs.value == rhs; };
    template<typename U> friend constexpr bool operator!=(ImplT lhs, const U& rhs) { return lhs.value != rhs; };
    template<typename U> friend constexpr bool operator==(const U& lhs, ImplT rhs) { return lhs == rhs.value; };
    template<typename U> friend constexpr bool operator!=(const U& lhs, ImplT rhs) { return lhs != rhs.value; };

protected:
    Type value;
};
}
