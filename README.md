<div align="center">
	<img width="80" height="80" src="https://raw.githubusercontent.com/disinclination/ngPost/master/src/resources/icons/ngPost.png" alt="ngPost"/>
 	<h3 align="center">ngPost - 5.0.0</h3>
	<img alt="Codacy grade" src="https://img.shields.io/codacy/grade/e790647f2eae44898d760b68ee6f5b78?style=for-the-badge">
	<img alt="GitHub Repo stars" src="https://img.shields.io/github/stars/disinclination/ngPost?style=for-the-badge">
	<img alt="GitHub top language" src="https://img.shields.io/github/languages/top/disinclination/ngPost?style=for-the-badge">
	<img alt="GitHub License" src="https://img.shields.io/github/license/disinclination/ngPost?style=for-the-badge&color=%23c20083">
	<img alt="GitHub Issues or Pull Requests" src="https://img.shields.io/github/issues-pr-raw/disinclination/ngPost?style=for-the-badge&color=%238575a8">
	<img alt="GitHub Issues or Pull Requests" src="https://img.shields.io/github/issues-raw/disinclination/ngPost?style=for-the-badge">

</div>

# About The Project

This application is a high-speed command line and GUI Usenet poster for binaries, designed for secure and efficient data posting. Developed with C++17 and [Qt 6.8.2](https://www.qt.io/blog/qt-6.8.2-released), it features file compression, par2 file generation, and a posting queue for managing multiple uploads. The tool automates tasks by scanning folders and posting files, with options for executing commands post-upload and shutting down the computer upon completion.


![ngPost_v5.0.0](https://github.com/disinclination/ngPost/blob/rc-4.17/pics/ngPost_v4.17.png?raw=true)

# Getting Started

All the features are [highlighted here](https://github.com/disinclination/ngPost/wiki/Features). Some of the prominent ones being:
- Full obfuscation of the post.
- Built in posting queue, allows for the addition of items to the queue whilst the queue is processing.
- Full posting automation.
- Compression using RAR or 7zip.
- Multiple server support.
- Multithreading.
- And many more.

[Releases will be available](https://github.com/disinclination/ngPost/releases) for, Linux 64 Bit and Windows 64 Bit. Support for MacOS and Raspbian will be considered in the future.

For building the project yourself, please refer to [the wiki](https://github.com/disinclination/ngPost/wiki/Build)

## Command Line Usage

[Please visit the wiki](https://github.com/disinclination/ngPost/wiki/Command-Line-Usage)

### Thanks
- Matthieu Bruel for the base project
- Uukrull for his intensive testing and feedbacks and for building all the MacOS packages.
- awsms for his testing on proper server with a 10Gb/s connection that helped to improve ngPost's upload speed and the multi-threading support
- animetosho for having developped ParPar, the fasted par2 generator ever!
- demanuel for the dev of NewsUP that was my first poster
- noobcoder1983, tensai then yuppie for the German translation
- tiriclote for the Spanish translation
- hunesco for the Portuguese translation
- Peng for the Chinese translation
- All the ngPost users
