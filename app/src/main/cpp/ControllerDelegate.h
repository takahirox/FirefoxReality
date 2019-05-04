/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONTROLLER_DELEGATE_DOT_H
#define CONTROLLER_DELEGATE_DOT_H

#include "vrb/MacroUtils.h"
#include "vrb/Forward.h"
#include "Device.h"
#include "GestureDelegate.h"

#include <memory>

namespace crow {

class ControllerDelegate;
typedef std::shared_ptr<ControllerDelegate> ControllerDelegatePtr;

class ControllerDelegate {
public:
  enum Button {
    BUTTON_TRIGGER   = 1u << 0u,
    BUTTON_TOUCHPAD  = 1u << 1u,
    BUTTON_APP       = 1u << 2u,
    BUTTON_A         = 1u << 3u,
    BUTTON_B         = 1u << 4u,
    BUTTON_X         = 1u << 5u,
    BUTTON_Y         = 1u << 6u,
    BUTTON_OTHERS    = 1u << 7u,  // Other buttons only for the immersive mode.
  };

  virtual void CreateController(const int32_t aControllerIndex, const int32_t aModelIndex, const std::string& aImmersiveName) = 0;
  virtual void CreateController(const int32_t aControllerIndex, const int32_t aModelIndex, const std::string& aImmersiveName, const vrb::Matrix& aBeamTransform) = 0;
  virtual void DestroyController(const int32_t aControllerIndex) = 0;
  virtual uint32_t GetControllerCount() = 0;
  virtual void SetCapabilityFlags(const int32_t aControllerIndex, const device::CapabilityFlags aFlags) = 0;
  virtual void SetEnabled(const int32_t aControllerIndex, const bool aEnabled) = 0;
  virtual void SetVisible(const int32_t aControllerIndex, const bool aVisible) = 0;
  virtual void SetTransform(const int32_t aControllerIndex, const vrb::Matrix& aTransform) = 0;
  virtual void SetButtonCount(const int32_t aControllerIndex, const uint32_t aNumButtons) = 0;
  virtual void SetButtonState(const int32_t aControllerIndex, const Button aWhichButton, const int32_t aImmersiveIndex, const bool aPressed, const bool aTouched, const float aImmersiveTrigger = -1.0f) = 0;
  virtual void SetAxes(const int32_t aControllerIndex, const float* aData, const uint32_t aLength) = 0;
  virtual void SetLeftHanded(const int32_t aControllerIndex, const bool aLeftHanded) = 0;
  virtual void SetTouchPosition(const int32_t aControllerIndex, const float aTouchX, const float aTouchY) = 0;
  virtual void EndTouch(const int32_t aControllerIndex) = 0;
  virtual void SetScrolledDelta(const int32_t aControllerIndex, const float aScrollDeltaX, const float aScrollDeltaY) = 0;
  virtual void SetAngularAcceleration(const int32_t aControllerIndex, const vrb::Vector& aVector) = 0;
  virtual void SetAngularVelocity(const int32_t aControllerIndex, const vrb::Vector& aVector) = 0;
  virtual void SetLinearAcceleration(const int32_t aControllerIndex, const vrb::Vector& aVector) = 0;
  virtual void SetLinearVelocity(const int32_t aControllerIndex, const vrb::Vector& aVector) = 0;
protected:
  ControllerDelegate() {}
private:
  VRB_NO_DEFAULTS(ControllerDelegate)
};

}

#endif // CONTROLLER_DELEGATE_DOT_H
