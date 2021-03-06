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
#include "grapheditor.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVariant>

#include "ui/keyframenavigator.h"
#include "ui/timelineheader.h"
#include "ui/labelslider.h"
#include "ui/graphview.h"
#include "project/effect.h"
#include "project/effectfield.h"
#include "project/effectrow.h"
#include "project/clip.h"
#include "panelmanager.h"
#include "debug.h"

constexpr auto ROW_DESC_FMT = "%1::%2::%3";
constexpr auto RECORD_ICON_RSC = ":/icons/record.png";
constexpr auto WINDOW_TITLE = "Graph Editor";
constexpr int WINDOW_WIDTH = 720;
constexpr int WINDOW_HEIGHT = 480;

GraphEditor::GraphEditor(QWidget* parent)
  : QDockWidget(parent)
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  setWindowTitle(tr(WINDOW_TITLE));
  resize(WINDOW_WIDTH, WINDOW_HEIGHT);

  auto* main_widget = new QWidget();
  setWidget(main_widget);
  auto* layout = new QVBoxLayout();
  main_widget->setLayout(layout);

  auto* tool_widget = new QWidget();
  tool_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
  auto* tools = new QHBoxLayout();
  tool_widget->setLayout(tools);

  auto* left_tool_widget = new QWidget();
  auto* left_tool_layout = new QHBoxLayout();
  left_tool_layout->setSpacing(0);
  left_tool_layout->setMargin(0);
  left_tool_widget->setLayout(left_tool_layout);
  tools->addWidget(left_tool_widget);
  auto* center_tool_widget = new QWidget();
  auto* center_tool_layout = new QHBoxLayout();
  center_tool_layout->setSpacing(0);
  center_tool_layout->setMargin(0);
  center_tool_widget->setLayout(center_tool_layout);
  tools->addWidget(center_tool_widget);
  auto right_tool_widget = new QWidget();
  auto right_tool_layout = new QHBoxLayout();
  right_tool_layout->setSpacing(0);
  right_tool_layout->setMargin(0);
  right_tool_widget->setLayout(right_tool_layout);
  tools->addWidget(right_tool_widget);

  keyframe_nav = new KeyframeNavigator();
  keyframe_nav->enable_keyframes(true);
  keyframe_nav->enable_keyframe_toggle(false);
  left_tool_layout->addWidget(keyframe_nav);
  left_tool_layout->addStretch();

  linear_button = new QPushButton(tr("Linear"));
  linear_button->setProperty("type", static_cast<int>(KeyframeType::LINEAR));
  linear_button->setCheckable(true);
  bezier_button = new QPushButton(tr("Bezier"));
  bezier_button->setProperty("type", static_cast<int>(KeyframeType::BEZIER));
  bezier_button->setCheckable(true);
  hold_button = new QPushButton(tr("Hold"));
  hold_button->setProperty("type", static_cast<int>(KeyframeType::HOLD));
  hold_button->setCheckable(true);

  center_tool_layout->addStretch();
  center_tool_layout->addWidget(linear_button);
  center_tool_layout->addWidget(bezier_button);
  center_tool_layout->addWidget(hold_button);

  layout->addWidget(tool_widget);

  auto* central_widget = new QWidget();
  auto* central_layout = new QVBoxLayout();
  central_widget->setLayout(central_layout);
  central_layout->setSpacing(0);
  central_layout->setMargin(0);
  header = new TimelineHeader();
  header->viewer_ = &panels::PanelManager::sequenceViewer();
  central_layout->addWidget(header);
  view = new GraphView();
  central_layout->addWidget(view);

  layout->addWidget(central_widget);

  auto* value_widget = new QWidget();
  value_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
  auto* values = new QHBoxLayout();
  value_widget->setLayout(values);
  values->addStretch();

  auto* central_value_widget = new QWidget();
  value_layout = new QHBoxLayout();
  value_layout->setMargin(0);
  value_layout->addWidget(new QLabel("")); // a spacer so the layout doesn't jump
  central_value_widget->setLayout(value_layout);
  values->addWidget(central_value_widget);

  values->addStretch();
  layout->addWidget(value_widget);

  current_row_desc = new QLabel();
  current_row_desc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
  current_row_desc->setAlignment(Qt::AlignCenter);
  layout->addWidget(current_row_desc);

  connect(view, SIGNAL(zoom_changed(double)), header, SLOT(update_zoom(double)));
  connect(view, SIGNAL(x_scroll_changed(int)), header, SLOT(set_scroll(int)));
  connect(view, &GraphView::selection_changed, this, &GraphEditor::set_key_button_enabled);

  connect(linear_button, SIGNAL(clicked(bool)), this, SLOT(set_keyframe_type()));
  connect(bezier_button, SIGNAL(clicked(bool)), this, SLOT(set_keyframe_type()));
  connect(hold_button, SIGNAL(clicked(bool)), this, SLOT(set_keyframe_type()));
}

