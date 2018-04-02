//User Input Based
#include <iostream>
#include <map>

#include "/home/ubuntu/Desktop/SVProject/src/interval.h"

#include "goto-programs/goto_program_template.h"
#include "goto-programs/goto_model.h"



class abstract_interpreter
{
	private:
		std::map<irep_idt, signed_interval*> signed_interval_map ;
		std::map<irep_idt, unsigned_interval*> unsigned_interval_map ;

	public:
		void run_interpreter(goto_modelt &goto_model);
		void print_all();
		void handle_declaration(goto_programt::instructiont &instruction, goto_modelt &goto_model);
		void handle_assignments(goto_programt::instructiont &instruction, goto_modelt &goto_model);	
};

