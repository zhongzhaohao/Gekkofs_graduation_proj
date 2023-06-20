## Spack

Spack is a package manager for supercomputers and Linux. It makes it easy to install scientific software for regular
users.
Spack is another method to install GekkoFS where Spack handles all the dependencies and setting up the environment.

### Install Spack

First, install Spack. You can find the instructions [here](https://spack.readthedocs.io/en/latest/getting_started.html)

```bash
git clone https://github.com/spack/spack.git
. spack/share/spack/setup-env.sh
```

Note that the second line needs to be executed every time you open a new terminal. It sets up the environment for Spack
and the corresponding environment variables, e.g., $PATH.

### Install GekkoFS with Spack

To install GekkoFS with Spack, the GekkoFS repository needs to be added to Spack as it is not part of the official Spack
repository.

```bash
spack repo add gekkofs/scripts/spack
```

When added, the GekkoFS package is available. Its installation variants and options can be checked via:

```bash
spack info gekkofs
```

Then install GekkoFS with Spack:

```bash
spack install gekkofs
# for installing tests dependencies and running tests
spack install -v --test=root gekkofs
```

Finally, GekkoFS is loaded into the currently used environment:

```bash
spack load gekkofs
```

This installs the latest release version including its required Git submodules. The installation directory is
`$SPACK_ROOT/opt/spack/linux-<arch>/<compiler>/<version>/gekkofs-<version>`. The GekkoFS daemon (`gkfs_daemon`) is
located in the `bin` directory and the GekkoFS client (`libgkfs_intercept.so`) is located in the `lib` directory.

Note that loading the environment adds the GekkoFS daemon to the `$PATH` environment variable. Therefore, the GekkoFS
daemon is started by running `gkfs_daemon`. Loading GekkoFS in Spack further provides the `$GKFS_CLIENT` environment
variable pointing to the interception library.

Therefore, the following commands can be run to use GekkoFS:

```bash
# Consult `-h` or the Readme for further options
gkfs_daemon -r /tmp/gkfs_rootdir -m /tmp/gkfs_mountdir &
LD_PRELOAD=$GKFS_CLIENT ls -l /tmp/gkfs_mountdir
LD_PRELOAD=$GKFS_CLIENT touch /tmp/gkfs_mountdir/foo
LD_PRELOAD=$GKFS_CLIENT ls -l /tmp/gkfs_mountdir
```

When done using GekkoFS, unload it from the environment:

```bash
spack unload gekkofs
```

### Alternative deployment (on many nodes)

`gekkofs/scripts/run/gkfs` provides a script to deploy GekkoFS in a single command on several nodes by using `srun`.
Consult the main [README](../../README.md) or GekkoFS documentation for details.

### Miscellaneous

Use GekkoFS's latest version (master branch) with Spack:

```
spack install gekkofs@latest
```

Use a specific compiler on your system, e.g., gcc-11.2.0:

```bash
spack install gekkofs@latest%gcc@11.2.0
```

#### FAQ

I cannot run the tests because Python is missing? For Spack and GCC, we rely on the system installed versions. If you
are working on a supercomputer, you may need to load the corresponding Python module first:

```bash
# GekkoFS tests require at least Python version 3.6.
module load python/3.9.10
```

Everything is failing during the compilation process? See question above, either a GCC is not loaded or it is too old
and does not support C++17 which we require. In any case, when using Spack it is good practice to use the system
compiler if possible:

```bash
# GekkoFS requires at least GCC version 8
module load gcc/11.2.0
```

This may not be enough for Spack to recognize it (depending on what time Spack is installed). Therefore, you need to add
the compiler to spack via:

```bash
spack compiler find
```

`spack compiler list` should then list the loaded compiler.