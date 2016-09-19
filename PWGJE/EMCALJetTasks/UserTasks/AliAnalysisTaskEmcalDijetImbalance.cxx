/**************************************************************************
 * Copyright(c) 1998-2016, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

#include <TClonesArray.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TList.h>

#include <AliVCluster.h>
#include <AliVParticle.h>
#include <AliLog.h>

#include "AliTLorentzVector.h"
#include "AliEmcalJet.h"
#include "AliRhoParameter.h"
#include "AliJetContainer.h"
#include "AliParticleContainer.h"
#include "AliClusterContainer.h"

#include "AliAnalysisTaskEmcalDijetImbalance.h"

/// \cond CLASSIMP
ClassImp(AliAnalysisTaskEmcalDijetImbalance);
/// \endcond

/**
 * Default constructor. Needed by ROOT I/O
 */
AliAnalysisTaskEmcalDijetImbalance::AliAnalysisTaskEmcalDijetImbalance() : 
  AliAnalysisTaskEmcalJet(),
  fHistManager(),
  fDeltaPhiMin(0),
  fTrigJetMinPt(0),
  fAssJetMinPt(0)
{
}

/**
 * Standard constructor. Should be used by the user.
 *
 * @param[in] name Name of the task
 */
AliAnalysisTaskEmcalDijetImbalance::AliAnalysisTaskEmcalDijetImbalance(const char *name) : 
  AliAnalysisTaskEmcalJet(name, kTRUE),
  fHistManager(name),
  fDeltaPhiMin(2),
  fTrigJetMinPt(0),
  fAssJetMinPt(0)
{
  SetMakeGeneralHistograms(kTRUE);
}

/**
 * Destructor
 */
AliAnalysisTaskEmcalDijetImbalance::~AliAnalysisTaskEmcalDijetImbalance()
{
}

/**
 * Performing run-independent initialization.
 * Here the histograms should be instantiated.
 */
void AliAnalysisTaskEmcalDijetImbalance::UserCreateOutputObjects()
{
  AliAnalysisTaskEmcalJet::UserCreateOutputObjects();

  AllocateClusterHistograms();
  AllocateTrackHistograms();
  AllocateJetHistograms();
  AllocateCellHistograms();
  AllocateDijetHistograms();

  TIter next(fHistManager.GetListOfHistograms());
  TObject* obj = 0;
  while ((obj = next())) {
    fOutput->Add(obj);
  }
  
  PostData(1, fOutput); // Post data for ALL output slots > 0 here.
}

/*
 * This function allocates the histograms for basic EMCal cluster QA.
 * A set of histograms (energy, eta, phi, number of cluster) is allocated
 * per each cluster container and per each centrality bin.
 */
