#include <iostream>
#include <cstdlib>
#include <stack>
#include <vector>

//#include <fstream>

#include "check_wp.h"
#include "z3_solver.h"

#include <ansi-c/expr2c.h>
#include <util/simplify_expr.h>

//Constructs weakest conservative precondition with following rules
// wp(x = E, wp') = wp'[x = E]
// wp(assert E, wp') = wp' && E
// wp(assume E, wp') = E -> wp'
// wp(ite E0 E1 E2, wp') = (E0 -> wp(E1, wp')) && ( (~E0 -> wp(E2, wp'))
//                       = (E0 && wp(E1, wp')) || (E1 && wp(E2, wp'))

check_wpt::ve_vectort
check_wpt::create_ite_ve_vector(ve_vectort nve,
				ve_vectort ve,
				const exprt& g)
{
  ve_vectort out;
  for (unsigned int nve_i = 0; nve_i < nve.size(); ++nve_i) {
    bool found = false;
    for (unsigned int ve_i = 0; ve_i < ve.size(); ++ve_i) {
      if (to_symbol_expr(nve[nve_i].first).get_identifier() ==
	  to_symbol_expr(ve[ve_i].first).get_identifier()) {
	if_exprt ite_expr = if_exprt(g, ve[ve_i].second, nve[nve_i].second);
	simplify(ite_expr, ns);
	out.push_back(ve_pairt(ve[ve_i].first, ite_expr));
	found = true;
	ve.erase(ve.begin() + ve_i);
      } 
    }
    if (!found) {
      if_exprt ite_expr = if_exprt(g, nve[nve_i].first, nve[nve_i].second);
      simplify(ite_expr, ns);
      out.push_back(ve_pairt(nve[nve_i].first, ite_expr));
    }
  }
  
  for (unsigned int ve_i = 0; ve_i < ve.size(); ++ve_i) {
    if_exprt ite_expr = if_exprt(g, ve[ve_i].second, ve[ve_i].first);
    simplify(ite_expr, ns);
    out.push_back(ve_pairt(ve[ve_i].first, ite_expr));
  }      

  return out;
						      
    
}
void
check_wpt::generate_wp (exprt &wp, goto_programt::instructionst instructions, const irep_idt& fname)
{

  fve_map[fname]; //to avoid recursion initialize
  ve_vectort ve_vector; //used to store variable and their substitutions in wp
  ve_vectort prev_ve_vector;
  ve_vectort else_ve_vector;
  
  exprt prev_wp = true_exprt();;
  exprt else_wp = true_exprt();
  
  for (goto_programt::instructionst::const_reverse_iterator r_it = instructions.rbegin();
       r_it != instructions.rend(); ++r_it) {

    // if (r_it->is_target()) {
    //   codet c = r_it->code;
    //   exprt g = r_it->guard;
    //   std::cout << "target node! " << (r_it->target_number)<< expr2c(c,ns) << std::endl;
    //   std::cout << "target node! " << expr2c(g,ns) << std::endl;
    // }
    //TODO: This logic utterly fails when multiple conditions are given in if(a || b)
    if (r_it->is_goto()) {
      exprt g = r_it->guard;
      //      std::cout << "goto with guard " << expr2c(g,ns) << std::endl;
      if (g == true_exprt()) {
	//probably else loop
	// std::cout << "wp value " << expr2c(wp,ns) << std::endl;
	// std::cout << "else_expr value " << expr2c(else_wp,ns) << std::endl;
	// std::cout << "prev_wp value " << expr2c(prev_wp,ns) << std::endl;
	else_wp = wp;
	wp = prev_wp;

	else_ve_vector = ve_vector;
	ve_vector = prev_ve_vector;
	
      } else {
	//we have completed a loop
	wp = or_exprt(and_exprt(wp, not_exprt(g)),
		      and_exprt(else_wp, g));
	ve_vector = create_ite_ve_vector(ve_vector, else_ve_vector, g); 
	//	std::cout << "wp value " << expr2c(wp,ns) << std::endl;
	if (r_it->is_target()) {
	  prev_wp = wp;
	  prev_ve_vector = ve_vector;
	}
      }
      
    } else if (r_it->is_assert()) {
      exprt g = r_it->guard;
      // std::cout << "Adding negation of assert " << expr2c(g,ns)
      // 		<< " to " << expr2c(wp,ns) << std::endl;
      //      wp = and_exprt(wp, not_exprt(g));
      wp = and_exprt(wp, g);
      //this is a target node but previous instruction is not a goto
      //so we are (hopefully) out of if/else block.
      if (r_it->is_target()) {
	goto_programt::instructionst::const_reverse_iterator tmp_itr = r_it;
	tmp_itr++;
	if (!(tmp_itr->is_goto())) {
	  prev_wp = wp;
	  prev_ve_vector = ve_vector;
	}
      }
      
    } else if (r_it->is_assume()) {
      exprt assumption = r_it->guard;
      // std::cout << "Weakest precondition is " << expr2c(wp, ns) << std::endl;
      // std::cout << "Assumption is " << expr2c(assumption, ns) << std::endl;
      //wp = and_exprt(assumption, wp);
      wp = implies_exprt(assumption, wp);
      //      simplify(wp, ns);
      //break;
    } else if (r_it->is_function_call()) {
      const code_function_callt &fcode = to_code_function_call(r_it->code);
      irep_idt fname = (fcode.function()).get(ID_identifier);

      goto_functionst::function_mapt::const_iterator
	fmap_itr = goto_functions.function_map.find(fname);
      if (fmap_itr == goto_functions.function_map.end()) {
	std::cout << "Undefined reference to " << fname << std::endl;
	exit(77);
      }
	  
      const goto_functionst::goto_functiont &f_goto = fmap_itr->second;
      if(!f_goto.body_available){
	std::cout<<"body not found for the function " << fname<<std::endl;
	exit(22);
      }

      fve_mapt::iterator fve_itr = fve_map.find(fname);
      if (fve_itr == fve_map.end()) {
	exprt f_wp = true_exprt();
	generate_wp(f_wp, (f_goto.body).instructions, fname);
	std::cout << "wp computed for the function " << fname
		  << " is " << expr2c(f_wp,ns) << std::endl;
	fve_itr = fve_map.find(fname);
	assert(fve_itr != fve_map.end());
      }
      
      //DEBUG
      for (ve_vectort::const_iterator ve_itr = ((fve_itr->second).second).begin();
	   ve_itr != ((fve_itr->second).second).end();
	   ++ve_itr) {
	std::cout << expr2c(ve_itr->first, ns) << " ==> "
		  << expr2c(ve_itr->second, ns) << std::endl;
      }
      

     // std::cout << "wp before substitution: " << expr2c(wp,ns) << std::endl;
      recursive_replace_once(wp, (fve_itr->second).second);
     // std::cout << "wp after substitution: " << expr2c(wp,ns) << std::endl;
      //std::cout << "fwp " << expr2c((fve_itr->second).first,ns) << std::endl;

      //conjunct given wp with wp of f
      wp = and_exprt(wp, (fve_itr->second).first);

    } else {
      codet c = r_it->code;
      const irep_idt &statement = c.get_statement(); 
      if(statement == ID_assign) {
	wp_assign(c, wp, ve_vector);
	//this is a target node but previous instruction is not a goto
	//so we are (hopefully) out of if/else block.
	if (r_it->is_target()) {
	  goto_programt::instructionst::const_reverse_iterator tmp_itr = r_it;
	  tmp_itr++;
	  if (!(tmp_itr->is_goto())) {
	    prev_wp = wp;
	    prev_ve_vector = ve_vector;
	  }
	}
      }
    }
    #ifdef DEBUG
    std::cout << "wp before simplification: " << expr2c(wp,ns) << std::endl;
    #endif
    simplify(wp, ns);
    #ifdef DEBUG
    std::cout << "wp after simplification: " << expr2c(wp,ns) << std::endl;
    #endif
  }

  fve_map[fname] = wve_pairt(wp, ve_vector);
  //  fve_map.insert(std::pair<irep_idt, wve_pairt>(fname, wve_pairt(wp, ve_vector)));
}


