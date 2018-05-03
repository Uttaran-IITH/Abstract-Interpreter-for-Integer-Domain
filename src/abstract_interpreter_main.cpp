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

// signed_interval handle_rhs(exprt expression, 
// 							std::map<irep_idt, signed_interval*> &signed_interval_map, 
// 							symbol_tablet &symbol_table)
// {

// 	if(expression.id() == ID_symbol)
// 	{
// 		symbol_exprt sym = to_symbol_expr(expression);
// 		const symbolt* symbol = symbol_table.lookup(sym.get_identifier());
// 		std::map<irep_idt, signed_interval*>::iterator it ;
// 		it = signed_interval_map.find(symbol->name);
// 		signed_interval* object ;
		
// 		if(it!= signed_interval_map.end())
// 		{
// 			object = it->second;
// 			return *object ;
// 		}

// 		else
// 		{
// 			std::cout<<"Variable not found\n\n";
// 					 	signed_interval* obj = new signed_interval();
// 		 	return *obj;
// 		}
// 	}

// 	else
// 	{
// 		if(expression.id() == ID_plus)
// 		{
// 			plus_exprt plus_expr = to_plus_expr(expression);
// 			std::cout<<"Plus expression found\n";
// 			signed_interval obj1 = handle_rhs(plus_expr.op0(), signed_interval_map, symbol_table);
// 			signed_interval obj2 = handle_rhs(plus_expr.op1(), signed_interval_map, symbol_table);

// 			std::cout<<"OBJECT 1 :"<<obj1.get_lower_bound()<<" "<<obj1.get_upper_bound()<<std::endl;
// 			std::cout<<"OBJECT 2 :"<<obj2.get_lower_bound()<<" "<<obj2.get_upper_bound()<<std::endl;

// 			return obj1;
// 		}

// 		else
// 		{
// 		 	signed_interval* obj = new signed_interval();
// 		 	return *obj;
// 		}
// 	}
// }