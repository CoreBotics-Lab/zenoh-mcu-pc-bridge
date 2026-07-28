import os
import importlib
import sys

# 1. Import standard predefined message types
from ..msg_interface.pre_defined_interface.z_std_msgs import *
from ..msg_interface.pre_defined_interface.z_geometry_msgs import *

# 2. Dynamically scan and import all modules inside custom_interface/msgs/
custom_msgs_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../msg_interface/custom_interface/msgs'))
if os.path.exists(custom_msgs_path):
    # Iterate through package subdirectories (e.g. custom_msgs)
    for pkg_dir in os.listdir(custom_msgs_path):
        pkg_path = os.path.join(custom_msgs_path, pkg_dir)
        if os.path.isdir(pkg_path):
            # Scan files inside package subdirectory
            for file in os.listdir(pkg_path):
                if file.endswith('.py') and not file.startswith('__'):
                    mod_name = file[:-3]
                    module_path = f"zenoh_ros.msg_interface.custom_interface.msgs.{pkg_dir}.{mod_name}"
                    try:
                        module = importlib.import_module(module_path)
                        # Expose module classes to this package namespace
                        target_class = f"z_{mod_name}"
                        if hasattr(module, target_class):
                            globals()[target_class] = getattr(module, target_class)
                    except Exception:
                        pass
