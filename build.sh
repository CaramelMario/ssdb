#!/bin/sh
set -e
if [ -z "$CXX" ]; then
	CXX=g++
fi
${CXX} -o a -x c++ - << EOF
#include <stdio.h>
int main() {
#ifdef __clang_major__
	printf("clang-%d.%d", __clang_major__, __clang_minor__);
#elif defined(__INTEL_COMPILER)
	printf("icc-%d.%d", __INTEL_COMPILER/100, __INTEL_COMPILER%100);
#elif defined(__GNUC__)
	printf("g++-%d.%d", __GNUC__, __GNUC_MINOR__);
#endif
	return 0;
}
EOF
COMPILER=`./a && rm -f a a.exe`
BASE_DIR=`pwd`
JEMALLOC_PATH="$BASE_DIR/deps/jemalloc-4.1.0"
#LEVELDB_PATH="$BASE_DIR/deps/leveldb-1.18"
if [ -z "$TERARK_DB_HOME" ]; then
	TERARK_DB_HOME="$PWD/../terark-db/pkg/terark-db-Linux-x86_64-g++-5.3-bmi2-0"
fi
TERARK_DB_LIB="-L$TERARK_DB_HOME/lib"
TERARK_FSA_LIB="-L$TERARK_DB_HOME/lib"
LEVELDB_PATH="$TERARK_DB_HOME/api/leveldb"

if [ -z "$TERARK_DEBUG" -o "$TERARK_DEBUG" = 0 ]; then
	TERARK_LIB_SUFFIX="r"
else
	TERARK_LIB_SUFFIX="d"
fi
TERARK_DB_LIB="$TERARK_DB_LIB -lterark-db-leveldb-api-${COMPILER}-${TERARK_LIB_SUFFIX} -lterark-db-${COMPILER}-${TERARK_LIB_SUFFIX}"
TERARK_FSA_LIB="$TERARK_FSA_LIB -lterark-fsa_all-${COMPILER}-${TERARK_LIB_SUFFIX}"

SNAPPY_PATH="$BASE_DIR/deps/snappy-1.1.0"

# dependency check
which autoconf > /dev/null 2>&1
if [ "$?" -ne 0 ]; then
	echo ""
	echo "ERROR! autoconf required! install autoconf first"
	echo ""
	exit 1
fi

if test -z "$TARGET_OS"; then
	TARGET_OS=`uname -s`
fi
if test -z "$MAKE"; then
	MAKE=make
fi
if test -z "$CC"; then
	CC=gcc
fi
if test -z "$CXX"; then
	CXX=g++
fi

case "$TARGET_OS" in
    Darwin)
        #PLATFORM_CLIBS="-pthread"
		#PLATFORM_CFLAGS=""
        ;;
    Linux)
        PLATFORM_CLIBS="-pthread -lrt"
        ;;
    OS_ANDROID_CROSSCOMPILE)
        PLATFORM_CLIBS="-pthread"
        SNAPPY_HOST="--host=i386-linux"
        ;;
    CYGWIN_*)
        PLATFORM_CLIBS="-lpthread"
        ;;
    SunOS)
        PLATFORM_CLIBS="-lpthread -lrt"
        ;;
    FreeBSD)
        PLATFORM_CLIBS="-lpthread"
		MAKE=gmake
        ;;
    NetBSD)
        PLATFORM_CLIBS="-lpthread -lgcc_s"
        ;;
    OpenBSD)
        PLATFORM_CLIBS="-pthread"
        ;;
    DragonFly)
        PLATFORM_CLIBS="-lpthread"
        ;;
    HP-UX)
        PLATFORM_CLIBS="-pthread"
        ;;
    *)
        echo "Unknown platform!" >&2
        exit 1
esac


DIR=`pwd`
cd $SNAPPY_PATH
if [ ! -f Makefile ]; then
	echo ""
	echo "##### building snappy... #####"
	./configure $SNAPPY_HOST
	# FUCK! snappy compilation doesn't work on some linux!
	find . | xargs touch
	make
	echo "##### building snappy finished #####"
	echo ""
fi
cd "$DIR"


case "$TARGET_OS" in
	CYGWIN*|FreeBSD|OS_ANDROID_CROSSCOMPILE)
		echo "not using jemalloc on $TARGET_OS"
	;;
	*)
		DIR=`pwd`
		cd $JEMALLOC_PATH
		if [ ! -f Makefile ]; then
			echo ""
			echo "##### building jemalloc... #####"
			sh ./autogen.sh
			./configure
			make
			echo "##### building jemalloc finished #####"
			echo ""
		fi
		cd "$DIR"
	;;
esac


rm -f src/version.h
echo "#ifndef SSDB_DEPS_H" >> src/version.h
echo "#ifndef SSDB_VERSION" >> src/version.h
echo "#define SSDB_VERSION \"`cat version`\"" >> src/version.h
echo "#endif" >> src/version.h
echo "#endif" >> src/version.h
case "$TARGET_OS" in
	CYGWIN*|FreeBSD)
	;;
	OS_ANDROID_CROSSCOMPILE)
        echo "#define OS_ANDROID 1" >> src/version.h
	;;
	*)
		echo "#ifndef IOS" >> src/version.h
		echo "#include <stdlib.h>" >> src/version.h
		echo "#include <jemalloc/jemalloc.h>" >> src/version.h
		echo "#endif" >> src/version.h
	;;
esac

rm -f build_config.mk
echo CC=$CC >> build_config.mk
echo CXX=$CXX >> build_config.mk
echo "MAKE=$MAKE" >> build_config.mk
echo "LEVELDB_PATH=$LEVELDB_PATH" >> build_config.mk
echo "JEMALLOC_PATH=$JEMALLOC_PATH" >> build_config.mk
echo "SNAPPY_PATH=$SNAPPY_PATH" >> build_config.mk

echo "CFLAGS=" >> build_config.mk
echo "CFLAGS = -DNDEBUG -D__STDC_FORMAT_MACROS -Wall -O2 -Wno-sign-compare" >> build_config.mk
echo "CFLAGS += ${PLATFORM_CFLAGS}" >> build_config.mk
echo "CFLAGS += -I \"$LEVELDB_PATH/include\"" >> build_config.mk
echo "CFLAGS += -I \"$TERARK_DB_HOME/include\"" >> build_config.mk

echo "CLIBS=" >> build_config.mk
echo "CLIBS += $TERARK_DB_LIB" >> build_config.mk
echo "CLIBS += $TERARK_FSA_LIB" >> build_config.mk
echo "CLIBS += \"$SNAPPY_PATH/.libs/libsnappy.a\"" >> build_config.mk

case "$TARGET_OS" in
	CYGWIN*|FreeBSD|OS_ANDROID_CROSSCOMPILE)
	;;
	*)
		echo "CLIBS += \"$JEMALLOC_PATH/lib/libjemalloc.a\"" >> build_config.mk
		echo "CFLAGS += -I \"$JEMALLOC_PATH/include\"" >> build_config.mk
	;;
esac

echo "CLIBS += ${PLATFORM_CLIBS}" >> build_config.mk


if test -z "$TMPDIR"; then
    TMPDIR=/tmp
fi

g++ -x c++ - -o $TMPDIR/ssdb_build_test.$$ 2>/dev/null <<EOF
	#include <unordered_map>
	int main() {}
EOF
if [ "$?" = 0 ]; then
	echo "CFLAGS += -DNEW_MAC" >> build_config.mk
fi

