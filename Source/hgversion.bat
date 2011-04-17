@echo off
if not exist hgversion.h hg tip --template "#define BUILD_VERSION {rev}\n" > %1hgversion.h