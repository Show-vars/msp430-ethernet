# Ethernet radio bridge

Ethernet radio bridge using [MSP430G2553](http://www.ti.com/product/MSP430G2553) ([Launchpad MSP-EXP430G2](http://www.ti.com/tool/msp-exp430g2)), [ENC28J60](http://www.microchip.com/wwwproducts/en/en022889) (some of the [pre assebled modules](http://www.geeetech.com/wiki/index.php/Arduino_ENC28J60_Ethernet_Module)) and any LoRa transceiver (still draft).

Unfortunately, MSP430G2553 have 512 bytes of ram only. So for this reason we unable to retransmit ethernet frames larger than ~480 bytes (intead of standart 1518 or Jumbo frames that may be larger than 9Kbytes).

Setting MTU on both sides less than 480 will allow to use this bridge with MSP430G2553. But as it turned out, not any desktop or mobile ethernet interfaces and OS's will allow to do this. Seems that Atheros SOC's with OpenWRT on board can handle small MTU and successfully do IP packet fragmentation.
