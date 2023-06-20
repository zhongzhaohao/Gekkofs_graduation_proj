# Copyright 2013-2023 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

# ----------------------------------------------------------------------------
# If you submit this package back to Spack as a pull request,
# please first remove this boilerplate and all FIXME comments.
#
# This is a template package file for Spack.  We've put "FIXME"
# next to all the things you'll want to change. Once you've handled
# them, you can save this file and test your package like this:
#
#     spack install parallax
#
# You can edit this file again by typing:
#
#     spack edit parallax
#
# See the Spack documentation for more information on packaging.
# ----------------------------------------------------------------------------

from spack.package import *


class Parallax(CMakePackage):
    """Parallax Key-vale database."""

    # FIXME: Add a proper url for your package's homepage here.
    homepage = "https://github.com/CARV-ICS-FORTH/paralla"
    git = "https://github.com/CARV-ICS-FORTH/parallax.git"

    # FIXME: Add a list of GitHub accounts to
    # notify when the package is updated.
    # maintainers("github_user1", "github_user2")

    # FIXME: Add proper versions and checksums here.
    version('1.0.0', commit='ffdea6e820f5c4c2d33e60d9a4b15ef9e6bbcfdd')

    # FIXME: Add dependencies if required.
    #depends_on("foo")

