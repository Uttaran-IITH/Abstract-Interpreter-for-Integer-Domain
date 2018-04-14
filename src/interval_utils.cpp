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
	bool t = a->is_plus_inf();
	a->set_upper_bound(-(a->get_lower_bound()), a->is_minus_inf());
	a->set_lower_bound(-temp, t);
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
	mp_integer m,n ;
	m = max(max(max(temp1,temp2),temp3),temp4);
	n = min(min(min(temp1,temp2),temp3),temp4);
	c->set_upper_bound(m, a.is_plus_inf() || b.is_plus_inf());
	c->set_lower_bound(n, a.is_minus_inf() || b.is_minus_inf());
}

bool meet(interval *a, interval *b, interval* &c) {
	mp_integer l1 = a->get_lower_bound();
	mp_integer u1 = a->get_upper_bound();
	mp_integer l2 = b->get_lower_bound();
	mp_integer u2 = b->get_upper_bound();
	if (u1 < l2 || u2 < l1) {
		std::cout << "Invalid";
		return false ;
	}
	else 
	{
		c->set_lower_bound(max(l1,l2),a->is_minus_inf() && b->is_minus_inf());
		c->set_upper_bound(min(u1,u2),a->is_plus_inf() && b->is_plus_inf());
		return true ;
	}
}

bool equals(interval *a, interval *b, interval* &c) {
	mp_integer l1 = a->get_lower_bound();
	mp_integer u1 = a->get_upper_bound();
	mp_integer l2 = b->get_lower_bound();
	mp_integer u2 = b->get_upper_bound();
	if (u1 < l2 || u2 < l1) {
		std::cout<<"Infeasible Branch\n";
		return false ;
	}
	else {
		
		if(meet(a, b, c))
			return true ;
		else
			return false;
	}
}

void multiply(interval a, interval b, interval *c) {
 	mp_integer temp[] = {
 		a.get_lower_bound() * b.get_lower_bound(),
 		a.get_lower_bound() * b.get_upper_bound(),
 		a.get_upper_bound() * b.get_lower_bound(),
 		a.get_upper_bound() * b.get_upper_bound()
 	};

 	mp_integer m = max(max(max(temp[0],temp[1]),temp[2]),temp[3]);
 	mp_integer n = min(min(min(temp[0],temp[1]),temp[2]),temp[3]);
	c->set_upper_bound(m, a.is_plus_inf() && b.is_plus_inf());
	c->set_lower_bound(n, a.is_minus_inf() && b.is_minus_inf());
}
 //void power(interval *a, mp_integer p) {
	// mp_integer n = pow(double(a->get_lower_bound()),(double)p);
	 //mp_integer m = pow((double)a->get_upper_bound(),(double)p);

//}
 bool less_than(interval *a, interval *b) {
 	mp_integer l1 = a->get_lower_bound();
 	mp_integer u1 = a->get_upper_bound();
 	mp_integer l2 = b->get_lower_bound();
 	mp_integer u2 = b->get_upper_bound();
 	if (u1 < l2) {
 		return true;
 	}
 	else if (u2 < l1) {
 		std::cout << "Invalid Branch \n";
		 return false;
 	}
 	else {
		a->set_lower_bound(l1,a->is_minus_inf());
		a->set_upper_bound(min(u1, u2),a->is_plus_inf() && b->is_plus_inf());
 	}
	 return true;
 }

bool greater_than(interval *a, interval *b) {
 	mp_integer l1 = a->get_lower_bound();
 	mp_integer u1 = a->get_upper_bound();
 	mp_integer l2 = b->get_lower_bound();
 	mp_integer u2 = b->get_upper_bound();
 	if (u1 < l2) {
 		std::cout << "Invalid Branch \n";
		 return false;
 	}
 	else if (u2 < l1) {
 		return true;
 	}
 	else {
 		a->set_lower_bound(max(l1,l2),a->is_minus_inf() && b->is_minus_inf());
 		a->set_upper_bound(u1,a->is_plus_inf());
 	}
	 return true;
 }
void join(interval *a, interval *b) {
 	mp_integer l1 = a->get_lower_bound();
 	mp_integer u1 = a->get_upper_bound();
 	mp_integer l2 = b->get_lower_bound();
 	mp_integer u2 = b->get_upper_bound();
 	a->set_lower_bound(min(l1, l2), a->is_minus_inf() || b->is_minus_inf());
 	a->set_upper_bound(max(u1, u2), a->is_plus_inf() || b->is_plus_inf());
 }
bool not_equals(interval *a, interval *b) {
	mp_integer l1 = a->get_lower_bound();
 	mp_integer u1 = a->get_upper_bound();
 	mp_integer l2 = b->get_lower_bound();
 	mp_integer u2 = b->get_upper_bound();
 	if ((l1 == u1) && (u1 == l2) && (l2 == u2)){
		 std::cout<<"Invalid Branch \n";
	 }
	 else
	 return true;

}