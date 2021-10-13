black .
if errorlevel 1 goto :eof

clang-format -i -style=file *.cc
if errorlevel 1 goto :eof

python fmt_c.py .
if errorlevel 1 goto :eof

git diff
