
.. _build-docs:

Building the docs locally
==========================

Official docs are available 
`through github.io <https://libhankel.github.io/libhankel/index.html>`_ . 
These are based on the "main" branch. When working on a dev branch, then 
docs must be built locally to check changes. Once the dev branch is merged into
main, the github.io docs will be re-deployed automatically to reflect the merged changes.

To build the docs locally, from the repository root:

.. code-block:: bash

   pip install -e .
   doxygen Doxyfile
   cd docs
   make html

