# Check recipe. Importing modules must define `check_args`.

# Run static analysis check.
check *opts='': (build_test '-o evget/*:verify_headers=True ' + check_args + ' ' + opts)
