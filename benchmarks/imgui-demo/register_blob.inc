// This file appended to blobs by the build system.
void register_blob(const char *name, const unsigned char *data, unsigned size);
__attribute__((constructor)) static void init(void)  {
    register_blob(NAME, blob, sizeof(blob));
}
