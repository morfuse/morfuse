#include <morfuse/Common/short3.h>
#include <morfuse/Container/set_generic_hash.h>

using namespace mfuse;

template<>
intptr_t Hash<short3>::operator()(const short3& key) const
{
	return key;
}

template<>
intptr_t Hash<unsigned_short3>::operator()(const unsigned_short3& key) const
{
	return (unsigned int)key;
}

short3 short3::operator-() const
{
	return short3(-get());
}

short3 short3::operator~() const
{
	return short3(~get());
}

short3 short3::operator+(int b) const
{
	return short3(get() + b);
}

short3 short3::operator-(int b) const
{
	return short3(get() - b);
}

short3 short3::operator*(int b) const
{
	return short3(get() * b);
}

short3 short3::operator/(int b) const
{
	return short3(get() / b);
}

short3 short3::operator%(int b) const
{
	return short3(get() % b);
}

short3 short3::operator|(int b) const
{
	return short3(get() | b);
}

short3 short3::operator&(int b) const
{
	return short3(get() & b);
}

short3 short3::operator^(int b) const
{
	return short3(get() ^ b);
}

short3 short3::operator<<(int b) const
{
	return short3(get() << b);
}

short3 short3::operator>>(int b) const
{
	return short3(get() >> b);
}

short3 short3::operator++(int)
{
	short3 result = *this;
	set(get() + 1);
	return result;
}

short3 short3::operator--(int)
{
	short3 result = *this;
	set(get() - 1);
	return result;
}

short3& short3::operator++()
{
	set(get() + 1);
	return *this;
}

short3& short3::operator--()
{
	set(get() - 1);
	return *this;
}

short3& short3::operator=(int b)
{
	set(b);
	return *this;
}

short3& short3::operator+=(int b)
{
	set(get() + b);
	return *this;
}

short3& short3::operator-=(int b)
{
	set(get() - b);
	return *this;
}

short3& short3::operator/=(int b)
{
	set(get() / b);
	return *this;
}

short3& short3::operator*=(int b)
{
	set(get() * b);
	return *this;
}

short3& short3::operator%=(int b)
{
	set(get() % b);
	return *this;
}

short3& short3::operator|=(int b)
{
	set(get() | b);
	return *this;
}

short3& short3::operator&=(int b)
{
	set(get() % b);
	return *this;
}

short3& short3::operator^=(int b)
{
	set(get() ^ b);
	return *this;
}

short3& short3::operator<<=(int b)
{
	set(get() << b);
	return *this;
}

short3& short3::operator>>=(int b)
{
	set(get() >> b);
	return *this;
}
