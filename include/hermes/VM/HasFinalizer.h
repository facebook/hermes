#ifndef HERMES_VM_HASFINALIZER_H
#define HERMES_VM_HASFINALIZER_H

/// Template parameter passed in during allocation, that signifies whether or
/// not the cell being allocated has a finalizer. Some GC implementations can
/// use this information to speed up finalizer handling.
enum class HasFinalizer { No = 0, Yes };

#endif
