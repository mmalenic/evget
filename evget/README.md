# libevget

The platform-independent core of evget. It defines the unified event schema, storage
layers, and the async components that the backends feed events into.

Backend libraries depend on this target and convert platform events into OS-agnostic
types, so a store produced on one platform queries alongside one from another.

This is built as the `libevget` target. See the [top-level README](../README.md) for build and
usage instructions.
