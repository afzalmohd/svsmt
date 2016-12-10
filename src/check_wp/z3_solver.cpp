#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>

#include <ansi-c/expr2c.h>
#include <util/namespace.h>
#include <util/expr.h>
#include <util/std_expr.h>
#include <util/symbol.h>


#include "z3_solver.h"


#define LT(x, y) (has_unsigned_type ? z3::ult(x,y) : (x < y))
#define LE(x, y) (has_unsigned_type ? z3::ule(x,y) : (x <= y))
#define GT(x, y) (has_unsigned_type ? z3::ugt(x,y) : (x > y))
#define GE(x, y) (has_unsigned_type ? z3::ule(x,y) : (x >= y))

bool
z3_solver::solve(exprt& expr, bool unsigned_type)
{
  has_unsigned_type = unsigned_type;
  std::cout << "sp expression after cbmc phase: " << expr2c(expr, ns) << std::endl;
    
  z3::expr final_expr = convert(expr);

  std::cout << "sp expression after converting to z3: " << final_expr << std::endl;
  // final_expr = final_expr.simplify();
  // std::cout << "wp expression after applying z3's simplification: " << final_expr << std::endl;

  return solve_underapprox(final_expr);
}

// Major part of this code is similar to the method convert_expr() present in
// cbmc-cbmc-4.5/src/solvers/smt2/smt2_conv.cpp. Modification is to generate
// z3 expressions instead of smt2 file output.
// Author of above mentioned file is Daniel Kroening, kroening@kroening.com

