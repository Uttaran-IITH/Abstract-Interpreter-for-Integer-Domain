//#ifndef INTERVAL_UTILS_H
//#define INTERVAL_UTILS_H

#include "interval.h"
#include "/home/uttaran/cbmc/src/util/mp_arith.h"
void add(interval a, interval b, interval *c);
void negate(interval *a);
void sub(interval a, interval b, interval *c);
void multiply(interval a, interval b, interval *c);
void power(interval *a, int p);
bool less_than(interval *a, interval *b,interval*,interval*,int);
bool equals(interval* a, interval* b, interval* &c);
bool meet(interval *a, interval *b, interval* &c);
void join(interval *a, interval *b);
bool greater_than(interval *a, interval *b,interval*,interval*,int);
//#endif
