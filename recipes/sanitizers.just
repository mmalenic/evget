# Sanitizer recipes which call out to scripts/sanitizers.py. Importing modules define
# `profile_args`, `backend_args`, and `compiler`.

[private]
_sanitizer action name filter='*' *opts='':
    python scripts/sanitizers.py {{ action }} {{ name }} \
        --compiler {{ compiler }} --build-type {{ capitalize(build_type) }} \
        {{ prepend('--profile-arg=', profile_args) }} \
        {{ prepend('--backend-arg=', backend_args) }} \
        --filter={{ filter }} {{ prepend('--opt=', opts) }}

# Run one sanitizer by name.
[group('sanitizer')]
test_sanitizer name filter='*' *opts='': (_sanitizer 'run' name filter opts)

# Run every sanitizer this compiler supports.
[group('sanitizer')]
test_sanitizers filter='*' *opts='': (_sanitizer 'run' '' filter opts)

# Build a sanitizer instrumentation tree.
[group('sanitizer')]
build_sanitizer name filter='*' *opts='': (_sanitizer 'build' name filter opts)

# Remove one sanitizer build tree.
[group('sanitizer')]
clean_sanitizer name:
    python scripts/sanitizers.py clean {{ name }} \
        --compiler {{ compiler }} --build-type {{ capitalize(build_type) }}

# Remove every sanitizer build this compiler supports.
[group('sanitizer')]
clean_sanitizers:
    python scripts/sanitizers.py clean \
        --compiler {{ compiler }} --build-type {{ capitalize(build_type) }}
