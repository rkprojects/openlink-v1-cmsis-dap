/*

Copyright 2019 Ravikiran Bukkasagara <contact@ravikiranb.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifndef _USB_HID_REPORT_DEF_H_
#define _USB_HID_REPORT_DEF_H_

// Main Items
#define HID_ITEM_Input(x)           0x81,x
#define HID_ITEM_Output(x)          0x91,x
#define HID_ITEM_Feature(x)         0xB1,x
#define HID_ITEM_Collection(x)      0xA1,x
#define HID_ITEM_EndCollection      0xC0

// Global Items
#define HID_ITEM_UsagePage_Vendor(x)    0x06,x,0xff
#define HID_ITEM_LogicalMin(x)          0x15,x
#define HID_ITEM_LogicalMax(x)          0x25,x
#define HID_ITEM_ReportSize(x)          0x75,x
#define HID_ITEM_ReportCount(x)         0x95,x

// Local Items
#define HID_ITEM_Usage(x)           0x09,x

#endif //_USB_HID_REPORT_DEF_H_

