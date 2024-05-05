#ifndef MINISOUNDSCAPE_H
#define MINISOUNDSCAPE_H

#include <stdarg.h> // variadic arguments
#include <vector>
#include "miniaudio.h"

/* --- utilities --- */

// map a value from an input range to an output range
#define MAP(x, in_min, in_max, out_min, out_max) ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)

// return a random number within a given range
#define RAND_IN_RANGE(start, end)                (MAP((float)(rand() % 100) / 100, -1.0f, 1.0f, start, end))

/* --- default macros --- */

#ifndef MS_SAMPLE_RATE
    #define MS_SAMPLE_RATE 44100
#endif

#ifndef MS_DEFAULT_FADE_AMOUNT_SECONDS
    #define MS_DEFAULT_FADE_AMOUNT_SECONDS 1.0
#endif

#ifndef MS_DEFAULT_FADE_AMOUNT
    #define MS_DEFAULT_FADE_AMOUNT MS_SAMPLE_RATE * MS_DEFAULT_FADE_AMOUNT_SECONDS
#endif

#ifndef MS_DEFAULT_TICK_RATE
    #define MS_DEFAULT_TICK_RATE 1.0
#endif

#ifndef MS_DEFAULT_FILETYPE
    #define MS_DEFAULT_FILETYPE WAV // WAV, MP3, or FLAC
#endif

#ifndef MS_DEFAULT_VOLUME
    #define MS_DEFAULT_VOLUME 1.0
#endif

/*

    Minisoundscape is an add-on for Miniaudio that adds utilites for developers to easily create automated soundscapes in their products.

    A soundscape is simply a collection of sounds. In Minisoundscape these are `ms_soundscape`s.

    A soundscape is simply a collection of sounds. Soundscapes contain
    1. An ambient sound (ma_sound) - this loops forever and generally sets the atmosphere
    2. Soundbites (ms_sound) - these are generally small sounds that can be spatialised (they can exist in 3D space) and are typically small, such as twigs breaking or a car driving by momentarily. They are played pseudo-randomly.

    To use a soundscape make sure you have all the sound files you want to use. You must have a decent understanding of pointers and understanding what heap and stack memory is will help too.

    These examples may not map directly onto your current setup.

    ```c
    // declare our ma_engine and an ms_soundscape - no need to declare any ms_sound objects here
    static ma_engine engine;
    static ms_soundscape soundscape;

    void setup() {
        ma_result result = ma_engine_init(NULL, &engine);
        if (result != MA_SUCCESS) printf("Failed to initialise engine.\n");

        // we declare our ms_sound objects as pointers here.
        // IMPORTANT: do not specify the file extension - Minisoundscape takes care of that for us.
        ms_sound* sound = new ms_sound;
        result = ms_sound_init("sound_name", &engine, 1, "file/path", &sound)
    }
    ```

    Developer macros:

    Optimisation/debug:
     - MS_VERBOSE                     | Prints status updates on what minisoundscape is doing, e.g. initialising or ticking a soundscape, playing a sound, loading a soundfile, etc.
     - MS_NO_SOUNDSCAPE               | Removes ms_soundscape related code. Useful if you only want the ms_sound objects
     - MS_NO_SPATIALIZATION           | Removes ms_origin_point related code. Useful if you aren't doing any spatialization!

    Customisation:
     - MS_SAMPLE_RATE                 | Sets the sample rate. Defaults to 44100.
     - MS_DEFAULT_TICK_RATE           | Sets the tick rate for each soundscape so that you don't have to do it manually - in seconds. Defaults to 1.0.
     - MS_DEFAULT_FILE_TYPE           | Sets the default file type for loading sounds so that you don't have to do it manually. Defaults to WAV. Can be set to MP3 or FLAC.
     - MS_DEFUALT_VOLUME              | Sets the default volume for sounds so that you don't have to do it manually. Defaults to 1.0.
     - MS_DEFAULT_FADE_AMOUNT_SECONDS | Sets the default amount of time when fading between soundscapes - in seconds. Defaults to 1.0. Doesn't change anything if you set MS_DEFAULT_FADE_AMOUNT manually
     - MS_DEFAULT_FADE_AMOUNT         | Sets the default amount of time when fading between soundscapes - in PCM frames. Defaults to the MS_SAMPLE_RATE * MS_DEFAULT_FADE_AMOUNT_SECONDS.

*/

