"""
script to set the project up as pip installable pkg.
"""

# setup.py
from setuptools import find_packages, setup

with open("README.md", "r", encoding="utf-8") as readme_file:
    long_description = readme_file.read()

setup(
    name="codectx",
    version="0.7",
    packages=find_packages(),
    entry_points={
        "console_scripts": [
            "codectx=codectx.main:main",
        ],
    },
    author="akhil",
    # author_email="akhiltiwari.13@gmail.com",
    description="A tool to generate source code context for LLM models",
    long_description=long_description,
    long_description_content_type="text/markdown",
    # url="https://github.com/akhiltiwari13/source-context",
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    python_requires=">=3.6",
)
