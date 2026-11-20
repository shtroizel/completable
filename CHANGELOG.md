# Changelog

All notable changes to completable are listed here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and versions follow
[Semantic Versioning](https://semver.org/spec/v2.0.0.html). The public interface is the command line, the
shell commands and the build options: removing or changing the meaning of one is a major change. Each
release is tagged vX.Y.Z.

## [Unreleased]

## [1.6.0] - 2026-11-19

### Added
- Shell mode without curses: `-c <command>` runs shell commands in order and exits, `-s` starts in the
  shell, `-n` leaves timing out of shell output so runs diff cleanly, `-l <library>` loads a matchmaker
  library at startup in dynamic loading builds, and `-h` prints usage
- With `-s` on a terminal, `:curses` switches to curses and `$` switches back, as often as needed; only
  `:q` (or ESC in curses) quits
- A ctest suite: shell commands, options, `:it` and `:itl` bounds, and curses tests through a pty driver
  (test/curses_driver.py, which needs pyte)
- CHANGELOG.md

### Changed
- Pinned to matchmaker 4.1.0 and matchable 15.0.0
- Property observers have ids, as matchable 15 requires, and run in id order
- Shell timing reports nanoseconds on a steady clock
- Length completions are sorted with std::sort, and get_words() is read through a const reference
- The matchmaker functions are listed once, in MM_FUNCTIONS, for the shim declarations, loads and clears

### Fixed
- Piped input ends the shell at end of input instead of printing the prompt forever
- `:it` and `:itl` no longer overflow on large counts or print rows for negative starts
- `:q` exits through the normal path, so the library is unloaded; `:def` output spells "definition"
- The busy animation waits on a condition variable instead of polling a shared bool, and a throwing task
  stops its thread before rethrowing
- Loading a library from the Matchmaker tab runs its observers after the busy animation stops, since
  ncurses isn't thread-safe
- Delete restores the display start of the window it switches to
- A failed or partial library load unloads everything and clears the book shims, so `:book` no longer
  calls into an unloaded library
- Long options in build_and_install.py take their values (`--build_dir=x`, `--install_dir=x`,
  `--matchmaker_dir=x`)