typedef struct ms_sound         ms_sound;
typedef struct ms_soundscape    ms_soundscape;
typedef struct ms_sound_speaker ms_sound_speaker;

/* --- ms_sound --- */

struct ms_sound {
    std::string name;
    int weight;
    vector<ma_sound*> sounds;
    // ranges work as an array of size 2. the 0th item is the start of the range, and the 1st item is the end of the range
    // e.g. setting `pan_range` to `{ -0.5, 0.5 }` would mean that panning will be randomly chosen from -0.5 to 0.5
    float pan_range[2];
    float pitch_range[2];
    float volume_range[2];
    #ifndef MS_NO_SPATIALIZATION
    // the distance range is how far away may be on an axis. the first three slots of the array are the starting ranges for each dimension, and the last 3 slots are the end ranges for each dimension
    // the array is essentially [x1, y1, z1, x2, y2, z2]
    // this will only work if there are no speakers available! if the sound has speakers assigned to it those will be prioritised
    float distance_range[6];
    vector<ms_sound_speaker*> speakers;
    #endif
};

typedef enum {
    WAV,
    MP3,
    FLAC
} ms_sound_filetype;

/* --- ms_soundscape --- */

#ifndef MS_NO_SOUNDSCAPE
struct ms_soundscape {
    std::string name;
    ma_sound* ambient;
    ma_engine* engine;
    vector<ms_sound*> sounds;
    ma_uint64 timeSinceLastTick; // long long
    float tickrate;
};
#endif /*  MS_NO_SOUNDSCAPE */

/* --- ms_sound_speaker ---*/

#ifndef MS_NO_SPATIALIZATION
struct ms_sound_speaker {
    std::string name;
    double x;
    double y;
    double z;
    ms_sound* sound;
};
#endif /* MS_NO_SPATIALIZATION */

/* --- ms_sound --- */

void      ms_sound_init(std::string name, ma_engine* engine, unsigned int weight, std::string filepath, ms_sound* sound, ms_sound_filetype filetype = MS_DEFAULT_FILETYPE, bool enable_spatialization = true);
void      ms_sound_init_empty(ms_sound* sound, unsigned int weight);
void      ms_sound_uninit(ms_sound* sound);

bool      ms_sound_is_playing(const ms_sound* sound);

ma_result ms_sound_start(ms_sound* sound);
ma_result ms_sound_stop(const ms_sound* sound);

#ifndef MS_NO_SPATIALIZATION
void      ms_sound_set_spatialization(ms_sound* sound, bool spatialization);
void      ms_sound_add_speaker(ms_sound* sound, const unsigned int speakerAmount, ...);
void      ms_sound_add_speaker(ms_sound* sound, ms_sound_speaker* speaker);
void      ms_sound_set_position(ms_sound* sound, double x, double y, double z);
void      ms_sound_set_distance(ms_sound* sound, double x, double y, double z);
void      ms_sound_set_distance(ms_sound* sound, double x1, double y1, double z1, double x2, double y2, double z2);
#endif /* MS_NO_SPATIALIZATION */

void      ms_sound_set_volume(ms_sound* sound, float volume);
void      ms_sound_set_volume(ms_sound* sound, float start, float end);
void      ms_sound_set_pitch(ms_sound* sound, float pitch);
void      ms_sound_set_pitch(ms_sound* sound, float start, float end);
void      ms_sound_set_pan(ms_sound* sound, float pan);
void      ms_sound_set_pan(ms_sound* sound,float start, float end);

