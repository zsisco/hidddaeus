# `hidddaeus`
A **H**uman **I**nterface **D**evice **D**aemon for **D**etecting **A**nomalous **E**xploits in **U**ser **S**pace

`hidddaeus` is written by Zachary Sisco (2016), and it uses code adapted from HID Listen and Raw HID I/O Routines by PJRC.COM, LLC and is released under the GNU General Public License version 3. 

## Overview
A vulnerability in the USB software stack allows a device to register as an arbitrary number of interfaces. Attackers exploit this vulnerability by masquerading a malicious USB device as a Human Interface Device (HID). Through this, the USB device emulates the functionality of a keyboard to inject malicious scripts stored in its firmware onto the host. 

`hidddaeus` presents a solution that leverages pattern recognition techniques to detect anomalous and potentially malicious HID activity on a Linux-based host. This is a new approach to HID-based attack detection and improves upon previous solutions by completely automating detection. In initial tests, `hidddaeus` achieves a mean detection accuracy of 89.3%. 

Please read [the report](report_using-pattern-recognition-to-detect-attacks-from-HIDs.pdf) for an in-depth overview of the system, the threat model, and the experimental results and analysis. 

### Usage
Set configuration (e.g. input file) in: `hidddaeus.h`

Build `hidddaeus` with: `make all`

