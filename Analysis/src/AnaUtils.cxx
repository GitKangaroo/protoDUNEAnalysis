#include "../include/AnaUtils.h"

int AnaUtils::GetParticleType(const int pdg)
{
  int type = -999;
  if(pdg==2212){
    type = gkProton;
  }
  else if(pdg==211){
    type = gkPiPlus;
  }
  else if(pdg==-211){
    type = gkPiMinus;
  }
  else if(pdg==111){
    type = gkPiZero;
  }
  else if(pdg==-11){
    type = gkPositron;
  }
  else if(pdg==11){
    type = gkElectron;
  }
  else if(pdg==-13){
    type = gkMuPlus;
  }
  else if(pdg==13){
    type = gkMuMinus;
  }
  else if(pdg==321||pdg==-321||pdg==310||pdg==130){
    type = gkKaon;
  }
 else if(pdg==2112){
    type = gkNeutron;
  }
  else if(pdg==22){
    type = gkGamma;
  }
  else if(pdg==14 || pdg==-14 || pdg==12 || pdg==-12){
    type = gkNeutrino;
  }
  else if(pdg==3122||pdg==3212||pdg==3222){
    type = gkHyperon;
  }
  else if(pdg>9999){
    type = gkNucleus;
  }
  else if(pdg==-1){
    type = gkNoTruth;
  }
  else{
    cout<<"AnaUtils::GetParticleType unknown pdg "<<pdg<<endl; exit(1);
  }

  return type;
}

int AnaUtils::GetFillEventType()
{
  int filleventtype = -999;
  if(AnaIO::Signal){
    filleventtype = gkSignal;
  }
  else if(AnaIO::true_beam_PDG==211){
    filleventtype = gkEvtBkg;
  }
  else{
    filleventtype = gkBmBkg;
  }

  return filleventtype;
}

void AnaUtils::SetFullSignal()
{
  //Phase space cut on protons 
  //0.45 GeV/c is the reconstruction threshold, 1 GeV/c is limit where momentum by range is reliable)
  //Leading proton momentum 0.45 - 1 GeV/c
  //Subleading proton momentum < 0.45 GeV/c
  //No cuts on pions

  AnaIO::Signal = false;
  vector<TLorentzVector> vecFSParticle = GetFSParticlesTruth();
  
  double LeadingPiZeroP = vecFSParticle[0].P();
  double LeadingProtonP = vecFSParticle[1].P();
  double SubLeadingProtonP = vecFSParticle[2].P();
  // Check event topology 
  // Initial pion beam and at least one proton and one pizero, no other mesons in final state (but not consider number of neutrons)
  if( AnaIO::true_beam_PDG==211 && IsSignal(nProton,nPiZero,nPiPlus,nParticleBkg) == true){
    // Proton momentum selection (below 0.45 GeV/c is not detectbale)
    if(LeadingProtonP < 1 && LeadingProtonP > 0.45 && SubLeadingProtonP < 0.45){
      // No restrictions on pizero momentum
      if(LeadingPiZeroP > 0) AnaIO::Signal = true; 
    }
  }
}

