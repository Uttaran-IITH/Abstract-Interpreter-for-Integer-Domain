#include "interval_utils.h"
#include <math.h>
#include <stdlib.h>
#include <iostream>

/*Returns Minimum of 2 mp_integers
 * Input -> Two mp_integers
 * Output -> One mp_integer
 */
mp_integer min(mp_integer a, mp_integer b) 
{
	if (a < b)
		return a;
	else
		return b;
}

/*Returns Maximum of 2 mp_integers
 * Input -> Two mp_integers
 * Output -> One mp_integer
 */
mp_integer max(mp_integer a, mp_integer b)
{
	if (a>b)
		return a;
	else
		return b;
}

/* Adds two intervals (a and b) and stores it in the 3rd argument (c)
 * Input -> Three intervals.
 * Output -> One interval
 */
void add(interval a, interval b, interval *c)
{
	bool plus_inf = false;
	bool minus_inf =  false;
	//Stores positive infinity flag
	if(a.is_plus_inf() || b.is_plus_inf())
		plus_inf = true ;
	//Stores negative infinity flag
	if(a.is_minus_inf() || b.is_minus_inf())
		minus_inf =  true ;

	//Addition of infinity with something will be infinity (both positive and negative)
	c->set_lower_bound(a.get_lower_bound() + b.get_lower_bound(), minus_inf);
	c->set_upper_bound(a.get_upper_bound() + b.get_upper_bound(), plus_inf);
}

/* Negates the interval without changing the original value
 * Input -> Two intervals.
 * Output -> One interval
 */
void negate(interval *a, interval *temp)
{
	temp->set_upper_bound(-a->get_lower_bound(),a->is_minus_inf());

	//Set the lower bound as negation of the upper bound, copy infinity flag

	//Set lower bound to stored upper bound but signs are flipped
	temp->set_lower_bound(-a->get_upper_bound(),a->is_plus_inf());
}

/* Negates the input interval (flips it)
 * Input -> One intervals.
 * Output -> One interval
 */
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



/*Computes the difference between two intervals which is same as adding the negation of the 2nd interval to 1st
 * Input -> Three intervals
 * Output -> One Interval
*/
void sub(interval a, interval b, interval *c)
{
	interval *temp = new interval(integer_type::SIGNED);
	negate(&b,temp);
	add(a, *temp, c);
}

/* Multiplies two intervals and stores it in the 3rd argument
 * Input -> Three intervals
 * Output -> One Interval
*/
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



/* Divides two intervals and stores it in the 3rd interval
 * Input -> Three intervals
 * Output -> One Interval
*/
bool divide(interval a,interval b,interval *c){

	//If the denominator interval is infinity in both directions
	if(b.is_minus_inf() && b.is_plus_inf()){
		c->set_lower_bound(-1,false);
		c->set_upper_bound(1,false);
		return true;
	}
	//If the denominator has zero inside the interval
	else if(b.get_lower_bound() * b.get_upper_bound() <= 0){
		c->set_lower_bound(0,true);
		c->set_upper_bound(0,true);
		return true;
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
	
		bool  pos_inf = false, neg_inf = false;

		//Possibility of getting a positive infinity
		if( (a.is_minus_inf() && (b.get_lower_bound() < 0 || b.get_upper_bound() < 0)) ||
			(a.is_plus_inf() && (b.get_lower_bound()> 0 || b.get_upper_bound() > 0))){
				pos_inf = true;

		}
		//Possibility of getting a negative infinity
		if( (a.is_minus_inf() && (b.get_lower_bound() > 0 || b.get_upper_bound() > 0)) ||
			(a.is_plus_inf() && (b.get_lower_bound()< 0 || b.get_upper_bound() < 0))){
				neg_inf = true;
		}
		c->set_lower_bound(n,neg_inf);
		c->set_upper_bound(m,pos_inf);
	}
	return true;
}
//void power(interval *a, unsigned int p) {
	//mp_integer n = pow(a->get_lower_bound(),p);
	//mp_integer m = pow(a->get_upper_bound(),p);
	//if (p%2 == 0 && n < 0)
		//n = -n;
	//a->set_lower_bound(n,a->is_minus_inf());
	//a->set_upper_bound(m,a->is_plus_inf());
//}


/*Computes the meet of the interval
 * Input -> Three intervals
 * Output -> One Interval
*/
bool meet(interval *a, interval *b, interval* &c) {
	mp_integer l1 = a->get_lower_bound();
	mp_integer u1 = a->get_upper_bound();
	mp_integer l2 = b->get_lower_bound();
	mp_integer u2 = b->get_upper_bound();
	
	//The intervals don't overlap so return false
	if ((u1 < l2 && !b->is_minus_inf()) || (u2 < l1 && !a->is_minus_inf())) {
		std::cout << "Invalid";
		return false ;
	}
	
	//One interval is subset of the another so return the subset interval
	if((u1 == u2) && (a->is_plus_inf() == b->is_plus_inf()) && (l1 == l2) && (a->is_minus_inf() == b->is_minus_inf())){
		c = a;
		return true;
	}
	
	//Else compute the meet
	else 
	{
		c->set_lower_bound(max(l1,l2),a->is_minus_inf() && b->is_minus_inf());
		c->set_upper_bound(min(u1,u2),a->is_plus_inf() && b->is_plus_inf());
		maybe = true;
		return true ;
	}
}


/*Checks if two intervals are equal
 * Input -> Three intervals
 * Output -> One Interval
*/
bool equals(interval *a, interval *b, interval* &c) {
	mp_integer l1 = a->get_lower_bound();
	mp_integer u1 = a->get_upper_bound();
	mp_integer l2 = b->get_lower_bound();
	mp_integer u2 = b->get_upper_bound();
	
	//No overlaping between the intervals so return false
	if ((u1 < l2 && !b->is_minus_inf())|| (u2 < l1 && !a->is_minus_inf())) {
		std::cout<<"Infeasible Branch\n";
		return false ;
	}
	
	//Compute the meet and return
	else 
		return meet(a,b,c);
}



