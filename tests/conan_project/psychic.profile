# Overlay profile: include this after your normal host/build settings.
#
# This keeps the compiler choice from your base profile and only injects the
# psychicstd CMake toolchain. The toolchain then adds the psychicstd include
# path and the exact linker/runtime flags needed for the selected compiler.
[conf]
*:tools.cmake.cmaketoolchain:user_toolchain=['{{ os.path.join(profile_dir, "..", "..", "cmake", "psychicstd-toolchain.cmake") }}']
