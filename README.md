# isotope [![Build status](https://ci.appveyor.com/api/projects/status/elwj9gjdn1pmceln/branch/master?svg=true)](https://ci.appveyor.com/project/smiley/isotope/branch/master)
I rarely use discs nowadays. I think the last disc I actually used was the Windows 7 installation DVD, and that's because I didn't have a USB drive handy.

However, I have a *bunch* of old games on discs and they all work (beautifully) on my modern Windows 10 PC. I have more than enough storage, and I'm not going to have a disc drive forever, so why not rip them?

But, uh oh, half the software is commercial, a quarter is old & incompatible, and [my few favourites are now ridden with adware](https://superuser.com/questions/20780/imgburn-and-adware-where-is-a-safe-place-to-download-it). And I'm not looking to burn new discs.

So, what do I do?

On Linux, it's as easy as piping one device into a file. On Windows, it's a little trickier but still super-simple. You just need a physical HANDLE, buffering and a way to make the user stop it because interfacing with disc drives in Windows is still horribly unstable.

That's why I made this tiny utility.

## How do I use this?
You can now download a build right on GitHub.

1. [Download the latest release](https://github.com/smiley/isotope/releases/latest) (in your system's architecture)
2. Drop it in `C:\Program Files\Isotope\isotope.exe`
3. Run the `register.reg` file
4. Right-click your disc drive & pick "Rip with Isotope..." to convert it to an ISO

You can also just run `isotope.exe`; no installation necessary.

## Known issues
- There's no installer. (But it's also self-contained)
- Ripping is done on a logical level, reading the same disc partition you see in `Computer`. Meaning we'll only rip "the first" if the disc:
  - Has more partitions;
  - Cheats Windows by showing a smaller partition and transparently reads the rest (e.g.: DRM);
  - Also has a Mac paritition you wish to rip.
