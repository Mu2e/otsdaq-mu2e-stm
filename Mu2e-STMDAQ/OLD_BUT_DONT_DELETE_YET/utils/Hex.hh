#ifndef Hex_hh
#define Hex_hh

#include <iomanip>

//ostream hex (0x...), n is # bytes to display
#define HEX( x, n ) "0x" << HEXNO0X( x, n )

//ostream hex (no 0x)
#define HEXNO0X( x, n ) std::setw(n) << std::setfill('0') << std::hex << (uint64_t)( x ) << std::dec

#endif
