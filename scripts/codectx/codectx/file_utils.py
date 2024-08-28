"""
Utility functions for file operations in CodeCtx.

This module provides functions for checking if files should be ignored,
generating a file tree, and reading file contents.
"""

import fnmatch
import logging
import os
from typing import List

logger = logging.getLogger(__name__)


def should_ignore(path: str, ignored_patterns: List[str], root_dir: str) -> bool:
    """
    Determine if a file or directory should be ignored based on the ignore patterns.

    Args:
        path (str): The full path of the file or directory to check.
        ignored_patterns (List[str]): List of patterns to ignore.
        root_dir (str): The root directory of the project.

    Returns:
        bool: True if the path should be ignored, False otherwise.
    """
    rel_path = os.path.relpath(path, root_dir)
    path_parts = rel_path.split(os.sep)

    for pattern in ignored_patterns:
        if pattern.endswith("/"):
            for i in range(len(path_parts)):
                check_path = os.path.join(*path_parts[: i + 1]) + "/"
                if fnmatch.fnmatch(check_path, pattern):
                    logger.debug(
                        "Ignoring directory: %s (matched pattern: %s)",
                        rel_path,
                        pattern,
                    )
                    return True
        else:
            if fnmatch.fnmatch(rel_path, pattern) or fnmatch.fnmatch(
                os.path.basename(path), pattern
            ):
                logger.debug(
                    "Ignoring file: %s (matched pattern: %s)", rel_path, pattern
                )
                return True

    logger.debug("Not ignoring: %s", rel_path)
    return False


def get_file_tree(root_dir: str, ignored_patterns: List[str]) -> List[str]:
    """
    Generate a list of files in the directory tree, excluding ignored files and directories.

    Args:
        root_dir (str): The root directory to start the file tree from.
        ignored_patterns (List[str]): List of patterns to ignore.

    Returns:
        List[str]: A list of relative file paths.
    """
    file_tree = []
    for root, dirs, files in os.walk(root_dir):
        dirs[:] = [
            d
            for d in dirs
            if not should_ignore(os.path.join(root, d), ignored_patterns, root_dir)
        ]
        logger.debug("Directories after filtering: %s", dirs)

        for file in files:
            file_path = os.path.join(root, file)
            if not should_ignore(file_path, ignored_patterns, root_dir):
                rel_path = os.path.relpath(file_path, root_dir)
                file_tree.append(rel_path)
                logger.debug("Added file to tree: %s", rel_path)

    return file_tree


def read_file_contents(file_path: str) -> str:
    """
    Read and return the contents of a file.

    Args:
        file_path (str): The path to the file to be read.

    Returns:
        str: The contents of the file.
    """
    with open(file_path, "r", encoding="utf-8") as f:
        return f.read()


def is_binary_file(file_path: str) -> bool:
    """
    Check if a file is binary.

    Args:
        file_path (str): The path to the file to be checked.

    Returns:
        bool: True if the file is binary, False otherwise.
    """
    try:
        with open(file_path, "rb") as file:
            chunk = file.read(1024)
            return b"\0" in chunk
    except IOError:
        return False
