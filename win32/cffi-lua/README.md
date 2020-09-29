# How To Build

    1. You must get vcpkg.
    2. Exec "vcpkg install libffi:x64-windows-static"
    3. Unzip "cffi-lua-0.1.1.zip" to "cffi-lua-0.1.1" directory
    4. Unzip "deps.zip" to "deps" directory.
    5. Replace "libffi.lib" & "libffi.dll" with your vcpkg "libffi.lib", and replace "liblua.lib" & "lua51.dll" with your own binary files.
    6. Copy "deps" into "cffi-lua-0.1.1" directory
    7. Enter "cffi-lua-0.1.1" directory.
    8. You must get a Meson. If you don't, exec "choco install meson" to get it. Add "meson.exe" path to "PATH" envrioment var.
    9. Create "build" directory,  exec "meson .. -Dlua_version=vendor -Dlibffi=vendor -Dshared_libffi" in it.
    10. Then exec "ninja all"
