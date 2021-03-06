/* 
 * Olive. Olive is a free non-linear video editor for Windows, macOS, and Linux.
 * Copyright (C) 2018  {{ organization }}
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "effectgizmo.h"

#include "ui/labelslider.h"
#include "effectfield.h"

EffectGizmo::EffectGizmo(const GizmoType type) :
  type_(type),
  cursor(-1)
{
  int point_count = (type == GizmoType::POLY) ? 4 : 1;
  world_pos.resize(point_count);
  screen_pos.resize(point_count);

  color = Qt::white;
}

void EffectGizmo::set_previous_value()
{
  if (x_field1 != nullptr) dynamic_cast<LabelSlider*>(x_field1->ui_element)->set_previous_value();
  if (y_field1 != nullptr) dynamic_cast<LabelSlider*>(y_field1->ui_element)->set_previous_value();
  if (x_field2 != nullptr) dynamic_cast<LabelSlider*>(x_field2->ui_element)->set_previous_value();
  if (y_field2 != nullptr) dynamic_cast<LabelSlider*>(y_field2->ui_element)->set_previous_value();
}

int EffectGizmo::get_point_count() {
  return world_pos.size();
}

GizmoType EffectGizmo::get_type() const {
  return type_;
}

int EffectGizmo::get_cursor() const {
  return cursor;
}

void EffectGizmo::set_cursor(const int value) {
  cursor = value;
}
