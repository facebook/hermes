export HERMES_WS_DIR=~/code/hermes_ws_perf/
cd $HERMES_WS_DIR/hermes/android

./gradlew githubRelease

./gradlew clean
rm -rf $HERMES_WS_DIR/build_android/outputs 

./gradlew githubReleaseNoIntl

mkdir $HERMES_WS_DIR/build_android/distributions/hermes-runtime-android-intl
mkdir $HERMES_WS_DIR/build_android/distributions/hermes-runtime-android-nointl
tar -C $HERMES_WS_DIR/build_android/distributions/hermes-runtime-android-intl -zxvf $HERMES_WS_DIR/build_android/distributions/hermes-runtime-android-intl-v0.7.0.tar.gz
tar -C $HERMES_WS_DIR/build_android/distributions/hermes-runtime-android-nointl -zxvf $HERMES_WS_DIR/build_android/distributions/hermes-runtime-android-nointl-v0.7.0.tar.gz