@echo off
for %%m in (MSDevDir MSVCDir Include Lib) do if not defined %%m call "..\..\..\regpaths.cmd"
