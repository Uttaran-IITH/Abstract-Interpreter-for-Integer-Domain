/***************************************
   LOOP CLONING : while and for
**************************************/  
#include <iostream>
#include <string>
#include <fstream>
#include "goto-programs/read_goto_binary.h"
#include "goto-programs/goto_functions.h"
#include "goto-programs/goto_program_template.h"
#include "langapi/mode.h"  //Required for register language function
#include "util/message.h"
#include "ansi-c/ansi_c_language.h"
#include "goto-programs/show_goto_functions.h"
#include "util/cmdline.h"
#include "goto-cc/compile.h"

int main()
{

		std::string filename = "/home/ubuntu/Desktop/CBMCPracticeStuff/GOTOProgs/nested_loops.o" ;
		
		stream_message_handlert message_handler(std::cout) ;

		goto_modelt goto_model ;
		
		register_language(new_ansi_c_language);

		read_goto_binary(filename, goto_model, message_handler);
		namespacet ns(goto_model.symbol_table);
			
		Forall_goto_functions(f_it, goto_model.goto_functions)
		{
			Forall_goto_program_instructions(it, f_it->second.body)
			{
				goto_programt::instructiont instruction = *it ;

				f_it->second.body.output_instruction(ns, "", std::cout, instruction)<<"\n";
			}
		}
}
