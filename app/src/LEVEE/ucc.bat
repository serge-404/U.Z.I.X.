@ECHO OFF
@rem
@rem Batch file to compile an UZIX application under MSDOS
@rem (c) 2001 A&L Software
@rem
@rem This program is under GPL License.
@rem

set CMODE=""
set OMODE=""
set CSTR=executable
set OSTR=no optimization

:params

if "%1" == "-c" goto compil
if "%1" == "-o" goto optimz

if "%1" == "" goto error
set CNAME=%~n1
shift
if not exist %CNAME%.c goto nofile

echo.
echo Compiling %CNAME%.c for UZIX (%CSTR%, %OSTR%)
echo.
echo Preprocessing...
cpm -h cpp -DMSX_UZIX_TARGET -DHI_TECH_C -DORI_UZIX -Dz80 -I %CNAME%.c $C1.T
echo Pass 1...
cpm -h p1 $C1.T $C2.T $C3.T
echo Generating C code...
cpm -h cgen $C2.T $C1.T

if %OMODE% == "o" (goto optim) else (goto usual)

:compile
if exist $c1.t del $c1.t > nul
if exist $c2.t del $c2.t > nul
if exist $c3.t del $c3.t > nul
if %CMODE% == "c" goto skiplink
echo Linking...
cpm -h link -Z -Ptext=256,data,bss,bssend -C256 -O%CNAME% C0U.O %CNAME%.O %1 %2 %3 %4 %5 %6 %7 %8 %9 LIBC-UZI.LIB
rem del %CNAME%.O > nul
:done
echo Done.
echo. 
goto end

:skiplink
if "%1" == "" goto done
echo Put %CNAME%.O to library %1
cpm libr r %1 %CNAME%.O
del %CNAME%.O
goto done

:usual
echo Generating assembly code...
cpm -h zas -N -o%CNAME%.O $C1.T
goto compile

:optim
echo Optimizing...
cpm -h optim $C1.T $C2.T
echo Generating assembly code...
cpm -h zas -J -X -N -o%CNAME%.O $C2.T
goto compile

:optimz
set OSTR=with optimization
set OMODE="o"
shift
goto params

:compil
set CSTR=object file
set CMODE="c"
shift
goto params

:nofile
echo Source file does not exist.
goto end

:error
echo.
echo usage: ucc sourcefile [-o] [-c] [library1] [library2...]
echo        source filename must be supplied without extension.
echo        option -o enables code optimization.
echo        option -c produces object code and put it into library (if specified).
echo        library1, library2, etc are other libraries to link or put to
echo examples:
echo  ucc hello               # compile hello executable without optimizing
echo  ucc -c -o cmp libls.lib # compile cmp.c with optimization and put to libls.lib
echo  ucc -o -c ls libls.lib  # compile ls.c executable and link it against libls.lib

:end