vector<TLorentzVector> AnaUtils::GetFSParticlesTruth()
{
  // Get true beam daughter information
  const vector<int> * pdg = AnaIO::true_beam_daughter_PDG;
  const vector<double> * px = AnaIO::true_beam_daughter_startPx; // MeV/c
  const vector<double> * py = AnaIO::true_beam_daughter_startPy; // MeV/c
  const vector<double> * pz = AnaIO::true_beam_daughter_startPz; // MeV/c
  // Class member variables
  nProton = 0;
  nNeutron = 0;
  nPiPlus = 0;
  nPiZero = 0;
  nGamma = 0;
  nParticleBkg = 0;
  TLorentzVector pPiplus, pPiZero, pProton, pSecondaryProton;

  // Get the size of final state particles
  const int np = pdg->size();
  // Fill the hTruthFSParticleNumber hitogram
  AnaIO::hTruthFSParticleNumber->Fill(np);

  //int ProtonIdx = -999;
  //int ProtonIndices[np];
  double Protonmom[np];
  double PiZeromom[np];
  double Gammamom[np];
  vector<TVector3> bufferProtonmom;
  vector<TVector3> bufferPiZeromom;
  vector<int> bufferType;

  // Now loop over FS particles
  for(int ii=0; ii<np; ii++){
    // Get the FS particle type
    const int itype = GetParticleType((*pdg)[ii]);
    // Get the FS particle 3-momentum
    const TVector3 tmpp( (*px)[ii], (*py)[ii], (*pz)[ii] );
    AnaIO::hTruthFSParticleType->Fill(itype);  
    // Check each FS type and save info
    // Proton
    if(itype == gkProton){
      // ii is the location index of the FS particle's vector
      //ProtonIndices[nProton]=ii;
      // Store particle's momentum magnitude
      Protonmom[nProton] = tmpp.Mag();
      // Store momentum vector
      bufferProtonmom.push_back(tmpp);
      // Increase proton number 
      nProton++; 
    } 
    // PiZero
    else if(itype == gkPiZero){
      PiZeromom[nPiZero] = tmpp.Mag();
      bufferPiZeromom.push_back(tmpp);
      nPiZero++;
    }
    // Gamma
    else if(itype == gkGamma){
      Gammamom[nGamma] = tmpp.Mag();
      nGamma++;
    }
    // PiPlus
    else if(itype == gkPiPlus){
      nPiPlus++;
    }
    // Neutron
    else if(itype == gkNeutron){
      nNeutron++;
    }
    // PiMinus and Kaon
    else if(itype==gkPiMinus||itype==gkKaon){
      nParticleBkg++;
    }
    // Store FS particle type info
    bufferType.push_back(itype);

  } // End of loop
  
  //=======================Proton=======================
  int leadingProtonID = 0, subldProtonID = -999;
  if(nProton>1){
    int Protonsortid[nProton];
    // Sort index according to it's momentum
    TMath::Sort(nProton, Protonmom, Protonsortid);
    // Save 
    leadingProtonID = Protonsortid[0];
    subldProtonID = Protonsortid[1];
  }
  if(nProton>0){
    pProton.SetVectM(bufferProtonmom[leadingProtonID], AnaFunctions::ProtonMass());
    //ProtonIdx = ProtonIndices[leadingProtonID];
    AnaIO::hTruthLeadingProtonP->Fill(Protonmom[leadingProtonID]);
  }
  if(nProton>1){
    pSecondaryProton.SetVectM(bufferProtonmom[subldProtonID], AnaFunctions::ProtonMass());
    AnaIO::hTruthSubLeadingProtonP->Fill(Protonmom[subldProtonID]);
  }
  //========================PiZero========================
  int leadingPiZeroID = 0;
  if(nPiZero>1){
    // Fill histogram for FS multi pi0 number
    AnaIO::hTruthFSMultiPi0->Fill(nPiZero);
    int PiZerosortid[nPiZero];
    TMath::Sort(nPiZero, PiZeromom, PiZerosortid);
    leadingPiZeroID = PiZerosortid[0];
  }
  if(nPiZero>0){
    // Fill histogram for FS pi0 number
    AnaIO::hTruthFSPi0Number->Fill(nPiZero);
    pPiZero.SetVectM(bufferPiZeromom[leadingPiZeroID], AnaFunctions::PiZeroMass());
  }
  //========================Gamma========================
  int leadingGammaID = 0;
  if(nGamma>1){
    int Gammasortid[nGamma];
    TMath::Sort(nGamma, Gammamom, Gammasortid);
    leadingGammaID = Gammasortid[0];
  }
  if(nGamma>0) AnaIO::hTruthGammaMaxE->Fill(Gammamom[leadingGammaID]);
  // Fill vector of FS particles 
  vector<TLorentzVector> vec;
  vec.push_back(pPiZero);
  vec.push_back(pProton);
  vec.push_back(pSecondaryProton);
  return vec;
}

bool AnaUtils::IsSignal(const int nProton, const int nPiZero, const int nPiPlus, const int nParticleBkg)
{
  bool tmpSig = false;
  if(nProton > 0 && nPiZero > 0 && nPiPlus == 0 && nParticleBkg == 0) tmpSig = true;
  return tmpSig;
} 

TVector3 AnaUtils::GetRecBeamFull(){

  TVector3 beamdir;
  // Set beam end direction
  beamdir.SetXYZ(AnaIO::reco_beam_trackEndDirX, 
                 AnaIO::reco_beam_trackEndDirY, 
                 AnaIO::reco_beam_trackEndDirZ );

  // Get the beam end kinetic energy
  double ke = AnaIO::reco_beam_interactingEnergy/1E3;
  if(ke<0) ke = 1E-10;
  // Get pion mass since signal is pion beam
  const double mpi = AnaFunctions::PionMass();
  // Calculate pion beam end momentum
  const double pionEndP = TMath::Sqrt(ke*ke+2*ke*mpi);
  // Get momentum 3 vector
  const TVector3 fullbeam = beamdir.Unit()*pionEndP;
  return fullbeam;
}

