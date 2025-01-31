// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright
// holders. All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
///
/// \brief This task contains the individual steps that are to be taken
///        in the first part of the tutorial. These are 6 steps, and at the end,
///        the participant is expected to have a pT resolution histogram
///        as a function of transverse momentum.
/// \author
/// \since

#include "Framework/runDataProcessing.h"
#include "Framework/AnalysisTask.h"
#include "Framework/HistogramRegistry.h"
#include "PWGHF/DataModel/HFSecondaryVertex.h"
#include "PWGHF/DataModel/HFCandidateSelectionTables.h"

using namespace o2;
using namespace o2::framework;
using namespace o2::framework::expressions;

// STEP 1
//<- starting point, define the derived table to be stored
// this can be done in a separated header file, but for semplicity we do it in
// the same file here

namespace o2::aod
{
namespace mytable
{
DECLARE_SOA_COLUMN(InvMassD0, invMassD0, float);           //!
DECLARE_SOA_COLUMN(InvMassD0bar, invMassD0bar, float);     //!
DECLARE_SOA_COLUMN(Pt, pt, float);                         //!
DECLARE_SOA_COLUMN(CosinePointing, cosinePointing, float); //!
DECLARE_SOA_INDEX_COLUMN(Collision, collision);            //!
} // namespace mytable

DECLARE_SOA_TABLE(MyTable, "AOD", "MYTABLE", //!
                  mytable::InvMassD0,
                  mytable::InvMassD0bar,
                  mytable::Pt,
                  mytable::CosinePointing,
                  mytable::CollisionId)

} // namespace o2::aod

// STEP 2
struct ReadHFCandidates { //<- simple workflow that loops over HF 2-prong
                          // candidates

  void process(aod::HfCandProng2 const& cand2Prongs)
  {

    // loop over HF 2-prong candidates
    for (auto& cand : cand2Prongs) {

      // check first if the HF 2-prong candidate is tagged as a D0
      bool isD0Sel = TESTBIT(cand.hfflag(), aod::hf_cand_prong2::DecayType::D0ToPiK);

      if (!isD0Sel) {
        continue;
      }

      auto invMassD0 = InvMassD0(cand);
      auto invMassD0bar = InvMassD0bar(cand);

      LOG(debug) << "Candidate with mass(D0) = " << invMassD0
                 << ", mass(D0bar) = " << invMassD0bar
                 << ", pt = " << cand.pt()
                 << ", cos(theta_P) = " << cand.cpa();
    }
  }
};

// STEP 3
struct ProduceDerivedTable { //<- workflow that loops over HF 2-prong
                             // candidates and fills a derived table

  Produces<aod::MyTable> tableWithDzeroCandidates;

  void process(aod::HfCandProng2 const& cand2Prongs, aod::Tracks const&)
  {

    // loop over 2-prong candidates
    for (auto& cand : cand2Prongs) {

      // check first if the HF 2-prong candidate is tagged as a D0
      bool isD0Sel = TESTBIT(cand.hfflag(), aod::hf_cand_prong2::DecayType::D0ToPiK);

      if (!isD0Sel) {
        continue;
      }

      auto invMassD0 = InvMassD0(cand);
      auto invMassD0bar = InvMassD0bar(cand);

      LOG(debug) << "Candidate with mass(D0) = " << invMassD0
                 << ", mass(D0bar) = " << invMassD0bar
                 << ", pt = " << cand.pt()
                 << ", cos(theta_P) = " << cand.cpa();

      // we retrieve also the event index from one of the daughters
      auto dauTrack = cand.index0_as<aod::Tracks>(); // positive daughter

      tableWithDzeroCandidates(invMassD0, invMassD0bar, cand.pt(), cand.cpa(), dauTrack.collisionId());
    }
  }
};

// STEP 4
struct ProduceDerivedTableFilter { //<- workflow that loops over HF 2-prong
                                   // candidates and fills a derived table after applying a filter on pt

  Produces<aod::MyTable> tableWithDzeroCandidates;
  Filter ptFilter = std::sqrt(aod::hf_cand_prong2::px * aod::hf_cand_prong2::px + aod::hf_cand_prong2::py * aod::hf_cand_prong2::py) > 4.;

  void process(soa::Filtered<aod::HfCandProng2> const& cand2Prongs, aod::Tracks const&)
  {
    // loop over 2-prong candidates
    for (auto& cand : cand2Prongs) {

      // check first if the HF 2-prong candidate is tagged as a D0
      bool isD0Sel = TESTBIT(cand.hfflag(), aod::hf_cand_prong2::DecayType::D0ToPiK);

      if (!isD0Sel) {
        continue;
      }

      auto invMassD0 = InvMassD0(cand);
      auto invMassD0bar = InvMassD0bar(cand);

      LOG(debug) << "Candidate with mass(D0) = " << invMassD0
                 << ", mass(D0bar) = " << invMassD0bar
                 << ", pt = " << cand.pt()
                 << ", cos(theta_P) = " << cand.cpa();

      // we retrieve also the collision index from one of the daughters
      auto dauTrack = cand.index0_as<aod::Tracks>(); // positive daughter

      tableWithDzeroCandidates(invMassD0, invMassD0bar, cand.pt(), cand.cpa(), dauTrack.collisionId());
    }
  }
};

// STEP 5
struct ReadDerivedTable { //<- workflow that reads derived table and fill
                          // histograms

  HistogramRegistry registry{"registry",
                             {{"hMassD0", ";#it{M}(K#pi) (GeV/#it{c}^{2});counts", {HistType::kTH1F, {{300, 1.75, 2.05}}}},
                              {"hMassD0bar", ";#it{M}(#piK) (GeV/#it{c}^{2});counts", {HistType::kTH1F, {{300, 1.75, 2.05}}}},
                              {"hPt", ";#it{p}_{T} (GeV/#it{c});counts", {HistType::kTH1F, {{50, 0., 50.}}}},
                              {"hCosp", ";cos(#vartheta_{P}) ;counts", {HistType::kTH1F, {{100, 0.8, 1.}}}}}};

  void process(aod::MyTable const& cand2Prongs)
  {

    // loop over 2-prong candidates
    for (auto& cand : cand2Prongs) {
      registry.fill(HIST("hMassD0"), cand.invMassD0());
      registry.fill(HIST("hMassD0bar"), cand.invMassD0bar());
      registry.fill(HIST("hPt"), cand.pt());
      registry.fill(HIST("hCosp"), cand.cosinePointing());
    }
  }
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  return WorkflowSpec{adaptAnalysisTask<ReadHFCandidates>(cfgc),
                      adaptAnalysisTask<ProduceDerivedTable>(cfgc),
                      adaptAnalysisTask<ProduceDerivedTableFilter>(cfgc),
                      adaptAnalysisTask<ReadDerivedTable>(cfgc),
                      adaptAnalysisTask<ReadDerivedTable>(cfgc)};
}