void
check_wpt::generate_sp (exprt &sp, goto_programt::instructionst instructions)
{

  exprt last_wp = true_exprt();;
  exprt else_wp = true_exprt();
  std::vector <symbol_exprt> t;
  std::stack <exprt> if_sp;
  std::stack <exprt> else_sp;
  std::stack <exprt> if_gaurd;
  std::stack <exprt> else_gaurd;
 // bool is_go = false;
  //exprt final_condn;
  exprt k2;
  int counter = 0;
  int count = 0;

  for (goto_programt::instructionst::const_iterator r_it = instructions.begin();
       r_it != instructions.end(); ++r_it) {

   
    std::string tname("t" + std::to_string(counter));
     t.push_back(symbol_exprt(tname, bv_typet()));
     if (r_it->is_assume()) {
      exprt g = r_it->guard;
      // std::cout << "Adding negation of assert " << expr2c(g,ns)
      //    << " to " << expr2c(wp,ns) << std::endl;
      sp = and_exprt(sp, g);
      std::cout << "SP in assume is " << expr2c(sp,ns) << std::endl;
      
    }
    else if(r_it->is_goto()){
      exprt g1 = r_it->guard;
      exprt sp_temp = false_exprt();
      if(g1 != true_exprt()){
        else_gaurd.push(g1);
        if_gaurd.push(not_exprt(g1));
           if(r_it->is_target())
      {
        if(count > 0)
        {
          if_sp.push(sp);
          count--;
        }
      }
      if (r_it->is_target()) {
        //tmp_itr--;
          if(if_sp.size() != 0)
          {
            sp_temp = if_sp.top();
            if_sp.pop();
          }
         /* if(else_sp.size() != 0)
          {
            sp = else_sp.top();
            else_sp.pop();
          }*/
          if(else_sp.size() != 0)
          {
            sp = else_sp.top();
            else_sp.pop();
          }
          sp = or_exprt(sp_temp,sp);
        

      }
        
        else_sp.push(and_exprt(g1,sp));
        sp = and_exprt(not_exprt(g1),sp);
        count++;
      }
    }
     
    else if (r_it->is_assert()) {
      exprt asrt = r_it->guard;
      exprt sp_temp = false_exprt();
      goto_programt::instructionst::const_iterator tmp_itr = r_it;
      //if this is a target node then we need to compute if then else
      if(r_it->is_target())
      {
        if(count > 0)
        {
          if_sp.push(sp);
          count--;
        }
      }
      if (r_it->is_target()) {
        //tmp_itr--;
          if(if_sp.size() != 0)
          {
            sp_temp = if_sp.top();
            if_sp.pop();
          }
         /* if(else_sp.size() != 0)
          {
            sp = else_sp.top();
            else_sp.pop();
          }*/
          if(else_sp.size() != 0)
          {
            sp = else_sp.top();
            else_sp.pop();
          }
          sp = or_exprt(sp_temp,sp);
        

      }
      simplify(sp,ns);
      std::cout << "Strongest postcondition is " << expr2c(sp, ns) << std::endl;
      std::cout << "Assert is " << expr2c(asrt, ns) << std::endl;
      sp = implies_exprt(sp, asrt);
      std::cout << "Final sp is " << expr2c(sp, ns)<< "\n" << std::endl;
      break;
    }

     else
    {
      exprt temp1,temp2;
      codet c = r_it->code;
      const irep_idt &statement = c.get_statement(); 
      if(statement == ID_assign)
      {

      goto_programt::instructionst::const_iterator tmp_itr = r_it;
      //if this is a target node then we need to compute if then else
      if (r_it->is_target()) {
        tmp_itr--;
        if(count > 0)
        {
          if_sp.push(sp);
          count--;
        }
        // this is for taking else part
      if (tmp_itr->is_goto()) {
        exprt g_temp = tmp_itr->guard;
        exprt temp_eg = true_exprt();
        if(g_temp == true_exprt())
        {
         //s_in_else = true;
          if(else_sp.size() != 0)
          {
            sp = else_sp.top();
            else_sp.pop();
          }
          /*if(else_gaurd.size() != 0)
          {
            temp_eg = else_gaurd.top();
            else_gaurd.pop();
          }*/
         // sp = and_exprt(sp,temp_eg);
         // std::cout << "before else sp is " << expr2c(sp, ns) << std::endl;
          code_assignt c_assign = to_code_assign(c);
          exprt lhs = c_assign.lhs();
          exprt rhs = c_assign.rhs();
           if (lhs.type().id() == ID_unsignedbv) {
               has_unsigned_type = true;
              }
          recursive_replace(rhs, lhs, t[counter]);
          exprt temp1 = equal_exprt(lhs,rhs);
         // std::cout << "Equel exp is " << expr2c(temp1, ns) << std::endl;
          recursive_replace(sp, lhs, t[counter]);
          sp = and_exprt(temp1,sp);

          //t[counter].type() = symbol_typet();
          exprt there_exists(ID_exists);
          //there_exists.type() = bool_typet();
          there_exists.copy_to_operands(t[counter]);
          there_exists.copy_to_operands(sp);
          sp = there_exists;
          //std::cout << "sp in one step is " << expr2c(sp, ns)<<"\n" << std::endl;
        }
        
      //   std::cout << "setting last_wp to " << expr2c(wp,ns) << std::endl;
    }
    else
    {
      exprt temp_sp = sp;
      if(if_sp.size() != 0)
      {
        temp_sp = if_sp.top();
        if_sp.pop();
      }
      if(else_sp.size() != 0)
      {
        sp=else_sp.top();
        else_sp.pop();
      }
        sp = or_exprt(temp_sp,sp);
        code_assignt c_assign = to_code_assign(c);
        exprt lhs = c_assign.lhs();
        exprt rhs = c_assign.rhs();
         if (lhs.type().id() == ID_unsignedbv) {
           has_unsigned_type = true;
            }
        recursive_replace(rhs, lhs, t[counter]);
        exprt temp1 = equal_exprt(lhs,rhs);
      //  std::cout << "Equel exp is " << expr2c(temp1, ns) << std::endl;
        recursive_replace(sp, lhs, t[counter]);
        sp = and_exprt(temp1,sp);

        //t[counter].type() = symbol_typet();
        exprt there_exists(ID_exists);
        //there_exists.type() = bool_typet();
        there_exists.copy_to_operands(t[counter]);
        there_exists.copy_to_operands(sp);
        sp = there_exists;
        //std::cout << "sp in one step is " << expr2c(sp, ns)<<"\n" << std::endl;
    }
  }
  else{
          code_assignt c_assign = to_code_assign(c);
          exprt lhs = c_assign.lhs();
          exprt rhs = c_assign.rhs();
           if (lhs.type().id() == ID_unsignedbv) {
           has_unsigned_type = true;
          }
          recursive_replace(rhs, lhs, t[counter]);
          exprt temp1 = equal_exprt(lhs,rhs);
         // std::cout << "Equel exp is " << expr2c(temp1, ns) << std::endl;
          recursive_replace(sp, lhs, t[counter]);
          sp = and_exprt(temp1,sp);

          //t[counter].type() = symbol_typet();
          exprt there_exists(ID_exists);
          //there_exists.type() = bool_typet();
          there_exists.copy_to_operands(t[counter]);
          there_exists.copy_to_operands(sp);
          sp = there_exists;
         // std::cout << "sp in one step is " << expr2c(sp, ns) <<"\n"<< std::endl;
      }
    }

  }
    counter++;
    
    #ifdef DEBUG
    std::cout << "sp before simplification: " << expr2c(sp,ns) << std::endl;
    #endif
    simplify(sp, ns);
    #ifdef DEBUG
    std::cout << "sp after simplification: " << expr2c(sp,ns) << std::endl;
    #endif
  }

}


