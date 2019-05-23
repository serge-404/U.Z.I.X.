```
/////////////////////////////////////////////////////////////////////////
//                                                                     //
//   Orion/Z (Orion-128 + Z80-CARD-II) emulator, version 1.08          //
//                                                                     //
//   Addon: TotalCommander archiver (WCX) plugin for serving ODI files //
//          (Orion Disk Image files). Allow copy/extract CP/M files    //
//          to/from ODI file "diskette" such simple as processing any  //
//          archives in TotalCommander interface.    Version 1.05      //
//                                                                     //
//   How to install this plugin (32 bit only) in TotalCommander:       //
//          1. Unzip odi.wcx, system.bin to any directory              //
//                (usually c:\wincmd\Plugins)                          //
//          2. In Windows Commander 5.5 (or newer), choose             //
//                Configuration - Options                              //
//          3. Open the 'Packer' page                                  //
//          4. Click 'Configure packer extension DLLs'                 //
//          5. type the "ODI" extension                                //
//          6. Click 'new type', and select the  odi.wcx               //
//          7. Click OK.  Click OK                                     //
//          8. Repeat steps 2-7 for other supported (specified in      //
//               ODI.INI) formats - such as TRD, DSK, etc.             //
//                                                                     //
//   How to install this plugin (32 bit only) in Far Manager:          //
//          0. Install wcx.dll (plugin allowing TC WCX-plugins usage   //
//             in Far) to                                              //
//                   {FAR_DIR}\Plugins\Multiarc\Formats\WCX\           //
//             Install wcx.fmt to                                      //
//                   {FAR_DIR}\Plugins\Multiarc\Formats\               //
//          1. Unzip odi.wcx, system.bin to                            //
//                   {FAR_DIR}\Plugins\Multiarc\Formats\WCX\           //
//          1.2. To Support any other CP/M format (specified in INI),  //
//               just copy ODI.WCX, ODI.INI to files with              //
//               file name correcponding to format extension (for      //
//               example:  DSK.WCX, DSK.INI ;  TRD.WCX, TRD.INI) to    //
//                   {FAR_DIR}\Plugins\Multiarc\Formats\WCX\           //
//          2. Restart Far                                             //
//                                                                     //
//                                                                     //
//   Installation process for ohi.wcx (HDD images plugin) is the same  //
//          (except replacing 'ODI' -> 'OHI')                          //
//                                                                     //
//                                                                     //
//   Author: Sergey A.        <a-s-m@km.ru>                            //
//                                                                     //
//   Copyright (C)2019 Sergey A.                                       //
//                                                                     //
//   This program is free software; you can redistribute it and/or     //
//                  modify it in any ways.                             //
//   This program is distributed "AS IS" in the hope that it will be   //
//   useful, but WITHOUT ANY WARRANTY; without even the implied        //
//   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  //
//                                                                     //
/////////////////////////////////////////////////////////////////////////
```

## О программе

ODI.WCX - функциональный аналог древних утилит FILE2ODI, ODI_FORM, ODI_GRAB для
работы с простой посекторной копией дискет с файловой системой CP/M 80.
В отличие от них работает со всеми юзерами (а не только с user 0), не
задваивает файловые дескрипторы, умеет удалять CP/M-файлы и вообще -
"Все в одном", удобный интерфейc Total Commander (или его бесплатного аналога
для Windows/Linux Double Commander) или  Far Manager :
работа с ODI-файлом как с файлом архива с подкаталогами (USER0..USER15). 
При создании архива на системные дорожки диска можно записать код операционной
системы (в примерах это файл system.bin), код операционной системы на системных
дорожках легко менять (через запись/чтение специального файла).

Аналогично плагин UDI.WCX - для работы с образами дисков с файловой системой UZIX.

## История изменений:

- В версии 1.05 стало можно управлять содержимым прошивок "расширенных ROM" -
когда в Ромдиске Ориона-128 или в микро-ромдиске в РОМ2 Орион-ПРО выше 64к
расположена CP/M (для больших ПЗУ типа 27080/27801). Можно:
	- удалять/копировать содержащиеся в прошивке файлы CP/M,
	- смотреть информацию (занято/свободно и т.п.)
	- заливать или читать систему CP/M с системных дорог,
	- удалять/копировать тамошние файлы Ордос для обоих вариантов 128/ПРО.
