This is a fork of [Polyworld 2.5](https://github.com/polyworld/polyworld/tree/2.5) that runs on Windows via [MinGW](http://www.mingw.org/).
Instructions beginning with `$` indicate commands to be run at an MSYS prompt.

## Git

  * Go to https://git-scm.com/downloads.
  * Download and run the latest installer.
  * Add Git's cmd directory to your Windows `PATH`.

## MinGW and MSYS

  * Go to http://sourceforge.net/projects/mingw/files/.
  * Download and run mingw-get-setup.exe.
  * Install the following packages during setup:
      * mingw-developer-toolkit
      * mingw32-base
      * mingw32-gcc-g++ (bin)
      * mingw32-libz (dev)
      * mingw32-pthreads-w32 (dev)
  * Add C:\MinGW\bin and C:\MinGW\msys\VERSION\bin to your Windows `PATH`.
  * `$ cp /usr/etc/fstab.sample /usr/etc/fstab`
  * Close and reopen MSYS.

## GSL

  * Go to http://ftpmirror.gnu.org/gsl/.
  * Download gsl-latest.tar.gz.
  * `$ tar -xzf gsl-latest.tar.gz`
  * `$ cd gsl-VERSION`
  * `$ ./configure --prefix=/mingw`
  * `$ make`
  * `$ make install`

## dlfcn-win32

  * `$ git clone https://github.com/dlfcn-win32/dlfcn-win32.git`
  * `$ cd dlfcn-win32`
  * `$ ./configure`
  * `$ make`
  * `$ make install`

## Qt 4

  * Go to http://download.qt-project.org/archive/qt/.
  * Browse to the latest version of Qt 4.
  * Download and run qt-opensource-windows-x86-mingw482-4.MINOR_VERSION.exe.
  * Add C:\Qt\4.MINOR_VERSION\bin to your Windows `PATH`.
  * `$ echo -e 'c:/Qt/4.MINOR_VERSION\t\t/qt' >> /usr/etc/fstab`

## Python 2 (32-bit)

  * Go to https://www.python.org/downloads/.
  * Download and run the installer for the latest 32-bit Python 2.
  * `$ echo -e 'c:/Python2MINOR_VERSION\t\t/python' >> /usr/etc/fstab`

## Polyworld

  * `$ git clone https://github.com/smailliwcs/polyworld.git`
  * `$ cd polyworld`
  * `$ git checkout --track origin/mingw`
  * `$ qmake`
  * `$ make debug-main`

Invoke Polyworld with an empty worldfile to make sure everything is working:

  * `$ echo @version 2 > worldfiles/empty.wf`
  * `$ debug/Polyworld.exe worldfiles/empty.wf`

Play the simulation back with PwMoviePlayer:

  * `$ cd tools/PwMoviePlayer`
  * `$ qmake`
  * `$ make`
  * `$ debug/PwMoviePlayer.exe -f ../../run/movie.pmv`
