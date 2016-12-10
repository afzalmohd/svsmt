#include <util/options.h>
#include <util/namespace.h>

#include <goto-programs/safety_checker.h>

//#include <goto-instrument/accelerate/accelerate.h>


class check_wpt:public safety_checkert
{
public:
  check_wpt(
    const namespacet &_ns,
    message_handlert &_message_handler):
    safety_checkert(_ns, _message_handler),
      ns(_ns),
      has_unsigned_type(false)
  {
  }
  
  virtual resultt operator()(
  const goto_functionst &goto_functions_in);

 private:
    //function summary
  typedef std::pair<exprt, exprt> ve_pairt;
  typedef std::vector<ve_pairt> ve_vectort;
  typedef std::pair<exprt, ve_vectort> wve_pairt;
  typedef std::map<irep_idt, wve_pairt> fve_mapt;

  void generate_wp(exprt& wp, goto_programt::instructionst, const irep_idt& fname);
  void generate_sp(exprt& sp, goto_programt::instructionst);
  void wp_assign(const codet& c, exprt& phi, ve_vectort&);
  void recursive_replace(exprt& phi,
			 const exprt& left,
			 const exprt& right);
  void recursive_replace_once(exprt& phi,
			      const ve_vectort& ve_vector);
  ve_vectort create_ite_ve_vector(ve_vectort nve,
				     ve_vectort ve,
				     const exprt& guard);

  namespacet ns;
  bool has_unsigned_type;
  goto_functionst goto_functions;

  fve_mapt fve_map;
};

