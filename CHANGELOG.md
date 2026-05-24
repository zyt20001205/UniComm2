# v0.2.0-alpha2

## Coding
- Integrated LLM support, currently supports deepseek. More to come...

## Dependency

- Qt: 6.10.1 -> 6.11.1

## APIS

- Rewrote ModbusAscii APIs.
- Rewrote ModbusRtu APIs.
- Added ModbusTcp support.
- Upgraded SMTP APIs.
- Added IMAP support.
- Added file operations.

## Infrastructure

- Implemented DocumentModule.
- Currently supports image/text/code/pdf pages. More to come...

## UI

- Added dark theme support.
- Password is now revealed on hover.
- Expanded menu bar with edit and code actions.
- Added file property dialog.

# v0.2.0-alpha1

## Infrastructure

- Switched from QScintilla to Scintilla to support the latest editor features.
- Added a ring buffer class and API registration for improved port control.
- Changed overlay to a fullscreen window for better visual presentation.

## Coding

- Added assembly view.
- Better support for `textDocument/signatureHelp` requests, now including function overload.
- Better support for `textDocument/documentSymbol` requests, with symbol breadcrumb displayed below the coding area.

## UI

- Expanded status bar features to display cursor position, EOL type, and encoding format.

# v0.1.0

## Initial release