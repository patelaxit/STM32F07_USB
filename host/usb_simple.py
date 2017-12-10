import usb.core
import usb.util as uu
from struct import *
import array
import vendor_requests

def bytesToString(inArray):
	return "".join(map(chr, inArray))

def stringToBytes(inString):
	return array.array('B', inString)

if __name__ == '__main__':

	# Find our device
	dev = usb.core.find(idVendor=0x10C4, idProduct=0x0007)
	if dev is None:
		raise ValueError('Device not found')
	
	# Set the default configuration
	dev.set_configuration()

	# Define request IN and OUT maks
	reqIn = uu.CTRL_IN | uu.CTRL_TYPE_VENDOR | uu.CTRL_RECIPIENT_INTERFACE
	reqOut = uu.CTRL_OUT | uu.CTRL_TYPE_VENDOR | uu.CTRL_RECIPIENT_INTERFACE

	while True:
		command = raw_input("> ")

		if command == "toggle":
			print "\n======================================================="
			print "Toggling LED"
			print "=======================================================\n"
			# usb_dev.ctrl_transfer(bmRequestType, bRequest, wValue=0, wIndex=0, data_or_wLength=None, timeout=None)
			dev.ctrl_transfer(reqOut, vendor_requests.LED_TOGGLE, 0, 0, 'toggle')
		
		elif command == "on":
			print "\n======================================================="
			print "All LEDs on"
			print "=======================================================\n"
			# usb_dev.ctrl_transfer(bmRequestType, bRequest, wValue=0, wIndex=0, data_or_wLength=None, timeout=None)
			dev.ctrl_transfer(reqOut, vendor_requests.LED_ON, 0, 0, 'on')
		
		elif command == "off":
			print "\n======================================================="
			print "All LEDs off"
			print "=======================================================\n"
			# usb_dev.ctrl_transfer(bmRequestType, bRequest, wValue=0, wIndex=0, data_or_wLength=None, timeout=None)
			dev.ctrl_transfer(reqOut, vendor_requests.LED_OFF, 0, 0, 'off')
		
		elif command == "loopback":
			# Create empty array
			str_out = "0x48"
			buf_out = stringToBytes(str_out)
			buf_out_len = len(buf_out)
			print "\n======================================================="
			print "Control Request Loopback"
			print "======================================================="
			print "String out: {}".format(str_out)
			print "Array out length: {}".format(buf_out_len)
			print "Array out: {}".format(buf_out)
			print "======================================================="
			# Write out the buffer to the device
			dev.ctrl_transfer(reqOut, vendor_requests.CTRL_LOOPBACK_WRITE, 0, 0, buf_out)

			# Read data back from the device
			buf_in = dev.ctrl_transfer(reqIn, vendor_requests.CTRL_LOOPBACK_READ, 0, 0, buf_out_len)
			buf_in_len = len(buf_in)
			str_in = bytesToString(buf_in)
			print "String in: {}".format(str_in)
			print "Array in length: {}".format(buf_in_len)
			print "Array in: {}".format(buf_in)
			print "=======================================================\n"

		elif command == "bulk":
			print "\n======================================================="
			print "Bulk Data Transfer"
			print "======================================================="
			message = raw_input("Enter message to send: ")
			data = stringToBytes(message)
			n = dev.write(0x01, data)
			
			print "======================================================="
			print "wrote {} bytes\nstring: {}\nbytes: {}".format(n, message, data)
			print "======================================================="
			
			data = dev.read(0x81, len(data))
			message = bytesToString(data)
			print "read {} bytes\nstring: {}\nbytes: {}".format(len(data), message, data)
			print "=======================================================\n"

		else:
			print "Wrong input value. Try again"
