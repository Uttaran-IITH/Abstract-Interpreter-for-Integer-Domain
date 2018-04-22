#ifndef INTERVAL_H
#define INTERVAL_H

#include "util/mp_arith.h"


enum integer_type{SIGNED=1 ,  UNSIGNED=2 };


class interval
{
	mp_integer lower_bound ;
	mp_integer upper_bound ;
	bool plus_inf ;
	bool minus_inf ;
	integer_type sign ;

	public :
	interval(integer_type type);
	mp_integer get_lower_bound();
	mp_integer get_upper_bound();
	void set_upper_bound(mp_integer value, bool plus_inf_temp);
	void set_lower_bound(mp_integer value, bool minus_inf_temp);
	integer_type get_sign();
	bool is_plus_inf();
	bool is_minus_inf();

	void print_interval();	
	void make_equal(interval temp_interval);
};

#endif

