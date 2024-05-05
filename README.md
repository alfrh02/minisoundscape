# Minisoundscape

Minisoundscape is an add-on for [Miniaudio](https://miniaud.io) that adds utilites for developers to easily create automated soundscapes in their products.

I would recommend being somewhat familiar with Miniaudio before using Minisoundscape.

Included is the source code as well as ofxMinisoundscape, which is an implementation for [openFrameworks](https://openframeworks.cc/). In the ofxMinisoundscape folder you can find some examples written for openFrameworks.

---

A soundscape is simply a collection of sounds. They contain an ambient sound that loops and sets the tone, and a group of smaller sounds that play with pseudo-random intervals. "Soundbites" refers to these smaller sounds.

Every object in Minisoundscape is a transparent structure, as is in Miniaudio. Functions are provided to properly change values, but I'm showing off the declarations of each object so that you can see what information each object holds and have a better understanding of how Minisoundscape works. You do not need to access each variable to change it.

For example `ms_sound_set_volume` will set the volume for you with checks to make sure nothing is out of range, as opposed to simply `sound->volume = 0`

# Soundscapes

In Minisoundscape these are `ms_soundscape` objects.

So that you can see what is in an `ms_soundscape` object, here is its declaration:

```c
struct ms_soundscape {
    std::string name;
    ma_sound* ambient;
    ma_engine* engine;
    vector<ms_sound*> sounds;
    ma_uint64 timeSinceLastTick; // long long
    float tickrate;
};
```

# Soundbites

`ms_sound` objects are intended to essentially be a replacement for Miniaudio's `ma_sound` for soundscapes.

`ms_sound` objects are able to store multiple sound files instead of one by having multiple `ma_sound` objects in a vector.

`ms_sound` also supports spatialisation. This can be done by assigning it speakers or by setting `distance_range`.

`ms_sound` is declared here:

```c
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
```

`ms_sound_speaker` objects are used for `ms_sound` to be emitted from. They contain x, y, and z coordinates. They also have a name, and cannot play more than one sound at once.

```c
struct ms_sound_speaker {
    std::string name;
    double x;
    double y;
    double z;
    ms_sound* sound;
};
```

---

## Usage

These examples may not map directly onto your current setup in terms of memory allocation.

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
    result = ms_sound_init("sound_name", &engine, 1, "file/path", &sound);

    // we make an empty sound with a weight of 80
    // since the previous sound has a weight of 1, it will have a 1/81 chance to play every time the soundscape ticks
    ms_sound* empty = new ms_sound;
    result = ms_sound_init_empty("empty", 80);

    // we push this into the soundscape
    // we can leave the ambient filepath as "" and it will be ignored. otherwise include a path to a sound you would like to constantly loop in the background
    // we can use variadic arguments here too - just specify how many sounds you are using
    ms_soundscape_init("soundscape", &engine, "", &soundscape, 2, sound, empty);
}

void update() {
    // tick the soundscape every update loop
    ms_soundscape_tick(&soundscape)
}
```

## Optimisation & Customisation

Optimisation/debug:
```
- MS_VERBOSE                     | Prints status updates on what minisoundscape is doing, e.g. initialising or ticking a soundscape, playing a sound, loading a soundfile, etc.
- MS_NO_SOUNDSCAPE               | Removes ms_soundscape related code. Useful if you only want the ms_sound objects
- MS_NO_SPATIALIZATION           | Removes ms_origin_point related code. Useful if you aren't doing any spatialization!
```
Customisation:
```
- MS_SAMPLE_RATE                 | Sets the sample rate. Defaults to 44100.
- MS_DEFAULT_TICK_RATE           | Sets the tick rate for each soundscape so that you don't have to do it manually - in seconds. Defaults to 1.0.
- MS_DEFAULT_FILE_TYPE           | Sets the default file type for loading sounds so that you don't have to do it manually. Defaults to WAV. Can be set to MP3 or FLAC.
- MS_DEFUALT_VOLUME              | Sets the default volume for sounds so that you don't have to do it manually. Defaults to 1.0.
- MS_DEFAULT_FADE_AMOUNT_SECONDS | Sets the default amount of time when fading between soundscapes - in seconds. Defaults to 1.0. Doesn't change anything if you set MS_DEFAULT_FADE_AMOUNT manually
- MS_DEFAULT_FADE_AMOUNT         | Sets the default amount of time when fading between soundscapes - in PCM frames. Defaults to the MS_SAMPLE_RATE * MS_DEFAULT_FADE_AMOUNT_SECONDS.
```