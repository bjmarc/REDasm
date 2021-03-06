import shutil
import os

os.chdir("../deploy")

if not os.path.exists("REDasm"):
 print("Skipping AppImage creation")
 exit(1)

shutil.rmtree("lib", ignore_errors=True)
os.mkdir("lib")
shutil.move("LibREDasm.so", "lib/")

shutil.copy("../artwork/logo_20190204.png", "./redasm.png")
shutil.copy("../ci/REDasm.desktop", "./")
