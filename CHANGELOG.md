# v0.2.0-alpha3

## Highlights

- Added a built-in Git workflow for status, log graph, diff/show, commit, push, fetch, merge/rebase, upstream detection, proxy configuration, and conflict resolution.
- Introduced a new terminal experience based on vterm/ConPTY, with ANSI/true-color rendering, UTF-8 support, cursor handling, mouse scroll, middle-click paste, and OSC 52 clipboard support.
- Added WebView and Markdown page support on top of the DocumentModule foundation.
- Added BigModel provider support.
- Added terminal session management and Markdown syntax highlighting support.
- Improved the desktop UI with better docking and split handles, a tree view toolbar, theme fixes, and explorer fixes.

## Git

- Added Git status, branch, remote, upstream, log graph, and file change display.
- Added commit and push windows, first-push handling, fetch support, and reset/show flows.
- Added merge/rebase and conflict resolution pages/widgets, including single-file and multi-file conflict handling.
- Added Git proxy support.
- Improved Git reliability with process queueing, file watcher handling, UTF-8 fixes, path fixes, and safer behavior during conflict resolution.

## Terminal

- Added vterm/ConPTY terminal widget infrastructure.
- Added ANSI color and true-color rendering.
- Added UTF-8 support, cursor rendering/blinking, resize behavior, scroll handling, mouse/key/modifier event routing, middle-click paste, bell callback, and OSC 52 clipboard support.
- Added configurable terminal sessions and batch/PowerShell icons.
- Added a terminal management window and persisted terminal configuration back to `config.json`.

## Document

- Added Markdown page support.
- Added WebView widget support.
- Added document navigation integration with status callbacks.
- Added conflict page/widget support for Git conflict resolution.
- Added Highlight.js-backed Markdown code highlighting.
- Improved Markdown HTML rendering and fixed several Markdown page issues.

## Coding

- Added BigModel provider support.
- Added agent interpreter mode for returning results to the LLM.

## UI

- Improved menu bar, status bar, tree view, split handles, docking visuals, file property dialog, and executable icon.
- Added tooltips to background task/status models.
- Fixed several layout, margin, overlay, theme, and explorer issues.

## Infrastructure

- Added background task infrastructure and status callbacks.
- Reorganized Git-related files under `service/git`.
- Added `QHtmlString` and `QFullHtmlString` conversions to `uni_cast`.
- Optimized CMake configuration and internal signal/property handling.

# v0.2.0-alpha2

## Coding

- Integrated initial LLM support with DeepSeek.

## Dependency

- Upgraded Qt from 6.10.1 to 6.11.1.

## APIs

- Rewrote ModbusAscii APIs.
- Rewrote ModbusRtu APIs.
- Added ModbusTcp support.
- Upgraded SMTP APIs.
- Added IMAP support.
- Added file operation APIs.

## Infrastructure

- Implemented the DocumentModule foundation.
- Added initial image, text, code, and PDF page support.

## UI

- Added dark theme support.
- Added password reveal-on-hover.
- Expanded the menu bar with edit and code actions.
- Added the file property dialog.

# v0.2.0-alpha1

## Infrastructure

- Switched from QScintilla to Scintilla to support newer editor features.
- Added a ring buffer class and API registration for improved port control.
- Changed the overlay to a fullscreen window for better visual presentation.

## Coding

- Added assembly view.
- Improved `textDocument/signatureHelp` support, including function overloads.
- Improved `textDocument/documentSymbol` support, with symbol breadcrumbs displayed below the coding area.

## UI

- Expanded status bar details for cursor position, EOL type, and encoding format.

# v0.1.0

## Initial release
