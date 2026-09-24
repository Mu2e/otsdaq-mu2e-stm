ifndef G4ROOT
 $(error - use setup.sh to define G4ROOT)
endif

INC += -I$(G4ROOT)/include/Geant4 -I$(CLROOT)/include
LIBS += -L$(G4ROOT)/lib64 -lG4global -lG4tracking -lG4run -lG4materials -lG4geometry -lG4particles -lG4processes -lG4event -lG4RayTracer -lG4digits_hits -lG4interfaces -lG4intercoms -lG4graphics_reps -lG4vis_management -lG4track -lG4analysis -lG4zlib -lG4ptl -lG4global -lG4modeling

# -lG4OpenGL
# -lG4Tasking removed on mu2edaq2
LIBS += -L$(CLROOT)/lib -lCLHEP

src := $(wildcard *.cc)

interim := $(subst .cc,.o,${src})

HOST := $(shell hostname)
ifeq ($(HOST),mu2edaq2.blackett.manchester.ac.uk)
 CPPFLAGS += -Wall -Wextra -pedantic -ggdb -Dmu2edaq2=1
else ifeq ($(HOST),mu2edaq1.blackett.manchester.ac.uk)
 CPPFLAGS += -Wall -Wextra -pedantic -ggdb -Dmu2edaq1=1
else
 $(error : CANNOT COMPILE GEANT4 on this machine)
 exit
endif

all: prog

geant4_main.o: geant4_main.cc GeEventAction.hh  GePhysicsList.hh  GePrimaryGeneratorAction.hh  GeRunAction.hh  GeSensitiveDetector.hh  GeSteppingAction.hh  GeUserDetectorConstruction.hh
	g++ ${CPPFLAGS} ${INC} $< -c -o $@

%.o: %.cc %.hh
	g++ ${CPPFLAGS} ${INC} $< -c -o $@


prog: ${interim} 
	g++ ${CPPFLAGS} ${LIBS} ${interim} -o geant4_main.exe

clean:
	echo ${interim}
	rm *.o
