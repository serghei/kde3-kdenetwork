#ifndef INTERFACE_DCOP_H
#define INTERFACE_DCOP_H

#include <dcopobject.h>

class dcop_interface : virtual public DCOPObject
 {
     K_DCOP
     k_dcop:
 
     virtual QString interface() const = 0;
 };

#endif

