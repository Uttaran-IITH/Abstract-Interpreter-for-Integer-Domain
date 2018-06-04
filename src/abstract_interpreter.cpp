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

/*******************************************************************
Function : run_interpreter

Inputs : goto_model

Outputs : void

Purpose : Iterate over the goto-functions in goto_model
*******************************************************************/

void abstract_interpreter::run_interpreter(goto_modelt &goto_model)
{
	std::cout<<"Running Interpreter : \n";
	namespacet ns(goto_model.symbol_table);

	threshold = -1 ;

	natural_loops_mutablet loops ;

	Forall_goto_functions(f_it, goto_model.goto_functions)
	{
		//Ignoring CPROVER_start and CPROVER_initialize
		if(f_it->first == "main")
		{	
			loops(f_it->second.body);

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

					case goto_program_instruction_typet::GOTO : if(!check_if_loop(loops, it) && status == NO_ASSERTION) //Handling if..else (that donot correspond to assertions)
																	handle_goto(*it, goto_model, it, target_changed, loops, ns);

																//Handling 'if' Block succeeding an assertion	
																else if(!check_if_loop(loops, it) && status != NO_ASSERTION)
																{
																	if(status == ASSERTION_FAILED)
																	{
																		it = instruction.get_target();
																		target_changed = true ;
																	}
																	status = NO_ASSERTION ;

																}
																//Handling Loops
																else
																{
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


/*******************************************************************
Function : check_if_loop

Inputs : loops(for a goto-function), GOTO instruction to be checked

Outputs : bool

Purpose : check whether the GOTO statement is the header for a loop
			return true if yes else no
*******************************************************************/
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


/*******************************************************************
Function : print_all

Inputs : void

Outputs : void

Purpose : print intervals for all the variables
*******************************************************************/
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


/*******************************************************************
Function : handle_declarations

Inputs : instruction to be handled, goto_model

Outputs : void

Purpose : Add the declared variable to the interval_map according to
		  its type(signed or unsigned)
*******************************************************************/
void abstract_interpreter :: handle_declaration(goto_programt::instructiont &instruction, goto_modelt &goto_model)
{
	code_declt decl = to_code_decl(instruction.code);
	symbol_exprt symbol_expr = to_symbol_expr(decl.symbol());
	const symbolt* symbol = goto_model.symbol_table.lookup(symbol_expr.get_identifier());

	//For signed intergers
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

	//For unsigned integers
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


/****************************************************************************
Function : handle_rhs

Inputs : assign statement's rhs expression, goto_model

Outputs : interval object (that the lhs of the assignment should be assigned)

Purpose : Recursive function to calculate the resulting interval
		  after all the operations that occur in the rhs expression
*****************************************************************************/
interval abstract_interpreter :: handle_rhs(exprt& expression, goto_modelt& goto_model)
{
	bool neg = false ;

	//Handles (-) unary operator
	if(can_cast_expr<unary_minus_exprt>(expression))
	{
		neg = true ;
		exprt curr = expression.op0();
		expression = curr ;
	}

	//Base Case : symbol 
	if(expression.id() == ID_symbol)
	{
		symbol_exprt sym_expr = to_symbol_expr(expression);
		const symbolt* symbol = goto_model.symbol_table.lookup(sym_expr.get_identifier());
		std::map<irep_idt, interval*>::iterator it ;
		interval* object = new interval(integer_type::SIGNED);

		//Find the symbol in the interval_map
		it = interval_map.find(symbol->name);
		//object = it->second ;

		//If (-) preceded then return the negated interval			
		if(neg)
		{	
			negate(it->second, object);
		}
		else
		{
			object = it->second ;
		}

		return *object ;

	}

	//Base Case : constant
	else if(expression.id() == ID_constant)
	{
		constant_exprt constant_expr = to_constant_expr(expression);
		mp_integer value;
		to_integer(constant_expr, value);
		
		interval* constant = new interval(integer_type::SIGNED);
		interval* object = new interval(integer_type::SIGNED);

		//For constant c create the interval [c,c]
		constant->set_lower_bound(value, false);
		constant->set_upper_bound(value, false);

		// If (-) preceded, negate
		if(neg)
		{	
			negate(constant, object);
		}
		else
		{
			 object = constant ;
		}

		return *object ;

	}

	//Recursive Cases
	else
	{
		//Handling '+' operator
		if(expression.id() == ID_plus)
		{
			plus_exprt plus_expr = to_plus_expr(expression);

			//Call handle_rhs recursively for the two operands of plus
			interval arg1 = handle_rhs(plus_expr.op0() , goto_model);
			interval arg2 = handle_rhs(plus_expr.op1(), goto_model);

			integer_type type = integer_type::SIGNED ; 

			//If both arguments unsigned, the result cannot be signed
			if(arg1.get_sign()  == integer_type::UNSIGNED &&
				arg2.get_sign() == integer_type::UNSIGNED)
			{
				type = integer_type::UNSIGNED ;
			}

			interval add_result(type) ;

			//Call add and store result in add_result
			add(arg1, arg2, &add_result);

			//If (-)preceded, regate
			if(neg)
			{	
				negate(&add_result);
			}

			return add_result;
		}

		//Handling '-' binary operator
		else if(expression.id() == ID_minus)
		{

			minus_exprt sub_expr =  to_minus_expr(expression);

			//Call handle_rhs recursively for the two operands of minus
			interval arg1 = handle_rhs(sub_expr.op0() , goto_model);
			interval arg2 = handle_rhs(sub_expr.op1(), goto_model);

			//If both arguments unsigned, the result cannot be signed
			integer_type type = integer_type::SIGNED ;
			// if(arg1.get_sign()  == integer_type::UNSIGNED &&
			// 	arg2.get_sign() == integer_type::UNSIGNED)
			// {
			// 	type = integer_type::UNSIGNED ;
			// }

			interval sub_result(type);

			//Subtract and store result in sub_result
			sub(arg1, arg2, &sub_result);

			// If (-) preceded, negate
			if(neg)
			{	
				negate(&sub_result);
			}

			return sub_result ;			

		}

		//Handling '*' operator
		else if(expression.id() == ID_mult)
		{

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

			if(neg)
			{	
				negate(&mult_result);
			}
			return mult_result ;
		}

		//Handle '/' operator
		else if(expression.id() == ID_div)
		{

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


/*******************************************************************
Function : handle_assignments

Inputs : instruction to be handled, goto_model

Outputs : void

Purpose : Finds the lhs of the ASSIGN, calculates the interval to be
		  assigned u=by calling handle_rhs() and set updates LHS 
		  variable's value
*******************************************************************/
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

		exprt simplified = simplify_expr(expression, ns);
		interval temp = handle_rhs(expression, goto_model);
		it->second->make_equal(temp);			

	}

}

//Deprecated
void create_complementary_expr(exprt &expr, exprt &comp_expr, goto_modelt &goto_model)
{
	not_exprt expr_not = to_not_expr(expr);
	expr_not.make_not();
	namespacet ns(goto_model.symbol_table);
	comp_expr = simplify_expr(expr_not, ns);
}


/**********************************************************************
Function : set_lhs

Inputs : lhs_symbol, goto_model

Outputs : pointer to the lhs symbol

Purpose : Called while checking GOTO guards, to get the interval of lhs
***********************************************************************/
void abstract_interpreter :: set_lhs(symbol_exprt &lhs_sym, interval* &lhs, goto_modelt &goto_model)
{
	const symbolt* symbol = goto_model.symbol_table.lookup(lhs_sym.get_identifier());
	std::map<irep_idt, interval*>::iterator it;

	it = interval_map.find(symbol->name);

	if(it!=interval_map.end())
	{
		lhs = it->second ;
	}
}


/**********************************************************************
Function : set_rhs

Inputs : rhs_symbol, goto_model

Outputs : pointer to the rhs symbol

Purpose : Called while checking GOTO guards, to get the interval of RHS
***********************************************************************/
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
		}
	}	

}


/******************************************************************************
Function : remove_not

Inputs : expr, namespace

Outputs : expr(modified)

Purpose : creates complementary expressions (those not handled by simplify_expr)
*********************************************************************************/
void abstract_interpreter :: remove_not(exprt &expr, namespacet &ns)
{	
	if(expr.id() == ID_and)
	{
		exprt lhs = expr.op0();
		exprt rhs = expr.op1();

		remove_not(lhs,ns);
		remove_not(rhs, ns);

		exprt new_expr = and_exprt(lhs, rhs);

		expr = new_expr ;
	}

	else if(expr.id() == ID_or)
	{
		exprt lhs = expr.op0();
		exprt rhs = expr.op1();

		remove_not(lhs,ns);
		remove_not(rhs, ns);

		exprt new_expr = or_exprt(lhs, rhs);

		expr = new_expr ;		
	}

	if(can_cast_expr<not_exprt>)
	{
		exprt inside_expr = expr.op0();

		if(inside_expr.id() == ID_equal)
		{
			exprt new_expr(ID_notequal);
			new_expr.operands().push_back(inside_expr.op0());
			new_expr.operands().push_back(inside_expr.op1());

			expr = new_expr ;
		}
		else if(inside_expr.id() == ID_ge)
		{
			exprt new_expr(ID_lt);
			new_expr.operands().push_back(inside_expr.op0());
			new_expr.operands().push_back(inside_expr.op1());

			expr = new_expr ;
		}
		else if(inside_expr.id() == ID_le)
		{
			exprt new_expr(ID_gt);
			new_expr.operands().push_back(inside_expr.op0());
			new_expr.operands().push_back(inside_expr.op1());
			expr = new_expr ;
		}
	}
}



/******************************************************************************
Function : iterate_over_if

Inputs : target insruction(start of if block), goto_model, 
		 target of the GOTO in if header, loops in the function, namespace

Outputs : target_end set to the instruction after the if..else contruct,
		  else_present variabe = whether an else block is present or not

Purpose : iterates over the if_block
*********************************************************************************/
void abstract_interpreter :: iterate_over_if(goto_programt::targett &it, goto_modelt &goto_model, 
						   goto_programt::targett &target_if, goto_programt::targett &target_end, bool &else_present, natural_loops_mutablet &loops, namespacet &ns)
{
	bool if_end = false ;

	while(!if_end && it != target_if)
	{
		goto_programt::instructiont instruction = *it ;

		std::cout<<"Instruction : "<<as_string(ns, *it)<<"\n";
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
														else
														{
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
			}

		print_all();
		getchar();
	}

	if(it == target_if)
		else_present = false ;
}


/******************************************************************************
Function : check_if_else_present

Inputs : target instruction, target_if = target to  

Outputs : target_end =  target to the end of if..else block , else_present = whether
															  else block present or not

Purpose : checks whether an else block is present or not in case if condition returns false
*********************************************************************************/
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

				it++ ;
				check_if_else_present(it, target_end, t, temp);

				if(temp)	
				{
				 	nested_goto_found ++ ;
				}
			}
		}

		it++ ;
	}

	if(it == target_if)
		else_present = false;
}


