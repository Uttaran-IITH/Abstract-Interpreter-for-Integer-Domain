#include "interval_utils.h"
#include <math.h>
#include <stdlib.h>
#include <iostream>
int min(int a, int b) {
	if (a<b)
		return a;
	else
		return b;
}
int max(int a, int b) {
	if (a>b)
		return a;
	else
		return b;
}
void add(interval a, interval b, interval *c) {
	c->set_lower_bound(a.get_lower_bound() + b.get_lower_bound());
	c->set_upper_bound(a.get_upper_bound() + b.get_upper_bound());
}
void negate(interval *a) {
	int temp = a->get_upper_bound();
	a->set_upper_bound(-(a->get_lower_bound()));
	a->set_lower_bound(-temp);
}
void sub(interval a, interval b, interval *c) {
	negate(&b);
	add(a, b, c);
}
void multiply(interval a, interval b, interval *c) {
	int temp[] = {
		a.get_lower_bound() * b.get_lower_bound(),
		a.get_lower_bound() * b.get_upper_bound(),
		a.get_upper_bound() * b.get_lower_bound(),
		a.get_upper_bound() * b.get_upper_bound()
	};
	int min = temp[0];
	int max = temp[0];
	for (int i = 1; i < 4; i++) {
		if (min > temp[i])
			min = temp[i];
		if (max < temp[i])
			max = temp[i];
	}
	c->set_lower_bound(min);
	c->set_upper_bound(max);
}
void power(interval *a, int p) {
	int sign[] = { 1,1 };
	if (a->get_lower_bound() < 0)
		sign[0] = -1;
	if (a->get_upper_bound() < 0)
		sign[1] = -1;
	a->set_lower_bound(sign[0] * abs(pow(a->get_lower_bound(), p)));
	a->set_upper_bound(sign[1] * abs(pow(a->get_upper_bound(), p)));
}
void less_than(interval *a, interval *b) {
	int l1 = a->get_lower_bound();
	int u1 = a->get_upper_bound();
	int l2 = b->get_lower_bound();
	int u2 = b->get_upper_bound();
	if (u1 < l2) {
		return;
	}
	else if (u2 < l1) {
		std::cout << "Not valid";
	}
	else {
		a->set_lower_bound(l1);
		a->set_upper_bound(min(u1, u2));
	}
}
void greater_than(interval *a, interval *b) {
	int l1 = a->get_lower_bound();
	int u1 = a->get_upper_bound();
	int l2 = b->get_lower_bound();
	int u2 = b->get_upper_bound();
	if (u1 < l2) {
		std::cout << "Not valid";
	}
	else if (u2 < l1) {
		return;
	}
	else {
		a->set_lower_bound(max(l1,l2));
		a->set_upper_bound(u1);
	}
}
void meet(interval *a, interval *b) {
	int l1 = a->get_lower_bound();
	int u1 = a->get_upper_bound();
	int l2 = b->get_lower_bound();
	int u2 = b->get_upper_bound();
	if (u1 < l2 || u2 < l1) {
		std::cout << "Invalid";
	}
	else {
		a->set_lower_bound(max(l1, l2));
		a->set_upper_bound(min(u1, u2));
	}
}
void join(interval *a, interval *b) {
	int l1 = a->get_lower_bound();
	int u1 = a->get_upper_bound();
	int l2 = b->get_lower_bound();
	int u2 = b->get_upper_bound();
	a->set_lower_bound(min(l1, l2));
	a->set_upper_bound(max(u1, u2));
}
void not_equals(interval *a, interval *b) {

}