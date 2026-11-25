# LibHankel — AI Agent Guidance

This file is read by AI coding assistants (Claude Code, Codex, Cursor, etc.) working
in this repository.  Please follow these rules in addition to any user instructions.

## Project Architecture

LibHankel is a C/C++ library implementing several Hankel transform strategies, with a
Python binding layer.  The main components are:

- **C source** (`src/`): one file per family of transform strategies.
  - `hankel_transform.c` — common entry point; dispatches to the chosen method.
  - `hankel_DE_quadrature.c` — double-exponential quadrature (Ogata and Ooura variants).
  - `hankel_DHT.c` — discrete Hankel transform.
  - `hankel_QWE.c` — quadrature with extrapolation (Chave and Key variants).
  - `hankel_fftlog.cpp` — FFTLog method (C++).
  - `hankel_G0.c` — G(0) correction term.
  - `form_factors.c` — built-in analytical form factors.
  - `utils/` — shared numerical utilities (interpolation, integration helpers).
- **Public headers** (`include/`): `libhankel.h` is the main C API.  Specialised
  headers cover form factors, interpolation schemes, and tabulated form factors.
- **Python binding** (`src/py_interface.c`, `python/`): CPython extension module
  exposing the C API as a `libhankel` Python package.
- **Tests** (`tests/`): one `.c` file per module; Python API tests live under
  `tests/python_api_tests/`.
- **Build system**: Meson.  Tests are off by default; enable with
  `-Dtests=true`.
- **Docs** (`docs/source/`): Sphinx, with separate sections for C API, Python API,
  usage examples, and contributor guides.

## AI Policy

- **Do not open pull requests autonomously.** A human contributor must review all
  AI-generated code and submit the pull request themselves.
- **Do not post to issues, discussions, or forums on behalf of a contributor.**
  Human communication must come from the human.
- Pull requests that used AI assistance must state which tool was used and how
  (the PR template has an AI assistance section for this).

## Core Working Rules

- **Fix root causes.** Fix the underlying mathematical or algorithmic cause.
  Do not patch individual failing test cases.
- **Preserve coding style.** Match the naming and patterns of the module you are
  editing.  Keep edits minimal and local to the requested change.
- **Avoid duplication.** Reuse or extend nearby logic instead of duplicating it.
  Do not add speculative abstractions or broad refactors unless asked.
- **No GPL dependencies.** The project is MIT-licensed; do not introduce GPL or
  LGPL dependencies.
- **Document the present, not the past.** Do not describe a removed or rejected
  approach in a comment.  Document only what the current code does.
- **Do not trust memorised API shapes.** Read the installed source or headers
  before calling an unfamiliar function — do not rely on training-data memory.

## Testing

Build and run the C tests:

```bash
meson setup build -Dtests=true
meson compile -C build
meson test -C build
```

Run the Python API tests:

```bash
pip install -e .
pytest tests/python_api_tests/
```

Add a test for every new feature or bug fix in the existing test file for that
module.  When behaviour changes, update the affected tests.  Write the narrowest
test that proves the change.

## Code Quality

LibHankel enforces style via pre-commit hooks: **ruff** for Python, **clang-format**
and **clang-tidy** for C/C++.  Write code that conforms to these — the hooks will
reject non-conforming commits.

## Documentation

User-facing docs live under `docs/source/` and are built with Sphinx:

```bash
cd docs
make html
```

The C API is documented in `docs/source/api/c_api.rst` and the Python API in
`docs/source/api/python_api.rst`.  When adding new public functions or types,
add a corresponding entry in the appropriate `.rst` file.

The public C headers (`include/`) use Doxygen-style comments.  When adding or
changing a public function or type, update its Doxygen comment.  The Doxyfile
at the repo root configures the build; run `doxygen Doxyfile` to generate HTML
output locally.
