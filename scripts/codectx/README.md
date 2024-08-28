# codectx

## Overview

codectx is a command-line tool designed to create a comprehensive context of a source code directory for Large Language Models (LLMs). It generates a single output file containing the directory structure and the contents of all relevant files, making it easier to provide context to LLMs for code-related queries.

## Features

- Generates a directory tree of the specified source code directory
- Copies the content of each file into a single output file
- Configurable ignore patterns to exclude unnecessary files and directories
- Relative path reporting for easy navigation
- Error handling for file reading issues

## Installation

### Prerequisites

- Python 3.6 or higher
- pip (Python package installer)

### Installing from PyPI

```bash
pip install codectx
```

### Installing from source

1. Clone the repository:

   ```bash
   git clone https://github.com/yourusername/source-context.git
   cd source-context
   ```

2. Install the package:
   ```bash
   pip install -e .
   ```

## Usage

After installation, you can use the `codectx` command from anywhere in your terminal:

```bash
codectx --src /path/to/source/directory --output output.txt
```

### Arguments

- `--src`: The path to the source code directory you want to analyze (required)
- `--output`: The filename where you want to save the output (required)

## Configuration

The tool comes with a default set of ignore patterns for common files and directories that are typically not needed in code analysis. These are defined in the `config.py` file.

Default ignored patterns:

- `.gitignore`
- `.git`
- `.cache`
- `build`
- `.vscode`
- `.session`
- `*.pyc`
- `__pycache__`
- `*.log`
- `*.tmp`

To modify the ignore patterns, you can edit the `IGNORED_PATTERNS` list in `config.py`.

## Output Format

The generated output file will have the following structure:

1. Directory Tree: A list of all files and directories in the source directory (excluding ignored items).
2. File Contents: The content of each file, preceded by its relative path.

Example:

```
Directory Tree:
===============
src/main.py
src/utils/helper.py
tests/test_main.py

File Contents:
==============

--- src/main.py ---
[Content of main.py]

--- src/utils/helper.py ---
[Content of helper.py]

--- tests/test_main.py ---
[Content of test_main.py]
```

## Development

To contribute to this project:

1. Fork the repository
2. Create a new branch for your feature
3. Implement your feature or bug fix
4. Add or update tests as necessary
5. Submit a pull request

## Troubleshooting

If you encounter any issues:

1. Ensure you're using Python 3.6 or higher
2. Check that you have the necessary permissions to read the source directory and write the output file
3. Verify that the source directory path is correct
4. If certain files are unexpectedly ignored, check the `IGNORED_PATTERNS` in `config.py`

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contact

If you have any questions, feel free to reach out to [Your Name] at [your.email@example.com].

## Acknowledgments

- Thanks to all contributors who have helped to improve this tool.
- Inspired by the need to provide comprehensive code context to LLMs.

```

This README provides:

1. An overview of the tool
2. Installation instructions
3. Usage guide with examples
4. Configuration details
5. Output format explanation
6. Development guidelines
7. Troubleshooting tips
8. License information
9. Contact details
10. Acknowledgments

You can further customize this README to include any specific details about your implementation or additional features you might add in the future.
```
