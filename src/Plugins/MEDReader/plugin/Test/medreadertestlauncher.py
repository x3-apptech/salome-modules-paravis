import subprocess as sp
import re,sys

def LaunchMEDReaderTest(fn,baseLineDir):
    """
    Tuleap 18933 : wrapper over MEDReader tests. Waiting for better solution.
    """
    p = sp.Popen(["python3",fn,"-B",baseLineDir],stdout=sp.PIPE,stderr=sp.PIPE)
    a,b = p.communicate()
    if p.returncode == 0:
        return

    stringToAnalyze = a.decode("utf-8")
    linesOfStringToAnalyze = stringToAnalyze.split('\n')
    pat = re.compile("<DartMeasurement name=\"ImageError\" type=\"numeric/double\">[\s+]([\S]+)[\s]*</DartMeasurement>")
    zeLine = [elt for elt in linesOfStringToAnalyze if pat.match(elt)]
    if len(zeLine) != 1:
        raise RuntimeError("Error for test {}".format(fn))
    delta = float(pat.match(zeLine[0]).group(1))
    if delta > 1:
        raise RuntimeError("Image comparison failed : {} > 1".format(delta))

def LaunchAllTests(baseLineDir):
    from glob import glob
    fis = glob("testMEDReader*.py")
    for fi in fis:
        print(fi)
        LaunchMEDReaderTest(fi,baseLineDir)

if __name__ == "__main__":
    LaunchMEDReaderTest(sys.argv[1],sys.argv[2])

