# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack import *


class Agios(CMakePackage):
    """AGIOS: an I/O request scheduling library at file level."""

    homepage = "https://github.com/francielizanon/agios"
    url      = "https://github.com/jeanbez/agios/archive/refs/tags/v1.0.tar.gz"
    git      = "https://github.com/francielizanon/agios.git"

    version('latest', branch='development')
    version('1.0', sha256='e8383a6ab0180ae8ba9bb2deb1c65d90c00583c3d6e77c70c415de8a98534efd')

    depends_on('libconfig')

