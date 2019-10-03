```
/////////////////////////////////////////////////////////////////////////
//                                                                     //
//   Orion/Z (Orion-128 + Z80-CARD-II) emulator, version 1.08 files.   //
//                                                                     //
//   Addon: TotalCommander archiver (WCX) plugin for serving OHI files //
// [ohi.wcx](https://github.com/serge-404/OriZEmu/tree/master/UTILS/OdiWcx-OhiWcx): (Orion HDD Image files). Allow copy/extract files to/from  //
//          any supported image MBR-partitions such simple as serving  //
//          any archives in TotalCommander interface. It is a wrapper  //
//          for wcx-plugins: ODI,UDI,FDI & compatible - allow use that //
//          plugins with an offset (within partitions), visualizing    //
//          image initializing (put MBR record to LBA=0) and creating  //
//          or deleting partitions - just press F7 and write partition //
//          type and size delimited with ' ' or ',' :  "UZIX 10240k",  //
//          "FAT,20M", "CPM 6553600",...               Version 1.0.7.x //
//                                                                     //
//   Addon: TotalCommander archiver (WCX) plugin for serving ODI files //
// [odi.wcx](https://github.com/serge-404/OriZEmu/tree/master/UTILS/OdiWcx-OhiWcx): (Orion Disk Image files). Allow copy/extract CP/M files    //
//          to/from ODI file "diskette" such simple as processing any  //
//          archives in TotalCommander interface.    Version 1.0.6.x   //
//                                                                     //
//   Addon: TotalCommander archiver (WCX) plugin for serving FDI files //
// fdi.wcx: (UZIX Disk Image files). Allow copy/extract FAT16/32 files //
//          to/from FDI file "diskette" such simple as processing      //
//          any archives in TotalCommander interface.  Version 1.0.0.1 //
//                                                                     //
//   Addon: TotalCommander archiver (WCX) plugin for serving UDI files //
// udi.wcx: (UZIX Disk Image files). Allow copy/extract UZIX files     //
//          to/from UDI/IMG file "diskette" such simple as processing  //
//          any archives in TotalCommander interface.  Version 1.0.0.1 //
//                                                                     //
//   Addon: Windows execuables for CLI-managing UZIX filesytem images  //
//          (both disk images and HDD images with base MBR-partitions) //
//mkfs.exe: creating=format UZIX filesystem within OHI image partition //
//fsck.exe: check/fix UZIX filesystem within OHI image partition       //
// ucp.exe: copy/extract UZIX files to/from UZIX filesystem within OHI //
//          or UDI images, cat, mkdir, mknod, chmod, symlinks, etc..   //
//                                                                     //
//   How to install this plugins (32 bit only) in TotalCommander:      //
//          1. Unzip odi.wcx(for ex.),system.bin,odi.ini to any folder //
//                (usually c:\wincmd\Plugins)                          //
//          2. In Windows Commander 5.5 (or newer), choose             //
//                Configuration - Options                              //
//          3. Open the 'Packer' page                                  //
//          4. Click 'Configure packer extension DLLs'                 //
//          5. type the "ODI" (for ex.) extension                      //
//          6. Click 'new type', and select the  odi.wcx               //
//          7. Click OK.  Click OK                                     //
//          8. Repeat steps 2-7 for other supported (specified in      //
//               ODI.INI) formats - such as TRD, DSK, etc.             //
//                                                                     //
//   How to install this plugins (32 bit only) in DoubleCommander:     //
//          **TODO** (actually it is very simillar to TotalCommander)  //
//                                                                     //
//   How to install this plugin (32 bit only) in Far Manager:          //
//          0. Install wcx.dll (plugin allowing TC WCX-plugins usage   //
//             in Far) to                                              //
//                   {FAR_DIR}\Plugins\Multiarc\Formats\WCX\           //
//             Install wcx.fmt to                                      //
//                   {FAR_DIR}\Plugins\Multiarc\Formats\               //
//          1. Unzip odi.wcx (for ex.), system.bin to                  //
//                   {FAR_DIR}\Plugins\Multiarc\Formats\WCX\           //
//          1.2. To Support any other CP/M format (specified in INI),  //
//               just copy ODI.WCX, ODI.INI to files with              //
//               file name correcponding to format extension (for      //
//               example:  DSK.WCX, DSK.INI ;  TRD.WCX, TRD.INI) to    //
//                   {FAR_DIR}\Plugins\Multiarc\Formats\WCX\           //
//          2. Restart Far                                             //
//                                                                     //
//                                                                     //
//   Installation process for ohi.wcx (HDD images plugin) or fdi.wcx   //
//   (FAT16/32 partitions) or udi.wcx (UZIX partitions) is the same    //
//          (except replacing 'ODI'->'OHI'/'FDI'/'UDI')                //
//                                                                     //
//                                                                     //
//   Author: Sergey A.        <a-s-m@km.ru>                            //
//                                                                     //
//   Copyright (C)2009-2019 Sergey A.                                  //
//                                                                     //
//   This program is free software; you can redistribute it and/or     //
//                  modify it in any ways.                             //
//   This program is distributed "AS IS" in the hope that it will be   //
//   useful, but WITHOUT ANY WARRANTY; without even the implied        //
//   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  //
//                                                                     //
/////////////////////////////////////////////////////////////////////////
```

