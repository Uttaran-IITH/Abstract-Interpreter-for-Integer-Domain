/***********************************************
 Description:
 ***********************************************/
#include <climits>

class signed_interval
{
	int lower_bound;
	int upper_bound;
	public:
		signed_interval();
		int get_lower_bound();
		int get_upper_bound();	
};

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

class unsigned_interval
{
	int lower_bound;
	int upper_bound;
	public:
		unsigned_interval();
		unsigned int get_lower_bound();
		unsigned int get_upper_bound();
};

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