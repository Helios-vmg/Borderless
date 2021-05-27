This is an image viewer with a minimalist GUI. It's intended to be a viewer and nothing else. Saving images will never be supported.

Current features:
* Minimally intrusive UI. Image display windows are completely frameless. No titlebar, no status bar, no nothing. The program is mainly controlled with the keyboard, and more complex functions are accessed through a context menu.
* Supported operating systems: Windows 7 and newer for the x86-64 architecture, FreeBSD, Linux.
* Supported formats: BMP, JPEG, PNG, GIF, WebP, basic SVG support. Other formats may already be supported by Qt (which handles the decoding), but they haven't been tested.
* Proper transparency display.

Possible future features:
* Support for traversing images in common archive formats.

Requirements (compilation):
* Qt 5.4.1 or newer
* Boost.Iostreams