//"and", "or", "+" and "mul" can have more than 2 operands.
//A && B && C is considered as one "and" and 3 operands.
//side effect: stores variables that are found in the expr in var_map
z3::expr
z3_solver::convert(exprt& expr)
{
  if (expr.is_true()) {
    return c.bool_val(true);
    
  } else if (expr.is_false()) {
    return c.bool_val(false);
    
  } else if (expr.id() == ID_symbol) {
    irep_idt  id = to_symbol_expr(expr).get_identifier();
    var_mapt::iterator it = var_map.find(id);
    if (it != var_map.end()) {
      return it->second;
      
    } else {
      const symbolt *symbol;
      if (!ns.lookup(id, symbol)) {
	//this displays variable name from program which is not verbose
 	// std::cout << id2string(symbol->base_name) << std::endl;
	z3::expr var = c.bv_const((id2string(symbol->base_name)).c_str(), z3_solver::bv_size);
	var_map.insert(var_map_pairt(id, var));
	return var;
      } else {
	//verbose name
	z3::expr var = c.bv_const(id2string(id).c_str(), z3_solver::bv_size);
	var_map.insert(var_map_pairt(id, var));
	return var;
      }
    }
    
  } else if (expr.id() == ID_constant) {
    //only bitvector and bool constants are supported;
    //bool is handled above
    assert(expr.type().id() == ID_unsignedbv || expr.type().id() == ID_signedbv);
    std::string value = id2string(to_constant_expr(expr).get_value());
    //    std::cout << " stoi " << value << " expr " << expr2c(expr, ns) << std::endl;
    //Note that stoi doesn't work for -2
    return (c.bv_val((int)std::stol(value, nullptr, 2), z3_solver::bv_size));
    
  } else if (expr.id() == ID_and) {
    assert(expr.operands().size() >= 2);
    z3::expr tmp_z3expr = convert(expr.op0());
    exprt::operandst::iterator it = (expr).operands().begin();
    ++it;;
    while (it != (expr).operands().end()) {
      tmp_z3expr = tmp_z3expr && convert(*it);
      ++it;
    }
    return tmp_z3expr;
    
  } else if (expr.id() == ID_or) {
    assert(expr.operands().size() >= 2);
    z3::expr tmp_z3expr = convert(expr.op0());
    exprt::operandst::iterator it = (expr).operands().begin();
    ++it;
    while (it != (expr).operands().end()) {
      tmp_z3expr = tmp_z3expr || convert(*it);
      ++it;
    }
    return tmp_z3expr;
    
  } else if (expr.id() == ID_implies) {
    assert(expr.operands().size() == 2);
    return (implies(convert(expr.op0()), convert(expr.op1())));
    
  } else if (expr.id() == ID_not) {
    assert(expr.operands().size() == 1);
    return (!convert(expr.op0()));
    
  } else if (expr.id() == ID_notequal) {
    assert(expr.operands().size() == 2);
    return (convert(expr.op0()) != convert(expr.op1()));
    
  } else if (expr.id() == ID_equal) {
    assert(expr.operands().size() == 2);
    return (convert(expr.op0()) == convert(expr.op1()));
    
  } else if (expr.id() == ID_le) {
    assert(expr.operands().size() == 2);
    return (LE(convert(expr.op0()), convert(expr.op1())));

  } else if (expr.id() == ID_lt) {
    assert(expr.operands().size() == 2);
    return (LT(convert(expr.op0()), convert(expr.op1())));

  } else if (expr.id() == ID_ge) {
    assert(expr.operands().size() == 2);
    return (GE(convert(expr.op0()), convert(expr.op1())));

  } else if (expr.id() == ID_gt) {
    assert(expr.operands().size() == 2);
    return (GT(convert(expr.op0()), convert(expr.op1())));

  } else if (expr.id() == ID_plus) {
    assert(expr.operands().size() >= 2);
    z3::expr tmp_z3expr = convert(expr.op0());
    exprt::operandst::iterator it = (expr).operands().begin();
    ++it;
    while (it != (expr).operands().end()) {
      tmp_z3expr = tmp_z3expr + convert(*it);
      // std::cout << *it << std::endl;
      // tmp_z3expr = z3::expr(c, Z3_mk_bvadd_no_overflow(c, tmp_z3expr, convert(*it), !has_unsigned_type));
      ++it;
    }
    return tmp_z3expr;
  } else if (expr.id() == ID_minus) {
    assert(expr.operands().size() == 2);
    return (convert(expr.op0()) - convert(expr.op1()));

  } else if (expr.id() == ID_div) {
    assert(expr.operands().size() == 2);
    return (convert(expr.op0()) / convert(expr.op1()));

  } else if (expr.id() == ID_mult) {
    assert(expr.operands().size() >= 2);
    z3::expr tmp_z3expr = convert(expr.op0());
    exprt::operandst::iterator it = (expr).operands().begin();
    ++it;
    while (it != (expr).operands().end()) {
      tmp_z3expr = tmp_z3expr * convert(*it);
      ++it;
    }
    return tmp_z3expr;

  } else if (expr.id() == ID_typecast) {
    assert(expr.operands().size() == 1);
    //this is for the case when numbers are used as bool
    if (expr.type().id() == ID_bool) {
      return !(convert(expr.op0()) == c.bv_val(0, z3_solver::bv_size));
    }
    return convert(expr.op0());
    
  } else if (expr.id() == ID_unary_minus) {
    assert(expr.operands().size() == 1);
    return -convert(expr.op0());
    
  } else if (expr.id() == ID_bitnot) {
    assert(expr.operands().size() == 1);
    return ~convert(expr.op0());
    
  } else if (expr.id() == ID_bitand) {
    assert(expr.operands().size() >= 2);
    z3::expr tmp_z3expr = convert(expr.op0());
    exprt::operandst::iterator it = (expr).operands().begin();
    ++it;
    while (it != (expr).operands().end()) {
      tmp_z3expr = tmp_z3expr & convert(*it);
      ++it;
    }
    return tmp_z3expr;
    
  } else if (expr.id() == ID_bitor) {
    assert(expr.operands().size() >= 2);
    z3::expr tmp_z3expr = convert(expr.op0());
    exprt::operandst::iterator it = (expr).operands().begin();
    ++it;
    while (it != (expr).operands().end()) {
      tmp_z3expr = tmp_z3expr | convert(*it);
      ++it;
    }
    return tmp_z3expr;
    
  } else if (expr.id() == ID_bitxor) {
    assert(expr.operands().size() >= 2);
        z3::expr tmp_z3expr = convert(expr.op0());
    exprt::operandst::iterator it = (expr).operands().begin();
    ++it;
    while (it != (expr).operands().end()) {
      tmp_z3expr = tmp_z3expr ^ convert(*it);
      ++it;
    }
    return tmp_z3expr;
    
  } else if (expr.id() == ID_exists) {
    assert(expr.operands().size() == 2);
    z3::expr phi = exists(convert(expr.op0()), convert(expr.op1()));
   // z3::tactic qe = z3::try_for(z3::tactic(c,"qe"), 100);    
   // z3::goal g(c);
   // g.add(phi);
   // z3::expr res(c);
   // z3::apply_result result_of_elimination = qe.apply(g);
   // if (result_of_elimination.size() != 0) {
   //	z3::goal result_goal = result_of_elimination[0];
   // 	phi = result_goal.as_expr();
   // }
    
    //std::cout << "after QE " << phi << std::endl;
    return phi;
  } else if (expr.id() == ID_if) {
    assert(expr.operands().size() == 3);
    return (z3::ite(convert(expr.op0()), convert(expr.op1()), convert(expr.op2())));
  } else {
    std::cout << "unsupported expression: " << expr.id_string() << std::endl;
    exit(13);
  }

}

