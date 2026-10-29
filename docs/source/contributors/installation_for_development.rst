
.. _installation_for_development:

Installation for development
==============================

Installation for a contributor is very similar to that for a C user.
As mentioned in other parts of the docs, LibHankel requires:

- Meson >= 1.4.0
- Ninja
- A C compiler and a C++ compiler (e.g. gcc/g++ on Linux, Apple clang on macOS, MinGW-w64 GCC on Windows)

Due to the required Meson version being no less than 1.4.0, it will be necessary
to install Meson through ``pip`` (installing through ``apt update`` most likely won't work).
It is recommended to use a Python virtual environment to avoid modifying system Python
packages:

.. code-block:: bash

   python3 -m venv .venv
   source .venv/bin/activate      # Linux / macOS
   # .venv\Scripts\activate       # Windows (Command Prompt)
   python -m pip install meson==1.4.0


Ninja is generally installed together with Meson. If it isn't, it can be installed with:

.. code-block:: bash

   sudo apt install ninja-build      # Debian / Ubuntu
   brew install ninja                # macOS
   pip install ninja                 # Windows


LibHankel also requires Boost development headers as a dependency:

.. code-block:: bash

   sudo apt update                   # Debian / Ubuntu
   sudo apt install libboost-dev

   brew install boost                # macOS

   vcpkg install boost-math          # Windows (vcpkg)

To build and install LibHankel, together with the test suite, please use:

.. code-block:: bash

   git clone --recurse-submodules https://github.com/libhankel/libhankel.git
   cd libhankel
   meson setup build -Dtests=true
   meson compile -C build
   meson install -C build

Please note that using ``git clone`` and ``meson setup build`` without the suggested flags will not
include the tests in the installation.

.. note::

   On Windows, use `MSYS2 <https://www.msys2.org/>`_ to install MinGW-w64 and make sure its
   ``bin`` directory is on your ``PATH`` before running Meson.  The vcpkg Boost headers are found
   automatically; if you install them elsewhere, pass ``CPPFLAGS=-I/path/to/boost`` to Meson.

Two build systems
------------------

LibHankel uses two separate build systems for two separate purposes:

**Meson** (``meson compile -C build``) builds the C test executables and installs the shared
library.  It is the authoritative build for CI and for users installing LibHankel as a C library.

**setuptools** (``setup.py``) builds the Python extension module — the ``libhankel`` C extension
that ``import libhankel`` loads.  It compiles the same C sources but produces a ``.so`` / ``.pyd``
file that Python can import directly from the source tree.

The two systems are independent: Meson knows nothing about the Python extension, and setuptools
knows nothing about the test executables.  After editing any C source file, you must rebuild
whichever artifact you want to use:

- To run the C tests: ``meson compile -C build``
- To use the updated code from Python: ``python setup.py build_ext --inplace``

The ``--inplace`` flag places the rebuilt extension next to ``setup.py`` rather than in a
temporary ``build/`` directory, so Python finds it immediately without an install step.

If you are working on the Python bindings, the typical one-time setup is:

.. code-block:: bash

   pip install -e .

This compiles the extension and registers the source directory as the installed package, so
``import libhankel`` works from anywhere in the virtual environment.  After that, whenever you
edit a C source file you only need to recompile:

.. code-block:: bash

   python setup.py build_ext --inplace

Python picks up the new ``.so`` automatically because the editable install already points here.

Code quality tools
-------------------

LibHankel uses `pre-commit <https://pre-commit.com/>`_ to run the following checks automatically
on every commit:

- **ruff** — Python formatting and linting
- **clang-format** — C/C++ formatting
- **clang-tidy** — C/C++ linting

ruff is managed by pre-commit and requires no separate installation.  clang-tidy and clang-format
must be installed manually:

.. code-block:: bash

   sudo apt install clang-tidy clang-format   # Debian / Ubuntu
   brew install llvm                           # macOS

On Windows (MSYS2):

.. code-block:: bash

   pacman -S mingw-w64-x86_64-clang-tools-extra

Install pre-commit and the hooks:

.. code-block:: bash

   pip install pre-commit
   pre-commit install

clang-tidy requires a compilation database to resolve includes and compiler flags. It looks for
one in a directory called ``build-c``, which must be created separately from the main build:

.. code-block:: bash

   meson setup build-c -Dtests=true
   meson compile -C build-c

This only needs to be done once. After that, pre-commit will run clang-tidy automatically
whenever you commit C or C++ files.
