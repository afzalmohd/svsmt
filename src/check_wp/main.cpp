#include "parseoptions.h"

int main(int argc, const char **argv)
{
  parseoptionst parseoptions(argc, argv);
  return parseoptions.main();
}
