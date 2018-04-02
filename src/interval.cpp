/***********************************************
 Description:
 ***********************************************/
#include <climits>
#include "interval.h"


signed_interval::signed_interval()
{
	lower_bound = INT_MIN ;
	upper_bound = INT_MAX ;
}

int signed_interval :: get_lower_bound()
{
	return lower_bound;
}

int signed_interval :: get_upper_bound()
{
	 return upper_bound ;
}



unsigned_interval :: unsigned_interval()
{
	lower_bound = 0;
	upper_bound = UINT_MAX ;
}

unsigned int unsigned_interval :: get_lower_bound()
{
	return lower_bound;
}

unsigned int unsigned_interval :: get_upper_bound()
{
	 return upper_bound ;
}