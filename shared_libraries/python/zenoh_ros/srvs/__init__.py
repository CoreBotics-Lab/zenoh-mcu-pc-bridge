import os
import importlib
import sys

# 1. Dynamically scan and import all modules inside custom_interface/srvs/
custom_srvs_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../msg_interface/custom_interface/srvs'))
if os.path.exists(custom_srvs_path):
    # Iterate through package subdirectories (e.g. custom_srvs)
    for pkg_dir in os.listdir(custom_srvs_path):
        pkg_path = os.path.join(custom_srvs_path, pkg_dir)
        if os.path.isdir(pkg_path):
            # Scan files inside package subdirectory
            for file in os.listdir(pkg_path):
                if file.endswith('.py') and not file.startswith('__'):
                    mod_name = file[:-3]
                    module_path = f"zenoh_ros.msg_interface.custom_interface.srvs.{pkg_dir}.{mod_name}"
                    try:
                        module = importlib.import_module(module_path)
                        # Expose module classes to this package namespace
                        if hasattr(module, mod_name):
                            globals()[mod_name] = getattr(module, mod_name)
                    except Exception:
                        pass
