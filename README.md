# isotope
I rarely use discs nowadays. I think the last disc I actually used was the Windows 7 installation DVD, and that's because I didn't have a USB drive handy.

However, I have a *bunch* of old games on discs and they all work (beautifully) on my modern Windows 10 PC. I have more than enough storage, and I'm not going to have a disc drive forever, so why not rip them?

But, uh oh, half the software is commercial, a quarter is old & incompatible, and [my few favourites are now ridden with adware](https://superuser.com/questions/20780/imgburn-and-adware-where-is-a-safe-place-to-download-it). And I'm not looking to burn new discs.

So, what do I do?

On Linux, it's as easy as piping one device into a file. On Windows, it's a little trickier but still super-simple. You just need a physical HANDLE, buffering and a way to make the user stop it because interfacing with disc drives in Windows is still horribly unstable.

That's why I made this tiny utility.
