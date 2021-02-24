## Release Procedure (Semi-Automated)

1. Bump the `VERSION_NAME` in `gradle.properties`.
2. `hg commit -m 'vx.y.z'`, submit, land.
3. Create a new release on the [Github release page](https://github.com/facebookincubator/fbjni/releases). This triggers a [release action](https://github.com/facebookincubator/fbjni/actions/workflows/release.yml).
4. Bump the `VERSION_NAME` to the next patch release and appending `-SNAPSHOT`, commit, land.

## Release Procedure (Manual)

In your `~/.gradle/gradle.properties`, set:
    - `SONATYPE_NEXUS_USERNAME`, `SONATYPE_NEXUS_PASSWORD` (you can find these on https://oss.sonatype.org/#profile;User%20Token)
    - `signing.secretKeyRingFile` (to your secring.gpg)
    - `signing.keyId` (Check `gpg --list-secret-keys`)
    - `signing.password`

1. Bump the `VERSION_NAME` in `gradle.properties`.
2. `hg commit -m 'vx.y.z'`, submit, land.
3. From the checked out GitHub repository, run `./gradlew assembleRelease uploadArtifacts` (Android uploads).
4. From the checked out GitHub repository, run `./gradlew -b host.gradle assemble uploadArtifacts` (Java-only uploads).
5. Bump the `VERSION_NAME` to the next patch release and appending `-SNAPSHOT`, commit, land.
6. Tag the release on the [Github release page](https://github.com/facebookincubator/fbjni/releases).
