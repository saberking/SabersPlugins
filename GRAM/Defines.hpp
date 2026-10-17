#ifndef DEFINES_HPP
#define DEFINES_HPP

#define DEBUG 1
// Define the 25 biquads.
// Format: X(index)
#define BIQUAD_LIST \
X(0)   X(1)  X(2)  X(3)  //X(4)  \
    X(5)  X(6)  X(7)  X(8)  X(9)  \
    X(10) X(11) X(12) X(13) X(14) \
    X(15) X(16)
#define BIQUAD_COUNT 4

#define BIQUAD_INNER_LIST(r)\
Y(r,0)  Y(r,1)  Y(r,2)  Y(r,3)


#undef X
#endif // DEFINES_HPP