/******************************************************************************
Function : iterate_over_if

Inputs : target insruction(start of if block), goto_model, 
		 target of the GOTO in if header, loops in the function, namespace

Outputs : target_end set to the instruction after the if..else contruct,
		  else_present variabe = whether an else block is present or not

Purpose : iterates over the if_block
*********************************************************************************/
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

/******************************************************************************
Function : restore_map

Inputs : copy

Outputs : (original map)

Purpose : restore the input map to original map
*********************************************************************************/
void abstract_interpreter ::restore_map(std::map<irep_idt, interval*> &copy)
{
	std::map<irep_idt, interval*>::iterator it = copy.begin();
	interval* copied_interval;

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
	
}


/******************************************************************************
Function : handle_goto

Inputs : target insruction(start of if block), goto_model, 
		 instruction pointer, loops in the function, namespace

Outputs : target_changed (sets to true always)

Purpose : handles gotos
*********************************************************************************/
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
		//std::cout<<"EXPRESSION FOR ELSE : "<<expr2c(expr_true, ns);
		if(expr_true.op0().id()== ID_lt || expr_true.op0().id() == ID_gt)
		{
			expr_true.op0().op1()= expr.op0().op1();
		}
		
		remove_not(expr_true, ns);
		std::cout<<"EXPRESSION FOR ELSE : "<<expr2c(expr_true, ns);

		expr.make_not();
		exprt expr_false =  expr;

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

			std::cout<<"INSIDE IF : \n";
			print_all();
			if_implemented = true ;

			
			iterate_over_if(++it, goto_model, target_if, target_end, else_present, loops, ns) ;

		}
		else
		{
			check_if_else_present(it, target_end, target_if, else_present);
		}


		if(else_present == true && maybe && if_implemented)
		{

			it = target_if ;
			print_all();
			copy_map(after_if_map);
			restore_map(original_map);
			check_condition(expr_true , goto_model, ns, true);

			std::cout<<"INSIDE ELSE : \n\n";
			print_all();
			iterate_over_else(it, goto_model, target_end, loops, ns);
			else_implemented = true ;
		}

		else if(else_present == true && !if_implemented)
		{

			check_condition(expr_true , goto_model, ns, true);
			
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
 			join_values(original_map) ;
 		}

 		if(else_present)
 			it = target_end ;
 		else
 			it = target_if ;


 		target_changed = true ;
 		std::cout<<"AFTER IF..ELSE BLOCK : \n\n";
 		print_all() ;
	}
	}
}

