# Module extension to expose compiler tool repos to the valdi_toolchain sub-module.
#
# In WORKSPACE mode, these repos are created by setup_additional_dependencies() in
# workspace_prepare.bzl. In bzlmod mode, WORKSPACE.bzlmod is used instead, so
# setup_additional_dependencies() is never called. This extension fills that gap by
# creating the same repos and making them visible to @@valdi_toolchain~.

SOURCES_FILEGROUP_BUILD_FILE_CONTENT = """
exports_files(glob(["**"]))
filegroup(
    name = "all_files",
    srcs = glob(["**/*"]),
    visibility = ["//visibility:public"],
)
"""

def _compiler_local_dir_impl(ctx):
    # Resolve the valdi workspace root via a known label.
    # Works whether @valdi is the main workspace or an external dep.
    valdi_root = ctx.path(Label("@valdi//:MODULE.bazel")).dirname
    target_path = valdi_root.get_child(ctx.attr.target_dir)

    # Check if the directory exists (binaries may not be fetched yet)
    check = ctx.execute(["test", "-d", str(target_path)])
    if check.return_code != 0:
        # Create an empty repo so the build doesn't fail during analysis.
        # Actual builds that need the binary will fail at execution time.
        ctx.file("BUILD.bazel", SOURCES_FILEGROUP_BUILD_FILE_CONTENT)
        return

    result = ctx.execute(["ls", "-1", str(target_path)])
    file_list = [f for f in result.stdout.strip().split("\n") if f]

    has_build_file = False
    for f in file_list:
        if f in ["BUILD", "BUILD.bazel"]:
            has_build_file = True
            break

    for f in file_list:
        ctx.symlink(str(target_path) + "/" + f, f)

    if not has_build_file:
        ctx.file("BUILD.bazel", SOURCES_FILEGROUP_BUILD_FILE_CONTENT)

_compiler_local_dir = repository_rule(
    implementation = _compiler_local_dir_impl,
    attrs = {"target_dir": attr.string(mandatory = True)},
    local = True,
)

def _valdi_compiler_repos_impl(module_ctx):
    _compiler_local_dir(name = "valdi_compiler_macos", target_dir = "bin/compiler/macos")
    _compiler_local_dir(name = "valdi_compiler_linux", target_dir = "bin/compiler/linux")
    _compiler_local_dir(name = "valdi_pngquant_macos", target_dir = "bin/pngquant/macos")
    _compiler_local_dir(name = "valdi_pngquant_linux", target_dir = "bin/pngquant/linux")
    _compiler_local_dir(name = "jscore_libs", target_dir = "third-party/jscore/libs")

valdi_compiler_repos = module_extension(
    implementation = _valdi_compiler_repos_impl,
)
