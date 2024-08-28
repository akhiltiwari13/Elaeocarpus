"""
Main module for CodeCtx.

This module contains the main function that orchestrates the code context
generation process.
"""

import argparse
import logging
import os
from datetime import datetime
from typing import List

from .config import IGNORED_PATTERNS
from .file_utils import get_file_tree
from .output_writer import write_output

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)


def setup_logging(log_file: str = None) -> None:
    """
    Set up logging to console and file.

    Args:
        log_file (str, optional): Path to the log file. If None, a default name is used.
    """
    if log_file is None:
        log_file = f"codectx_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log"

    # Create a logger
    logger = logging.getLogger()
    logger.setLevel(logging.DEBUG)

    # Create console handler and set level to debug
    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.DEBUG)

    # Create file handler and set level to debug
    file_handler = logging.FileHandler(log_file, encoding="utf-8")
    file_handler.setLevel(logging.DEBUG)

    # Create formatter
    formatter = logging.Formatter(
        "%(asctime)s - %(name)s - %(levelname)s - %(message)s"
    )

    # Add formatter to handlers
    console_handler.setFormatter(formatter)
    file_handler.setFormatter(formatter)

    # Add handlers to logger
    logger.addHandler(console_handler)
    logger.addHandler(file_handler)


def main():
    """
    Main function to run the CodeCtx tool.

    This function parses command-line arguments, processes the specified
    source directory, and generates the output file containing the code context.
    """
    parser = argparse.ArgumentParser(
        description="Generate source code context for LLM models."
    )
    parser.add_argument("--source", required=True, help="Path to the source directory")
    parser.add_argument("--output", required=True, help="Output filename")
    parser.add_argument("--log", required=False, help="Logfile name (optional)")
    args = parser.parse_args()

    setup_logging(args.log)
    # logger = logging.getLogger(__name__)

    src_path = os.path.abspath(args.source)
    output_file = args.output

    if not os.path.isdir(src_path):
        print(f"Error: {src_path} is not a valid directory.")
        return

    logger.debug("Processing directory: %s", src_path)
    file_tree = get_file_tree(src_path, IGNORED_PATTERNS)
    logger.info("Found %d files", len(file_tree))

    logger.info("Writing output to: %s", output_file)
    write_output(src_path, file_tree, output_file)
    logger.info("Finished writing output")


if __name__ == "__main__":
    main()
