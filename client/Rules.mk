#------------------------------------
#Potentials
POTDIRS += ./potentials/EMT/
LIBS += ./potentials/EMT/libEMT.a
POTENTIALS += "+EMT "
POTDIRS += ./potentials/Morse/
LIBS += ./potentials/Morse/libMorse.a
POTENTIALS += "+MORSE "
POTDIRS += ./potentials/LennardJones/
LIBS += ./potentials/LennardJones/libLJ.a
POTENTIALS += "+LJ"
#POTDIRS += ./potentials/EAM/
#LIBS += ./potentials/EAM/libEAM.a
POTENTIALS += "-EAM"
POTDIRS += ./potentials/QSC/
LIBS += ./potentials/QSC/libQSC.a
POTENTIALS += "+QSC"
POTDIRS += ./potentials/Water/
LIBS += ./potentials/Water/libwater.a
POTENTIALS += "+H2O"
POTDIRS += ./potentials/Water_Pt/
LIBS += ./potentials/Water_Pt/libtip4p_pt.a
POTENTIALS += "+H2O_Pt"
POTDIRS += ./potentials/IMD/
LIBS += ./potentials/IMD/libIMD.a
POTENTIALS += "+IMD"
POTDIRS += ./potentials/bopfox
LIBS += ./potentials/bopfox/libbopfox.a
POTENTIALS += "+bopfox"

#Potentials relying on fortran
ifdef NO_FORTRAN
    POTENTIALS += "-Aluminum -Lenosky -SW -Tersoff -EDIP -H2O_H"
    OPOTDIRS += ./potentials/Aluminum/ ./potentials/Lenosky/ ./potentials/SW/ \
                ./potentials/Tersoff/ ./potentials/EDIP/ ./potentials/Water_H/
else
    POTENTIALS += "+Aluminum +Lenosky +SW +Tersoff +EDIP +H2O_H"
    FPOTDIRS += ./potentials/Aluminum/ ./potentials/Lenosky/ ./potentials/SW/ \
                ./potentials/Tersoff/ ./potentials/EDIP/ ./potentials/Water_H/
    LIBS += ./potentials/Aluminum/libAL.a ./potentials/Lenosky/libLenosky.a \
            ./potentials/SW/libSW.a ./potentials/Tersoff/libTersoff.a \
            ./potentials/EDIP/libEDIP.a ./potentials/Water_H/libtip4p_h.a
endif

#Optional potentials
ifdef BOPFOX
    CXXFLAGS += -DBOPFOX
    LDFLAGS += libbopfox.a -lgfortran
    POTDIRS += ./potentials/bop
    LIBS += ./potentials/bop/libbop.a
    POTENTIALS += "+bop"
else
    OPOTDIRS += ./potentials/bop
    POTENTIALS += "-bop"
endif

ifdef LAMMPS_POT
   CXXFLAGS += -DLAMMPS_POT
   POTDIRS += ./potentials/LAMMPS
   LIBS += ./potentials/LAMMPS/liblammps.a ./potentials/LAMMPS/liblmp_serial.a ./potentials/LAMMPS/libfakempi.a
   POTENTIALS += "+LAMMPS"
else
   OPOTDIRS += ./potentials/LAMMPS
   POTENTIALS += "-LAMMPS"
endif

ifdef NEW_POT
   CXXFLAGS += -DNEW_POT
   POTDIRS += ./potentials/NewPotential
   LIBS += ./potentials/NewPotential/libnewpotential.a
   POTENTIALS += "+NewPotential"
else
   OPOTDIRS += ./potentials/NewPotential
   POTENTIALS += "-NewPotential"
endif

ifndef WIN32
    POTDIRS += ./potentials/VASP
    LIBS += ./potentials/VASP/libVASP.a
    POTENTIALS += "+VASP"
else
    OPOTDIRS += ./potentials/VASP
    POTENTIALS += "-VASP"
endif

