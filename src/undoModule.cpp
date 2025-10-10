#include "undoModule.h"

UndoModule::UndoModule(QObject *parent)
    : QUndoStack(parent) {
    this->setUndoLimit(100);
}
