"""
Output writer module for CodeCtx.

This module is responsible for writing the generated code context to an output file.
"""

import logging
import os
from typing import List

from .file_utils import is_binary_file, read_file_contents

logger = logging.getLogger(__name__)


def write_output(src_path: str, file_tree: List[str], output_file: str) -> None:
    """
    Write the generated code context to an output file.

    Args:
        src_path (str): The root directory of the source code.
        file_tree (List[str]): List of relative file paths in the source directory.
        output_file (str): The path to the output file where the context will be written.
    """
    with open(output_file, "w", encoding="utf-8") as f:
        f.write("Directory Tree:\n")
        f.write("===============\n")
        for item in file_tree:
            f.write(f"{item}\n")
        f.write("\n\nFile Contents:\n")
        f.write("==============\n")
        for item in file_tree:
            full_path = os.path.join(src_path, item)
            f.write(f"\n--- {item} ---\n")
            if is_binary_file(full_path):
                f.write("[Binary file, content not displayed]\n")
            else:
                try:
                    content = read_file_contents(full_path)
                    f.write(content)
                except (IOError, UnicodeDecodeError) as e:
                    # logger.error(f"Error reading file {full_path}: {str(e)}")
                    logger.error("Error reading file %s: %s", full_path, str(e))
                    f.write(f"Error reading file: {str(e)}\n")
            f.write("\n")
