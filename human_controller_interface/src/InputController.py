import xml.etree.ElementTree
 

class InputController():
	"""
	Parent class for all Input Controllers for IRONLab Fetch Robot
	"""

	def __init__(self, spec):
		self.spec_etree = spec.getroot()
		self.create_control_dict()

	def create_control_dict(self):
		self.control_dict = {}
		for input_signal in self.spec_etree.iter('input'):
			input_name = input_signal.attrib['name']
			x_in = input_signal.find('type')
			self.control_dict[input_name] = {}
			self.control_dict[input_name]["type"] = x_in.text
			self.control_dict[input_name]["input"] = None

	def update_control_signal(self, signal_name, signal_update):
		self.control_dict[signal_name]["input"] = signal_update
		#check for input type match here? or somewhere else?

	def get_all_controls(self):
		return self.control_dict

	def get_control_signal(self, signal_name):
		return self.control_dict[signal_name]