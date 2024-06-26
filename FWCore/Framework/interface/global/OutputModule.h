#ifndef FWCore_Framework_global_OutputModule_h
#define FWCore_Framework_global_OutputModule_h
// -*- C++ -*-
//
// Package:     FWCore/Framework
// Class  :     edm::global::OutputModule
//
/**\class edm::global::OutputModule OutputModule.h "FWCore/Framework/interface/global/OutputModule.h"

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
//

// system include files

// user include files
#include "FWCore/Framework/interface/global/outputmoduleAbilityToImplementor.h"

// forward declarations
namespace edm {
  namespace global {
    template <typename... T>
    class OutputModule : public virtual OutputModuleBase, public outputmodule::AbilityToImplementor<T>::Type... {
    public:
      OutputModule(edm::ParameterSet const& iPSet)
          : OutputModuleBase(iPSet), outputmodule::AbilityToImplementor<T>::Type(iPSet)... {}
      OutputModule(const OutputModule&) = delete;                   // stop default
      const OutputModule& operator=(const OutputModule&) = delete;  // stop default

      // Required to work around ICC bug, but possible source of bloat in gcc.
      // We do this only in the case of the intel compiler as this might end up
      // creating a lot of code bloat due to inline symbols being generated in
      // each DSO which uses this header.
#ifdef __INTEL_COMPILER
      virtual ~OutputModule() = default;
#endif

      // ---------- const member functions ---------------------
      bool wantsProcessBlocks() const noexcept final { return WantsProcessBlockTransitions<T...>::value; }
      bool wantsInputProcessBlocks() const noexcept final { return WantsInputProcessBlockTransitions<T...>::value; }
      bool wantsStreamRuns() const noexcept final { return WantsStreamRunTransitions<T...>::value; }
      bool wantsStreamLuminosityBlocks() const noexcept final {
        return WantsStreamLuminosityBlockTransitions<T...>::value;
      }

      // ---------- static member functions --------------------

      // ---------- member functions ---------------------------

    private:
      // ---------- member data --------------------------------
    };
  }  // namespace global
}  // namespace edm

#endif
