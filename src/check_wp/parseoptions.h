#include <util/ui_message.h>
#include <util/parseoptions.h>
#include <util/options.h>

#include <goto-programs/goto_functions.h>
#include <goto-programs/safety_checker.h>
#include <langapi/language_ui.h>


class parseoptionst:
  public parseoptions_baset,
  public language_uit
{
public:
  virtual int doit();

  parseoptionst(int argc, const char **argv);
  virtual ~parseoptionst(){}

protected:

  virtual bool get_goto_program(
    goto_functionst &goto_functions);
};
