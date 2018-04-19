//User Input Based
#include <iostream>
#include <map>

#include "interval.h"

#include "goto-programs/goto_program.h"
#include "goto-programs/goto_model.h"
#include "analyses/natural_loops.h"


class abstract_interpreter
{
	private:
		// std::map<irep_idt, signed_interval*> signed_interval_map ;
		// std::map<irep_idt, unsigned_interval*> unsigned_interval_map ;
		std::map<irep_idt, interval*> interval_map;
		int threshold = -1 ;

		interval handle_rhs(exprt& expression, goto_modelt& goto_model);
		void set_rhs(exprt &rhs_expr, interval* &rhs , goto_modelt &goto_model);
		void set_lhs(symbol_exprt &lhs_sym, interval* &lhs, goto_modelt &goto_model);
		bool check_if_loop(natural_loops_mutablet &loops, goto_programt::targett &target);
		bool check_condition(exprt &expr, goto_modelt &goto_model, namespacet &ns);
	public:
		void run_interpreter(goto_modelt &goto_model);
		void print_all();
		void handle_declaration(goto_programt::instructiont &instruction, goto_modelt &goto_model);
		void handle_assignments(goto_programt::instructiont &instruction, goto_modelt &goto_model);	
		void handle_goto(goto_programt::instructiont &instruction, goto_modelt &goto_model, 
						goto_programt::targett &it, bool &target_changed);	
		void handle_loops(natural_loops_mutablet::natural_loopt &current_loop ,  natural_loops_mutablet &all_loops, goto_modelt &goto_model, namespacet &ns);

};

