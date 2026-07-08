#ifndef UNICOMM_TERMINALTYPES_H
#define UNICOMM_TERMINALTYPES_H

namespace TerminalCursorShape {
    enum {
        Block = 1,
        Underline,
        BarLeft,
    };
}

namespace TerminalMouseMode {
    enum {
        None = 0,
        Click,
        Drag,
        Move,
    };
}

#endif //UNICOMM_TERMINALTYPES_H
