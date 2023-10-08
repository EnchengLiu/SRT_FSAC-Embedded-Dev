File:    CANdb++/Timing Analysis/Readme.txt
Content: Explanation of the files in the Timing Analysis folder
Author:  Thomas R. Schmidt, Vector Informatik GmbH
Date:    2002-10-10
Changes:

1. Introduction

CANdb++ Version 2.5 (Admin only) includes a Timing-Analysis on
networks. The results can be written to a CSV (comma separated values)
file and processed in Excel.

Some standard formats are provided in a template document, included in
this directory.


2. Content of directory
   Readme.txt            this file
   TimingAnalysis.XLT    Excel template document for the
                         interpretation of generated results

3. Usage

After the Timing Analysis is completed, store the results to a file
(see button in the result dialog), e.g. "results.csv".

Load the TimingAnalysis.XLT in Excel (e.g. double-click on the file in
the Windows Explorer), activating the included macros. Then activate
the "Load_ResultsCSV_and_update_charts" macro.

If this does not work, load results.csv in another sheet, select it
completely and paste it in the empty "Timing Analysis" sheet in the
first document. Then activate the macro "NameRanges".

Now the diagrams should be available:
"Latencies per Message" displays the absolute minimum and maximum
latency times of the considered messages.

"Latency Distribution" shows the message latencies in relation to
their maximum latency, i.e. a value of 100 % or more will indicate a
violation of this value.

"PriorityInversion" will display the number and severity of observed
priority inversions, or be empty if none were registered.


4. Hints

In order to use the template, some windows settings may have to be
changed:

- The value format has to be set to use the dot "." as a decimal
point, not the comma "," like e.g. the German default
setting. Otherwise many numbers may not be interpreted correctly!

