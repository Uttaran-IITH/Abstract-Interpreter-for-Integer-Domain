//User Input Based
#include <iostream>
#include <map>
#include "/home/ubuntu/Desktop/SVProject/src/interval.cpp"
#include "goto-programs/read_goto_binary.h"
#include "goto-programs/goto_functions.h"
#include "goto-programs/goto_program_template.h"
#include "langapi/mode.h"  //Required for register language function
#include "util/message.h"
#include "ansi-c/ansi_c_language.h"
#include "goto-programs/show_goto_functions.h"
#include "util/cmdline.h"
#include "goto-cc/compile.h"


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

