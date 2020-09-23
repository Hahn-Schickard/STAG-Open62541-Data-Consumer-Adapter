import subprocess
import sys
import os
import shutil
import argparse


def is_installed(name: str):
    try:
        subprocess.run(
            [name, "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except subprocess.CalledProcessError:
        print("Program: ", name, " is not installed!")
        return False
    return True


def read_ignore_list(filename: str):
    if os.path.isfile(filename):
        file = open(filename, "r")
        ignore_list = file.readlines()
        ignore_list = [ignore_pattern.strip()
                       for ignore_pattern in ignore_list]
        file.close
        return ignore_list


def generate_coverage_report():
    if is_installed("lcov"):
        try:
            workspace_name = "code_coverage_report"
            if os.path.exists(workspace_name):
                shutil.rmtree(workspace_name, ignore_errors=True)
            os.mkdir(workspace_name)
            output_file = "code_coverage.info"
            if os.path.isfile(output_file):
                os.remove(output_file)
            subprocess.run(["lcov", "--directory", "build/", "--capture",
                            "--output-file", output_file, "-rc", "lcov_branch_coverage=1"])
            current_file_loc = os.path.dirname(os.path.realpath(__file__))
            ignore_list = read_ignore_list(
                current_file_loc + "/Coverage_Ignores.txt")
            for ignore_pattern in ignore_list:
                print("Ignoring filenaems that fit: ",
                      ignore_pattern, "pattern!")
                subprocess.run(["lcov", "--remove", output_file,
                                ignore_pattern, "-o", output_file])
            subprocess.run(["lcov", "--list", output_file])
            subprocess.run(
                ["genhtml", output_file, "--branch-coverage", "--output-directory", workspace_name + "/"])
        except subprocess.CalledProcessError:
            sys.exit(1)
    else:
        sys.exit(1)


def run_target(runnable: bool, target: str, arguments: list):
    if runnable:
        try:
            cmd_list = [target]
            for argument in arguments:
                cmd_list.append(argument)
            subprocess.run(
                cmd_list, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except subprocess.CalledProcessError:
            sys.exit(1)


def to_bool(value):
    valid = {'true': True, 't': True, '1': True,
             'false': False, 'f': False, '0': False,
             }

    if isinstance(value, bool):
        return value

    if not isinstance(value, str):
        raise ValueError('invalid literal for boolean. Not a string.')

    lower_value = value.lower()
    if lower_value in valid:
        return valid[lower_value]
    else:
        raise ValueError('invalid literal for boolean: "%s"' % value)


parser = argparse.ArgumentParser()
parser.add_argument("runnable", help="set to True if a geven target is runnable",
                    type=str)
parser.add_argument("target", help="full path to the binary target that will be analyzed",
                    type=str)
parser.add_argument('arguments', nargs='?', default=[],
                    help="add an argument to the list of arguments, that are used by the target binary")

runnable = to_bool(parser.parse_args().runnable)
target = parser.parse_args().target
arguments = parser.parse_args().arguments

run_target(runnable, target, arguments)
generate_coverage_report()
