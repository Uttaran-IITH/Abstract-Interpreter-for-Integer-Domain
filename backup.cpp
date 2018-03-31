/***************************************
   LOOP CLONING : while and for
**************************************/  
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
#include "goto-cc/compile.h"
#include "util/c_types.h"
#include "ansi-c/c_typecast.h"


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

int main()
{

		std::string filename = "/home/ubuntu/Desktop/SVProject/example.o" ;
		
		stream_message_handlert message_handler(std::cout);

		goto_modelt goto_model ;
		
		register_language(new_ansi_c_language);

		read_goto_binary(filename, goto_model, message_handler);
		namespacet ns(goto_model.symbol_table);
			
		std::map<irep_idt, signed_interval*> signed_interval_map ;
		std::map<irep_idt, unsigned_interval*> unsigned_interval_map ;

		Forall_goto_functions(f_it, goto_model.goto_functions)
		{
			Forall_goto_program_instructions(it, f_it->second.body)
			{
				goto_programt::instructiont instruction = *it ;

				if(it->type ==  goto_program_instruction_typet::DECL)
				{
				 		code_declt decl = to_code_decl(it->code);
				 		symbol_exprt declared_symbol = to_symbol_expr(decl.symbol());
				 		const symbolt* symbol = goto_model.symbol_table.lookup(declared_symbol.get_identifier());
				 		symbol->show(std::cout);
				 		//_typet ctype = get_c_type(symbol->type);
				 		if(symbol->type.get(ID_C_c_type) == ID_signed_int)
				 			std::cout<<"Signed ID_integer";
				 		else if(symbol->type.get(ID_C_c_type) == ID_unsigned_int)
				 			std::cout<<"Unsigned Integer";

				}
				//f_it->second.body.output_instruction(ns, "", std::cout, instruction)<<"\n";
			}
		}
}
