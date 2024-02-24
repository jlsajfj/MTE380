from platformio.public import DeviceMonitorFilterBase

class Data(DeviceMonitorFilterBase):
  NAME = "data"

  def rx(self, text):
    return text
