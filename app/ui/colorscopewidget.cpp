/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
 * Copyright (C) 2019
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "colorscopewidget.h"

#include <QPainter>
#include <QPen>
#include <cmath>

#include "io/colorconversions.h"


constexpr int HORIZONTAL_STEP = 8;
constexpr int PEN_ALPHA = 24;
constexpr int MINOR_GRID_STEP = 8;
constexpr int MAJOR_GRID_STEP = MINOR_GRID_STEP / 2;

namespace {
  const QPen r_pen(QColor(255,0,0,PEN_ALPHA));
  const QPen g_pen(QColor(0,255,0,PEN_ALPHA));
  const QPen b_pen(QColor(0,0,255,PEN_ALPHA));
  const QPen bk_pen(Qt::black);
  const QPen bka_pen(QColor(0,0,0, 128));
  const QPen luma_pen(QColor(160,160,160, PEN_ALPHA));
}

using ui::ColorScopeWidget;
using io::color_conversion::rgbToLuma;

ColorScopeWidget::ColorScopeWidget(QWidget *parent) : QWidget(parent)
{
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

/**
 * @brief Update the image used to draw scope
 * @param img Image to be drawn from
 */
void ColorScopeWidget::updateImage(QImage img)
{
  img_ = std::move(img);
}


void ColorScopeWidget::paintEvent(QPaintEvent*/*event*/)
{
  QPainter painter(this);
  // clear last paint
  painter.eraseRect(0, 0, width(), img_.height());

  const auto w_step = static_cast<double>(img_.width()) / width();
  const auto h_step = static_cast<double>(height()) / 256;
  QRgb val;

  // FIXME: far too slow to be done for every pixel
  // with QImage::setPixel the blending of values is lost
  if (mode_ == 1) {
    painter.setPen(luma_pen);
  }
  for (auto w = 0; w < width(); ++w) {
    for (auto h = 0; h < img_.height(); h+=HORIZONTAL_STEP) {
      // draw pixel value (per channel) on y-axis at x-position
      val = img_.pixel(static_cast<int32_t>(lround(w * w_step)), h);
      if (mode_ == 0) {
        painter.setPen(r_pen);
        painter.drawPoint(w, height() - static_cast<int32_t>(lround(qRed(val)*h_step)));
        painter.setPen(g_pen);
        painter.drawPoint(w, height() - static_cast<int32_t>(lround(qGreen(val)*h_step)));
        painter.setPen(b_pen);
        painter.drawPoint(w, height() - static_cast<int32_t>(lround(qBlue(val)*h_step)));
      } else if (mode_ == 1) {
        painter.drawPoint(w, height() - static_cast<int32_t>(lround(rgbToLuma(val)*h_step)));
      }
    }
  }

  // paint surrounding box
  painter.setPen(bk_pen);
  painter.drawRect(0, 0, width() -1, height()-1);

  // FIXME: major lines do not get drawn on heights of odd numbers
  // paint grid-lines
  QVector<qreal> dashes;
  dashes << 3 << 3;
  QPen minor_pen(bka_pen);
  minor_pen.setDashPattern(dashes);
  const int32_t major_step = static_cast<int32_t>(lround(static_cast<double>(height()) / MAJOR_GRID_STEP));
  const int32_t minor_step = static_cast<int32_t>(lround(static_cast<double>(height()) / MINOR_GRID_STEP));

  for (auto h = minor_step; h < height(); h+=minor_step) {
    if (h % major_step == 0) {
      painter.setPen(bka_pen);
    } else {
      painter.setPen(minor_pen);
    }
    painter.drawLine(1, height() - h, width() - 1, height() -  h);
  }
}
