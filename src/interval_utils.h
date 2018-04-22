#ifndef INTERVAL_UTILS_H
#define INTERVAL_UTILS_H

#include "interval.h"
bool maybe = false;
//enum guard_resultt {ALWAYS_TRUE, NEVER_TRUE, MAYBE};

void add(interval a, interval b, interval *c);
void negate(interval *a);
void sub(interval a, interval b, interval *c);
void multiply(interval a, interval b, interval *c);
bool divide(interval a, interval b, interval *c);
void power(interval *a, int p);
bool less_than(interval *a, interval *b,interval*,interval*,int l = 0 );
bool equals(interval* a, interval* b, interval* &c);
bool meet(interval *a, interval *b, interval* &c);
void join(interval *a, interval *b);
bool widen(interval *a,interval *b,interval *temp);
bool greater_than(interval *a, interval *b,interval*,interval*,int l = 0);
bool not_equals(interval*,interval*);
#endif
