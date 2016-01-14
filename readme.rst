=============
 Tinto Panel
=============
Tinto panel is simple ligthweight X11 desktop panel, it is a fork of `tint2 <http://code.google.com/p/tint2/>`_, which
is also a fork of `ttm <http://code.google.com/p/ttm/>`_.

Getting the sources
-------------------
Just issue the following command:
::
   % git clone https://bitbucket.org/crowseye/tinto

Building & Instalation
----------------------
To build tinto yourself you will need the development version of these libraries, note that some Linux distributions
called ``libXXX-dev``, ``libXXX-devel``, ``XXX-dev`` or ``XXX-devel``, so you will need you find what your distribution
call them.

* X11/Xlib
* Xinerama
* Xrandr
* Xrender
* Xcomposite
* Xdamage
* Imlib2
* Gtk+2
* Cairo
* Pango
* PangoCairo
* Rsvg
* Glib
* Gobject
* Startup Notification

Once you have insalled the dependencies then issue these commands to build tinto:
::
   % cd tinto
   % cmake . -Bbuild
   % make install

Running
-------
To run the apllication just issue the following commands in your terminal.
::
   % tinto  -c  path-to-config-file


Credits
-------
Developers
^^^^^^^^^^
* Pål Staurland <staura at gmail.com> - for the original `ttm <http://code.google.com/p/ttm/>`_
* Thierry Lorthiois <lorthiois at bbsoft.fr> from Omega distribution
* Andreas Fink <andreas.fink85 at googlemail.com>
* Euan Freeman <euan04 at gmail.com> (tintwizard)
* Christian Ruppert <Spooky85 at gmail.com> (autotools build system)
* Ovidiu M <mrovi9000 at gmail.com> : launcher, bug fixes

Contributors
^^^^^^^^^^^^
* Kwaku Yeboah <kwakuyeboah at gmail.com> : wiki page
* Daniel Moerner <dmoerner at gmail.com> : man page and debian package
* Doug Barton : freebsd package
* James Buren <ryuo at frugalware.org> : Frugalware package
* Pierre-Emmanuel Andre <pea at raveland.org> : openbsd port
* Redroar : arch package
* Rene Garcia <garciamx at gmail.com> : launcher SVG support
* Marcelo Vianna : taskbar sorting
* Xico Atelo : startup notifications
* Craig Oakes : WM flags, issue tracker organization