Последние 2 пункта - через спец.подкаталог "UseThis_ToAccess_SystemTracks" - 
там и система в виде файла, и подкаталог с ордосовским барахлом. Настраиватся
аналогично прочим форматам: выбираете в настройках плагинов ТС какое расширение
(я настроил себе "ROM") обрабатывать плагином ODI.WCX
- В версии 1.04 "в помощь разработчикам ОС" добавлена опция модификации системы
на системных дорожках - теперь в образе диска или партиции кроме Юзеров
(каталогов CPM) еще есть спецкаталог "UseThis_ToAccess_SystemTracks" где в виде
виртуального файла можно копировать в обе стороны из/в системные дорожки (ODI.WCX).
И аналогично есть доступ к MBR (OНI.WCX). Копирование из образа копирует "как есть",
а копирование в образ проверяет и по необходимости сохраняет системные области 
(в MBR это информация о разделах, в системе - DPB, VolLabel, UserNames, FileDates).
Т.е. под компильнули mbrboot.bin или dos.bin, затем плагином/ТС закинули в образ.
- В версии 1.03 плагин исправлен для поддержки чтения и упаковки файлов более
512кб (максимальный размер файла для CP/M 2.2 - 4мб)
- В версии 1.01 исправлены ошибки предыдущих версий, добавлены функции для связи
с плагином ohi.wcx - "wrapper"-ом для работы с файловыми системами CP/M  внутри
партиций образа жесткого диска (Orion Hdd Image = OHI).
- В версии 1.0 добавлено отображения наличия файлов в "подкаталогах" (User-ах).
User-ы с файлами выводятся на верхнем регистре (заглавными буквами: 
 "USER_0","USER_1",...), пустые User-ы выводятся на нижнем регистре (строчными
буквами: "user_0","user_1",...).
- В версии 0.999 файл system.bin не обязательно должен находиться в том же каталоге,
где и odi.wcx, теперь имя файла образа системы для AutoSysGen настраивается в INI.
Если имя файла образа системы для AutoSysGen в INI в описании вормата не указано,
то параметры создаваемого архива вычисляются по раширению создаваемого файла - из
соответствующей строки INI секции [FORMATS] берется DPB (Disk Parameters Block) и
по нему вычисляются все параметры. Поэтому размер "диска" (т.е. архива) может быть
любым коррекно настроенным CP/M-диском (любого размера). Например, формат OD2
(Orion-HighDensity, на Орионе поддерживается в ACPM 2.x или AltairDos/BestDos)
имеет размер вдвое больший, чем стандартные 800к. А с дополнительными, >80, 
дорожками - более чем в два раза больше.
- В версии 0.99 при создании архива (образа диска) всегда брался BOOT-сектор
из ВООТ-сектора system.bin, по нему определялся размер ODI-файла,
положение и размер каталога и т.п.

Во всех версиях из этого же файла (В версии 0.99 всегда, В версии 0.999 - если
указано в INI ) на системные дорожки копируется система СР/М - получается
загрузочный диск.

В составе дистрибутива несколько файлов с кодами клонов систем CP/M 2.2:

  -  acpm1-53.bin  Advanced CP/M  SP-Computer Club 1992г.  - CP/M 2.2
                   один-в-один, с ее дискет можно грузиться безо всяких
                   дополнительных драйверов - работает через Монитор F800

  -  system.bin    AltairDOS 3.5 (AltairDOS с поддержкой HDD) для варианта
                   загрузки с дисковода (Floopy Boot)

  -  system.hdd    AltairDOS 3.5 (AltairDOS с поддержкой HDD) для варианта
                   загрузки с жесткого диска (IDE HDD Boot), используется 
                   OHI.WCX при создании архива (образа жесткого диска).
  Комментарий:
                для загрузки AltairDOS 3.x  в  user0  обязательно
                должен присутствовать цветной драйвер консоли drivеr.sys.

Hужный системный файл переименуйте в system.bin и положите туда же,
где расположен odi.wcx (для версии 0.99) или настройте его имя в INI.
   
