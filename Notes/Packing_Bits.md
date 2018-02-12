# Packing Bits #

## Bit Fields in C ##

In C, we can specify the size (in bits) of struct and union members.

The idea: use memory more efficiently when we know the value of a group or a group of fields will never exceed a certain value, or is within a small range.

A special unnamed bit field of size 0 is used to force alignment on next boundary.

Cannot have pointers to bit field members as they may not start at a byte boundary.

Assigning an out-of-range value to a bit field -> Implementation dependent.

Bit fields cannot be static.

An array of bit fields is not allowed.
