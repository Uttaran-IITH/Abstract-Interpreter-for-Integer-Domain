/**************************************************
 Description: Functions on a single interval object
 **************************************************/
#include <climits>
#include <iostream>
#include <string>
#include "interval.h"


interval :: interval(integer_type type)
{
	if(type == SIGNED)
	{
		lower_bound = INT_MIN ;		
		upper_bound = INT_MAX ;
		sign = SIGNED ;
		plus_inf = true ;
		minus_inf  = true ;
	}
	
	else
	{
		lower_bound = 0 ;
		upper_bound = INT_MAX ;
		sign = UNSIGNED ;
		plus_inf = true ;
		minus_inf = false ;
	}	
}

mp_integer interval :: get_lower_bound()
{
	return lower_bound;
}

mp_integer interval :: get_upper_bound()
{
	 return upper_bound ;
}

integer_type interval :: get_sign()
{
	return sign ;
}

bool interval :: is_plus_inf()
{
	return plus_inf;
}

bool interval :: is_minus_inf()
{
	return minus_inf;
}

void interval :: set_lower_bound(mp_integer value, bool minus_inf_temp)
{
	lower_bound = value ;
	minus_inf = minus_inf_temp ;

	if(minus_inf)
	{

		lower_bound = INT_MIN ;
		//std::cout<<"LOWER BOUND  : "<<lower_bound<<"\n\n";
	}
}

void interval :: set_upper_bound(mp_integer value, bool plus_inf_temp)
{
	upper_bound = value ;
	plus_inf = plus_inf_temp ;

	if(plus_inf)
		upper_bound = INT_MAX;
}

void interval :: print_interval()
{
	std::string lower = "" ;
	std::string upper = "" ;
	if((sign == SIGNED) && (minus_inf==true))
		lower  = "-inf";

	if(plus_inf == true)
		upper = "+inf" ;


	std::cout<<"[ " ;

	if(lower == "")
		std::cout<<lower_bound ;
	else
		std::cout<<lower ; //<<" "<<lower_bound;
	
	std::cout<<" , " ;

	if(upper == "")
		std::cout<<upper_bound ;
	else
		std::cout<<upper ; //<<" "<<upper_bound ;

	std::cout<<" ]";  
}

void interval :: make_equal(interval temp_interval)
{
	// if(this->get_sign() == integer_type::SIGNED)
	// {
		this->lower_bound = temp_interval.get_lower_bound();
		this->upper_bound = temp_interval.get_upper_bound();
		this->minus_inf = temp_interval.is_minus_inf();
		this->plus_inf = temp_interval.is_plus_inf();

	// 	std::cout<<"SIGNED\n\n";
	// }
	// else
	// {
	// 	this->plus_inf = temp_interval.is_plus_inf();
	// 	this->minus_inf = false ;
		
	// 	if(temp_interval.get_lower_bound()<0)	
	// 		this->lower_bound = 0 ;
	// 	else
	// 		this->lower_bound = temp_interval.get_lower_bound();

	// 	if(temp_interval.get_upper_bound()<0)
	// 		this->upper_bound = 0 ;
	// 	else
	// 		this->upper_bound = temp_interval.get_upper_bound();

	// 	std::cout<<"UNSIGNED\n\n";

	// }
}