По умолчанию, плагин считает, что диск содержит БлокПараметровДиска (DPB),
где описана конфигурация диска (формат CP/M Корвета, Специалиста, Ориона).
В этом случае плагин на все параметры настраивается автоматически. Если DPB
отсутствует (как в упрощенных в этом смысле CP/M Профи, Корвета, Robotron-1715,
MSX, Spectrum) и в INI в секции [PARAMS] указано   `USE_DPBLESS_DISKS=1`,
то плагин будет открывать такие архивы ориентируясь на DPB (DiskParamBlock),
описанные в секции [FORMATS]. Для работы с такими дискам в файле odi.ini активные
форматы должны быть перечислены в INI в секции [PARAMS] в ключе FORMATS_LIST.

Можно просматривать свойства уже созданных архивов, если открыв в одной панели 
коммандера архив, и перейдя в другую, перед упаковкой файла по Alt+F5 (Files->Pack)
в окне упаковщика (там где выбирается тип архиватора) нажать кнопку "Configure"

Не давайте разным форматам одинаковое расширение!
Такой вариант плагином не поддерживается !


## Примечание для OHI.WCX и FAR Manager:

  Инструкция по инсталляции немного усложняется. Делаем следующее:
   1. Far пока не запускаем. Распаковываем из архива OdiWcx.zip 
      файлы odi.ini, odi.wcx, ohi.ini, ohi.wcx в каталог 
      {Far_dir}\Plugins\MultiArc\Formats\WCX\
   2. Делаем в каталоге {Far_dir}\Plugins\MultiArc\Formats\WCX\ 
      КОПИЮ файлов odi.ini, odi.wcx в файлы с отличающимся именем, 
      к примеру odi_tmp.ini, odi_tmp.wcx
   3. Правим ohi.ini до такого вида:

[COMMON]
LibList=odi_tmp.wcx
DefaultFSsize=67108864
OScode=system.hdd

Все. можно запускать FAR и пользоваться.

Отчего все эти магические пассы: я чтобы упростить себе жизнь, в ohi.wcx
только работаю с партициями (вычисляю начало реальной файловой системы),
для обработки же файловой системы CP/M в нужный момент подгружаю odi.wcx,
которая суть dll. Винда кэширует загружаемые процессом dll (фактически, 
вторично ничего не загружает возвращает указатели на ранее загруженное), 
что черевато глюками и принудительно никак не отключается. О том, что это 
dll "другая" и незакэширована, Винда ориентируется по имени файла.

Far уже при старте загружает все dll, в т.ч. odi.wcx, чего ТС не делает 
до первого обращения к соотв. архиву, поэтому переменные в dll в FAR и 
TC инициализируются по-разному. В-общем, как-то так...

Ключ "DefaultFSsize" - размер создаваемого образа диска в байтах. Диск 
(образ) создается любого размера не более 4Gb, в нем создается MBR c 
BootLoader (то самое меню, которое при старте на Орионе предлагает выбрать
с какой из партиций загружаться) и одна партиция с файловой системой 
размером не более 32Mb, на системную дорожку пишется файл, указанный в 
ключе "OScode". Все остальное место - свободное, под CP/M на нем можно 
насоздавать еще партиций при помощи программы fdisk.com.

```
Лиричесоке отступление.
Для CP/M 2.2 ограничение емкости файловой системы - гигабайт. У меня
прекрасно в обычной 2.2 работают FS размером в 64М, и то это ограничение
сознательно наложено лично мной - сугубо из-за размера буфера ALV в BIOS,
(который в общей сложности при максимальном для CPM размере блока в 16кб
позволяет описать занятость до 100Мб), т.к. я посчитал что увеличить TPA
(подняв повыше BIOS) важнее, чем емкость файловой системы, которых можно
иметь несколько и подмонтировать одну вместо другой "на лету". Также, я
отказался от каталогов "а-ля FAT" в CP/M, т.к. прекрасно обходился без них.
Использовал классическую FS CP/M с шестнадцатью юзерами. Классический СР/М
софт, который умел работать с юзерами, нормально не работает с любыми
переделками "под каталоги". Поэтому я решил что все это от лукавого 
(как и FAT в качестве основной FS), и не поддерживаю такое в плагине. 
```

