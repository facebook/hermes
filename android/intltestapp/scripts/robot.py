import subprocess
import time
import os
from datetime import datetime
import re

DEVICE_ID=""
ADB_PATH="/home/hermes/Android/Sdk/platform-tools/adb"

APP_ID="com.facebook.hermes.intltest.intl"
MAINACTIVITY="com.facebook.hermes.intltest.MainActivity"

totalPSSPattern = re.compile("^\\s*TOTAL:\\s*(\\d+).*")
javaHeapPattern = re.compile("^\\s*Java Heap:\\s*(\\d+).*")
nativeHeapPattern = re.compile("^\\s*Native Heap:\\s*(\\d+).*")
privateOtherPattern = re.compile("^\\s*Private Other:\\s*(\\d+).*")

totalPSSList = []
javaHeapList = []
nativeHeapList = []
privateOtherList = []

def getAdbCmd(subCmds):
    adbCmd = [ADB_PATH]
    if(len(DEVICE_ID) > 0):
        adbCmd.extend(["-s", DEVICE_ID])

    adbCmd.extend(subCmds)
    return adbCmd

def execute(cmd, output_dump):
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    for line in p.stdout.readlines():
        output_dump.write(line.decode("utf-8"))

def settle():
    time.sleep(2)

def settleLong():
    time.sleep(10)

def stopApp(output_dump):
    execute(getAdbCmd(["shell", "am", "force-stop", APP_ID]), output_dump)  

def startApp(output_dump):
    execute(getAdbCmd(["shell", "am", "start", "-n", APP_ID + "/" + MAINACTIVITY]), output_dump)

# adb shell am broadcast -a com.facebook.hermes.intltest.eval --es "script" "var\ x=\'abcd\'\;x;"
def runREPLCommand(command, output_dump):
    execute(getAdbCmd(["shell", "am", "broadcast", "-a", "com.facebook.hermes.intltest.eval", "--es", "script", command]), output_dump)

def prepareSummary(lines, output_summary):
    
    for line in lines:
        match = totalPSSPattern.match(line)
        if(match):
            totalPSSList.append(match.group(1))

        match = javaHeapPattern.match(line)
        if(match):
            javaHeapList.append(match.group(1))

        match = nativeHeapPattern.match(line)
        if(match):
            nativeHeapList.append(match.group(1))

        match = privateOtherPattern.match(line)
        if(match):
            privateOtherList.append(match.group(1))
            

def dumpSys(output_dump, output_summary):
    p = subprocess.Popen(getAdbCmd(["shell", "dumpsys", "meminfo", "-a", APP_ID]), stdout=subprocess.PIPE)
    lines = []
    for line in p.stdout.readlines():
        lines.append(line.decode("utf-8"))

    for line in lines:
        output_dump.write(line)

    prepareSummary(lines, output_summary)

def writeSummary(output_summary):
    for pss in totalPSSList:
        output_summary.write("Total PSS \t " + pss + "\n")

    output_summary.write("\n")

    for javaHeap in javaHeapList:
        output_summary.write("Java Heap \t " + javaHeap + "\n")

    output_summary.write("\n")

    for nativeHeap in nativeHeapList:
        output_summary.write("Native Heap \t " + nativeHeap + "\n")

    output_summary.write("\n")

    for privateOther in privateOtherList:
        output_summary.write("Private Other \t " + privateOther + "\n")

    output_summary.write("\n")

def run(output_dump, output_summary):
    output_dump.write("Starting.\n\n")
    output_summary.write("Starting.\n\n")
        
    stopApp(output_dump)
    settle()

    startApp(output_dump)
    settle()
    settle()

    # Initial state
    dumpSys(output_dump, output_summary)

    for n in range(100):
        runREPLCommand("Intl.Collator\(\)", output_dump)
        settle()
        dumpSys(output_dump, output_summary)

    runREPLCommand("hm:collect", output_dump)
    dumpSys(output_dump, output_summary)
    runREPLCommand("dr:collect", output_dump)
    dumpSys(output_dump, output_summary)
    runREPLCommand("dr:collect", output_dump)
    dumpSys(output_dump, output_summary)
    settle()
    dumpSys(output_dump, output_summary)
    settle()
    dumpSys(output_dump, output_summary)
    settle()
    dumpSys(output_dump, output_summary)

    writeSummary(output_summary)    

def main():
    scriptFolder = os.path.dirname(os.path.realpath(__file__))
    perfFolderPath = os.path.join(scriptFolder, "PerfFiles-" + datetime.now().strftime("%m_%d_%H_%M_%S"))
    os.mkdir(perfFolderPath)

    with open(os.path.join(perfFolderPath, 'dump.txt'), 'w') as output_dump:
        with open(os.path.join(perfFolderPath, 'summary.txt'), 'w') as output_summary:
            run(output_dump, output_summary)


# ADB_PATH="e:\\nugetcache\\androidsdk.29.0.1\\platform-tools\\adb.exe"
# with open('output.txt', 'w') as output_dump:
# subprocess.call([ADB_PATH, "shell", "am", "start", "-n", "com.fluidhelloworld/.MultiActivity"])

# for n in range(5):
#     time.sleep(5)
#     subprocess.call([ADB_PATH, "shell", "am", "broadcast", "-a", "com.fluidhelloworld.NEW_DICE", "--es", "mode", "RN_REUSE"])

if __name__ == "__main__":
    main()