
.. _making-a-release:

Making a release
=================

Releases are published to `PyPI <https://pypi.org/project/libhankel/>`_ automatically
when a version tag is pushed to GitHub.  The steps below walk through the full process.

1. Bump the version number
---------------------------

The version is recorded in two places; both must be updated to the same value:

- ``pyproject.toml`` — the ``version`` field under ``[project]``
- ``meson.build`` — the ``version:`` keyword in the top-level ``project()`` call

Use `semantic versioning <https://semver.org/>`_: ``MAJOR.MINOR.PATCH``.

2. Commit and merge to main
-----------------------------

Open a pull request with the version bump and merge it into ``main`` before tagging.
The tag should always point to a commit on ``main``.

3. Create a GitHub release with a description
-----------------------------------------------

Go to the repository on GitHub and click **Releases → Draft a new release**.

- **Choose a tag** — type the new version tag (e.g. ``v0.1.2``) and select
  *Create new tag: v0.1.2 on publish*.  GitHub will create the tag at the
  current ``main`` HEAD when you publish the release.
- **Target** — leave this set to ``main``.
- **Release title** — use the version number, e.g. ``v0.1.2``.
- **Description** — write a summary of what changed.  A useful structure is:

  .. code-block:: text

     ## What's new
     - Short description of new feature or fix (#PR number)
     - …

     ## Bug fixes
     - …

     ## Breaking changes (if any)
     - …

  GitHub's **Generate release notes** button inserts a list of merged pull
  requests automatically; you can use that as a starting point and edit it
  down to the most user-facing highlights.

- Leave **Set as the latest release** checked.
- Click **Publish release**.

Publishing the release creates the tag and triggers the
``release.yml`` CI workflow, which builds wheels for Linux, macOS, and
Windows plus a source distribution, then uploads everything to PyPI.

4. Verify the release
----------------------

Once the workflow finishes (a few minutes), check:

- The `Actions tab <https://github.com/libhankel/libhankel/actions>`_ shows the
  ``Publish to PyPI`` workflow completed successfully.
- The new version appears on `PyPI <https://pypi.org/project/libhankel/>`_.
- ``pip install libhankel==<new version>`` works in a fresh virtual environment.
