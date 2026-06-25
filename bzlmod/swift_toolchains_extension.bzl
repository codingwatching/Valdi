"""Module extension that instantiates one Swift toolchain repo per
SWIFT_TOOLCHAINS entry.

Toolchains are *registered* in MODULE.bazel itself (via
`register_toolchains(...)`) -- Bazel does not permit
`native.register_toolchains` inside a module extension impl.
"""

load("//bzl:swift_toolchains.bzl", "SWIFT_TOOLCHAINS")
load("//bzl/swift_toolchains:rules.bzl", "make_swift_toolchains")

def _swift_toolchains_ext_impl(_ctx):
    make_swift_toolchains(SWIFT_TOOLCHAINS)

swift_toolchains_extension = module_extension(
    implementation = _swift_toolchains_ext_impl,
)