TVector3 AnaUtils::GetTruthBeamFull()
{
  const TVector3 tmpbeam(AnaIO::true_beam_endPx,
                         AnaIO::true_beam_endPy,
                         AnaIO::true_beam_endPz );
  return tmpbeam;
}

void AnaUtils::FillBeamKinematics(const int kMC)
{
  // Get event type
  const int evtType = GetFillEventType();
  
  const TVector3 recBeamFull = GetRecBeamFull();
  if(kMC){
    const TVector3 truthBeamFull = GetTruthBeamFull();

    const double beamthetaRes    = (recBeamFull.Theta()-truthBeamFull.Theta())*TMath::RadToDeg();//use absolute difference 
    const double beammomentumRes = recBeamFull.Mag()/truthBeamFull.Mag()-1;

    plotUtils.FillHist(AnaIO::hBeamThetaRes,    truthBeamFull.Theta()*TMath::RadToDeg(), beamthetaRes);
    plotUtils.FillHist(AnaIO::hBeamMomentumRes, truthBeamFull.Mag(),                     beammomentumRes);
  }

  plotUtils.FillHist(AnaIO::hRecBeamTheta,    recBeamFull.Theta()*TMath::RadToDeg(), evtType);
  plotUtils.FillHist(AnaIO::hRecBeamMomentum, recBeamFull.Mag(),                     evtType);
}

vector<double> AnaUtils::GetdEdxVector(const vector<double> &arraydEdx, const bool kForward)
{
  // Get the size of this raw dEdx vector
  const unsigned int ncls = arraydEdx.size();

  vector<double> dEdxVec;
  // Start from [2] because [0] and [1] in both start and last are weird
  for(unsigned int kk=2; kk<ncls; kk++){
    // Fill the vector start from beginning of arraydEdx
    if(kForward) dEdxVec.push_back(arraydEdx[kk]);
    // Fill it backwards
    else{
     const double endpe = arraydEdx[ncls-1-kk];
     dEdxVec.push_back(endpe);
    }
  }
  return dEdxVec;
}

double AnaUtils::GetTruncatedMean(const vector<double> & dEdxVec, const unsigned int nsample0, const unsigned int nsample1, const double lowerFrac, const double upperFrac)
{
  // Require nsample0 < nsample1 < dEdxVec size
  if(nsample1>=dEdxVec.size() || nsample0>=nsample1) return -999;
  
  vector<double> dEdxVecTruncated;
  // Get the truncated dEdx vector specified by nsample0 and nsample1
  for(unsigned int ii=nsample0; ii<=nsample1; ii++){
    dEdxVecTruncated.push_back(dEdxVec[ii]);
  }
  // Get the upper and lower bound  
  const int iter0 = dEdxVecTruncated.size()*lowerFrac;
  const int iter1 = dEdxVecTruncated.size()*upperFrac;
  const int nterm = iter1-iter0;
  if( nterm<=0 ) return -999;
  // Sort this truncated dEdx vector 
  std::sort(dEdxVecTruncated.begin(), dEdxVecTruncated.end());
  double sum = 0.0;
  // Calcuate the mean value dEdx specified by lowerFrac and upperFrac
  for(int ii=iter0; ii< iter1; ii++){
    sum += dEdxVecTruncated[ii];
  }
  return sum / (nterm+1E-10);
}

double AnaUtils::GetFSParticleTME(const unsigned int ii, const bool kForward)
{
  // Get the raw dEdx vector
  const vector<double> recodEdxarray = (*AnaIO::reco_daughter_allTrack_calibrated_dEdX_SCE)[ii];
  // Get the meaningful dEdx vector
  vector<double> dEdxVec = GetdEdxVector(recodEdxarray,kForward);
  // Get the size of dEdxVec 
  const int ndEdx = dEdxVec.size();
  double TME = -999;
  
  if(kForward) TME = GetTruncatedMean(dEdxVec, 2, 6, 0.4,  0.95); 
  else TME  = GetTruncatedMean(dEdxVec, 2, ndEdx-8, 0.05, 0.6);

  return TME;
}

