/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Copyright (c) 2022-2023 Belousov Oleg aka R1CBU
 */

#include "vol.h"
#include "msg.h"
#include "radio.h"
#include "main.h"
#include "params/params.h"
#include "voice.h"
#include "util.h"

static vol_mode_t   vol_mode = VOL_VOL;

void vol_update(int16_t diff, bool voice) {
    int32_t     x;
    float       f;
    char        *s;
    bool        b;
    int32_t     freq;

    uint32_t    color = vol->mode == VOL_EDIT ? 0xFFFFFF : 0xBBBBBB;

    switch (vol_mode) {
        case VOL_VOL:
            x = radio_change_vol(diff);
            msg_set_text_fmt("#%3X Volume: %i", color, x);

            if (diff) {
                voice_say_int("Audio level", x);
            } else if (voice) {
                voice_say_text_fmt("Audio level");
            }
            break;

        case VOL_RFG:
            x = radio_change_rfg(diff);
            msg_set_text_fmt("#%3X RF gain: %i", color, x);

            if (diff) {
                voice_say_int("RF gain", x);
            } else if (voice) {
                voice_say_text_fmt("RF gain");
            }
            break;

        case VOL_SQL:
            x = radio_change_sql(diff);
            msg_set_text_fmt("#%3X Voice SQL: %i", color, x);

            if (diff) {
                voice_say_int("Squelch level %i", x);
            } else if (voice) {
                voice_say_text_fmt("Squelch level");
            }
            break;

        case VOL_FILTER_LOW:
            freq = params_current_mode_filter_low_get();
            if (diff) {
                // TODO: make step depending on freq
                freq = align_int(freq + diff * 10, 10);
            }
            x = radio_change_filter_low(freq);
            msg_set_text_fmt("#%3X Filter low: %i Hz", color, x);

            if (diff) {
                voice_delay_say_text_fmt("%i", x);
            } else if (voice) {
                voice_say_text_fmt("Low filter limit");
            }
            break;

        case VOL_FILTER_HIGH:
            freq = params_current_mode_filter_high_get();
            if (diff) {
                uint8_t freq_step;
                switch (radio_current_mode()) {
                case x6100_mode_cw:
                case x6100_mode_cwr:
                    freq_step = 10;
                    break;
                default:
                    freq_step = 50;
                    break;
                }
                freq = align_int(freq + diff * freq_step, freq_step);
            }
            x = radio_change_filter_high(freq);
            msg_set_text_fmt("#%3X Filter high: %i Hz", color, x);

            if (diff) {
                voice_say_int("High filter limit", x);
            } else if (voice) {
                voice_say_text_fmt("High filter limit");
            }
            break;

        case VOL_FILTER_BW:;
            uint32_t new_bw = params_current_mode_filter_bw_get();
            if (diff) {
                new_bw = align_int(new_bw + diff * 20, 20);
            }
            x = radio_change_filter_bw(new_bw);
            msg_set_text_fmt("#%3X Filter bw: %i Hz", color, x);

            if (diff) {
                voice_delay_say_text_fmt("%i", x);
            } else if (voice) {
                voice_say_text_fmt("Bandwidth filter limit");
            }
            break;

        case VOL_PWR:
            f = radio_change_pwr(diff);
            msg_set_text_fmt("#%3X Power: %0.1f W", color, f);

            if (diff) {
                voice_say_float("Transmit power", f);
            } else if (voice) {
                voice_say_text_fmt("Transmit power");
            }
            break;

        case VOL_MIC:
            x = radio_change_mic(diff);

            switch (x) {
                case x6100_mic_builtin:
                    s = "Built-In";
                    break;

                case x6100_mic_handle:
                    s = "Handle";
                    break;

                case x6100_mic_auto:
                    s = "Auto";
                    break;
            }

            msg_set_text_fmt("#%3X MIC: %s", color, s);

            if (diff) {
                voice_say_text("Mic selector", s);
            } else if (voice) {
                voice_say_text_fmt("Mic selector");
            }
            break;

        case VOL_HMIC:
            x = radio_change_hmic(diff);
            msg_set_text_fmt("#%3X H-MIC gain: %i", color, x);

            if (diff) {
                voice_say_int("Hand microphone gain", x);
            } else if (voice) {
                voice_say_text_fmt("Hand microphone gain");
            }
            break;

        case VOL_IMIC:
            x = radio_change_imic(diff);
            msg_set_text_fmt("#%3X I-MIC gain: %i", color, x);

            if (diff) {
                voice_say_int("Internal microphone gain", x);
            } else if (voice) {
                voice_say_text_fmt("Internal microphone gain");
            }
            break;

        case VOL_MONI:
            x = radio_change_moni(diff);
            msg_set_text_fmt("#%3X Moni level: %i", color, x);

            if (diff) {
                voice_say_int("Monitor level", x);
            } else if (voice) {
                voice_say_text_fmt("Monitor level");
            }
            break;

        case VOL_SPMODE:
            b = radio_change_spmode(diff);
            msg_set_text_fmt("#%3X Speaker mode: %s", color, b ? "On" : "Off");
            break;

        case VOL_VOICE_LANG:
            s = voice_change(diff);
            msg_set_text_fmt("#%3X Voice: %s", color, s);

            if (diff) {
                voice_say_lang();
            } else if (voice) {
                voice_say_text_fmt("Voice selector");
            }
            break;

        case VOL_VOICE_RATE:
            x = params_uint8_change(&params.voice_rate, diff);
            msg_set_text_fmt("#%3X Voice rate: %i", color, x);

            if (diff == 0 && voice) {
                voice_say_text_fmt(params.voice_rate.voice);
            }
            break;

        case VOL_VOICE_PITCH:
            x = params_uint8_change(&params.voice_pitch, diff);
            msg_set_text_fmt("#%3X Voice pitch: %i", color, x);

            if (diff == 0 && voice) {
                voice_say_text_fmt(params.voice_pitch.voice);
            }
            break;

        case VOL_VOICE_VOLUME:
            x = params_uint8_change(&params.voice_volume, diff);
            msg_set_text_fmt("#%3X Voice volume: %i", color, x);

            if (diff == 0 && voice) {
                voice_say_text_fmt(params.voice_volume.voice);
            }
            break;

        default:
            break;
    }
}

void vol_change_mode(int16_t dir) {
    vol_mode = loop_modes(dir, vol_mode, params.vol_modes, VOL_LAST-1);
    vol_update(0, true);
}

void vol_set_mode(vol_mode_t mode) {
    vol_mode = mode;
    vol->mode = VOL_EDIT;
}
