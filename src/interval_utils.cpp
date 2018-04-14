#include "interval_utils.h"
#include <math.h>
#include <stdlib.h>
#include <iostream>
mp_integer min(mp_integer a, mp_integer b) {
	if (a<b)
		return a;
	else
		return b;
}
mp_integer max(mp_integer a, mp_integer b) {
	if (a>b)
		return a;
	else
		return b;
}
void add(interval a, interval b, interval *c) {
	bool plus_inf = false;
	bool minus_inf =  false;

	if(a.is_plus_inf() || b.is_plus_inf())
		plus_inf = true ;

	if(a.is_minus_inf() || b.is_minus_inf())
		minus_inf =  true ;

	c->set_lower_bound(a.get_lower_bound() + b.get_lower_bound(), minus_inf);
	c->set_upper_bound(a.get_upper_bound() + b.get_upper_bound(), plus_inf);
}

void negate(interval *a) {
	mp_integer temp = a->get_upper_bound();
	a->set_upper_bound(-(a->get_lower_bound()), a->is_plus_inf());
	a->set_lower_bound(-temp, a->is_minus_inf());
}

void sub(interval a, interval b, interval *c) {
	negate(&b);
	add(a, b, c);
}

void multiply(interval a , interval b , interval* c)
{
	mp_integer temp1 = a.get_lower_bound() * b.get_lower_bound() ;
	mp_integer temp2 = a.get_lower_bound() * b.get_upper_bound() ;
	mp_integer temp3 = a.get_upper_bound() * b.get_lower_bound() ;
	mp_integer temp4 = a.get_upper_bound() * b.get_upper_bound() ;

	mp_integer min , max ;

	mp_integer x, y ;

	//Find max
	x = ((temp1>temp2)? temp1 : temp2 );	
	y = ((x>temp3)? x : temp3);
	max  = ((y>temp4)? y : temp4);

	//Find min
	x = ((temp1<temp2)? temp1 : temp2 );	
	y = ((x<temp3)? x : temp3);
	min  = ((y<temp4)? y : temp4);

	bool plus_inf = false;
	bool minus_inf =  false;

	if(a.is_plus_inf() || b.is_plus_inf())
		plus_inf = true ;

	if(a.is_minus_inf() || b.is_minus_inf())
		minus_inf =  true ;

	c->set_lower_bound(min, minus_inf);
	c->set_upper_bound(max, plus_inf);
}
// void multiply(interval a, interval b, interval *c) {
// 	mp_integer temp[] = {
// 		a.get_lower_bound() * b.get_lower_bound(),
// 		a.get_lower_bound() * b.get_upper_bound(),
// 		a.get_upper_bound() * b.get_lower_bound(),
// 		a.get_upper_bound() * b.get_upper_bound()
// 	};

// 	mp_integer min = temp[0];
// 	mp_integer max = temp[0];
// 	for (mp_integer i = 1; i < 4; i+=1) {
// 		if (min > temp[i])
// 			min = temp[i];
// 		if (max < temp[i])
// 			max = temp[i];
// 	}

// 	bool plus_inf = false;
// 	bool minus_inf =  false;

// 	if(a.is_plus_inf() || b.is_plus_inf())
// 		plus_inf = true ;

// 	if(a.is_minus_inf() || b.is_minus_inf())
// 		minus_inf =  true ;

// 	c->set_lower_bound(min, minus_inf);
// 	c->set_upper_bound(max, plus_inf);
// }

// void power(interval *a, mp_integer p) {
// 	mp_integer sign[] = { 1,1 };
// 	if (a->get_lower_bound() < 0)
// 		sign[0] = -1;
// 	if (a->get_upper_bound() < 0)
// 		sign[1] = -1;
// 	a->set_lower_bound(sign[0] * abs(pow(a->get_lower_bound(), p)));
// 	a->set_upper_bound(sign[1] * abs(pow(a->get_upper_bound(), p)));
// }
// void less_than(interval *a, interval *b) {
// 	mp_integer l1 = a->get_lower_bound();
// 	mp_integer u1 = a->get_upper_bound();
// 	mp_integer l2 = b->get_lower_bound();
// 	mp_integer u2 = b->get_upper_bound();
// 	if (u1 < l2) {
// 		return;
// 	}
// 	else if (u2 < l1) {
// 		std::cout << "Not valid";
// 	}
// 	else {
// 		a->set_lower_bound(l1);
// 		a->set_upper_bound(min(u1, u2));
// 	}
// }

// void greater_than(interval *a, interval *b) {
// 	mp_integer l1 = a->get_lower_bound();
// 	mp_integer u1 = a->get_upper_bound();
// 	mp_integer l2 = b->get_lower_bound();
// 	mp_integer u2 = b->get_upper_bound();
// 	if (u1 < l2) {
// 		std::cout << "Not valid";
// 	}
// 	else if (u2 < l1) {
// 		return;
// 	}
// 	else {
// 		a->set_lower_bound(max(l1,l2));
// 		a->set_upper_bound(u1);
// 	}
// }
// void meet(interval *a, interval *b) {
// 	mp_integer l1 = a->get_lower_bound();
// 	mp_integer u1 = a->get_upper_bound();
// 	mp_integer l2 = b->get_lower_bound();
// 	mp_integer u2 = b->get_upper_bound();
// 	if (u1 < l2 || u2 < l1) {
// 		std::cout << "Invalid";
// 	}
// 	else {
// 		a->set_lower_bound(max(l1, l2));
// 		a->set_upper_bound(min(u1, u2));
// 	}
// }
// void join(interval *a, interval *b) {
// 	mp_integer l1 = a->get_lower_bound();
// 	mp_integer u1 = a->get_upper_bound();
// 	mp_integer l2 = b->get_lower_bound();
// 	mp_integer u2 = b->get_upper_bound();
// 	a->set_lower_bound(min(l1, l2));
// 	a->set_upper_bound(max(u1, u2));
// }
// void not_equals(interval *a, interval *b) {

// }