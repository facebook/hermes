## Release Procedure

In your `~/.gradle/gradle.properties`, set `bintrayUsername` and `bintrayApiKey` values
you can find in your [Bintray profile](https://bintray.com/profile/edit).

1. Bump the `VERSION_NAME` in `gradle.properties`.
2. `hg commit -m 'vx.y.z'`, submit, land.
3. From the checked out GitHub repository, run `./gradlew bintrayUpload` (Android uploads).
4. From the checked out GitHub repository, run `./gradlew -b host.gradle bintrayUpload` (Java-only uploads).
5. Bump the `VERSION_NAME` to the next patch release and appending `-SNAPSHOT`, commit, land.
6. Tag the release on the [Github release page](https://github.com/facebookincubator/fbjni/releases).
