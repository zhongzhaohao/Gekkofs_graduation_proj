# Copyright 2013-2023 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack import *


# for Clion. Comment out when using spack
# from spack.build_systems.cmake import *
# from spack.directives import *
# from spack.multimethod import when
# from spack.util.executable import which
# from spack.package import PackageBase

class Gekkofs(CMakePackage):
    """GekkoFS is a distributed burst buffer file system in user space. It is capable of aggregating the local I/O
    capacity and performance of each compute node in a HPC cluster to produce a high-performance storage space that
    can be accessed in a distributed manner. This storage space allows HPC applications and simulations to run in
    isolation from each other with regards to I/O, which reduces interferences and improves performance."""
    homepage = "https://storage.bsc.es/gitlab/hpc/gekkofs"
    git = "https://storage.bsc.es/gitlab/hpc/gekkofs.git"
    url = "https://storage.bsc.es/projects/gekkofs/releases/gekkofs-v0.9.1.tar.gz"

    maintainers = ['marc_vef', 'ramon_nou']
    # set various versions
    version('latest', branch='master', submodules=True)
    version('0.9.0', sha256='f6f7ec9735417d71d68553b6a4832e2c23f3e406d8d14ffb293855b8aeec4c3a', deprecated=True)
    version('0.9.1', sha256='1772b8a9d4777eca895f88cea6a1b4db2fda62e382ec9f73508e38e9d205d5f7')
    # apply patches
    patch('date-tz.patch')
    # set arguments
    variant('build_type',
            default='Release',
            description='CMake build type',
            values=('Debug', 'Release', 'RelWithDebInfo')
            )

    variant('forwarding', default=False, description='Enables the GekkoFS I/O forwarding mode.')
    variant('agios', default=False, description='Enables the AGIOS scheduler for the forwarding mode.')
    variant('guided_distributor', default=False, description='Enables the guided distributor.')
    variant('prometheus', default=False, description='Enables Prometheus support for statistics.')
    variant('parallax', default=False, description='Enables Parallax key-value database.', when='latest')
    variant('rename', default=False, description='Enables experimental rename support.', when='latest')
    variant('dedicated_psm2', default=False, description='Use dedicated _non-system_ opa-psm2 version 11.2.185.')
    variant('compile', default='x86', multi=False, values=('x86', 'powerpc', 'arm'),
            description='Architecture to compile syscall intercept.')
    # general dependencies
    depends_on('cmake@3.6.0:', type='build')
    depends_on('lz4')
    depends_on('argobots')
    depends_on('syscall-intercept@arm', when='compile=arm')
    depends_on('syscall-intercept@powerpc', when='compile=powerpc')
    depends_on('syscall-intercept@x86', when='compile=x86')
    depends_on('date cxxstd=14 +shared +tz tzdb=system')
    depends_on('opa-psm2@11.2.185', when='+dedicated_psm2')
    # 0.9.0 specific
    depends_on('libfabric@1.13.2', when='@0.9:,latest')
    depends_on('mercury@2.1.0 -debug +ofi -mpi -bmi +sm +shared +boostsys -checksum', when='@0.9:,latest')
    depends_on('mochi-margo@0.9.6', when='@0.9:,latest')
    depends_on('rocksdb@6.20.3 -shared +static +lz4 -snappy -zlib -zstd -bz2 +rtti', when='@0.9.0')
    # 0.9.1 specific
    depends_on('rocksdb@6.26.1 -shared +static +lz4 -snappy -zlib -zstd -bz2 +rtti', when='@0.9.1:,latest')

    # Additional features
    # Agios I/O forwarding
    depends_on('agios@1.0', when='@0.8: +agios')
    depends_on('agios@latest', when='@master +agios')
    # Prometheus CPP
    depends_on('prometheus-cpp', when='@0.9:,latest +prometheus')
    depends_on('parallax', when='@0.9:,latest +parallax')

    # known incompatbilities
    conflicts('%gcc@11:', when='@:0.9.1')

    def cmake_args(self):
        """Set up GekkoFS CMake arguments"""
        args = [
            self.define('CMAKE_INSTALL_LIBDIR', self.prefix.lib),
            self.define('GKFS_BUILD_TESTS', self.run_tests),
            self.define_from_variant('GKFS_ENABLE_FORWARDING', 'forwarding'),
            self.define_from_variant('GKFS_ENABLE_AGIOS', 'agios'),
            self.define_from_variant('GKFS_USE_GUIDED_DISTRIBUTION', 'guided_distributor'),
            self.define_from_variant('GKFS_ENABLE_PROMETHEUS', 'prometheus'),
            self.define_from_variant('GKFS_USE_PARALLAX', 'parallax'),
            self.define_from_variant('GKFS_RENAME_SUPPORT', 'rename'),
        ]
        return args

    def check(self):
        """Run tests"""
        with working_dir(self.build_directory):
            make('test', parallel=False)

    def setup_run_environment(self, env):
        env.set('GKFS_CLIENT', join_path(self.prefix.lib, 'libgkfs_intercept.so'))
