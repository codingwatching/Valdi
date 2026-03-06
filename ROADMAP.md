# Valdi Roadmap

Valdi is currently in **beta**. This roadmap covers what we're working on before 1.0 and beyond.

This is a living document. For questions or feedback, open a [GitHub Discussion](https://github.com/Snapchat/Valdi/discussions).

---

## Current status: beta

Valdi has been in production at Snap for 8 years. The beta label reflects the open source tooling and documentation, not the framework itself. We'll exit beta when the developer experience is solid end-to-end.

**Known friction in the current beta:**
- `valdi dev_setup` requires `git-lfs` to download pre-built binaries — we're working to remove this
- Bazel integration requires `local_path_override` workarounds for projects outside this repo
- Documentation has gaps, especially around advanced native integration

---

## Near-term (pre-1.0)

### Binary publishing via GitHub Releases
Pre-built compiler binaries and runtime libraries will be hosted on GitHub Releases instead of checked into the repo. This removes the `git-lfs` requirement and makes `valdi dev_setup` work reliably without any additional tools.

### Bzlmod support
Proper [bzlmod](https://bazel.build/external/bzlmod) support, so Valdi can be used as a standard Bazel module without `local_path_override`. This is a prerequisite for Bazel Central Registry submission.

### Interactive playground
A zero-install way to try Valdi — write and run components without setting up a local environment. Useful for quick experimentation and sharing examples.

### 1.0 release
1.0 will ship when the following are true:

- [ ] `valdi dev_setup` works reliably first-try on macOS arm64 and Linux
- [ ] Bazel integration works without `local_path_override` hacks
- [ ] Compiler has no known stability regressions for a full release cycle
- [ ] Interactive playground exists (zero-install eval path)
- [ ] `CONTRIBUTING.md` and PR triage process established and running
- [ ] Core docs cover: getting started, `onRender`, component model, troubleshooting
- [ ] At least one external contributor has merged a non-trivial code PR

---

## Longer-term (post-1.0)

### Bazel Central Registry
Once bzlmod is working and upstream rule patches are merged, we'll submit Valdi to the [Bazel Central Registry](https://registry.bazel.build/) so it's discoverable by the entire Bazel ecosystem.

### Hosted documentation site
A proper versioned, searchable documentation site. The current docs in `/docs/` are comprehensive but not web-hosted yet.

### Web target (2.0)
Valdi already has early web support. 2.0 will make this a first-class target. The API may change before 2.0; not recommended for production yet.

---

## What's not on this roadmap

- **Windows / Intel Mac support** — not planned
- **Non-Bazel build system** — Bazel is the build system; we may evaluate CMake for non-Bazel environments post-1.0
- **Community library registry** — use [GitHub Discussions](https://github.com/Snapchat/Valdi/discussions) to share libraries for now

---

## Contributing

The items on this roadmap represent what we're focused on, but contributions in any area are welcome. See [CONTRIBUTING.md](./CONTRIBUTING.md) for how to contribute, including what types of PRs are realistic given the monorepo model.

For feature requests, open a [GitHub Discussion](https://github.com/Snapchat/Valdi/discussions) so we can discuss before you invest in a large PR.
