import os
import subprocess
import sys

# Define the remote destination (replace with actual destination)
REMOTE_USER = "akhil"
REMOTE_HOST = "132.1.16.138"
REMOTE_PATH = "/home/akhil/projects/platform"


def get_changed_files():
    """Get a list of changed files from `git status`."""
    result = subprocess.run(
        ["git", "status", "--porcelain"], capture_output=True, text=True
    )
    if result.returncode != 0:
        print("Error running git status")
        sys.exit(1)

    # Extract the file paths from the output
    changed_files = []
    for line in result.stdout.splitlines():
        # Each line starts with two status characters, followed by the file path
        status = line[:2].strip()
        file_path = line[3:]
        if status in {"M", "A", "R", "C", "T"}:  # M=modified, A=added, etc.
            changed_files.append(file_path)

    return changed_files


def rsync_files(files):
    """Sync the specified files to the remote server."""
    for file_path in files:
        # Ensure the file exists before syncing (important for moved/renamed files)
        if os.path.exists(file_path):
            destination = f"{REMOTE_USER}@{REMOTE_HOST}:{REMOTE_PATH}/{file_path}"
            # Create remote directory structure if needed
            remote_dir = os.path.dirname(destination)
            subprocess.run(
                ["ssh", f"{REMOTE_USER}@{REMOTE_HOST}", f"mkdir -p {remote_dir}"]
            )

            # Run rsync for the file
            result = subprocess.run(["rsync", "-avz", file_path, destination])
            if result.returncode == 0:
                print(f"Synced {file_path}")
            else:
                print(f"Failed to sync {file_path}")
        else:
            print(f"Skipped {file_path} (does not exist locally)")


def main():
    # Get the list of changed files
    changed_files = get_changed_files()
    if not changed_files:
        print("No changes to sync.")
        return

    # Sync the changed files
    rsync_files(changed_files)


if __name__ == "__main__":
    main()
