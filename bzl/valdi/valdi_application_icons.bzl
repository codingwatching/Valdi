load(
    "//bzl/valdi:valdi_application_icons_helper.bzl",
    "make_application_icons",
)

def valdi_application_icons(src, round_src = None):
    return make_application_icons("all", src, round_src = round_src)