void ms_sound_init(std::string name, ma_engine* engine, unsigned int weight, std::string filepath, ms_sound* sound, ms_sound_filetype filetype, bool enable_spatialization) {
    sound->name            = name;
    sound->weight          = weight;

    // -1.0f to 1.0f
    sound->pan_range[0]    = 0.0f;
    sound->pan_range[1]    = 0.0f;

    // 0.0f to inf
    sound->pitch_range[0]  = 1.0f;
    sound->pitch_range[1]  = 1.0f;

    // 0.0f to inf
    sound->volume_range[0] = 1.0f;
    sound->volume_range[1] = 1.0f;

    #ifndef MS_NO_SPATIALIZATION
    // 0.0f to inf
    sound->distance_range[0] = 0.0f;
    sound->distance_range[1] = 0.0f;
    #endif

    #ifdef MS_VERBOSE
        std::cout << "ms_sound_init :: initialising " << sound->name << std::endl;
    #endif

    ma_uint32 flags = 0;
    #ifndef MS_NO_SPATIALIZATION
    if (!enable_spatialization) flags = MA_SOUND_FLAG_NO_SPATIALIZATION;
    #else
    flags = MA_SOUND_FLAG_NO_SPATIALIZATION;
    #endif

    unsigned short i = 0;
    while (true) {
        std::string str = filepath + to_string(i);
        switch (filetype) {
            case WAV:  str += ".wav";  break;
            case MP3:  str += ".mp3";  break;
            case FLAC: str += ".flac"; break;
        }
        if (!std::filesystem::exists(str)) break; // check if our file exists, if it doesn't we break the while loop

        #ifdef MS_VERBOSE
            else std::cout << "ms_sound_init :: loading " << str << std::endl;
        #endif

        ma_sound* s = new ma_sound;
        ma_result result = ma_sound_init_from_file(engine, str.c_str(), flags, NULL, NULL, s);

        #ifndef MS_NO_SPATIALIZATION
            ma_sound_set_positioning(s, ma_positioning_relative);
            ma_sound_set_pinned_listener_index(s, 0);
        #endif

        if (result != MA_SUCCESS) {
            #ifdef MS_VERBOSE
                std::cout << "ms_sound_init :: " << str << " failed to initialise and will not be added to " << sound->name << std::endl;
            #endif
        } else {
            sound->sounds.push_back(s);
        }

        i++;
    }
}

void ms_sound_init_empty(ms_sound* sound, unsigned int weight) {
    sound->name = "empty";
    sound->weight = weight;
}

void ms_sound_uninit(ms_sound* sound) {
    for (size_t i = sound->sounds.size(); i == 0; i--) {
        delete sound->sounds[i];
    }
}

bool ms_sound_is_playing(const ms_sound* sound) {
    for (ma_sound* s : sound->sounds) {
        if (ma_sound_is_playing(s)) return true;
    }
    return false;
}

ma_result ms_sound_start(ms_sound* sound) {
    if (!ms_sound_is_playing(sound) && sound->name != "empty") {
        size_t i = rand() % sound->sounds.size();
        #ifdef MS_VERBOSE
            std::cout << "ms_sound_start :: playing " << sound->name << "[" << to_string(i) << "]" << endl;
        #endif
        ma_sound_set_pitch (sound->sounds[i], RAND_IN_RANGE(sound->pitch_range[0],  sound->pitch_range[1]));
        ma_sound_set_volume(sound->sounds[i], RAND_IN_RANGE(sound->volume_range[0], sound->volume_range[1]));
        ma_sound_set_pan   (sound->sounds[i], RAND_IN_RANGE(sound->pan_range[0],    sound->pan_range[1]));
        #ifndef MS_NO_SPATIALIZATION
            if (sound->speakers.size() > 0) {
                ms_sound_speaker* speaker = sound->speakers[rand() % sound->speakers.size()];
                ms_sound_set_position(sound, speaker->x, speaker->y, speaker->z);
            } else if (sound->distance_range[0] != 0.0 && sound->distance_range[1] != 0.0 && sound->distance_range[2] != 0.0) {
                ms_sound_set_position(
                    sound,
                    RAND_IN_RANGE(sound->distance_range[0], sound->distance_range[3]),
                    RAND_IN_RANGE(sound->distance_range[1], sound->distance_range[4]),
                    RAND_IN_RANGE(sound->distance_range[2], sound->distance_range[5])
                );
            }
        #endif /* MS_NO_SPATIALIZATION */
        return ma_sound_start(sound->sounds[i]);
    }
    return MA_SUCCESS;
}

ma_result ms_sound_stop(const ms_sound* sound) {
    if (ms_sound_is_playing(sound)) {
        #ifdef MS_VERBOSE
            std::cout << "ms_sound_stop :: stopping " << sound->name << endl;
        #endif
        for (size_t i = 0; i < sound->sounds.size(); i++) {
            ma_sound_stop(sound->sounds[i]);
        }
    }
    return MA_SUCCESS;
}

