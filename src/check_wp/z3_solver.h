#include "c++/z3++.h"

class z3_solver
{
public:
  //  enum underapprox_type { zero_ext, one_ext, sign_ext};
  
 z3_solver(const namespacet& in_ns)
   : ns(in_ns),
    has_unsigned_type(false)
      { ut = ZEXT; st = EXP; }

  bool solve(exprt& expr, bool has_unsigned_type);

  enum step_type {LINEAR, EXP};
  enum underapprox_type {ZEXT, OEXT, SEXT};

 private:

  /* class var_assumptions { */
  /* public:    */
  /*   var_asumptions(underapprox_type type =  z3_solver::zero_ext) { */
  /*     assert(type != zero_ext); */
  /*   } */
  /*   bool need_assumption(const z3::expr& var); */
  /*   void add_variable(const oz3::expr& var); */
  /*   z3::expr get_assumption(const z3::expr& var); */
  /*   z3::expr get_formula(const z3::expr& var, z3::expr& assumption); */
    
  /* private: */
  /*   typedef std::map<z3::expr, unsigned int> va_mapt; */
  /*   vassumptions_mapt va_map; */
  /* }; */
  
  z3::expr convert(exprt& expr);
  bool solve_underapprox(z3::expr& phi);

  z3::context c; 
  namespacet ns;
  typedef std::map<irep_idt, z3::expr> var_mapt;
  typedef std::pair<irep_idt, z3::expr> var_map_pairt;
  var_mapt var_map;
  static const unsigned int bv_size = 32;
  bool has_unsigned_type;

  //underapproximation related methods and data structure
  const unsigned int LINEAR_STEP_SIZE = 4;
  const unsigned int EXP_STEP_SIZE = 2;
  const unsigned int LINEAR_INIT_WIDTH = 0; //we will go one step ahead
  const unsigned int EXP_INIT_WIDTH = 2; //we will go one step ahead
  
  bool assumption_exists(const z3::expr& a);
  z3::expr assumption_to_variable(const z3::expr& a);
  bool require_assumption(const z3::expr& var);
  z3::expr get_next_assumption(const z3::expr& var);
  z3::expr get_formula(const z3::expr& var, const z3::expr& a);
  void add_variable(const z3::expr& var);
  z3::expr get_mask(const z3::expr& var);
  void cleanup_underapprox();

  step_type st;
  underapprox_type ut;
  struct dummy_lt {
    bool operator () (const z3::expr& a, const z3::expr& b) const {
      return true;
    }
  };  
  typedef std::map<z3::expr, z3::expr, dummy_lt> avmapt;
  avmapt avmap;
  typedef std::map<z3::expr, unsigned int, dummy_lt> vwmapt;
  vwmapt vwmap;
  
};




