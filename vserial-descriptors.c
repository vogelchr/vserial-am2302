/*
Copyright 2014  Christian Vogel <vogelchr@vogel.cx>

Copyright 2013  Dean Camera (dean [at] fourwalledcubicle [dot] com)

Based on the "VirtualSerial" demo, to be found in the
LUFA (Lightweight USB Framework for AVRs) library.
http://www.fourwalledcubicle.com/LUFA.php
*/


#include "vserial-descriptors.h"

/*
 * Device descriptor structure. This descriptor, located in FLASH memory,
 * describes the overall  device characteristics, including the supported USB
 * version, control endpoint size and the  number of device configurations. The
 * descriptor is read out by the USB host when the enumeration  process begins.
 */

#define USB_DESC_HDR(STRUCT, TYPEID)     \
	.Header = {                      \
		.Size = sizeof(STRUCT),  \
		.Type = (TYPEID)         \
	}

const USB_Descriptor_Device_t PROGMEM DeviceDescriptor = {
	USB_DESC_HDR(USB_Descriptor_Device_t, DTYPE_Device),
	.USBSpecification       = VERSION_BCD(01.10),
	.Class                  = CDC_CSCP_CDCClass,
	.SubClass               = CDC_CSCP_NoSpecificSubclass,
	.Protocol               = CDC_CSCP_NoSpecificProtocol,

	.Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

	.VendorID               = 0x03EB,
	.ProductID              = 0x2044,
	.ReleaseNumber          = VERSION_BCD(00.01),

	.ManufacturerStrIndex   = STRING_ID_Manufacturer,
	.ProductStrIndex        = STRING_ID_Product,
	.SerialNumStrIndex      = USE_INTERNAL_SERIAL,

	.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

/** Configuration descriptor structure. This descriptor, located in FLASH memory, describes the usage
 *  of the device in one of its supported configurations, including information about any device interfaces
 *  and endpoints. The descriptor is read out by the USB host during the enumeration process when selecting
 *  a configuration so that the host may correctly communicate with the USB device.
 */
const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor = {
	.Config = {
		USB_DESC_HDR(USB_Descriptor_Configuration_Header_t,
			     DTYPE_Configuration),

		.TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
		.TotalInterfaces        = 2,

		.ConfigurationNumber    = 1,
		.ConfigurationStrIndex  = NO_DESCRIPTOR,

		.ConfigAttributes       = (USB_CONFIG_ATTR_RESERVED |
					   USB_CONFIG_ATTR_SELFPOWERED),

		.MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
	},
	.CDC_CCI_Interface = {
		USB_DESC_HDR(USB_Descriptor_Interface_t, DTYPE_Interface),

		.InterfaceNumber        = 0,
		.AlternateSetting       = 0,

		.TotalEndpoints         = 1,

		.Class                  = CDC_CSCP_CDCClass,
		.SubClass               = CDC_CSCP_ACMSubclass,
		.Protocol               = CDC_CSCP_ATCommandProtocol,

		.InterfaceStrIndex      = NO_DESCRIPTOR
	},
	.CDC_Functional_Header = {
		USB_DESC_HDR(USB_CDC_Descriptor_FunctionalHeader_t, DTYPE_CSInterface),

		.Subtype                = CDC_DSUBTYPE_CSInterface_Header,

		.CDCSpecification       = VERSION_BCD(01.10),
	},
	.CDC_Functional_ACM = {
		USB_DESC_HDR(USB_CDC_Descriptor_FunctionalACM_t, DTYPE_CSInterface),
		.Subtype                = CDC_DSUBTYPE_CSInterface_ACM,

		.Capabilities           = 0x06,
	},
	.CDC_Functional_Union = {
		USB_DESC_HDR(USB_CDC_Descriptor_FunctionalUnion_t, DTYPE_CSInterface),
		.Subtype                = CDC_DSUBTYPE_CSInterface_Union,

		.MasterInterfaceNumber  = 0,
		.SlaveInterfaceNumber   = 1,
	},
	.CDC_NotificationEndpoint = {
		USB_DESC_HDR(USB_Descriptor_Endpoint_t, DTYPE_Endpoint),
			.EndpointAddress        = CDC_NOTIFICATION_EPADDR,
			.Attributes             = (EP_TYPE_INTERRUPT | 
						   ENDPOINT_ATTR_NO_SYNC |
						   ENDPOINT_USAGE_DATA),
			.EndpointSize           = CDC_NOTIFICATION_EPSIZE,
			.PollingIntervalMS      = 0xFF
	},
	.CDC_DCI_Interface = {
		USB_DESC_HDR(USB_Descriptor_Interface_t, DTYPE_Interface),
		.InterfaceNumber        = 1,
		.AlternateSetting       = 0,

		.TotalEndpoints         = 2,

		.Class                  = CDC_CSCP_CDCDataClass,
		.SubClass               = CDC_CSCP_NoDataSubclass,
		.Protocol               = CDC_CSCP_NoDataProtocol,

		.InterfaceStrIndex      = NO_DESCRIPTOR
	},
	.CDC_DataOutEndpoint = {
		USB_DESC_HDR(USB_Descriptor_Endpoint_t, DTYPE_Endpoint),
			.EndpointAddress        = CDC_RX_EPADDR,
			.Attributes             = (EP_TYPE_BULK |
						   ENDPOINT_ATTR_NO_SYNC |
						   ENDPOINT_USAGE_DATA),
			.EndpointSize           = CDC_TXRX_EPSIZE,
			.PollingIntervalMS      = 0x05
	},
	.CDC_DataInEndpoint = {
		USB_DESC_HDR(USB_Descriptor_Endpoint_t, DTYPE_Endpoint),
			.EndpointAddress        = CDC_TX_EPADDR,
			.Attributes             = (EP_TYPE_BULK |
						   ENDPOINT_ATTR_NO_SYNC |
						   ENDPOINT_USAGE_DATA),
			.EndpointSize           = CDC_TXRX_EPSIZE,
			.PollingIntervalMS      = 0x05
	}
};

/*
 * Language descriptor structure. This descriptor, located in FLASH memory, is
 * returned when the host requests  the string descriptor with index 0 (the
 * first index). It is actually an array of 16-bit integers, which indicate  via
 * the language ID table available at USB.org what languages the device supports
 * for its string descriptors.
 */
const USB_Descriptor_String_t PROGMEM LanguageString =
{
	.Header                 = {
		.Size = USB_STRING_LEN(1),
		.Type = DTYPE_String
	},
	.UnicodeString          = {LANGUAGE_ID_ENG}
};

/*
 * Manufacturer descriptor string. This is a Unicode string containing the
 * manufacturer's details in human readable  form, and is read out upon request
 * by the host when the appropriate string ID is requested, listed in the Device
 * Descriptor.
 */
const USB_Descriptor_String_t PROGMEM ManufacturerString =
{
	.Header                 = {
		.Size = USB_STRING_LEN(11),
		.Type = DTYPE_String
	},
	.UnicodeString          = L"Dean Camera"
};

/*
 * Product descriptor string. This is a Unicode string containing the product's
 * details in human readable form,  and is read out upon request by the host
 * when the appropriate string ID is requested, listed in the Device
 * Descriptor.
 */
const USB_Descriptor_String_t PROGMEM ProductString =
{
	.Header                 = {
		.Size = USB_STRING_LEN(13),
		.Type = DTYPE_String
	},
	.UnicodeString          = L"LUFA CDC Demo"
};

/*
 * This function is called by the library when in device mode, and must be
 * overridden (see library "USB Descriptors"  documentation) by the application
 * code so that the address and size of a requested descriptor can be given  to
 * the USB library. When the device receives a Get Descriptor request on the
 * control endpoint, this function  is called so that the descriptor details can
 * be passed back and the appropriate descriptor sent back to the  USB host.
 */
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    const void** const DescriptorAddress)
{
	const uint8_t  DescriptorType   = (wValue >> 8);
	const uint8_t  DescriptorNumber = (wValue & 0xFF);

	const void* Address = NULL;
	uint16_t    Size    = NO_DESCRIPTOR;

	switch (DescriptorType) {
	case DTYPE_Device:
		Address = &DeviceDescriptor;
		Size    = sizeof(USB_Descriptor_Device_t);
		break;
	case DTYPE_Configuration:
		Address = &ConfigurationDescriptor;
		Size    = sizeof(USB_Descriptor_Configuration_t);
		break;
	case DTYPE_String:
		switch (DescriptorNumber) 		{
		case STRING_ID_Language:
			Address = &LanguageString;
			Size    = pgm_read_byte(&LanguageString.Header.Size);
			break;
		case STRING_ID_Manufacturer:
			Address = &ManufacturerString;
			Size    = pgm_read_byte(&ManufacturerString.Header.Size);
			break;
		case STRING_ID_Product:
			Address = &ProductString;
			Size    = pgm_read_byte(&ProductString.Header.Size);
			break;
		}

		break;
	}
	*DescriptorAddress = Address;
	return Size;
}

