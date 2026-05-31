# Only takes effect on Windows.
set windows-shell := ['cmd.exe', '/d', '/s', '/c']

import 'recipes/common.just'
import 'recipes/check.just'
import 'recipes/extra.just'

# Top-level builds default to the host's backend and compiler. We use Ninja on windows to simplify recipes
# cross-platform.
profile_args := if os_family() == 'windows' { '-c tools.cmake.cmaketoolchain:generator=Ninja' } else { '' }
backend_args := ''
check_args := if os_family() == 'windows' { '-o evget/*:run_msvc_analyze=True' } else { '-o evget/*:run_clang_tidy=True' }

# Generate documentation.
doc *opts='': (build opts)
    -git clone --depth 1 --branch v2.3.4 https://github.com/jothepro/doxygen-awesome-css.git build/doxygen-awesome-css
    cmake -E env CLANG_DATABASE_PATH=build/{{ capitalize(build_type) }} doxygen

# Linux related commands
mod linux 'recipes/linux.just'

# Windows related commands
mod windows 'recipes/windows.just'
