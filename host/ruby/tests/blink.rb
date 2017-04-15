require 'ifusb'

# blink IO2 30 times

Ifusb.init

Ifusb::GPIO.config(Ifusb::IO2,Ifusb::OUTPUT)

30.times do |t|
	Ifusb::GPIO.set(Ifusb::IO2)
	sleep 1
	Ifusb::GPIO.clear(Ifusb::IO2)
	sleep 1
end

Ifusb.close