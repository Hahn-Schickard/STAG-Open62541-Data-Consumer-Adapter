import os
import sys
import shutil
import markdown
import argparse
import fileinput
import re
import getProjectName
import getVersionFromTag

parser = argparse.ArgumentParser()
parser.add_argument("homepage", help="project homepage",
                    type=str)

homepage = parser.parse_args().homepage

root_dir_loc = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..'))
project_name = getProjectName.getProjectName(root_dir_loc)
project_version = getVersionFromTag.getVersionFromTag(True)
workspace_name = "public"
workspace_loc = os.path.join(root_dir_loc, workspace_name)
if os.path.exists(workspace_name):
    shutil.rmtree(workspace_name, ignore_errors=True)
amalgam_template_loc = os.path.join(root_dir_loc, 'docs')
shutil.copytree(amalgam_template_loc, workspace_loc)
shutil.move(os.path.join(workspace_loc, 'code_documentation', 'html'),
            os.path.join(workspace_loc, "documentation"))
shutil.rmtree(os.path.join(
    workspace_loc, 'code_documentation'), ignore_errors=True)
code_coverage_loc = os.path.join(root_dir_loc, 'code_coverage_report')
shutil.copytree(code_coverage_loc, os.path.join(
    workspace_loc, 'code_coverage'))
with open(os.path.join(root_dir_loc, 'README.md'), "r", encoding="utf-8") as input_file:
    text = input_file.read()
html = markdown.markdown(text)
with open(os.path.join(workspace_loc, 'about.html'), "w", encoding="utf-8") as output_file:
    output_file.writelines(html)
with fileinput.FileInput(os.path.join(workspace_loc, 'index.html'), inplace=True) as file:
    for line in file:
        print(line.replace("PROJECT_WEBSITE", homepage), end='')
with fileinput.FileInput(os.path.join(workspace_loc, 'index.html'), inplace=True) as file:
    for line in file:
        print(line.replace("PROJECT_NAME", project_name), end='')
with fileinput.FileInput(os.path.join(workspace_loc, 'index.html'), inplace=True) as file:
    for line in file:
        print(line.replace("PROJECT_VERSION", project_version), end='')
fp = open(
    os.path.join(workspace_loc, 'code_coverage', 'index.html'))
for i, line in enumerate(fp):
    if i == 44:
        coverage_percent = line.strip()
fp.close()
coverage_percent = re.sub('<.*?>', '', coverage_percent)
coverage_percent = re.sub(r'([.]+[\s\S]*)', '', coverage_percent)
with fileinput.FileInput(os.path.join(workspace_loc, 'index.html'), inplace=True) as file:
    for line in file:
        print(line.replace("PERCENT", coverage_percent), end='')
coverage_colour = "red"
if int(coverage_percent) > 40:
    coverage_colour = "orange"
if int(coverage_percent) > 60:
    coverage_colour = "yellow"
if int(coverage_percent) > 80:
    coverage_colour = "green"
with fileinput.FileInput(os.path.join(workspace_loc, 'index.html'), inplace=True) as file:
    for line in file:
        print(line.replace("COLOUR", coverage_colour), end='')
with open(os.path.join(workspace_loc, 'license.html'), 'w', encoding="utf-8") as output_file:
    license_loc = os.path.join(root_dir_loc, 'LICENSE')
    if os.path.isfile(license_loc):
        license = markdown.markdown(
            open(license_loc, "r", encoding="utf-8").read())
        output_file.writelines(license)
    else:
        output_file.writelines(markdown.markdown("No license"))
    notice_loc = os.path.join(root_dir_loc, 'NOTICE')
    if os.path.isfile(notice_loc):
        notice = markdown.markdown(
            open(notice_loc, "r", encoding="utf-8").read())
        output_file.writelines(notice)
