#include "DataFormats/HGCalDigi/interface/HGCalElectronicsId.h"

//
HGCalElectronicsId::HGCalElectronicsId(
  bool zside, bool cmflag,uint16_t fedid, uint8_t captureblock, uint8_t econdidx, uint8_t econderx, uint8_t halfrocch) {

  //there are 2 common mode channels per erdx
  halfrocch = cmflag ? (halfrocch-37)%2 : halfrocch;

  value_ =
    ((zside & kZsideMask) << kZsideShift) |
    ((cmflag & kCommonModeMask) << kCommonModeShift) |
    ((fedid & kFEDIDMask) << kFEDIDShift) | ((captureblock & kCaptureBlockMask) << kCaptureBlockShift) |
           ((econdidx & kECONDIdxMask) << kECONDIdxShift) | ((econderx & kECONDeRxMask) << kECONDeRxShift) |
           ((halfrocch & kHalfROCChannelMask) << kHalfROCChannelShift);
}

//
HGCalElectronicsId::HGCalElectronicsId(
  bool zside, uint16_t fedid, uint8_t captureblock, uint8_t econdidx, uint8_t econderx, uint8_t halfrocch)
  : HGCalElectronicsId(zside, (halfrocch==37 || halfrocch==38), fedid, captureblock, econdidx, econderx, halfrocch) {
}

//
uint16_t HGCalElectronicsId::fedId() const { return (value_ >> kFEDIDShift) & kFEDIDMask; }

//
bool HGCalElectronicsId::zSide() const { return (value_ >> kZsideShift) & kZsideMask; }

//
bool HGCalElectronicsId::isCM() const { return (value_ >> kCommonModeShift) & kCommonModeMask; }

//
uint8_t HGCalElectronicsId::captureBlock() const { return (value_ >> kCaptureBlockShift) & kCaptureBlockMask; }

//
uint8_t HGCalElectronicsId::econdIdx() const { return (value_ >> kECONDIdxShift) & kECONDIdxMask; }

//
uint8_t HGCalElectronicsId::econdeRx() const { return (value_ >> kECONDeRxShift) & kECONDeRxMask; }

//

uint8_t HGCalElectronicsId::halfrocChannel() const { return (value_ >> kHalfROCChannelShift) & kHalfROCChannelMask; }

//
uint8_t HGCalElectronicsId::sequentialHalfrocChannel() const { return halfrocChannel() + isCM()*37; }
