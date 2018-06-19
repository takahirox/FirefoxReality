/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "WidgetResizer.h"
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

struct ResizeBar;

typedef std::shared_ptr<ResizeBar> ResizeBarPtr;

static const float kBarSize = 0.04f;
static const float kHandleRadius = 0.08f;
static vrb::Color kDefaultColor(0x2BD5D5FF);
static vrb::Color kActiveColor(0xF7CE4D);


struct ResizeBar {
  static ResizeBarPtr Create(vrb::ContextWeak& aContext, const vrb::Vector& aCenter, const vrb::Vector& aScale) {
    auto result = std::make_shared<ResizeBar>();
    result->center = aCenter;
    result->scale = aScale;
    vrb::Vector max(kBarSize * 0.5f, kBarSize * 0.5f, 0.0f);
    result->geometry = Quad::CreateGeometry(aContext, -max, max);
    result->transform = vrb::Transform::Create(aContext);
    result->transform->AddNode(result->geometry);
    result->SetActive(false);
    return result;
  }

  void SetActive(bool aActive) {
    geometry->GetRenderState()->SetMaterial(vrb::Color(0.5f, 0.5f, 0.5f), aActive ? kActiveColor : kDefaultColor, vrb::Color(0.0f, 0.0f, 0.0f), 0.0f);
  }

  vrb::Vector center;
  vrb::Vector scale;
  vrb::GeometryPtr geometry;
  vrb::TransformPtr transform;
};


struct ResizeHandle;
typedef std::shared_ptr<ResizeHandle> ResizeHandlePtr;

struct ResizeHandle {
  enum class ResizeMode {
    Horizontal,
    Vertical,
    Both
  };

  static ResizeHandlePtr Create(vrb::ContextWeak& aContext, const vrb::Vector& aCenter, ResizeMode aResizeMode, const std::vector<ResizeBarPtr>& aAttachedBars) {
    auto result = std::make_shared<ResizeHandle>();
    result->center = aCenter;
    result->resizeMode = aResizeMode;
    result->attachedBars = aAttachedBars;
    vrb::Vector max(kHandleRadius, kHandleRadius, 0.0f);
    result->geometry = ResizeHandle::CreateGeometry(aContext);
    result->transform = vrb::Transform::Create(aContext);
    result->transform->AddNode(result->geometry);
    result->SetActive(false);
    return result;
  }

  void SetActive(bool aActive) {
    geometry->GetRenderState()->SetMaterial(vrb::Color(0.5f, 0.5f, 0.5f), aActive ? kActiveColor : kDefaultColor, vrb::Color(0.0f, 0.0f, 0.0f), 0.0f);
  }

  static vrb::GeometryPtr CreateGeometry(vrb::ContextWeak& aContext) {

    const vrb::Vector center(0.0f, 0.0f, 0.0f);


    vrb::VertexArrayPtr array = vrb::VertexArray::Create(aContext);
    array->AppendVertex(center);
    array->AppendNormal(vrb::Vector(0.0f, 0.0f, 1.0f));

    std::vector<int> index;
    std::vector<int> normalIndex;

    const int kSides = 30;
    double delta = 2.0 * M_PI / kSides;
    for (int i = 0; i < kSides; ++i) {
      const double angle = delta * i;
      array->AppendVertex(vrb::Vector(kHandleRadius * (float)cos(angle), kHandleRadius * (float)sin(angle), 0.0f));
      if (i > 0) {
        index.push_back(1);
        index.push_back(i + 1);
        index.push_back(i + 2);
        normalIndex.push_back(1);
        normalIndex.push_back(1);
        normalIndex.push_back(1);
      }
    }

    index.push_back(1);
    index.push_back(array->GetVertexCount());
    index.push_back(2);
    normalIndex.push_back(1);
    normalIndex.push_back(1);
    normalIndex.push_back(1);

    vrb::GeometryPtr geometry = vrb::Geometry::Create(aContext);
    vrb::RenderStatePtr state = vrb::RenderState::Create(aContext);
    geometry->SetVertexArray(array);
    geometry->SetRenderState(state);
    geometry->AddFace(index, index, normalIndex);


    return geometry;
  }

  vrb::Vector center;
  ResizeMode resizeMode;
  std::vector<ResizeBarPtr> attachedBars;
  vrb::GeometryPtr geometry;
  vrb::TransformPtr transform;
};

struct WidgetResizer::State {
  vrb::ContextWeak context;
  vrb::Vector min;
  vrb::Vector max;
  bool resizing;
  vrb::TogglePtr root;
  std::vector<ResizeHandlePtr> resizeHandles;
  std::vector<ResizeBarPtr> resizeBars;
  vrb::TransformPtr resizeBackground;

  State()
      : resizing(false)
  {}

