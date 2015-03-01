# Waysome

Note: Waysome is work in progress and not ready or useable yet.

## Call for help

We ran into a serious problem with the DRM driver of the kernel returning an
error value where we wouldn't expect one. We debugged the issue (the kernel
driver) but we do not understand what went wrong.

If you can help us with this, please have a look at PR #624 and contribute
your opinion on the topic, we would highly appreciate it!

---

Waysome is a window manager using wayland. It is especially made for powerusers
and people who like to configure their window manager very atomicly.

Waysome is developed as a semester project at
[Hochschule Furtwangen University](http://hs-furtwangen.de).

Waysome does not define any behaviour. It can be configured and scripted over
an API, which allows the user to define the tiling, floating and overall
behaviour. For example, if you want it to behave like
[i3wm](http://i3wm.org), you can configure it
this way. You can also configure it to behave like
[dwm](http://dwm.suckless.org),
[awesome](http://awesome.naquadah.org) or anything
else. You can script it to do tiling in a completely new way, to fit your needs
in every single bit. Therefor, waysome can be considered as tiling window
manager 'framework'.

Yet, a minimal configuration will be shipped with the repository.

# Contributing

If you want to contribute to waysome, please read the CONTRIBUTING file.

# License

Waysome is distributed under the terms of LGPLv2, for more information see the
LICENSE file.

Copyright (C) 2014-2015 Julian Ganz, Manuel Messner, Marcel Müller,
Matthias Beyer, Nadja Sommerfeld
