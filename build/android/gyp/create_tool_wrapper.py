#!/usr/bin/env python
#
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Creates a simple wrapper script that passes the correct --output-directory.
"""

import argparse
import os

_TEMPLATE = """\
#!/usr/bin/env python
#
# This file was generated by //build/android/gyp/create_tool_script.py

import os
import sys

cmd = '{cmd}'
args = [os.path.basename(cmd), '{flag_name}={output_directory}'] + sys.argv[1:]
os.execv(cmd, args)
"""

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--output', help='Output path for executable script.')
  parser.add_argument('--target', help='Path to script being wrapped.')
  parser.add_argument('--output-directory', help='Value for --output-directory')
  parser.add_argument('--flag-name',
                      help='Flag name to use instead of --output-directory',
                      default='--output-directory')
  args = parser.parse_args()

  with open(args.output, 'w') as script:
    script.write(_TEMPLATE.format(
        cmd=os.path.abspath(args.target),
        flag_name=args.flag_name,
        output_directory=os.path.abspath(args.output_directory)))

  os.chmod(args.output, 0750)


if __name__ == '__main__':
  main()