bool abstract_interpreter :: check_condition(exprt &expr, goto_modelt &goto_model, namespacet &ns, bool update)
{
	bool can_enter_loop = false ;


	if(expr.has_operands())
	{		

		if(expr.id() == ID_and)
		{
			getchar();
			
			bool lhs = false ;
			bool rhs = false ;
			bool something_changed = false ;

			std::map<irep_idt, interval*> interval_map_before;
			copy_map(interval_map_before);
			
			lhs = check_condition(expr.op0(), goto_model, ns , update);

			if(lhs)
			{
				something_changed = true ;
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
				rhs = check_condition(expr.op1(), goto_model,ns, update);
			}

			can_enter_loop = lhs || rhs ;

			return can_enter_loop ;
		}

		if(can_cast_expr<binary_relation_exprt>(expr))
		{

			binary_relation_exprt binary_relation_expr = to_binary_relation_expr(expr);

			symbol_exprt lhs_sym = to_symbol_expr(binary_relation_expr.lhs());
			exprt rhs_expr = binary_relation_expr.rhs();

			interval* rhs ;
			interval* lhs ;

			set_lhs(lhs_sym, lhs, goto_model);
	

			set_rhs(rhs_expr, rhs, goto_model);

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

			}

			else if(binary_relation_expr.id() == ID_le || binary_relation_expr.id() == ID_lt)
			{
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
	bool converged = false ;

	if(threshold ==  -1)
	{
		std::cout<<"Enter threshold : ";
		std::cin>>threshold ;
	}
	goto_programt::instructiont &instruction = **l_it;

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


	print_all();

	int iter_number = 0 ;
	std::map<irep_idt, interval*> interval_map_prev_iteration;

	while(check_condition(simplified, goto_model, ns, true) && enter_loop && !converged && iter_number<threshold)
	{
		bool target_changed = false ;
		++l_it ;

		// std::map<irep_idt, interval*> interval_map_prev_iteration;
		// copy_map(interval_map_prev_iteration);

		while(l_it != current_loop.end())
		{

			goto_programt::targett target = *(l_it) ;
			std::cout<<"Instruction : "<<as_string(ns, *target)<<"\n";

			switch(target->type)
			{
				case goto_program_instruction_typet::DECL :  handle_declaration(*target, goto_model); break;

				case goto_program_instruction_typet::ASSIGN : handle_assignments(*target, goto_model); target_changed = false ; break ;

				case goto_program_instruction_typet::GOTO : if(!check_if_loop(all_loops, target) && !target->is_backwards_goto() && status == NO_ASSERTION) 
															{

																handle_goto(*target, goto_model, target, target_changed, all_loops, ns);

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
					l_it++;		
				}
		}

		l_it = current_loop.begin(); 

		//if(iter_number > 0)
			join_values(interval_map_before_loop);
		// else
		// 	join_values(interval_map_prev_iteration);

		std::cout<<"\n\n ITERATION COMPLETE \n\n";
		print_all();

		//Map of Previous Iteration

		if(iter_number>0)
		{
			check_for_convergence(interval_map_prev_iteration, converged);
		}


		copy_map(interval_map_prev_iteration);
		// std::cout<<"Copies MAP : in HANDLE LOOPS \n\n";
		// std::map<irep_idt, interval*>::iterator eti = interval_map_prev_iteration.begin();

		// while(eti!=interval_map_prev_iteration.end())
		// {
		// 	std::cout<<id2string(eti->first)<<" : ";
		// 	eti->second->print_interval();
		// 	eti++ ;
		// }

		iter_number++ ;
	}	

	if(iter_number>=threshold && !converged && check_condition(simplified, goto_model, ns, false))
	{
		std::cout<<"--------------> WIDENING \n\n";
		widening(current_loop, all_loops,goto_model, ns); 
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

			join(it->second, it_init->second);

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

	// std::cout<<"\nPrev Iteration Map : \n" ;
	// while(it_init!=interval_map_prev_iteration.end())
	// {
	// 	std::cout<<id2string(it_init->first)<<" : ";
	// 	it_init->second->print_interval();
	// 	it_init++ ;
	// }

	getchar();

	it_init = interval_map_prev_iteration.begin();

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
			std::cout<<"Symbol : "<<id2string(it->first)<<"\n";
			std::cout<<"Didnot Find Symbol for Convergence\n";
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


	while(it!=interval_map.end())
	{
		std::cout<<"Copying Symbol :"<<id2string(it->first)<<" \n ";
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
}

void abstract_interpreter :: widening (natural_loops_mutablet::natural_loopt &current_loop, natural_loops_mutablet &all_loops,
										  goto_modelt &goto_model, namespacet &ns)
{
	std::map<irep_idt, interval*> interval_map_before;
	copy_map(interval_map_before);

	print_all();

	natural_loops_mutablet::natural_loopt::iterator l_it = current_loop.begin() ;
	l_it++ ;

	bool target_changed = false ;

	while(l_it != current_loop.end())
	{

		goto_programt::targett target = *(l_it) ;
		std::cout<<"Instruction : "<<as_string(ns, *target)<<"\n";

		switch(target->type)
		{
			case goto_program_instruction_typet::DECL :  handle_declaration(*target, goto_model); break;

			case goto_program_instruction_typet::ASSIGN :handle_assignments(*target, goto_model); target_changed = false ; break ;

			case goto_program_instruction_typet::GOTO : if(!check_if_loop(all_loops, target) && !target->is_backwards_goto()) 
														{	
															handle_goto(*target, goto_model, target, target_changed,all_loops,ns);
															target_changed = true ;
															l_it = current_loop.find(target);
														}

														else if(check_if_loop(all_loops, target))
														{
															l_it = current_loop.find(target->get_target());	
															target_changed = true ;		   
														}
														
														else if(target->is_backwards_goto())
														{
															target_changed = true ;
															 l_it = current_loop.end();
														}
														else
														{
														} break ;	

			default: std::cout<<"Cannot Recognise the instruction\n"; target_changed = false ;
		}

		print_all();
		getchar();

		if(!target_changed)
			{
				l_it++;	
			}
	}	

	join_values(interval_map_before);


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

		exprt simplified = simplify_expr(expression, ns);


		interval temp = handle_rhs(expression, goto_model);
		
		interval temp_widened(integer_type::SIGNED);

		bool widened = widen(it->second, &temp , &temp_widened);

			
		if(widened)
			it->second->make_equal(temp_widened);



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