safety_checkert::resultt
check_wpt::operator()(const goto_functionst &goto_functions_in) 
{
  goto_functions = goto_functions_in;
  goto_functionst::function_mapt::const_iterator
    f_it = goto_functions.function_map.find("c::main");

  if (f_it == goto_functions.function_map.end()) {
    std::cout<<"c::main not found"<<std::endl;
    exit(11);
  }
  const goto_functionst::goto_functiont &main_function = f_it->second;

  if(!main_function.body_available){
    std::cout<<"c::main body not found"<<std::endl;
    exit(22);
  }

#ifdef DEBUG
  exprt wp = true_exprt();
  generate_wp(wp, (main_function.body).instructions, "c::main");
  //A program is error free iff wp(program, true) is valid.
  //So we will check for satisfiablity of negation of wp.
  exprt verification_condn = not_exprt(wp);  
#else
  exprt sp = true_exprt();
  generate_sp(sp, (main_function.body).instructions);
  exprt verification_condn = not_exprt(sp);
 #endif  
  z3_solver solver(ns);
  if (solver.solve(verification_condn, has_unsigned_type)) {
    //    std::cout << "Back to check_wp";
    return safety_checkert::UNSAFE;
  } else {
    //std::cout << "Back to check_wp";
    return safety_checkert::SAFE;
  }

} 
 
