# v0.2.0-alpha2

## Dependency

- Qt: 6.10.1 -> 6.11.0

## APIS

- Rewrite ModbusAscii APIs.
- Rewrite ModbusRtu APIs.
- Added ModbusTcp support.
- Upgrade SMTP APIs.
- Added IMAP support.
- Added file operations.

## Infrastructure

- DocumentModule implemented.
- Current supports image/text/code page. More to go...

## UI

- Password is now shown when hovered.
- Expanded menu bar features to perform edit, code actions.
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