ifdef EONMPI
    POTENTIALS += "+MPIPot"
    POTDIRS += ./potentials/MPIPot
    LIBS += ./potentials/MPIPot/libMPIPot.a
else
    POTENTIALS += "-MPIPot"
    OPOTDIRS += ./potentials/MPIPot
endif

#------------------------------------
#MPI settings continued
ifdef EONMPI
    ifdef EONMPIBGP
        CXXFLAGS += -I/bgsys/drivers/ppcfloor/gnu-linux/include/python2.6/
        CXXFLAGS += -DEONMPIBGP
        LDFLAGS  += -L/bgsys/drivers/ppcfloor/gnu-linux/lib/ -Wl,-dy -lpython2.6 -lm
    else
        python_include_path=$(shell python -c "import sys,os; print os.path.join(sys.prefix, 'include', 'python'+sys.version[:3])")
        python_lib=$(shell python -c "import sys,os; print 'python'+sys.version[:3]")
        python_lib_path=$(shell python -c "import sys,os;print os.path.join(sys.prefix, 'lib')")
        ifneq ($(python_lib_path),/usr/lib)
            LDFLAGS += -L${python_lib_path} -l${python_lib} -lm
        endif
        LDFLAGS += -l${python_lib} -lm
        CXXFLAGS += -I${python_include_path}
    endif
endif

#------------------------------------
#client source code
OBJECTS += ClientEON.o INIFile.o MinModeSaddleSearch.o Dimer.o EpiCenters.o \
           Hessian.o ConjugateGradients.o HelperFunctions.o Matter.o \
           Parameters.o Potential.o Quickmin.o ProcessSearchJob.o PointJob.o \
           MinimizationJob.o HessianJob.o ParallelReplicaJob.o \
           ReplicaExchangeJob.o Dynamics.o BondBoost.o FiniteDifferenceJob.o \
           NudgedElasticBandJob.o TestJob.o BasinHoppingJob.o \
           SaddleSearchJob.o ImprovedDimer.o NudgedElasticBand.o Lanczos.o \
           Bundling.o Job.o CommandLine.o DynamicsJob.o Log.o \
           LBFGS.o LowestEigenmode.o Optimizer.o Prefactor.o \
           DynamicsSaddleSearch.o PrefactorJob.o

#------------------------------------
#Build rules
all: $(POTDIRS) $(FPOTDIRS) client
	@echo
	@echo "EON Client Compilation Options"
	@echo "BOINC: $(BOINC)" 
	@echo "DEBUG: $(DEBUG)"
	@echo "POTENTIALS: $(POTENTIALS)"

client: $(OBJECTS) $(LIBS)
	$(CXX) -o $(TARGET_NAME) $^ $(LDFLAGS)

libeon: $(filter-out ClientEON.o,$(OBJECTS)) $(POTDIRS) $(FPOTDIRS)
	$(AR) libeon.a $(filter-out ClientEON.o,$(OBJECTS)) potentials/*/*.o potentials/EMT/Asap/*.o

include Depend.mk

$(LIBS):
	$(MAKE) -C $@

$LIBS: $(POTDIRS) $(FPOTDIRS)

$(POTDIRS):
	$(MAKE) -C $@ CC="$(CC)" CXX="$(CXX)" LD="$(LD)" AR="$(AR)" RANLIB="$(RANLIB)"

$(FPOTDIRS):
	$(MAKE) -C $@ CC="$(CC)" CXX="$(CXX)" LD="$(LD)" AR="$(FAR)" FC="$(FC)" FFLAGS="$(FFLAGS)" RANLIB="$(RANLIB)"

clean:
	rm -f $(OBJECTS) client

clean-all: clean
	for pot in $(POTDIRS) $(FPOTDIRS) $(OPOTDIRS); do $(MAKE) -C $$pot clean ; done

.PHONY : all $(POTDIRS) $(FPOTDIRS) clean clean-all
# DO NOT DELETE