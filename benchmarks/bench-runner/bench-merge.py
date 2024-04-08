#!/usr/bin/env python3
# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

import argparse
import json

import output
import stats

def printMerged(jsonObjs):
    benchmarks = list()
    # We only do the benchmarks done by the first runner (the "reference").
    for result in jsonObjs[0][0]["results"]:
        benchmarks.append(result)
    benchmarks.sort()
    
    nColumns = 4
    row_fmt = output.AsciiTableFormatter.strFmt * nColumns
    print(row_fmt
          .format(*["runner", "benchmark", "Total time", "ratio"],
                  width=output.AsciiTableFormatter.colWidth))
    print("=" * nColumns * output.AsciiTableFormatter.colWidth)

    firstJsonObjEncoded = jsonObjs[0]
    firstJsonObj = firstJsonObjEncoded[0]
    firstRunnerName = firstJsonObj["runtime"]
    firstRunnerResults = firstJsonObj["results"]
    # dicts matching runtime names (for runtimes other than firstRunnerName)
    # to the sequence of ratios for the runtime wrt the firstRunner
    runtimeToRatioSeq = dict()
    for benchmark in benchmarks:
        firstRunnerMean = firstRunnerResults[benchmark]["totalTime"]["mean"]
        n = 0
        for jsonObjDec in jsonObjs:
            jsonObj = jsonObjDec[0]
            runtime = jsonObj["runtime"]
            if not runtime in runtimeToRatioSeq:
                runtimeToRatioSeq[runtime] = []
            results = jsonObj["results"]
            if not benchmark in results:
                continue
            bmResults = results[benchmark]
            bmMean = bmResults["totalTime"]["mean"]
            ratio = bmMean / firstRunnerMean
            runtimeToRatioSeq[runtime].append(ratio)
            metric = stats.Metric(bmResults["totalTime"]["samples"])
            print(row_fmt.format(runtime, benchmark,
                                 output.secondsAndBounds(metric),
                                 "{:7.3f}".format(ratio),
                                 width=output.AsciiTableFormatter.colWidth))
        print("-" * nColumns * output.AsciiTableFormatter.colWidth)

    for jsonObjDec in jsonObjs:
        jsonObj = jsonObjDec[0]
        runtime = jsonObj["runtime"]
        prod = 1
        n = 0;
        seq = runtimeToRatioSeq[runtime]
        for ratio in seq:
            prod *= ratio
        print(row_fmt.format(runtime, "geomean", "",
                             "{:7.3f}".format(prod ** (1/len(seq))),
                             width=output.AsciiTableFormatter.colWidth))


def main():
    # Support for command line arguments
    argparser = argparse.ArgumentParser(description="Runs gc benchmarks")
    argparser.add_argument(
        "jsonFileNames",
        nargs="*",
        help="JSON files to merge",
        action="store",
    )
    args = argparser.parse_args()
    
    jsonObjs = []
    for jsonFileName in args.jsonFileNames:
        try:
            with open(jsonFileName, 'r') as jsonFile:
                jsonObjs.append(json.JSONDecoder().raw_decode(jsonFile.read()))
        except json.JSONDecodeError as e:
            print("file " + jsonfile + " could not be parsed.  Contents:")
            print(e)
            return
    printMerged(jsonObjs)


if __name__ == "__main__":
    main()