double AnaUtils::GetChi2NDF(const int ii)
{
  const double chi2       = (*AnaIO::reco_daughter_allTrack_Chi2_proton)[ii];
  const double ndof       = (*AnaIO::reco_daughter_allTrack_Chi2_ndof)[ii];

  const double chnf = chi2/(ndof+1E-10);

  return chnf;
}

TVector3 AnaUtils::GetTruthMatchedTrackVectLab(const int ii)
{
  const TVector3 trackVectLab((*AnaIO::reco_daughter_PFP_true_byHits_startPx)[ii],
                              (*AnaIO::reco_daughter_PFP_true_byHits_startPy)[ii],
                              (*AnaIO::reco_daughter_PFP_true_byHits_startPz)[ii] );
  return trackVectLab;
}

TVector3 AnaUtils::GetRecTrackVectLab(const int ii, const bool kProton)
{ 
  // MBR: momentum by range
  const double trackMBR = kProton? (*AnaIO::reco_daughter_allTrack_momByRange_proton)[ii] : (*AnaIO::reco_daughter_allTrack_momByRange_muon)[ii]; 
  TVector3 trackVectLab;
  // Get this reco particle momentum vector in lab frame
  trackVectLab.SetMagThetaPhi(trackMBR, (*AnaIO::reco_daughter_allTrack_Theta)[ii], (*AnaIO::reco_daughter_allTrack_Phi)[ii]);
  
  return trackVectLab;
}

TLorentzVector AnaUtils::GetMomentumRefBeam(const bool isTruth, const int recIndex, const bool kProton)
{
  // Get true/reco beam momentum vector
  const TVector3 tmpBeam = isTruth ? GetTruthBeamFull() : GetRecBeamFull();
  // Get true/reco FS particle momentum vector
  const TVector3 vecLab = isTruth ? GetTruthMatchedTrackVectLab(recIndex) : GetRecTrackVectLab(recIndex, kProton);
  // Get theta angle reletive to the beam
  const double thetaRefBeam = AnaFunctions::GetThetaRef(vecLab, tmpBeam.Unit());

  TVector3 vectRefBeam;
  vectRefBeam.SetMagThetaPhi(vecLab.Mag(), thetaRefBeam, 0);

  TLorentzVector momentumRefBeam;
  momentumRefBeam.SetVectM(vectRefBeam, kProton? AnaFunctions::ProtonMass() : AnaFunctions::PionMass() );

  return momentumRefBeam;
}

void AnaUtils::FillFSParticleKinematics(const int recIndex, const int truthParticleType, const int recParticleType)
{
  // ---------------- Fill proton kinematics ----------------//
  if(recParticleType == gkProton){
    // Get this reco particle momentum vector relative to beam
    const TLorentzVector recMomRefBeam = GetMomentumRefBeam(false /*=>reco*/, recIndex, true /*=>proton*/);
    plotUtils.FillHist(AnaIO::hRecProtonMomentum,recMomRefBeam.P(), truthParticleType);
    plotUtils.FillHist(AnaIO::hRecProtonTheta, recMomRefBeam.Theta()*TMath::RadToDeg(), truthParticleType); 
    // Truth-matching
    if(truthParticleType == gkProton){ // MC only (Data loop won't pass this)
      // Get the truth-matched particle truth momentum vector relative to beam
      const TLorentzVector truthMomRefBeam = GetMomentumRefBeam(true /*=>truth-matched*/, recIndex, true /*=>proton*/);   
      const double momentumRes = recMomRefBeam.P()/truthMomRefBeam.P()-1;
      const double thetaRes    = (recMomRefBeam.Theta()-truthMomRefBeam.Theta())*TMath::RadToDeg();
      plotUtils.FillHist(AnaIO::hProtonMomentumRes, truthMomRefBeam.P(), momentumRes);
      plotUtils.FillHist(AnaIO::hProtonThetaRes, truthMomRefBeam.Theta()*TMath::RadToDeg(), thetaRes); 
    }
  }
  // ---------------- Fill piplus kinematics ----------------//
  if(recParticleType == gkPiPlus){
    // Get this reco particle momentum vector relative to beam
    const TLorentzVector recMomRefBeam = GetMomentumRefBeam(false /*=>reco*/, recIndex, false/*=>piplus*/);
    plotUtils.FillHist(AnaIO::hRecPiPlusMomentum,recMomRefBeam.P(), truthParticleType);
    plotUtils.FillHist(AnaIO::hRecPiPlusTheta, recMomRefBeam.Theta()*TMath::RadToDeg(), truthParticleType);
    // Truth-matching
    if(truthParticleType == gkPiPlus){ // MC only (Data loop won't pass this)
      // Get the truth-matched particle truth momentum vector relative to beam
      const TLorentzVector truthMomRefBeam = GetMomentumRefBeam(true /*=>truth-matched*/, recIndex, false /*=>piplus*/);
      const double momentumRes = recMomRefBeam.P()/truthMomRefBeam.P()-1;
      const double thetaRes    = (recMomRefBeam.Theta()-truthMomRefBeam.Theta())*TMath::RadToDeg();
      plotUtils.FillHist(AnaIO::hPiPlusMomentumRes, truthMomRefBeam.P(), momentumRes);
      plotUtils.FillHist(AnaIO::hPiPlusThetaRes, truthMomRefBeam.Theta()*TMath::RadToDeg(), thetaRes);
    }  
  }
  // ---------------- Fill shower kinematics ----------------//
  if(recParticleType == gkGamma){
    // Get this reco particle momentum vector relative to beam
    const TLorentzVector recMomLab = GetRecShowerVectLab(recIndex); 
    plotUtils.FillHist(AnaIO::hRecShowerEnergy, recMomLab.E(), truthParticleType);
    // Truth-matching
    if(truthParticleType == gkGamma){ // MC only (Data loop won't pass this)
    
    }
  }
}