/*Computes the interval range which is less than a given interval
 * Input -> Four intervals, the integer is not required as input
 * Output -> Two Intervals
*/
bool less_than(interval *a, interval *b, interval *temp_a, interval *temp_b, int l) {
	mp_integer l1 = a->get_lower_bound();
 	mp_integer u1 = a->get_upper_bound();
 	mp_integer l2 = b->get_lower_bound();
 	mp_integer u2 = b->get_upper_bound();

	//The entire interval (a) lies to the left of the other interval (b). So return them as they are
 	if ( (!a->is_plus_inf()) && (u1 <= l2) && (!b->is_minus_inf()) ) {
		temp_a->make_equal(*a);
		temp_b->make_equal(*b);
 		return true;
 	}
 	
 	//The entire interval (a) lies to the right of the other interval (b). So return false
 	else if ( (!b->is_plus_inf()) && (u2 < l1) && (!a->is_minus_inf()) ) {
 		std::cout << "Invalid Branch \n";
		 return false;
 	}
 	
 	//We compute the interval of (a) which is less than interval (b)
 	else {
		temp_a->set_lower_bound(l1,a->is_minus_inf());
		temp_a->set_upper_bound(min(u1, u2),a->is_plus_inf() && b->is_plus_inf());
 	}
 	
 	//Prevents infinte looping
	if(l == 0)
	{	
		//Interval of (b) which is greater than Interval (a)
		if(greater_than(b,a,temp_b,temp_a,1)){}

	}
	//Flag required for assert operations
	maybe = true;
	 return true;

 }



/*Computes the interval range which is greater than a given interval
 * Input -> Four intervals, the integer is not required as input
 * Output -> Two Intervals
*/
bool greater_than(interval *a, interval *b, interval *temp_a, interval *temp_b, int l) {
 	mp_integer l1 = a->get_lower_bound();
 	mp_integer u1 = a->get_upper_bound();
 	mp_integer l2 = b->get_lower_bound();
 	mp_integer u2 = b->get_upper_bound();

//The entire interval (a) lies to the left of the other interval (b). So return false
	if ((!a->is_plus_inf()) && (u1 < l2) && (!b->is_minus_inf()) ) {
 		std::cout << "Invalid Branch \n";
		 return false;
 	}

//The entire interval (a) lies to the right of the other interval (b). So return them as they are
 	else if ((!b->is_plus_inf()) && (u2 <= l1) && (!a->is_minus_inf())) {
 		std::cout<<"Entering HERE \n\n";
 		temp_a->make_equal(*a);
		temp_b->make_equal(*b);

		return true;
 	}
//Compute the intersecting interval	 
 	else {
 		temp_a->set_lower_bound(max(l1,l2),a->is_minus_inf() && b->is_minus_inf());
 		temp_a->set_upper_bound(u1,a->is_plus_inf());
 		std::cout<<"After this : \n";
 		temp_a->print_interval();
 	}
//Prevents infinite loop
	if(l == 0)
		if(less_than(b,a,temp_b,temp_a,1)){}

//Flag required for assert	 
	 maybe = true;
	 return true;
 }


/*Computes the join of the interval
 * Input -> Two intervals
 * Output -> One Interval
*/ 
void join(interval *a, interval *b) {
 	mp_integer l1 = a->get_lower_bound();
 	mp_integer u1 = a->get_upper_bound();
 	mp_integer l2 = b->get_lower_bound();
 	mp_integer u2 = b->get_upper_bound();
 	
	//Computing the join of two intervals 
	a->set_lower_bound(min(l1, l2), a->is_minus_inf() || b->is_minus_inf());
 	a->set_upper_bound(max(u1, u2), a->is_plus_inf() || b->is_plus_inf());
 }



/*Computes if intervals are not equal
 * Input -> Two intervals
 * Output -> One Interval
*/
bool not_equals(interval *a, interval *b) {
	mp_integer l1 = a->get_lower_bound();
 	mp_integer u1 = a->get_upper_bound();
 	mp_integer l2 = b->get_lower_bound();
 	mp_integer u2 = b->get_upper_bound();
 	
	//If both intervals are singleton sets and they are exactly equal then return false
	if ((l1 == u1) && (u1 == l2) && (l2 == u2) && !a->is_minus_inf() && !a->is_plus_inf() && !b->is_plus_inf() && !b->is_minus_inf()){
		 std::cout<<"Invalid Branch \n";
		 return false;
	 }
	 else
	 return true;

}


/*Widens the interval
 * Input -> Three intervals
 * Output -> One Interval
*/
bool widen(interval *a,interval *b,interval *temp){
	mp_integer l1 = a->get_lower_bound();
 	mp_integer u1 = a->get_upper_bound();
 	mp_integer l2 = b->get_lower_bound();
 	mp_integer u2 = b->get_upper_bound();
	
	//If the interval increases per interation set upper value to positive infinity
	if(l1 > l2){
			temp->set_upper_bound(a->get_upper_bound(), a->is_plus_inf());
			temp->set_lower_bound(b->get_lower_bound(),true);

			return true;
	}

	//If the interval decreases per iteration set lower value to negative infinity
	if (u1 < u2){
			temp->set_lower_bound(a->get_lower_bound(), a->is_minus_inf());
			temp->set_upper_bound(b->get_upper_bound(),true);
			return true ;
	}
	//Else we don't widen
	return false;
}