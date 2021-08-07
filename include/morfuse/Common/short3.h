#pragma once

#include <cstdint>

namespace mfuse
{
#pragma pack(1)

typedef struct short3_data {
	unsigned short		highmid;
	unsigned char		low;
} short3_data_t;

#pragma pack()

class short3 {
private:
	short3_data_t data;

public :
	operator		int( void ) const { return get(); }

	short3() = default;
	short3(int32_t value) { set(value); }
	explicit short3(uint64_t value) { set((uint32_t)value); }

	short3			operator-( void ) const;
	short3			operator~( void ) const;

	short3			operator+( int b ) const;
	short3			operator-( int b ) const;
	short3			operator*( int b ) const;
	short3			operator/( int b ) const;
	short3			operator%( int b ) const;
	short3			operator|( int b ) const;
	short3			operator&( int b ) const;
	short3			operator^( int b ) const;
	short3			operator<<( int b ) const;
	short3			operator>>( int b ) const;

	short3			operator++( int );
	short3			operator--( int );

	short3&			operator++( );
	short3&			operator--( );

	short3&			operator=( int b );
	short3&			operator+=( int b );
	short3&			operator-=( int b );
	short3&			operator*=( int b );
	short3&			operator/=( int b );
	short3&			operator%=( int b );

	short3&			operator|=( int b );
	short3&			operator&=( int b );
	short3&			operator^=( int b );

	short3&			operator<<=( int b );
	short3&			operator>>=( int b );

	bool			operator!( void ) { return !get(); };

	bool			operator==( int b ) { return get() == b; };
	bool			operator!=( int b ) { return get() != b; };
	bool			operator<=( int b ) { return get() <= b; };
	bool			operator>=( int b ) { return get() >= b; };
	bool			operator<( int b ) { return get() < b; };
	bool			operator>( int b ) { return get() > b; };

	bool			operator==( short3 &b ) { return get() == b.get(); };
	bool			operator!=( short3 &b ) { return get() != b.get(); };
	bool			operator<=( short3 &b ) { return get() <= b.get(); };
	bool			operator>=( short3 &b ) { return get() >= b.get(); };
	bool			operator<( short3 &b ) { return get() < b.get(); };
	bool			operator>( short3 &b ) { return get() > b.get(); };

	
protected:
	int				get() const { return *( int * )this & 0xFFFFFF; }
	void			set(uint32_t value) { data.highmid = value; data.low = *( ( unsigned char * )&value + 2 ); }
};

class unsigned_short3 : public short3 {
public:
	operator unsigned int(void) const { return get(); }
};
}