// bool
// z3_solver::need_assumption(const z3::expr& var)
// {
//   va_mapt::iterator va_itr = va_map.find(var);
//   assert(va_itr != va_map.end());
//   return (va_itr->second < z3_solver::max_steps);
// }

// std::string
// z3_solver::var_assumptions::get_assumption_name(const z3::expr& var)
// {
//   assert(need_assumption(var));
//   va_mapt::iterator va_itr = va_map.find(var);
//   assert(va_itr != va_map.end());
//   va_itr->second++;
//   //declaration name of the variable followed by step value
//   return (((va_itr->first).name()).str() + std::to_str(va_itr->second));
// }

// z3::expr
// z3_solver::var_assumptions::get_formula(const z3::expr& var, const z3::expr& assumption)
// {
//   return implies (assumption, 


//map.find() is not working hence iterate
z3::expr
z3_solver::assumption_to_variable(const z3::expr& a) {
  for (avmapt::iterator itr = avmap.begin(); itr != avmap.end(); ++itr) {
    if (z3::eq(itr->first, a)) {
      return itr->second;
    }
  }
  assert(0);
}

//map.find() is not working hence iterate
bool
z3_solver::require_assumption(const z3::expr& var) {
  for (vwmapt::iterator itr = vwmap.begin(); itr != vwmap.end(); ++itr) {
    if (z3::eq(itr->first, var)) {
      switch(st) {
      case LINEAR:
	return (z3_solver::bv_size > itr->second);
      case EXP:
	return (z3_solver::bv_size > itr->second);
      }
    }
  }
  return false;
}

void
z3_solver::add_variable(const z3::expr& var)
{
  switch(st) {
  case LINEAR:
    vwmap.insert(std::pair<z3::expr, unsigned int>(var, LINEAR_INIT_WIDTH));
    break;
  case EXP:
    vwmap.insert(std::pair<z3::expr, unsigned int>(var, EXP_INIT_WIDTH));
  }

}


z3::expr
z3_solver::get_next_assumption(const z3::expr& var)
{
  assert(require_assumption(var));
  vwmapt::iterator itr;
  for ( itr = vwmap.begin(); itr != vwmap.end(); ++itr) {
    if (z3::eq(itr->first, var)) {
      switch(st) {
      case LINEAR:
	itr->second = itr->second + LINEAR_STEP_SIZE;
	break;
      case EXP:
	itr->second = itr->second * EXP_STEP_SIZE;
	break;
      }
      break;
    }
  }
  //declaration name of var followed by width to make it unique
  std::string aname(Z3_ast_to_string(c,var));
  aname += std::to_string((itr->second));
  z3::expr assumption = c.bool_const(aname.c_str());
  avmap.insert(std::pair<z3::expr, z3::expr>(assumption, var));
  return assumption;
}

z3::expr
z3_solver::get_mask(const z3::expr& var)
{
  for(vwmapt::iterator itr = vwmap.begin(); itr != vwmap.end(); ++itr) {
    if (z3::eq(itr->first, var)) {
      return (c.bv_val(itr->second, z3_solver::bv_size));
    }
  }
  assert(0);
  
}
  
z3::expr
z3_solver::get_formula(const z3::expr& var, const z3::expr& a)
{
  //a -> (x[n]..x[width] == 0)
  //z3::expr all_one = to_expr(c, Z3_mk_repeat(c, z3_solver::bv_size, c.bv_val(1, 1)));
  //std::cout << "all ones" << all_one << std::endl;
  //all_one = to_expr(c, Z3_mk_bvlshr(c, all_one, get_mask(var)));
  //std::cout << "all ones" << all_one << std::endl;
  z3::expr all_one = to_expr(c, Z3_mk_bvlshr(c, ~(c.bv_val(0, z3_solver::bv_size)), get_mask(var)));
  //std::cout << "n-w ones " << all_one << std::endl;
  
  return (implies (a, (to_expr(c, Z3_mk_bvlshr(c, var, get_mask(var))) == all_one)));
}

void
z3_solver::cleanup_underapprox()
{
  avmap.clear();
  vwmap.clear();
}

