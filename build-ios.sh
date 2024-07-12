#!/bin/bash
set -e

PLATFORMPATH="/Applications/Xcode.app/Contents/Developer/Platforms"
TOOLSPATH="/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin"
export IPHONEOS_DEPLOYMENT_TARGET="8.0"
pwd=`pwd`

findLatestSDKVersion()
{
    sdks=`ls $PLATFORMPATH/$1.platform/Developer/SDKs`
    arr=()
    for sdk in $sdks
    do
       arr[${#arr[@]}]=$sdk
    done

    # Last item will be the current SDK, since it is alpha ordered
    count=${#arr[@]}
    if [ $count -gt 0 ]; then
       sdk=${arr[$count-1]:${#1}}
       num=`expr ${#sdk}-4`
       SDKVERSION=${sdk:0:$num}
    else
       SDKVERSION="8.0"
    fi
}

buildit()
{
    target=$1
    hosttarget=$1
    platform=$2

    if [[ $hosttarget == "x86_64" ]]; then
        hosttarget="i386"
        gmpinclude="./Frameworks/GMP.xcframework/ios-x86_64-simulator/Headers"
        gmplib="./Frameworks/GMP.xcframework/ios-x86_64-simulator"
    elif [[ $hosttarget == "arm64" ]]; then
        hosttarget="arm"
        gmpinclude="./Frameworks/GMP.xcframework/ios-arm64/Headers"
        gmplib="./Frameworks/GMP.xcframework/ios-arm64"
    fi

    export CC="$(xcrun -sdk iphoneos -find clang)"
    export CPP="$CC -E"
    export CFLAGS="-arch ${target} -isysroot $PLATFORMPATH/$platform.platform/Developer/SDKs/$platform$SDKVERSION.sdk -miphoneos-version-min=$SDKVERSION"
    export AR=$(xcrun -sdk iphoneos -find ar)
    export RANLIB=$(xcrun -sdk iphoneos -find ranlib)
    export CPPFLAGS="-arch ${target}  -isysroot $PLATFORMPATH/$platform.platform/Developer/SDKs/$platform$SDKVERSION.sdk -miphoneos-version-min=$SDKVERSION"
    export LDFLAGS="-arch ${target} -isysroot $PLATFORMPATH/$platform.platform/Developer/SDKs/$platform$SDKVERSION.sdk"

    mkdir -p $pwd/output/$target

    ./configure --prefix="$pwd/output/$target" --enable-shared=no --host=$hosttarget-apple-darwin  --with-gmp-include=$gmpinclude --with-gmp-lib=$gmplib

    make clean
    make
    make install
    make clean
}

findLatestSDKVersion iPhoneOS

buildit x86_64 iPhoneSimulator
buildit arm64 iPhoneOS

xcodebuild -create-xcframework -library "$pwd/output/arm64/lib/libmpfr.a" -headers "$pwd/output/arm64/include" -library "$pwd/output/x86_64/lib/libmpfr.a" -headers "$pwd/output/x86_64/include" -output "$pwd/output/MPFR.xcframework"

# LIPO=$(xcrun -sdk iphoneos -find lipo)
# $LIPO -create $pwd/output/armv7/lib/libpresage.a  $pwd/output/armv7s/lib/libpresage.a $pwd/output/arm64/lib/libpresage.a $pwd/output/x86_64/lib/libpresage.a $pwd/output/i386/lib/libpresage.a -output libpresage.a
