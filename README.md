This is an image viewer with a minimalist GUI. It's intended to be a viewer and nothing else. Saving images will never be supported.

Current features:
* Minimally intrusive UI. Image display windows are completely frameless. No titlebar, no status bar, no nothing. The program is mainly controlled with the keyboard, and more complex functions are accessed through a context menu.
* Supported formats: BMP, JPEG, PNG, GIF. Other formats may already be supported by Qt (which handles the decoding), but they haven't been tested.
* Proper transparency display.

Planned features:
* SVG support.

Possible future features:
* Support for traversing images in common archive formats.
* Plugin interface of some kind.

Requirements:
* Qt 5.4.1 or newer.
