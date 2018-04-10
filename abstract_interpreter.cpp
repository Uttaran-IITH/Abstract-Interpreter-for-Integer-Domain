#include <iostream>
#include <map>

#include "goto-programs/goto_functions.h"
#include "goto-programs/goto_program_template.h"

//Include
#include "abstract_interpreter.h"



void abstract_interpreter::run_interpreter(goto_modelt &goto_model)
{
	std::cout<<"Running Interpreter : \n";

	Forall_goto_functions(f_it, goto_model.goto_functions)
	{
		Forall_goto_program_instructions(it, f_it->second.body)
		{
			goto_programt::instructiont instruction = *it ;

			std::cout<<"Instruction : "<<it->to_string()<<"\n";

			switch(it->type)
			{
				case goto_program_instruction_typet::DECL :  handle_declaration(*it, goto_model); break;

				case goto_program_instruction_typet::ASSIGN : handle_assignments(*it, goto_model); break ;

				default: std::cout<<"Cannot Recognise the instruction\n";	
			}
		}	
	}
}


void abstract_interpreter :: handle_declaration(goto_programt::instructiont &instruction, goto_modelt &goto_model)
{
	code_declt decl = to_code_decl(instruction.code);
	symbol_exprt symbol_expr = to_symbol_expr(decl.symbol());
	symbolt symbol1 = goto_model.symbol_table.lookup(symbol_expr.get_identifier());
        symbolt* symbol = &symbol1;
        
	if(symbol->type.id() == ID_signedbv)
	{
		std::map<irep_idt, interval*>::iterator it ;

		it = interval_map.find(symbol->name);

		if(it !=interval_map.end())
		{
			std::cout<<"Double Declaration O.O";
		}

		else
		{
			interval_map.insert(std::pair<irep_idt, interval*>(symbol->name, new interval(integer_type::SIGNED)));
		}
	}

	else if(symbol->type.get(ID_C_c_type) ==  ID_unsignedbv)
	{
		std::map<irep_idt, interval*>::iterator it ;

		it = interval_map.find(symbol->name);

		if(it != interval_map.end())
		{
			std::cout<<"Double Declaration O.O";
		}

		else
		{
			interval_map.insert(std::pair<irep_idt, interval*>(symbol->name, new interval(integer_type::UNSIGNED)));
		}
	}
}


void abstract_interpreter :: handle_assignments(goto_programt::instructiont &instruction, goto_modelt &goto_model)
{
	
}