#ZX Spectrum+ Keyboard Project

Being a ZX Spectrum fan with a small collection including my original ZX Spectrum 48k from Christmas 1983, I decided to take an empty Spectrum+ case and turn it into a keyboard that could be used with a Rasberry Pi running the Fuse-sdl emulator.

There is some great information on Google about others that have tackled the problem, so having checked out what others have done including a great write up by [PJ Evans](http://mrpjevans.com/2015/12/building-the-zx-raspberry-part-one/) I __borrowed__ my sons Arduino kit and started experimenting.

This github project is the Arduino code I'm using to drive the keyboard through a Pro Micro and I've also attached a [Fritzing](http://fritzing.org/home/) diagram of the wiring :O)

Thanks go to all those who have gone before me and shared their information. It made the whole exercise much easier.

#Hardware used


ZX Spectrum+ case. I found a cheap one on eBay that had no mother board, so I didn't need to harm a working Spectrum to make the Spectrum Pi.

[Raspberry Pi 2 B](https://www.raspberrypi.org/products/raspberry-pi-2-model-b/)

[Micro USB 5 pin female to male left angle extension cable USB 2](http://www.amazon.co.uk/gp/product/B00NIGQ2FK?psc=1&redirect=true&ref_=oh_aui_detailpage_o00_s00)

[Short 12cm USB 2.0A Extension Cable](http://www.amazon.co.uk/gp/product/B00DX51EL8?psc=1&redirect=true&ref_=oh_aui_detailpage_o01_s00)

[Super Short Mini Travel Micro USB Data Charge Cable](http://www.amazon.co.uk/gp/product/B00B3NWGGO?psc=1&redirect=true&ref_=oh_aui_detailpage_o02_s00)

[Short 10cm Flat HDMI extension cable](http://www.ebay.co.uk/itm/350966684102?_trksid=p2060353.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT)

##Keyboard Electronics

[Pro Micro Arduino compatible ATMega32U4](http://www.ebay.co.uk/itm/131464919559?_trksid=p2060353.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT)

6 x 10k Resisters

[8 x 1N4148 Diodes](http://www.maplin.co.uk/search?text=1N4148&x=0&y=0)

[Veroboard](http://www.maplin.co.uk/p/veroboard-copper-stripboard-100x160mm-a62rl)

ZX Spectrum Keyboard membrane connectors. Got mine from eBay for about £2.50

Piezo sounder. I used one that came with my sons Arduino kit

[Single core wires](http://www.ebay.co.uk/itm/262202710636?_trksid=p2060353.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT)




