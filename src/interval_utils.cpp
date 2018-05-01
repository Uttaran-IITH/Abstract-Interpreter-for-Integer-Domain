#include "interval_utils.h"
#include <math.h>
#include <stdlib.h>
#include <iostream>

//Returns MAX of 2 integers
mp_integer min(mp_integer a, mp_integer b) 
{
	if (a < b)
		return a;
	else
		return b;
}

//Returns MIN of 2 integers
mp_integer max(mp_integer a, mp_integer b)
{
	if (a>b)
		return a;
	else
		return b;
}

//Adds two intervals and stores it in the 3rd argument
void add(interval a, interval b, interval *c)
{
	bool plus_inf = false;
	bool minus_inf =  false;

	if(a.is_plus_inf() || b.is_plus_inf())
		plus_inf = true ;

	if(a.is_minus_inf() || b.is_minus_inf())
		minus_inf =  true ;

	//Addition of infinity with something will be infinity (both positive and negative)
	c->set_lower_bound(a.get_lower_bound() + b.get_lower_bound(), minus_inf);
	c->set_upper_bound(a.get_upper_bound() + b.get_upper_bound(), plus_inf);
}

//Negates the interval (flips it)
void negate(interval *a)
{
	//Store the upper bound values and flags
	mp_integer temp = a->get_upper_bound();
	bool t = a->is_plus_inf();

	//Set the lower bound as negation of the upper bound, copy infinity flag
	a->set_upper_bound(-(a->get_lower_bound()), a->is_minus_inf());

	//Set lower bound to stored upper bound but signs are flipped
	a->set_lower_bound(-temp, t);
}


//Computes the difference between two intervals which is same as adding the negation
void sub(interval a, interval b, interval *c)
{
	negate(&b);
	add(a, b, c);
}

//Multiplies two intervals and stores it in the 3rd argument
void multiply(interval a, interval b, interval *c)
{
 	mp_integer temp[] = {
 		a.get_lower_bound() * b.get_lower_bound(),
 		a.get_lower_bound() * b.get_upper_bound(),
 		a.get_upper_bound() * b.get_lower_bound(),
 		a.get_upper_bound() * b.get_upper_bound()
 	};
	bool pos_inf = 0, neg_inf = 0;

	//Ways to get a positive infinity flag in the result
	if( (a.is_minus_inf() && (b.get_lower_bound() < 0 || b.get_upper_bound() < 0 || b.is_minus_inf())) || 
		(b.is_minus_inf() && (a.get_lower_bound() < 0 || a.get_upper_bound() < 0)) || 
		(a.is_plus_inf() && (b.is_plus_inf() || b.get_lower_bound() > 0 || b.get_upper_bound() > 0 )) ||
		(b.is_plus_inf() && (a.get_lower_bound() > 0 && a.get_upper_bound() > 0))  ) 
		pos_inf = 1 ;

	//Ways to get a negative infinity in the result
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
	if((b.get_lower_bound()==0 && !b.is_minus_inf()) || (b.get_upper_bound()==0 && !b.is_plus_inf())){
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
	mp_integer m = max(max(max(temp[0],temp[1]),temp[2]),temp[3]);
 	mp_integer n = min(min(min(temp[0],temp[1]),temp[2]),temp[3]);
	if((!a.is_minus_inf() && !a.is_plus_inf()) && (b.is_plus_inf() || b.is_minus_inf())){
		c->set_lower_bound(min(n,0),false);
		c->set_upper_bound(max(m,0),false);
	}
	else if ((a.is_minus_inf() || a.is_plus_inf()) && (!b.is_plus_inf() && !b.is_minus_inf())){
		c->set_lower_bound(n,a.is_minus_inf());
		c->set_upper_bound(m,a.is_plus_inf());
	}
	else{
		c->set_lower_bound(n,false);
		c->set_upper_bound(m,false);
	}
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
//Computes the meet of the interval
bool meet(interval *a, interval *b, interval* &c) {
	mp_integer l1 = a->get_lower_bound();
	mp_integer u1 = a->get_upper_bound();
	mp_integer l2 = b->get_lower_bound();
	mp_integer u2 = b->get_upper_bound();
	if ((u1 < l2 && !b->is_minus_inf()) || (u2 < l1 && !a->is_minus_inf())) {
		std::cout << "Invalid";
		return false ;
	}
	if((u1 == u2) && (a->is_plus_inf() == b->is_plus_inf()) && (l1 == l2) && (a->is_minus_inf() == b->is_minus_inf())){
		c = a;
		return true;
	}
	else 
	{
		c->set_lower_bound(max(l1,l2),a->is_minus_inf() && b->is_minus_inf());
		c->set_upper_bound(min(u1,u2),a->is_plus_inf() && b->is_plus_inf());
		maybe = true;
		return true ;
	}
}

bool equals(interval *a, interval *b, interval* &c) {
	mp_integer l1 = a->get_lower_bound();
	mp_integer u1 = a->get_upper_bound();
	mp_integer l2 = b->get_lower_bound();
	mp_integer u2 = b->get_upper_bound();
	if ((u1 < l2 && !b->is_minus_inf())|| (u2 < l1 && !a->is_minus_inf())) {
		std::cout<<"Infeasible Branch\n";
		return false ;
	}
	else 
		return meet(a,b,c);
}


bool less_than(interval *a, interval *b, interval *temp_a, interval *temp_b, int l) {
	mp_integer l1 = a->get_lower_bound();
 	mp_integer u1 = a->get_upper_bound();
 	mp_integer l2 = b->get_lower_bound();
 	mp_integer u2 = b->get_upper_bound();

	std::cout<<"\n\n PRINTING RESULT : ";
	std::cout<<l1<<" "<<u1<<" "<<l2<<" "<<u2<<" "<<"\n"; 	
 	if ( (!a->is_plus_inf()) && (u1 <= l2) && (!b->is_minus_inf()) ) {
		temp_a->make_equal(*a);
		temp_b->make_equal(*b);

		//temp_b->print_interval();
 		return true;
 	}
 	else if ( (!b->is_plus_inf()) && (u2 < l1) && (!a->is_minus_inf()) ) {
 		std::cout << "Invalid Branch \n";
		 return false;
 	}
 	else {
		temp_a->set_lower_bound(l1,a->is_minus_inf());
		temp_a->set_upper_bound(min(u1, u2),a->is_plus_inf() && b->is_plus_inf());
 	}
	if(l == 0)
	{	
		if(greater_than(b,a,temp_b,temp_a,1)){}

	}

	//temp_a->print_interval();
	//temp_b->print_interval();
	maybe = true;
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
 	else if ((!b->is_plus_inf()) && (u2 <= l1) && (!a->is_minus_inf())) {
 		std::cout<<"Entering HERE \n\n";
 		temp_a->make_equal(*a);
		temp_b->make_equal(*b);

		return true;
 	}
 	else {
 		temp_a->set_lower_bound(max(l1,l2),a->is_minus_inf() && b->is_minus_inf());
 		temp_a->set_upper_bound(u1,a->is_plus_inf());
 		std::cout<<"After this : \n";
 		temp_a->print_interval();
 	}
 	
	if(l == 0)
		if(less_than(b,a,temp_b,temp_a,1)){}


	std::cout<<"\n\nGreater than result : ";		
	temp_a->print_interval();
	temp_b->print_interval();	 
	 maybe = true;
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