#ifndef MS_NO_SPATIALIZATION
void ms_sound_set_spatialization(ms_sound* sound, bool spatialization) {
    for (ma_sound* s : sound->sounds) {
        ma_sound_set_spatialization_enabled(s, spatialization);
    }
}

void ms_sound_add_speaker(ms_sound* sound, const unsigned int speakerAmount, ...) {
    #ifdef MS_VERBOSE
        std::cout << "ms_sound_add_speaker :: adding " << speakerAmount << " speaker(s) to " << sound->name << std::endl;
    #endif
    va_list vl;
    va_start(vl, speakerAmount);

    for (size_t i = 0; i < speakerAmount; i++) {
        sound->speakers.push_back(va_arg(vl, ms_sound_speaker*));
    }

    va_end(vl);
}

void ms_sound_add_speaker(ms_sound* sound, ms_sound_speaker* speaker) {
    ms_sound_add_speaker(sound, 1, speaker);
}

void ms_sound_set_position(ms_sound* sound, double x, double y, double z) {
    for (ma_sound* s : sound->sounds) {
	    ma_sound_set_position(s, x, y, z);
    }
}

void ms_sound_set_distance(ms_sound* sound, double x, double y, double z) {
    ms_sound_set_distance(sound, x, y, z, x, y, z);
}

void ms_sound_set_distance(ms_sound* sound, double x1, double y1, double z1, double x2, double y2, double z2) {
    sound->distance_range[0] = x1;
    sound->distance_range[1] = y1;
    sound->distance_range[2] = z1;

    sound->distance_range[3] = x2;
    sound->distance_range[4] = y2;
    sound->distance_range[5] = z2;
}
#endif /* MS_NO_SPATIALIZATION */

void ms_sound_set_volume(ms_sound* sound, float volume) {
    if (volume < 0.0f) {
        #ifdef MS_VERBOSE
            std::cout << "ms_sound_set_volume :: `volume` must be greater than or equal to 0" << std::endl;
        #endif
        volume = 0.0f;
    }

    sound->volume_range[0] = volume;
    sound->volume_range[1] = volume;
}

void ms_sound_set_volume(ms_sound* sound, float start, float end) {
    if (start < end) { // swap start and end
        float v0 = start;
        float v1 = end;
        start = v1;
        end = v0;
    }

    if (start < 0.0f) {
        #ifdef MS_VERBOSE
            std::cout << "ms_sound_set_volume_range :: `start` must be greater than or equal to 0" << std::endl;
        #endif
        start = 0.0f;
    }

    sound->volume_range[0] = start;
    sound->volume_range[1] = end;
}

void ms_sound_set_pitch(ms_sound* sound, float pitch) {
    if (pitch <= 0.0f) {
        #ifdef MS_VERBOSE
            std::cout << "ms_sound_set_pitch :: `pitch` must be greater than 0" << std::endl;
        #endif
    }

    sound->pitch_range[0] = pitch;
    sound->pitch_range[1] = pitch;
}

void ms_sound_set_pitch(ms_sound* sound, float start, float end) {
    if (start < end) { // swap start and end
        float p0 = start;
        float p1 = end;
        start = p1;
        end = p0;
    }

    if (start <= 0.0f) {
        #ifdef MS_VERBOSE
            std::cout << "ms_sound_set_pitch_range :: `start` must be greater than 0" << std::endl;
        #endif
        start = 0.0f;
    }

    sound->pitch_range[0] = start;
    sound->pitch_range[1] = end;
}

void ms_sound_set_pan(ms_sound* sound, float pan) {
    if (pan < -1.0f) {
        #ifdef MS_VERBOSE
            std::cout << "ms_sound_set_pan :: `pan` must be greater than or equal to -1.0" << std::endl;
        #endif
        pan = -1.0f;
    }

    if (pan > 1.0f) {
        #ifdef MS_VERBOSE
            std::cout << "ms_sound_set_pan :: `pan` must be less than or equal to 1.0" << std::endl;
        #endif
        pan = 1.0f;
    }

    sound->pan_range[0] = pan;
    sound->pan_range[1] = pan;
}

