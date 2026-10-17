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


#define DISCARD(...)

#define Y_CHOOSE_0_0 ,DISCARD
#define Y_CHOOSE_1_1 ,DISCARD
#define Y_CHOOSE_2_2 ,DISCARD
#define Y_CHOOSE_3_3 ,DISCARD


#define Y_CHOOSE_MATCH_ALL(...) __VA_ARGS__

#define Y_GET_MACRO_HELPER(macro, fallback, ...) fallback



#define Y_EVAL_CHOICE(args) Y_GET_MACRO_HELPER args

#define Y_GET_MACRO(i, j) Y_EVAL_CHOICE((Y_CHOOSE_##i##_##j, Y_CHOOSE_MATCH_ALL))

#define Y_FINAL(i,j) Y_GET_MACRO(i,j)(Y_REAL(i,j))

#define BIQUAD_INNER_LIST(r)\
Y_FINAL(r,0)  Y_FINAL(r,1)  Y_FINAL(r,2)  Y_FINAL(r,3)


#endif // DEFINES_HPP
