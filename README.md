# PdfiumView

[![Release version](https://img.shields.io/github/v/release/makuke1234/PdfiumView?display_name=release&include_prereleases)](https://github.com/makuke1234/PdfiumView/releases/latest)
[![Total downloads](https://img.shields.io/github/downloads/makuke1234/PdfiumView/total)](https://github.com/makuke1234/PdfiumView/releases)
![C++ version](https://img.shields.io/badge/version-C++20-blue.svg)

A small and compact PDF viewer for Windows, written in C++ utilising [PDFium library](https://pdfium.googlesource.com/pdfium/).<br>
*Binaries for PDFium can be obtained [here](https://github.com/bblanchon/pdfium-binaries).*


# Obtaining

The x86 (32-bit) Windows binaries can be downloaded [here](https://github.com/makuke1234/PdfiumView/releases).


# Screenshots

## PdfiumView with about dialog open

![Screen 1](https://github.com/makuke1234/PdfiumView/raw/master/images/screen1.PNG)

## Highlighted tab close button

![Highlighted button](https://github.com/makuke1234/PdfiumView/raw/master/images/screen2.PNG)

## Pressed tab close button

![Pressed button](https://github.com/makuke1234/PdfiumView/raw/master/images/screen3.PNG)


# Changelog

* 0.5 (planned)
	* [ ] Populate status bar with useful messages
	* [ ] Add zooming capability

* 0.4
	* Make about dialog always centered
		* *About dialog button is also centered*
	* Make window size less restricted
	* More refactoring of codebase
	* Optimise memory usage of file open dialogs
	* Fix annoying bug with tab becoming active every time attempting to close it
	* Fix bug with scrolling multiple pages at once with some scrollwheels
	* Prevent key spamming on tab closing
	* Add status bar
	* Update PDFium library
	* Compress icon

* 0.3.0
	* Develop about dialog

* 0.2.0
	* Refactor codebase
	* Upgrade PDFium version
	* Tabulating using Ctrl+Tab & Ctrl+Shift+Tab
	* Fix most bugs (hopefully)
	* Revamped tab close buttons
	* More optimised rendering (buffered pages)

* 0.1.0
	* Initial release


# License

This project uses the MIT license.


# Credits

* Icon made by [Dimitry Miroliubov](https://www.flaticon.com/authors/dimitry-miroliubov) from [flaticon](http://www.flaticon.com/)

