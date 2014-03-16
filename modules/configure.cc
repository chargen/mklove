#!/bin/bash
#
# Compiler detection
# Sets:
#  CC, CXX, CFLAGS, CPPFLAGS, LDFLAGS, PKG_CONFIG, instALL


mkl_require host

function checks {

    # C compiler
    mkl_meta_set "ccenv" "name" "C compiler from CC env"
    if ! mkl_command_check "ccenv" "WITH_CC" cont "$CC --version"; then
        if mkl_command_check "gcc" "WITH_GCC" cont "gcc --version"; then
            CC=gcc
        elif mkl_command_check "clang" "WITH_CLANG" cont "clang --version"; then
            CC=clang
        elif mkl_command_check "cc" "WITH_CC" fail "cc --version"; then
            CC=cc
        fi
    fi
    export CC="${CC}"
    mkl_mkvar_set CC CC "$CC"

    # C++ compiler
    mkl_meta_set "cxxenv" "name" "C++ compiler from CXX env"
    if ! mkl_command_check "cxxenv" "WITH_CXX" cont "$CXX --version" ; then
        mkl_meta_set "gxx" "name" "C++ compiler (g++)"
        mkl_meta_set "clangxx" "name" "C++ compiler (clang++)"
        mkl_meta_set "cxx" "name" "C++ compiler (c++)"
        if mkl_command_check "gxx" "WITH_GXX" cont "g++ --version"; then
            CXX=g++
        elif mkl_command_check "clangxx" "WITH_CLANGXX" cont "clang++ --version"; then
            CXX=clang++
        elif mkl_command_check "cxx" "WITH_CXX" fail "c++ --version"; then
            CXX=c++
        fi
    fi
    export CXX="${CXX}"
    mkl_mkvar_set "CXX" CXX $CXX

    # Provide prefix and checks for various other build tools.
    local t=
    for t in LD:ld NM:nm OBJDUMP:objdump STRIP:strip ; do
        local tenv=${t%:*}
        t=${t#*:}

        [[ -z ${!tenv} ]] && mkl_env_set "$tenv" "$t"

        if mkl_prog_check "$t" "" fail "${!tenv}" ; then
            export "$tenv"="${!tenv}"
            mkl_mkvar_set $tenv $tenv "${!tenv}"
        fi
    done

    # Compiler and linker flags
    [[ ! -z $CFLAGS ]]   && mkl_mkvar_append "CFLAGS" "CFLAGS" $CFLAGS
    [[ ! -z $CPPFLAGS ]] && mkl_mkvar_append "CPPFLAGS" "CPPFLAGS" $CPPFLAGS
    [[ ! -z $CXXFLAGS ]] && mkl_mkvar_append "CFLAGS" "CXXFLAGS" $CXXFLAGS
    [[ ! -z $LDFLAGS ]]  && mkl_mkvar_append "CFLAGS" "LDFLAGS" $LDFLAGS

    mkl_mkvar_append CPPFLAGS CPPFLAGS "-g"


    # pkg-config
    if [ -z "$PKG_CONFIG" ]; then
        PKG_CONFIG=pkg-config
    fi

    if mkl_command_check "pkgconfig" "WITH_PKGCONFIG" cont "$PKG_CONFIG --version"; then
        export PKG_CONFIG
    fi
    mkl_mkvar_set "pkgconfig" PKG_CONFIG $PKG_CONFIG

    [[ ! -z "$PKG_CONFIG_PATH" ]] && mkl_env_append PKG_CONFIG_PATH "$PKG_CONFIG_PATH"

    # install
    if [ -z "$INSTALL" ]; then
        INSTALL=install
    fi

    if mkl_command_check "install" "WITH_INSTALL" cont "$INSTALL --version"; then
        export INSTALL
    fi
    mkl_mkvar_set "install" INSTALL $INSTALL
}


mkl_option "Compiler" "env:CC" "--cc=CC" "Build using C compiler CC" "\$CC"
mkl_option "Compiler" "env:CXX" "--cxx=CXX" "Build using C++ compiler CXX" "\$CXX"
mkl_option "Compiler" "ARCH" "--arch=ARCH" "Build for architecture" "$(uname -m)"
mkl_option "Compiler" "CPU" "--cpu=CPU" "Build and optimize for specific CPU" "generic"


for n in CFLAGS CPPFLAGS CXXFLAGS LDFLAGS; do
    mkl_option "Compiler" "mk:$n" "--$n=$n" "Add $n flags"
done

mkl_option "Compiler" "env:PKG_CONFIG_PATH" "--pkg-config-path" "Extra paths for pkg-config"

mkl_option "Compiler" "WITH_PROFILING" "--enable-profiling" "Enable profiling"
function opt_enable-profiling {
    if [[ $2 == "y" ]]; then
        mkl_allvar_set "" "WITH_PROFILING" "y"
        mkl_mkvar_append CPPFLAGS CPPFLAGS "-pg"
        mkl_mkvar_append LDFLAGS LDFLAGS   "-pg"
    fi
}
