include ../config.inc
CBMC ?= /home/sumanth/cmi/invariants/interpolants/cbmc-cbmc-4.5

SRC = main.cpp parseoptions.cpp check_wp.cpp z3_solver.cpp

OBJ+= $(CBMC)/src/ansi-c/ansi-c$(LIBEXT) \
      $(CBMC)/src/cpp/cpp$(LIBEXT) \
      $(CBMC)/src/linking/linking$(LIBEXT) \
      $(CBMC)/src/assembler/assembler$(LIBEXT) \
      $(CBMC)/src/big-int/big-int$(LIBEXT) \
      $(CBMC)/src/goto-programs/goto-programs$(LIBEXT) \
      $(CBMC)/src/goto-symex/goto-symex$(LIBEXT) \
      $(CBMC)/src/analyses/analyses$(LIBEXT) \
      $(CBMC)/src/pointer-analysis/pointer-analysis$(LIBEXT) \
      $(CBMC)/src/langapi/langapi$(LIBEXT) \
      $(CBMC)/src/util/util$(LIBEXT) \
      $(CBMC)/src/solvers/solvers$(LIBEXT)

INCLUDES= -I $(CBMC)/src -I $(Z3)/src/api

LIBS = -lz3

#LDFLAGS = "-Wl,-rpath,/usr/local/lib"


include $(CBMC)/src/config.inc
include $(CBMC)/src/common


CLEANFILES = check_wp$(EXEEXT)

all: check_wp$(EXEEXT)

debug:  DEBUG = -DDEBUG
debug: check_wp$(EXEEXT)

#c++11 is required for stoi
CP_CXXFLAGS += -g $(DEBUG)  -ggdb -std=c++11

#-rpath to avoid setting LD_LIBRARY_PATH
LINKFLAGS += "-Wl,-rpath,$(Z3)/build" -L$(Z3)/build

ifneq ($(wildcard $(CBMC)/src/java/Makefile),)
  OBJ += $(CBMC)/src/java/java$(LIBEXT)
  CP_CXXFLAGS += -DHAVE_JAVA
endif

ifneq ($(wildcard $(CBMC)/src/specc/Makefile),)
  OBJ += $(CBMC)/src/specc/specc$(LIBEXT)
  CP_CXXFLAGS += -DHAVE_SPECC
endif

ifneq ($(wildcard $(CBMC)/src/php/Makefile),)
  OBJ += $(CBMC)/src/php/php$(LIBEXT)
  CP_CXXFLAGS += -DHAVE_PHP
endif

###############################################################################

check_wp$(EXEEXT): $(OBJ)
	$(LINKBIN)

