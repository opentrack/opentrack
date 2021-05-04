ONNX Runtime
------------

Recommended approach on Windws: Build a shared library from sources. Use static MSVC
runtime library. The v1.6.0 branch should work fine.

Source location: https://github.com/microsoft/onnxruntime

In order to build, execute `build.bat` as follows:

```
$ build.bat --config Release --x86 --cmake_extra_defines CMAKE_INSTALL_PREFIX="D:\Dev\onnxruntime-x86-release" --build_dir .\buildx86\ --enable_msvc_static_runtime --build_shared_lib --skip_tests --cmake_generator "Visual Studio 15 2017"
$ cmake --install .\buildx64\Release
```

Replace the argument for `--cmake_generator` if needed. Also adjust the build-and install directories.

This should place all required files in the directory specified by CMAKE_INSTALL_PREFIX.

See also https://www.onnxruntime.ai/docs/how-to/build.html.