void ms_sound_set_pan(ms_sound* sound, float start, float end) {
    if (start < end) { // swap start and end
        float p0 = start;
        float p1 = end;
        start = p1;
        end = p0;
    }

    if (start < -1.0f) {
        #ifdef MS_VERBOSE
            std::cout << "ms_sound_set_pan :: `pan` must be greater than or equal to -1.0" << std::endl;
        #endif
        start = -1.0f;
    }

    if (end > 1.0f) {
        #ifdef MS_VERBOSE
            std::cout << "ms_sound_set_pan :: `pan` must be less than or equal to 1.0" << std::endl;
        #endif
        end = 1.0f;
    }

    sound->pan_range[0] = start;
    sound->pan_range[1] = end;
}

/* --- ms_soundscape --- */

#ifndef MS_NO_SOUNDSCAPE

ma_result ms_soundscape_init(const std::string name, ma_engine* engine, std::string ambientFilepath, ms_soundscape* soundscape, const unsigned int soundsAmount, ...);
ma_result ms_soundscape_init(const std::string name, ma_engine* engine, std::string ambientFilepath, ms_soundscape* soundscape, ms_sound* sound);
ma_result ms_soundscape_init(const std::string name, ma_engine* engine, std::string ambientFilepath, ms_soundscape* soundscape);
void      ms_soundscape_uninit(ms_soundscape* soundscape);

#ifdef MS_VERBOSE
void      ms_soundscape_debug_list(ms_soundscape* soundscape);
#endif

void      ms_soundscape_add_sound(ms_soundscape* soundscape, const unsigned int soundsAmount, ...);
void      ms_soundscape_add_sound(ms_soundscape* soundscape, ms_sound* sound);
void      ms_soundscape_set_tickrate(ms_soundscape* soundscape, float tickrate);

bool      ms_soundscape_is_playing(const ms_soundscape* soundscape);

ma_result ms_soundscape_tick(ms_soundscape* soundscape);
ma_result ms_soundscape_start(const ms_soundscape* soundscape);
ma_result ms_soundscape_stop(const ms_soundscape* soundscape);
ma_result ms_soundscape_play_sound(const ms_soundscape* soundscape);
ma_result ms_soundscape_play_sound_skip_empty(const ms_soundscape* soundscape);
void      ms_soundscape_stop_all_sounds(const ms_soundscape* soundscape);

void      ms_soundscape_set_volume(ms_soundscape* soundscape, float volume);
void      ms_soundscape_set_volume(ms_soundscape* soundscape, float start, float end);
void      ms_soundscape_set_pitch(ms_soundscape* soundscape, float pitch);
void      ms_soundscape_set_pitch(ms_soundscape* soundscape, float start, float end);
void      ms_soundscape_set_pan(ms_soundscape* soundscape, float pan);
void      ms_soundscape_set_pan(ms_soundscape* soundscape, float start, float end);

ma_result ms_soundscape_init(const std::string name, ma_engine* engine, std::string ambientFilepath, ms_soundscape* soundscape, const unsigned int soundsAmount, ...) {
    soundscape->name = name;
    soundscape->engine = engine;

    // check if the fifth & fourth to last characters in `ambientFilepath` are not '.'
    // this means the user can input "file.wav", "file.mp3", "file.flac", and "file" and all are valid
    // we default to using .wav as it is a more common file type
    // this means we can make the function less verbose by implying the filetype in the filepath dynamically (as opposed to having a whole new argument that would have to be specified) :-)
    if (ambientFilepath[ambientFilepath.size() - 4] != '.' && ambientFilepath[ambientFilepath.size() - 5] != '.') ambientFilepath += ".wav";

    ma_sound* ambient = new ma_sound;
    ma_sound_init_from_file(soundscape->engine, ambientFilepath.c_str(), 0, NULL, NULL, ambient);
    soundscape->ambient = ambient;
    ma_sound_set_looping(soundscape->ambient, true);

    soundscape->timeSinceLastTick = 0;
    soundscape->tickrate = MS_DEFAULT_TICK_RATE * MS_SAMPLE_RATE;

    if (soundsAmount > 0) {
        va_list vl;
        va_start(vl, soundsAmount);
        for (size_t i = 0; i < soundsAmount; i++) {
            ms_sound* s = va_arg(vl, ms_sound*);
            for (size_t i = 0; i < s->weight; i++) {
                soundscape->sounds.push_back(s);
            }
        }
        va_end(vl);
    }

    #ifdef MS_VERBOSE
        std::cout << "ms_soundscape_init :: initialising " << soundscape->name << " with " << soundsAmount << " ms_sound(s):" << std::endl;
    #endif

    return MA_SUCCESS;
}

