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
#ifndef AUDIO_H
#define AUDIO_H

#include <QVector>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>


constexpr int AUDIO_IBUFFER_SIZE = 192000;

class QIODevice;
class QAudioOutput;



class AudioSenderThread : public QThread {
	Q_OBJECT
public:
	AudioSenderThread();
	void run();
	void stop();
	QWaitCondition cond;
	bool close;
	QMutex lock;
public slots:
	void notifyReceiver();
private:
	QVector<qint16> samples;
	int send_audio_to_output(int offset, int max);
};

//FIXME: christ almighty. Get rid of the globals, somehow.
extern QAudioOutput* audio_output;
extern QIODevice* audio_io_device;
extern AudioSenderThread* audio_thread;
extern QMutex audio_write_lock;

extern qint8 audio_ibuffer[AUDIO_IBUFFER_SIZE];
extern int audio_ibuffer_read;
extern long audio_ibuffer_frame;
extern double audio_ibuffer_timecode;
extern bool audio_scrub;
extern bool recording;
extern bool audio_rendering;


void clear_audio_ibuffer();

int current_audio_freq();

bool is_audio_device_set();

void init_audio();
void stop_audio();
int get_buffer_offset_from_frame(double framerate, long frame);

bool start_recording();
void stop_recording();
QString get_recorded_audio_filename();

#endif // AUDIO_H
