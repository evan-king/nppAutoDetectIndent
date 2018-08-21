# Auto Detect Indention Plugin for [Notepad++](https://notepad-plus-plus.org/)

[![version][version-img]][version-url]
[![mit license][license-img]][license-url]
[![build status][appveyor-img]][appveyor-url]

Detects indention (tab or spaces) and auto adjust <kbd>Tab</kbd> key on-the-fly

## What It Does

| Indention used in file | <kbd>Tab</kbd> key            |
| ---------------------- | ----------------------------- |
| Tabs                   | 1 tab                         |
| For example: 3 Spaces  | 3 spaces                      |
| Cannot determine       | Depends on Notepad++ settings |

## Installation
1. Extract the downloaded archive
2. Depending on your Notepad++ version (x86 or x64), move the respective `nppAutoDetectIndent.dll` into Notepad++ `plugin` folder.<br>
   For Notepad++ x86: `C:\Program Files (x86)\Notepad++\plugins`<br>
   For Notepad++ x64: `C:\Program Files\Notepad++\plugins`

## Download

[Latest version](./archive/1.0.zip) (x86 & x64 included)

## Compiler
Visual Studio 2017 Community

## Origin

Chocobo1 first discovered this functionality in [Geany editor](https://www.geany.org/): [detecting-indent-type](https://geany.org/manual/dev/#detecting-indent-type)<br>
Finding autodetection preferable to reliance on systems like [editorconfig](http://editorconfig.org/), he implemented the [original version](https://github.com/Chocobo1/nppAutoDetectIndent).

After using the plugin for some time, I became increasingly annoyed with the plugin failing to correctly identify the width of space-based indentation.  I was able to note some ways that the accuracy could be enhanced, however Chocobo1 was not actively supporting his version (or at least interested in my contributions).  Hence this fork.

[version-url]: https://github.com/evan-king/nppAutoDetectIndent/releases
[version-img]: https://img.shields.io/github/release/evan-king/nppAutoDetectIndent.svg?style=flat

[appveyor-url]: https://ci.appveyor.com/project/evan-king/nppAutoDetectIndent
[appveyor-img]: https://ci.appveyor.com/api/projects/status/github/evan-king/nppAutoDetectIndent?branch=master&svg=true

[license-url]: https://github.com/evan-king/nppAutoDetectIndent/blob/master/LICENSE
[license-img]: https://img.shields.io/aur/license/yaourt.svg?style=flat
