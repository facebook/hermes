The original benchmark stripped of types so it can run with v8. The conversion
was performed by the script `strip.sh` using `Juno`.

To run the benchmark, use `v8 main.mjs`.

It can probably run with NodeJS too, but appears that all files need to be renamed
to have a `.mjs` extension as well as the imports referring to them.
