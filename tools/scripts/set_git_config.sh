#!/bin/bash

git config --global user.name "akhiltiwari13"
git config --global user.email "akhil_blr@algoquantfin.com"
git config --global core.editor "nvim"
git config --global filter.lfs.clean "git-lfs clean -- %f"
git config --global filter.lfs.smudge "git-lfs smudge -- %f"
git config --global filter.lfs.process "git-lfs filter-process"
git config --global filter.lfs.required true

echo "Global Git configurations have been set."
