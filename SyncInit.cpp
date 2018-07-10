#include "CameraModel.h"
#include "Observer.h"

// ARC lock
Synchronized CameraModel::_syncObject = Synchronized();

// Obeserver lock
Synchronized Observable::_syncObject = Synchronized();