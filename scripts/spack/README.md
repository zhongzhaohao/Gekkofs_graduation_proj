
### Spack

You can use Spack to install GekkoFS and let it handle all the dependencies. First, you will need to install Spack:

```
git clone https://github.com/spack/spack.git
. spack/share/spack/setup-env.sh
```

Once Spack is installed and available in your path, add gekkofs to the Spack namespace.

```
spack repo add gekkofs/scripts/spack
```

You can then check that Spack can find GekkoFS by typing:

```
spack info gekkofs
```

Finally, just install GekkoFS. You can also install variants (tests, forwarding mode, AGIOS scheduling).

```bash
spack install gekkofs
# for installing tests dependencies and running tests
spack install -v --test=root gekkofs +tests
```

Remember to load GekkoFS to run:

```
spack load gekkofs
```

If you want to enable the forwarding mode:

```
spack install gekkofs +forwarding
```

If you want to enable the AGIOS scheduling library for the forwarding mode:

```
spack install gekkofs +forwarding +agios
```

If you want to use the latest developer branch of GekkoFS:

```
spack install gekkofs@latest
```

The default is using version 0.9.1 the last stable release.
