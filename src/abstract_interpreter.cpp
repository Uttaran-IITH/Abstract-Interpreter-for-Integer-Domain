#include <iostream>
#include <map>

#include "goto-programs/goto_functions.h"
#include "goto-programs/goto_program.h"
#include "ansi-c/expr2c.h"
#include "util/mp_arith.h"
#include "util/arith_tools.h"
#include "util/simplify_expr.h"
#include "analyses/natural_loops.h"
#include "util/string2int.h"
//Include
#include "abstract_interpreter.h"
#include "interval.h"
#include "interval_utils.h"


void abstract_interpreter::run_interpreter(goto_modelt &goto_model)
{
	std::cout<<"Running Interpreter : \n";
	namespacet ns(goto_model.symbol_table);


	natural_loops_mutablet loops ;

	Forall_goto_functions(f_it, goto_model.goto_functions)
	{
		if(f_it->first == "main")
		{	
			loops(f_it->second.body);

			std::cout<<"Function : "<<f_it->first<<"\n\n";

	  	for(goto_programt::instructionst::iterator \
	      	it=(f_it->second).body.instructions.begin(); \
	      	it!=(f_it->second).body.instructions.end(); )
		{
			goto_programt::instructiont instruction = *it ;

			std::cout<<"Instruction : "<<as_string(ns, *it)<<"\n";
			bool target_changed = false;

			switch(it->type)
			{
				case goto_program_instruction_typet::DECL :  handle_declaration(*it, goto_model); break;

				case goto_program_instruction_typet::ASSIGN : handle_assignments(*it, goto_model); break ;

				case goto_program_instruction_typet::GOTO : if(!check_if_loop(loops, it)) 
																handle_goto(*it, goto_model, it, target_changed);
															else
															{
																handle_loops(loops.loop_map.find(it)->second, loops, goto_model, ns);
																it = instruction.get_target();
																target_changed = true ;   
															}
															break ;	
				default: std::cout<<"Cannot Recognise the instruction\n";	
			}

			print_all();

			if(!target_changed)
				it++;
		}
		}	
	}
}

bool abstract_interpreter :: check_if_loop(natural_loops_mutablet &loops, 
											goto_programt::targett &target)
{
	for(natural_loops_mutablet::loop_mapt::iterator l_it = loops.loop_map.begin() ; l_it != loops.loop_map.end() ; l_it++)
	{
		if(l_it->first == target)
			return true ;
	}

	return false; 
}


