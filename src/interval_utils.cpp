#include "interval_utils.h"
#include <math.h>
#include <stdlib.h>
#include <iostream>
mp_integer min(mp_integer a, mp_integer b) {
	if (a < b)
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

bool meet(interval *a, interval *b, interval* &c) {
	mp_integer l1 = a->get_lower_bound();
	mp_integer u1 = a->get_upper_bound();
	mp_integer l2 = b->get_lower_bound();
	mp_integer u2 = b->get_upper_bound();
	if ((u1 < l2 && !b->is_minus_inf()) || (u2 < l1 && !a->is_minus_inf())) {
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
	if ((u1 < l2 && !b->is_minus_inf())|| (u2 < l1 && !a->is_minus_infinity()) {
		std::cout<<"Infeasible Branch\n";
		return false ;
	}
	else 
		return meet(a,b,c);
}

void multiply(interval a, interval b, interval *c) {
 	mp_integer temp[] = {
 		a.get_lower_bound() * b.get_lower_bound(),
 		a.get_lower_bound() * b.get_upper_bound(),
 		a.get_upper_bound() * b.get_lower_bound(),
 		a.get_upper_bound() * b.get_upper_bound()
 	};
	bool pos_inf = 0, neg_inf = 0;
	if( (a.is_minus_inf() && (b.get_lower_bound() < 0 || b.get_upper_bound() < 0 || b.is_minus_inf())) || 
		(b.is_minus_inf() && (a.get_lower_bound() < 0 || a.get_upper_bound() < 0)) || 
		(a.is_plus_inf() && (b.is_plus_inf() || b.get_lower_bound() > 0 || b.get_upper_bound() > 0 )) ||
		(b.is_plus_inf() && (a.get_lower_bound() > 0 && a.get_upper_bound() > 0))  ) 
		pos_inf = 1 ;
	if( (a.is_minus_inf() && (b.get_lower_bound() > 0 || b.get_upper_bound() > 0 || b.is_plus_inf())) || 
		(b.is_minus_inf() && (a.get_lower_bound() > 0 || a.get_upper_bound() > 0)) || 
		(a.is_plus_inf() && (b.is_minus_inf() || b.get_lower_bound() < 0 || b.get_upper_bound() < 0 )) ||
		(b.is_plus_inf() && (a.get_lower_bound() < 0 && a.get_upper_bound() < 0))  ) 
		neg_inf = 1 ;
		
	mp_integer m = max(max(max(temp[0],temp[1]),temp[2]),temp[3]);
 	mp_integer n = min(min(min(temp[0],temp[1]),temp[2]),temp[3]);
	c->set_upper_bound(m, pos_inf);
	c->set_lower_bound(n, neg_inf);
}
bool divide(interval a,interval b,interval *c){
	if(b.get_lower_bound()==0 || b.get_upper_bound()==0){
		std::cout<<"Cannot divide by 0\n";
		return false;
	}
	else{
		mp_integer temp[] = {
 		a.get_lower_bound() / b.get_lower_bound(),
 		a.get_lower_bound() / b.get_upper_bound(),
 		a.get_upper_bound() / b.get_lower_bound(),
 		a.get_upper_bound() / b.get_upper_bound()
 	};
	bool pos_inf = 0, neg_inf = 0;
	if( (a.is_minus_inf() && (b.get_lower_bound() < 0 || b.get_upper_bound() < 0 || b.is_minus_inf())) || 
		(b.is_minus_inf() && (a.get_lower_bound() < 0 || a.get_upper_bound() < 0)) || 
		(a.is_plus_inf() && (b.is_plus_inf() || b.get_lower_bound() > 0 || b.get_upper_bound() > 0 )) ||
		(b.is_plus_inf() && (a.get_lower_bound() > 0 && a.get_upper_bound() > 0))  ) 
		pos_inf = 1 ;
	if( (a.is_minus_inf() && (b.get_lower_bound() > 0 || b.get_upper_bound() > 0 || b.is_plus_inf())) || 
		(b.is_minus_inf() && (a.get_lower_bound() > 0 || a.get_upper_bound() > 0)) || 
		(a.is_plus_inf() && (b.is_minus_inf() || b.get_lower_bound() < 0 || b.get_upper_bound() < 0 )) ||
		(b.is_plus_inf() && (a.get_lower_bound() < 0 && a.get_upper_bound() < 0))  ) 
		neg_inf = 1 ;
 	mp_integer m = max(max(max(temp[0],temp[1]),temp[2]),temp[3]);
 	mp_integer n = min(min(min(temp[0],temp[1]),temp[2]),temp[3]);
	c->set_upper_bound(m + 1, pos_inf);
	c->set_lower_bound(n, neg_inf);
	}
	return true;
}
void power(interval *a, unsigned int p) {
	mp_integer n = pow(a->get_lower_bound(),p);
	mp_integer m = pow(a->get_upper_bound(),p);
	if (p%2 == 0 && n < 0)
		n = -n;
	a->set_lower_bound(n,a->is_minus_inf());
	a->set_upper_bound(m,a->is_plus_inf());
}
bool less_than(interval *a, interval *b, interval *temp_a, interval *temp_b, int l) {
	mp_integer l1 = a->get_lower_bound();
 	mp_integer u1 = a->get_upper_bound();
 	mp_integer l2 = b->get_lower_bound();
 	mp_integer u2 = b->get_upper_bound();

	std::cout<<"\n\n PRINTING RESULT : ";
	std::cout<<l1<<" "<<u1<<" "<<l2<<" "<<u2<<" "<<"\n"; 	
 	if ( (!a->is_plus_inf()) && (u1 < l2) && (!b->is_minus_inf()) ) {
 		return true;
 	}
 	else if ( (!b->is_plus_inf()) && (u2 < l1) && (!a->is_minus_inf()) ) {
 		std::cout << "Invalid Branch \n";
		 return false;
 	}
 	else {
		temp_a->set_lower_bound(l1,a->is_minus_inf());
		temp_b->set_upper_bound(min(u1, u2),a->is_plus_inf() && b->is_plus_inf());
 	}
	if(l == 0)
	{	
		if(greater_than(b,a,temp_b,temp_a,1)){}

	}

	temp_a->print_interval();
	temp_b->print_interval();
	 return true;

 }

bool greater_than(interval *a, interval *b, interval *temp_a, interval *temp_b, int l) {
 	mp_integer l1 = a->get_lower_bound();
 	mp_integer u1 = a->get_upper_bound();
 	mp_integer l2 = b->get_lower_bound();
 	mp_integer u2 = b->get_upper_bound();

	std::cout<<"\n\n PRINTING RESULT : ";
	std::cout<<l1<<" "<<l2<<" "<<u1<<" "<<u2<<" "<<"\n";
 	if ((!a->is_plus_inf()) && (u1 < l2) && (!b->is_minus_inf()) ) {
 		std::cout << "Invalid Branch \n";
		 return false;
 	}
 	else if ((!b->is_plus_inf()) && (u2 < l1) && (!a->is_minus_inf())) {
 		return true;
 	}
 	else {
 		temp_a->set_lower_bound(max(l1,l2),a->is_minus_inf() && b->is_minus_inf());
 		temp_a->set_upper_bound(u1,a->is_plus_inf());
 	}
 	
	if(l == 0)
		if(less_than(b,a,temp_b,temp_a,1)){}


	temp_a->print_interval();
	temp_b->print_interval();	 

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
 	if ((l1 == u1) && (u1 == l2) && (l2 == u2) && !a->is_minus_inf() && !a->is_plus_inf() && !b->is_plus_inf() && !b->is_minus_inf()){
		 std::cout<<"Invalid Branch \n";
		 return false;
	 }
	 else
	 return true;

}
bool widen(interval *a,interval *b,interval *temp){
	mp_integer l1 = a->get_lower_bound();
 	mp_integer u1 = a->get_upper_bound();
 	mp_integer l2 = b->get_lower_bound();
 	mp_integer u2 = b->get_upper_bound();
	if(l1 > l2){
			temp->set_upper_bound(a->get_upper_bound(), a->is_plus_inf());
			temp->set_lower_bound(b->get_lower_bound(),true);

			return true;
	}
	if (u1 < u2){

			std::cout<<"Should Come Here for Widening\n";

			temp->set_lower_bound(a->get_lower_bound(), a->is_minus_inf());
			temp->set_upper_bound(b->get_upper_bound(),true);
			return true ;
	}
	
	return false;
}
