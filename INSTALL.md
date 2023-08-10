# Build and installation instructions

## Compile-time build dependencies

 - cmake
 - cmake-extras
 - at-spi2-core
 - gobject-introspection
 - intltool
 - gtk-doc
 - libxml2
 - vala
 - mono
 - glib2
 - gtk3
 - gtk3-docs
 - gtk-sharp-3
 - libayatana-indicator
 - libdbusmenu-gtk3
 - libdbusmenu-glib
 - libgirepository
 - dbus-test-runner - **For testing**
 - xorg-server-xvfb - **For testing**
 - gcovr - **For coverage**
 - lcov - **For coverage**

## For end-users and packagers

```
cd libayatana-appindicator
mkdir build
cd build
cmake .. -DENABLE_GTKDOC=ON
make
sudo make install
```

**The install prefix defaults to `/usr`, change it with `-DCMAKE_INSTALL_PREFIX=/some/path`**
<br>
**The libexec prefix defaults to `/libexec`, change it with `-DCMAKE_INSTALL_LIBEXECDIR=lib`**

## For testers - unit tests only

```
cd libayatana-appindicator
mkdir build
cd build
cmake .. -DENABLE_GTKDOC=ON -DENABLE_WERROR=ON -DENABLE_TESTS=ON
make
make test
```

## For testers - both unit tests and code coverage

```
cd libayatana-appindicator
mkdir build
cd build
cmake .. -DENABLE_GTKDOC=ON -DENABLE_WERROR=ON -DENABLE_COVERAGE=ON
make
make coverage
```
