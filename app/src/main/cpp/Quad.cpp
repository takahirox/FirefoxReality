/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Quad.h"
#include "vrb/ConcreteClass.h"

#include "vrb/Color.h"
#include "vrb/Context.h"
#include "vrb/Matrix.h"
#include "vrb/Geometry.h"
#include "vrb/RenderState.h"
#include "vrb/SurfaceTextureFactory.h"
#include "vrb/TextureSurface.h"
#include "vrb/Toggle.h"
#include "vrb/Transform.h"
#include "vrb/Vector.h"
#include "vrb/VertexArray.h"

namespace crow {

struct Quad::State {
  vrb::ContextWeak context;
  int32_t textureWidth;
  int32_t textureHeight;
  vrb::TogglePtr root;
  vrb::TransformPtr transform;
  vrb::GeometryPtr geometry;
  Quad::ScaleMode scaleMode;
  vrb::Vector worldMin;
  vrb::Vector worldMax;

  State()
      : textureWidth(0)
      , textureHeight(0)
      , scaleMode(ScaleMode::Fill)
      , worldMin(0.0f, 0.0f, 0.0f)
      , worldMax(0.0f, 0.0f, 0.0f)
  {}

  void Initialize() {
    geometry = Quad::CreateGeometry(context, worldMin, worldMax);
    transform = vrb::Transform::Create(context);
    transform->AddNode(geometry);
    root = vrb::Toggle::Create(context);
    root->AddNode(transform);
  }
};

QuadPtr
Quad::Create(vrb::ContextWeak aContext, const vrb::Vector& aMin, const vrb::Vector& aMax) {
  QuadPtr result = std::make_shared<vrb::ConcreteClass<Quad, Quad::State> >(aContext);
  result->m.worldMin = aMin;
  result->m.worldMax = aMax;
  result->m.Initialize();
  return result;
}

vrb::GeometryPtr
Quad::CreateGeometry(vrb::ContextWeak aContext, const vrb::Vector &aMin, const vrb::Vector &aMax) {
  vrb::VertexArrayPtr array = vrb::VertexArray::Create(aContext);
  const vrb::Vector bottomRight(aMax.x(), aMin.y(), aMin.z());
  array->AppendVertex(aMin); // Bottom left
  array->AppendVertex(bottomRight); // Bottom right
  array->AppendVertex(aMax); // Top right
  array->AppendVertex(vrb::Vector(aMin.x(), aMax.y(), aMax.z())); // Top left

  array->AppendUV(vrb::Vector(0.0f, 1.0f, 0.0f));
  array->AppendUV(vrb::Vector(1.0f, 1.0f, 0.0f));
  array->AppendUV(vrb::Vector(1.0f, 0.0f, 0.0f));
  array->AppendUV(vrb::Vector(0.0f, 0.0f, 0.0f));

  vrb::Vector normal = (bottomRight - aMin).Cross(aMax - aMin).Normalize();
  array->AppendNormal(normal);

  vrb::RenderStatePtr state = vrb::RenderState::Create(aContext);
  vrb::GeometryPtr geometry = vrb::Geometry::Create(aContext);
  geometry->SetVertexArray(array);
  geometry->SetRenderState(state);

  std::vector<int> index;
  index.push_back(1);
  index.push_back(2);
  index.push_back(3);
  index.push_back(4);
  std::vector<int> normalIndex;
  normalIndex.push_back(1);
  normalIndex.push_back(1);
  normalIndex.push_back(1);
  normalIndex.push_back(1);
  geometry->AddFace(index, index, normalIndex);

  // Draw the back for now
  index.clear();
  index.push_back(1);
  index.push_back(4);
  index.push_back(3);
  index.push_back(2);

  array->AppendNormal(-normal);
  normalIndex.clear();
  normalIndex.push_back(2);
  normalIndex.push_back(2);
  normalIndex.push_back(2);
  normalIndex.push_back(2);
  geometry->AddFace(index, index, normalIndex);

  return geometry;
}

void
Quad::SetTexture(const vrb::TextureSurfacePtr& aTexture, int32_t aWidth, int32_t aHeight) {
  m.textureWidth = aWidth;
  m.textureHeight = aHeight;
  m.geometry->GetRenderState()->SetTexture(aTexture);
}

void
Quad::SetMaterial(const vrb::Color& aAmbient, const vrb::Color& aDiffuse, const vrb::Color& aSpecular, const float aSpecularExponent) {
  m.geometry->GetRenderState()->SetMaterial(aAmbient, aDiffuse, aSpecular, aSpecularExponent);
}

void
Quad::GetTextureSize(int32_t& aWidth, int32_t& aHeight) const {
  aWidth = m.textureWidth;
  aHeight = m.textureHeight;
}

void
Quad::GetWorldMinAndMax(vrb::Vector& aMin, vrb::Vector& aMax) const {
  aMin = m.worldMin;
  aMax = m.worldMax;
}

const vrb::Vector&
Quad::GetWorldMin() const {
  return m.worldMin;
}

const vrb::Vector&
Quad::GetWorldMax() const {
  return m.worldMax;
}

void
Quad::GetWorldSize(float& aWidth, float& aHeight) const {
  aWidth = m.worldMax.x() - m.worldMin.x();
  aHeight = m.worldMax.y() - m.worldMin.y();
}

vrb::Vector
Quad::GetNormal() const {
  return m.geometry->GetVertexArray()->GetNormal(0);
}

vrb::NodePtr
Quad::GetRoot() const {
  return m.root;
}

vrb::TransformPtr
Quad::GetTransformNode() const {
  return m.transform;
}

static const float kEpsilon = 0.00000001f;

bool
Quad::TestIntersection(const vrb::Vector& aStartPoint, const vrb::Vector& aDirection, vrb::Vector& aResult, bool aClamp, bool& aIsInside, float& aDistance) const {
  aDistance = -1.0f;
  if (!m.root->IsEnabled(*m.transform)) {
    return false;
  }
  vrb::Matrix modelView = m.transform->GetWorldTransform().AfineInverse();
  vrb::Vector point = modelView.MultiplyPosition(aStartPoint);
  vrb::Vector direction = modelView.MultiplyDirection(aDirection);
  vrb::Vector normal = GetNormal();
  const float dotNormals = direction.Dot(normal);
  if (dotNormals > -kEpsilon) {
    // Not pointed at the plane
    return false;
  }

  const float dotV = (m.worldMin - point).Dot(normal);

  if ((dotV < kEpsilon) && (dotV > -kEpsilon)) {
    return false;
  }

  const float length = dotV / dotNormals;
  vrb::Vector result = point + (direction * length);

  if ((result.x() >= m.worldMin.x()) && (result.y() >= m.worldMin.y()) &&(result.z() >= (m.worldMin.z() - 0.1f)) &&
      (result.x() <= m.worldMax.x()) && (result.y() <= m.worldMax.y()) &&(result.z() <= (m.worldMax.z() + 0.1f))) {

    aIsInside = true;
  }

  aResult = result;

  aDistance = (aResult - point).Magnitude();

  // Clamp to keep pointer in quad.
  if (aClamp) {
    if (result.x() > m.worldMax.x()) { result.x() = m.worldMax.x(); }
    else if (result.x() < m.worldMin.x()) { result.x() = m.worldMin.x(); }

    if (result.y() > m.worldMax.y()) { result.y() = m.worldMax.y(); }
    else if (result.y() < m.worldMin.y()) { result.y() = m.worldMin.y(); }
  }

  return true;
}

void
Quad::ConvertToQuadCoordinates(const vrb::Vector& point, float& aX, float& aY) const {
  vrb::Vector value = point;
  // Clamp value to window bounds.
  if (value.x() > m.worldMax.x()) { value.x() = m.worldMax.x(); }
  else if (value.x() < m.worldMin.x()) { value.x() = m.worldMin.x(); }
  if (value.y() > m.worldMax.y()) { value.y() = m.worldMax.y(); }
  else if (value.y() < m.worldMin.y()) { value.y() = m.worldMin.y(); }

  // Convert to window coordinates.
  aX = (((value.x() - m.worldMin.x()) / (m.worldMax.x() - m.worldMin.x())) * (float)m.textureWidth);
  aY = (((m.worldMax.y() - value.y()) / (m.worldMax.y() - m.worldMin.y())) * (float)m.textureHeight);
}

Quad::Quad(State& aState, vrb::ContextWeak& aContext) : m(aState) {
  m.context = aContext;
}

Quad::~Quad() {}

} // namespace crow
