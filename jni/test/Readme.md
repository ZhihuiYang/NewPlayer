### use googltest perform unit test for mediaplayer ###
add test case into foo_unittest.cpp or create a new test cases container file, but remember to modify Android.mk,
add the new test cases container file to LOCAL_SRC_FILES .

build test application:
    run build.cmd
    it will rebuild mediaplayer and generate a command line application named "foo_unittest" 

run test case:
    adb push libs/armeabi/foo_unitest /data/local/tmp/
    adb push libs/armeabi/libmediaplayer.so /data/local/tmp/
    adb push libs/armeabi/libopenh264.so /data/local/tmp/
    adb shell
    cd /data/local/tmp
    chmod 777 foo_unitest
    export LD_LIBRARY_PATH=.
    ./foo_unitest

    