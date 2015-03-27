# OpenMyo

OS-independent Myo Library

## Background

[Thalmic's Myo](https://www.thalmic.com/en/myo/) is a cool device that is limited only... by the fact it supports Windows, OS X, iOS, and Android.

That's a very heafty set of OSes, but not only does it kick out all the developers who are actually willing to work on something cool, amazing, or mind-blowing because they don't want to be on one of thoses OSes. But it also means that, as of March 2015, you need a different SDK for each OS.

There are reasons for this, but if we look at some of the most succesful libraries in existance, they are either very focused (so they support a platform's features perfectly) or tend to start with "Open" have the same SDK for every platform, with only the compiled library being different.

Throw on top of that that I, [Rcmaniac25](https://github.com/rcmaniac25), primarily use BlackBerry 10 which is not a supported OS (unless you did Android...), and I wanted something I could use nativly.

## What that last part means

First implementation (or in progress implementation) is BlackBerry only. But that's to simplify initial development.

## Goal

Feature parity with all Myo features from Thalmic's offical SDKs.

It will also only expose C-APIs (public/myo/libmyo.h) to keep the library as flexible to all languages and systems to support.

## Future

I plan to break out the BlackBerry/QNX specific code to it's own folder, then other OSes (Windows, OS X, Linux, Android, iOS, etc.) can simply implement the OS-specific code and it will be supported. The end result is that if you want to develop with Myo, you write your code once, link to the platform's library, and you're done. No seperate SDKs, no additional companion app or software. Just direct access to the Myo.

I also intend to make a CMake file for this so any IDE/build system can be used. Right now it's BlackBerry's Momentics IDE.

Lastly, as this may/may-not grow, I hope to update this readme to be less "I will give an opinionated description of this" and more of a generic library.

## Usage

This is a tricky one (kinda)... so the library itself is designed to be independent and on it's own, but the public headers are ripped directly from the offical Windows SDK. Due to differences in licensing, they are not included and have to be downloaded seperatly. Once that's done, the library can be compiled for whatever platform is desired.

If you are just using the library, after getting the headers, then proceed to write your program referencing the normal includeds, they don't even need to be from this library. But when you go to compile, link to the appropriate library here.

## License

OpenMyo is licensed using the modified BSD license. For more details, please see LICENSE.
This is because it uses the [Myo Bluetooth Protocol](https://github.com/thalmiclabs/myo-bluetooth), which uses this license. If it wasn't for that, the license would've been either [Simplified BSD/ISC](http://choosealicense.com/licenses/isc/) or [MIT](http://choosealicense.com/licenses/mit/).