ma_result ms_soundscape_init(std::string name, ma_engine* engine, std::string ambientFilepath, ms_soundscape* soundscape, ms_sound* sound) {
    return ms_soundscape_init(name, engine, ambientFilepath, soundscape, 1, sound);
}

ma_result ms_soundscape_init(std::string name, ma_engine* engine, std::string ambientFilepath, ms_soundscape* soundscape) {
    return ms_soundscape_init(name, engine, ambientFilepath, soundscape, 0, "this is ignored");
}

void ms_soundscape_uninit(ms_soundscape* soundscape) {
    if (soundscape->sounds.size() == 0) return;
    for (size_t i = soundscape->sounds.size(); i == 0; i--) {
        ms_sound_uninit(soundscape->sounds[i]);
    }
}

#ifdef MS_VERBOSE
void ms_soundscape_debug_list(ms_soundscape* soundscape) {
    std::cout << "ms_soundscape_debug_list :: " << soundscape->name << std::endl;
    for (size_t i = 0; i < soundscape->sounds.size(); i++) {
        std::cout << "ms_soundscape_debug_list :: " << soundscape->sounds[i]->name << std::endl;
    }
}
#endif

ma_result ms_soundscape_tick(ms_soundscape* soundscape) {
    if (ma_engine_get_time_in_pcm_frames(soundscape->engine) < soundscape->timeSinceLastTick + soundscape->tickrate) return MA_SUCCESS; // not enough time has passed for us to tick yet
    #ifdef MS_VERBOSE
        std::cout << "ms_soundscape_tick :: " << soundscape->name << " ticking" << std::endl;
    #endif
    soundscape->timeSinceLastTick = ma_engine_get_time_in_pcm_frames(soundscape->engine);
    ms_soundscape_play_sound(soundscape);
    return MA_SUCCESS;
}

void ms_soundscape_add_sound(ms_soundscape* soundscape, const unsigned int soundsAmount, ...) {
    va_list vl;
    va_start(vl, soundsAmount);
    for (size_t i = 0; i < soundsAmount; i++) {
        ms_soundscape_add_sound(soundscape, va_arg(vl, ms_sound*));
    }
    va_end(vl);
}

void ms_soundscape_add_sound(ms_soundscape* soundscape, ms_sound* sound) {
    #ifdef MS_VERBOSE
        std::cout << "ms_soundscape_add_sound :: adding " << sound->name << " to " << soundscape->name << std::endl;
    #endif
    for (size_t i = 0; i < sound->weight; i++) {
        soundscape->sounds.push_back(sound);
    }
}

void ms_soundscape_set_tickrate(ms_soundscape* soundscape, float tickrate) {
    if (tickrate < 0) {
        #ifdef MS_VERBOSE
            std::cout << "ms_soundscape_set_tickrate :: " << soundscape->name << ", tickrate below zero" << std::endl;
        #endif
        tickrate = 0;
    }
    soundscape->tickrate = tickrate * MS_SAMPLE_RATE;
}

bool ms_soundscape_is_playing(const ms_soundscape* soundscape) {
    for (ms_sound* s : soundscape->sounds) {
        if (ms_sound_is_playing(s)) return true;
    }
    return false;
}

// https://github.com/mackron/miniaudio/issues/714
ma_result ms_soundscape_start(const ms_soundscape* soundscape) {
    if (soundscape->ambient != nullptr) {
        // ma_sound_set_stop_time_in_milliseconds(soundscape->ambient, ~(ma_uint64)0);
        // ma_sound_set_fade_in_milliseconds(soundscape->ambient, 0.0f, 1.0f, FADE_AMOUNT_MS);
        ma_sound_set_stop_time_in_milliseconds(soundscape->ambient, ~(ma_uint64)0);
        ma_sound_set_fade_in_pcm_frames(soundscape->ambient, 0.0f, 1.0f, MS_DEFAULT_FADE_AMOUNT);
        ma_sound_start(soundscape->ambient);
    }
    return MA_SUCCESS;
}

