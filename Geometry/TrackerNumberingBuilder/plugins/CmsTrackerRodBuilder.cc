#include "Geometry/TrackerNumberingBuilder/plugins/CmsTrackerRodBuilder.h"
#include "DetectorDescription/Core/interface/DDFilteredView.h"
#include "DetectorDescription/DDCMS/interface/DDFilteredView.h"
#include "Geometry/TrackerNumberingBuilder/interface/GeometricDet.h"
#include "Geometry/TrackerNumberingBuilder/plugins/ExtractStringFromDDD.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "Geometry/TrackerNumberingBuilder/plugins/CmsDetConstruction.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include <vector>

template <class T>
void CmsTrackerRodBuilder<T>::buildComponent(T& fv, GeometricDet* g, std::string s) {
  CmsDetConstruction<T> theCmsDetConstruction;
  theCmsDetConstruction.buildComponent(fv, g, s);
}

template <class T>
void CmsTrackerRodBuilder<T>::sortNS(T& fv, GeometricDet* det) {
  GeometricDet::ConstGeometricDetContainer& comp = det->components();

  std::stable_sort(comp.begin(), comp.end(), CmsTrackerLevelBuilderHelper::isLessModZ);

  for (uint32_t i = 0; i < comp.size(); i++) {
    det->component(i)->setGeographicalID(i + 1);
  }

  if (comp.empty()) {
    edm::LogError("CmsTrackerRodBuilder") << "Where are the Rod's modules?";
  }
}

template class CmsTrackerRodBuilder<DDFilteredView>;
template class CmsTrackerRodBuilder<cms::DDFilteredView>;
