//Operations on intervals

#include "interval_utils.h"

interval add_intervals(interval arg1, interval arg2)
{
	interval* result ;

	if((arg1.get_sign() == integer_type::SIGNED) || 
		(arg2.get_sign() == integer_type::UNSIGNED))
	{
		result = new interval(integer_type::SIGNED);
	}
	else
	{
		result = new interval(integer_type::UNSIGNED);
	}

	result->set_lower_bound(arg1.get_lower_bound() + arg2.get_lower_bound());
	result->set_upper_bound(arg1.get_upper_bound() + arg2.get_upper_bound());

	return *result;
}