void abstract_interpreter :: print_all()
{
	std::map<irep_idt, interval*>::iterator it ;
	std::cout<<"---------------- INTERVALS -------------------------\n";

	for(it = interval_map.begin() ; it!=interval_map.end() ; it++)
	{
		std::cout<<id2string(it->first)<<" : ";
		it->second->print_interval();
		std::cout<<"\n";
	}

	std::cout<<"*************************************************\n\n";
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

		// std::cout<<" LOWER  : "<<integer2string(binary2integer(id2string(constant_expr.get_value()), false))<<"\n";

		constant->set_lower_bound(value, false);
		constant->set_upper_bound(value, false);
		std::cout<<"Constant Interval : " ;
		constant->print_interval();
		std::cout<<"\n";
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

			integer_type type = integer_type::SIGNED ; 
			if(arg1.get_sign()  == integer_type::UNSIGNED &&
				arg2.get_sign() == integer_type::SIGNED)
			{
				type = integer_type::UNSIGNED ;
			}

			interval add_result(type) ;

			add(arg1, arg2, &add_result);

			std::cout<<"After Adding : ";
			add_result.print_interval() ;
			std::cout<<"\n";
			return add_result;
		}

		//FOR SUBTRACT CASE : Check on how to assign signed and unsigned
		else if(expression.id() == ID_minus)
		{
			std::cout<<"Subtract Expression : ";
			minus_exprt sub_expr =  to_minus_expr(expression);

			interval arg1 = handle_rhs(sub_expr.op0() , goto_model);
			interval arg2 = handle_rhs(sub_expr.op1(), goto_model);

			integer_type type = integer_type::SIGNED ;
			if(arg1.get_sign()  == integer_type::UNSIGNED &&
				arg2.get_sign() == integer_type::SIGNED)
			{
				type = integer_type::UNSIGNED ;
			}

			interval sub_result(type);

			sub(arg1, arg2, &sub_result);

			std::cout<<"After Subtraction : ";
			sub_result.print_interval() ;
			std::cout<<"\n";

			return sub_result ;			

		}

		else if(expression.id() == ID_mult)
		{
			std::cout<<"Multiply Expression : ";
			mult_exprt mult_expr =  to_mult_expr(expression);

			interval arg1 = handle_rhs(mult_expr.op0() , goto_model);
			interval arg2 = handle_rhs(mult_expr.op1(), goto_model);

			integer_type type = integer_type::SIGNED ;
			if(arg1.get_sign()  == integer_type::UNSIGNED &&
				arg2.get_sign() == integer_type::SIGNED)
			{
				type = integer_type::UNSIGNED ;
			}

			interval mult_result(type);

			multiply(arg1, arg2, &mult_result);

			std::cout<<"After Multiply : ";
			mult_result.print_interval() ;
			std::cout<<"\n";

			return mult_result ;
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
		interval temp = handle_rhs(expression, goto_model);
		it->second->make_equal(temp);
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

void abstract_interpreter :: set_lhs(symbol_exprt &lhs_sym, interval* &lhs, goto_modelt &goto_model)
{
	const symbolt* symbol = goto_model.symbol_table.lookup(lhs_sym.get_identifier());
	std::map<irep_idt, interval*>::iterator it;

	it = interval_map.find(symbol->name);

	if(it!=interval_map.end())
	{
		lhs = it->second ;
		std::cout<<"LHS Set : ";
		lhs->print_interval();
		std::cout<<"\n";
	}
}

void abstract_interpreter :: set_rhs(exprt &rhs_expr, interval* &rhs , goto_modelt &goto_model)
{

	if(rhs_expr.id() == ID_constant)
	{
		constant_exprt constant_expr = to_constant_expr(rhs_expr);
		mp_integer value;
		to_integer(constant_expr, value);

		rhs = new interval(integer_type::SIGNED) ;
		rhs->set_lower_bound(value, false);
		rhs->set_upper_bound(value, false);

		std::cout<<"RHS Set : ";
		rhs->print_interval();
	}

	else
	{
		symbol_exprt symbol_expr = to_symbol_expr(rhs_expr);
		const symbolt* symbol = goto_model.symbol_table.lookup(symbol_expr.get_identifier());
		std::map<irep_idt, interval*>::iterator it;

		it = interval_map.find(symbol->name);

		if(it!=interval_map.end())
		{
			rhs = it->second ;
			std::cout<<"RHS Set : ";
			rhs->print_interval();
			std::cout<<"\n";
		}
	}	

}
void abstract_interpreter :: handle_goto(goto_programt::instructiont &instruction, goto_modelt &goto_model,
										goto_programt::targett &it, bool &target_changed)
{
	namespacet ns(goto_model.symbol_table);

	if(!instruction.is_backwards_goto())
	{
		exprt expr(instruction.guard);
		exprt comp_expr;
		std::cout<<"EXPRESSION NIL? :"<<expr.has_operands()<<"\n";

	if(expr.has_operands())
	{		
		bool take_branch ;
		exprt expr_true = simplify_expr(expr, ns);

		expr.make_not();
		exprt expr_false =  expr;

		std::cout<<"Branch Condition : "<<expr2c(expr_true, ns)<<"\n";

		std::cout<<"Press 1 to take if branch and 0 for else branch : ";
		std::cin>>take_branch;
		std::cout<<"TAKE BRANCH? : "<<take_branch<<"\n";

		bool can_take_branch;
		exprt check_expr ;

		if(!take_branch)

			check_expr = expr_false ;
		else
			check_expr = expr_true ;

		std::cout<<"CHECKING RELATION : "<<expr2c(simplify_expr(check_expr, ns), ns)<<"\n\n";


		if(can_cast_expr<binary_relation_exprt>(check_expr))
		{
			binary_relation_exprt binary_relation_expr = to_binary_relation_expr(check_expr);

			symbol_exprt lhs_sym = to_symbol_expr(binary_relation_expr.lhs());
			exprt rhs_expr = binary_relation_expr.rhs();

			interval* rhs ;
			interval* lhs ;

			//Set LHS
			set_lhs(lhs_sym, lhs, goto_model);
			set_rhs(rhs_expr, rhs, goto_model);


			if(expr.id() == ID_equal)
			{
				interval* temp ;
				if(lhs->get_sign() == integer_type::UNSIGNED 
					|| rhs->get_sign() ==integer_type::UNSIGNED)
				{
					temp = new interval(integer_type::UNSIGNED);
				}
				else
					temp = new interval(integer_type::SIGNED);

				can_take_branch = equals(lhs, rhs, temp);

				if(can_take_branch)
				{
					lhs->make_equal(*temp);
					rhs->make_equal(*temp);
				}
			}
			// else if(expr.id() == ID_notequal)
			// {
			// 	//take_branch = 
			// }
			else if(expr.id() == ID_ge || expr.id() == ID_gt)
			{

			}
			else if(expr.id() == ID_le || expr.id() == ID_lt)
			{

			}

			else
				std::cout<<"Unidentified Binary Relation Operator\n\n";

			can_take_branch = true ;

			if(take_branch && can_take_branch)
			{
				std::cout<<"Setting Target : \n";
				target_changed = true ;
				it = instruction.get_target();

				//std::cout<<"Set target as : "<<as_string(ns, *it)<<"\n\n";
			}
		}
	}

	else
	{
		std::cout<<"Target Changed\n";
		target_changed = true ;
		it = instruction.get_target();
	}

	}
}

bool abstract_interpreter :: check_condition(exprt &expr, goto_modelt &goto_model, namespacet &ns)
{
	bool can_enter_loop = false ;

	if(expr.has_operands())
	{		
		if(can_cast_expr<binary_relation_exprt>(expr))
		{
			binary_relation_exprt binary_relation_expr = to_binary_relation_expr(expr);

			symbol_exprt lhs_sym = to_symbol_expr(binary_relation_expr.lhs());
			exprt rhs_expr = binary_relation_expr.rhs();

			interval* rhs ;
			interval* lhs ;

			//Set LHS
			std::cout<<"Before\n";
			set_lhs(lhs_sym, lhs, goto_model);
	
			//Set RHS
			set_rhs(rhs_expr, rhs, goto_model);
			std::cout<<"After\n";

			if(expr.id() == ID_equal)
			{
				interval* temp ;
				if(lhs->get_sign() == integer_type::UNSIGNED 
					|| rhs->get_sign() ==integer_type::UNSIGNED)
				{
					temp = new interval(integer_type::UNSIGNED);
				}
				else
					temp = new interval(integer_type::SIGNED);

				can_enter_loop = equals(lhs, rhs, temp);

			}

			else if(expr.id() == ID_notequal)
			{
				can_enter_loop = not_equals(lhs, rhs);
			}

			else if(expr.id() == ID_ge || expr.id() == ID_gt)
			{
				interval* temp_a;
				interval* temp_b;
				temp_a = new interval(integer_type::SIGNED);
				temp_b = new interval(integer_type :: UNSIGNED);
				std::cout<<"Before Less than : "<<can_enter_loop<<"\n\n";
				can_enter_loop = less_than(lhs,rhs,temp_a,temp_b);
				std::cout<<"After : "<<can_enter_loop<<"\n\n";
			}

			else if(expr.id() == ID_le || expr.id() == ID_lt)
			{
				interval* temp_a;
				interval* temp_b;
				temp_a = new interval(integer_type::SIGNED);
				temp_b = new interval(integer_type :: UNSIGNED);

				can_enter_loop = greater_than(lhs,rhs,temp_a,temp_b);
			}

			else
				std::cout<<"Unidentified Binary Relation Operator\n\n";
	}	


}


	return can_enter_loop ;	
}

void abstract_interpreter :: handle_loops (natural_loops_mutablet::natural_loopt &current_loop, natural_loops_mutablet &all_loops,
										  goto_modelt &goto_model, namespacet &ns)
{
	std::cout<<"****** Loop Found ******\n";

	natural_loops_mutablet::natural_loopt::iterator l_it = current_loop.begin() ;
	bool enter_loop = false ;

	goto_programt::instructiont &instruction = **l_it;
	exprt simplified = simplify_expr(instruction.guard, ns);

	if(check_condition(simplified, goto_model, ns))
	{
		std::cout<<"Enter the loop ? (1 = yes , 0 = no) :  ";
		std::cin>>enter_loop ;		
	}
	//l_it->check_loop_condition(*target, goto_model);


	while(check_condition(simplified, goto_model, ns) && enter_loop)
	{
		bool target_changed = false ;
		std::cout<<"Entered LOOOOP\n";
		++l_it ;

		while(l_it != current_loop.end())
		{

			std::cout<<"\nComes in here\n" ;
			goto_programt::targett target = *(l_it) ;
			std::cout<<"Instruction : "<<as_string(ns, *target)<<"\n";

			switch(target->type)
			{
				case goto_program_instruction_typet::DECL :  handle_declaration(*target, goto_model); break;

				case goto_program_instruction_typet::ASSIGN : std::cout<<"Assignment\n\n" ;handle_assignments(*target, goto_model); break ;

				case goto_program_instruction_typet::GOTO : if(!check_if_loop(all_loops, target) && !target->is_backwards_goto()) 
																handle_goto(*target, goto_model, target, target_changed);
															else if(check_if_loop(all_loops, target))
															{
																handle_loops(all_loops.loop_map.find(*l_it)->second, all_loops, goto_model, ns);
																l_it = current_loop.find(target->get_target());	
																target_changed = true ;		   
															}
															
															else if(target->is_backwards_goto())
															{
																target_changed = true ;
																 l_it = current_loop.end();
															}break ;	

				default: std::cout<<"Cannot Recognise the instruction\n";	
			}

		print_all();

		if(!target_changed)
			{
				l_it++;
				//std::cout<<"Instruction : "<<as_string(ns, **l_it)<<"\n";

			}
		}

		l_it = current_loop.begin(); 
		std::cout<<"One Iteration Done\n";
	}	

	// if(!check_condition(simplified, goto_model, ns))
	// {

	// }
}