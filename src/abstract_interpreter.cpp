#include <iostream>
#include <map>

#include "goto-programs/goto_functions.h"
#include "goto-programs/goto_program.h"
#include "ansi-c/expr2c.h"
#include "util/mp_arith.h"
#include "util/arith_tools.h"
#include "util/simplify_expr.h"
#include "util/string2int.h"
//Include
#include "abstract_interpreter.h"
#include "interval.h"
#include "interval_utils.h"


void abstract_interpreter::run_interpreter(goto_modelt &goto_model)
{
	std::cout<<"Running Interpreter : \n";

	Forall_goto_functions(f_it, goto_model.goto_functions)
	{
		if(f_it->first == "main")
		{	
			std::cout<<"Function : "<<f_it->first<<"\n\n";
		Forall_goto_program_instructions(it, f_it->second.body)
		{
			goto_programt::instructiont instruction = *it ;

			std::cout<<"Instruction : "<<it->to_string()<<"\n";

			switch(it->type)
			{
				case goto_program_instruction_typet::DECL :  handle_declaration(*it, goto_model); break;

				case goto_program_instruction_typet::ASSIGN : handle_assignments(*it, goto_model); break ;

				case goto_program_instruction_typet::GOTO : handle_goto(*it, goto_model); break ;

				default: std::cout<<"Cannot Recognise the instruction\n";	
			}
		}
		}	
	}
}


void abstract_interpreter :: handle_declaration(goto_programt::instructiont &instruction, goto_modelt &goto_model)
{
	code_declt decl = to_code_decl(instruction.code);
	symbol_exprt symbol_expr = to_symbol_expr(decl.symbol());
	const symbolt* symbol = goto_model.symbol_table.lookup(symbol_expr.get_identifier());

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

	else if(symbol->type.id() ==  ID_unsignedbv)
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

interval abstract_interpreter :: handle_rhs(exprt& expression, goto_modelt& goto_model)
{
	if(expression.id() == ID_symbol)
	{
		symbol_exprt sym_expr = to_symbol_expr(expression);
		const symbolt* symbol = goto_model.symbol_table.lookup(sym_expr.get_identifier());
		std::map<irep_idt, interval*>::iterator it ;
		interval* object ;

		it = interval_map.find(symbol->name);

			object = it->second ;
			return *object ;

	}

	else if(expression.id() == ID_constant)
	{
		constant_exprt constant_expr = to_constant_expr(expression);
		mp_integer value;
		to_integer(constant_expr, value);
		
		std::cout<<"Constant Found : "<<value<<"\n\n";
		interval* constant = new interval(integer_type::SIGNED);

		constant->set_lower_bound(unsafe_string2int(id2string(constant_expr.get_value())));
		constant->set_upper_bound(unsafe_string2int(id2string(constant_expr.get_value())));

		return *constant ;

	}

	else
	{
		if(expression.id() == ID_plus)
		{
			std::cout<<"Plus Expression : ";
			plus_exprt plus_expr = to_plus_expr(expression);
			interval arg1 = handle_rhs(plus_expr.op0() , goto_model);
			interval arg2 = handle_rhs(plus_expr.op1(), goto_model);

			interval add_result(integer_type::SIGNED) ;
			add(arg1, arg2, &add_result);
			std::cout<<"After Adding : ";
			//add_result.print_interval() ;
			std::cout<<"\n";
			return add_result;
		}
		else if(expression.id() == ID_mult)
		{
			std::cout<<"Multiply Operator\n\n";
			interval* empty = new interval(integer_type::SIGNED);
			return *empty ;
		}
		else
		{
			std::cout<<"Couldnot Recognize Operator\n\n";
			interval* empty = new interval(integer_type::SIGNED);
			return *empty ;

		}
	}
}

void abstract_interpreter :: handle_assignments(goto_programt::instructiont &instruction, goto_modelt &goto_model)
{
	code_assignt assign = to_code_assign(instruction.code);	
	symbol_exprt symbol_expr = to_symbol_expr(assign.lhs());	
	const symbolt* symbol = goto_model.symbol_table.lookup(symbol_expr.get_identifier());

	std::map<irep_idt, interval*>::iterator it ;
	it = interval_map.find(symbol->name);

	if(it!=interval_map.end())
	{
		exprt expression = assign.rhs();
		namespacet ns(goto_model.symbol_table);

		std::cout<<"RHS Expression  : "<<expr2c(expression, ns)<<"\n"; 
		exprt simplified = simplify_expr(expression, ns);

		//CHECK FOR CONSTANT PROPAGATION?? //MAYBE NOT //CONFIRM ONCE

		std::cout<<"Simplified Expression : "<<expr2c(simplified,ns)<<"\n";
		handle_rhs(expression, goto_model);
	}

}

void create_complementary_expr(exprt &expr, exprt &comp_expr, goto_modelt &goto_model)
{
	not_exprt expr_not = to_not_expr(expr);
	expr_not.make_not();
	namespacet ns(goto_model.symbol_table);
	comp_expr = simplify_expr(expr_not, ns);
	std::cout<<"Before not simplification : "<<expr2c(expr_not,ns)<<"\n\n";
	std::cout<<"After not simplification : "<<expr2c(comp_expr,ns)<<"\n\n";
}


void abstract_interpreter :: handle_goto(goto_programt::instructiont &instruction, goto_modelt &goto_model)
{
	if(!instruction.is_backwards_goto())
	{

		exprt expr(instruction.guard);
		exprt comp_expr;
		std::cout<<"EXPRESSION NIL? :"<<expr.has_operands()<<"\n";

	if(expr.has_operands())
	{		
		bool take_branch ;

		std::cout<<"Branch Encountered. Press 1 to take if branch and 0 for else branch : ";
		std::cin>>take_branch;
		std::cout<<"TAKE BRANCH? : "<<take_branch<<"\n";

		if(!take_branch)
		{
			create_complementary_expr(expr, comp_expr, goto_model);
		}

		if(can_cast_expr<binary_relation_exprt>(expr))
		{
			binary_relation_exprt binary_relation_expr = to_binary_relation_expr(expr);

			if(expr.id() == ID_equal)
			{
				//take_branch =  
			}
			else if(expr.id() == ID_notequal)
			{
				//take_branch = 
			}
			else if(expr.id() == ID_ge || expr.id() == ID_gt)
			{

			}
			else if(expr.id() == ID_le || expr.id() == ID_lt)
			{

			}

			else
				std::cout<<"Unidentified Binary Relation Operator\n\n";


			if(take_branch)
			{

			}
		}
	}

	}

}