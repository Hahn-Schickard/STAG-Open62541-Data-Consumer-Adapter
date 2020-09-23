import subprocess
import sys
import re


def is_installed(name: str):
    try:
        subprocess.run(
            [name, "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except subprocess.CalledProcessError:
        print("Program: ", name, " is not installed!")
        return False
    return True


def getVersionFromTag(exact_match=True):
    if is_installed("git"):
        git_cmd = ["git", "describe", "--abbrev=0"]
        if exact_match:
            git_cmd.append("--exact-match")
        output = subprocess.run(
            git_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        tag = output.stdout.decode('utf-8').strip("\n")
        if output.returncode != 0:
            raise subprocess.CalledProcessError(
                output.returncode, git_cmd, stderr=output.stderr.decode('utf-8'))
        return re.sub('([A-Z]*?[^0-9.])', '', tag)
    else:
        raise OSError("File not found")


if __name__ == "__main__":
    try:
        print(getVersionFromTag(False))
    except Exception:
        sys.exit(1)
