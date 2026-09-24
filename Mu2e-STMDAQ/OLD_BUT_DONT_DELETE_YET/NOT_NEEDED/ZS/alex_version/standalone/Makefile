WARNING = -Wall -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-unused-value -fPIC -Wno-deprecated

#CC_OPTS = -O3 -DUSE_SQL
#CC_OPTS = -O3 
#CC_OPTS = -Wall -Wno-unused-variable -ggdb -g3 -DUSE_SQL -DUSE_GRAPHICS # turn on debugging information and graphics
#CC_OPTS = -Wall -Wno-unused-variable -ggdb -g3 -DUSE_SQL # turn on debugging information and no graphics 

#Use this
#CC_OPTS = -Wall -Wno-unused-variable -O3 -DUSE_SQL # turn on code optimisation and database access
CC_OPTS = -Wall -Wno-unused-variable -O3 -DUSE_SQL -DPRINT_COUT # turn on code optimisation, database access and couts
DEPOPTS = -MT $@ -MMD -MP -MF $*.d

MAIN_CC = ZS_alex.cc 
MAIN_OBJ = $(patsubst %.cc, %.o, $(MAIN_CC))
MAIN_EXE = $(patsubst %.cc, %.exe, $(MAIN_CC))
DEPFILES = $(patsubst %.cc, %.d, $(MAIN_CC))


.PHONY: all clean

all: $(MAIN_EXE)

clean:
	rm $(MAIN_OBJ) $(DEPFILES)

%.o: %.cc Makefile %.d | 
	echo "<**Compiling**>" $<
	g++ $(CC_OPTS) $(WARNING) $(DEPOPTS) $(INCLUDE_CC) -c $< -o  $@

%.exe: %.o
	echo "<**Linking**>"  $(notdir $@)
	g++ $(CC_OPTS) $^ -o $@ 

$(DEPFILES):
include $(wildcard $(DEPFILES))

