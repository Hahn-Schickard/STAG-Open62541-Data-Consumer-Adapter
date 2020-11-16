import subprocess
import sys
import os
import argparse


def is_installed(name: str):
    try:
        subprocess.run(
            [name, "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except subprocess.CalledProcessError:
        print("Program: ", name, " is not installed!")
        return False
    return True


def file_exists(name: str):
    if os.path.isfile(name):
        return True
    else:
        print("File: ", name, " can not be found!")
        return False


def log_output(lines: list, filename: str):
    if os.path.isfile(filename):
        os.remove(filename)
    file = open(filename, "w")
    for line in lines:
        print(line)
        file.write(line + '\n')
    file.close()


def count_errors(lines: list, error_beggining_marker: str, error_end_marker: str):
    error_marker_occurances = [i for i in lines if error_beggining_marker in i]
    error_counter_string = error_marker_occurances.pop()
    error_marker_index = error_counter_string.find(
        error_beggining_marker) + len(error_beggining_marker)
    error_marker_size = error_counter_string.find(error_end_marker)
    return int(error_counter_string[error_marker_index:error_marker_size])


def run_memory_analysis(analyzer: str, settings: list, error_beggining_marker: str, error_end_marker: str, target: str, arguments: list):
    if is_installed(analyzer) and file_exists(target):
        try:
            cmd_list = [analyzer]
            for setting in settings:
                cmd_list.append(setting)
            cmd_list.append(target)
            for argument in arguments:
                cmd_list.append(argument)
            output = subprocess.run(
                cmd_list, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
            result = output.stderr.decode('utf-8').split('\n')
            log_output(result, analyzer+"-results.log")
            error_count = count_errors(
                result, error_beggining_marker, error_end_marker)
            if error_count > 0:
                print(analyzer, " found ", error_count, " errors!")
                sys.exit(1)
        except subprocess.CalledProcessError:
            sys.exit(1)
    else:
        sys.exit(1)


parser = argparse.ArgumentParser()
parser.add_argument("target", help="full path to the binary target that will be analyzed",
                    type=str)
parser.add_argument('arguments', nargs='?', default=[],
                    help="add an argument to the list of arguments, that are used by the target binary")

target = parser.parse_args().target
arguments = parser.parse_args().arguments

print("Runing memory analysis with valgrind for target: ",
      target, " with argument list:", arguments)
run_memory_analysis("valgrind", ["--leak-check=full", "--show-leak-kinds=all",
                                 "--track-origins=yes", "--verbose"], "ERROR SUMMARY: ", "errors", target, arguments)