  void Initialize() {
    root = vrb::Toggle::Create(context);

    vrb::Vector horizontalSize(0.0f, 0.5f, 0.0f);
    vrb::Vector verticalSize(0.5f, 0.0f, 0.0f);
    ResizeBarPtr leftTop = CreateResizeBar(vrb::Vector(0.0f, 0.75f, 0.0f), horizontalSize);
    ResizeBarPtr leftBottom = CreateResizeBar(vrb::Vector(0.0f, 0.25f, 0.0f), horizontalSize);
    ResizeBarPtr rightTop = CreateResizeBar(vrb::Vector(1.0f, 0.75f, 0.0f), horizontalSize);
    ResizeBarPtr rightBottom = CreateResizeBar(vrb::Vector(1.0f, 0.25f, 0.0f), horizontalSize);
    ResizeBarPtr topLeft = CreateResizeBar(vrb::Vector(0.25f, 1.0f, 0.0f), verticalSize);
    ResizeBarPtr topRight = CreateResizeBar(vrb::Vector(0.75f, 1.0f, 0.0f), verticalSize);
    ResizeBarPtr bottomLeft = CreateResizeBar(vrb::Vector(0.25f, 0.0f, 0.0f), verticalSize);
    ResizeBarPtr bottomRight = CreateResizeBar(vrb::Vector(0.75f, 0.0f, 0.0f), verticalSize);

    CreateResizeHandle(vrb::Vector(0.0f, 1.0f, 0.0f), ResizeHandle::ResizeMode::Both, {leftTop, topLeft});
    CreateResizeHandle(vrb::Vector(1.0f, 1.0f, 0.0f), ResizeHandle::ResizeMode::Both, {rightTop, topRight});
    CreateResizeHandle(vrb::Vector(0.0f, 0.0f, 0.0f), ResizeHandle::ResizeMode::Both, {leftBottom, bottomLeft});
    CreateResizeHandle(vrb::Vector(1.0f, 0.0f, 0.0f), ResizeHandle::ResizeMode::Both, {rightBottom, bottomRight});
    CreateResizeHandle(vrb::Vector(0.5f, 1.0f, 0.0f), ResizeHandle::ResizeMode::Vertical, {topLeft, topRight});
    CreateResizeHandle(vrb::Vector(0.5f, 0.0f, 0.0f), ResizeHandle::ResizeMode::Vertical, {bottomLeft, bottomRight});
    CreateResizeHandle(vrb::Vector(0.0f, 0.5f, 0.0f), ResizeHandle::ResizeMode::Horizontal, {leftTop, leftBottom});
    CreateResizeHandle(vrb::Vector(1.0f, 0.5f, 0.0f), ResizeHandle::ResizeMode::Horizontal, {rightTop, rightBottom});

    UpdateResizeSize();
  }

  ResizeBarPtr CreateResizeBar(const vrb::Vector& aCenter, vrb::Vector aScale) {
    ResizeBarPtr result = ResizeBar::Create(context, aCenter, aScale);
    resizeBars.push_back(result);
    root->AddNode(result->transform);
    return result;
  }

  ResizeHandlePtr CreateResizeHandle(const vrb::Vector& aCenter, ResizeHandle::ResizeMode aResizeMode, const std::vector<ResizeBarPtr>& aBars) {
    ResizeHandlePtr result = ResizeHandle::Create(context, aCenter, aResizeMode, aBars);
    resizeHandles.push_back(result);
    root->AddNode(result->transform);
    return result;
  }

  float WorldWidth() const {
    return max.x() - min.x();
  }

  float WorldHeight() const {
    return max.y() - min.y();
  }


  void UpdateResizeSize() {
    const float width = WorldWidth();
    const float height = WorldHeight();

    for (ResizeBarPtr& bar: resizeBars) {
      float targetWidth = bar->scale.x() > 0.0f ? bar->scale.x() * fabsf(width) : kBarSize;
      float targetHeight = bar->scale.y() > 0.0f ? bar->scale.y() * fabs(height) : kBarSize;
      vrb::Matrix matrix = vrb::Matrix::Position(vrb::Vector(min.x() + width * bar->center.x(), min.y() + height * bar->center.y(), 0.005f));
      matrix.ScaleInPlace(vrb::Vector(targetWidth / kBarSize, targetHeight / kBarSize, 1.0f));
      bar->transform->SetTransform(matrix);
    }

    for (ResizeHandlePtr& handle: resizeHandles) {
      vrb::Matrix matrix = vrb::Matrix::Position(vrb::Vector(min.x() + width * handle->center.x(), min.y() + height * handle->center.y(), 0.005f));
      handle->transform->SetTransform(matrix);
    }
  }

  ResizeHandlePtr GetIntersectingHandler(const vrb::Vector& point) {
    for (const ResizeHandlePtr& handle: resizeHandles) {
      vrb::Vector worldCenter(min.x() + WorldWidth() * handle->center.x(), min.y() + WorldHeight() * handle->center.y(), 0.0f);
      float distance = (point - worldCenter).Magnitude();
      if (distance < kHandleRadius * 2.0f) {
        return handle;
      }
    }
    return nullptr;
  }
};

WidgetResizerPtr
WidgetResizer::Create(vrb::ContextWeak aContext, const vrb::Vector& aMin, const vrb::Vector& aMax) {
  WidgetResizerPtr result = std::make_shared<vrb::ConcreteClass<WidgetResizer, WidgetResizer::State> >(aContext);
  result->m.min = aMin;
  result->m.max = aMax;
  result->m.Initialize();
  return result;
}


vrb::NodePtr
WidgetResizer::GetRoot() const {
  return m.root;
}

void
WidgetResizer::ToggleVisible(bool aVisible) {
  m.root->ToggleAll(aVisible);
}

bool
WidgetResizer::TestIntersection(const vrb::Vector& point) const {
  vrb::Vector extraMin = vrb::Vector(m.min.x() - kBarSize * 0.5f, m.min.y() - kBarSize * 0.5f, 0.0f);
  vrb::Vector extraMax = vrb::Vector(m.max.x() + kBarSize * 0.5f, m.max.y() + kBarSize * 0.5f, 0.0f);

  if ((point.x() >= extraMin.x()) && (point.y() >= extraMin.y()) &&(point.z() >= (extraMin.z() - 0.1f)) &&
      (point.x() <= extraMax.x()) && (point.y() <= extraMax.y()) &&(point.z() <= (extraMax.z() + 0.1f))) {

    return true;
  }

  return m.GetIntersectingHandler(point).get() != nullptr;
}


WidgetResizer::WidgetResizer(State& aState, vrb::ContextWeak& aContext) : m(aState) {
  m.context = aContext;
}

WidgetResizer::~WidgetResizer() {}

} // namespace crow
