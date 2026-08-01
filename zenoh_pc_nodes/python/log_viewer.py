#!/usr/bin/env python3
"""
log_viewer.py - Standalone Python node to monitor live logs published to '/zenoh_ros/log'.
"""

import sys
import os

# Ensure shared zenoh_ros library is in path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros.logging.viewer import main

if __name__ == '__main__':
    main()
