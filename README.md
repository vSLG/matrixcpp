# MatrixCpp

A C++, Qt-based and async matrix library. Still in development.

## Dependencies

Currently, both building and running requires `qt5-base`.

# Cloning, Building & installing

```sh
$ git clone "https://github.com/vSLG/matrixcpp.git" && cd matrixcpp
$ mkdir build && cd build
$ cmake ..
$ cmake --build . --parallel $(nproc)
$ cmake --install .  # May need root permissions.
# Optional install parameter: --prefix /path/to/install
```

# Running tests

Before running tests, `test/account_info` file must be created and contain information about an account to test.

The file must be in this format:

```
<homeserver url> <user id> <device id> <access token>
```

After creating it, run the file `./test/ClientTest` (in the `build` directory)
