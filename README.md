# Smart Home Log Conversion and Validation Tool

This project is a C-based smart home log processing tool developed for **CME2202 Assignment 1**.  
It converts smart home device logs between different file formats and validates XML output using an XSD schema.

## Overview

The program supports three main operations:

- Converting **CSV** log files into **binary**
- Converting **binary** log files into **XML**
- Validating **XML** files against an **XSD schema**

The system processes smart home device records containing information such as device ID, timestamp, temperature, humidity, battery level, firmware version, status, location, and event code.

## Project Files

- `smart-home-log-conversion.c` → main program
- `smart-home-log-conversion.xsd` → XML Schema Definition file
- `validate.c` → XML validation helper/example
- `setupParams.json` → sorting configuration
- `CME2202_Assignment1.pdf` → assignment description

## Features

### 1. CSV to Binary Conversion
The program:
- reads smart home device log entries from a CSV file
- removes non-ASCII characters from fields
- converts special status symbols into text values
- stores cleaned records in binary format

Status conversions:
- `⚠` → `WARN`
- `✓` → `OK`
- `❌` → `BAD`

If a status value is empty, it is set to:
- `UNKNOWN`

If an alert level is empty, it is set to:
- `NONE`

---

### 2. Binary to XML Conversion
The program:
- reads binary records
- sorts them using parameters from `setupParams.json`
- generates structured XML output
- includes decimal and hexadecimal representations of `event_code`

The XML contains:
- decimal event code
- big-endian hexadecimal event code
- little-endian hexadecimal event code

---

### 3. XML Validation
The program validates the generated XML file using the provided XSD schema file.

## Data Fields

Each log record contains the following fields:

- `device_id`
- `timestamp`
- `temperature`
- `humidity`
- `status`
- `location`
- `alert_level`
- `battery`
- `firmware_ver`
- `event_code`

## Requirements

To compile and run this project, you need:

- `gcc`
- `libxml2`

## Compilation

### Compile the main program

```bash
gcc smart-home-log-conversion.c -o deviceTool $(xml2-config --cflags --libs)
Compile the validation helper
gcc validate.c -o validate $(xml2-config --cflags --libs)
Usage

Run the main program with:

./deviceTool <input_file> <output_file> -separator <1|2|3> -opsys <1|2|3>
Separator Options
1 = comma ,
2 = tab
3 = semicolon ;
Operating System Options
1 = Windows
2 = Linux
3 = macOS
Program Menu

After running the program, the following menu appears:

1 Convert CSV to Binary
2 Convert Binary to XML
3 Validate XML with XSD
4 Exit
Example Usage
Convert CSV to Binary
./deviceTool logdata.csv logdata.dat -separator 1 -opsys 2

Then choose:

1
Convert Binary to XML
./deviceTool logdata.dat logdata.xml -separator 1 -opsys 2

Then choose:

2
Validate XML with XSD
./deviceTool logdata.xml smart-home-log-conversion.xsd -separator 1 -opsys 2

Then choose:

3
Sorting Configuration

Sorting behavior is controlled by setupParams.json.

Example:

{
  "dataFileName": "logdata.dat",
  "keyStart": 0,
  "keyEnd": 7,
  "order": "ASC"
}

This file determines:

the data file to process
the substring range used from device_id
sorting order
XML Validation Rules

The XML schema enforces the following constraints:

device_id must follow the format: AAA1234
location maximum length: 30 characters
firmware_ver must follow format: vX.Y.Z
temperature must be between -30.0 and 60.0
humidity must be between 0 and 100
battery must be between 0 and 100
status must be one of:
OK
BAD
WARN
UNKNOWN
alert_level must be one of:
LOW
MEDIUM
HIGH
CRITICAL
NONE
Notes
Non-ASCII characters are removed during preprocessing.
Empty status values are replaced with UNKNOWN.
Empty alert levels are replaced with NONE.
XML output includes both big-endian and little-endian hexadecimal values for event_code.