ma_result ms_soundscape_stop(const ms_soundscape* soundscape) {
    // ma_sound_set_fade_in_milliseconds(soundscape->ambient, -1.0f, 0.0f, FADE_AMOUNT_MS);
    ma_sound_set_fade_in_pcm_frames(soundscape->ambient, -1.0f, 0.0f, MS_DEFAULT_FADE_AMOUNT);
    ma_sound_set_stop_time_in_pcm_frames(soundscape->ambient, ma_engine_get_time_in_pcm_frames(soundscape->engine) + MS_DEFAULT_FADE_AMOUNT);
    return MA_SUCCESS;
}

ma_result ms_soundscape_play_sound(const ms_soundscape* soundscape) {
    return ms_sound_start(soundscape->sounds[rand() % soundscape->sounds.size()]);
}

ma_result ms_soundscape_play_sound_skip_empty(const ms_soundscape* soundscape) {
    for (size_t i = 0; i < soundscape->sounds.size(); i++) {
        size_t r = rand() % soundscape->sounds.size();
        if (soundscape->sounds[r]->name != "empty") return ms_sound_start(soundscape->sounds[r]);
    }
    return ms_sound_start(soundscape->sounds[rand() % soundscape->sounds.size()]);
}

void ms_soundscape_stop_all_sounds(const ms_soundscape* soundscape) {
    for (ms_sound* sound : soundscape->sounds) {
        ms_sound_stop(sound);
    }
}

void ms_soundscape_set_volume(ms_soundscape* soundscape, float volume) {
    for (ms_sound* s : soundscape->sounds) {
        ms_sound_set_volume(s, s->volume_range[0] * volume, s->volume_range[1] * volume);
    }
}

void ms_soundscape_set_volume(ms_soundscape* soundscape, float start, float end) {
    for (ms_sound* s : soundscape->sounds) {
        ms_sound_set_volume(s, s->volume_range[0] * start, s->volume_range[1] * end);
    }
}

void ms_soundscape_set_pitch(ms_soundscape* soundscape, float pitch) {
    for (ms_sound* s : soundscape->sounds) {
        ms_sound_set_pitch(s, s->pitch_range[0] * pitch, s->pitch_range[1] * pitch);
    }
}

void ms_soundscape_set_pitch(ms_soundscape* soundscape, float start, float end) {
    for (ms_sound* s : soundscape->sounds) {
        ms_sound_set_pitch(s, s->pitch_range[0] * start, s->pitch_range[1] * end);
    }
}

void ms_soundscape_set_pan(ms_soundscape* soundscape, float pan) {
    for (ms_sound* s : soundscape->sounds) {
        ms_sound_set_pan(s, pan, pan);
    }
}

void ms_soundscape_set_pan(ms_soundscape* soundscape, float start, float end) {
    for (ms_sound* s : soundscape->sounds) {
        ms_sound_set_pan(s, start, end);
    }

}

#endif /* MS_NO_SOUNDSCAPE */

/* --- ms_sound_speaker --- */

#ifndef MS_NO_SPATIALIZATION

void ms_sound_speaker_init(std::string name, ms_sound_speaker* speaker, double x, double y, double z);
void ms_sound_speaker_uninit(ms_sound_speaker* speaker);
bool ms_sound_speaker_is_occupied(ms_sound_speaker* speaker);

void ms_sound_speaker_init(std::string name, ms_sound_speaker* speaker, double x, double y, double z) {
    #ifdef MS_VERBOSE
        std::cout << "ms_sound_speaker_init :: initialising " << name << std::endl;
    #endif
    speaker->name = name;
    speaker->x    = x;
    speaker->y    = y;
    speaker->z    = z;
}

void ms_sound_speaker_uninit(ms_sound_speaker* speaker) {
    delete speaker;
}

bool ms_sound_speaker_is_occupied(ms_sound_speaker* speaker) {
    return ms_sound_is_playing(speaker->sound);
}

#endif // MS_NO_SPATIALIZATION

#endif // MINISOUNDSCAPE_H