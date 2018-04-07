/***********************************************
 Description:
 ***********************************************/
#include <climits>
#include "interval.h"


interval :: interval(integer_type type)
{
	if(type == SIGNED)
	{
		lower_bound = -1 ;
		upper_bound = -1 ;
		sign = SIGNED ;
		plus_inf = true ;
		minus_inf  = false ;
	}
	
	else
	{
		lower_bound = 0 ;
		upper_bound = -1 ;
		sign = UNSIGNED ;
		plus_inf = true ;
		minus_inf = false ;
	}	
}

int interval :: get_lower_bound()
{
	return lower_bound;
}

int interval :: get_upper_bound()
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

// signed_interval::signed_interval()
// {
// 	lower_bound = INT_MIN ;
// 	upper_bound = INT_MAX ;
// 	plus_inf = true ;
// 	minus_inf = false ;
// }

// int signed_interval :: get_lower_bound()
// {
// 	return lower_bound;
// }

// int signed_interval :: get_upper_bound()
// {
// 	 return upper_bound ;
// }



// unsigned_interval :: unsigned_interval()
// {
// 	lower_bound = 0;
// 	upper_bound = UINT_MAX ;
// 	plus_inf = true ;
// }

// unsigned int unsigned_interval :: get_lower_bound()
// {
// 	return lower_bound;
// }

// unsigned int unsigned_interval :: get_upper_bound()
// {
// 	 return upper_bound ;
// }