void
check_wpt::wp_assign(const codet& c, exprt& phi, ve_vectort& ve_vector)
{
  code_assignt c_assign = to_code_assign(c);
  exprt lhs = c_assign.lhs();
  exprt rhs = c_assign.rhs();

  if (lhs.type().id() == ID_unsignedbv) {
    has_unsigned_type = true;
  }
  
  recursive_replace(phi, lhs, rhs);

  //add the lhs replacement into a list incrementally; used for function summary
  bool found_lhs = false;
  for(ve_vectort::iterator ve_itr = ve_vector.begin();
      ve_itr != ve_vector.end();
      ++ve_itr) {
    if ((ve_itr->first).id() == ID_symbol &&
	to_symbol_expr(ve_itr->first).get_identifier() ==
	to_symbol_expr(lhs).get_identifier()) {
      found_lhs = true;
    }
    // std::cout << " expr " << expr2c(ve_itr->second,ns) << std::endl
    // 	      << " lhs " << expr2c(lhs, ns) << std::endl
    // 	      << " RHS " << expr2c(rhs, ns) << std::endl;
    recursive_replace(ve_itr->second, lhs, rhs);
    //std::cout << " expr after replace " << expr2c(ve_itr->second,ns) << std::endl;
  }
  if (!found_lhs) {
    ve_vector.push_back(ve_pairt(lhs, rhs));
  }
    
}