Russian:
## О программе

[ODI.WCX](https://github.com/serge-404/OriZEmu/tree/master/UTILS/OdiWcx-OhiWcx) - плагин для работы с простой посекторной копией дискет с файловой
системой CP/M 80. Работает со юзерами (0..15), умеет удалять CP/M-файлы,
создавать файловую систему, т.е. "Все в одном" - удобный интерфейc
Total Commander (или его бесплатного аналога для Windows/Linux Double Commander)
или  Far Manager: работа с ODI-файлом как с файлом архива с подкаталогами
(USER0..USER15). На системные дорожки диска можно записать код операционной
системы - как при создании ФС (в примерах INI это файл system.bin), так и
позже поменять (через запись/чтение специального файла).

Аналогично работают плагины:
 UDI.WCX - для работы с образами дисков с файловой системой UZIX.
 FDI.WCX - для работы с образами дисков с файловой системой FAT (16/32).

"дискетные" (xDI.WCX) плагины могут работать с MBR-образами дисков (*.OHI)
под управлением плагина-обертки OHI.WCX, который для каждого типа партиций
(CPM/UZIX/FAT) вызывает соответствующий плагин (настраиваемо в OHI.INI).

В UDI.WCX (плагин для образов ФС UZIX) есть опция работы с bootsector и
системными дорожками образа - запись/чтение кода бутсектора и кода ОС внутрь
системных структур не повреждая исходного форматирования ФС через виртуальный
подкаталог с виртуальными файлами boot.bin (может быть переименован в INI),
system.bin (при последующей перезаписи если записываемый файл хотя бы на 32
байта меньше системной области, в системной области сохранится имя и дата
записанного файла ядра). Фича необходимая, т.к. во всех репозиториях ЮЗИКСа,
что я видел, бутсектор и ядро ОС компилируются раздельно, и их после компиляциии
как-то надо записывать на диск.

В CPM-разделах код бутсектора и ОС заливается одним общим файлом (т.к. в
АльтаирДОС оно у меня компилируется в один общий файл - чтобы при крайней
необходимости можно было тупо одной командой POWER.COM залить скомпилированное
на системные треки в нативной CP/M, впрочем в АльтаирДОС это умеет и fdisk,
причем более интеллектуально), это тоже обрабатывается через виртуальный файл,
но уже соответственно в ODI.WCX.

[OHI.WCX](https://github.com/serge-404/OriZEmu/tree/master/UTILS/OdiWcx-OhiWcx) имеет возможность инициализации образа (создать запись MBR, доступ к
его коду позже аналогично есть через специальный файл в корне архива), а также
создания и удаления primary MBR партиций типа CPM, FAT и UZIX (создаются вместе
с файловой системой). Работа тут как с подкаталогами (создать-F7, удалить F8),
при создании в качестве "имени каталога" указывается тип и размер партиции,
например "UZIX 20M", "FAT 32768000" или "CPM 1500K". Т.е. не нужен fdisk -
взяли любой подходящий файл (лучше зануленый - чтобы архивировался хорошо),
переименовали в OHI, проинитили (просто войдя в этот образ), создали партиции,
в них создали структуру каталогов (в случае UZIX), накидали файлы. Всё.


Для сборки использую Borland C++ Builder 5, makefile не делал (открывайте .bpr)
для UDI.WCX и FDI.WCX, и Borland Delphi 5 для ODI.WCX и OНI.WCX


## История изменений ODI.WCX:

- В версии 1.05 стало можно управлять содержимым прошивок "расширенных ROM" -
когда в Ромдиске Ориона-128 или в микро-ромдиске в РОМ2 Орион-ПРО выше 64к
расположена CP/M (для больших ПЗУ типа 27080/27801). Можно:
	- удалять/копировать содержащиеся в прошивке файлы CP/M,
	- смотреть информацию (занято/свободно и т.п.)
	- *заливать или читать систему CP/M с системных дорог,
	- *удалять/копировать Ордос-файлы 0-страницы ПЗУ для вариантов 128/ПРО.
*Последние 2 пункта - через спец.подкаталог "UseThis_ToAccess_SystemTracks" - 
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
Для CP/M 2.2 ограничение емкости файловой системы - 1 гигабайт. У меня
прекрасно в обычной 2.2 работают FS размером в 64М, и то это ограничение
сознательно наложено лично мной - сугубо из-за размера буфера ALV в BIOS,
(который в общей сложности при максимальном для CPM размере блока в 16кб
позволяет описать занятость до 100Мб), т.к. я посчитал что увеличить TPA
отдав максимум памяти стандартным приложениям (подняв повыше BIOS) важнее,
чем емкость файловой системы, которых на одном устройстве можно иметь несколько
(как и устройств) и подмонтировать одну вместо другой "на лету". Также, я
отказался от каталогов "а-ля FAT" в CP/M, т.к. прекрасно обходился без них.
Использовал классическую FS CP/M с шестнадцатью юзерами (в v1.06 это количество
настраиваемо в INI). Классический СР/М софт, который умел работать с юзерами,
нормально не работает с любыми переделками "под каталоги". Поэтому я решил что
все это "от лукавого" (как и FAT в качестве основной FS в CP/M), и более не
поддерживаю такое в плагине. Хотите подкаталоги - делайте в образе отдельный
раздел с файловой системой FAT или UZIX - такие партиции и плагинами поддержаны
(см. выше), и в CP/M есть утилиты для копирования из/в такие партиции.
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
*****
* QDI (Quorum-64 Disk Image, CP/M) - image size 819200 bytes(?):
*   5 x 1024b sectors, double density, 80 tracks(?), 4k catalog, 4 system tracks, 16k extents
**
*
*описатель формата: <расширение файла>=<DiskParamBlock>[,<OS image file for sysgen>]
*
[PARAMS]
USE_DPBLESS_DISKS=1
FORMATS_LIST=ODI, OD2, TRD, DSK, PRO, QDI

[FORMATS]
ODI = 01010301050050002800040F0084017F00C00020000400 , system.bin
OD2 = 010103010A0055005000040F004403FF00F00040000200
TRD = 01010101100050002000040F003A017F00C00020000200 
DSK = 01010200090050002400040F01AD007F00C00020000200 
PRO = 01010301050050002800040F008E017F00C00020000000
*QDI = 01010301050050002800040F008E017F00C00020000400
QDI = 01010301050050002800040F0084017F00C00020000400

===============================================================================

UZIX utilities:

   MKFS:       This makes a file system.  It takes four arguments: a drive
	       name (optionally - with partition number), the total number of blocks, the number of inodes and
	       the total of reserved blocks (for kernel image, used by
	       mkboot) [optional].

   FSCK:       This checks file system consistency.  It takes a single
	       argument, the drive name to check.

   UCP:        This is an interactive program that performs a number of
	       operations.  It can be used to make and delete directories and
	       device files, upload and download files between the host OS and
	       the UZIX filesystem, list directories, and print files.  It
	       takes a single argument, the drive name. It then prompts
	       for commands.  Type 'help' at the prompt for a complete list of
	       commands.

fsck example:
=============
REM
REM  HDD partition # 1 image fsck (second partition, zero-based numbering)
REM
c:\Temp\WCX_BIN>fsck 1:c:\Temp\WCX_BIN\uzix-ori.ohi

fsck - UZIX utility for filesystem image check, V1.0.

Checking drive `1:c:\Temp\WCX_BIN\uzix-ori.ohi` with fsize 1440 blocks, isize 25
 blocks, rsize 0 blocks. Confirm? y

Pass 1: Checking inodes.
Inode 184 with mode 0173366 is not of correct type. Zap? y
Inode 185 with mode 0173366 is not of correct type. Zap? y
Pass 2: Rebuilding free list.
        Rebuild free list? y
Pass 3: Checking block allocation.
Pass 4: Checking directory entries.
Pass 5: Checking link counts.
Done.

REM
REM  HDD partition # 0 image fsck (first partition, zero-based numbering)
REM
c:\Temp\WCX_BIN>
c:\Temp\WCX_BIN>fsck 0:c:\Temp\WCX_BIN\uzix-ori.ohi

fsck - UZIX utility for filesystem image check, V1.0.

Checking drive `0:c:\Temp\WCX_BIN\uzix-ori.ohi` with fsize 1440 blocks, isize 25
 blocks, rsize 60 blocks. Confirm? y

Pass 1: Checking inodes.
Pass 2: Rebuilding free list.
        Rebuild free list? y
Pass 3: Checking block allocation.
Pass 4: Checking directory entries.
Pass 5: Checking link counts.
Done.

REM
REM  FDD image fsck (no MBR, no partitions)
REM
c:\Temp\WCX_BIN>
c:\Temp\WCX_BIN>fsck -y c:\Temp\WCX_BIN\uzix.udi

fsck - UZIX utility for filesystem image check, V1.0.

Checking drive `c:\Temp\WCX_BIN\uzix.udi` with fsize 1440 blocks, isize 25 block
s, rsize 60 blocks. Confirm? Y

Pass 1: Checking inodes.
Pass 2: Rebuilding free list.
        Rebuild free list? Y
Pass 3: Checking block allocation.
Pass 4: Checking directory entries.
Pass 5: Checking link counts.
Done.

```
