#ifndef STD_MSGS_Z_HEADER_PC_H
#define STD_MSGS_Z_HEADER_PC_H
// z_std_msgs.h already #includes z_builtin_interfaces.h internally,
// so including it here transitively makes z_Time available too.
#include "../../msg_interface/pre_defined_interface/z_std_msgs.h"
// Convenience aliases — user code only needs <zenoh_ros/std_msgs/z_Header.h>
using z_Header   = z_std_msgs::z_Header;
using z_Time     = builtin_interfaces::z_Time;
using z_Duration = builtin_interfaces::z_Duration;
#endif
