#!/usr/bin/env python3
"""
Script to reconstruct directory structure and files from CodeCtx output.
"""
import os
import argparse
import logging
from typing import Tuple, List

def parse_codectx_file(file_path: str) -> Tuple[List[str], dict]:
    """Parse CodeCtx output file into directory tree and file contents."""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Split into directory tree and file contents sections
    tree_section = content.split('Directory Tree:\n')[1].split('File Contents:\n')[0]
    files_section = content.split('File Contents:\n')[1]

    # Parse directory tree
    files = [line.strip() for line in tree_section.strip().split('\n') if line.strip()]
    files = [f for f in files if f != '===============']

    # Parse file contents
    contents = {}
    current_file = None
    current_content = []
    
    for line in files_section.split('\n'):
        if line.startswith('--- ') and line.endswith(' ---'):
            if current_file:
                contents[current_file] = '\n'.join(current_content)
            current_file = line[4:-4]
            current_content = []
        else:
            current_content.append(line)
    
    if current_file:
        contents[current_file] = '\n'.join(current_content)

    return files, contents

def create_directory_structure(base_dir: str, files: List[str]) -> None:
    """Create the directory structure for the files."""
    for file_path in files:
        full_path = os.path.join(base_dir, file_path)
        directory = os.path.dirname(full_path)
        
        if directory and not os.path.exists(directory):
            os.makedirs(directory)

def write_files(base_dir: str, contents: dict) -> None:
    """Write the contents to their respective files."""
    for file_path, content in contents.items():
        full_path = os.path.join(base_dir, file_path)
        with open(full_path, 'w', encoding='utf-8') as f:
            f.write(content)

def main():
    parser = argparse.ArgumentParser(description='Reconstruct code from CodeCtx output')
    parser.add_argument('--input', required=True, help='CodeCtx output file')
    parser.add_argument('--output-dir', required=True, help='Directory to reconstruct code in')
    args = parser.parse_args()

    logging.basicConfig(level=logging.INFO)
    logger = logging.getLogger(__name__)

    logger.info(f"Parsing CodeCtx file: {args.input}")
    files, contents = parse_codectx_file(args.input)

    logger.info(f"Creating directory structure in: {args.output_dir}")
    create_directory_structure(args.output_dir, files)

    logger.info("Writing files")
    write_files(args.output_dir, contents)
    
    logger.info("Reconstruction complete")

if __name__ == '__main__':
    main()