В TC новый файл-архив (образ диска в нашем случае) создается по alt+F5 
там же можно посмотреть свойства файловых систем если для существующего 
архива открыть окно по alt+F5 и нажать "Configure". Как это в Far - не знаю.




## Пример ODI.INI:

```
*****
* ODI (Orion Disk Image, CP/M) - image size strictly (!) 819200 bytes:
*   5 x 1024b sectors, double density, 80 tracks, 4k catalog, 4 system tracks, 16k extents
**
* ODI_5x1024x2x80= 01010301050050002800040F0084017F00C00020000400
*      0101
* len: $03;    // размер сектора 0=128, 1=256, 2=512, 3=1024  - orion(korvet) feature - must be defined!
* den: $01;    // плотность (или стороны ?):  0=одна,  1=две  - orion(korvet) feature - must be defined!
* sec: $0005;  // phisical sectors per track                  - this and items below are standard CPM features
* trk: $0050;  // phisical tracks on disk (one side)
* spt: $0028;  // logical sectors (128b) per track
* bsh: $04;    // Block Shift - Block Size is given by 128 * 2^(BSH).  Here BlockSize=2048
* blm: $0F;    // Block Mask - Block Size is given by 128 * (BLM +1).  Here BlockSize=2048
* exm: $00;    // Extent Mask (0=16k, 1=32k, 3=64k, 7=128k)
* dsm: $0184;  // user space size in 2048b blocks = SEC * (TRK-OFF) - (CKS/8)
* drm: $007F;  // max quantity of file records (FCBs) in catalog -1
* al:  $00C0;  // 16-bit Directory Allocation Pattern
* cks: $0020;  // Directory Check Sum = catalog size (in logical blocks)
* off: $0004;  // system tracks 
*
*****
* OD2 (Orion Disk Image, HighDensity, ACPM 2.x или Altair-DOS) - image size 1740800 bytes:
*  10 x 1024b sectors, double density, 85 tracks, 8k catalog, 2 system tracks, 16k extents
**
* OD2_10x1024x2x80= 010103010A0055005000040F004403FF00F00040000200
*      0101
* len: $03;    // размер сектора 0=128, 1=256, 2=512, 3=1024  - orion(korvet) feature - must be defined!
* den: $01;    // плотность (или стороны ?):  0=одна,  1=две  - orion(korvet) feature - must be defined!
* sec: $000A;  // phisical sectors per track                  - this and items below are standard CPM features
* trk: $0055;  // phisical tracks on disk (one side)
* spt: $0050;  // logical sectors (128b) per track
* bsh: $04;    // Block Shift - Block Size is given by 128 * 2^(BSH).  Here BlockSize=2048
* blm: $0F;    // Block Mask - Block Size is given by 128 * (BLM +1).  Here BlockSize=2048
* exm: $00;    // Extent Mask (0=16k, 1=32k, 3=64k, 7=128k)
* dsm: $0344;  // user space size in 2048b blocks = SEC * (TRK-OFF) - (CKS/8)
* drm: $00FF;  // max quantity of file records (FCBs) in catalog -1
* al:  $00F0;  // 16-bit Directory Allocation Pattern
* cks: $0040;  // Directory Check Sum = catalog size (in logical blocks)
* off: $0002;  // system tracks 
*
*****
* PRO (Profi Disk Image, CP/M) - image size strictly (!) 819200 bytes:
*   5 x 1024b sectors, double density, 80 tracks, 4k catalog, 0 system tracks, 16k extents
**
* PROFI_5x1024x2x80= 01010301050050002800040F008E017F00C00020000000
*      0101
* len: $03;    // размер сектора 0=128, 1=256, 2=512, 3=1024  - orion(korvet) feature - must be defined!
* den: $01;    // плотность (или стороны ?):  0=одна,  1=две  - orion(korvet) feature - must be defined!
* sec: $0005;  // phisical sectors per track                  - this and items below are standard CPM features
* trk: $0050;  // phisical tracks on disk (one side)
* spt: $0028;  // logical sectors (128b) per track
* bsh: $04;    // Block Shift - Block Size is given by 128 * 2^(BSH).  Here BlockSize=2048
* blm: $0F;    // Block Mask - Block Size is given by 128 * (BLM +1).  Here BlockSize=2048
* exm: $00;    // Extent Mask (0=16k, 1=32k, 3=64k, 7=128k)
* dsm: $018E;  // user space size in 2048b blocks = SEC * (TRK-OFF) - (CKS/8)
* drm: $007F;  // max quantity of file records (FCBs) in catalog -1
* al:  $00C0;  // 16-bit Directory Allocation Pattern
* cks: $0020;  // Directory Check Sum = catalog size (in logical blocks)
* off: $0000;  // system tracks 
*
*****
* TRD (Spectrum Disk Image)          - image size strictly (!) 655360 bytes:
*   16 x 256b sectors, double density, 80 tracks, 4k catalog, 2 system tracks, 16k extents
**
* TRD_16x256x2x80=    01010101100050002000040F003A017F00C00020000200
*      $0101
* len: $01;    // размер сектора 0=128, 1=256, 2=512, 3=1024  - orion(korvet) feature - must be defined!
* den: $01;    // плотность (или стороны ?):  0=одна,  1=две  - orion(korvet) feature - must be defined!
* sec: $0010;  // phisical sectors per track                  - this and items below are standard CPM features
* trk: $0050;  // phisical tracks on disk (one side)
* spt: $0020;  // logical sectors (128b) per track
* bsh: $04;    // Block Shift - Block Size is given by 128 * 2^(BSH).  Here BlockSize=2048
* blm: $0F;    // Block Mask - Block Size is given by 128 * (BLM +1).  Here BlockSize=2048
* exm: $00;    // Extent Mask (0=16k, 1=32k, 3=64k, 7=128k)
* dsm: $013A;  // user space size in 2048b blocks = SEC * (TRK-OFF) - (CKS/8)
* drm: $007F;  // max quantity of file records (FCBs) in catalog -1
* al:  $00C0;  // 16-bit Directory Allocation Pattern
* cks: $0020;  // Directory Check Sum = catalog size (in logical blocks)
* off: $0002;  // system tracks 
*  
*****
* DSK (MSX CP/M Plus Disk Image)     - image size strictly (!) 368640 bytes:
*   9 x 512b sectors, single density, 80 tracks, 4k catalog, 2 system tracks, 32k extents
**
* DSK_9x512x1x80=   01010200090050002400040F01AD007F00C00020000200
* может, что и 2х40, но это без разницы
*      $0101
* len: $02;    // размер сектора 0=128, 1=256, 2=512, 3=1024  - orion(korvet) feature - must be defined!
* den: $00;    // плотность (или стороны ?):  0=одна,  1=две  - orion(korvet) feature - must be defined!
* sec: $0009;  // phisical sectors per track                  - this and items below are standard CPM features
* trk: $0050;  // phisical tracks on disk (one side)
* spt: $0024;  // logical sectors (128b) per track
* bsh: $04;    // Block Shift - Block Size is given by 128 * 2^(BSH).  BlockSize=2048
* blm: $0F;    // Block Mask - Block Size is given by 128 * (BLM +1).  BlockSize=2048
* exm: $01;    // Extent Mask (0=16k, 1=32k, 3=64k, 7=128k)
* dsm: $00AD;  // user space size in 2048b blocks = SEC * (TRK-OFF) - (CKS/8)
* drm: $007F;  // max quantity of file records (FCBs) in catalog -1
* al:  $00C0;  // 16-bit Directory Allocation Pattern
* cks: $0020;  // Directory Check Sum = catalog size (in logical blocks)
* off: $0002;  // system tracks 
*  
*
*описатель формата: <расширение файла>=<DiskParamBlock>[,<OS image file for sysgen>]
*
[PARAMS]
USE_DPBLESS_DISKS=1
FORMATS_LIST=ODI, OD2, TRD, DSK, PRO

[FORMATS]
ODI = 01010301050050002800040F0084017F00C00020000400 , system.bin
OD2 = 010103010A0055005000040F004403FF00F00040000200
TRD = 01010101100050002000040F003A017F00C00020000200 
DSK = 01010200090050002400040F01AD007F00C00020000200 
PRO = 01010301050050002800040F008E017F00C00020000000
```
