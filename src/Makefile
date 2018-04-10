CBMC = /home/uttaran/cbmc/src

SRC = abstract_interpreter_main.cpp \
	  abstract_interpreter.cpp \
	  interval_utils.cpp \
	  interval.cpp

OBJ += $(CBMC)/ansi-c/ansi-c$(LIBEXT) \
      $(CBMC)/cpp/cpp$(LIBEXT) \
      $(CBMC)/linking/linking$(LIBEXT) \
      $(CBMC)/big-int/big-int$(LIBEXT) \
      $(CBMC)/goto-programs/goto-programs$(LIBEXT) \
      $(CBMC)/util/util$(LIBEXT) \
      $(CBMC)/langapi/langapi$(LIBEXT) \
      $(CBMC)/analyses/analyses$(LIBEXT) \
      $(CBMC)/goto-instrument/goto_program2code$(OBJEXT)

INCLUDES = -I $(CBMC)/

LIBS =

include $(CBMC)/config.inc
include $(CBMC)/common

CLEANFILES = abstract_interpreter_main$(EXEEXT)

all: abstract_interpreter_main$(EXEEXT)

abstract_interpreter_main$(EXEEXT): $(OBJ)
	$(LINKBIN)
