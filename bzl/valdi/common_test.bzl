"""Unit tests for constants exported by common.bzl."""

load("@bazel_skylib//lib:unittest.bzl", "asserts", "unittest")
load(":common.bzl", "IOS_OS_CONSTRAINT")

def _ios_os_constraint_is_resolved_label_test_impl(ctx):
    env = unittest.begin(ctx)

    # IOS_OS_CONSTRAINT must be a Label constructed at Valdi's own load time,
    # not a bare "@platforms//os:ios" string. A bare string passed to a
    # target_compatible_with attribute resolves against the *calling* package's
    # repo mapping, so any downstream module that calls valdi_module() but does
    # not itself declare the `platforms` bazel_dep fails to build for iOS.
    # A Label bakes in Valdi's repo mapping regardless of caller. See
    # Valdi_Widgets#20.
    asserts.equals(env, "Label", type(IOS_OS_CONSTRAINT))

    constraint_str = str(IOS_OS_CONSTRAINT)
    asserts.true(
        env,
        constraint_str.endswith("//os:ios"),
        "expected constraint to target //os:ios, got %s" % constraint_str,
    )
    asserts.true(
        env,
        "platforms" in constraint_str,
        "expected constraint to resolve against the platforms repo, got %s" % constraint_str,
    )

    return unittest.end(env)

ios_os_constraint_is_resolved_label_test = unittest.make(_ios_os_constraint_is_resolved_label_test_impl)

def common_test_suite(name):
    """Registers unit tests for common.bzl.

    Args:
        name: name of the generated test_suite target.
    """
    unittest.suite(
        name,
        ios_os_constraint_is_resolved_label_test,
    )
