import os
import re



# Get all files
current_dir = os.getcwd()
files = os.listdir(current_dir)

# Filter out HPCG static 
HPCG_static_files = []
for file in files:
    if "HPCG-Benchmark" in file:
        HPCG_static_files.append(file)

def getNumber(str):
    return re.findall(r"-?\d+\.?\d*", str)[0]
def getAvg(list):
    return sum(list) / len(list)

DDOT = []
WAXPBY = []
SpMV = []
MG = []
total = []
Final = []

# Averange
for file in HPCG_static_files:
    with open(file, "r") as data_file:
        lines = data_file.readlines()
        line_DDOT_GFLOPS = lines[103]
        line_WAXPBY_GFLOPS = lines[104]
        line_SpMV_GFLOPS = lines[105]
        line_MG_GFLOPS = lines[106]
        line_total_GFLOPS = lines[107]
        line_Final_GFLOPS = lines[118]

        DDOT.append(float(getNumber(line_DDOT_GFLOPS)))
        WAXPBY.append(float(getNumber(line_WAXPBY_GFLOPS)))
        SpMV.append(float(getNumber(line_SpMV_GFLOPS)))
        MG.append(float(getNumber(line_MG_GFLOPS)))
        total.append(float(getNumber(line_total_GFLOPS)))
        Final.append(float(getNumber(line_Final_GFLOPS)))

print(DDOT)
print(WAXPBY)
print(SpMV)
print(MG)
print(total)
print(Final)

