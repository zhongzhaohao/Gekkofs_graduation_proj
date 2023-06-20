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
#     spack install prometheus-cpp
#
# You can edit this file again by typing:
#
#     spack edit prometheus-cpp
#
# See the Spack documentation for more information on packaging.
# ----------------------------------------------------------------------------

from spack.package import *


class PrometheusCpp(CMakePackage):
    """Prometheus CPP bindings."""

    # FIXME: Add a proper url for your package's homepage here.
    homepage = "https://gitgub.com/jupp0r/prometheus-cpp"
    url = "https://github.com/jupp0r/prometheus-cpp/releases/download/v1.0.0/prometheus-cpp-with-submodules.tar.gz"

    # FIXME: Add a list of GitHub accounts to
    # notify when the package is updated.
    # maintainers("github_user1", "github_user2")

    version("1.0.0", sha256="593ea4e2b6f4ecf9bd73136e35f5ec66a69249f8cbbb90963e9e04517fb471c0")

    # FIXME: Add dependencies if required.
    # depends_on("foo")