void
check_wpt::recursive_replace(exprt& phi, const exprt& lhs, const exprt& rhs) {

  // std::cout << " lhs " << expr2c(lhs,ns) << " rhs " << expr2c(rhs,ns) << std::endl;
  // std::cout << "phi is " << expr2c(phi,ns) << std::endl;
  
  Forall_operands(o_it, phi) {
    recursive_replace(*o_it, lhs, rhs);
  }

 #ifdef DEBUG
  // std::cout << "Displaying left operand information: " ;
  // std::cout << lhs.id() << "\t" << (lhs.operands()).size() << std::endl;
  #endif

  if (phi.id() == ID_symbol &&
      to_symbol_expr(phi).get_identifier() ==
      to_symbol_expr(lhs).get_identifier()) {
    phi = rhs;
  }
}

void
check_wpt::recursive_replace_once(exprt& phi, const ve_vectort& ve_vector) {

  // std::cout << " lhs " << expr2c(lhs,ns) << " rhs " << expr2c(rhs,ns) << std::endl;
  // std::cout << "phi is " << expr2c(phi,ns) << std::endl;
  
  Forall_operands(o_it, phi) {
    recursive_replace_once(*o_it, ve_vector);
  }

 #ifdef DEBUG
  // std::cout << "Displaying left operand information: " ;
  // std::cout << lhs.id() << "\t" << (lhs.operands()).size() << std::endl;
  #endif

  if (phi.id() == ID_symbol) {
    for (ve_vectort::const_iterator ve_itr = ve_vector.begin();
	 ve_itr != ve_vector.end();
	 ++ve_itr) {
      // std::cout << "phi is " << expr2c(phi,ns) << std::endl
      // 		<< expr2c(ve_itr->first, ns) << std::endl;
      if (to_symbol_expr(phi).get_identifier() ==
	  to_symbol_expr(ve_itr->first).get_identifier()) {
	phi = (ve_itr->second);
	break;
      }
    }
  }
}

