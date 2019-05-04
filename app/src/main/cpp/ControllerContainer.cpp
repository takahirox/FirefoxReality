/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ControllerContainer.h"
#include "Controller.h"
#include "Pointer.h"

#include "vrb/ConcreteClass.h"
#include "vrb/Color.h"
#include "vrb/Geometry.h"
#include "vrb/Group.h"
#include "vrb/Matrix.h"
#include "vrb/ModelLoaderAndroid.h"
#include "vrb/RenderState.h"
#include "vrb/Toggle.h"
#include "vrb/Transform.h"
#include "vrb/Vector.h"
#include "vrb/VertexArray.h"

using namespace vrb;

namespace crow {

struct ControllerContainer::State {
  std::vector<Controller> list;
  CreationContextWeak context;
  TogglePtr root;
  GroupPtr pointerContainer;
  std::vector<GroupPtr> models;
  GeometryPtr beamModel;
  bool visible;

  void Initialize(vrb::CreationContextPtr& aContext) {
    context = aContext;
    root = Toggle::Create(aContext);
    visible = true;
  }

  bool Contains(const int32_t aControllerIndex) {
    return (aControllerIndex >= 0) && (aControllerIndex < list.size());
  }

  void SetUpModelsGroup(const int32_t aModelIndex) {
    if (aModelIndex >= models.size()) {
      models.resize((size_t)(aModelIndex + 1));
    }
    if (!models[aModelIndex]) {
      CreationContextPtr create = context.lock();
      models[aModelIndex] = std::move(Group::Create(create));
    }
  }
};

ControllerContainerPtr
ControllerContainer::Create(vrb::CreationContextPtr& aContext, const vrb::GroupPtr& aPointerContainer) {
  auto result = std::make_shared<vrb::ConcreteClass<ControllerContainer, ControllerContainer::State> >(aContext);
  result->m.pointerContainer = aPointerContainer;
  return result;
}

TogglePtr
ControllerContainer::GetRoot() const {
  return m.root;
}

void
ControllerContainer::LoadControllerModel(const int32_t aModelIndex, const ModelLoaderAndroidPtr& aLoader, const std::string& aFileName) {
  m.SetUpModelsGroup(aModelIndex);
  aLoader->LoadModel(aFileName, m.models[aModelIndex]);
}

void
ControllerContainer::InitializeBeam() {
  if (m.beamModel) {
    return;
  }
  CreationContextPtr create = m.context.lock();
  VertexArrayPtr array = VertexArray::Create(create);
  const float kLength = -1.0f;
  const float kHeight = 0.004f;

  array->AppendVertex(Vector(-kHeight, -kHeight, 0.0f)); // Bottom left
  array->AppendVertex(Vector(kHeight, -kHeight, 0.0f)); // Bottom right
  array->AppendVertex(Vector(kHeight, kHeight, 0.0f)); // Top right
  array->AppendVertex(Vector(-kHeight, kHeight, 0.0f)); // Top left
  array->AppendVertex(Vector(0.0f, 0.0f, kLength)); // Tip

  array->AppendNormal(Vector(-1.0f, -1.0f, 0.0f).Normalize()); // Bottom left
  array->AppendNormal(Vector(1.0f, -1.0f, 0.0f).Normalize()); // Bottom right
  array->AppendNormal(Vector(1.0f, 1.0f, 0.0f).Normalize()); // Top right
  array->AppendNormal(Vector(-1.0f, 1.0f, 0.0f).Normalize()); // Top left
  array->AppendNormal(Vector(0.0f, 0.0f, -1.0f).Normalize()); // in to the screen


  RenderStatePtr state = RenderState::Create(create);
  state->SetMaterial(Color(1.0f, 1.0f, 1.0f), Color(1.0f, 1.0f, 1.0f), Color(0.0f, 0.0f, 0.0f), 0.0f);
  state->SetLightsEnabled(false);
  GeometryPtr geometry = Geometry::Create(create);
  geometry->SetVertexArray(array);
  geometry->SetRenderState(state);

  std::vector<int> index;
  std::vector<int> uvIndex;

  index.push_back(2);
  index.push_back(1);
  index.push_back(5);
  geometry->AddFace(index, uvIndex, index);

  index.clear();
  index.push_back(3);
  index.push_back(2);
  index.push_back(5);
  geometry->AddFace(index, uvIndex, index);

  index.clear();
  index.push_back(4);
  index.push_back(3);
  index.push_back(5);
  geometry->AddFace(index, uvIndex, index);

  index.clear();
  index.push_back(1);
  index.push_back(4);
  index.push_back(5);
  geometry->AddFace(index, uvIndex, index);

  m.beamModel = std::move(geometry);
  for (Controller& controller: m.list) {
    if (controller.transform) {
      controller.transform->AddNode(m.beamModel);
    }
  }
}

void
ControllerContainer::Reset() {
  for (Controller& controller: m.list) {
    if (controller.transform) {
      controller.transform->RemoveFromParents();
    }
    if (controller.pointer) {
      controller.pointer->GetRoot()->RemoveFromParents();
    }
    controller.Reset();
  }
}

std::vector<Controller>&
ControllerContainer::GetControllers() {
  return m.list;
}

const std::vector<Controller>&
ControllerContainer::GetControllers() const {
  return m.list;
}

// crow::ControllerDelegate interface
uint32_t
ControllerContainer::GetControllerCount() {
  return m.list.size();
}

void
ControllerContainer::CreateController(const int32_t aControllerIndex, const int32_t aModelIndex, const std::string& aImmersiveName) {
  CreateController(aControllerIndex, aModelIndex, aImmersiveName, vrb::Matrix::Identity());
}

void
ControllerContainer::CreateController(const int32_t aControllerIndex, const int32_t aModelIndex, const std::string& aImmersiveName, const vrb::Matrix& aBeamTransform) {
  if ((size_t)aControllerIndex >= m.list.size()) {
    m.list.resize((size_t)aControllerIndex + 1);
  }
  Controller& controller = m.list[aControllerIndex];
  controller.index = aControllerIndex;
  controller.immersiveName = aImmersiveName;
  if (!controller.transform && (aModelIndex >= 0)) {
    m.SetUpModelsGroup(aModelIndex);
    CreationContextPtr create = m.context.lock();
    controller.transform = Transform::Create(create);
    controller.pointer = Pointer::Create(create);
    if ((m.models.size() >= aModelIndex) && m.models[aModelIndex]) {
      controller.transform->AddNode(m.models[aModelIndex]);
      if (m.beamModel) {
        controller.beamToggle = vrb::Toggle::Create(create);
        if (aBeamTransform.IsIdentity()) {
          controller.beamParent = controller.beamToggle;
          controller.beamToggle->AddNode(m.beamModel);
        } else {
          vrb::TransformPtr beamTransform = Transform::Create(create);
          beamTransform->SetTransform(aBeamTransform);
          beamTransform->AddNode(m.beamModel);
          controller.beamParent = beamTransform;
          controller.beamToggle->AddNode(beamTransform);
        }
        controller.transform->AddNode(controller.beamToggle);
        controller.beamToggle->ToggleAll(false);
      }
      if (m.root) {
        m.root->AddNode(controller.transform);
        m.root->ToggleChild(*controller.transform, false);
      }
      if (m.pointerContainer) {
        m.pointerContainer->AddNode(controller.pointer->GetRoot());
      }
    } else {
      VRB_ERROR("Failed to add controller model");
    }
  }
}

void
ControllerContainer::DestroyController(const int32_t aControllerIndex) {
  if (m.Contains(aControllerIndex)) {
    m.list[aControllerIndex].Reset();
  }
}

void
ControllerContainer::SetCapabilityFlags(const int32_t aControllerIndex, const device::CapabilityFlags aFlags) {
  if (!m.Contains(aControllerIndex)) {
    return;
  }

  m.list[aControllerIndex].deviceCapabilities = aFlags;
}

void
ControllerContainer::SetEnabled(const int32_t aControllerIndex, const bool aEnabled) {
  if (!m.Contains(aControllerIndex)) {
    return;
  }
  m.list[aControllerIndex].enabled = aEnabled;
  if (!aEnabled) {
    SetVisible(aControllerIndex, false);
  }
}

void
ControllerContainer::SetVisible(const int32_t aControllerIndex, const bool aVisible) {
  if (!m.Contains(aControllerIndex)) {
    return;
  }
  Controller& controller = m.list[aControllerIndex];
  if (controller.transform && m.visible) {
    m.root->ToggleChild(*controller.transform, aVisible);
  }
  if (controller.pointer && !aVisible) {
    controller.pointer->SetVisible(false);
  }
}

void
ControllerContainer::SetTransform(const int32_t aControllerIndex, const vrb::Matrix& aTransform) {
  if (!m.Contains(aControllerIndex)) {
    return;
  }
  Controller& controller = m.list[aControllerIndex];
  controller.transformMatrix = aTransform;
  if (controller.transform) {
    controller.transform->SetTransform(aTransform);
  }
}

void
ControllerContainer::SetButtonCount(const int32_t aControllerIndex, const uint32_t aNumButtons) {
  if (!m.Contains(aControllerIndex)) {
    return;
  }
  m.list[aControllerIndex].numButtons = aNumButtons;
}

void
ControllerContainer::SetButtonState(const int32_t aControllerIndex, const Button aWhichButton, const int32_t aImmersiveIndex, const bool aPressed, const bool aTouched, const float aImmersiveTrigger) {
  assert(kControllerMaxButtonCount > aImmersiveIndex
         && "Button index must < kControllerMaxButtonCount.");

  if (!m.Contains(aControllerIndex)) {
    return;
  }

  const int32_t immersiveButtonMask = 1 << aImmersiveIndex;

  if (aPressed) {
    m.list[aControllerIndex].buttonState |= aWhichButton;
  } else {
    m.list[aControllerIndex].buttonState &= ~aWhichButton;
  }

  if (aImmersiveIndex >= 0) {
    if (aPressed) {
      m.list[aControllerIndex].immersivePressedState |= immersiveButtonMask;
    } else {
      m.list[aControllerIndex].immersivePressedState &= ~immersiveButtonMask;
    }

    if (aTouched) {
      m.list[aControllerIndex].immersiveTouchedState |= immersiveButtonMask;
    } else {
      m.list[aControllerIndex].immersiveTouchedState &= ~immersiveButtonMask;
    }

    float trigger = aImmersiveTrigger;
    if (trigger < 0.0f) {
      trigger = aPressed ? 1.0f : 0.0f;
    }
    m.list[aControllerIndex].immersiveTriggerValues[aImmersiveIndex] = trigger;
  }
}

void
ControllerContainer::SetAxes(const int32_t aControllerIndex, const float* aData, const uint32_t aLength) {
  assert(kControllerMaxAxes >= aLength
         && "Axis length must <= kControllerMaxAxes.");

  if (!m.Contains(aControllerIndex)) {
    return;
  }

  m.list[aControllerIndex].numAxes = aLength;
  for (int i = 0; i < aLength; ++i) {
    m.list[aControllerIndex].immersiveAxes[i] = aData[i];
  }
}

void
ControllerContainer::SetLeftHanded(const int32_t aControllerIndex, const bool aLeftHanded) {
  if (!m.Contains(aControllerIndex)) {
    return;
  }

  m.list[aControllerIndex].leftHanded = aLeftHanded;
}

void
ControllerContainer::SetTouchPosition(const int32_t aControllerIndex, const float aTouchX, const float aTouchY) {
  if (!m.Contains(aControllerIndex)) {
    return;
  }
  Controller& controller = m.list[aControllerIndex];
  controller.touched = true;
  controller.touchX = aTouchX;
  controller.touchY = aTouchY;
}

void
ControllerContainer::EndTouch(const int32_t aControllerIndex) {
  if (!m.Contains(aControllerIndex)) {
    return;
  }
  m.list[aControllerIndex].touched = false;
}

void
ControllerContainer::SetScrolledDelta(const int32_t aControllerIndex, const float aScrollDeltaX, const float aScrollDeltaY) {
  if (!m.Contains(aControllerIndex)) {
    return;
  }
  Controller& controller = m.list[aControllerIndex];
  controller.scrollDeltaX = aScrollDeltaX;
  controller.scrollDeltaY = aScrollDeltaY;
}

void
ControllerContainer::SetAngularAcceleration(const int32_t aControllerIndex, const vrb::Vector &aVector) {
    if (!m.Contains(aControllerIndex)) {
        return;
    }
    Controller& controller = m.list[aControllerIndex];
    controller.angularAcceleration = aVector;
}

void
ControllerContainer::SetAngularVelocity(const int32_t aControllerIndex, const vrb::Vector &aVector) {
    if (!m.Contains(aControllerIndex)) {
        return;
    }
    Controller& controller = m.list[aControllerIndex];
    controller.angularVelocity = aVector;
}

void
ControllerContainer::SetLinearAcceleration(const int32_t aControllerIndex, const vrb::Vector &aVector) {
    if (!m.Contains(aControllerIndex)) {
        return;
    }
    Controller& controller = m.list[aControllerIndex];
    controller.linearAcceleration = aVector;
}

void
ControllerContainer::SetLinearVelocity(const int32_t aControllerIndex, const vrb::Vector &aVector) {
    if (!m.Contains(aControllerIndex)) {
        return;
    }
    Controller& controller = m.list[aControllerIndex];
    controller.linearVelocity = aVector;
}

void ControllerContainer::SetPointerColor(const vrb::Color& aColor) const {
  for (Controller& controller: m.list) {
    if (controller.beamParent) {
      GeometryPtr geometry = std::dynamic_pointer_cast<vrb::Geometry>(controller.beamParent->GetNode(0));
      if (geometry) {
        geometry->GetRenderState()->SetMaterial(aColor, aColor, vrb::Color(0.0f, 0.0f, 0.0f), 0.0f);
      }
    }
    if (controller.pointer) {
      controller.pointer->SetPointerColor(aColor);
    }
  }
}

void
ControllerContainer::SetVisible(const bool aVisible) {
  if (m.visible == aVisible) {
    return;
  }
  m.visible = aVisible;
  if (aVisible) {
    for (int i = 0; i < m.list.size(); ++i) {
      if (m.list[i].enabled) {
        m.root->ToggleChild(*m.list[i].transform, true);
      }
    }
  } else {
    m.root->ToggleAll(false);
  }
}

ControllerContainer::ControllerContainer(State& aState, vrb::CreationContextPtr& aContext) : m(aState) {
  m.Initialize(aContext);
}

ControllerContainer::~ControllerContainer() {
  if (m.root) {
    m.root->RemoveFromParents();
    m.root = nullptr;
  }
}

} // namespace crow