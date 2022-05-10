# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack import *


class SyscallIntercept(CMakePackage):
    """FIXME: Put a proper description of your package here."""

    homepage = "https://github.com/pmem/syscall_intercept"
    git      = "https://github.com/pmem/syscall_intercept.git"

    maintainers = ['jeanbez']

    version('304404', commit='304404581c57d43478438d175099d20260bae74e')

    depends_on('capstone@4.0.1')

    patch('syscall-intercept.patch')
