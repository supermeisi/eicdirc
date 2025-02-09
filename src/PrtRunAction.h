#ifndef PrtRunAction_h
#define PrtRunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"

class G4Timer;
class G4Run;

class PrtRunAction : public G4UserRunAction {
public:
  PrtRunAction();
  virtual ~PrtRunAction();

public:
  virtual void BeginOfRunAction(const G4Run *aRun);
  virtual void EndOfRunAction(const G4Run *aRun);

private:
  G4Timer *fTimer;
  G4String fOutFile;
};

#endif
