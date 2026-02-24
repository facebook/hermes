# Boost External Dependencies

This directory contains Boost 1.76.0 component libraries needed for the Hermes inspector functionality.

## Version Information
- **Version**: 1.76.0
- **Source**: Individual component repositories from https://github.com/boostorg/
- **Components**: 22 header-only Boost libraries

## Component List
The following Boost components are included:

1. **algorithm** - Generic algorithms
2. **asio** - Asynchronous I/O and networking
3. **assert** - Customizable assert macros
4. **bind** - Function object binders
5. **concept_check** - Concept checking utilities
6. **config** - Configuration and platform detection
7. **container_hash** - Hash functions for containers
8. **core** - Core utilities
9. **detail** - Implementation details
10. **function** - Function object wrappers
11. **integer** - Integer type selection and operations
12. **iterator** - Iterator utilities and adaptors
13. **move** - Move semantics utilities
14. **mpl** - Metaprogramming library
15. **preprocessor** - Preprocessor metaprogramming
16. **range** - Range-based algorithms and utilities
17. **static_assert** - Static assertion macros
18. **throw_exception** - Exception throwing utilities
19. **type_index** - Type indexing utilities
20. **type_traits** - Type traits and metafunctions
21. **utility** - Utility components
22. **variant** - Type-safe unions

## Directory Structure
```
external/boost/
├── README.md                (this file)
├── download_boost.ps1       (download script)
├── algorithm/               (boost algorithm component)
│   ├── include/
│   └── ...
├── asio/                    (boost asio component)
│   ├── include/
│   └── ...
├── assert/                  (boost assert component)
└── ...                      (20 more components)
```

## Setup Instructions
To set up or update Boost components:

1. **Download and clean all components** (already completed):
   ```powershell
   cd external/boost
   .\download_boost.ps1
   ```

2. **Components are automatically cleaned** during download:
   - Git repositories (`.git/`) are removed
   - Only essential files are kept: `include/`, `meta/`, `CMakeLists.txt`, and source files
   - Documentation, examples, and tests are removed to minimize size

3. **Components are automatically detected** by CMake configuration

## Conversion Benefits
- ✅ **Eliminates 22 FetchContent warnings** - No more deprecation messages
- ✅ **Significantly faster configuration** - No network downloads for 22 repositories
- ✅ **Better CI reliability** - No network dependencies during build
- ✅ **Offline builds** - Can build without internet connectivity
- ✅ **Version control** - Full control over exact source code used
- ✅ **Consistent versions** - All components locked to boost-1.76.0
- ✅ **Repository-safe** - Git metadata removed, safe to add to your repository
- ✅ **Minimal footprint** - Only essential files preserved

## Usage
The Boost components are used by the Hermes inspector functionality. All 22 component include directories are automatically added to the build path via the `BOOST_INCLUDES` variable.

## Updating
When updating Boost components:

1. **Update the version** in `download_boost.ps1` (change `boost-1.76.0` to new version)
2. **Re-run the download script** (will automatically clean up):
   ```powershell
   # Remove existing components
   Remove-Item algorithm, asio, assert, bind, concept_check, config, container_hash, core, detail, function, integer, iterator, move, mpl, preprocessor, range, static_assert, throw_exception, type_index, type_traits, utility, variant -Recurse -Force
   # Download and clean new versions
   .\download_boost.ps1
   ```
3. **Update this README** with new version information
4. **Test the build** to ensure compatibility

## Technical Notes
- All components are header-only libraries (no compilation required)
- Each component is cloned with `--depth 1` for minimal download size
- **Git repositories are automatically removed** after download
- **Only essential files are preserved**: `include/`, `meta/`, `CMakeLists.txt`, source files
- **Removed during cleanup**: documentation, examples, tests, CI files, Git metadata
- Components are automatically validated during CMake configuration
- Original FetchContent approach downloaded 22 separate repositories during each configuration
- **Safe for version control**: No Git metadata conflicts with your main repository

## Maintenance
- **Component repositories**: https://github.com/boostorg/[component-name]
- **Documentation**: https://www.boost.org/doc/libs/1_76_0/
- **Release notes**: https://www.boost.org/users/history/version_1_76_0.html

Successfully converted from FetchContent to external dependencies on July 28, 2025.
