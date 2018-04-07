
enum integer_type{SIGNED=1 ,  UNSIGNED=2 };


class interval
{
	int lower_bound ;
	int upper_bound ;
	bool plus_inf ;
	bool minus_inf ;
	integer_type sign ;

	public :
	interval(integer_type type);
	int get_lower_bound();
	int get_upper_bound();
	integer_type get_sign();
	bool is_plus_inf();
	bool is_minus_inf();
};


// class signed_interval
// {
// 	int lower_bound;
// 	int upper_bound;
// 	bool plus_inf ;
// 	bool minus_inf ;

// 	public:
// 		signed_interval();
// 		int get_lower_bound();
// 		int get_upper_bound();	
// };

// class unsigned_interval
// {
// 	int lower_bound;
// 	int upper_bound;
// 	bool plus_inf ;

// 	public:
// 		unsigned_interval();
// 		unsigned int get_lower_bound();
// 		unsigned int get_upper_bound();
// };