#ifdef OLD
bool
z3_solver::solve_underapprox(z3::expr& expr)
{
 
  z3::solver s(c);

  // std::cout << s << std::endl;
  s.add(expr);
  switch (s.check()) {
  case z3::unsat :
    std::cout << s.statistics() << std::endl;
    return false;
  case z3::sat :
    std::cout << s.statistics() << std::endl;
    return true;
  case z3::unknown :
    std::cout << "Solver was not able to solve!";
    exit(14);
  default:
    return true;
  }
}   
#else
bool
z3_solver::solve_underapprox(z3::expr& phi)
{
  //using tactic let us try to elimante quantifiers
    /*z3::tactic qe = 
    	z3::tactic(c, "qe");
    z3::goal g(c);
    g.add(phi);
    z3::expr res(c);
    z3::apply_result result_of_elimination = qe.apply(g);
    z3::goal result_goal = result_of_elimination[0];
    phi = result_goal.as_expr();
    std::cout << "after qe " << phi << std::endl;
  	*/
  //  Z3_global_param_set("pp.decimal", "true"); //not working :(
  
  //to get the original formula in unsat core we need to define
  //a boolean called "answer literal" which will be used as
  //assumption and formula will be changed to ans_lit -> phi.
  z3::expr ans_lit = c.bool_const("ans_lit");
  z3::solver s(c);
         
  std::vector<z3::expr> assumptions; //keeps track of currently active assumptions
  //copy of above along with ans_lit. This list is passed to the solver.
  z3::expr_vector ev_assumptions(c); 
  ev_assumptions.push_back(ans_lit);
  s.add(implies(ans_lit, phi));

  for (var_mapt::iterator vitr = var_map.begin(); vitr != var_map.end(); ++vitr) {
    z3::expr x = vitr->second;
    add_variable(x);
    if (require_assumption(x)) {
      z3::expr a = get_next_assumption(x);
      z3::expr phi1 = get_formula(x,a);
#ifdef DEBUG
      std::cout << "Adding new formula " << phi1 << " with assumption " << a << std::endl;
#endif
      s.add(phi1);
      assumptions.push_back(a);
      ev_assumptions.push_back(a);
    }
  }

  bool done = false;
  while (!done) {
    done = true;
    //std::cout << "solving formula: " << s << std::endl;
    if (s.check(ev_assumptions) == z3::sat) {
      std::cout << "SAT! Model: " << std::endl;
      std::cout << s.get_model() << std::endl;
#ifdef DEBUG
      std::cout << s.statistics() << std::endl;
#endif
      cleanup_underapprox();
      return true; // TODO: change the return type
    }
    
    z3::expr_vector unsat_core = s.unsat_core();
    std::cout << "UNSAT core: " << unsat_core << std::endl;
    for (unsigned int i = 0; i < unsat_core.size(); i++) {
      //TODO: Use comparator and std::find_if
      // std::vector<z3::expr> aitr = std::find_if(assumptions.begin(),
      // 						assumptions.end(),
      // 						dummy_lt(unsat_core[i]));
      // if (aitr == assumptions.end()) {
      // 	continue;
      // }
      // done = false;
      // z3::expr v = assumption_to_variable(*aitr);
      // assumptions.erase(aitr);
      // if (require_assumption(v)) {
      // 	assumptions.push_back(get_next_assumption(v));
      // }

      unsigned int j;
      for (j = 0; j < assumptions.size(); j++) {
      	if (z3::eq(unsat_core[i], assumptions[j])) {
#ifdef DEBUG
	  std::cout << assumptions[j] << " is an assumption in unsat core" << std::endl;
#endif
      	    break;
      	  }
      }
      if (j == assumptions.size()) {
      	continue;
      }
      done = false;
      z3::expr v = assumption_to_variable(assumptions[j]);
      assumptions.erase(assumptions.begin()+j);
      if (require_assumption(v)) {
	z3::expr a = get_next_assumption(v);
	//	std::cout << "Adding assumption " << a << "to " << v << std::endl;
	z3::expr phi1 = get_formula(v, a);
	//	std::cout << "Adding new formula " << phi1 << " with assumption " << a << std::endl;
	s.add(phi1);
      	assumptions.push_back(a);
      }
    }
    if (done) {
      //no assumptions were found in unsat core so return happily
      std::cout << "No assumptions were found. Program is safe." << std::endl;
      #ifdef DEBUG
      std::cout << s.statistics() << std::endl;
      #endif
      cleanup_underapprox();
      return false;
    }   
    ev_assumptions.resize(0);//clear
    for (unsigned int i = 0; i < assumptions.size(); i++) {
      ev_assumptions.push_back(assumptions[i]);
    }
    ev_assumptions.push_back(ans_lit);
  }
  return false;
}
#endif
