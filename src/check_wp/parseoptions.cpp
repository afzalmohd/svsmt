#include <cstdlib>
#include <fstream>
#include <memory>
#include <iostream>

#include <util/config.h>
#include <util/expr_util.h>
#include <goto-programs/goto_convert_functions.h>
#include <analyses/goto_check.h>
#include <langapi/mode.h>
#include <ansi-c/ansi_c_language.h>

#include "parseoptions.h"
#include "check_wp.h"

parseoptionst::parseoptionst(int argc, const char **argv):
  parseoptions_baset("", argc, argv),
  language_uit("Weakest_Precondition", cmdline)
{
}

int parseoptionst::doit()
{

 if(config.set(cmdline))
  {
    usage_error();
    exit(1);
  }

  register_language(new_ansi_c_language);
  //register_language(new_cpp_language);

  goto_functionst goto_functions;

  if(get_goto_program(goto_functions)) {
    return 6;
  }
    
  const namespacet ns(symbol_table);

  check_wpt check_wp(ns, ui_message_handler);

  switch(check_wp(goto_functions))
  {
  case safety_checkert::SAFE:
     std::cout << "VERIFICATION SUCCESSFUL" << std::endl;
    return 0;
  
  case safety_checkert::UNSAFE:
    std::cout << "VERIFICATION FAILED" << std::endl;
    //std::cout << std::endl << "Counterexample:" << std::endl;
    //show_goto_trace(std::cout, ns, check_wp.error_trace);
    return 10;
  
  default:;
    return 8;
  }
}

bool parseoptionst::get_goto_program(goto_functionst &goto_functions)
{
  if(cmdline.args.size()==0)
  {
    error("Please provide a program to verify");
    return true;
  }

  //std::cout << "Testing: " << cmdline.args[0] << std::endl;
  
  try
  {
      if(parse()) return true;
      if(typecheck()) return true;
      if(final()) return true;

      clear_parse();

      if(symbol_table.symbols.find(ID_main)==symbol_table.symbols.end())
      {
        error("Please provide a main function");
        return true;
      }

      goto_convert(
        symbol_table, goto_functions, ui_message_handler);
  }

  catch(const char *e)
  {
    error(e);
    return true;
  }

  catch(const std::string e)
  {
    error(e);
    return true;
  }
  
  catch(int)
  {
    return true;
  }
  
  catch(std::bad_alloc)
  {
    error("Out of memory");
    return true;
  }
  
  return false;
}

  
