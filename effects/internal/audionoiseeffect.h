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
#ifndef AUDIONOISEEFFECT_H
#define AUDIONOISEEFFECT_H

#include "project/effect.h"

class AudioNoiseEffect : public Effect {
public:
	AudioNoiseEffect(Clip* c, const EffectMeta* em);
    virtual void process_audio(double timecode_start, double timecode_end, quint8* samples, int nb_bytes, int channel_count);

	EffectField* amount_val;
	EffectField* mix_val;
};

#endif // AUDIONOISEEFFECT_H
