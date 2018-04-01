/***************************************
   LOOP CLONING : while and for
**************************************/
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <climits>
#include "/home/uttaran/cbmc/src/goto-programs/read_goto_binary.h"
#include "/home/uttaran/cbmc/src/goto-programs/goto_functions.h"
//#include "/home/uttaran/cbmc/src/goto-programs/goto_program_template.h"
#include "/home/uttaran/cbmc/src/langapi/mode.h" //Required for register language function
#include "/home/uttaran/cbmc/src/util/message.h"
#include "/home/uttaran/cbmc/src/ansi-c/ansi_c_language.h"
#include "/home/uttaran/cbmc/src/goto-programs/show_goto_functions.h"
#include "/home/uttaran/cbmc/src/util/cmdline.h"
#include "/home/uttaran/cbmc/src/goto-cc/compile.h"
#include "/home/uttaran/cbmc/src/util/c_types.h"
#include "/home/uttaran/cbmc/src/ansi-c/c_typecast.h"

using namespace std;
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
	lower_bound = INT_MIN;
	upper_bound = INT_MAX;
}

int signed_interval ::get_lower_bound()
{
	return lower_bound;
}

int signed_interval ::get_upper_bound()
{
	return upper_bound;
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

unsigned_interval ::unsigned_interval()
{
	lower_bound = 0;
	upper_bound = UINT_MAX;
}

unsigned int unsigned_interval ::get_lower_bound()
{
	return lower_bound;
}

unsigned int unsigned_interval ::get_upper_bound()
{
	return upper_bound;
}

int main()
{

	std::string filename = "/home/uttaran/SVProject/example.o";

	stream_message_handlert message_handler(std::cout);

	goto_modelt goto_model;

	register_language(new_ansi_c_language);

	read_goto_binary(filename, goto_model, message_handler);
	namespacet ns(goto_model.symbol_table);

	std::map<irep_idt, signed_interval *> signed_interval_map;
	std::map<irep_idt, unsigned_interval *> unsigned_interval_map;

	Forall_goto_functions(f_it, goto_model.goto_functions)
	{
		Forall_goto_program_instructions(it, f_it->second.body)
		{
			goto_programt::instructiont instruction = *it;
			std::map<char, int>::iterator map_it;
			if (it->type == goto_program_instruction_typet::DECL)
			{
				code_declt decl = to_code_decl(it->code);
				symbol_exprt declared_symbol = to_symbol_expr(decl.symbol());
				const symbolt *symbol = goto_model.symbol_table.lookup(declared_symbol.get_identifier());
				symbol->show(std::cout);
				//_typet ctype = get_c_type(symbol->type);
				if (symbol->type.get(ID_C_c_type) == ID_signed_int)
				{
					map_it = signed_interval_map.find();
					if (map_it != signed_interval_map.end())
					{
						signed_interval *s = new signed_interval();
						signed_interval_map.insert(pair<irep_idt, signed_interval *>( ,s));
					}
				}
				else if (symbol->type.get(ID_C_c_type) == ID_unsigned_int)
					map_it = unsigned_interval_map.find();
				if (map_it != unsigned_interval_map.end())
				{
					unsigned_interval *u = new unsigned_interval();
					unsigned_interval_map.insert(pair<irep_idt, unsigned_interval *>( ,u));
				}
			}
			if (it->type == goto_program_instruction_typet::ASSIGN)
			{
				code_declt decl = to_code_decl(it->code);
				symbol_exprt declared_symbol = to_symbol_expr(decl.symbol());
				const symbolt *symbol = goto_model.symbol_table.lookup(declared_symbol.get_identifier());
				symbol->show(std::cout);
				//_typet ctype = get_c_type(symbol->type);
				if (symbol->type.get(ID_C_c_type) == ID_signed_int)
					std::cout << "Signed ID_integer";
				else if (symbol->type.get(ID_C_c_type) == ID_unsigned_int)
					std::cout << "Unsigned Integer";
			}
		}
	}
