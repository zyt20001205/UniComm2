#ifndef UNICOMM_GLOBALS_H
#define UNICOMM_GLOBALS_H

#include <visa.h>

#include "mainWindow/mainWindow.h"

class QNetworkAccessManager;
class QStandardItemModel;

class DatabaseModule;
class DatatableModule;
class DataplotModule;
class DebugModule;
class LogModule;
class PortModule;
class DocumentModule;
class StructureModule;
class ThreadpoolModule;
class UndoModule;

extern MainWindow *g_mainWindow;
extern QNetworkAccessManager *g_networkAccessManager;
extern GlobalManager *g_globalManager;
extern Ripgrep *g_ripgrep;
extern DatabaseModule *g_database;
extern DatatableModule *g_datatable;
extern DataplotModule *g_dataplot;
extern DocumentModule *g_document;
extern LogModule *g_log;
extern NuspellModule *g_nuspell;
extern PortModule *g_port;
extern ThreadpoolModule *g_thread;
extern UndoModule *g_undo;

extern bool g_terminating;
extern QUrl g_workspaceUrl;
extern QJsonObject g_mainConfig;
extern QJsonObject g_workspaceConfig;

extern ViSession g_rm;

extern QVariantMap g_cursorPosition;
extern QHash<QChar, int> g_gitStatus;

extern QHash<QUrl, QHash<int, QVariantHash> > g_breakpoints;
extern QStandardItemModel *g_portStandardItemModel;
extern QStandardItemModel *g_databaseStandardItemModel;
extern QStandardItemModel *g_datatableHeaderItemModel;
extern QStandardItemModel *g_datatableStandardItemModel;
extern QStandardItemModel *g_watchStandardItemModel;

namespace Theme {
    enum {
        Light,
        Dark,
    };
}

namespace LogLevel {
    enum {
        Error,
        Warning,
        Info,
        Transmit,
        Receive
    };
}

namespace PortType {
    enum {
        SerialPort,
        Visa,
        TcpClient,
        SslClient,
        TcpServer,
        UdpSocket,
        VideoStream
    };
}

namespace ImagePipeline {
    enum {
        Scale,
        Threshold
    };
}

namespace InterpreterMode {
    enum {
        Run,
        Debug,
        Agent,
        Terminate
    };
}

namespace Debug {
    enum {
        Terminate,
        Resume,
        Pause,
        StepOver,
        StepInto,
        StepOut,
        RunToCursor
    };
}

namespace ScintillaIndicator {
    enum {
        Typo,
        Hint,
        Info,
        Warning,
        Error,
        Password,
        Highlight,
        Read,
        Write,
        Hyperlink,
        Result,
        Current,
        ConflictStart,
        ConflictSeparator,
        ConflictEnd
    };
}

namespace ScintillaMarker {
    enum {
        Region,
        BreakpointEnabled,
        BreakpointDisabled,
        Navigation,
        Debug,
        Hint,
        ConflictCurrent,
        ConflictIncoming
    };
}

namespace LuaTokenType {
    enum {
        Unused,
        Namespace,
        Class,
        Type,
        Parameter,
        Variable,
        Property,
        EnumMember,
        FunctionCall,
        FunctionDeclaration,
        Method,
        Macro,
        Keyword,
        Comment,
        String,
        Number,
        Operator
    };
}

namespace CustomStyle {
    enum {
        Default = 32,
        LineNumber,
        BraceLight,
        BraceBad,
        ControlChar,
        IndentGuide,
        CallTip,
        FoldDisplayText,
        LastPredefined,
        Annotation = 40
    };
}

namespace LspSymbolKind {
    enum {
        File = 1,
        Module,
        Namespace,
        Package,
        Class,
        Method,
        Property,
        Field,
        Constructor,
        Enum,
        Interface,
        Function,
        Variable,
        Constant,
        String,
        Number,
        Boolean,
        Array,
        Object,
        Key,
        Null,
        EnumMember,
        Struct,
        Event,
        Operator,
        TypeParameter
    };
}

namespace LspTokenType {
    enum {
        Namespace,
        Type,
        Class,
        Enum,
        Interface,
        Struct,
        TypeParameter,
        Parameter,
        Variable,
        Property,
        EnumMember,
        Event,
        Function,
        Method,
        Macro,
        Keyword,
        Modifier,
        Comment,
        String,
        Number,
        RegExp,
        Operator,
        Decorator
    };
}

namespace LspTokenModifiers {
    enum {
        Declaration = 1 << 0,
        Definition = 1 << 1,
        Readonly = 1 << 2,
        Static = 1 << 3,
        Deprecated = 1 << 4,
        Abstract = 1 << 5,
        Async = 1 << 6,
        Modification = 1 << 7,
        Documentation = 1 << 8,
        DefaultLibrary = 1 << 9,
        Global = 1 << 10,
    };
}

namespace GitStatus {
    enum {
        /* '?' */Untracked,
        /* '!' */Ignored,
        /* ' ' */Unmodified,
        /* 'M' */Modified,
        /* 'T' */FileTypeChanged,
        /* 'A' */Added,
        /* 'D' */Deleted,
        /* 'R' */Renamed,
        /* 'C' */Copied,
        /* 'U' */UpdatedButUnmerged
    };
}

namespace GitConflict {
    enum {
        None,
        Merge,
        Rebase
    };
}

#endif //UNICOMM_GLOBALS_H
