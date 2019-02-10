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
#ifndef PREVIEWGENERATOR_H
#define PREVIEWGENERATOR_H

#include <QThread>
#include <QSemaphore>
#include <memory>

#include "project/footage.h"
#include "project/media.h"

#define ICON_TYPE_VIDEO 0
#define ICON_TYPE_AUDIO 1
#define ICON_TYPE_IMAGE 2
#define ICON_TYPE_ERROR 3

struct FootageStream;
struct AVFormatContext;

class PreviewGenerator : public QThread
{
    Q_OBJECT
public:
    PreviewGenerator(MediaPtr item, FootagePtr ftg, const bool replacing, QObject* parent=nullptr);
    virtual ~PreviewGenerator();

    PreviewGenerator() = delete;
    PreviewGenerator(const PreviewGenerator&) = delete;
    PreviewGenerator(const PreviewGenerator&&) = delete;
    PreviewGenerator& operator=(const PreviewGenerator&) = delete;
    PreviewGenerator& operator=(const PreviewGenerator&&) = delete;

    void cancel();
protected:
    virtual void run();
signals:
    void set_icon(int, bool);
private:
    void parse_media();
    /**
     * @brief retrieve pre-existing (file-system) previews
     * @param hash used to generate file_path
     * @return  true==already exists and loaded in FootageStream of Media (groan)
     */
    bool retrieve_preview(const QString &hash);
    void generate_waveform();
    void finalize_media();
    AVFormatContext* fmt_ctx;
    MediaPtr media;
    FootageWPtr footage;
    bool retrieve_duration;
    bool contains_still_image;
    bool replace;
    bool cancelled;
    QString data_path;
    QString get_thumbnail_path(const QString &hash, FootageStreamPtr& ms);
    QString get_waveform_path(const QString& hash, FootageStreamPtr& ms);
};

using PreviewGeneratorPtr = std::shared_ptr<PreviewGenerator>;
using PreviewGeneratorUPtr = std::unique_ptr<PreviewGenerator>;
using PreviewGeneratorWPtr = std::weak_ptr<PreviewGenerator>;
#endif // PREVIEWGENERATOR_H
