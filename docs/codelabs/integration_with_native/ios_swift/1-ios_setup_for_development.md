# Setup for development
In the last section, we generated a new valdimodule. Now, let's hook it all up to iOS.
We will create a new option in the Settings menu from scratch and will load a new view when it is clicked.

If you're continuing directly from the last section. You'll need to reset the Playground.tsx file back to the original to prevent compilation errors. We won't be using the Playground module in this part.

## Building your component for Swift
In the module's `BUILD.bazel`, set `ios_language = "swift"` on the `valdi_module()` call. The relevant attributes should look like:

```python
valdi_module(
    name = "hello_world",
    # ...
    ios_module_name = "SCCHelloWorld",
    ios_class_prefix = "SCCHelloWorld",
    ios_language = "swift",
)
```

**TODO**: Update this for open source
