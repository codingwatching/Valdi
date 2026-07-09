"""A macOS test runner that executes the `.xctest` bundle via the `xctest`
agent directly, instead of `xcodebuild test-without-building`.

rules_apple's default macOS runner drives tests through `xcodebuild`, which
requires a live `testmanagerd` service — only available inside a GUI/Aqua
login session. Headless CI runners have no such session, so `xcodebuild`
fails with "The connection to service named com.apple.testmanagerd.control
was invalidated ... No such process" before any test runs. The `xctest`
agent loads the bundle in-process and needs no testmanagerd, so it runs the
same tests headlessly.
"""

load("@build_bazel_rules_apple//apple:providers.bzl", "apple_provider")

def _macos_xctest_agent_runner_impl(ctx):
    ctx.actions.expand_template(
        template = ctx.file._test_template,
        output = ctx.outputs.test_runner_template,
        substitutions = {},
    )

    execution_environment = {}
    xcode_config = ctx.attr._xcode_config[apple_common.XcodeVersionConfig]
    xcode_version = str(xcode_config.xcode_version())
    if xcode_version:
        execution_environment["XCODE_VERSION_OVERRIDE"] = xcode_version

    return [
        apple_provider.make_apple_test_runner_info(
            test_runner_template = ctx.outputs.test_runner_template,
            execution_requirements = {"requires-darwin": ""},
            execution_environment = execution_environment,
        ),
        DefaultInfo(),
    ]

macos_xctest_agent_runner = rule(
    _macos_xctest_agent_runner_impl,
    attrs = {
        "_test_template": attr.label(
            default = Label("//bzl/testing:macos_xctest_agent_runner.template.sh"),
            allow_single_file = True,
        ),
        "_xcode_config": attr.label(
            default = configuration_field(
                fragment = "apple",
                name = "xcode_config_label",
            ),
        ),
    },
    outputs = {"test_runner_template": "%{name}.sh"},
    fragments = ["apple", "objc"],
    doc = "macOS test runner that runs the xctest bundle via the xctest agent (no testmanagerd).",
)
