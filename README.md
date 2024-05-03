# HPC Script Helper

HPC Script Helper is a tool to generate job files using templates for use in a high performance computer (HPC). The goal of the tool is to speed up the process of creating job files to submit to a HPC. Currently the HPC Script Helper is only available on Windows.

## Installation:

To install the HPC Script Helper, clone the github repository or download a zip file of it. All that is required to run the helper, is bundled inside of the /executeable/ folder which contains the .exe file. This folder can be placed anywhere to the user's preferences with the tool still being able to run.

## Usage:

The HPC Script Helper takes in templates for different styles of job files. This lets the user customise what data they want to input for a given template. The templates are .txt text files with tags inside to represent where the user would want data to be injected. A tag is opened with {% and closed with %}. Anything in-between the tag open and close will be used as field names for data input. For example:

'{% Number of cores %}'

This tag will show up as the following input box for the user to input data:

'Number of core: xxxxxxxxxxx'

Note that the tool will remove any spaces inbetween the tag open and the first character. This does not affect data injection; it is instead an asethic choice to create readable field titles. After creating a template job script the user selects it from the starting page and then moves onto the setup page. The setup page will generate fields for every tag found within the template to allow the user to input data. After filling out every field the user can either save the data in a .csv format or to generate a job file. The job file dialog does not presuppose any specific file formats so it is the users responsibility to save the file in the appropriate format for the scheduling system being used.

## Open Source

This program is fully open source with the license file being supplied inside the main directory under "COPYING.txt". This program was built using Qt community which is available at https://www.qt.io/download.
