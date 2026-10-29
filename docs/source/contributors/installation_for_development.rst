
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

.. note::

   On Windows, use `MSYS2 <https://www.msys2.org/>`_ to install MinGW-w64 and make sure its
   ``bin`` directory is on your ``PATH`` before running Meson.  The vcpkg Boost headers are found
   automatically; if you install them elsewhere, pass ``CPPFLAGS=-I/path/to/boost`` to Meson.

To build and install LibHankel, together with the test suite, please use:

.. code-block:: bash

   git clone --recurse-submodules https://github.com/libhankel/libhankel.git
   cd libhankel
   meson setup build -Dtests=true
   meson compile -C build
   meson install -C build

Please note that using ``git clone`` and ``meson setup build`` without the suggested flags will not
include the tests in the installation.

To also use LibHankel from Python, install the Python extension in editable mode:

.. code-block:: bash

   pip install -e .

This compiles the Python extension and registers the source tree as the installed package, so
``import libhankel`` works immediately from the virtual environment.  NumPy is installed
automatically as a dependency.  The Meson and setuptools builds are independent — see
:ref:`two-build-systems` below for details.

.. _two-build-systems:

Two build systems
------------------

LibHankel uses two separate build systems for two separate purposes:

**Meson** (``meson compile -C build``) builds the C test executables and installs the shared
library.  It is the authoritative build for CI and for users installing LibHankel as a C library.

**setuptools** (``setup.py``) builds the Python extension module — the ``libhankel`` C extension
that ``import libhankel`` loads.  It compiles the same C sources but produces a ``.so`` / ``.pyd``
file that Python can import directly from the source tree.

The two systems are independent: Meson knows nothing about the Python extension, and setuptools
knows nothing about the test executables.  After editing any C source file, rebuild whichever
artifact you want to use:

- To run the C tests: ``meson compile -C build``
- To update the Python extension: ``python setup.py build_ext --inplace``

The ``--inplace`` flag places the rebuilt ``.so`` next to ``setup.py`` so Python finds it
immediately without a reinstall step.

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

Coverage
---------

CI automatically measures C source coverage on every push to ``main`` and on every pull request,
using gcovr to collect data from the Meson build and uploading the result to
`Codecov <https://codecov.io>`_.  Codecov posts a comment on each pull request showing
overall coverage, per-file changes, and which new lines are not yet exercised by any test.

To reproduce the coverage build locally, install ``gcovr`` first:

.. code-block:: bash

   sudo apt install gcovr          # Debian / Ubuntu
   brew install gcovr              # macOS
   pip install gcovr               # anywhere (slower but cross-platform)

Then configure a separate build with coverage instrumentation enabled:

.. code-block:: bash

   meson setup build-cov -Dtests=true -Db_coverage=true
   meson compile -C build-cov
   meson test -C build-cov

After the tests run, ``.gcda`` files accumulate in ``build-cov/``.  Generate a report filtered
to the library sources only:

.. code-block:: bash

   gcovr --root . --filter "src/" --html --output coverage.html .
   # or for a quick terminal summary:
   gcovr --root . --filter "src/" --txt .

Open ``coverage.html`` in a browser to see a line-by-line view: green lines were hit by at
least one test, red lines were not.  The ``--filter "src/"`` flag excludes external libraries,
submodules, and test utilities from the report so only the library's own code is shown.

.. note::

   Coverage instrumentation adds ``--coverage`` to the compiler flags, which significantly
   slows down the compiled binaries.  Keep a separate ``build-cov/`` directory for this purpose
   and use the normal ``build/`` for day-to-day development and benchmarking.
