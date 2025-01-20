"""
Configuration module for CodeCtx.

This module defines the patterns for files and directories to be ignored
during the code context generation process.
"""

from typing import List

IGNORED_PATTERNS: List[str] = [
    ".gitignore",
    ".git/",
    ".cache/",
    ".ccls/",
    "build/",
    ".vscode/",
    ".session/",
    "*.pyc",
    "__pycache__/",
    "*.log",
    "*.tmp",
    "venv/",
    "compile_commands.json",
    # bazel project related exclusions
    "bazel-*/",
    "external/",
    # specific to Arthur
    "third_party/",
    "Resources/",
    "LICENSE",
    ".gitmodules",
    "README.md",
    ".clang-format",
    ".cmake-format.yaml",
    "cmake/StaticAnalyzers.cmake",
    "cmake/PreventInSourceBuilds.cmake",
    "cmake/Cache.cmake",
    "cmake/Conan.cmake",
    "cmake/Sanitizers.cmake",
    "cmake/StandardProjectSettings.cmake",
    "cmake/Doxygen.cmake",
    "cmake/CompilerWarnings.cmake",
    # specific to platform
    "3rdparty",
]

# added for testing. just want the cmake layout
CONSIDERED_PATTERNS: List[str] = [
    "CMakeLists.txt",
]


def add_ignored_pattern(pattern):
    """
    Add a new pattern to the list of ignored patterns.

    Args:
        pattern (str): The pattern to be added to the ignore list.
    """
    IGNORED_PATTERNS.append(pattern)


def remove_ignored_pattern(pattern):
    """
    Remove a pattern from the list of ignored patterns if it exists.

    Args:
        pattern (str): The pattern to be removed from the ignore list.
    """
    if pattern in IGNORED_PATTERNS:
        IGNORED_PATTERNS.remove(pattern)
