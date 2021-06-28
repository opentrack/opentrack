ONNX Runtime
------------

Recommended approach on Windws: Build a shared library from sources. Use static MSVC
runtime library. The v1.6.0 branch should work fine.

Source location: https://github.com/microsoft/onnxruntime

In order to build, execute `build.bat` as follows:

```
$ build.bat --config RelWithDebInfo --x86 --build_dir .\buildx86\ \
 --enable_msvc_static_runtime --build_shared_lib --skip_tests \
 --cmake_generator "Visual Studio 15 2017"
```

Replace the argument for `--cmake_generator` if needed.

The result is a messy directory `buildx86\RelWithDebInfo\RelWithDebInfo`,
but no proper distribution. However only a few files are needed. They can
be copied manually and are listed in the following in their respective folders:

```
onnxruntime-x86-release/include:
cpu_provider_factory.h                 onnxruntime_cxx_api.h
experimental_onnxruntime_cxx_api.h     onnxruntime_cxx_inline.h
experimental_onnxruntime_cxx_inline.h  onnxruntime_session_options_config_keys.h
onnxruntime_c_api.h

onnxruntime-x86-release/lib:
onnxruntime.dll  onnxruntime.exp  onnxruntime.lib  onnxruntime.pdb
```

See also https://www.onnxruntime.ai/docs/how-to/build.html
