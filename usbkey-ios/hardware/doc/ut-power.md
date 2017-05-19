Power unit-test
===============

The goal of this test is to validate the 3.3V regulator.

No load
-------

    The first step is tu start the regulator without load. At this point a
bug has been detected : the EN pin mus NOT be left floating (see 7.4.3 of
LM3671 datasheet). To fix this, a strap has been added between VIN and EN
pin.

 * Input : 5V from laboratory power supply
 * Mesured output voltage (at TP3) : 3.38V
 * Mesured ripple : 29.2mV, main freq 30.30kHz

    The device is in PFM mode, this values are acceptable.

Light load
----------

    In this second test, a small resistive load (100ohm) has been connected
to TP3.

 * Input : 5V (mesured input current : 30mA)
 * Mesured output voltage (at TP3) : 3.34V
 * Mesured ripple : 39.2mV

Large load
----------

    This test has not been made yet.
