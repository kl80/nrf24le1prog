#nRF24LE1 AVRasp downloader 

Consist of AVRasp firmware and command line utility. Command line required libusb (or libusb-win32 for windows). I have checked it for OS X only but it should work for Windows and Linux, will be happy for build for these platforms.

Command line:
- <b>nrf24le1prog test</b> - test nrf24le1 module by enabling and disabling write flag.
- <b>nrf24le1prog write code <i>\<filename></i></b> - reads firmware from file and write to memory, before writing flash pages will be erased, only required pages will be processed so if firmware is 985 bytes length only 2 pages will be erased and programmed.
- <b>nrf24le1prog read code <i>\<filename></i></b> - reads code meory (16kb) to file
- <b>nrf24le1prog validate code <i>\<filename></i></b> - compare firmware in device and file, only first bytes will be compared if file length is less then 16kb
- <b>nrf24le1prog erase code</b> - erases all code pages, it's not erase all which can break device, it doesn't clear infopage and nvl memory

#How to build

1. AVRasp - open console, go to avrasp folder, type <b>make clean hex</b>, requires avr-gcc in path. nrf24le1prog_asp.hex should be downloaded to AVRasp hardware.
2. Command line utility - open console, go to prog folder, type <b>make clean prog</b>, requires libusb installed and added to path.

#Connect to nRF24LE1

#Thanks to

MaxMS for his http://homes-smart.ru and ideas for programming nRF24le1 using AVRasp.<br>
Thomas Fischl for AVRasp, http://www.fischl.de/usbasp.

----------------
#Программатор для nRF24LE1 на основе AVRasp

Состоит из прошивки для AVRasp и утилиты командной строки. Последняя требует установленного libusb (libusb-win32 в случае Windows).
Проверена для OS X, но должна работать для Windows и Linux, буду признателен, если кто-нибудь соберет и проверит для этих операционных систем.

Командная строка:
- <b>nrf24le1prog test</b> - проверяет nrf24le1 переключая разрешение на запись в регистре статуса, смотрим, чтобы была надпись SUCCESSED, в случае FAILED проверяем соединение или берем другой модуль.
- <b>nrf24le1prog write code <i>\<filename></i></b> - записывает прошивку из файла, предварительно очищая страницы, будут очищены и записаны только необходимые страницы, т.е. если размер прошивки 980 байт, только 2 первые страницы будут очищены и записаны.
- <b>nrf24le1prog read code <i>\<filename></i></b> - записывает содержимое памяти программы (16кб) в файл.
- <b>nrf24le1prog validate code <i>\<filename></i></b> - сравнивает прошивку с содержимым файла, если рамер файла меньше 16кб - остаток игнорирется.
- <b>nrf24le1prog erase code</b> - очищает память программы (16кб), не затрагивая inforpage и nvl память, безопасна для устройства.

#Как собрать

1. AVRasp - в консоли открыть папку avrasp, выполнить <b>make clean hex</b>, требует установленного avr-gcc, добавленного в PATH. После этого прошить nrf24le1prog_asp.hex с помощью avrdude.
2. Утилита командной строки - в консоли открыть папку prog folder, выполнить <b>make clean prog</b>, требует libusb в переменной PATH.

#Подключить nRF24LE1

#Благодарности

MaxMS и его сайту http://homes-smart.ru
<br>
Thomas Fischl за AVRasp, http://www.fischl.de/usbasp.
