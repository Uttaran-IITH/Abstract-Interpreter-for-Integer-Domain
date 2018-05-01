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

#include "abstract_interpreter.h"
#include "interval.h"
#include "interval_utils.h"

bool maybe;

enum assert_status{NO_ASSERTION, ASSERTION_PASSED, ASSERTION_FAILED};

assert_status status = NO_ASSERTION ;

// Main Interpreter Function
void abstract_interpreter::run_interpreter(goto_modelt &goto_model)
{
	std::cout<<"Running Interpreter : \n";
	namespacet ns(goto_model.symbol_table);

	threshold = -1 ;

	natural_loops_mutablet loops ;

	Forall_goto_functions(f_it, goto_model.goto_functions)
	{
		//Run only for the 'main' function 
		if(f_it->first == "main")
		{	
			loops(f_it->second.body);

			//std::cout<<"Function : "<<f_it->first<<"\n\n";

	  	for(goto_programt::instructionst::iterator \
	      	it=(f_it->second).body.instructions.begin(); \
	      	it!=(f_it->second).body.instructions.end(); )
		{
			goto_programt::instructiont instruction = *it ;

			std::cout<<"Instruction : "<<as_string(ns, *it)<<"\n";
			getchar();
			bool target_changed = false;

			switch(it->type)
			{
				case goto_program_instruction_typet::DECL :  handle_declaration(*it, goto_model); break;

				case goto_program_instruction_typet::ASSIGN : handle_assignments(*it, goto_model); break ;

				case goto_program_instruction_typet::GOTO : if(!check_if_loop(loops, it) && status == NO_ASSERTION) 
																handle_goto(*it, goto_model, it, target_changed, loops, ns);

															else if(!check_if_loop(loops, it) && status != NO_ASSERTION)
															{
																if(status == ASSERTION_FAILED)
																{
																	it = instruction.get_target();
																	target_changed = true ;
																}
																status = NO_ASSERTION ;

															}
															else
															{
																//std::cout<<"\n\n LOOP FOUND \n\n";
																handle_loops(loops.loop_map.find(it)->second, loops, goto_model, ns);
																it = instruction.get_target();
																target_changed = true ;   
															}
															break ;	
				case goto_program_instruction_typet::ASSERT : handle_assertions(*it, goto_model, ns); target_changed = false; break;											
				
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


// Handling Declarations
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
			//std::cout<<"Double Declaration O.O";
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
			//std::cout<<"Double Declaration O.O";
		}

		else
		{
			interval_map.insert(std::pair<irep_idt, interval*>(symbol->name, new interval(integer_type::UNSIGNED)));
		}
	}
}


interval abstract_interpreter :: handle_rhs(exprt& expression, goto_modelt& goto_model)
{
	bool neg = false ;
	if(can_cast_expr<unary_minus_exprt>(expression))
	{
		neg = true ;
		exprt curr = expression.op0();
		expression = curr ;
	}

	if(expression.id() == ID_symbol)
	{
		symbol_exprt sym_expr = to_symbol_expr(expression);
		const symbolt* symbol = goto_model.symbol_table.lookup(sym_expr.get_identifier());
		std::map<irep_idt, interval*>::iterator it ;
		interval* object ;

		it = interval_map.find(symbol->name);

			object = it->second ;
		if(neg)
		{	
			negate(object);
		}
			return *object ;

	}

	else if(expression.id() == ID_constant)
	{
		constant_exprt constant_expr = to_constant_expr(expression);
		mp_integer value;
		to_integer(constant_expr, value);
		
		//std::cout<<"Constant Found : "<<value<<"\n\n";
		interval* constant = new interval(integer_type::SIGNED);

		// std::cout<<" LOWER  : "<<integer2string(binary2integer(id2string(constant_expr.get_value()), false))<<"\n";

		constant->set_lower_bound(value, false);
		constant->set_upper_bound(value, false);

		//std::cout<<"Constant Interval : " ;
		//constant->print_interval();
		//std::cout<<"\n";

		if(neg)
		{	
			negate(constant);
		}

		return *constant ;

	}
	else
	{
		if(expression.id() == ID_plus)
		{
			//std::cout<<"Plus Expression : ";
			plus_exprt plus_expr = to_plus_expr(expression);
			interval arg1 = handle_rhs(plus_expr.op0() , goto_model);
			interval arg2 = handle_rhs(plus_expr.op1(), goto_model);

			integer_type type = integer_type::SIGNED ; 
			if(arg1.get_sign()  == integer_type::UNSIGNED &&
				arg2.get_sign() == integer_type::UNSIGNED)
			{
				type = integer_type::UNSIGNED ;
			}

			interval add_result(type) ;

			add(arg1, arg2, &add_result);

			//std::cout<<"After Adding : ";
			//add_result.print_interval() ;
			//std::cout<<"\n";

			if(neg)
			{	
				negate(&add_result);
			}
			return add_result;
		}

		//FOR SUBTRACT CASE : Check on how to assign signed and unsigned
		else if(expression.id() == ID_minus)
		{
			//std::cout<<"Subtract Expression : ";
			minus_exprt sub_expr =  to_minus_expr(expression);

			interval arg1 = handle_rhs(sub_expr.op0() , goto_model);
			interval arg2 = handle_rhs(sub_expr.op1(), goto_model);

			integer_type type = integer_type::SIGNED ;
			if(arg1.get_sign()  == integer_type::UNSIGNED &&
				arg2.get_sign() == integer_type::UNSIGNED)
			{
				type = integer_type::UNSIGNED ;
			}

			interval sub_result(type);

			sub(arg1, arg2, &sub_result);

			//std::cout<<"After Subtraction : ";
			//sub_result.print_interval() ;
			//std::cout<<"\n";

			if(neg)
			{	
				negate(&sub_result);
			}

			return sub_result ;			

		}

		else if(expression.id() == ID_mult)
		{
			//std::cout<<"Multiply Expression : ";
			mult_exprt mult_expr =  to_mult_expr(expression);

			interval arg1 = handle_rhs(mult_expr.op0() , goto_model);
			interval arg2 = handle_rhs(mult_expr.op1(), goto_model);

			integer_type type = integer_type::SIGNED ;
			if(arg1.get_sign()  == integer_type::UNSIGNED &&
				arg2.get_sign() == integer_type::UNSIGNED)
			{
				type = integer_type::UNSIGNED ;
			}

			interval mult_result(type);

			multiply(arg1, arg2, &mult_result);

			//std::cout<<"After Multiply : ";
			//mult_result.print_interval() ;
			//std::cout<<"\n";

			if(neg)
			{	
				negate(&mult_result);
			}
			return mult_result ;
		}

		else if(expression.id() == ID_div)
		{
			//std::cout<<"Divide Expression : ";
			div_exprt div_expr = to_div_expr(expression);

			interval arg1 = handle_rhs(div_expr.op0(), goto_model);
			interval arg2 = handle_rhs(div_expr.op1(), goto_model);

			integer_type type = integer_type::SIGNED ;

			if(arg1.get_sign()  == integer_type::UNSIGNED &&
				arg2.get_sign() == integer_type::UNSIGNED)
			{
				type = integer_type::UNSIGNED ;
			}

			interval div_result(type);

			divide(arg1, arg2, &div_result);

			//std::cout<<"After Divide : ";
			//div_result.print_interval() ;
			//std::cout<<"\n";

			if(neg)
			{	
				negate(&div_result);
			}
			return div_result ;
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

		//std::cout<<"RHS Expression  : "<<expr2c(expression, ns)<<"\n"; 
		exprt simplified = simplify_expr(expression, ns);

		//CHECK FOR CONSTANT PROPAGATION?? //MAYBE NOT //CONFIRM ONCE

		//std::cout<<"Simplified Expression : "<<expr2c(simplified,ns)<<"\n";
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
	//std::cout<<"Before not simplification : "<<expr2c(expr_not,ns)<<"\n\n";
	//std::cout<<"After not simplification : "<<expr2c(comp_expr,ns)<<"\n\n";
}

void abstract_interpreter :: set_lhs(symbol_exprt &lhs_sym, interval* &lhs, goto_modelt &goto_model)
{
	const symbolt* symbol = goto_model.symbol_table.lookup(lhs_sym.get_identifier());
	std::map<irep_idt, interval*>::iterator it;

	it = interval_map.find(symbol->name);

	if(it!=interval_map.end())
	{
		lhs = it->second ;
		// std::cout<<"LHS Set : ";
		// lhs->print_interval();
		// std::cout<<"\n";
	}
}

void abstract_interpreter :: set_rhs(exprt &rhs_expr, interval* &rhs , goto_modelt &goto_model)
{
	//std::cout<<"\n\n SETTING RHS \n\n";

	if(rhs_expr.id() == ID_constant)
	{
		//std::cout<<"Before integer\n";
		constant_exprt constant_expr = to_constant_expr(rhs_expr);
		mp_integer value;
		//std::cout<<"Before integer\n";
		to_integer(constant_expr, value);
		//std::cout<<"After integer : "<<value<<"\n\n" ;
		rhs = new interval(integer_type::SIGNED) ;
		rhs->set_lower_bound(value, false);
		rhs->set_upper_bound(value, false);

		// std::cout<<"RHS Set : ";
		// rhs->print_interval();
	}

	else
	{
		//std::cout<<"\nComing in ELse\n";

		symbol_exprt symbol_expr = to_symbol_expr(rhs_expr);
		const symbolt* symbol = goto_model.symbol_table.lookup(symbol_expr.get_identifier());
		std::map<irep_idt, interval*>::iterator it;

		it = interval_map.find(symbol->name);

		if(it!=interval_map.end())
		{
			rhs = it->second ;
			// std::cout<<"RHS Set : ";
			// rhs->print_interval();
			// std::cout<<"\n";
		}
	}	

}

void abstract_interpreter :: remove_not(exprt &expr, namespacet &ns)
{	
	if(can_cast_expr<not_exprt>)
	{
		exprt inside_expr = expr.op0();

		if(inside_expr.id() == ID_equal)
		{
			//exprt new_expr(inside_expr.op0(), inside_expr.op1());
			exprt new_expr(ID_notequal);
			new_expr.operands().push_back(inside_expr.op0());
			new_expr.operands().push_back(inside_expr.op1());

			//std::cout<<"NEW_EXPR : "<<expr2c(simplify_expr(new_expr, ns), ns)<<"\n\n";
			expr = new_expr ;
		}
		else if(inside_expr.id() == ID_ge)
		{
			//binary_exprt new_expr(inside_expr.op0(), ID_lt, inside_expr.op1());
			exprt new_expr(ID_lt);
			new_expr.operands().push_back(inside_expr.op0());
			new_expr.operands().push_back(inside_expr.op1());

			expr = new_expr ;
		}
		else if(inside_expr.id() == ID_le)
		{
			//binary_exprt new_expr(inside_expr.op0(), ID_gt , inside_expr.op1());
			exprt new_expr(ID_gt);
			new_expr.operands().push_back(inside_expr.op0());
			new_expr.operands().push_back(inside_expr.op1());
			expr = new_expr ;
		}
	}
}

void abstract_interpreter :: iterate_over_if(goto_programt::targett &it, goto_modelt &goto_model, 
						   goto_programt::targett &target_if, goto_programt::targett &target_end, bool &else_present, natural_loops_mutablet &loops, namespacet &ns)
{
	bool if_end = false ;

	while(!if_end && it != target_if)
	{
		goto_programt::instructiont instruction = *it ;
		//std::cout<<"STAYS HERE\n\n";

		std::cout<<"Instruction : "<<as_string(ns, *it)<<"\n";
		//std::cout<<"Target If  : "<<as_string(ns, *target_if)<<"\n";
		bool target_changed = false;

		switch(it->type )
		{
			case goto_program_instruction_typet::DECL :  handle_declaration(*it, goto_model); break;

			case goto_program_instruction_typet::ASSIGN : handle_assignments(*it, goto_model); target_changed = false ; break ;

			case goto_program_instruction_typet::GOTO : if(!check_if_loop(loops, it) && status == NO_ASSERTION && instruction.guard.has_operands()) 
														{
															handle_goto(*it, goto_model, it, target_changed, loops,ns);
															target_changed = true ;
														}

														else if(!check_if_loop(loops, it) && status == NO_ASSERTION && !instruction.guard.has_operands()) 
														{

															target_end = it->get_target();
															else_present = true ;
															if_end = true ;
															target_changed = true ;
														}

														else if(!check_if_loop(loops, it) && status != NO_ASSERTION)
														{
															if(status == ASSERTION_FAILED)
															{
																it = instruction.get_target();
																target_changed = true ;
															}
															status = NO_ASSERTION ;

														}
														else //if(check_if_loop(loops, it))
														{
															//std::cout<<"\n HANDLE LOOPS FOUND in iterate_over_if \n";
															handle_loops(loops.loop_map.find(it)->second, loops, goto_model, ns);
															it = instruction.get_target();
															target_changed = true ;   
														}
														break ;	
			case goto_program_instruction_typet::ASSERT : handle_assertions(*it, goto_model, ns); target_changed = false; break;											
			
			default: std::cout<<"Cannot Recognise the instruction\n";	
		}

		if(!target_changed)
			{
				it++ ;
				//std::cout<<"Incremented it\n\n";
			}

		print_all();
		getchar();
	}

	if(it == target_if)
		else_present = false ;
}

void abstract_interpreter :: check_if_else_present(goto_programt::targett &it, goto_programt::targett &target_end, goto_programt::targett &target_if, bool &else_present)
{
	bool if_end = false ;
	int nested_goto_found = 0  ;

	++it ;

	while(it!= target_if && !if_end)
	{
		if(it->type == goto_program_instruction_typet::GOTO )
		{
			if(!(*it).guard.has_operands() && nested_goto_found ==0)
			{
				if_end = true ;
				else_present = true ;

				//ADDITIONS *********************************************
				target_end = it->get_target();
			}
			else if(!(*it).guard.has_operands() && nested_goto_found > 0)
			{
				nested_goto_found -- ;
			}
			else if((*it).guard.has_operands())
			{
				//ADDITIONS **************************************************
				bool temp = false;
				goto_programt::targett t = it->get_target();
				//goto_programt::targett new_it = it ;

				//std::cout<<"\n COMES BEFORE THE RECURSIVE CALL \n";
				it++ ;
				check_if_else_present(it, target_end, t, temp);

				if(temp)	
				{
					//std::cout<<"FOUND  a NEESTED IF ELSE \n";
				 	nested_goto_found ++ ;
				}
			}
		}

		it++ ;
	}

	if(it == target_if)
		else_present = false;
}

void abstract_interpreter :: iterate_over_else(goto_programt::targett &it, goto_modelt &goto_model, 
						   goto_programt::targett &target_end, natural_loops_mutablet &loops , namespacet &ns)
{
	while(it!= target_end)
	{

		std::cout<<"Instruction : "<<as_string(ns, *it)<<"\n";
		bool target_changed = false;

		switch(it->type )
		{
			case goto_program_instruction_typet::DECL :  handle_declaration(*it, goto_model); break;

			case goto_program_instruction_typet::ASSIGN : handle_assignments(*it, goto_model); target_changed = false ; break ;

			case goto_program_instruction_typet::GOTO : if(!check_if_loop(loops, it) && status == NO_ASSERTION && (*it).guard.has_operands()) 
														{	
															handle_goto(*it, goto_model, it, target_changed, loops,ns); 
															target_changed = true ;
														}

														// if(!check_if_loop(loops, it) && status == NO_ASSERTION && !instruction.guard.has_operands()) 
														// {
														// 	if(target_if == it)
														// 		else_present = false ;

														// 	else
														// 	{
														// 		target_end = it->get_target();
														// 		else_present = true ;
														// 	}

														// 	if_end = true ;

														// }

														else if(!check_if_loop(loops, it) && status != NO_ASSERTION)
														{
															if(status == ASSERTION_FAILED)
															{
																it = (*it).get_target();
																target_changed = true ;
															}
															status = NO_ASSERTION ;

														}
														else
														{
															//std::cout<<"\n\n LOOP FOUND HERE  in iterate over else\n\n";
															handle_loops(loops.loop_map.find(it)->second, loops, goto_model, ns);
															it = (*it).get_target();
															target_changed = true ;   
														}
														break ;	
			case goto_program_instruction_typet::ASSERT : handle_assertions(*it, goto_model, ns); target_changed = false; break;											
			
			default: std::cout<<"Cannot Recognise the instruction\n";	
		}

		if(!target_changed)
			it++ ;

		print_all();
		getchar() ;
	}		
}

void abstract_interpreter ::restore_map(std::map<irep_idt, interval*> &copy)
{
	std::map<irep_idt, interval*>::iterator it = copy.begin();
	interval* copied_interval;

	// std::cout<<"\n MAP HERE : \n";
	// print_all();

	interval_map.clear();

	while(it!=copy.end())
	{
		if(it->second->get_sign() == integer_type::SIGNED)
		{
			copied_interval = new interval(integer_type::SIGNED);
		}
		else
		{
			copied_interval = new interval(integer_type::UNSIGNED);
		}

		if(it->second->is_minus_inf())
			copied_interval->set_lower_bound(-1, true);
		else
			copied_interval->set_lower_bound(it->second->get_lower_bound(), false);

		if(it->second->is_plus_inf())
			copied_interval->set_upper_bound(-1, true);
		else
			copied_interval->set_upper_bound(it->second->get_upper_bound(), false);

		interval_map.insert(std::pair<irep_idt, interval*>(it->first, copied_interval));

		it++ ;
	}

	// std::cout<<"\nRestored MAP : \n";
	// print_all();	
}

void abstract_interpreter :: handle_goto(goto_programt::instructiont &instruction, goto_modelt &goto_model,
										goto_programt::targett &it, bool &target_changed, natural_loops_mutablet &loops, namespacet &ns)
{
	//namespacet ns(goto_model.symbol_table);

	//Target of the initial GOTO ; Points towards the else block if present 
	goto_programt::targett target_if ;

	//In case else present, target to the end of if..else block
	goto_programt::targett target_end ;


	if(!instruction.is_backwards_goto())
	{
		exprt expr(instruction.guard);
		exprt comp_expr;
		//std::cout<<"EXPRESSION NIL? :"<<expr.has_operands()<<"\n";

	//A GOTO With a GUARD
	if(expr.has_operands())
	{		
		//bool take_branch ;
		exprt expr_true = simplify_expr(expr,ns) ;
		remove_not(expr_true, ns);

		expr.make_not();
		exprt expr_false =  expr;

		// std::cout<<"\n EXPRESSION TRUE : "<<expr2c(expr_true, ns)<<"\n";
		// std::cout<<"\n EXPRESSION FALSE : "<<expr2c(expr_false, ns)<<"\n";
		// std::cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";

		//bool can_take_branch = false ;

		target_if = instruction.get_target();

		//std::cout<<"Target Instruction : "<<as_string(ns, *target_if)<<"\n";
		//getchar();

		//BOOL VARIABLES
		bool else_present = true ;
		bool if_implemented = false;
		bool else_implemented = false ;

		//Before every if..else block
		maybe = false ;

		//Map Before the if..else block
		std::map<irep_idt, interval*> original_map;
		//Map after the if block
		std::map<irep_idt, interval*> after_if_map;

		copy_map(original_map);
		
		//If branch in actual code = expr_false here
		if(check_condition(expr_false, goto_model, ns, true))
		{
			//std::cout<<"\n in IF \n\n";
			//getchar();
			std::cout<<"INSIDE IF : \n";
			print_all();
			if_implemented = true ;

			
			iterate_over_if(++it, goto_model, target_if, target_end, else_present, loops, ns) ;

		}
		else
		{
			//std::cout<<"\n IF not implemented \n\n";
			//getchar();

			//std::cout<<"CHECKS WHETHER ELSE PRESENT OR NOT \n";
			check_if_else_present(it, target_end, target_if, else_present);
		}

		//std::cout<<"Else Present : "<<else_present<<"\n";

		if(else_present == true && maybe && if_implemented)
		{
			//std::cout<<"COMING IN HERE ???????????\n\n";
			//getchar();

			it = target_if ;
			//std::cout<<"After IF : \n\n";
			print_all();
			copy_map(after_if_map);
			restore_map(original_map);

			std::cout<<"INSIDE ELSE : \n\n";
			print_all();
			iterate_over_else(it, goto_model, target_end, loops, ns);
			else_implemented = true ;
		}

		else if(else_present == true && !if_implemented)
		{
			//std::cout<<"COMING IN ELSE BUT NO IF ^^^^^^^^^^^^^ \n\n";
			//getchar();
			it = target_if ;
			iterate_over_else(it, goto_model, target_end, loops, ns);
			else_implemented = true ;
		}



		if(if_implemented && else_implemented)
		{
			join_values(after_if_map);
 		}

 		if(if_implemented && !else_present && maybe)
 		{
 			//original ad after if
 			join_values(original_map) ;
 		}

 		if(else_present)
 			it = target_end ;
 		else
 			it = target_if ;

 		//std::cout<<"Starting from : "<<as_string(ns, *it)<<"\n";
 		//getchar();
 		target_changed = true ;
 		std::cout<<"AFTER IF..ELSE BLOCK : \n\n";
 		print_all() ;
	}
	}
}

bool abstract_interpreter :: check_condition(exprt &expr, goto_modelt &goto_model, namespacet &ns, bool update)
{
	bool can_enter_loop = false ;

	//std::cout<<"Condition Expression : "<<expr2c(expr, ns)<<"\n\n";

	if(expr.has_operands())
	{		
		//std::cout<<"Expresiion has operands \n\n";

		if(expr.id() == ID_and)
		{
			//std::cout<<"Comes inside AND Atleast\n\n";
			getchar();
			
			bool lhs = false ;
			bool rhs = false ;
			bool something_changed = false ;

			std::map<irep_idt, interval*> interval_map_before;
			copy_map(interval_map_before);
			
			lhs = check_condition(expr.op0(), goto_model, ns , update);
			//std::cout<<"LHS CALCULATED  : "<<lhs<<"\n";

			if(lhs)
			{
				something_changed = true ;
				//std::cout<<"Called FOR RHS\n\n";
				rhs = check_condition(expr.op1(), goto_model, ns , update);
			}

			can_enter_loop = lhs && rhs ;

			if(!can_enter_loop && something_changed && update)
			{
				restore_map(interval_map_before);
			}

			return can_enter_loop ;
		}

		if(expr.id() == ID_or)
		{
			bool lhs = false ;
			bool rhs = false ;

			lhs = check_condition(expr.op0(), goto_model,ns,update);
			
			if(!lhs)
			{
				//std::cout<<"When LHS FALSE SHOULD COME HERE\n";
				rhs = check_condition(expr.op1(), goto_model,ns, update);
			}

			can_enter_loop = lhs || rhs ;

			return can_enter_loop ;
		}

		if(can_cast_expr<binary_relation_exprt>(expr))
		{
			//std::cout<<"COmING IN HERE \n";

			binary_relation_exprt binary_relation_expr = to_binary_relation_expr(expr);

			symbol_exprt lhs_sym = to_symbol_expr(binary_relation_expr.lhs());
			exprt rhs_expr = binary_relation_expr.rhs();

			interval* rhs ;
			interval* lhs ;

			//Set LHS
			//std::cout<<"Before\n";
			set_lhs(lhs_sym, lhs, goto_model);
	
			//Set RHS
			//std::cout<<"Here\n\n";
			//std::cout<<"RHS Expression : "<<expr2c(rhs_expr, ns)<<"\n\n";
			set_rhs(rhs_expr, rhs, goto_model);
			//std::cout<<"After\n";

			if(binary_relation_expr.id() == ID_equal)
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

				if(can_enter_loop && update)
				{
					lhs->make_equal(*temp);
					rhs->make_equal(*temp);
				}

			}

			else if(binary_relation_expr.id() == ID_notequal)
			{
				can_enter_loop = not_equals(lhs, rhs);
			}

			else if(binary_relation_expr.id() == ID_ge || binary_relation_expr.id() == ID_gt)
			{
				//std::cout<<"Comes into greater_than \n ";
				interval* temp_a;
				interval* temp_b;
				temp_a = new interval(integer_type::SIGNED);
				temp_b = new interval(integer_type :: UNSIGNED);
				//std::cout<<"Before Less than : "<<can_enter_loop<<"\n\n";
				can_enter_loop = greater_than(lhs,rhs,temp_a,temp_b);

				if(can_enter_loop && update)
				{
					//std::cout<<"UPDATED GREATER THAN : ";
					temp_a->print_interval();
					lhs->make_equal(*temp_a);

					rhs->make_equal(*temp_b);					
				}

				//std::cout<<"\n*******************___________________ ************\n";
				//temp_b->print_interval();
				//std::cout<<"\n\n";
				//std::cout<<"After : "<<can_enter_loop<<"\n\n";
			}

			else if(binary_relation_expr.id() == ID_le || binary_relation_expr.id() == ID_lt)
			{
				//std::cout<<"Comes into less than\n";
				interval* temp_a;
				interval* temp_b;
				temp_a = new interval(integer_type::SIGNED);
				temp_b = new interval(integer_type :: UNSIGNED);

				can_enter_loop = less_than(lhs,rhs,temp_a,temp_b);

				if(can_enter_loop && update)
				{
					lhs->make_equal(*temp_a);
					rhs->make_equal(*temp_b);					
				}
				// lhs->make_equal(*temp_a);
				// rhs->make_equal(*temp_b);
			}

			else
				std::cout<<"Unidentified Binary Relation Operator\n\n";
		}
				// else 

}


	return can_enter_loop ;	
}

void abstract_interpreter :: handle_loops (natural_loops_mutablet::natural_loopt &current_loop, natural_loops_mutablet &all_loops,
										  goto_modelt &goto_model, namespacet &ns)
{
	std::cout<<"****** Loop Found ******\n";

	natural_loops_mutablet::natural_loopt::iterator l_it = current_loop.begin() ;
	bool enter_loop = false ;
	bool converged = false ;

	if(threshold ==  -1)
	{
		std::cout<<"Enter threshold : ";
		std::cin>>threshold ;
	}
    //std::cin.get();	
	goto_programt::instructiont &instruction = **l_it;
	//exprt simplified = simplify_expr(instruction.guard, ns);
	//exprt expr = instruction.guard ;
	exprt simplified = instruction.guard;
	simplified.make_not();

	if(check_condition(simplified, goto_model, ns, false))
	{
		std::cout<<"Enter the loop ? (1 = yes , 0 = no) :  ";
		std::cin>>enter_loop ;		
	}

	//Original Map Before Execution of a Loop		
	std::map<irep_idt, interval*> interval_map_before_loop;
	copy_map(interval_map_before_loop);

	//std::cout<<"Before Entering the Loop\n\n";

	print_all();

	int iter_number = 0 ;

	while(check_condition(simplified, goto_model, ns, true) && enter_loop && !converged && iter_number<threshold)
	{
		bool target_changed = false ;
		//std::cout<<"Entered LOOOOP\n";
		++l_it ;

		//Map of Previous Iteration
		std::map<irep_idt, interval*> interval_map_prev_iteration;
		copy_map(interval_map_prev_iteration);

		while(l_it != current_loop.end())
		{

			//std::cout<<"\nComes in here\n" ;
			goto_programt::targett target = *(l_it) ;
			std::cout<<"Instruction : "<<as_string(ns, *target)<<"\n";

			switch(target->type)
			{
				case goto_program_instruction_typet::DECL :  handle_declaration(*target, goto_model); break;

				case goto_program_instruction_typet::ASSIGN : handle_assignments(*target, goto_model); target_changed = false ; break ;

				case goto_program_instruction_typet::GOTO : if(!check_if_loop(all_loops, target) && !target->is_backwards_goto() && status == NO_ASSERTION) 
															{
																
																//std::cout<<"Before calling handle goto \n\n";
																//getchar();
																handle_goto(*target, goto_model, target, target_changed, all_loops, ns);

																// std::cout<<"Handle GOTO Returns \n\n";
																// getchar();

																// if(target_changed)
																target_changed = true ;

																l_it = current_loop.find(target); 
															}

															else if(!check_if_loop(all_loops, target) && status != NO_ASSERTION)
															{
																if(status == ASSERTION_FAILED)
																{
																	l_it = current_loop.find(target->get_target());
																	target_changed = true ;
																}
																else
																{
																	target_changed = false ;
																}
																status = NO_ASSERTION ;

															}	
															else if(check_if_loop(all_loops, target))
															{
																//std::cout<<"\n HANDLE LOOPS In handle_loops\n\n";
																handle_loops(all_loops.loop_map.find(*l_it)->second, all_loops, goto_model, ns);
																l_it = current_loop.find(target->get_target());	
																target_changed = true ;		   
															}
															
															else if(target->is_backwards_goto())
															{
																target_changed = true ;
																 l_it = current_loop.end();
															}break ;	

				case goto_program_instruction_typet::ASSERT : handle_assertions(*target, goto_model, ns); target_changed = false; break;
				
				default: std::cout<<"Cannot Recognise the instruction\n"; target_changed = false ;
			}

			print_all();
			getchar();

			if(!target_changed)
				{
					l_it++;			//std::cout<<"Instruction : "<<as_string(ns, **l_it)<<"\n";
				}

			//std::cin.get();
		}

		l_it = current_loop.begin(); 
		//std::cout<<"One Iteration Done\n";

//		if(iter_number == 1)
			join_values(interval_map_before_loop);
		// else
		// 	join_values(interval_map_prev_iteration);

		std::cout<<"\n\n ITERATION COMPLETE \n\n";
		print_all();
		check_for_convergence(interval_map_prev_iteration, converged);
		iter_number++ ;
	}	

	if(iter_number>=threshold && !converged && check_condition(simplified, goto_model, ns, false))
	{
		std::cout<<"--------------> WIDENING \n\n";
		widening(current_loop, all_loops,goto_model, ns); 
		//For inner loops, if encountered, simply ignore as they would have already been widened.
	}
}

void abstract_interpreter :: join_values(std::map<irep_idt, interval*> &interval_map_before_loop)
{
	std::map<irep_idt, interval*>::iterator it = interval_map.begin();
	std::map<irep_idt, interval*>::iterator it_init ;

	while(it!=interval_map.end())
	{
		it_init = interval_map_before_loop.find(it->first);

		if(it_init!=interval_map_before_loop.end())
		{
			//std::cout<<"Before Join : ";
			//it->second->print_interval();
			//it_init->second->print_interval();
			join(it->second, it_init->second);

			//std::cout<<"After join : ";
			//it->second->print_interval();

		}
		else
		{
			std::cout<<"Didnot Find Symbol\n";
		}

		it++ ;
	}
}

void abstract_interpreter :: check_for_convergence(std::map<irep_idt, interval*> &interval_map_prev_iteration, bool &converged)
{
	std::map<irep_idt, interval*>::iterator it = interval_map.begin();
	std::map<irep_idt, interval*>::iterator it_init ;	

	bool conv = true ; ;
	bool lower_same, upper_same ;
	while(it!=interval_map.end())
	{
		it_init = interval_map_prev_iteration.find(it->first);

		if(it_init!=interval_map_prev_iteration.end())
		{
			//Checking equivalence of 2 intervals
			interval* first ;
			interval* second ;

			first = it->second; 
			second = it_init->second ;

			if(first->is_minus_inf() && second->is_minus_inf())
				lower_same = true ;

			else if(first->get_lower_bound() == second->get_lower_bound() && !(first->is_minus_inf() || second->is_minus_inf()))
				lower_same = true ;
			else
				lower_same = false ;


			if(first->is_plus_inf() && second->is_plus_inf())
				upper_same = true ;

			else if(first->get_upper_bound() == second->get_upper_bound() && !(first->is_plus_inf() || second->is_plus_inf()))
				upper_same = true ;
			else
				upper_same = false ;

			if(!(lower_same && upper_same))
			{
				conv = false ;
				converged =false ;
				break ;
			}
		}
		else
		{
			std::cout<<"Didnot Find Symbol\n";
		}

		it++ ;
	}

	if(conv)
		converged = true ;
}

void abstract_interpreter ::copy_map(std::map<irep_idt, interval*> &copy)
{
	std::map<irep_idt, interval*>::iterator it = interval_map.begin();
	interval* copied_interval;

	// std::cout<<"Before COPY : \n";
	// print_all() ;
	while(it!=interval_map.end())
	{
		if(it->second->get_sign() == integer_type::SIGNED)
		{
			copied_interval = new interval(integer_type::SIGNED);
		}
		else
		{
			copied_interval = new interval(integer_type::UNSIGNED);
		}

		if(it->second->is_minus_inf())
			copied_interval->set_lower_bound(-1, true);
		else
			copied_interval->set_lower_bound(it->second->get_lower_bound(), false);

		if(it->second->is_plus_inf())
			copied_interval->set_upper_bound(-1, true);
		else
			copied_interval->set_upper_bound(it->second->get_upper_bound(), false);

		copy.insert(std::pair<irep_idt, interval*>(it->first, copied_interval));

		it++ ;
	}


	//std::cout<<"\n COPIED MAP : \n";
	//std::map<irep_idt, interval*>::iterator it2 ;
	// std::cout<<"---------------- INTERVALS -------------------------\n";

	// for(it2 = copy.begin() ; it2!=copy.end() ; it2++)
	// {
	// 	std::cout<<id2string(it2->first)<<" : ";
	// 	it2->second->print_interval();
	// 	std::cout<<"\n";
	// }

	// std::cout<<"*************************************************\n\n";

}

void abstract_interpreter :: widening (natural_loops_mutablet::natural_loopt &current_loop, natural_loops_mutablet &all_loops,
										  goto_modelt &goto_model, namespacet &ns)
{
	std::map<irep_idt, interval*> interval_map_before;
	copy_map(interval_map_before);

	//std::cout<<"BEFORE WIDENEING : ******************************\n\n";
	print_all();

	natural_loops_mutablet::natural_loopt::iterator l_it = current_loop.begin() ;
	l_it++ ;

	bool target_changed = false ;

	while(l_it != current_loop.end())
	{

		//std::cout<<"\nComes in here\n" ;
		goto_programt::targett target = *(l_it) ;
		std::cout<<"Instruction : "<<as_string(ns, *target)<<"\n";

		switch(target->type)
		{
			case goto_program_instruction_typet::DECL :  handle_declaration(*target, goto_model); break;

			case goto_program_instruction_typet::ASSIGN :handle_assignments(*target, goto_model); target_changed = false ; break ;

			case goto_program_instruction_typet::GOTO : if(!check_if_loop(all_loops, target) && !target->is_backwards_goto()) 
														{	
															//std::cout<<"IN HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\n";
															//getchar() ;
															handle_goto(*target, goto_model, target, target_changed,all_loops,ns);
															// getchar();
															// print_all();
															target_changed = true ;
															l_it = current_loop.find(target);
														}

														else if(check_if_loop(all_loops, target))
														{
															//handle_loops(all_loops.loop_map.find(*l_it)->second, all_loops, goto_model, ns);
															//std::cout<<"IN HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE ********************\n\n";

															l_it = current_loop.find(target->get_target());	
															target_changed = true ;		   
														}
														
														else if(target->is_backwards_goto())
														{
															//std::cout<<"IN HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE $$$$$$$$$$$$$$$$$$\n\n";
															target_changed = true ;
															 l_it = current_loop.end();
														}
														else
														{
															//std::cout<<"IN HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE ^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n";
														} break ;	

			default: std::cout<<"Cannot Recognise the instruction\n"; target_changed = false ;
		}

		print_all();
		getchar();

		if(!target_changed)
			{
				l_it++;			//std::cout<<"Instruction : "<<as_string(ns, **l_it)<<"\n";
			}
	}	

	join_values(interval_map_before);

	// std::cout<<"-----------> After WIDENING \n";
	// print_all();

	std::map<irep_idt, interval*>::iterator it_b = interval_map_before.begin();
	std::map<irep_idt, interval*>::iterator it_o ;

	while(it_b!=interval_map_before.end())
	{
		it_o = interval_map.find(it_b->first);

		if(it_o != interval_map.end())
		{
			if(!check_if_equal(it_o->second, it_b->second))
			{
				interval* widened ;
				if(it_o->second->get_sign() == integer_type::SIGNED)
					widened = new interval(integer_type::SIGNED);
				else
					widened = new interval(integer_type::UNSIGNED);

				// std::cout<<"\n GOING TO WIDEN : ";
				// it_b->second->print_interval();
				// it_o->second->print_interval(); 
				// std::cout<<"\n\n";

				bool w = widen(it_b->second, it_o->second, widened) ;

				if(w)
					it_o->second->make_equal(*widened);

			}
		}

		it_b++;
	}

	std::cout<<"-------------> AFTER WIDENING \n\n";
	print_all();
}

bool abstract_interpreter :: check_if_equal(interval* first , interval* second)
{
	bool lower_same, upper_same ;

	if(first->is_minus_inf() && second->is_minus_inf())
		lower_same = true ;

	else if(first->get_lower_bound() == second->get_lower_bound() && !(first->is_minus_inf() || second->is_minus_inf()))
		lower_same = true ;
	else
		lower_same = false ;	


	if(first->is_plus_inf() && second->is_plus_inf())
		upper_same = true ;

	else if(first->get_upper_bound() == second->get_upper_bound() && !(first->is_plus_inf() || second->is_plus_inf()))
		upper_same = true ;
	else
		upper_same = false ;	


	if(lower_same && upper_same)
		return true ;
	else
		return false ;
}

void abstract_interpreter :: handle_assignments_widen(goto_programt::instructiont &instruction, goto_modelt &goto_model, std::map<irep_idt, interval*> &interval_map_prev)
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

		//std::cout<<"RHS Expression  : "<<expr2c(expression, ns)<<"\n"; 
		exprt simplified = simplify_expr(expression, ns);

		//CHECK FOR CONSTANT PROPAGATION?? //MAYBE NOT //CONFIRM ONCE

		//std::cout<<"Simplified Expression : "<<expr2c(simplified,ns)<<"\n";
		interval temp = handle_rhs(expression, goto_model);
		
		interval temp_widened(integer_type::SIGNED);

		//std::map<irep_idt, interval*>::iterator it_p = interval_map_prev.find(it->first);

		bool widened = widen(it->second, &temp , &temp_widened);

			// std::cout<<"Sent for Widenening : ";
			// it->second->print_interval();
			//it_p->second->print_interval();
			// temp.print_interval();
			// temp_widened.print_interval();
			// std::cout<<"\n\n";
			
		if(widened)
			it->second->make_equal(temp_widened);

		// std::cout<<"Widened : ";
		// it->second->print_interval();
		// std::cout<<"\n&&&&&&&&&&&&&&\n";


	}

}

bool abstract_interpreter :: check_assert(exprt &expr, goto_modelt &goto_model, namespacet &ns)
{
	bool  is_not = false ;

	if(can_cast_expr<not_exprt>(expr))
	{
		expr.make_not();
		is_not = true ;
	}	

	if(expr.id() == ID_equal || expr.id() == ID_notequal || expr.id() == ID_lt || expr.id() == ID_le || expr.id() == ID_gt || expr.id() == ID_ge)
	{
		maybe = false ;

		if(!is_not)
		{

		   if(check_condition(expr, goto_model, ns, false) && !maybe)
		   	return true ;
		   else
		   	return false ;
		}
		else
		{
			//std::cout<<"Is not false\n";
			if(!check_condition(expr, goto_model, ns, false) && !maybe) 
				return true;
			else
				return false ;
		}	
	}

	else
	{
		if(expr.id() == ID_and)
		{
			bool lhs = check_assert(expr.op0(), goto_model, ns);
			bool rhs = check_assert(expr.op1(), goto_model, ns);
			bool extra1 = true ;
			bool extra2 = true ;

			//std::cout<<"NUMBER OF OPERANDS : "<<expr.operands().size()<<"\n";
			if(expr.operands().size() >= 3)
				extra1 = check_assert(expr.op2(), goto_model, ns);

			if(expr.operands().size() == 4)
				extra2 = check_assert(expr.op3(), goto_model, ns);

			if(is_not)
				return !(lhs && rhs && extra1 && extra2);
			else
				return (lhs && rhs && extra1 && extra2);
		}

		else if(expr.id() == ID_or)
		{
			bool lhs = check_assert(expr.op0(), goto_model, ns);
			bool rhs = check_assert(expr.op1(), goto_model, ns);
			bool extra1 = true ;
			bool extra2 = true ;

			if(expr.operands().size() >= 3)
				extra1 = check_assert(expr.op2(), goto_model, ns);

			if(expr.operands().size() == 4)
				extra2 = check_assert(expr.op3(), goto_model, ns);

			if(is_not)
				return !(lhs || rhs || extra1 || extra2);
			else
				return (lhs || rhs || extra1 || extra2);			
		}

		else
		{
			std::cout<<"Cannot Recognise Operator\n";
			return false ;
		}
	}	
}

void abstract_interpreter :: handle_assertions(goto_programt::instructiont &instruction ,
												goto_modelt &goto_model,
												namespacet &ns)
{
	exprt guard_expr = instruction.guard  ;

	if(check_assert(guard_expr, goto_model, ns))
	{
		std::cout<<"Assertion PASS\n\n" ;
		status = ASSERTION_PASSED ;
	}
	else
	{
		std::cout<<"Assertions FAILS\n\n";
		status = ASSERTION_FAILED ;
	}
}	