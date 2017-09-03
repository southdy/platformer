[![Build Status](https://travis-ci.org/louis-mclaughlin/platformer.svg?branch=master)](https://travis-ci.org/louis-mclaughlin/platformer)

![ScreenShot](https://raw.githubusercontent.com/louis-mclaughlin/platformer/master/raw/textures/promo.png)

# Setup

## Linux

### Ubuntu
```
$ sudo apt-get install git cmake g++ libglu1-mesa-dev libfreetype6-dev libogg-dev libopenal-dev libpulse-dev libglib2.0-dev
```
```
$ mkdir build
$ cd build
$ cmake ..
```
## OSX
- Install [Xcode](https://developer.apple.com/xcode/)
- Install [Homebrew](http://brew.sh/)
```
$ brew install cmake
$ ./cmake_xcode.sh
```
## Windows
- Install [CMake](https://cmake.org/download/) (Select 'Add CMake to the system PATH')
- Install [Visual Studio Community](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx)
- Run the cmake script for your Visual Studio version e.g. 2015 -> cmake_vs2015_x64.bat

## Android
- Build the game for your desktop platform
- Download the android [NDK](http://developer.android.com/ndk/downloads/index.html)
- Download the android [SDK Tools Only](http://developer.android.com/sdk/index.html)
- Download the android dependencies via external/GamePlay/install
- Install the android platform-tools via the android SDK launcher
- Set the following values in user.config and run the game
```
run_tools = true
build_android = true
android_sdk_tools_dir = <insert_path> // e.g. ~/android-sdk-linux/tools/
android_ndk_dir = <insert_path> // e.g. ~/android-ndk-r10e/
ant_dir = <insert_path> // e.g. /usr/bin
```