void GraphEditor::update_panel() {
  if (isVisible()) {
    if (row != nullptr) {
      int slider_index = 0;
      for (int i=0;i<row->fieldCount();i++) {
        EffectField* field = row->field(i);
        if (field->type_ == EffectFieldType::DOUBLE) {
          slider_proxies.at(slider_index)->set_value(row->field(i)->get_current_data().toDouble(), false);
          slider_index++;
        }
      }
    }

    header->update();
    view->update();
  }
}

void GraphEditor::set_row(EffectRow *r)
{
  for (int i=0;i<slider_proxies.size();i++) {
    delete slider_proxies.at(i);
    delete slider_proxy_buttons.at(i);
  }
  slider_proxies.clear();
  slider_proxy_buttons.clear();
  slider_proxy_sources.clear();

  if (row != nullptr) {
    // clear old row connections
    disconnect(keyframe_nav, SIGNAL(goto_previous_key()), row, SLOT(goto_previous_key()));
    disconnect(keyframe_nav, SIGNAL(toggle_key()), row, SLOT(toggle_key()));
    disconnect(keyframe_nav, SIGNAL(goto_next_key()), row, SLOT(goto_next_key()));
  }

  bool found_vals = false;

  if (r != nullptr && r->isKeyframing()) {
    for (int i=0;i<r->fieldCount();i++) {
      EffectField* field = r->field(i);
      if (field->type_ == EffectFieldType::DOUBLE) {
        auto* slider_button = new QPushButton();
        slider_button->setCheckable(true);
        slider_button->setChecked(true);
        slider_button->setIcon(QIcon(RECORD_ICON_RSC));
        slider_button->setProperty("field", i);
        slider_button->setIconSize(QSize(8, 8));
        slider_button->setMaximumSize(QSize(12, 12));
        connect(slider_button, SIGNAL(toggled(bool)), this, SLOT(set_field_visibility(bool)));
        slider_proxy_buttons.append(slider_button);
        value_layout->addWidget(slider_button);

        auto* slider = new LabelSlider();
        slider->set_color(get_curve_color(i, r->fieldCount()).name());
        connect(slider, SIGNAL(valueChanged()), this, SLOT(passthrough_slider_value()));
        slider_proxies.append(slider);
        value_layout->addWidget(slider);

        slider_proxy_sources.append(dynamic_cast<LabelSlider*>(field->ui_element));

        found_vals = true;
      }
    }
  }

  if (found_vals && (r != nullptr) && (r->parent_effect != nullptr) ) {
    row = r;
    QString fmt(ROW_DESC_FMT);
    const QString desc = fmt.arg(row->parent_effect->parent_clip->timeline_info.name_,
                                 row->parent_effect->meta.name,
                                 row->get_name());
    current_row_desc->setText(desc);
    header->set_visible_in(row->parent_effect->parent_clip->timeline_info.in);

    connect(keyframe_nav, SIGNAL(goto_previous_key()), row, SLOT(goto_previous_key()));
    connect(keyframe_nav, SIGNAL(toggle_key()), row, SLOT(toggle_key()));
    connect(keyframe_nav, SIGNAL(goto_next_key()), row, SLOT(goto_next_key()));
  } else {
    row = nullptr;
    current_row_desc->setText("");
  }
  view->set_row(row);
  update_panel();
}

bool GraphEditor::view_is_focused() {
  return view->hasFocus() || header->hasFocus();
}

bool GraphEditor::view_is_under_mouse() {
  return view->underMouse() || header->underMouse();
}

void GraphEditor::delete_selected_keys() {
  view->delete_selected_keys();
}

void GraphEditor::select_all() {
  view->select_all();
}

void GraphEditor::set_key_button_enabled(bool e, const KeyframeType type)
{
  linear_button->setEnabled(e);
  linear_button->setChecked(type == KeyframeType::LINEAR);
  bezier_button->setEnabled(e);
  bezier_button->setChecked(type == KeyframeType::BEZIER);
  hold_button->setEnabled(e);
  hold_button->setChecked(type == KeyframeType::HOLD);
}

void GraphEditor::passthrough_slider_value() {
  for (int i=0;i<slider_proxies.size();i++) {
    if (slider_proxies.at(i) == sender()) {
      slider_proxy_sources.at(i)->set_value(slider_proxies.at(i)->value(), true);
    }
  }
}

void GraphEditor::set_keyframe_type()
{
  linear_button->setChecked(linear_button == sender());
  bezier_button->setChecked(bezier_button == sender());
  hold_button->setChecked(hold_button == sender());
  view->set_selected_keyframe_type(static_cast<KeyframeType>(sender()->property("type").toInt()));
}

void GraphEditor::set_field_visibility(bool b) {
  view->set_field_visibility(sender()->property("field").toInt(), b);
}