void AliAnalysisTaskEmcalDijetImbalance::AllocateClusterHistograms()
{
  TString histname;
  TString histtitle;
  TString groupname;
  AliClusterContainer* clusCont = 0;
  TIter next(&fClusterCollArray);
  while ((clusCont = static_cast<AliClusterContainer*>(next()))) {
    groupname = clusCont->GetName();
    fHistManager.CreateHistoGroup(groupname);
    for (Int_t cent = 0; cent < fNcentBins; cent++) {

      // Cluster histograms (PHOS+EMCal)

      histname = TString::Format("%s/histClusterEnergy_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{E}_{cluster} (GeV);counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 2, fMinBinPt, fMaxBinPt / 2);

      histname = TString::Format("%s/histClusterEtaPhi_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{#eta}_{cluster};#it{#phi}_{cluster};counts", histname.Data());
      fHistManager.CreateTH2(histname, histtitle, fNbins / 6, -1, 1, fNbins / 2, 0, TMath::TwoPi());

      histname = TString::Format("%s/histNClusters_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;number of clusters;events", histname.Data());
      if (fForceBeamType != kpp) {
        fHistManager.CreateTH1(histname, histtitle, 500, 0, 3000);
      }
      else {
        fHistManager.CreateTH1(histname, histtitle, 200, 0, 200);
      }

      // EMCal cluster histograms

      histname = TString::Format("%s/histEMCalClusterEnergy_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{E}_{cluster} (GeV);counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 2, fMinBinPt, fMaxBinPt / 2);

      histname = TString::Format("%s/histEMCalClusterEnergyExotic_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{E}_{cluster}^{exotic} (GeV);counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 2, fMinBinPt, fMaxBinPt / 2);

      histname = TString::Format("%s/histEMCalClusterNonLinCorrEnergy_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{E}_{cluster}^{non-lin.corr.} (GeV);counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 2, fMinBinPt, fMaxBinPt / 2);

      histname = TString::Format("%s/histEMCalClusterHadCorrEnergy_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{E}_{cluster}^{had.corr.} (GeV);counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 2, fMinBinPt, fMaxBinPt / 2);

      histname = TString::Format("%s/histEMCalClusterPhi_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{#phi}_{cluster};counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 2, 0, TMath::TwoPi());

      histname = TString::Format("%s/histEMCalClusterEta_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{#eta}_{cluster};counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 6, -1, 1);

      histname = TString::Format("%s/histEMCalNClusters_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;number of clusters;events", histname.Data());
      if (fForceBeamType != kpp) {
        fHistManager.CreateTH1(histname, histtitle, 500, 0, 3000);
      }
      else {
        fHistManager.CreateTH1(histname, histtitle, 200, 0, 200);
      }

      // PHOS cluster histograms

      histname = TString::Format("%s/histPHOSClusterEnergy_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{E}_{cluster} (GeV);counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 2, fMinBinPt, fMaxBinPt / 2);

      histname = TString::Format("%s/histPHOSClusterPhi_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{#phi}_{cluster};counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 2, 0, TMath::TwoPi());

      histname = TString::Format("%s/histPHOSClusterEta_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{#eta}_{cluster};counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 6, -1, 1);

      histname = TString::Format("%s/histPHOSNClusters_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;number of clusters;events", histname.Data());
      if (fForceBeamType != kpp) {
        fHistManager.CreateTH1(histname, histtitle, 500, 0, 3000);
      }
      else {
        fHistManager.CreateTH1(histname, histtitle, 200, 0, 200);
      }

    }
  }
}

/*
 * This function allocates the histograms for basic EMCal QA.
 * One 2D histogram with the cell energy spectra and the number of cells
 * per event is allocated per each centrality bin.
 */
void AliAnalysisTaskEmcalDijetImbalance::AllocateCellHistograms()
{
  TString histname;
  TString histtitle;
  TString groupname(fCaloCellsName);

  fHistManager.CreateHistoGroup(groupname);
  for (Int_t cent = 0; cent < fNcentBins; cent++) {
    histname = TString::Format("%s/histCellEnergyvsAbsId_%d", groupname.Data(), cent);
    histtitle = TString::Format("%s;cell abs. ID;#it{E}_{cell} (GeV);counts", histname.Data());
    fHistManager.CreateTH2(histname, histtitle, 20000, 0, 20000, fNbins / 2, fMinBinPt, fMaxBinPt / 2);

    histname = TString::Format("%s/histNCells_%d", groupname.Data(), cent);
    histtitle = TString::Format("%s;number of cells;events", histname.Data());
    if (fForceBeamType != kpp) {
      fHistManager.CreateTH1(histname, histtitle, 500, 0, 6000);
    }
    else {
      fHistManager.CreateTH1(histname, histtitle, 200, 0, 200);
    }
  }
}

/*
 * This function allocates the histograms for basic tracking QA.
 * A set of histograms (pT, eta, phi, difference between kinematic properties
 * at the vertex and at the EMCal surface, number of tracks) is allocated
 * per each particle container and per each centrality bin.
 */
void AliAnalysisTaskEmcalDijetImbalance::AllocateTrackHistograms()
{
  TString histname;
  TString histtitle;
  TString groupname;
  AliParticleContainer* partCont = 0;
  TIter next(&fParticleCollArray);
  while ((partCont = static_cast<AliParticleContainer*>(next()))) {
    groupname = partCont->GetName();
    fHistManager.CreateHistoGroup(groupname);
    for (Int_t cent = 0; cent < fNcentBins; cent++) {
      histname = TString::Format("%s/histTrackPt_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{p}_{T,track} (GeV/#it{c});counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 2, fMinBinPt, fMaxBinPt / 2);

      histname = TString::Format("%s/histTrackPhi_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{#phi}_{track};counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 2, 0, TMath::TwoPi());

      histname = TString::Format("%s/histTrackEta_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{#eta}_{track};counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 6, -1, 1);

      if (TClass(partCont->GetClassName()).InheritsFrom("AliVTrack")) {
        histname = TString::Format("%s/fHistDeltaEtaPt_%d", groupname.Data(), cent);
        histtitle = TString::Format("%s;#it{p}_{T,track}^{vertex} (GeV/#it{c});#it{#eta}_{track}^{vertex} - #it{#eta}_{track}^{EMCal};counts", histname.Data());
        fHistManager.CreateTH2(histname, histtitle, fNbins / 2, fMinBinPt, fMaxBinPt, 50, -0.5, 0.5);

        histname = TString::Format("%s/fHistDeltaPhiPt_%d", groupname.Data(), cent);
        histtitle = TString::Format("%s;#it{p}_{T,track}^{vertex} (GeV/#it{c});#it{#phi}_{track}^{vertex} - #it{#phi}_{track}^{EMCal};counts", histname.Data());
        fHistManager.CreateTH2(histname, histtitle, fNbins / 2, fMinBinPt, fMaxBinPt, 200, -2, 2);

        histname = TString::Format("%s/fHistDeltaPtvsPt_%d", groupname.Data(), cent);
        histtitle = TString::Format("%s;#it{p}_{T,track}^{vertex} (GeV/#it{c});#it{p}_{T,track}^{vertex} - #it{p}_{T,track}^{EMCal} (GeV/#it{c});counts", histname.Data());
        fHistManager.CreateTH2(histname, histtitle, fNbins / 2, fMinBinPt, fMaxBinPt, fNbins / 2, -fMaxBinPt/2, fMaxBinPt/2);

        histname = TString::Format("%s/fHistEoverPvsP_%d", groupname.Data(), cent);
        histtitle = TString::Format("%s;#it{P}_{track} (GeV/#it{c});#it{E}_{cluster} / #it{P}_{track} #it{c};counts", histname.Data());
        fHistManager.CreateTH2(histname, histtitle, fNbins / 2, fMinBinPt, fMaxBinPt, fNbins / 2, 0, 4);
      }

      histname = TString::Format("%s/histNTracks_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;number of tracks;events", histname.Data());
      if (fForceBeamType != kpp) {
        fHistManager.CreateTH1(histname, histtitle, 500, 0, 5000);
      }
      else {
        fHistManager.CreateTH1(histname, histtitle, 200, 0, 200);
      }
    }
  }
}

/*
 * This function allocates the histograms for basic jet QA.
 * A set of histograms (pT, eta, phi, area, number of jets, corrected pT) is allocated
 * per each jet container and per each centrality bin.
 */
void AliAnalysisTaskEmcalDijetImbalance::AllocateJetHistograms()
{
  TString histname;
  TString histtitle;
  TString groupname;
  AliJetContainer* jetCont = 0;
  TIter next(&fJetCollArray);
  while ((jetCont = static_cast<AliJetContainer*>(next()))) {
    groupname = jetCont->GetName();
    fHistManager.CreateHistoGroup(groupname);
    for (Int_t cent = 0; cent < fNcentBins; cent++) {
      histname = TString::Format("%s/histJetPt_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{p}_{T,jet} (GeV/#it{c});counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins, fMinBinPt, fMaxBinPt);

      histname = TString::Format("%s/histJetArea_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{A}_{jet};counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 2, 0, 1.5);

      histname = TString::Format("%s/histJetPhi_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{#phi}_{jet};counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 2, 0, TMath::TwoPi());

      histname = TString::Format("%s/histJetEta_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{#eta}_{jet};counts", histname.Data());
      fHistManager.CreateTH1(histname, histtitle, fNbins / 6, -1, 1);

      histname = TString::Format("%s/histJetEtaPhi_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;#it{#eta}_{jet};#it{#phi}_{jet};counts", histname.Data());
      fHistManager.CreateTH2(histname, histtitle, fNbins / 6, -1, 1, fNbins / 2, 0, TMath::TwoPi());

      histname = TString::Format("%s/histNJets_%d", groupname.Data(), cent);
      histtitle = TString::Format("%s;number of jets;events", histname.Data());
      if (fForceBeamType != kpp) {
        fHistManager.CreateTH1(histname, histtitle, 500, 0, 500);
      }
      else {
        fHistManager.CreateTH1(histname, histtitle, 100, 0, 100);
      }

      if (!jetCont->GetRhoName().IsNull()) {
        histname = TString::Format("%s/histJetCorrPt_%d", groupname.Data(), cent);
        histtitle = TString::Format("%s;#it{p}_{T,jet}^{corr} (GeV/#it{c});counts", histname.Data());
        fHistManager.CreateTH1(histname, histtitle, fNbins, -fMaxBinPt / 2, fMaxBinPt / 2);
        
        histname = TString::Format("%s/histJetRho_%d", groupname.Data(), cent);
        histtitle = TString::Format("%s;{#rho} (GeV);counts", histname.Data());
        fHistManager.CreateTH1(histname, histtitle, fNbins, 0, 500);
      }
    }
  }
}

/*
 * This function allocates the histograms for the dijet analysis.
 * A set of histograms is allocated per each jet container and per each centrality bin.
 */
void AliAnalysisTaskEmcalDijetImbalance::AllocateDijetHistograms()
{
  TString histname;
  TString histtitle;
  TString groupname;
  AliJetContainer* jetCont = 0;
  TIter next(&fJetCollArray);
  while ((jetCont = static_cast<AliJetContainer*>(next()))) {
    groupname = jetCont->GetName();
    //fHistManager.CreateHistoGroup(groupname);
    for (Int_t cent = 0; cent < fNcentBins; cent++) {
      
      // Loop over leading hadron requirement
      for (Int_t k=0; k<2; k++) {
      
        // Loop over trigJetMinPt thresholds
        for (Int_t i=0; i<4; i++) {
          
          // Loop over assJetMinPt thresholds
          for (Int_t j=0; j<4; j++) {
            
            TString label = TString::Format("_%d_had%d_trig%d_ass%d", cent, k, i, j);

            // Some basic plots for di-jet pairs

            histname = TString::Format("%s/histDijetLeadingJetPt", groupname.Data()) += label;
            histtitle = TString::Format("%s;Leading Jet p_{T} (GeV);counts", histname.Data());
            fHistManager.CreateTH1(histname, histtitle, fNbins, fMinBinPt, fMaxBinPt);
            
            histname = TString::Format("%s/histDijetLeadingJetPtuncorr", groupname.Data()) += label;
            histtitle = TString::Format("%s;Uncorrected Leading Jet p_{T} (GeV);counts", histname.Data());
            fHistManager.CreateTH1(histname, histtitle, fNbins, fMinBinPt, fMaxBinPt);

            histname = TString::Format("%s/histDijetSubleadingJetPt", groupname.Data()) += label;
            histtitle = TString::Format("%s;Subleading Jet p_{T} (GeV);counts", histname.Data());
            fHistManager.CreateTH1(histname, histtitle, fNbins, fMinBinPt, fMaxBinPt);

            histname = TString::Format("%s/histDijetLeadingJetPhi", groupname.Data()) += label;
            histtitle = TString::Format("%s;Leading Jet #phi;counts", histname.Data());
            fHistManager.CreateTH1(histname, histtitle, 100, 0, TMath::TwoPi());

            histname = TString::Format("%s/histDijetSubleadingJetPhi", groupname.Data()) += label;
            histtitle = TString::Format("%s;Subleading Jet #phi;counts", histname.Data());
            fHistManager.CreateTH1(histname, histtitle, 100, 0, TMath::TwoPi());

            //histname = TString::Format("%s/histDijetLeadingJetEta_%d", groupname.Data(), cent);
            //histtitle = TString::Format("%s;Leading Jet #eta;counts", histname.Data());
            //fHistManager.CreateTH1(histname, histtitle, 100, -1, 1);

            //histname = TString::Format("%s/histDijetSubleadingJetEta_%d", groupname.Data(), cent);
            //histtitle = TString::Format("%s;Subleading Jet #eta;counts", histname.Data());
            //fHistManager.CreateTH1(histname, histtitle, 100, -1, 1);

            //histname = TString::Format("%s/histDijetDeltaEta_%d", groupname.Data(), cent);
            //histtitle = TString::Format("%s;#Delta#eta;counts", histname.Data());
            //fHistManager.CreateTH1(histname, histtitle, 100, -2, 2);

            // Some di-jet observables
       
            histname = TString::Format("%s/histDijetAJ", groupname.Data()) += label;
            histtitle = TString::Format("%s;A_{J};counts", histname.Data());
            fHistManager.CreateTH1(histname, histtitle, 100, 0, 1);

            histname = TString::Format("%s/histDijetxJ", groupname.Data()) += label;
            histtitle = TString::Format("%s;x_{J};counts", histname.Data());
            fHistManager.CreateTH1(histname, histtitle, 100, 0, 1);
            
            histname = TString::Format("%s/histDijetDeltaPhi", groupname.Data()) += label;
            histtitle = TString::Format("%s;#Delta#phi;counts", histname.Data());
            fHistManager.CreateTH1(histname, histtitle, 100, 0, 4);

            //histname = TString::Format("%s/histDijetkT_%d", groupname.Data(), cent);
            //histtitle = TString::Format("%s;k_{Ty} (GeV);counts", histname.Data());
            //fHistManager.CreateTH1(histname, histtitle, 100, 0, 100);

            //histname = TString::Format("%s/histDijetLeadingJetNTracks_%d", groupname.Data(), cent);
            //histtitle = TString::Format("%s;Leading Jet N tracks;counts", histname.Data());
            //fHistManager.CreateTH1(histname, histtitle, 100, 0, 100);

            //histname = TString::Format("%s/histDijetSubleadingJetNTracks_%d", groupname.Data(), cent);
            //histtitle = TString::Format("%s;Subleading Jet N tracks;counts", histname.Data());
            //fHistManager.CreateTH1(histname, histtitle, 100, 0, 100);

            //histname = TString::Format("%s/histDijetLeadingJetArea_%d", groupname.Data(), cent);
            //histtitle = TString::Format("%s;Leading Jet Area;counts", histname.Data());
            //fHistManager.CreateTH1(histname, histtitle, 100, 0, 3);

            //histname = TString::Format("%s/histDijetSubleadingJetArea_%d", groupname.Data(), cent);
            //histtitle = TString::Format("%s;Subleading Jet Area;counts", histname.Data());
            //fHistManager.CreateTH1(histname, histtitle, 100, 0, 3);

            // Leading jets that don't have acceptable associated jet
            histname = TString::Format("%s/histUnmatchedLeadingJetPt", groupname.Data()) += label;
            histtitle = TString::Format("%s;Leading Jet p_{T} (GeV);counts", histname.Data());
            fHistManager.CreateTH1(histname, histtitle, fNbins, fMinBinPt, fMaxBinPt);

            // When leading jet doesn't have an acceptable associated jet, plot the subleading jet in the event
            histname = TString::Format("%s/histUnmatchedSubleadingJetPt", groupname.Data()) += label;
            histtitle = TString::Format("%s;Subleading Jet p_{T} (GeV);counts", histname.Data());
            fHistManager.CreateTH1(histname, histtitle, fNbins, fMinBinPt, fMaxBinPt);
            
          }
        }
      }
    }
  }
}

/**
 * The body of this function should contain instructions to fill the output histograms.
 * This function is called inside the event loop, after the function Run() has been
 * executed successfully (i.e. it returned kTRUE).
 * @return Always kTRUE
 */
Bool_t AliAnalysisTaskEmcalDijetImbalance::FillHistograms()
{
  DoJetLoop();
  DoTrackLoop();
  DoClusterLoop();
  DoCellLoop();

  return kTRUE;
}

/**
 * This function performs a loop over the reconstructed jets
 * in the current event and fills the relevant histograms.
 */
void AliAnalysisTaskEmcalDijetImbalance::DoJetLoop()
{
  TString histname;
  TString groupname;
  AliJetContainer* jetCont = 0;
  TIter next(&fJetCollArray);
  while ((jetCont = static_cast<AliJetContainer*>(next()))) {
    groupname = jetCont->GetName();
    UInt_t count = 0;
    for(auto jet : jetCont->accepted()) {
      if (!jet) continue;
      count++;

      histname = TString::Format("%s/histJetPt_%d", groupname.Data(), fCentBin);
      fHistManager.FillTH1(histname, jet->Pt());

      histname = TString::Format("%s/histJetArea_%d", groupname.Data(), fCentBin);
      fHistManager.FillTH1(histname, jet->Area());

      histname = TString::Format("%s/histJetPhi_%d", groupname.Data(), fCentBin);
      fHistManager.FillTH1(histname, jet->Phi());

      histname = TString::Format("%s/histJetEta_%d", groupname.Data(), fCentBin);
      fHistManager.FillTH1(histname, jet->Eta());

      histname = TString::Format("%s/histJetEtaPhi_%d", groupname.Data(), fCentBin);
      fHistManager.FillTH1(histname, jet->Eta(), jet->Phi());

      if (jetCont->GetRhoParameter()) {
        histname = TString::Format("%s/histJetCorrPt_%d", groupname.Data(), fCentBin);
        fHistManager.FillTH1(histname, jet->Pt() - jetCont->GetRhoVal() * jet->Area());
      }
    }
    histname = TString::Format("%s/histNJets_%d", groupname.Data(), fCentBin);
    fHistManager.FillTH1(histname, count);
    
    if (jetCont->GetRhoParameter()) {
      histname = TString::Format("%s/histJetRho_%d", groupname.Data(), fCentBin);
      fHistManager.FillTH1(histname, jetCont->GetRhoVal());
    }
  }
}

/**
 * This function performs a loop over the reconstructed tracks
 * in the current event and fills the relevant histograms.
 */
void AliAnalysisTaskEmcalDijetImbalance::DoTrackLoop()
{
  AliClusterContainer* clusCont = GetClusterContainer(0);

  TString histname;
  TString groupname;
  AliParticleContainer* partCont = 0;
  TIter next(&fParticleCollArray);
  while ((partCont = static_cast<AliParticleContainer*>(next()))) {
    groupname = partCont->GetName();
    UInt_t count = 0;
    for(auto part : partCont->accepted()) {
      if (!part) continue;
      count++;

      histname = TString::Format("%s/histTrackPt_%d", groupname.Data(), fCentBin);
      fHistManager.FillTH1(histname, part->Pt());

      histname = TString::Format("%s/histTrackPhi_%d", groupname.Data(), fCentBin);
      fHistManager.FillTH1(histname, part->Phi());

      histname = TString::Format("%s/histTrackEta_%d", groupname.Data(), fCentBin);
      fHistManager.FillTH1(histname, part->Eta());

      if (partCont->GetLoadedClass()->InheritsFrom("AliVTrack")) {
        const AliVTrack* track = static_cast<const AliVTrack*>(part);

        histname = TString::Format("%s/fHistDeltaEtaPt_%d", groupname.Data(), fCentBin);
        fHistManager.FillTH1(histname, track->Pt(), track->Eta() - track->GetTrackEtaOnEMCal());

        histname = TString::Format("%s/fHistDeltaPhiPt_%d", groupname.Data(), fCentBin);
        fHistManager.FillTH1(histname, track->Pt(), track->Phi() - track->GetTrackPhiOnEMCal());

        histname = TString::Format("%s/fHistDeltaPtvsPt_%d", groupname.Data(), fCentBin);
        fHistManager.FillTH1(histname, track->Pt(), track->Pt() - track->GetTrackPtOnEMCal());

        if (clusCont) {
          Int_t iCluster = track->GetEMCALcluster();
          if (iCluster >= 0) {
            AliVCluster* cluster = clusCont->GetAcceptCluster(iCluster);
            if (cluster) {
              histname = TString::Format("%s/fHistEoverPvsP_%d", groupname.Data(), fCentBin);
              fHistManager.FillTH2(histname, track->P(), cluster->GetNonLinCorrEnergy() / track->P());
            }
          }
        }
      }
    }

    histname = TString::Format("%s/histNTracks_%d", groupname.Data(), fCentBin);
    fHistManager.FillTH1(histname, count);
  }
}

/**
 * This function performs a loop over the reconstructed EMCal/PHOS clusters
 * in the current event and fills the relevant histograms.
 */
void AliAnalysisTaskEmcalDijetImbalance::DoClusterLoop()
{
  TString histname;
  TString groupname;
  AliClusterContainer* clusCont = 0;
  TIter next(&fClusterCollArray);
  while ((clusCont = static_cast<AliClusterContainer*>(next()))) {
    groupname = clusCont->GetName();

    for(auto cluster : clusCont->all()) {
      if (!cluster) continue;

      if (cluster->GetIsExotic()) {
        histname = TString::Format("%s/histEMCalClusterEnergyExotic_%d", groupname.Data(), fCentBin);
        fHistManager.FillTH1(histname, cluster->E());
      }
    }

    UInt_t count = 0;
    UInt_t countEMCal = 0;
    UInt_t countPHOS = 0;
    for(auto cluster : clusCont->accepted()) {
      if (!cluster) continue;
      count++;

      AliTLorentzVector nPart;
      cluster->GetMomentum(nPart, fVertex);

      histname = TString::Format("%s/histClusterEnergy_%d", groupname.Data(), fCentBin);
      fHistManager.FillTH1(histname, cluster->E());

      histname = TString::Format("%s/histClusterEtaPhi_%d", groupname.Data(),fCentBin);
      fHistManager.FillTH2(histname, nPart.Eta(), nPart.Phi_0_2pi());

      if (cluster->IsEMCAL()) {

        countEMCal++;

        histname = TString::Format("%s/histEMCalClusterEnergy_%d", groupname.Data(), fCentBin);
        fHistManager.FillTH1(histname, cluster->E());

        histname = TString::Format("%s/histEMCalClusterNonLinCorrEnergy_%d", groupname.Data(), fCentBin);
        fHistManager.FillTH1(histname, cluster->GetNonLinCorrEnergy());

        histname = TString::Format("%s/histEMCalClusterHadCorrEnergy_%d", groupname.Data(), fCentBin);
        fHistManager.FillTH1(histname, cluster->GetHadCorrEnergy());

        histname = TString::Format("%s/histEMCalClusterPhi_%d", groupname.Data(), fCentBin);
        fHistManager.FillTH1(histname, nPart.Phi_0_2pi());

        histname = TString::Format("%s/histEMCalClusterEta_%d", groupname.Data(), fCentBin);
        fHistManager.FillTH1(histname, nPart.Eta());

      } else if (cluster->IsPHOS()){

        countPHOS++;

        histname = TString::Format("%s/histPHOSClusterEnergy_%d", groupname.Data(), fCentBin);
        fHistManager.FillTH1(histname, cluster->E());

        histname = TString::Format("%s/histPHOSClusterPhi_%d", groupname.Data(), fCentBin);
        fHistManager.FillTH1(histname, nPart.Phi_0_2pi());

        histname = TString::Format("%s/histPHOSClusterEta_%d", groupname.Data(), fCentBin);
        fHistManager.FillTH1(histname, nPart.Eta());

      }
    }

    histname = TString::Format("%s/histNClusters_%d", groupname.Data(), fCentBin);
    fHistManager.FillTH1(histname, count);

    histname = TString::Format("%s/histEMCalNClusters_%d", groupname.Data(), fCentBin);
    fHistManager.FillTH1(histname, countEMCal);

    histname = TString::Format("%s/histPHOSNClusters_%d", groupname.Data(), fCentBin);
    fHistManager.FillTH1(histname, countPHOS);

  }
}

/**
 * This function performs a loop over the reconstructed EMCal cells
 * in the current event and fills the relevant histograms.
 */
void AliAnalysisTaskEmcalDijetImbalance::DoCellLoop()
{
  if (!fCaloCells) return;

  TString histname;

  const Short_t ncells = fCaloCells->GetNumberOfCells();

  histname = TString::Format("%s/histNCells_%d", fCaloCellsName.Data(), fCentBin);
  fHistManager.FillTH1(histname, ncells);

  histname = TString::Format("%s/histCellEnergyvsAbsId_%d", fCaloCellsName.Data(), fCentBin);
  for (Short_t pos = 0; pos < ncells; pos++) {
    Double_t amp   = fCaloCells->GetAmplitude(pos);
    Short_t absId  = fCaloCells->GetCellNumber(pos);

    fHistManager.FillTH2(histname, absId, amp);
  }
}

/**
 * This function is executed automatically for the first event.
 * Some extra initialization can be performed here.
 */
void AliAnalysisTaskEmcalDijetImbalance::ExecOnce()
{
  AliAnalysisTaskEmcalJet::ExecOnce();
}

/**
 * Run analysis code here, if needed.
 * It will be executed before FillHistograms().
 * If this function return kFALSE, FillHistograms() will *not*
 * be executed for the current event
 * @return Always kTRUE
 */
Bool_t AliAnalysisTaskEmcalDijetImbalance::Run()
{
  TString histname;
  TString groupname;
  
  AliJetContainer* jetCont = 0;
  TIter next(&fJetCollArray);
  while ((jetCont = static_cast<AliJetContainer*>(next()))) {
    groupname = jetCont->GetName();

    // Get trigger jet
    AliEmcalJet* trigJet = 0;
    if (jetCont->GetRhoParameter())
      trigJet = jetCont->GetLeadingJet("rho");
    else
      trigJet = jetCont->GetLeadingJet();
    
    if(!trigJet) continue;
    Double_t trigJetPt = trigJet->Pt() - jetCont->GetRhoVal() * trigJet->Area();
    Double_t trigJetEta = trigJet->Eta();
    Double_t trigJetPhi = trigJet->Phi();
    
    // Iterate separately for each leading hadron cut
    Double_t leadingHadCut;
    for (Int_t k=0; k<2; k++) {
      
      // Skip the event if the leading jet doesn't satisfy the leading hadron threshold
      leadingHadCut = 5*k;
      if (jetCont->GetLeadingHadronPt(trigJet) < leadingHadCut) continue;
        
      // Code for a single set of pT thresolds
      /*
      if( trigJetPt < fTrigJetMinPt) continue;
    
      // Look for associated jet
      AliEmcalJet *assJet = 0;
      for(auto assJetCand : jetCont->accepted()) {
        if (!assJetCand) continue;
        Double_t assJetCandPt = assJetCand->Pt() - jetCont->GetRhoVal() * assJetCand->Area();
        if ( assJetCandPt < fAssJetMinPt ) continue;
        if ( TMath::Abs(trigJet->Phi() - assJetCand->Phi()) < fDeltaPhiMin ) continue;
        if (assJet) {
          Double_t assJetPt = assJet->Pt() - jetCont->GetRhoVal() * assJet->Area();
          if ( assJetCandPt < assJetPt ) continue;
        }
        
        assJet = assJetCand;
      }
      */
      
      // Loop through a variety of leading jet pT thresholds
      Double_t trigJetMinPt;
      for (Int_t i = 0; i < 4; i++) {

        // Set the trig jet pT threshold
        trigJetMinPt = 35 + 5*i;
        if( trigJetPt < trigJetMinPt) continue;
        
        // Loop through a variety of subleading jet pT thresholds
        Double_t assJetMinPtFrac;
        Double_t assJetMinPt;
        for (Int_t j=0; j < 4; j++) {
          if (j==0) assJetMinPtFrac = 0;
          if (j==1) assJetMinPtFrac = 0.4;
          if (j==2) assJetMinPtFrac = 0.5;
          if (j==3) assJetMinPtFrac = 0.6;
          Double_t assJetMinPt = trigJetMinPt * assJetMinPtFrac;
          
          // histogram label
          TString label = TString::Format("_%d_had%d_trig%d_ass%d", fCentBin, k, i, j);
          
          // Find the subleading jet in the opposite hemisphere
          AliEmcalJet *assJet = 0;
          for(auto assJetCand : jetCont->accepted()) {
            if (!assJetCand) continue;
            Double_t assJetCandPt = assJetCand->Pt() - jetCont->GetRhoVal() * assJetCand->Area();
            if ( assJetCandPt < assJetMinPt ) continue;
            if ( TMath::Abs(trigJet->Phi() - assJetCand->Phi()) < fDeltaPhiMin ) continue;
            if (assJet) {
              Double_t assJetPt = assJet->Pt() - jetCont->GetRhoVal() * assJet->Area();
              if ( assJetCandPt < assJetPt ) continue;
            }
            
            assJet = assJetCand;
          }
          
           
          // If we find an acceptable associated jet, fill the di-jet histograms
          if(assJet){
            
            // Define kinematic variables for the di-jet pair
            Double_t assJetPt   = assJet->Pt() - jetCont->GetRhoVal() * assJet->Area();
            Double_t assJetPhi  = assJet->Phi();
            Double_t assJetEta  = assJet->Eta();

            // Some basic plots for the found di-jet pairs

            histname = TString::Format("%s/histDijetLeadingJetPt", groupname.Data()) += label;
            fHistManager.FillTH1(histname, trigJetPt);
            
            histname = TString::Format("%s/histDijetLeadingJetPtuncorr", groupname.Data()) += label;
            fHistManager.FillTH1(histname, trigJet->Pt());

            histname = TString::Format("%s/histDijetLeadingJetPhi", groupname.Data()) += label;
            fHistManager.FillTH1(histname, trigJetPhi);

            //histname = TString::Format("%s/histDijetLeadingJetEta_%d", groupname.Data(), fCentBin);
            //fHistManager.FillTH1(histname, trigJetEta);

            histname = TString::Format("%s/histDijetSubleadingJetPt", groupname.Data()) += label;
            fHistManager.FillTH1(histname, assJetPt);

            histname = TString::Format("%s/histDijetSubleadingJetPhi", groupname.Data()) += label;
            fHistManager.FillTH1(histname, assJetPhi);

            //histname = TString::Format("%s/histDijetSubleadingJetEta_%d", groupname.Data(), fCentBin);
            //fHistManager.FillTH1(histname, assJetEta);

            //Double_t deltaEta = trigJetEta - assJetEta;
            //histname = TString::Format("%s/histDijetDeltaEta_%d", groupname.Data(), fCentBin);
            //fHistManager.FillTH1(histname, deltaEta);

            // Some di-jet observables

            Double_t AJ = (trigJetPt - assJetPt)/(trigJetPt + assJetPt);
            histname = TString::Format("%s/histDijetAJ", groupname.Data()) += label;
            fHistManager.FillTH1(histname, AJ);

            Double_t xJ = assJetPt / trigJetPt;
            histname = TString::Format("%s/histDijetxJ", groupname.Data()) += label;
            fHistManager.FillTH1(histname, xJ);

            Double_t deltaPhi = TMath::Abs(trigJetPhi - assJetPhi);
            histname = TString::Format("%s/histDijetDeltaPhi", groupname.Data()) += label;
            fHistManager.FillTH1(histname, deltaPhi);

            //Double_t kT = TMath::Abs( trigJetPt * TMath::Sin(deltaPhi) );
            //histname = TString::Format("%s/histDijetkT_%d", groupname.Data(), fCentBin);
            //fHistManager.FillTH1(histname, kT);

            // Study internal structure of the leading, subleading jets

            //Double_t nTracksLeadingJet = trigJet->GetNumberOfTracks();
            //histname = TString::Format("%s/histDijetLeadingJetNTracks_%d", groupname.Data(), fCentBin);
            //fHistManager.FillTH1(histname, nTracksLeadingJet);

            //Double_t nTracksSubleadingJet = assJet->GetNumberOfTracks();
            //histname = TString::Format("%s/histDijetSubleadingJetNTracks_%d", groupname.Data(), fCentBin);
            //fHistManager.FillTH1(histname, nTracksSubleadingJet);

            //histname = TString::Format("%s/histDijetLeadingJetArea_%d", groupname.Data(), fCentBin);
            //fHistManager.FillTH1(histname, trigJet->Area());

            //histname = TString::Format("%s/histDijetSubleadingJetArea_%d", groupname.Data(), fCentBin);
            //fHistManager.FillTH1(histname, assJet->Area());

            // Study momentum balance
            // TO DO: loop through tracks/clusters in event (as in above loops) using ClusterAt(i)/TrackAt(i)
            // and group according to proximity to leading, subleading jet axes

          } else { // If we don't find an associated jet

            // Plot leading jets that did not find any associated jet
            histname = TString::Format("%s/histUnmatchedLeadingJetPt", groupname.Data()) += label;
            fHistManager.FillTH1(histname, trigJetPt);
            
            // Find subleading jet in event
            AliEmcalJet *subleadingJet = 0;
            for(auto subleadingJetCand : jetCont->accepted()) {
              if (!subleadingJetCand) continue;
              Double_t subleadingJetCandPt = subleadingJetCand->Pt() - jetCont->GetRhoVal() * subleadingJetCand->Area();
              if (subleadingJetCandPt < trigJetPt-0.01) {
                if (subleadingJet) {
                  Double_t subleadingJetPt = subleadingJet->Pt() - jetCont->GetRhoVal() * subleadingJet->Area();
                  if ( subleadingJetCandPt < subleadingJetPt ) continue;
                }
                
                subleadingJet = subleadingJetCand;
              }
            }
            if(subleadingJet) {
              histname = TString::Format("%s/histUnmatchedSubleadingJetPt", groupname.Data()) += label;
              fHistManager.FillTH1(histname, subleadingJet->Pt() - jetCont->GetRhoVal() * subleadingJet->Area());
            }
          }
        }
      }
    }
    
  }
  
  return kTRUE;
}

/**
 * This function is called once at the end of the analysis.
 */
void AliAnalysisTaskEmcalDijetImbalance::Terminate(Option_t *)
{
  
}
