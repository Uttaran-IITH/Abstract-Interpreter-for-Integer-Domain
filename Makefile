SRC = backup.cpp

OBJ += /home/ubuntu/Desktop/cbmc/src/ansi-c/ansi-c$(LIBEXT) \
      /home/ubuntu/Desktop/cbmc/src/cpp/cpp$(LIBEXT) \
      /home/ubuntu/Desktop/cbmc/src/linking/linking$(LIBEXT) \
      /home/ubuntu/Desktop/cbmc/src/big-int/big-int$(LIBEXT) \
      /home/ubuntu/Desktop/cbmc/src/goto-programs/goto-programs$(LIBEXT) \
      /home/ubuntu/Desktop/cbmc/src/util/util$(LIBEXT) \
      /home/ubuntu/Desktop/cbmc/src/langapi/langapi$(LIBEXT) \
      /home/ubuntu/Desktop/cbmc/src/analyses/analyses$(LIBEXT) \
      /home/ubuntu/Desktop/cbmc/src/goto-instrument/goto_program2code$(OBJEXT)

INCLUDES = -I /home/ubuntu/Desktop/cbmc/src/

LIBS =

include /home/ubuntu/Desktop/cbmc/src/config.inc
include /home/ubuntu/Desktop/cbmc/src/common

CLEANFILES = abstract_interpreter_main$(EXEEXT)

all: abstract_interpreter_main$(EXEEXT)

abstract_interpreter_main$(EXEEXT): $(OBJ)
	$(LINKBIN)
