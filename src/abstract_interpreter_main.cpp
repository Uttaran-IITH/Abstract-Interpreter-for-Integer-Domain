/***************************************************
  Description : main
  ***************************************************/
#include <iostream>
#include <string>

#include "abstract_interpreter.h"


#include "goto-programs/read_goto_binary.h"
#include "goto-programs/goto_functions.h"
#include "goto-programs/goto_program.h"
#include "langapi/mode.h"  //Required for register language function
#include "util/message.h"
#include "ansi-c/ansi_c_language.h"
#include "goto-programs/show_goto_functions.h"
#include "util/cmdline.h"
#include "goto-cc/compile.h"


int main(int argc, const char **argv)
{

		std::string filename ;


		if(argc == 1 )
			std::cout<<"Please provide an input file ";
		else if(argc ==2)
			filename = argv[1];
		else
			std::cout<<"Donot provide more than one argument";

		stream_message_handlert message_handler(std::cout);

		goto_modelt goto_model ;
		
		register_language(new_ansi_c_language);

		read_goto_binary(filename, goto_model, message_handler);
		namespacet ns(goto_model.symbol_table);
			
		abstract_interpreter interpreter ;

		//goto_model.goto_functions.output(ns, std::cout);
		interpreter.run_interpreter(goto_model);	
}