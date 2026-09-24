if [ "$HOSTNAME" = "mu2edaq1.hep.manchester.ac.uk" ]
then
    export LD_LIBRARY_PATH=/work/geant4/4.10.07.p03/lib64:/work/clhep/2.4.4.0/lib/
    export G4INSTALL=/work/geant4/4.10.07.p03/
    export G4NEUTRONHPDATA=$G4INSTALL/data/G4NDL4.6
    export G4LEDATA=$G4INSTALL/data/G4EMLOW7.13
    export G4LEVELGAMMADATA=$G4INSTALL/data/G4PhotonEvaporation5.7
    export G4RADIOACTIVEDATA=$G4INSTALL/data/G4RadioactiveDecay5.6
    export G4PARTICLEXSDATA=$G4INSTALL/data/G4PARTICLEXS3.1.1
    export G4PIIDATA=$G4INSTALL/data/G4PII1.3
    export G4REALSURFACEDATA=$G4INSTALL/data/G4RealSurface2.2
    export G4SAIDXSDATA=$G4INSTALL/data/G4SAIDDATA2.0
    export G4ABLADATA=$G4INSTALL/data/G4ABLA3.1
    export G4INCLDATA=$G4INSTALL/data/G4INCL1.0
    export G4ENSDFSTATEDATA=$G4INSTALL/data/G4ENSDFSTATE2.3
    export G4ROOT=/work/geant4/4.10.07.p03/ # for Makefile
    export CLROOT=/work/clhep/2.4.4.0
elif [ "$HOSTNAME" = "mu2edaq2.blackett.manchester.ac.uk" ]
then
    export LD_LIBRARY_PATH=/work/mu2e/geant4/4-v11.2.1/lib64:/work/mu2e/clhep/2.4.7.1/lib/
    export G4INSTALL=/work/mu2e/geant4/
    export G4NEUTRONHPDATA=$G4INSTALL/data/G4NDL4.7
    export G4LEDATA=$G4INSTALL/data/G4EMLOW8.5
    export G4LEVELGAMMADATA=$G4INSTALL/data/G4PhotonEvaporation5.7
    export G4RADIOACTIVEDATA=$G4INSTALL/data/G4RadioactiveDecay5.6
    export G4PARTICLEXSDATA=$G4INSTALL/data/G4PARTICLEXS4.0
    export G4PIIDATA=$G4INSTALL/data/G4PII1.3
    export G4REALSURFACEDATA=$G4INSTALL/data/G4RealSurface2.2
    export G4SAIDXSDATA=$G4INSTALL/data/G4SAIDDATA2.0
    export G4ABLADATA=$G4INSTALL/data/G4ABLA3.3
    export G4INCLDATA=$G4INSTALL/data/G4INCL1.2
    export G4ENSDFSTATEDATA=$G4INSTALL/data/G4ENSDFSTATE2.3
    export G4ROOT=/work/mu2e/geant4/4-v11.2.1/ # for Makefile
    export CLROOT=/work/mu2e/clhep/2.4.7.1
fi
export G4DIR=$G4INSTALL/lib64