TVector3 AnaUtils::GetRecShowerDistVector(const int ii)
{
  // Get reco beam end point
  const TVector3 vtx(AnaIO::reco_beam_endX, AnaIO::reco_beam_endY, AnaIO::reco_beam_endZ);
  // Get reco shower start position
  const TVector3 shw((*AnaIO::reco_daughter_allShower_startX)[ii], (*AnaIO::reco_daughter_allShower_startY)[ii], (*AnaIO::reco_daughter_allShower_startZ)[ii]);
  // Calculate distance vector
  const TVector3 dist=shw-vtx;
  return dist;
}

TLorentzVector AnaUtils::GetRecShowerVectLab(const int ii)
{
  // Get reco shower direction
  const TVector3 showerDir((*AnaIO::reco_daughter_allShower_dirX)[ii],(*AnaIO::reco_daughter_allShower_dirY)[ii],(*AnaIO::reco_daughter_allShower_dirZ)[ii] );
  // Get reco shower energy in GeV
  const double showerE = (*AnaIO::reco_daughter_allShower_energy)[ii] * 1E-3;
  const double showerE_corrected = showerE;//(showerE - 0.01167)/0.68;
  // Combine diretion and energy to get shower 3 momemtum
  const TVector3 showerMomentum = showerDir*showerE_corrected; 
  // Get shower lorentzVector
  const TLorentzVector showerLv( showerMomentum, showerMomentum.Mag() );

  return showerLv;
}

TLorentzVector AnaUtils::GetPiZero()
{
  
  const int truthEventType = GetFillEventType();
  // Get the size of shower array
  const int shsize = showerArray.size();
  plotUtils.FillHist(AnaIO::hRecPi0Nshower, shsize, truthEventType); 

  // Declare PiZero vector
  TLorentzVector PiZeroVec;
  // Need to have at least two showers to reconstruct pi0
  if(shsize>=2){
    // Get [0] element of shower energy vector
    const double* shE = &(showerEarr[0]);
    int *nindex = new int[shsize];
    // Sort shower energy
    TMath::Sort(shsize, shE, nindex, true);
    // Set leading and subleading shower vector
    const TLorentzVector ldShower = showerArray[nindex[0]];
    const TLorentzVector slShower = showerArray[nindex[1]];
    // Calculate opening angle
    const double openingAngle = ldShower.Angle(slShower.Vect())*TMath::RadToDeg();
    plotUtils.FillHist(AnaIO::hRecShowerOpenAngle, openingAngle, truthEventType);
    
    const TVector3 ldShowerPos = showerPos[nindex[0]];
    const TVector3 slShowerPos = showerPos[nindex[1]];
    const TVector3 distVect = ldShowerPos - slShowerPos;

    const double separation = distVect.Mag();
    plotUtils.FillHist(AnaIO::hRecPi0ShowerSep, separation, truthEventType);
    // Combine leading shower and subleading shower to get pi0 vector 
    PiZeroVec = ldShower + slShower;
    const double mpi0 = PiZeroVec.M();
    plotUtils.FillHist(AnaIO::hRecPi0Mass, mpi0, truthEventType);
  } 
  return PiZeroVec;
}
