import usb.core

# LEDs
UP_LED = 0
DOWN_LED = 1
LEFT_LED = 2
RIGHT_LED = 3

# LED functions
LED_ON = 1
LED_OFF = 2
LED_TOGGLE = 3


def led_on(usb_dev, led):
	usb_dev.ctrl_transfer(0x40, led, LED_ON, 0, 'LED on')

def led_off(usb_dev, led):
	usb_dev.ctrl_transfer(0x40, led, LED_OFF, 0, 'LED off')

def led_toggle(usb_dev, led):
	usb_dev.ctrl_transfer(0x40, led, LED_TOGGLE, 0, 'LED toggle')

def led_toggle_all(usb_dev):
	led_toggle(usb_dev, UP_LED)
	led_toggle(usb_dev, DOWN_LED)
	led_toggle(usb_dev, LEFT_LED)
	led_toggle(usb_dev, RIGHT_LED)

def led_on_all(usb_dev):
	led_on(usb_dev, UP_LED)
	led_on(usb_dev, DOWN_LED)
	led_on(usb_dev, LEFT_LED)
	led_on(usb_dev, RIGHT_LED)

def led_off_all(usb_dev):
	led_off(usb_dev, UP_LED)
	led_off(usb_dev, DOWN_LED)
	led_off(usb_dev, LEFT_LED)
	led_off(usb_dev, RIGHT_LED)

if __name__ == '__main__':

	# Find our device
	dev = usb.core.find(idVendor=0x10C4, idProduct=0x0007)
	if dev is None:
		raise ValueError('Device not found')
	
	# Set the default configuration
	dev.set_configuration()

	while True:
		command = raw_input("> ")

		if command == "on":
			led_on_all(dev)
		elif command == "off":
			led_off_all(dev)
		elif command == "toggle":
			led_toggle_all(dev)
		elif command == "up on":
			led_on(dev, UP_LED)
		elif command == "up off":
			led_off(dev, UP_LED)
		elif command == "up toggle":
			led_toggle(dev, UP_LED)
		elif command == "down on":
			led_on(dev, DOWN_LED)
		elif command == "down off":
			led_off(dev, DOWN_LED)
		elif command == "down toggle":
			led_toggle(dev, DOWN_LED)
		elif command == "left on":
			led_on(dev, LEFT_LED)
		elif command == "left off":
			led_off(dev, LEFT_LED)
		elif command == "left toggle":
			led_toggle(dev, LEFT_LED)
		elif command == "right on":
			led_on(dev, RIGHT_LED)
		elif command == "right off":
			led_off(dev, RIGHT_LED)
		elif command == "right toggle":
			led_toggle(dev, RIGHT_LED)
		else:
			print "Wrong input value. Try again"
