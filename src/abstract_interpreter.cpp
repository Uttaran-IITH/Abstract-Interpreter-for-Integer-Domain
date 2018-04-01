#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <climits>

#include "goto-programs/read_goto_binary.h"
#include "goto-programs/goto_functions.h"
#include "goto-programs/goto_program_template.h"
#include "langapi/mode.h"  //Required for register language function
#include "util/message.h"
#include "ansi-c/ansi_c_language.h"
#include "goto-programs/show_goto_functions.h"
#include "util/cmdline.h"

//Include
#include "/home/ubuntu/Desktop/SVProject/src/abstract_interpreter.h"



void abstract_interpreter::run_interpreter(goto_modelt &goto_model)
{
	std::cout<<"Running Interpreter : \n";

	Forall_goto_functions(f_it, goto_model.goto_functions)
	{
		Forall_goto_program_instructions(it, f_it->second.body)
		{
			goto_programt::instructiont instruction = *it ;

			switch(it->type)
			{
				case goto_program_instruction_typet::DECL :
					handle_declaration(instruction, goto_model); break;

				case goto_program_instruction_typet::ASSIGN :
					handle_assignments(instruction, goto_model); break ;

				default: std::cout<<"Cannot Recognise the instruction\n";	
			}
		}	
	}
}

void abstract_interpreter :: handle_declaration(goto_programt::instructiont &instruction, goto_modelt &goto_model)
{
	code_declt decl = to_code_decl(instruction.code);
	symbol_exprt symbol_expr = to_symbol_expr(decl.symbol());
	const symbolt* symbol = goto_model.symbol_table.lookup(symbol_expr.get_identifier());

	if(symbol->type.get(ID_C_c_type) == ID_signed_int)
	{
		std::map<irep_idt, signed_interval*>::iterator s_it ;

		s_it = signed_interval_map.find(symbol->name);

		if(s_it != signed_interval_map.end())
		{
			std::cout<<"Double Declaration O.O";
		}

		else
		{
			signed_interval_map.insert(std::pair<irep_idt, signed_interval*>(symbol->name, new signed_interval()));
		}
	}

	else if(symbol->type.get(ID_C_c_type) ==  ID_unsigned_int)
	{
		std::map<irep_idt, unsigned_interval*>::iterator u_it ;

		u_it = unsigned_interval_map.find(symbol->name);

		if(u_it != unsigned_interval_map.end())
		{
			std::cout<<"Double Declaration O.O";
		}

		else
		{
			unsigned_interval_map.insert(std::pair<irep_idt, unsigned_interval*>(symbol->name, new unsigned_interval()));
		}
	}
}


void handle_assignments(goto_programt::instructiont &instruction, goto_modelt &goto_model)
{
	
}