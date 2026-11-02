# Thoughts
Use a produce-consumer model. 
- One thread is interfacing with audio and reading it, outputting to a circular buffer.
- Either another thread or the same thread is doing windowing, FFT, converting to DB, and pushing result to a queue
- The final thread is for visualizing the result, likely at 60 fps.

Apparently it is bad to allocate memory inside the audio loop will cause stutters. Instead I should be re-using fixed-size buffers.
Also make sure queues have a max capacity.

As of now, idea is to use
- RTAudio
- FFTW
- SFML

# Fixing vcpkg installation
Need to put all feature information in `vcpkg.json`. Then use `vcpkg install` in terminal.

Then
```sh
cmake --preset vcpkg # only if CMakeLists or other stuff has changed
cmake --build --preset vcpkg
./spectogram
```
# Experiences with `cmake`, `vcpkg`
Make sure `$env:VCPKG_ROOT` is set.
Once I finally got it to compile, it still wasn't linking. The status code is `-1073741515` which is `STATUS_DLL_NOT_FOUND`. Causes:
- `CMakePresets.json` was using the classic `mingw64` compiler branch (`msvcrt.dll`), while system paths and dependencies were pulling from the modern `ucrt64` branch (`ucrtbase.dll`). This broke underlying child modules.
- `vcpkg`'s automated DLL copying (`VCPKG_APPLOCAL_DEPS`) failed because it is primarily optimized for `MSVC`, leaving the build directory empty of runtime binaries.
Solution:
- Unified the Toolchain: Switched the compiler in `CMakePresets.json` to the modern Universal C Runtime path to match the `x64-mingw-dynamic` `vcpkg` triplet.
- Explicit Linking: Updated `CMakeLists.txt` to explicitly link `SFML` components directly to the executable target (`spectogram`) rather than relying solely on the static library (`spectogram_lib`).
# Fixing that stuff
That was actually horrible. But now instead of dynamically linking, it's going to be statically linked. 

Took 2 whole days to finally be able to start to even write some code.
# Designing Layout
There are no layout classes, so I need to make them myself.

I decided to add `TGUI` for layouts and gui. 
## Getting resizing while dragging.
I followed the normal and suggested procedure of using the game event loop and handling events there, such as a resize event. However the problem is that it would only rerender when I let go of dragging the window. 

After some research, and Claude :), it turns out for both Windows and Mac when a window is being resized, they hijack the main thread and only return control back afterwards. Meaning a resize event is only sent after it is finished. On linux OTOH, it continuously sends resize events as you drag it.

This meant that I had to write platform-specific code to interact with these elements. Claude excells here, and gave a working answer quickly (at least for windows), as I have no knowledge of windows internals. Nevertheless, I did do some digging into this, mainly for windows because delving into one platform-specific api is already alot, so three is way too much to do. So I don't actually know if the mac or linux code works, will need to test that later, but probably after I finish the project. 

But the key idea is realizing that my rendering code is good, but wasn't triggering when it needed to, so hence why I needed to look deeper.

Also note, thanks to claude again, is that my solution actually depends on an internal implementation detail, because SFML tracks continuous resizes while dragging and caches the size until its done, and then emits a resize event. Specifically it checks if it is mid-drag by checking a flag set by `WM_ENTERSIZEMOVE` flag which is cleared by `WM_EXITSIZEMOVE`. When it gets cleared, then it emits a resize event. So what I'm doing is sending a fake `WM_EXITSIZEMOVE` and then a fake `WM_ENTERSIZEMOVE` again so that it keeps queuing all drag changes. But this is really an internal detail I think, so yeah. Current version of SFML being used is 3.0.2, which I am now requiring to be exact. 
## Fixing Graphical Error with Resizing
After doing this, I noticed when I was resizing many visual artifacts were showing up on the `CanvasSFML`. The two problems were that I was accidentally calling `m_gui.add(m_canvas.widget())` in `update_layout`, when that should only happen in `init`. And the second issue was that I forgot to call `m_canvas.update_layout`, thus the old scale coordinated were being used.
# Designing Ring Buffer
Designing it and implementing it initially was not difficult at all. However, the bigger thing was making it thread safe but also nice and fast for use in the audio thread for example.

I learned a lot! Like memory orders and whatnot.
# Rendering Plan
Spectrogram canvas will have two ring buffers. One is a reference to the `fft -> gui` ring buffer, and one is an owned buffer storing all the float data for the texture, so that changing the color scheme can still be done.

Then, for rendering, we give way to a fragment shader. In the fragment shader, we simulate the effect of a ring buffer as well. We keep track of a pointer index into the texture. When we add a column, we overwrite the column at the pointer and increment. We then render the texture starting at the pointer with `fract` which does repeat.
