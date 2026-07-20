#include "agent/provider/baseProvider.h"

BaseProvider::BaseProvider(QObject *parent)
    : QObject(parent),
      m_service("UniComm") {
}
