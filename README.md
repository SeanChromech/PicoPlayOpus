# PicoPlayOpus
Play Ogg-wrapped Opus files to an I2S interface on Raspberry Pi Pico, using C.

**License:** MIT for anything I've added.  Submodules retain their own licenses.

## The What
- This is a demonstration of using the Raspberry Pi Pico to play Opus files.
- It uses the [Opus codec](https://opus-codec.org/) to decode the audio.
- The decoded audio is fed into the Pico audio stuff provided in the Pico-Extras repository.
- Ogg_stripper is provided to strip the Opus data from an Ogg file, provided either as a file or raw data in an array.
- The project has some niceties, including FreeRTOS and printf over USB serial.

## The Why
Opus is a great codec for high-quality audio at low bitrates. It is specifically suited to compressing human speech.  
Because of this, it is commonly used as the most-efficient output format in Text-To-Speech (TTS) applications.  
Neural TTS has become available (usually for free!) from a number of providers, including Google, Microsoft, and Amazon.  
These services can generate audio from text that is almost indistinguishable from a human voice.  This enables a lot of
interesting applications when playable on a small, low-power device like the Pico.  Human interface, accessibility, that
sort of thing.

The Pico has enough storage on-board to store a few minutes of audio, and plenty of resources to decode it.  Because of
the dual-core nature of the RP2040, it is possible to decode audio in one core while doing other things in the other
core.

## The How
I'd recommend using this project as a starting point for your own project.  It's not really intended to be a library 
since, in reality, it's just a few other libraries glued together with some bits in between.

A quick tour of the project:
1. main.c is where the application is.  The audio stuff is in the app_task function.  In this example, it just reads a
    file from an onboard flash array and plays it.  You can replace this with whatever you want.
Here's what it does:
    1. Initialize the audio output backend.  Buffers, I2S, etc.
    2. Initialize the Opus decoder.
    3. Set the Ogg file stored in the array as the source and prepare it for decoding.
    4. Loop until the file is done playing:
        1. Grab a free output buffer from the audio output backend.
        2. Read an Ogg packet into an input buffer.
        3. Decode the packet into the free output buffer.
        4. Give the now-loaded buffer back to the audio output backend.  The buffer is now played.
2. ogg_stripper.c/.h is a simple library to strip the Opus data from an Ogg file.  It can be used to strip the data
    from a file on the filesystem, or from a raw array of data.  It's not a full Ogg parser, it just looks for the
    Opus data and returns it.  As such, it doesn't check checksums or anything like that.  I also don't guarantee that
    it will work with all Ogg files, but it works with the ones I've tried.
3. Lots of stuff added to CMakeLists.txt to make it all work.  I'm not a CMake expert, so it's probably not the best
    way to do it, but it works.  A lot of it is copied from the Pico-Extras examples and the Opus repo.
4. settings.h has, as one would expect, settings for the rest of the project.  Of particular note is the pin assignments
   for the I2S interface.
NOTE: If you're having HardFault issues, try increasing the stack size for the app_task thread.  Opus uses a lot of 
stack since it wasn't really designed for embedded use.

## Submodules
I'm a fan of using submodules to include other libraries in my projects.  That way, you're not locked into a specific
version of a library, and you can easily update to newer versions.  This project uses a few submodules:
- [Pico-SDK](http://www.github.com/raspberrypi/pico-sdk) and its submodules, notably TinyUSB.
- [Pico-Extras](http://www.github.com/raspberrypi/pico-extras)
- [FreeRTOS](http://www.github.com/FreeRTOS/FreeRTOS-Kernel) Specifically, the smp branch for multi-core support.
- [Opus](http://www.github.com/xiph/opus)

Because of the heavy use of submodules, you'll need to clone this repo with the --recurse-submodules flag.

## Getting Started
1. I'm assuming you already have a working Pico development environment.
If not, check out the [Getting Started](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf) guide.
2. Clone this repo with the --recurse-submodules flag.
3. Change the settings in settings.h to match your setup.  Specifically, the I2S pins.
4. Build the project with CMake.  I use CLion, but you can use whatever you want, like VSCode or the command line.
5. Flash the project to your Pico and test.
6. Once it works, add your own code to app_task to do whatever you want.
7. Probably move the audio stuff to its own file and FreeRTOS task.  I didn't do that because that's all this example 
   does.  Rename things as you see fit.

Enjoy!