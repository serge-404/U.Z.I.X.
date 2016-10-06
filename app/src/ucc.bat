@rem
@rem Batch file to compile an UZIX application under MSDOS
@rem (c) 2001 A&L Software
@rem
@rem This program is under GPL License.
@rem

@echo off
if "%1" == "" goto error
set CNAME=%~n1
shift
if not exist %CNAME%.c goto nofile

echo.
echo Compiling %CNAME%.c for UZIX
echo.
echo Preprocessing...
cpm -h cpp -DMSX_UZIX_TARGET -DHI_TECH_C -Dz80 -I %CNAME%.c $C1.T
echo Pass 1...
cpm -h p1 $C1.T $C2.T $C3.T
echo Generating C code...
cpm -h cgen $C2.T $C1.T

if "%1" == "-o" goto optim
echo Generating assembly code...
cpm -h zas -N -o%CNAME%.O $C1.T

:compile
if exist $c1.t del $c1.t > nul
if exist $c2.t del $c2.t > nul
if exist $c3.t del $c3.t > nul
echo Linking...
cpm -h link -Z -Ptext=256,data,bss,bssend -C256 -O%CNAME% C0U.O %CNAME%.O %1 %2 %3 %4 %5 %6 %7 %8 %9 LIBC-UZI.LIB
rem del %CNAME%.O > nul
echo Done.
goto end

:optim
echo Optimizing...
cpm -h optim $C1.T $C2.T
echo Generating assembly code...
cpm -h zas -J -X -N -o%CNAME%.O $C2.T
shift
goto compile

:nofile
echo Source file does not exist.
goto end

:error
echo usage: ucc sourcefile [-o] [library1] [library2...]
echo        source filename must be supplied without extension.
echo        option -o disables code optimization.
echo        library1, library2, etc are other libraries to link

:end

